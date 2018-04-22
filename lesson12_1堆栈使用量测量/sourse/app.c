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

t_timer timer1;
t_timer timer2;
t_timer timer3;

uint32_t bit1 = 0;
uint32_t bit2 = 0;
uint32_t bit3 = 0;

void cb_timer_func(void *arg)
{
	uint32_t * bit = (uint32_t *)arg;
	
	*bit ^= 0x01;

}

void task1Entry (void *param)
{

	tSetSysTickPeriod(10);

	t_timer_init(&timer1, 100, 10, cb_timer_func, (void *)&bit1, TIMER_CONFIG_TYPE_HARD);
	t_timer_start(&timer1);
	
	t_timer_init(&timer2, 200, 20, cb_timer_func, (void *)&bit2, TIMER_CONFIG_TYPE_HARD);
	t_timer_start(&timer2);
	
	t_timer_init(&timer3, 300, 30, cb_timer_func, (void *)&bit3, TIMER_CONFIG_TYPE_SOFT);
	t_timer_start(&timer3);
	
	
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
	uint32_t stoped = 0;

	for(;;)
	{
		
		task2flag = 0;
		tTaskDelay(1);
		task2flag = 1;
		tTaskDelay(1);
		
		if(stoped == 0)
		{
			tTaskDelay(500);
			t_timer_stop(&timer1);
			stoped = 1;
		}
		

	}
}

int task3flag;
t_task_info task_info3;

void task3Entry (void *param)
{

	
	for(;;)
	{

		task3flag = 0;
		tTaskDelay(1);
		task3flag = 1;
		tTaskDelay(1);

		t_get_task_info(currentTask, &task_info3);

	}
}

int task4flag;
t_task_info task_info4;
void task4Entry (void *param)
{
	for(;;)
	{	
		task4flag = 0;
		tTaskDelay(1);
		task4flag = 1;
		tTaskDelay(1);
		t_get_task_info(currentTask, &task_info4);
		
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




