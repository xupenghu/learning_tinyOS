#ifndef __TINYOS__H
#define __TINYOS__H
#include <stdint.h>


typedef uint32_t tTastStack;

typedef struct _tTask{
	tTastStack *stack;

}tTask;


// 当前任务：记录当前是哪个任务正在运行
extern tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern tTask * nextTask;


void tTaskSwitch(void);
void tTaskRunFirst(void);

#endif
