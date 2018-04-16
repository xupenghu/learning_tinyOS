#include "tinyOS.h"
#include "ARMCM3.h"
#include "tlib.h"
#include "config.h"

tTask * currentTask; //当前任务 记录当前是哪个任务在运行
tTask * nextTask; 	//下一个即将要执行的任务 在进行任务切换时，先设置好该值，然后任务切换过程中会从中读取下一个任务的信息

tlist taskTable[TINYOS_PRIO_COUNT];


//任务优先级的位图数据结构
tbitmap task_prio_bitmap;

//延时任务队列
tlist task_delay_list;

void t_task_delay_list_init()
{
	list_init(&task_delay_list);
}

tTask tIdleTask;
tTastStack idleTaskEnv[TINYOS_IDELTASK_STACK_SIZE];


//返回最高优先级的就绪任务
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
void t_task_sched_ready (tTask * task)
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
void t_task_sched_unready (tTask * task)
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
void t_time_task_wait (tTask * task, uint32_t ticks)
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
void t_time_task_wake_up (tTask * task)
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
void t_task_sched ()
{
	// 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
  uint32_t status = t_task_enter_critical();
	tTask * temp_task;
	//如果有任务用了调度锁，直接退出
	if(sched_lock_count > 0)
	{
		t_task_exit_critical(status);
		return;
	}
	
	temp_task = t_task_highest_ready();
	
	if (currentTask != temp_task)
	{
		nextTask = temp_task;
		tTaskSwitch();
	
	}
		// 退出临界区
		t_task_exit_critical(status);  //当推出临界区的时候，PendSV被挂起的中断会立即得到执行。
	
}


void t_task_system_tick_handler(void)
{
	tnode * node;
	tTask * task;
	uint32_t status = t_task_enter_critical();
	//循环遍历扫描延时队列
	for(node = task_delay_list.head_node.nextnode; node != &(task_delay_list.head_node); node = node->nextnode)
	{
		task = tNodeParent(node , tTask, delay_node);
		if (--task->delay_ticks == 0)
		{
			t_time_task_wake_up(task); //将任务从延时队列中移除
			t_task_sched_ready(task);  //将任务设置为可调度状态
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
	
	 t_task_exit_critical(status);

  // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
	t_task_sched();
	
}




void delay(int count)
{
	while(count-- > 0);
	
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
	
	for(i = 0 ; i < TINYOS_PRIO_COUNT; i ++)  //调度列表全部清0
	{
		list_init(&taskTable[i]);
	}
	
}



//禁止任务调度
void disable_task_sched()
{
	// 进入临界区
    uint32_t status = t_task_enter_critical();	
	
		if (sched_lock_count < 255)
		{
			sched_lock_count ++;
		}
	
	// 退出临界区
    t_task_exit_critical(status);  

}

//允许任务调度
void enable_task_sched()
{
	uint32_t status = t_task_enter_critical();	
	if(sched_lock_count > 0)
	{
		if(--sched_lock_count == 0)
		{
			t_task_sched ();
		}
	}
	t_task_exit_critical(status); 
}


int main(void)
{
	task_sched_init(); //调度部分初始化，
	t_task_delay_list_init();
	app_init();

	tTaskInit(&tIdleTask, idleTaskEntry, (void *)0, TINYOS_PRIO_COUNT-1, &idleTaskEnv[TINYOS_IDELTASK_STACK_SIZE]);

	// 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = t_task_highest_ready();
	
	//切换到nextTask 这个函数不会返回
	tTaskRunFirst();

	return 0;
}













