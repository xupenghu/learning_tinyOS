#include "tinyOS.h"

#include <string.h>

tTask tTask1;
tTask tTask2;
tTask	tTask3;
tTask tTask4;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];

int task1flag;

void task1Entry (void *param)
{

	for(;;)
	{

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
		
		task2flag = 0;
		tTaskDelay(1);
		task2flag = 1;
		tTaskDelay(1);

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

int task4flag;

void task4Entry (void *param)
{
	float cpu_usage = 0;
	for(;;)
	{	
		task4flag = 0;
		tTaskDelay(1);
		task4flag = 1;
		tTaskDelay(1);

		cpu_usage = t_cpu_usage_get();
		
	}

}

void app_init()
{
	// 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
	t_task_init(&tTask1, task1Entry, (void* )0x11111111, 0, task1Env, sizeof(task1Env));
	t_task_init(&tTask2, task2Entry, (void *)0x22222222, 0, task2Env, sizeof(task2Env));
	
	t_task_init(&tTask3, task3Entry, (void *)0x33333333, 1, task3Env, sizeof(task3Env));
	t_task_init(&tTask4, task4Entry, (void *)0x44444444, 1, task4Env, sizeof(task4Env));

}




