#include "main.h"

volatile uint32_t g_ms = 0;

volatile uint32_t g_us = 0;
volatile uint16_t g_us_now = 0;
volatile uint16_t g_us_prev = 0;

void systick_init(void)
{
    SysTick_Config(72000000U / 1000U);
}

void delay_us(uint32_t us)
{
    volatile uint32_t i;

    for (i = 0; i < us * 6U; i++)
    {
        __NOP();
    }
}

void g_us_init(void)
{
    RCC->APB2ENR |= (1U << 18);  // TIM17时钟使能

    /* 目标1mhz，即CNT每1us计数1次 */
    TIM17->PSC = 71;
    TIM17->ARR = 0xFFFF;
    
    TIM17->CR1 |= (1U << 0);  // TIM17使能
}

void g_us_process(void)
{
    g_us_now = (uint16_t)(TIM17->CNT);
    g_us += (uint16_t)(g_us_now - g_us_prev);
    g_us_prev = g_us_now;
}

uint32_t get_us(void)
{
    g_us_now = (uint16_t)(TIM17->CNT);
    return g_us + (uint16_t)(g_us_now - g_us_prev);
}