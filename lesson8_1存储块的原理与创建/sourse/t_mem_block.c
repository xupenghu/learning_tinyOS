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
	mem_block->block_count = block_count;
	mem_block->block_size = block_size;
	mem_block->mem_start = mem_start;
	
	list_init(&mem_block->block_list);
	
	//将内存块插入到存储控制块中
	while(mem_block_start < mem_block_end)
	{
		tnode_init((tnode *)mem_block_start);
		tlist_add_last(&mem_block->block_list, (tnode *)mem_block_start);
		
		mem_block_start += block_size;
	}

}





