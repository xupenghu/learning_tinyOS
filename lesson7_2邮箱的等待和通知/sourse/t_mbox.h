#ifndef T_MBOX__H
#define T_MBOX_H

#include "t_event.h"

typedef struct _t_mbox
{	
	// 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
	t_event event;
	
	uint32_t count;		//当前消息数量
	uint32_t max_count;	//最大消息数量
	uint32_t read;			//读索引
	uint32_t write;			//写索引
	void ** msgbuffer;  //邮箱消息缓冲区

}t_mbox;

//邮箱初始化
void t_mbox_init(t_mbox * mbox, uint32_t max_count, void ** msgbuffer);

/**********************************************************************************************************
** Function name        :   tMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   mbox 等待的邮箱
** parameters           :   msg 消息存储缓存区
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_mbox_wait (t_mbox * mbox, void **msg, uint32_t wait_ticks) ;

/**********************************************************************************************************
** Function name        :   tMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   mbox 获取消息的邮箱
** parameters           :   msg 消息存储缓存区
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_mbox_no_wait_get (t_mbox * mbox, void **msg);

/**********************************************************************************************************
** Function name        :   tMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t t_mbox_notify (t_mbox * mbox, void * msg, uint32_t notify_option);


#endif



