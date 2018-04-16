#ifndef T_EVENT__H
#define T_EVENT__H

#include "tlib.h"


typedef enum _t_event_type
{
	tEventTypeUnknown   = 0, 				// 未知类型
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

#endif

