#include "t_mbox.h"

//邮箱初始化
void t_mbox_init(t_mbox * mbox, uint32_t max_count, void ** msgbuffer)
{
	t_event_init(mbox->event, t_event_type_mbox);
	
	mbox->count = 0;
	mbox->write = 0;
	mbox->read = 0;
	mbox->msgbuffer = msgbuffer;
	mbox->max_count = max_count;

}








