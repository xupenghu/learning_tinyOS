#ifndef __TINYOS__H
#define __TINYOS__H
#include <stdint.h>


typedef uint32_t tTastStack;

typedef struct _tTask{
	//记录了当前任务的堆栈指针
	tTastStack *stack;
	
	uint32_t delayTicks;

}tTask;


// 当前任务：记录当前是哪个任务正在运行
extern tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern tTask * nextTask;


void tTaskSwitch(void);
void tTaskRunFirst(void);

uint32_t tTaskEnterCritical (void);
void tTaskExitCritical (uint32_t status);


//调度锁部分
extern uint8_t sched_lock_count;
void task_sched_init(void);
void disable_task_sched(void);
void enable_task_sched(void);

#endif
