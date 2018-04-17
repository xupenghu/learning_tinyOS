#include "tinyOS.h"



tTask tTask1;
tTask tTask2;
tTask	tTask3;
tTask tTask4;

tTastStack task1Env[1024];
tTastStack task2Env[1024];
tTastStack task3Env[1024];
tTastStack task4Env[1024];

//定义两个等待队列
t_event event_wait_timeout;
t_event event_wait_normal;


int task1flag;

void task1_clean_func(void * param)
{
	task1flag = 0;
}

void task1Entry (void *param)
{

	tSetSysTickPeriod(10);

	t_event_init(&event_wait_timeout, t_event_type_unknown);    //初始化
	
	for(;;)
	{
		t_event_wait(&event_wait_timeout, currentTask, 0, 0, 5);
		t_task_sched();
		
		task1flag = 0;
		tTaskDelay(1);
		task1flag = 1;
		tTaskDelay(1);
	}

}

// 

int task2flag;
void task2Entry (void *param)
{

	for(;;)
	{
		t_event_wait(&event_wait_normal, currentTask, 0, 0 , 0);
		t_task_sched();
		
		task2flag = 0;
		tTaskDelay(1);
		task2flag = 1;
		tTaskDelay(1);

	}
}

int task3flag;
void task3Entry (void *param)
{
	t_event_init(&event_wait_normal, t_event_type_unknown);
	for(;;)
	{
		t_event_wait(&event_wait_normal, currentTask, 0, 0 , 0);
		t_task_sched();
		
		task3flag = 0;
		tTaskDelay(1);
		task3flag = 1;
		tTaskDelay(1);

	}
}

int task4flag;
void task4Entry (void *param)
{

	for(;;)
	{
		tTask * task = t_event_wake_up(&event_wait_normal, 0, 0);
		t_task_sched();
		
		task4flag = 0;
		tTaskDelay(1);
		task4flag = 1;
		tTaskDelay(1);

	}
}

void app_init()
{
	// 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
	tTaskInit(&tTask1, task1Entry, (void* )0x11111111, 0, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
	
	tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 0, &task3Env[1024]);
	tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, &task4Env[1024]);

}




