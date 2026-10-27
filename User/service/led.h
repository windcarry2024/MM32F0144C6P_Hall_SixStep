#ifndef LED_H
#define LED_H

#include "mm32_device.h"

typedef struct
{
    uint8_t red : 1;  // 红色LED开关
    uint8_t grn : 1;  // 绿色LED开关
} LEDFlag;

void led_init(void);
void led_process(void);

extern LEDFlag ledFlag;

#endif