
#include "tinyOS.h"


//邮箱初始化
void t_mbox_init(t_mbox * mbox, uint32_t max_count, void ** msgbuffer)
{
	t_event_init(&mbox->event, t_event_type_mbox);
	
	mbox->count = 0;
	mbox->write = 0;
	mbox->read = 0;
	mbox->msgbuffer = msgbuffer;
	mbox->max_count = max_count;

}

/**********************************************************************************************************
** Function name        :   tMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   mbox 等待的邮箱
** parameters           :   msg 消息存储缓存区
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_mbox_wait (t_mbox * mbox, void **msg, uint32_t wait_ticks) 
{
	uint32_t status = t_task_enter_critical();
	
	//如果邮箱里有消息，则直接将消息取出来返回
	if(mbox->count > 0)
	{
		* msg = mbox->msgbuffer[mbox->read++];
		--mbox->count;
		//如果读到末尾了 则再将读索引移到头部
		if(mbox->read > mbox->max_count)
		{
			mbox->read = 0;
		}
		
		t_task_exit_critical(status);
		return t_error_no_error;
	}
	else
	{
		//否则将当前任务加入到等待邮箱的等待队列中去
		t_event_wait(&mbox->event, currentTask, (void*)0, t_event_type_mbox, wait_ticks);
		t_task_exit_critical(status);
		//执行一次任务调度 切换到其他任务
		t_task_sched();
		
		//运行到这里说明已经等到消息了，所以就直接从当前任务中读取等到的消息
		* msg = currentTask->event_msg;
		
		return currentTask->wait_event_resalt;

	}

}

/**********************************************************************************************************
** Function name        :   tMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   mbox 获取消息的邮箱
** parameters           :   msg 消息存储缓存区
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_mbox_no_wait_get (t_mbox * mbox, void **msg)
{
	uint32_t status = t_task_enter_critical();
	if(mbox->count > 0)
	{
		* msg = mbox->msgbuffer[mbox->read++];
		--mbox->count;
		if(mbox->read > mbox->max_count)
		{
			mbox->read = 0;
		}
		
		t_task_exit_critical(status);
		return t_error_no_error;
	}	
	else
	{
			t_task_exit_critical(status);
			return t_error_resource_unavaliable;
	}
	
}

/**********************************************************************************************************
** Function name        :   tMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t t_mbox_notify (t_mbox * mbox, void * msg, uint32_t notify_option)
{
	uint32_t status = t_task_enter_critical();
	
	//如果有任务在等待
	if(t_event_wait_count(&mbox->event) > 0)
	{
		//直接唤醒该任务到调度队列中去，注意唤醒该任务时已经将msg传入该任务的event_msg字段了
		tTask * task = t_event_wake_up(&mbox->event, (void * )msg, t_error_no_error);
		//如果该任务的优先级比当前任务的优先级高，则直接调度过去执行该任务
		if(task->prio < currentTask->prio)
		{
			t_task_sched();
		}
	}
	else
	{
		//如果计数缓冲区满了 则直接退出
		if(mbox->count > mbox->max_count)
		{
			t_task_exit_critical(status);
			return t_error_resoure_full;
		}
		
		//如果是按照优先级发送的 则直接在read索引处写值
		if(notify_option & tMBOXSendFront)
		{
			if(mbox->read <= 0)
			{
				mbox->read = mbox->max_count - 1;
			}
			else
			{
				mbox->read --;
			}
			
			mbox->msgbuffer[mbox->read] = msg;

		}
		else //否则就在write索引处写值
		{
			mbox->msgbuffer[mbox->write++] = msg ;
			
			if(mbox->write > mbox->max_count)
			{
				mbox->write = 0;
			}
			
		}
		//消息计数量＋＋
		mbox->count ++;
		
	}
	
	t_task_exit_critical(status);
	return t_error_no_error;
}


