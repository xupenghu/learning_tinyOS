#include "tinyOS.h"



tTask tTask1;
tTask tTask2;
tTask	tTask3;

tTastStack task1Env[1024];
tTastStack task2Env[1024];
tTastStack task3Env[1024];

int task1flag;
void task1Entry (void *param)
{

	tSetSysTickPeriod(10);
	for(;;)
	{
		task1flag = 0;
		t_task_suspend(currentTask);
		task1flag = 1;
		t_task_suspend(currentTask);
	}

}

// 

int task2flag;
void task2Entry (void *param)
{
	for(;;)
	{
		task2flag = 0;
		tTaskDelay(1);
		t_suspend_task_wake_up(&tTask1);
		task2flag = 1;
		tTaskDelay(1);
		t_suspend_task_wake_up(&tTask1);
	}
}

int task3flag;
void task3Entry (void *param)
{
	for(;;)
	{
		task3flag = 0;
		tTaskDelay(1);
		task3flag = 1;
		tTaskDelay(1);
	}
}

void app_init()
{
	// 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
	tTaskInit(&tTask1, task1Entry, (void* )0x11111111, 0, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
	
	tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 1, &task3Env[1024]);

}




