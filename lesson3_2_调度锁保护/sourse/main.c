#include "tinyOS.h"
#include "ARMCM3.h"


tTask * currentTask; //当前任务 记录当前是哪个任务在运行
tTask * nextTask; 	//下一个即将要执行的任务 在进行任务切换时，先设置好该值，然后任务切换过程中会从中读取下一个任务的信息

tTask * taskTable[2];
tTask * IdleTask;

tTask tTask1;
tTask tTask2;
tTask tIdleTask;

tTastStack task1Env[1024];
tTastStack task2Env[1024];
tTastStack idleTaskEnv[1024];

void tTaskInit (tTask *task, void (*entry)(void *), void *param, tTastStack * stack)
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
	//如果有任务用了调度锁，直接退出
	if(sched_lock_count > 0)
	{
		tTaskExitCritical(status);
		return;
	}
	
	
	if (currentTask == IdleTask)
	{
		if(taskTable[0]->delayTicks == 0)
		{
			nextTask = taskTable[0];
		}
		else if ( 0 == taskTable[0])
		{
			nextTask = taskTable[1];
		}
		else
		{
			return;
		}
	}
	else 
	{
		if(currentTask == taskTable[0])
		{
			if (0 == taskTable[1]->delayTicks)
			{
				nextTask = taskTable[1];
			}
			else if (currentTask->delayTicks != 0)
			{
				nextTask = IdleTask;
			}
			else
				return;
		}
		else if(currentTask == taskTable[1])
		{
			if (0 == taskTable[0]->delayTicks)
			{
				nextTask = taskTable[0];
			}
			else if (currentTask->delayTicks != 0)
			{
				nextTask = IdleTask;
			}
			else 
				return;
		}
	}

	tTaskSwitch();
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
	int i;
	for (i = 0; i < 2; i ++)
	{
		if(taskTable[i]->delayTicks > 0)
		{
			taskTable[i]->delayTicks--;
		}
	}
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
	//当前任务的软件定时器置相应的延时节拍
	currentTask->delayTicks = delay;
	
	tTaskSched();
}

int task1flag;
void task1Entry (void *param)
{
	uint32_t status; //保存上一次的状态，可以进行临界区嵌套。
	tSetSysTickPeriod(10);
	for(;;)
	{
		task1flag = 0;
		tTaskDelay(1);
		task1flag = 1;
		tTaskDelay(1);
	}

}

int task2flag;
void task2Entry (void *param)
{
	for(;;)
	{
		disable_task_sched();  //启用调度锁
		task2flag = 0;
		tTaskDelay(1);
		task2flag = 1;
		tTaskDelay(1);
		enable_task_sched();  //失能调度锁
	}
}

void idleTaskEntry (void *param)
{
	for(;;)
	{
		
	}
}

//调度锁部分
uint8_t sched_lock_count;

void task_sched_init()
{
	sched_lock_count = 0;
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
	task_sched_init(); //调度部分初始化，目前只初始化了调度锁
	
	// 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
	tTaskInit(&tTask1, task1Entry, (void* )0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, &task2Env[1024]);
	
	//将任务加入到任务表中
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	
	tTaskInit(&tIdleTask, idleTaskEntry, (void *)0, &idleTaskEnv[1024]);
	IdleTask = &tIdleTask;
	
	//我们希望先运行任务1
	nextTask = taskTable[0];
	
	//切换到nextTask 这个函数不会返回
	tTaskRunFirst();

	return 0;
}













