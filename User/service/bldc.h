#ifndef BLDC_H
#define BLDC_H

#include "mm32_device.h"

#define BLDC_ALIGN_CCR 360  // 预定位的占空比值是多少
#define BLDC_ALIGN_US 2000  // 预定位的需要定多久
#define BLDC_OPEN_US 8000  // 开环强拖的换相周期是多少
#define BLDC_OPEN_CCR 500  // 开环强拖的占空比值是多少
#define BLDC_OPEN_CNT_TAR 6  // 开环强拖需要换相多少次再切闭环
#define BLDC_DM_MA_MAX 3900  // DM限流保护阈值

typedef uint8_t BLDCState;
enum
{
    BLDC_IDLE,
    BLDC_ALIGN,
    BLDC_OPEN,
    BLDC_CLOSE,
    BLDC_STOP
};

typedef struct
{
    uint8_t sts : 1;  // 电机开关
    uint8_t dir : 1;  // 电机旋转方向
    uint8_t close : 1;  // 闭环霍尔自转开关
} BLDCFlag;

/* 旋转方向 */
enum
{
    BLDC_DIR_A,
    BLDC_DIR_B
};

void bldc_init(void);
void bldc_process(void);

extern BLDCFlag bldcFlag;
extern uint16_t bldc_ccr;

#endif