#include "main.h"

void SysTick_Handler(void)
{
  g_ms++;
}

void EXTI4_15_IRQHandler(void)
{
  if ((EXTI->PR & (1U << 13)) | (EXTI->PR & (1U << 14)) | (EXTI->PR & (1U << 15)))
  {
    EXTI->PR |= (1U << 13) | (1U << 14) | (1U << 15);  // 清除EXTI13/14/15中断事件
    
    hallFlag.trig = 1;
    hall_trig_us = get_us();
  }
}

void DMA1_Channel1_IRQHandler(void)
{
  if (DMA1->ISR & (1U << 1))
  {
    DMA1->IFCR |= (1U << 1);  // 清除通道1传输完成标志

    adcFlag.dma_cplt = 1;
  }
}


