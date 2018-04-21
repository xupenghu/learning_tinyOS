#ifndef T_FLAG_GROUP__H
#define T_FLAG_GROUP__H

#include "t_event.h"

typedef struct _t_flag_group
{
	//定义一个事件控制块
	t_event event;
	
	//当前事件标志组 最多可以有32个标志 如果想要更多 可以使用位图来实现
	uint32_t flags;
	
}t_flag_group;




#endif






