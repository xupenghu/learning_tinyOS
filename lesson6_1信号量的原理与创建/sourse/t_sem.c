#include "t_sem.h"

//信号量初始化
void t_sem_init(t_sem * sem, uint32_t start_count, uint32_t max_count)
{
	t_event_init(&sem->event, t_event_type_sem);
	
	sem->max_count = max_count;
	
	if(max_count == 0)
	{
		sem->count = start_count;
	}
	else
	{
		sem->count = (start_count > max_count) ? max_count : start_count;
	}

}





