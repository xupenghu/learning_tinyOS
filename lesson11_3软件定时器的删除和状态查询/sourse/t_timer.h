#ifndef T_TIMER__H
#define T_TIMER__H

#include "tlib.h"

// 软硬定时器
#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)


typedef enum _t_timer_state
{
	t_timer_create,				//定时器已经创建
	t_timer_started,			//定时器已经启动
	t_timer_runing,				//定时器正在执行回调函数
	t_timer_stoped,				//定时器已经停止
	t_timer_destroyed,		//定时器已经销毁
	
}t_timer_state;


typedef struct _t_timer
{
	//链表节点
	tnode link_node;
	//定时器的状态
	uint32_t state;
	//定时器配置参数
	uint32_t config;
	//当前定时计数值
	uint32_t delay_ticks;
	//开始定时值
	uint32_t start_delay_ticks;
	//周期定时值
	uint32_t duration_ticks;
	//定时时间到回调函数
	void (*cb_timer_func)(void * arg);
	
	void *arg;

}t_timer;



typedef struct _t_timer_info
{

	//定时器的状态
	uint32_t state;
	//定时器配置参数
	uint32_t config;

	//开始定时值
	uint32_t start_delay_ticks;
	//周期定时值
	uint32_t duration_ticks;
	//定时时间到回调函数
	void (*cb_timer_func)(void * arg);
	
	void *arg;

}t_timer_info;








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
                 void (*timer_func) (void * arg), void * arg, uint32_t config);


/**********************************************************************************************************
** Function name        :   tTimerStart
** Descriptions         :   启动定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void t_timer_start(t_timer * timer);
/**********************************************************************************************************
** Function name        :   tTimerStop
** Descriptions         :   终止定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void t_timer_stop(t_timer * timer);
/**********************************************************************************************************
** Function name        :   tTimerCallFuncList
** Descriptions         :   遍历指定的定时器列表，调用各个定时器处理函数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void t_timer_call_func_list (tlist * timer_list);


/**********************************************************************************************************
** Function name        :   tTimerSoftTask
** Descriptions         :   处理软定时器列表的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void t_timer_soft_task(void * param);
/**********************************************************************************************************
** Function name        :   tTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void t_timer_module_tick_notify (void);
/**********************************************************************************************************
** Function name        :   tTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void timer_module_init(void);





/**********************************************************************************************************
** Function name        :   tTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   timer 查询的定时器
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void t_timer_get_info (t_timer * timer, t_timer_info * info);

/**********************************************************************************************************
** Function name        :   tTimerDestroy
** Descriptions         :   销毁定时器
** parameters           :   timer 销毁的定时器
** Returned value       :   无
***********************************************************************************************************/
void t_timer_destroy (t_timer * timer);









#endif



