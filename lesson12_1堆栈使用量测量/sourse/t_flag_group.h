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


typedef struct _t_flag_group_info
{
	//当前事件标志组的信息
	uint32_t flags;
	//等待任务数量
	uint32_t task_count;

}t_flag_group_info;




#define	TFLAGGROUP_CLEAR		(0x0 << 0)
#define	TFLAGGROUP_SET			(0x1 << 0)
#define	TFLAGGROUP_ANY			(0x0 << 1)
#define	TFLAGGROUP_ALL			(0x1 << 1)

#define TFLAGGROUP_SET_ALL		(TFLAGGROUP_SET | TFLAGGROUP_ALL)
#define	TFLAGGROUP_SET_ANY		(TFLAGGROUP_SET | TFLAGGROUP_ANY)
#define TFLAGGROUP_CLEAR_ALL	(TFLAGGROUP_CLEAR | TFLAGGROUP_ALL)
#define TFLAGGROUP_CLEAR_ANY	(TFLAGGROUP_CLEAR | TFLAGGROUP_ANY)

#define	TFLAGGROUP_CONSUME		(0x1 << 7)



void t_flag_group_init(t_flag_group * flag_group, uint32_t flag);

/**********************************************************************************************************
** Function name        :   tFlagGroupCheckAndConsume
** Descriptions         :   辅助函数。检查并消耗掉事件标志
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   type 事件标志检查类型
** parameters           :   flags 待检查事件标志存储地址和检查结果存储位置
** Returned value       :   tErrorNoError 事件匹配；tErrorResourceUnavaliable 事件未匹配
***********************************************************************************************************/
static uint32_t t_flag_group_check_and_consume (t_flag_group * flag_group, uint32_t type, uint32_t * flags);

/**********************************************************************************************************
** Function name        :   tFlagGroupWait
** Descriptions         :   等待事件标志组中特定的标志
** parameters           :   flagGroup 等待的事件标志组
** parameters           :   waitType 等待的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** parameters           :   waitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t t_flag_group_wait (t_flag_group * flag_group, uint32_t wait_type, uint32_t request_flag,
						uint32_t * result_flag, uint32_t wait_ticks);
/**********************************************************************************************************
** Function name        :   tFlagGroupNoWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   flagGroup 获取的事件标志组
** parameters           :   waitType 获取的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** Returned value       :   获取结果,tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t t_flag_group_no_wait_get (t_flag_group * flag_group, uint32_t wait_type, uint32_t request_flag, uint32_t * result_flag);


/**********************************************************************************************************
** Function name        :   tFlagGroupNotify
** Descriptions         :   通知事件标志组中的任务有新的标志发生
** parameters           :   flagGroup 事件标志组
** parameters           :   isSet 是否是设置事件标志
** parameters           :   flags 产生的事件标志
***********************************************************************************************************/
void t_flag_group_notify (t_flag_group * flag_group, uint8_t is_set, uint32_t flags);




/**********************************************************************************************************
** Function name        :   tFlagGroupGetInfo
** Descriptions         :   查询事件标志组的状态信息
** parameters           :   flagGroup 事件标志组
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void t_flag_group_get_info (t_flag_group * flag_group, t_flag_group_info * info);

/**********************************************************************************************************
** Function name        :   tFlagGroupDestroy
** Descriptions         :   销毁事件标志组
** parameters           :   flagGroup 事件标志组
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t t_flag_group_destroy (t_flag_group * flag_group);





#endif






