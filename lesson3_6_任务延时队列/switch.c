#include "tinyOS.h"
#include <ARMCM3.h>




#define NVIC_INT_CTRL			0xE000ED04
#define NVIC_PENDSVSET		0x10000000
#define NVIC_SYSPRI2			0xE000ED22
#define NVIC_PENDSV_PRI		0x000000FF

#define MEM32(addr)		*(volatile unsigned long *)(addr)
#define MEM8(addr)		*(volatile unsigned char *)(addr)


void tTaskRunFirst()
{
	__set_PSP(0);  													//用于区别是第一个任务跑起来了，还是在进行任务切换
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;		//设置为最低优先级
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;	//触发PendSV中断
}

void tTaskSwitch()
{

	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;	//触发PendSV中断
}




__asm void PendSV_Handler(void)
{
	IMPORT currentTask
	IMPORT nextTask
	
	MRS R0, PSP			//获取当前任务的堆栈指针
	CBZ	R0, PendSV_Handler_nosave
	
	STMDB	R0!, {R4-R11} //保存除异常中自动保存的寄存器外其他的寄存器值自动保存起来{R4-R11}
	
	LDR R1, =currentTask
	LDR R1, [R1]
	STR R0, [R1]		//保存好后，将最后的堆栈栈顶位置，保存到currentTask->stack处，相当于完成了一次当前任务的压栈操作
	
PendSV_Handler_nosave
	
	LDR		R0, =currentTask
	LDR 	R1, =nextTask
	LDR		R2, [R1]
	STR 	R2, [R0]				//先将currentTask切换成nextTask 也就是下一任务变成了当前任务
	
	LDR		R0, [R2]
	LDMIA	R0!, {R4-R11}
		
	MSR	PSP,	R0
	ORR	LR,	LR, #0x04
	BX	LR
	
}

/**********************************************************************************************************
** Function name        :   tTaskEnterCritical
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入之前的临界区状态值
***********************************************************************************************************/
uint32_t tTaskEnterCritical (void) 
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();        // CPSID I
    return primask;
}

/**********************************************************************************************************
** Function name        :   tTaskExitCritical
** Descriptions         :   退出临界区,恢复之前的临界区状态
** parameters           :   status 进入临界区之前的CPU
** Returned value       :   进入临界区之前的临界区状态值
***********************************************************************************************************/
void tTaskExitCritical (uint32_t status) 
{
    __set_PRIMASK(status);
}





