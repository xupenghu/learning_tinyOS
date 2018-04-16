#ifndef __TINYOS__H
#define __TINYOS__H
#include <stdint.h>
#include "tlib.h"

typedef uint32_t tTastStack;

#define TINYOS_TASK_STATE_RDY				0
#define TINYOS_TASK_STATE_DELAYED		(0x1<<1)
#define TINYOS_TASK_STATE_SUSPEND		(0x1<<2)

typedef struct _tTask{
	//记录了当前任务的堆栈指针
	tTastStack *stack;
	
	//任务列表
	tnode link_node;
	//延时时间
	uint32_t delay_ticks;
	
	//延时结点， 通过delay_node就可以将tTask加入到延时队列中来
	tnode delay_node;
	
	//任务的优先级
	uint32_t prio;
	
	//任务的状态标志
	uint32_t state;
	
	//时间片 最小单位也是时间片的整数倍
	uint32_t slice;
	
	uint32_t suspend_count;
	

}tTask;


// 当前任务：记录当前是哪个任务正在运行
extern tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern tTask * nextTask;


void tTaskSwitch(void);
void tTaskRunFirst(void);


//临界区函数
uint32_t t_task_enter_critical (void) ;
void t_task_exit_critical (uint32_t status);


//调度锁部分
extern uint8_t sched_lock_count;
void task_sched_init(void);
void disable_task_sched(void);
void enable_task_sched(void);


//CPU部分
/*********************************************************************************************************
** 系统时钟节拍定时器System Tick配置
** 在我们目前的环境（模拟器）中，系统时钟节拍为12MHz
** 请务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
**********************************************************************************************************/
void tSetSysTickPeriod(uint32_t ms);
void SysTick_Handler (void) ;

//time部分
void tTaskDelay (uint32_t delay);


//app 部分
void app_init(void);




//task部分
void tTaskInit (tTask *task, void (*entry)(void *), void *param, uint32_t prio,  tTastStack * stack);
void t_task_system_tick_handler(void);
//任务挂起
void t_task_suspend(tTask * task);
//挂起任务的恢复
void t_suspend_task_wake_up(tTask * task);






//返回最高优先级的就绪任务
tTask * t_task_highest_ready(void);


/**********************************************************************************************************
** Function name        :   ttask_sched_ready
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void t_task_sched_ready (tTask * task);

/**********************************************************************************************************
** Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task   
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void t_task_sched_unready (tTask * task);

/**********************************************************************************************************
** Function name        :   ttime_task_wait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void t_time_task_wait (tTask * task, uint32_t ticks);

/**********************************************************************************************************
** Function name        :   tTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   task  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void t_time_task_wake_up (tTask * task);


void t_task_sched (void);







#endif
