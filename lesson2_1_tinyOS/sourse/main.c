#include "tinyOS.h"

tTask * currentTask; //当前任务 记录当前是哪个任务在运行
tTask * nextTask; 	//下一个即将要执行的任务 在进行任务切换时，先设置好该值，然后任务切换过程中会从中读取下一个任务的信息

tTask * taskTable[2];

tTask tTask1;
tTask tTask2;

tTastStack task1Env[1024];
tTastStack task2Env[1024];

void tTaskInit (tTask *task, void (*entry)(void *), void *param, tTastStack * stack)
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
}

void tTaskSched ()
{
	if (currentTask == taskTable[0])
	{
		nextTask = taskTable[1];
	}
	else
	{
		nextTask = taskTable[0];
	}
	tTaskSwitch();
	
}

void delay(int count)
{
	while(count-- > 0);
	
}


int task1flag;
void task1Entry (void *param)
{
	for(;;)
	{
		task1flag = 0;
		delay(100);
		task1flag = 1;
		delay(100);
		tTaskSched();
	
	}

}

int task2flag;
void task2Entry (void *param)
{
	for(;;)
	{
		task2flag = 0;
		delay(100);
		task2flag = 1;
		delay(100);
		tTaskSched();	
	}
}





int main(void)
{
	// 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
	tTaskInit(&tTask1, task1Entry, (void* )0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, &task2Env[1024]);
	
	//将任务加入到任务表中
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	
	//我们希望先运行任务1
	nextTask = taskTable[0];
	
	//切换到nextTask 这个函数不会返回
	tTaskRunFirst();

	return 0;
}













