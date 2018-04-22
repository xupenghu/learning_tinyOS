#ifndef T_SEM__H
#define T_SEM__H
#include "t_event.h"

typedef struct _t_sem
{
	//事件控制块
	t_event event;
	//计数值
	uint32_t count;
	//最大计数值
	uint32_t max_count;

}t_sem;

//信号量初始化
void t_sem_init(t_sem * sem, uint32_t start_count, uint32_t max_count);


/**********************************************************************************************************
** Function name        :   tSemWait
** Descriptions         :   等待信号量
** parameters           :   sem 等待的信号量
** parameters           :   waitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_sem_wait (t_sem * sem, uint32_t waitTicks);

/**********************************************************************************************************
** Function name        :   tSemNoWaitGet
** Descriptions         :   获取信号量，如果信号量计数不可用，则立即退回
** parameters           :   sem 等待的信号量
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_sem_no_wait_get (t_sem * sem);

/**********************************************************************************************************
** Function name        :   tSemNotify
** Descriptions         :   通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
** parameters           :   sem 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void t_sem_notify (t_sem * sem);








#endif


