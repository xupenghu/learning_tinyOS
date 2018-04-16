#include "tinyOS.h"
#include "config.h"



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
	task->suspend_count = 0; 				//初始不挂起
	task->task_request_delete_flag = 0; //没有请求被删除
	task->clean = (void(*)(void *))0;		//初始化不设置清理函数
	task->clean_param = (void *)0;
	
	tnode_init(&task->delay_node);	//初始化延时结点
	
	tnode_init(&task->link_node);
	
	
	t_task_sched_ready(task);  //最后将任务设为就绪态

}
//任务挂起
void t_task_suspend(tTask * task)
{
	uint32_t status = t_task_enter_critical();
	//如果任务状态不是延时
	if(!(task->state & TINYOS_TASK_STATE_DELAYED))
	{
		//如果是第一次挂起，将挂起标志位置1 然后将任务从任务队列中移除
		if(++task->suspend_count <= 1)
		{
			task->state |= TINYOS_TASK_STATE_SUSPEND;
			
			t_task_sched_unready(task);
			//如果是自己挂起自己，那么切换到其他任务运行
			if(task == currentTask)
			{
				t_task_sched();
			}
			
		}
		
	}
	
	t_task_exit_critical(status);
}

//挂起任务的恢复
void t_suspend_task_wake_up(tTask * task)
{
	uint32_t status = t_task_enter_critical();
	
	//如果任务处于挂起状态
	if(task->state & TINYOS_TASK_STATE_SUSPEND)
	{
		if(--(task->suspend_count) == 0)
		{
			//清除挂起标志
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;
			//将任务加入到就绪队列中
			t_task_sched_ready(task);
			//执行一次任务调度
			t_task_sched();
			
		}
	}
	
	
	t_task_exit_critical(status);
}

/**********************************************************************************************************
** Function name        :   tTaskSetCleanCallFunc
** Descriptions         :   设置任务被删除时调用的清理函数
** parameters           :   task  待设置的任务
** parameters           :   clean  清理函数入口地址
** parameters           :   param  传递给清理函数的参数
** Returned value       :   无
***********************************************************************************************************/
void t_task_set_clean_call_func (tTask * task, void (*clean)(void * param), void * param) 
{
    task->clean = clean;
    task->clean_param = param;
}

/**********************************************************************************************************
** Function name        :   tTaskForceDelete
** Descriptions         :   强制删除指定的任务
** parameters           :   task  需要删除的任务
** Returned value       :   无
***********************************************************************************************************/
void t_task_force_delete (tTask * task) 
{
	//进入临界区
	uint32_t status = t_task_enter_critical();
	//如果任务处于延时状态，那么从延时队列中删除任务
	if(task->state & TINYOS_TASK_STATE_DELAYED)
	{
		t_time_task_remove(task);
	}
	//如果任务处于非挂起状态则就是就绪状态，从调度队列中删除任务
	else if(!(task->state & TINYOS_TASK_STATE_SUSPEND))
	{
		t_task_sched_remove(task);
	}
	
	//如果定义了任务清理函数，则调用
	if(task->clean)
	{
			task->clean(task->clean_param);
	}
	
	//如果删除任务是当前正在运行的任务，则执行一次调度
	if(task == currentTask)
	{
		t_task_sched();
	}
	
	//退出临界区
	t_task_exit_critical(status);
}


/**********************************************************************************************************
** Function name        :   tTaskRequestDelete
** Descriptions         :   请求删除某个任务，由任务自己决定是否删除自己
** parameters           :   task  需要删除的任务
** Returned value       :   无
***********************************************************************************************************/
void t_task_request_delete (tTask * task)
{
	uint32_t status = t_task_enter_critical();
	//相应任务请求删除标志自家1
	task->task_request_delete_flag++;
	
	t_task_exit_critical(status);
}

/**********************************************************************************************************
** Function name        :   tTaskIsRequestedDelete
** Descriptions         :   是否已经被请求删除自己
** parameters           :   无
** Returned value       :   非0表示请求删除，0表示无请求
***********************************************************************************************************/
uint8_t t_task_is_requested_delete (void)
{
	uint32_t status = t_task_enter_critical();
	uint8_t delet = 0;
	delet = currentTask->task_request_delete_flag;
	t_task_exit_critical(status);
	return delet;
}

/**********************************************************************************************************
** Function name        :   tTaskDeleteSelf
** Descriptions         :   删除自己
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void t_task_delete_self (void)
{
	uint32_t status = t_task_enter_critical();
	//移除调度队列
	t_task_sched_remove(currentTask);
	
	//运行清理函数
	if(currentTask->clean)
	{
		currentTask->clean(currentTask->clean_param);
	}
	
	//执行一次调度 调度到其他任务中去
	t_task_sched();
	
	t_task_exit_critical(status);

}


//获取任务状态信息
void t_get_task_info(tTask * task, task_info * info)
{
	uint32_t status = t_task_enter_critical();
	info->delay_ticks = task->delay_ticks;
	info->suspend_count = task->suspend_count;
	info->prio = task->prio;
	info->slice	=	task->slice;
	info->state	=	task->state;
	t_task_exit_critical(status);
}






