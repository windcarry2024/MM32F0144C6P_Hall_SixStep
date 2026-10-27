#ifndef BTN_H
#define BTN_H

#include "mm32_device.h"

#define BTN_START_DEB_MS 25  // START按键消抖时间
#define BTN_CTRL_DEB_MS 25  // CTRL按键消抖时间

typedef uint8_t BTNState;
enum
{
    BTN_IDLE,
    BTN_DEB_PRS,
    BTN_PRS,
    BTN_DEB_REL
};

typedef struct
{
    uint8_t start_trig : 1;  // START按键边沿事件
    uint8_t start : 1;  // START按键事件
    uint8_t start_prs : 1;  // START按键是否被按压中
    uint8_t ctrl_trig : 1;  // CTRL按键边沿事件
    uint8_t ctrl : 1;  // CTRL按键事件
    uint8_t ctrl_prs : 1;  // CTRL按键是否被按压中
} BTNFlag;

void btn_init(void);
void btn_process(void);

extern BTNFlag btnFlag;

#endif