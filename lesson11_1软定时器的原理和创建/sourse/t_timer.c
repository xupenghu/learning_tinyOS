#include "tinyOS.h"

/**********************************************************************************************************
** Function name        :   tTimerInit
** Descriptions         :   初始化定时器
** parameters           :   timer 等待初始化的定时器
** parameters           :   start_delay_ticks 定时器初始启动的延时ticks数。
** parameters           :   durationTicks 给周期性定时器用的周期tick数，一次性定时器无效
** parameters           :   timerFunc 定时器回调函数
** parameters           :   arg 传递给定时器回调函数的参数
** parameters           :   timerFunc 定时器回调函数
** parameters           :   config 定时器的初始配置
** Returned value       :   无
***********************************************************************************************************/
void t_timer_init (t_timer * timer, uint32_t start_delay_ticks, uint32_t duration_ticks,
                 void (*timer_func) (void * arg), void * arg, uint32_t config)
{
	tnode_init(&timer->link_node);
	timer->start_delay_ticks = start_delay_ticks;
	timer->duration_ticks = duration_ticks;
	timer->cb_timer_func = timer_func;
	timer->arg = arg;
	timer->config = config;
	
	if(start_delay_ticks == 0)
	{
		timer->delay_ticks = duration_ticks;
	}
	else
	{
		timer->delay_ticks = start_delay_ticks;
	}

	timer->state = t_timer_create;
	
}









