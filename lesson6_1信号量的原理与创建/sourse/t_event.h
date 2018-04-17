#ifndef T_EVENT__H
#define T_EVENT__H

#include "config.h"
#include "tlib.h"
#include "t_task.h"



typedef enum _t_event_type
{
	t_event_type_unknown   = 0, 				// 未知类型
	t_event_type_sem,
	
}t_event_type;

typedef struct _t_event
{
	t_event_type type;		//事件类型
	
	tlist wait_list;			//等待队列
}t_event;

/**********************************************************************************************************
** Function name        :   tEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   event 事件控制块
** parameters           :   type 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void t_event_init(t_event * event, t_event_type type);


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
void t_event_wait (t_event * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);

/**********************************************************************************************************
** Function name        :   tEventRemoveTask
** Descriptions         :   将任务从其等待队列中强制移除
** parameters           :   task 待移除的任务
** parameters           :   result 告知事件的等待结果 如超时等
** Returned value       :   无
***********************************************************************************************************/
void t_event_remove_task (tTask * task, void * msg, uint32_t result);

/**********************************************************************************************************
** Function name        :   tEventWakeUp
** Descriptions         :   从事件控制块中唤醒首个等待的任务 注意 我们只唤醒了一个任务 当需要唤醒等待队列中的全部任务时，就需要循环调用该函数了
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   首个等待的任务，如果没有任务等待，则返回0
***********************************************************************************************************/

tTask * t_event_wake_up (t_event * event, void * msg, uint32_t result);


/**********************************************************************************************************
** Function name        :   tEventWaitCount
** Descriptions         :   事件控制块中等待的任务数量
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t t_event_wait_count (t_event * event);


/**********************************************************************************************************
** Function name        :   tEventRemoveAll
** Descriptions         :   清除所有等待中的任务，将事件发送给所有任务
** parameters           :   event 事件控制块
** parameters           :   msg 事件消息
** parameters           :   result 告知事件的等待结果
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
uint32_t t_event_remove_all (t_event * event, void * msg, uint32_t result);












#endif

