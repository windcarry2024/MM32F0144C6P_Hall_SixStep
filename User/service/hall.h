#ifndef HALL_H
#define HALL_H

#include "mm32_device.h"

#define HALL_DEB_US 3  // 霍尔消抖时间
#define HALL_DEB_CNT_TAR 5  // 霍尔消抖次数

typedef uint8_t HallState;
enum
{
    HALL_IDLE,
    HALL_DEB_CHG,
    HALL_CHG
};

typedef struct
{
    volatile uint8_t trig : 1;  // 霍尔边沿事件
    uint8_t comm : 1;  // 霍尔通知换相事件
    uint8_t chg : 1;  // 霍尔索引值改变事件
} HallFlag;

typedef struct
{
    uint8_t a : 1;
    uint8_t b : 1;
    uint8_t c : 1;
} Hall;

void hall_init(void);
void hall_process(void);

extern HallFlag hallFlag;
extern uint8_t hall_idx;
extern volatile uint32_t hall_trig_us;
extern uint8_t hall_trig_idx;
extern uint32_t hall_chg_us;

#endif