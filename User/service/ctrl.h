#ifndef CTRL_H
#define CTRL_H

#include "mm32_device.h"

#define CTRL_BAT_STOP_MV 10000  // VBAT多少时，强制关机且不允许开机
#define CTRL_RPM_TAR_CNT 2  // 额外分多少个速度档，0代表不分档只用基准档转
#define CTRL_RPM_TAR_STEP 1000  // 每个速度档之间相差多少

typedef struct
{
    uint8_t keep : 1;  // 维持事件
} CTRLFlag;

typedef uint8_t CTRLState;
enum
{
    CTRL_IDLE,
    CTRL_EXEC,
    CTRL_STOP
};

void ctrl_init(void);
void ctrl_process(void);

extern CTRLFlag ctrlFlag;

#endif