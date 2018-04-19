#ifndef T_MEM_BLOCK__H
#define T_MEM_BLOCK__H

#include "t_event.h"
#include "tlib.h"

typedef struct _t_mem_block
{
	t_event event;		//存储事件控制块
	tlist block_list;	//存储块列表
	
	uint32_t block_size;	//存储块大小
	uint32_t block_count; //存储块数量
	void * mem_start;			//存储块起始地址

}t_mem_block;

/**********************************************************************************************************
** Function name        :   tMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   memBlock 等待初始化的存储控制块
** parameters           :   memStart 存储区的起始地址
** parameters           :   blockSize 每个块的大小
** parameters           :   blockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/

void t_mem_block_init(t_mem_block * mem_block, uint8_t * mem_start, uint32_t blocksize, uint32_t block_count);


#endif

