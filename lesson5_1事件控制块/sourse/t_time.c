#include "tinyOS.h"


void tTaskDelay (uint32_t delay)
{
	 uint32_t status = t_task_enter_critical();
	//当前任务的软件定时器置相应的延时节拍
	t_time_task_wait(currentTask, delay);
	t_task_sched_unready(currentTask);
	
	t_task_exit_critical(status);
	
	t_task_sched();
}



