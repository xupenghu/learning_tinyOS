#include "tinyOS.h"


#define NVIC_INT_CTRL			0xE000ED04
#define NVIC_PENDSVSET		0x10000000
#define NVIC_SYSPRI2			0xE000ED22
#define NVIC_PENDSV_PRI		0x000000FF

tTask tTask1;
tTask tTask2;

tTastStack task1Env[1024];
tTastStack task2Env[1024];

void tTaskInit (tTask *task, void (*entry)(void *), void *param, tTastStack * stack)
{
	task->stack = stack;
}


void task1 (void *param)
{
	for(;;)
	{
	
	}

}

void task2 (void *param)
{
	for(;;)
	{
	
	}
}

#define MEM32(addr)		*(volatile unsigned long *)(addr)
#define MEM8(addr)		*(volatile unsigned char *)(addr)

void trigger_PENDSV(void)
{
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI; //设置为最低优先级
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;	//触发PENDSV中断
		
}


typedef struct _BlockType_t
{
	unsigned long * stackPtr;
}BlockType_t;

BlockType_t * blockPtr;

BlockType_t block;

unsigned long stackbuffer[1024];

void delay(int count)
{
	while(count-- > 0);
	
}
	int flag;
int main(void)
{
	tTaskInit(&tTask1, task1, (void*)0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2, task2, (void *)0x22222222, &task2Env[1024]);
	
	block.stackPtr = &stackbuffer[1024];
	blockPtr = &block;

	while(1)
	{
		flag  = 0;
		delay(100);
		flag = 1 ;
		delay(100);
		trigger_PENDSV();
	}
	return 0;
}













