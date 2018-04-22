#include "tinyOS.h"

void t_mutex_init(t_mutex * mutex)
{
	t_event_init(&mutex->event, t_event_type_mutex);
	mutex->lock_count = 0;
	mutex->ower = (tTask *)0;
	
	mutex->ower_origin_prio = TINYOS_PRIO_COUNT;
	
}

