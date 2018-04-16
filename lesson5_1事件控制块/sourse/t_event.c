#include "t_event.h"

/**********************************************************************************************************
** Function name        :   tEventInit
** Descriptions         :   初始化事件控制块
** parameters           :   event 事件控制块
** parameters           :   type 事件控制块的类型
** Returned value       :   无
***********************************************************************************************************/
void t_event_init(t_event * event, t_event_type type)
{
	event->type	=	type;
	list_init(&event->wait_list);
	
}





