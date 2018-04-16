#include "tinyOS.h"
#include "config.h"



void tTaskInit (tTask *task, void (*entry)(void *), void *param, uint32_t prio,  tTastStack * stack)
{
	*(--stack) = (unsigned long )(1<<24); //XPSR 	设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
	*(--stack) = (unsigned long )entry;		//程序的入口地址
	*(--stack) = (unsigned long )0x14;		//R14(LR)寄存器 任务不会通过return来结束自己，所以没有使用
	*(--stack) = (unsigned long )0x12;		//R12 没有使用
	*(--stack) = (unsigned long )0x3;		
	*(--stack) = (unsigned long )0x2;
	*(--stack) = (unsigned long )0x1;
	*(--stack) = (unsigned long )param;		//程序传入参数保存在R0寄存器中
	//以上这些操作在PendSV中会自动由硬件完成，不需要手动保存，同样的在推出PendSV中会由硬件自动弹出，所以在保存现场和退出现场时，我们不需要手动来完成这些操作
	
	//R4-R11的值需要我们手动来保存，初始化时没有使用这些寄存器的值，所有就直接初始化成这些寄存器的对应位号，方便调试时来查看对应寄存器的值
	*(--stack) = (unsigned long )0x11;
	*(--stack) = (unsigned long )0x10;
	*(--stack) = (unsigned long )0x09;
	*(--stack) = (unsigned long )0x08;
	*(--stack) = (unsigned long )0x07;
	*(--stack) = (unsigned long )0x06;
	*(--stack) = (unsigned long )0x05;
	*(--stack) = (unsigned long )0x04;

	
	task->stack = stack;   //保存最终的stack值 让该任务的堆栈指针指向栈顶
	

	task->slice = TINYOS_SLICE_MAX;
	
	task->delay_ticks = 0;
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY ;
	task->suspend_count = 0; 				//初始不挂起
	
	tnode_init(&task->delay_node);	//初始化延时结点
	
	tnode_init(&task->link_node);
	
	
	t_task_sched_ready(task);  //最后将任务设为就绪态

}
//任务挂起
void t_task_suspend(tTask * task)
{
	uint32_t status = t_task_enter_critical();
	//如果任务状态不是延时
	if(!(task->state & TINYOS_TASK_STATE_DELAYED))
	{
		//如果是第一次挂起，将挂起标志位置1 然后将任务从任务队列中移除
		if(++task->suspend_count <= 1)
		{
			task->state |= TINYOS_TASK_STATE_SUSPEND;
			
			t_task_sched_unready(task);
			//如果是自己挂起自己，那么切换到其他任务运行
			if(task == currentTask)
			{
				t_task_sched();
			}
			
		}
		
	}
	
	t_task_exit_critical(status);
}

//挂起任务的恢复
void t_suspend_task_wake_up(tTask * task)
{
	uint32_t status = t_task_enter_critical();
	
	//如果任务处于挂起状态
	if(task->state & TINYOS_TASK_STATE_SUSPEND)
	{
		if(--(task->suspend_count) == 0)
		{
			//清除挂起标志
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;
			//将任务加入到就绪队列中
			t_task_sched_ready(task);
			//执行一次任务调度
			t_task_sched();
			
		}
	}
	
	
	t_task_exit_critical(status);
}


