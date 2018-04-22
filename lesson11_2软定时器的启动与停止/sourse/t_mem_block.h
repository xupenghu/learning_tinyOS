#ifndef T_MEM_BLOCK__H
#define T_MEM_BLOCK__H

#include "t_event.h"
#include "tlib.h"

typedef struct _t_mem_block
{
	t_event event;		//存储事件控制块
	tlist block_list;	//存储块列表
	
	uint32_t block_size;	//存储块大小
	uint32_t max_count;		//最多允许的存储块数量
	
	void * mem_start;			//存储块起始地址

}t_mem_block;


typedef struct _t_mem_block_info
{
	uint32_t block_size;		//存储块的大小	
	uint32_t block_count;		//当前存储块的数量
	uint32_t max_count;			//最多允许的存储块数量
	uint32_t task_count;		//等待任务的计数


}t_mem_block_info;




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


/**********************************************************************************************************
** Function name        :   tMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   memBlock 等待的存储块
** parameters						:   mem 存储块存储的地址
** parameters           :   waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_mem_block_wait (t_mem_block * mem_block, uint8_t ** mem, uint32_t wait_ticks);

/**********************************************************************************************************
** Function name        :   tMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   memBlock 等待的存储块
** parameters			:   mem 存储块存储的地址
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_mem_block_no_wait_get (t_mem_block * mem_block, void ** mem);
/**********************************************************************************************************
** Function name        :   tMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   memBlock 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void t_mem_block_notify (t_mem_block * mem_block, uint8_t * mem);




/**********************************************************************************************************
** Function name        :   tMemBlockGetInfo
** Descriptions         :   查询存储控制块的状态信息
** parameters           :   memBlock 存储控制块
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void t_mem_block_get_info (t_mem_block * mem_block, t_mem_block_info * info);


/**********************************************************************************************************
** Function name        :   tMemBlockDestroy
** Descriptions         :   销毁存储控制块
** parameters           :   memBlock 需要销毁的存储控制块
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t t_mem_block_destroy (t_mem_block * mem_block);



#endif

