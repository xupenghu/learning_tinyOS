#ifndef T_MBOX__H
#define T_MBOX_H

#include "t_event.h"

typedef struct _t_mbox
{	
	// 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
	t_event * event;
	
	uint32_t count;		//当前消息数量
	uint32_t max_count;	//最大消息数量
	uint32_t read;			//读索引
	uint32_t write;			//写索引
	void ** msgbuffer;  //邮箱消息缓冲区

}t_mbox;

//邮箱初始化
void t_mbox_init(t_mbox * mbox, uint32_t max_count, void ** msgbuffer);

#endif



