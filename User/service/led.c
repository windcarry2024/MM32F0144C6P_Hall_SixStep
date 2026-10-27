#include "main.h"

LEDFlag ledFlag = {0};

void led_process(void)
{
    if (ledFlag.red)
    {
        GPIOD->BSRR |= (1U << 2);
    }
    else
    {
        GPIOD->BSRR |= (1U << 18);
    }

    if (ledFlag.grn)
    {
        GPIOB->BSRR |= (1U << 9);
    }
    else
    {
        GPIOB->BSRR |= (1U << 25);
    }
}

void led_init(void)
{
    /* PD2->红色LED */
    RCC->AHBENR |= (1U << 20);  // GPIOD时钟使能
    GPIOD->BSRR |= (1U << 18);
    GPIOD->CRL &= ~(0b1111 << 8);
    GPIOD->CRL |= (0b0001 << 8);

    /* PB9->绿色LED */
    RCC->AHBENR |= (1U << 18);  // GPIOB时钟使能
    GPIOB->BSRR |= (1U << 25);
    GPIOB->CRH &= ~(0b1111 << 4);
    GPIOB->CRH |= (0b0001 << 4);
}