#include "tinyOS.h"

void t_flag_group_init(t_flag_group * flag_group, uint32_t flag)
{
	
	t_event_init(&flag_group->event, t_event_type_flag_group);
	flag_group->flags = flag;
	
	
}






