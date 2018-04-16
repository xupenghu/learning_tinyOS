#include "tinyOS.h"
#include "ARMCM3.h"
#include "tlib.h"
#include "config.h"

tTask * currentTask; //当前任务 记录当前是哪个任务在运行
tTask * nextTask; 	//下一个即将要执行的任务 在进行任务切换时，先设置好该值，然后任务切换过程中会从中读取下一个任务的信息

tlist taskTable[TINYOS_PRO_COUNT];

tTask tTask1;
tTask tTask2;
tTask	tTask3;
tTask tIdleTask;

//任务优先级的位图数据结构
tbitmap task_prio_bitmap;

//延时任务队列
tlist task_delay_list;

void ttask_delay_list_init()
{
	list_init(&task_delay_list);
}




tTastStack task1Env[1024];
tTastStack task2Env[1024];
tTastStack task3Env[1024];

tTastStack idleTaskEnv[1024];



void tTaskInit (tTask *task, void (*entry)(void *), void *param, uint32_t prio,  tTastStack * stack)
{
	*(--stack) = (unsigned long )(1<<24); //XPSR 	设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
	*(--stack) = (unsigned long )entry;		//程序的入口地址
	*(--stack) = (unsigned long )0x14;		//R14(LR)寄存器 任务不会通过return来结束自己，所以没有使用
	*(--stack) = (unsigned long )0x12;		//R12 没有使用
	*(--stack) = (unsigned long )0x3;		
	*(--stack) = (unsigned long )0x2;
	*(--stack) = (unsigned long )0x1;
	*(--stack) = (unsigned long )param;		//程序传入参数保存在R0寄存器中
	//以上这些操作在PendSV中会自动由硬件完成，不需要手动保存，同样的在推出PendSV中会由硬件自动弹出，所以在保存现场和退出现场时，我们不需要手动来完成这些操作
	
	//R4-R11的值需要我们手动来保存，初始化时没有使用这些寄存器的值，所有就直接初始化成这些寄存器的对应位号，方便调试时来查看对应寄存器的值
	*(--stack) = (unsigned long )0x11;
	*(--stack) = (unsigned long )0x10;
	*(--stack) = (unsigned long )0x09;
	*(--stack) = (unsigned long )0x08;
	*(--stack) = (unsigned long )0x07;
	*(--stack) = (unsigned long )0x06;
	*(--stack) = (unsigned long )0x05;
	*(--stack) = (unsigned long )0x04;

	
	task->stack = stack;   //保存最终的stack值 让该任务的堆栈指针指向栈顶
	

	task->slice = TINYOS_SLICE_MAX;
	
	task->delay_ticks = 0;
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY ;
	
	tnode_init(&task->delay_node);	//初始化延时结点
	
	tnode_init(&task->link_node);
	tlist_add_last(&taskTable[prio], &task->link_node);		//将初始化好的任务装入tasktable中
	
	tbitmap_setbit(&task_prio_bitmap, prio); //将相应优先级的位图置1 使得任务处于就绪状态
	
	
}

tTask * t_task_highest_ready()
{
	uint32_t highest_prio;
	tnode * node;
	highest_prio = tbitmap_get_first_set(&task_prio_bitmap);
	node = tlist_first(&taskTable[highest_prio]);
	return (tTask *)tNodeParent(node, tTask, link_node);
}

/**********************************************************************************************************
** Function name        :   ttask_sched_ready
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void ttask_sched_ready (tTask * task)
{
	
	tlist_add_last(&taskTable[task->prio], &(task->link_node));
	tbitmap_setbit(&task_prio_bitmap, task->prio);
	
}

/**********************************************************************************************************
** Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task   
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void ttask_sched_unready (tTask * task)
{
	tlist_remove(&taskTable[task->prio], &(task->link_node));
	
	if(list_count(&taskTable[task->prio]) == 0)
	{
		tbitmap_clearbit(&task_prio_bitmap, task->prio);
	}
	
}

/**********************************************************************************************************
** Function name        :   ttime_task_wait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void ttime_task_wait (tTask * task, uint32_t ticks)
{
	tlist_add_first(&task_delay_list, &task->delay_node);
	task->delay_ticks = ticks;
	task->state |= TINYOS_TASK_STATE_DELAYED;
}
/**********************************************************************************************************
** Function name        :   tTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   task  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void ttime_task_wake_up (tTask * task)
{
	tlist_remove(&task_delay_list, &task->delay_node);
	task->state &= ~TINYOS_TASK_STATE_DELAYED;
}

/*
*调度函数的算法是：
*1、当前任务如果是空闲任务，就判断taskTable里的任务的delayTicks是否为0，如果为0，说明延时时间到，就切换到该任务继续执行，如果不为0，则说明该
*任务还需要继续休眠。	那么就不做任何切换，继续执行空闲任务。
*2、当前任务如果是其他任务，那么就判断另一个任务的delayTicks是否为0，如果为0，则切换到该任务继续执行，如果不为0，则判断自身的delayTicks是否为0，如果为0
*		那么自身就需要继续运行，如果不为0，那么就交出CPU的运行权。运行空闲任务。
*/
void tTaskSched ()
{
	// 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
  uint32_t status = tTaskEnterCritical();
	tTask * temp_task;
	//如果有任务用了调度锁，直接退出
	if(sched_lock_count > 0)
	{
		tTaskExitCritical(status);
		return;
	}
	
	temp_task = t_task_highest_ready();
	
	if (currentTask != temp_task)
	{
		nextTask = temp_task;
		tTaskSwitch();
	
	}
		// 退出临界区
		tTaskExitCritical(status);  //当推出临界区的时候，PendSV被挂起的中断会立即得到执行。
	
}

