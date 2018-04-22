#include "tinyOS.h"



/**********************************************************************************************************
** Function name        :   tEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   event 事件控制块
** parameters           :   type 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void t_event_init(t_event * event, t_event_type type)
{
	event->type	=	type;
	list_init(&event->wait_list);
	
}

/**********************************************************************************************************
** Function name        :   tEventWait
** Descriptions         :   让指定在事件控制块上等待事件发生
** parameters           :   event 事件控制块
** parameters           :   task 等待事件发生的任务
** parameters           :   msg 事件消息存储的具体位置
** parameters           :   state 消息类型
** parameters           :   timeout 等待多长时间
** Returned value       :   
***********************************************************************************************************/
void t_event_wait (t_event * event, tTask * task, void * msg, uint32_t state, uint32_t timeout)
{
	uint32_t status = t_task_enter_critical();
	
	task->state |= state;			//标记任务处于某种等待事件的状态
	task->event_msg = msg;		//设置任务等待事件消息的存储位置
	task->wait_event = event;	//设置任务等待事件的事件结构
	
	task->wait_event_resalt = t_error_no_error;  	//清空等待事件的返回结果
	
	t_task_sched_unready(task);								//将任务从就绪队列中移除
	
	tlist_add_last(&event->wait_list, &task->link_node);   //将任务插入到等待队列中的末尾
	
	if(timeout)							//如果有超时时间，则将任务同时加入到延时队列中去
	{												//当时间到达时，有延时机制将任务从延时队列中删除 同时从事件列表中移除
		t_time_task_wait(task, timeout);
	}
	
	t_task_exit_critical(status);
}

/**********************************************************************************************************
** Function name        :   tEventRemoveTask
** Descriptions         :   将任务从其等待队列中强制移除
** parameters           :   task 待移除的任务
** parameters           :   result 告知事件的等待结果 如超时等
** Returned value       :   无
***********************************************************************************************************/
void t_event_remove_task (tTask * task, void * msg, uint32_t result)
{
	uint32_t status = t_task_enter_critical();
	
	//将任务从等待队列中移除
	tlist_remove(&task->wait_event->wait_list, &task->link_node);
	
	//将任务中等待事件相关字段清0
	task->state &= ~TINYOS_TASK_WAIT_MASK;
	task->wait_event_resalt = result;
	task->event_msg = msg;
	task->wait_event = (t_event *)0;
	
	t_task_exit_critical(status);
}

/**********************************************************************************************************
** Function name        :   tEventWakeUp
** Descriptions         :   从事件控制块中唤醒首个等待的任务 注意 我们只唤醒了一个任务 当需要唤醒等待队列中的全部任务时，就需要循环调用该函数了
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
tTask * t_event_wake_up (t_event * event, void * msg, uint32_t result)
{
	tnode * node = (tnode *)0;
	tTask * task = (tTask *)0;
	uint32_t status = t_task_enter_critical();
	
	if((node = tlist_remove_first(&event->wait_list)) != (tnode *)0)
	{
		task = (tTask *)tNodeParent(node, tTask, link_node);
		
		//将处于任务处于等待的相关字段设置
		task->event_msg = msg;
		task->wait_event_resalt = result;
		task->wait_event = (t_event *)0;
		task->state &= ~TINYOS_TASK_WAIT_MASK;
		
		//如果设置了等待超时，说明此任务也在延时列表中，我们直接将此任务从延时列表中删除
		if(task->delay_ticks)
		{
			t_time_task_wake_up(task);
		}
		
		//最后再将任务加入到调度列表中去
		t_task_sched_ready(task);
		
	}
	
	t_task_exit_critical(status);
	return task;
}


/**********************************************************************************************************
** Function name        :   tEventWakeUpTask
** Descriptions         :   从事件控制块中唤醒指定任务
** parameters           :   event 事件控制块
** parameters           :   task 等待唤醒的任务
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/
void t_event_wake_up_task (t_event * event, tTask * task, void * msg, uint32_t result)
{
	tnode * node = (tnode *)0;
	uint32_t status = t_task_enter_critical();
	
	//首先将任务从等待列表中移除
	 tlist_remove(&event->wait_list, &task->link_node);

		//将处于任务处于等待的相关字段设置
		task->event_msg = msg;
		task->wait_event_resalt = result;
		task->wait_event = (t_event *)0;
		task->state &= ~TINYOS_TASK_WAIT_MASK;
		
		//如果设置了等待超时，说明此任务也在延时列表中，我们直接将此任务从延时列表中删除
		if(task->delay_ticks)
		{
			t_time_task_wake_up(task);
		}
		
		//最后再将任务加入到调度列表中去
		t_task_sched_ready(task);

	t_task_exit_critical(status);
}







/**********************************************************************************************************
** Function name        :   tEventRemoveAll
** Descriptions         :   清除所有等待中的任务，将事件发送给所有任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t t_event_remove_all (t_event * event, void * msg, uint32_t result)
{
	tnode * node = (tnode *)0;
	tTask * task = (tTask *)0;
	uint32_t count = t_event_wait_count(event);
	uint32_t status = t_task_enter_critical();
	
	//依次从等待队列中取出第一个节点并释放，知道取完为止
	while((node = tlist_remove_first(&event->wait_list)) != (tnode *)0)
	{
		task = (tTask *)tNodeParent(node, tTask, link_node);
		
		//将处于任务处于等待的相关字段设置
		task->event_msg = msg;
		task->wait_event_resalt = result;
		task->wait_event = (t_event *)0;
		task->state &= ~TINYOS_TASK_WAIT_MASK;
		
		//如果设置了等待超时，说明此任务也在延时列表中，我们直接将此任务从延时列表中删除
		if(task->delay_ticks)
		{
			t_time_task_wake_up(task);
		}
		
		//最后再将任务加入到调度列表中去
		t_task_sched_ready(task);
		
	}
	
	t_task_exit_critical(status);
	return count;

}


/**********************************************************************************************************
** Function name        :   tEventWaitCount
** Descriptions         :   事件控制块中等待的任务数量
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t t_event_wait_count (t_event * event)
{
	uint32_t status = t_task_enter_critical();
	
	uint32_t count = list_count(&event->wait_list);
	
	t_task_exit_critical(status);
	return count;
}






