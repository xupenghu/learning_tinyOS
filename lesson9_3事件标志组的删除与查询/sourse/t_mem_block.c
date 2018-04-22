#include "tinyOS.h"


/**********************************************************************************************************
** Function name        :   tMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   memBlock 等待初始化的存储控制块
** parameters           :   memStart 存储区的起始地址
** parameters           :   blockSize 每个块的大小
** parameters           :   blockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/

void t_mem_block_init(t_mem_block * mem_block, uint8_t * mem_start, uint32_t block_size, uint32_t block_count)
{
	uint8_t * mem_block_start = (uint8_t *)mem_start;
	uint8_t * mem_block_end = (uint8_t *)mem_start + block_size * block_count;
	
	//每个存储块需要用来放置链接指针，所以需求空间应该大于链接指针存储的空间
	if(block_size < sizeof(tnode))
	{
		return;
	}
	//存储事件控制块初始化
	t_event_init(&mem_block->event, t_event_type_mem_block);
	mem_block->max_count = block_count;
	mem_block->block_size = block_size;
	mem_block->mem_start = mem_start;
	
	list_init(&mem_block->block_list);
	
	//将内存块插入到存储控制块中
	while(mem_block_start < mem_block_end)
	{
		//注意我们这边直接将存储类型强制转换为tnode类型，让存储块的前两个字段存储列表的指针，
		//为什么可以这么做呢？因为存储块中现在的内存空间没有任何数据，我们可以随意操作。
		
		tnode_init((tnode *)mem_block_start);
		tlist_add_last(&mem_block->block_list, (tnode *)mem_block_start);
		
		mem_block_start += block_size;
	}

}

/**********************************************************************************************************
** Function name        :   tMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   memBlock 等待的存储块
** parameters						:   mem 存储块存储的地址
** parameters           :   waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_mem_block_wait (t_mem_block * mem_block, uint8_t ** mem, uint32_t wait_ticks)
{
	uint32_t status = t_task_enter_critical();
	
	//如果存储块中还有没有用的存储块，那么直接获取
	if(list_count(&mem_block->block_list) > 0)
	{
		*mem = (uint8_t *)tlist_remove_first(&mem_block->block_list);
		t_task_exit_critical(status);
		return t_error_no_error;
	}
	else 	//否则将任务加入到等待队列中等待
	{
		t_event_wait(&mem_block->event, currentTask, 0, t_event_type_mem_block, wait_ticks);
		t_task_exit_critical(status);
		
		t_task_sched();
		
		*mem = currentTask->event_msg;
		
		return currentTask->wait_event_resalt;		
	}

}

/**********************************************************************************************************
** Function name        :   tMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   memBlock 等待的存储块
** parameters			:   mem 存储块存储的地址
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_mem_block_no_wait_get (t_mem_block * mem_block, void ** mem)
{
	uint32_t status = t_task_enter_critical();
	
	if(list_count(&mem_block->block_list) > 0)
	{
		*mem = (uint8_t *)tlist_remove_first(&mem_block->block_list);
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
** Function name        :   tMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   memBlock 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void t_mem_block_notify (t_mem_block * mem_block, uint8_t * mem)
{
	uint32_t status = t_task_enter_critical();
	//如果有任务等待，就将等待队列头部的任务释放，然后将mem_block的值赋给此任务
	if(t_event_wait_count(&mem_block->event) > 0)
	{
		tTask * task = t_event_wake_up(&mem_block->event, (void*)mem, t_error_no_error);
		//如果唤醒的任务的优先级大于当前正在运行任务的优先级 就切换到唤醒任务中去执行
		if(task->prio < currentTask->prio)
		{
			t_task_sched();
		}
	}
	else 
	{
		//如果没有任务在等待，就将这个存储块插入到任务控制块的尾部
		tlist_add_last(&mem_block->block_list, (tnode *)mem);
	}
	
	t_task_exit_critical(status);

}

/**********************************************************************************************************
** Function name        :   tMemBlockGetInfo
** Descriptions         :   查询存储控制块的状态信息
** parameters           :   memBlock 存储控制块
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void t_mem_block_get_info (t_mem_block * mem_block, t_mem_block_info * info)
{
	uint32_t status = t_task_enter_critical();
	//复制需要的信息
	info->block_count = list_count(&mem_block->block_list);
	info->max_count = mem_block->max_count;
	info->block_size = mem_block->block_size;
	info->task_count = t_event_wait_count(&mem_block->event);
	
	t_task_exit_critical(status);

}


/**********************************************************************************************************
** Function name        :   tMemBlockDestroy
** Descriptions         :   销毁存储控制块
** parameters           :   memBlock 需要销毁的存储控制块
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t t_mem_block_destroy (t_mem_block * mem_block)
{
	uint32_t status = t_task_enter_critical();
	//删除所有等待的队列 返回等待队列的数量 因为我们使用的是全局变量数组 所以销毁存储控制块的更多目的是为了释放等待存储资源的这些任务
	uint32_t count = t_event_remove_all(&mem_block->event, 0, t_error_del);
	t_task_exit_critical(status);
	
	//如果有任务被释放，就执行一次调度函数
	if(count)
	{
		t_task_sched();
	}
	
	return count;
	
}




