#ifndef SYSCK_H
#define SYSCK_H

#include "mm32_device.h"

/* AHB = CPU = APB2 = 72MHZ , APB1 = 36MHZ , SystemCoreClock = 72000000 */
void systick_init(void);
void delay_us(uint32_t us);
void g_us_init(void);
void g_us_process(void);
uint32_t get_us(void);

extern volatile uint32_t g_ms;

#endif