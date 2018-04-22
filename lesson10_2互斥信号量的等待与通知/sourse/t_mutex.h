#ifndef T_MUTEX__H
#define T_MUTEX__H

#include "t_event.h"

typedef struct _t_mutex
{
	//任务事件控制块
	t_event event;
	//被锁定的次数
	uint32_t lock_count;
	//互斥锁的任务拥有者
	tTask * owner;
	//拥有者的原始优先级
	uint32_t owner_origin_prio;
	
	
}t_mutex;

void t_mutex_init(t_mutex * mutex);

/**********************************************************************************************************
** Function name        :   tMutexWait
** Descriptions         :   等待信号量
** parameters           :   mutex 等待的信号量
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_mutex_wait (t_mutex * mutex, uint32_t wait_ticks);


/**********************************************************************************************************
** Function name        :   tMutexNoWaitGet
** Descriptions         :   获取信号量，如果已经被锁定，立即返回
** parameters           :   tMutex 获取的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_mutex_no_wait_get (t_mutex * mutex);
/**********************************************************************************************************
** Function name        :   tMutexNotify
** Descriptions         :   通知互斥信号量可用
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t t_mutex_notify (t_mutex * mutex);











#endif

