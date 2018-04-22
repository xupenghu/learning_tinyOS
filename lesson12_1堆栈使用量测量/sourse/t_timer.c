#include "config.h"
#include "tinyOS.h"


static tlist t_timer_hard_list;		//硬定时器列表
static tlist t_timer_soft_list;		//软定时器列表

static t_sem t_timer_protect_sem;	//定时器访问保护计数信号量
static t_sem t_timer_ticks_sem;		//软定时器和systicks同步信号量




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

/**********************************************************************************************************
** Function name        :   tTimerStart
** Descriptions         :   启动定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void t_timer_start(t_timer * timer)
{
	switch(timer->state)
	{
		case t_timer_create:
		case t_timer_stoped:
			timer->delay_ticks = timer->start_delay_ticks ? timer->start_delay_ticks : timer->duration_ticks;
			timer->state = t_timer_started;
			
			// 根据定时器类型加入相应的定时器列表
			if (timer->config & TIMER_CONFIG_TYPE_HARD)
			{
				uint32_t status = t_task_enter_critical();
				
				tlist_add_last(&t_timer_hard_list, &timer->link_node);
				
				t_task_exit_critical(status);
				
			}
			else
			{
				t_sem_wait(&t_timer_protect_sem, 0);
				tlist_add_last(&t_timer_soft_list, &timer->link_node);
				t_sem_notify(&t_timer_protect_sem);
			}

			break;
		default:
			break;
	}
}
/**********************************************************************************************************
** Function name        :   tTimerStop
** Descriptions         :   终止定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void t_timer_stop(t_timer * timer)
{
	switch(timer->state)
	{
		case t_timer_started:
		case t_timer_runing:
		//判断定时器是那种方式		
		if(timer->config & TIMER_CONFIG_TYPE_HARD)
		{
			//如果是硬件定时器，进入临界区然后移除该定时器
			uint32_t status = t_task_enter_critical();
			
			tlist_remove(&t_timer_hard_list, &timer->link_node);
			
			t_task_exit_critical(status);
		}
		else
		{
			//否则就是软件定时器  
			t_sem_wait(&t_timer_protect_sem, 0);
			tlist_remove(&t_timer_soft_list, &timer->link_node);
			t_sem_notify(&t_timer_protect_sem);
		}
		
			break;
		default:
			break;
	}
}

/**********************************************************************************************************
** Function name        :   tTimerCallFuncList
** Descriptions         :   遍历指定的定时器列表，调用各个定时器处理函数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void t_timer_call_func_list (tlist * timer_list)
{
	tnode * node;
	//检查链表中的所有定时器，如果定时时间到 则执行定时器回调函数
	for(node = timer_list->head_node.nextnode; node != &(timer_list->head_node) ; node = node->nextnode)
	{
		t_timer * timer = tNodeParent(node , t_timer, link_node);
		//如果定时时间到 则执行定时器回调函数
		if((timer->delay_ticks == 0 ) || (--timer->delay_ticks == 0))
		{
			timer->state = t_timer_runing;
			
			//执行定时器回调函数
			timer->cb_timer_func(timer->arg);
			
			timer->state = t_timer_started;
			
			//如果是周期性定时，则重新赋值
			if(timer->duration_ticks > 0)
			{
				timer->delay_ticks = timer->duration_ticks;
			}
			else
			{
				//否则是一次性定时，移除该定时器
				tlist_remove(timer_list, &timer->link_node);
				
				timer->state = t_timer_stoped;
			}
			
		}
		
	}
}


static tTask timer_task;

static tTaskStack tiemr_task_stack[TINYOS_TIMERTASK_STATCK_SIZE];

/**********************************************************************************************************
** Function name        :   tTimerSoftTask
** Descriptions         :   处理软定时器列表的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void t_timer_soft_task(void * param)
{
	
	for(;;)
	{
		//等待系统定时任务的信号量通知
		t_sem_wait(&t_timer_ticks_sem, 0);
		//获取保护信号量
		t_sem_wait(&t_timer_protect_sem, 0);
		//处理软件定时器列表
		t_timer_call_func_list(&t_timer_soft_list);
		//释放信号量保护
		t_sem_notify(&t_timer_protect_sem);
		
		
	}
}


/**********************************************************************************************************
** Function name        :   tTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void t_timer_module_tick_notify (void)
{
	uint32_t status = t_task_enter_critical();
	//处理硬件定时器列表
	t_timer_call_func_list(&t_timer_hard_list);

	t_task_exit_critical(status);
	//通知信号量 用于软件定时器和systick同步
	t_sem_notify(&t_timer_ticks_sem);
	
}

/**********************************************************************************************************
** Function name        :   tTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void timer_module_init()
{
	//初始化列表和信号量
	list_init(&t_timer_hard_list);
	list_init(&t_timer_soft_list);
	
	t_sem_init(&t_timer_protect_sem, 1, 1);
	t_sem_init(&t_timer_ticks_sem, 0, 0);		// 不限数量

#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRIO_COUNT - 1)
    #error "The proprity of timer task must be greater then (TINYOS_PRO_COUNT - 1)"
#endif	
	
	
	t_task_init(&timer_task, t_timer_soft_task, (void *)0, TINYOS_TIMERTASK_PRIO, tiemr_task_stack, sizeof(tiemr_task_stack));

}

/**********************************************************************************************************
** Function name        :   tTimerGetInfo
** Descriptions         :   查询状态信息
** parameters           :   timer 查询的定时器
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void t_timer_get_info (t_timer * timer, t_timer_info * info)
{
	uint32_t status = t_task_enter_critical();
	
	info->arg = timer->arg;
	info->cb_timer_func = timer->cb_timer_func;
	info->config = timer->config;
	info->start_delay_ticks = timer->start_delay_ticks;
	info->duration_ticks = timer->duration_ticks;
	info->state = timer->state;
	
	
	t_task_exit_critical(status);
	
}

/**********************************************************************************************************
** Function name        :   tTimerDestroy
** Descriptions         :   销毁定时器
** parameters           :   timer 销毁的定时器
** Returned value       :   无
***********************************************************************************************************/
void t_timer_destroy (t_timer * timer)
{
	t_timer_stop(timer);
	timer->state = t_timer_destroyed;
	
}

