#include "tinyOS.h"

void t_flag_group_init(t_flag_group * flag_group, uint32_t flag)
{
	
	t_event_init(&flag_group->event, t_event_type_flag_group);
	flag_group->flags = flag;
	
	
}

/**********************************************************************************************************
** Function name        :   tFlagGroupCheckAndConsume
** Descriptions         :   辅助函数。检查并消耗掉事件标志
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   type 事件标志检查类型
** parameters           :   flags 待检查事件标志存储地址和检查结果存储位置
** Returned value       :   tErrorNoError 事件匹配；tErrorResourceUnavaliable 事件未匹配
***********************************************************************************************************/
static uint32_t t_flag_group_check_and_consume (t_flag_group * flag_group, uint32_t type, uint32_t * flags)
{
	uint32_t src_flags = * flags;
	uint32_t is_set = type & TFLAGGROUP_SET;
	uint32_t is_all = type & TFLAGGROUP_ALL;
	uint32_t is_consume = type & TFLAGGROUP_CONSUME;
	
	// 有哪些类型的标志位出现
	// flagGroup->flags & flags：计算出哪些位为1
	// ~flagGroup->flags & flags:计算出哪位为0
	uint32_t calculate_flag = is_set ? (flag_group->flags & src_flags):(~flag_group->flags & src_flags);
	
	if((is_all != 0 && calculate_flag == src_flags)||(is_all == 0 && calculate_flag != src_flags))
	{
		if(is_consume)
		{
			flag_group->flags &= ~src_flags;
		}
		else
		{
			flag_group->flags |= src_flags;
		}
		
		*flags = calculate_flag;
		return t_error_no_error;
		
	}
	*flags = calculate_flag;
	return t_error_resource_unavaliable;
	
}

/**********************************************************************************************************
** Function name        :   tFlagGroupWait
** Descriptions         :   等待事件标志组中特定的标志
** parameters           :   flagGroup 等待的事件标志组
** parameters           :   waitType 等待的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** parameters           :   waitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_flag_group_wait (t_flag_group * flag_group, uint32_t wait_type, uint32_t request_flag,
						uint32_t * result_flag, uint32_t wait_ticks)
{
	uint32_t resault;
	uint32_t flags = request_flag;
	
	
	uint32_t status = t_task_enter_critical();
	
	resault = t_flag_group_check_and_consume(flag_group, wait_type, &flags);
	
	if(resault != t_error_no_error)
	{
		//如果事件标志不满足条件 就插入到等待队列中去
		currentTask->wait_flags_type = wait_type;
		currentTask->event_flags = request_flag;
		
		t_event_wait(&flag_group->event, currentTask, 0, t_event_type_flag_group, wait_ticks );
		
		t_task_exit_critical(status);
		
		t_task_sched();
		
		*result_flag = currentTask->event_flags;
		resault = currentTask->wait_event_resalt;
		
		
	}
	else
	{
		
		*result_flag = flags;
		t_task_exit_critical(status);
	}
	
	return resault;
}


/**********************************************************************************************************
** Function name        :   tFlagGroupNoWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   flagGroup 获取的事件标志组
** parameters           :   waitType 获取的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** Returned value       :   获取结果,tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_flag_group_no_wait_get (t_flag_group * flag_group, uint32_t wait_type, uint32_t request_flag, uint32_t * result_flag)
{
	uint32_t result ;
	uint32_t flags = request_flag;
	uint32_t status = t_task_enter_critical();
	
	result = t_flag_group_check_and_consume(flag_group, wait_type, &flags);
	
	*result_flag = flags;
	
	t_task_exit_critical(status);
	
	return result;
}



/**********************************************************************************************************
** Function name        :   tFlagGroupNotify
** Descriptions         :   通知事件标志组中的任务有新的标志发生
** parameters           :   flagGroup 事件标志组
** parameters           :   isSet 是否是设置事件标志
** parameters           :   flags 产生的事件标志
***********************************************************************************************************/
void t_flag_group_notify (t_flag_group * flag_group, uint8_t is_set, uint32_t flags)
{
	tlist * wait_list;
	tnode * node;
	tnode * next_node;
	
	uint32_t sched = 0;
	
	uint32_t status = t_task_enter_critical();
	
	//首先将事件标志组中的flag设置
	if(is_set)
	{
		flag_group->flags |= flags;
	}
	else
	{
		flag_group->flags &= ~flags;
	}
	
	wait_list = &(flag_group->event.wait_list);
	//遍历事件标志组
	for(node = wait_list->head_node.nextnode; node != &(wait_list->head_node); node = next_node)
	{
		uint32_t result;
		
		tTask * task = tNodeParent(node, tTask, link_node);
		
		uint32_t flags = task->event_flags;
		
		next_node = node->nextnode;
		
		result = t_flag_group_check_and_consume(flag_group, task->wait_flags_type, &flags);
		//如果有匹配的 就唤醒该任务
		if(result == t_error_no_error)
		{
			task->event_flags = flags;
			t_event_wake_up_task( &flag_group->event, task, (void *)0, t_error_no_error);
			sched = 1;

		}

	
	}
		// 如果有任务就绪，则执行一次调度
		if(sched)
		{
			t_task_sched();
		}
	
	t_task_exit_critical(status);


}

/**********************************************************************************************************
** Function name        :   tFlagGroupGetInfo
** Descriptions         :   查询事件标志组的状态信息
** parameters           :   flagGroup 事件标志组
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void t_flag_group_get_info (t_flag_group * flag_group, t_flag_group_info * info)
{
	uint32_t status = t_task_enter_critical();
	
	info->flags = flag_group->flags;
	info->task_count = t_event_wait_count(&flag_group->event);
	
	t_task_exit_critical(status);

}


/**********************************************************************************************************
** Function name        :   tFlagGroupDestroy
** Descriptions         :   销毁事件标志组
** parameters           :   flagGroup 事件标志组
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t t_flag_group_destroy (t_flag_group * flag_group)
{
	uint32_t status = t_task_enter_critical();
	
	//清楚等待队列中的所有任务
	uint32_t count = t_event_remove_all(&flag_group->event, 0 , t_error_del);
	
	
	t_task_exit_critical(status);
	//如果有任务 就执行一次调度函数
	if(count > 0)
	{
		t_task_sched();
	}
	
	return count;
}