/*********************************************************************************************************
** 系统时钟节拍定时器System Tick配置
** 在我们目前的环境（模拟器）中，系统时钟节拍为12MHz
** 请务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
**********************************************************************************************************/
void tSetSysTickPeriod(uint32_t ms)
{
  SysTick->LOAD  = ms * SystemCoreClock / 1000 - 1; 
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
  SysTick->VAL   = 0;                           
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk; 
}

void tTaskSystemTickHandler(void)
{
	tnode * node;
	tTask * task;
	uint32_t status = tTaskEnterCritical();
	//循环遍历扫描延时队列
	for(node = task_delay_list.head_node.nextnode; node != &(task_delay_list.head_node); node = node->nextnode)
	{
		task = tNodeParent(node , tTask, delay_node);
		if (--task->delay_ticks == 0)
		{
			ttime_task_wake_up(task); //将任务从延时队列中移除
			ttask_sched_ready(task);  //将任务设置为可调度状态
		}
		
	}

	if(--(currentTask->slice) == 0) //当前任务的时间片已用完
	{
		if(list_count(&taskTable[currentTask->prio]) > 0)
		{
			tlist_remove_first(&taskTable[currentTask->prio]); 					//移除第第一个结点
			tlist_add_last(&taskTable[currentTask->prio], &(currentTask->link_node)); //将第一个节点加入到列表的最后一个节点中去
			
			// 重置计数器
      currentTask->slice = TINYOS_SLICE_MAX;
			
		}
	}
	
	 tTaskExitCritical(status);

  // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
	tTaskSched();
	
}


/**********************************************************************************************************
** Function name        :   SysTick_Handler
** Descriptions         :   SystemTick的中断处理函数。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void SysTick_Handler () 
{
    // 什么都没做，除了进行任务切换
    // 由于tTaskSched自动选择另一个任务切换过去，所以其效果就是
    // 两个任务交替运行，与上一次例子不同的是，这是由系统时钟节拍推动的
    // 如果说，上一个例子里需要每个任务主动去调用tTaskSched切换，那么这里就是不管任务愿不愿意，CPU
    // 的运行权都会被交给另一个任务。这样对每个任务就很公平了，不存在某个任务拒不调用tTaskSched而一直占用CPU的情况
    tTaskSystemTickHandler();
}


void delay(int count)
{
	while(count-- > 0);
	
}
void tTaskDelay (uint32_t delay)
{
	 uint32_t status = tTaskEnterCritical();
	//当前任务的软件定时器置相应的延时节拍
	ttime_task_wait(currentTask, delay);
	ttask_sched_unready(currentTask);
	
	tTaskExitCritical(status);
	
	tTaskSched();
}

int task1flag;
void task1Entry (void *param)
{

	tSetSysTickPeriod(10);
	for(;;)
	{
		task1flag = 0;
		tTaskDelay(1);
		task1flag = 1;
		tTaskDelay(1);
	}

}

// 

int task2flag;
void task2Entry (void *param)
{
	for(;;)
	{
		task2flag = 0;
		delay(100);
		task2flag = 1;
		delay(100);
	}
}

int task3flag;
void task3Entry (void *param)
{
	for(;;)
	{
		task3flag = 0;
		delay(100);
		task3flag = 1;
		delay(100);
	}
}

uint32_t idle_count;

void idleTaskEntry (void *param)
{
	for(;;)
	{
		idle_count ++;
	}
}

//调度锁部分
uint8_t sched_lock_count;

void task_sched_init()
{
	int i ;
	sched_lock_count = 0;
	tbitmap_init(&task_prio_bitmap); //调度优先级位图全部清0
	
	for(i = 0 ; i < TINYOS_PRO_COUNT; i ++)  //调度列表全部清0
	{
		list_init(&taskTable[i]);
	}
	
}



//禁止任务调度
void disable_task_sched()
{
	// 进入临界区
    uint32_t status = tTaskEnterCritical();	
	
		if (sched_lock_count < 255)
		{
			sched_lock_count ++;
		}
	
	// 退出临界区
    tTaskExitCritical(status);  

}

//允许任务调度
void enable_task_sched()
{
	uint32_t status = tTaskEnterCritical();	
	if(sched_lock_count > 0)
	{
		if(--sched_lock_count == 0)
		{
			tTaskSched ();
		}
	}
	tTaskExitCritical(status); 
}


int main(void)
{
	task_sched_init(); //调度部分初始化，
	ttask_delay_list_init();
	
	// 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
	tTaskInit(&tTask1, task1Entry, (void* )0x11111111, 0, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
	
	tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 1, &task3Env[1024]);
	

	
	tTaskInit(&tIdleTask, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT-1, &idleTaskEnv[1024]);

	
	// 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = t_task_highest_ready();
	
	//切换到nextTask 这个函数不会返回
	tTaskRunFirst();

	return 0;
}













