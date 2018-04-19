#include "tinyOS.h"
#include "ARMCM3.h"


/*********************************************************************************************************
** 系统时钟节拍定时器System Tick配置
** 在我们目前的环境（模拟器）中，系统时钟节拍为12MHz
** 请务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
**********************************************************************************************************/
void tSetSysTickPeriod(uint32_t ms)
{
  SysTick->LOAD  = ms * SystemCoreClock / 1000 - 1; 
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
  SysTick->VAL   = 0;                           
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk; 
}

/**********************************************************************************************************
** Function name        :   SysTick_Handler
** Descriptions         :   SystemTick的中断处理函数。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void SysTick_Handler () 
{
    // 什么都没做，除了进行任务切换
    // 由于tTaskSched自动选择另一个任务切换过去，所以其效果就是
    // 两个任务交替运行，与上一次例子不同的是，这是由系统时钟节拍推动的
    // 如果说，上一个例子里需要每个任务主动去调用tTaskSched切换，那么这里就是不管任务愿不愿意，CPU
    // 的运行权都会被交给另一个任务。这样对每个任务就很公平了，不存在某个任务拒不调用tTaskSched而一直占用CPU的情况
    t_task_system_tick_handler();
}


