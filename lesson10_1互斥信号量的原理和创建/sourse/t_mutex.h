#ifndef T_MUTEX__H
#define T_MUTEX__H

#include "t_event.h"

typedef struct _t_mutex
{
	//任务事件控制块
	t_event event;
	//被锁定的次数
	uint32_t lock_count;
	//互斥锁的任务拥有者
	tTask * ower;
	//拥有者的原始优先级
	uint32_t ower_origin_prio;
	
	
}t_mutex;

void t_mutex_init(t_mutex * mutex);

#endif

