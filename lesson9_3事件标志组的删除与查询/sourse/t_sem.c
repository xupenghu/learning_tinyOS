#include "t_sem.h"
#include "tinyOS.h"


//信号量初始化
void t_sem_init(t_sem * sem, uint32_t start_count, uint32_t max_count)
{
	t_event_init(&sem->event, t_event_type_sem);
	
	sem->max_count = max_count;
	
	if(max_count == 0)
	{
		sem->count = start_count;
	}
	else
	{
		sem->count = (start_count > max_count) ? max_count : start_count;
	}

}

/**********************************************************************************************************
** Function name        :   tSemWait
** Descriptions         :   等待信号量
** parameters           :   sem 等待的信号量
** parameters           :   waitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_sem_wait (t_sem * sem, uint32_t waitTicks)
{
	uint32_t status = t_task_enter_critical();
	
	//如果计数值大于0 则说明当前条件成立 则不需要等待 直接退出 并且消耗了一个资源
	if(sem->count > 0)
	{
		--sem->count;
		t_task_exit_critical(status);
		return t_error_no_error;
		
	}
	//否则将当前任务挂起，切换到其他任务中运行
	else
	{
		//加入到等待队列中去
		t_event_wait(&sem->event, currentTask, 0, t_event_type_sem, waitTicks);
		t_task_exit_critical(status);
		//切换到其他任务运行
		t_task_sched();
		
		//执行到这句说明已经切换到当前任务继续运行了，那么我们就从当前任务中取出等待的结果
		return currentTask->wait_event_resalt;
		
	}
}

/**********************************************************************************************************
** Function name        :   tSemNoWaitGet
** Descriptions         :   获取信号量，如果信号量计数不可用，则立即退回
** parameters           :   sem 等待的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_sem_no_wait_get (t_sem * sem)
{
	uint32_t status = t_task_enter_critical();
	//首先检查一下信号量计数是否大于0 如果大于0 就消耗掉一个，然后返回没有错误
	if(sem->count > 0)
	{
		--sem->count;
		t_task_exit_critical(status);
		return t_error_no_error;
	}
	else //否则直接返回信号量不可用
	{
		t_task_exit_critical(status);
		return t_error_resource_unavaliable;
	}
	
}

/**********************************************************************************************************
** Function name        :   tSemNotify
** Descriptions         :   通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
** parameters           :   sem 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void t_sem_notify (t_sem * sem)
{
	uint32_t status = t_task_enter_critical();
	
	//如果当前信号量有任务在等待
	if(t_event_wait_count(&sem->event) > 0)
	{
		//则直接将该任务从等待队列中唤醒
		tTask * task = t_event_wake_up(&sem->event, 0, t_error_no_error);
		
		//如果该任务大于当前正在执行的任务，则执行任务调度函数
		if(task->prio > currentTask->prio)
		{
			t_task_sched();
		}
		
	}
	else //否则就将信号量的计数值加1
	{
		++sem->count;
		//如果计数值大于最大值，那么计数值等于最大值
		if((sem->max_count !=0 ) && (sem->count > sem->max_count))
		{
			sem->count = sem->max_count;
		}
	}
	t_task_exit_critical(status);
}


