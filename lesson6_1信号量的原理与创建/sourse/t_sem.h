#ifndef T_SEM__H
#define T_SEM__H
#include "t_event.h"

typedef struct _t_sem
{
	//事件控制块
	t_event event;
	//计数值
	uint32_t count;
	//最大计数值
	uint32_t max_count;

}t_sem;

//信号量初始化
void t_sem_init(t_sem * sem, uint32_t start_count, uint32_t max_count);

#endif


