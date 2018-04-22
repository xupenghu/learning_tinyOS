#include "tinyOS.h"

void t_mutex_init(t_mutex * mutex)
{
	t_event_init(&mutex->event, t_event_type_mutex);
	mutex->lock_count = 0;
	mutex->owner = (tTask *)0;
	
	mutex->owner_origin_prio = TINYOS_PRIO_COUNT;
	
}

/**********************************************************************************************************
** Function name        :   tMutexWait
** Descriptions         :   等待信号量
** parameters           :   mutex 等待的信号量
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_mutex_wait (t_mutex * mutex, uint32_t wait_ticks)
{
	uint32_t status = t_task_enter_critical();
	
	//如果互斥锁没有使用 则直接使用
	if(mutex->lock_count <= 0)
	{
		mutex->owner = currentTask;
		mutex->lock_count ++;
		mutex->owner_origin_prio = currentTask->prio;
		
		t_task_exit_critical(status);
		return t_error_no_error;
		
	}
	else
	{
		//如果是当前任务再次申请使用互斥锁 则只需把互斥锁计数值加1
		if(mutex->owner == currentTask)
		{
			mutex->lock_count ++;
			t_task_exit_critical(status);
			
			return t_error_no_error;
			
		}
		else
		{
			//如果当前申请任务优先级高于已获得该锁的优先级 则执行优先级继承 保证当前任务更快的获取该锁
			if(currentTask->prio < mutex->owner->prio )
			{
				
				tTask * owner = mutex->owner;
				
				if(owner->state == TINYOS_TASK_STATE_RDY)
				{
					//将任务从就绪列表中移除
					t_task_sched_unready(owner);
					//修改优先级
					owner->prio = currentTask->prio;
					//将任务加入到就绪列表中
					t_task_sched_ready(owner);
					
					
				}
				else
				{
					owner->prio = currentTask->prio;
				}
				
			}
			
							
			t_event_wait(&mutex->event, currentTask, 0, t_event_type_mutex, wait_ticks);
			t_task_exit_critical(status);
			//执行一次任务调度 切换到其他任务中去执行
			t_task_sched();
			
			//获得了该锁 返回获取事件的结果
			return currentTask->wait_event_resalt;
			
		}
	}
}



/**********************************************************************************************************
** Function name        :   tMutexNoWaitGet
** Descriptions         :   获取信号量，如果已经被锁定，立即返回
** parameters           :   tMutex 获取的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_mutex_no_wait_get (t_mutex * mutex)
{
	uint32_t status = t_task_enter_critical();
	
		//如果互斥锁没有使用 则直接使用
	if(mutex->lock_count <= 0)
	{
		mutex->owner = currentTask;
		mutex->lock_count ++;
		mutex->owner_origin_prio = currentTask->prio;
		
		t_task_exit_critical(status);
		return t_error_no_error;
		
	}
	else
	{
		//如果是当前任务再次申请使用互斥锁 则只需把互斥锁计数值加1
		if(mutex->owner == currentTask)
		{
			mutex->lock_count ++;
			t_task_exit_critical(status);
			
			return t_error_no_error;
			
		}

		t_task_exit_critical(status);			
		return t_error_resource_unavaliable;

	}
	
}


/**********************************************************************************************************
** Function name        :   tMutexNotify
** Descriptions         :   通知互斥信号量可用
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t t_mutex_notify (t_mutex * mutex)
{
	uint32_t status = t_task_enter_critical();
	
	//如果信号量没有被锁定 直接退出
	if(mutex->lock_count <= 0)
	{
		t_task_exit_critical(status);
		return t_error_no_error;
	}
	
	//如果不是当前任务通知 直接退出
	if(mutex->owner != currentTask)
	{
		t_task_exit_critical(status);
		return t_error_owner;
	}
	
	//如果锁定计数不为0 则减1后退出
	if(--mutex->lock_count > 0)
	{
		t_task_exit_critical(status);
		return t_error_no_error;
	}
	
	//如果有优先级继承 则将优先级继承还原
	if(mutex->owner_origin_prio != mutex->owner->prio)
	{
		if(mutex->owner->state == TINYOS_TASK_STATE_RDY)
		{
			//将任务从就绪列表中移除
			t_task_sched_unready(mutex->owner);
			//修改优先级
			currentTask->prio = mutex->owner_origin_prio;
			//将任务加入到就绪列表中
			t_task_sched_ready(mutex->owner);

		}
		else
		{
			currentTask->prio = mutex->owner_origin_prio;
		}
	
	}
		
	//如果有等待任务 则唤醒等待队列头部的任务执行 并让此任务从新上锁
	if(t_event_wait_count(&mutex->event) > 0)
	{
		tTask * task = t_event_wake_up(&mutex->event, 0, t_error_no_error);
		
		mutex->owner = task;
		mutex->lock_count ++;
		mutex->owner_origin_prio = task->prio;
		
		if(task->prio < currentTask->prio)
		{
			t_task_sched();
		}
		
	}
	t_task_exit_critical(status);
	return t_error_no_error;

}

/**********************************************************************************************************
** Function name        :   tMutexGetInfo
** Descriptions         :   查询状态信息
** parameters           :   mutex 查询的互斥信号量
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void t_mutex_get_info (t_mutex * mutex, t_mutex_info * info)
{
	uint32_t status = t_task_enter_critical();
	
	//拷贝各个需要的信息
	info->lock_count = mutex->lock_count;
	
	info->owner = mutex->owner;
	
	info->wait_task_count = t_event_wait_count(&mutex->event);
	
	info->owner_prio = mutex->owner_origin_prio;
	
	//如果当前锁没有被使用 继承优先级无效
	if(mutex->owner != (tTask *)0)
	{
		
		info->inherited_prio = mutex->owner->prio;
	
	}
	else
	{
		info->inherited_prio = TINYOS_PRIO_COUNT;
	}

	t_task_exit_critical(status);
}


/**********************************************************************************************************
** Function name        :   tMutexDestroy
** Descriptions         :   销毁信号量
** parameters           :   mutex 销毁互斥信号量
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t t_mutex_destroy (t_mutex * mutex)
{

	uint32_t count;
	uint32_t status = t_task_enter_critical();
	
	//如果锁定计数器不为0 则说明有任务使用该互斥锁
	if(mutex->lock_count > 0)
	{
		//判断是否使用了优先级继承
		if(mutex->owner->prio != mutex->owner_origin_prio)
		{
			if(mutex->owner->state == TINYOS_TASK_STATE_RDY)
			{
				t_task_sched_unready(mutex->owner);
				
				mutex->owner->prio = mutex->owner_origin_prio;
				
				t_task_sched_ready(mutex->owner);
				
			}
			else
			{
				mutex->owner->prio = mutex->owner_origin_prio;
			}
		}
		//销毁所以等待列表中的任务
		count = t_event_remove_all(&mutex->event, (void *)0, t_error_del);
		//如果有任务 就执行一次任务调度
		if(count > 0)
		{
			t_task_sched();
		}
	
	}
	
	t_task_exit_critical(status);
	//返回被释放的任务数量
	return count;
}



