#ifndef SPD_H
#define SPD_H

#include "mm32_device.h"

/* 极对数：3 */
#define SPD_DFT_RPM_TAR 1000  // 默认RPM目标稳速，也是转速的基准档
#define SPD_POLE_PAIRS 3  // 电机极对数
#define SPD_RPM_DEAD 1  // RPM目标稳速死区值百分比

#define SPD_RPM_MS 10  // 多久间隔执行一次PI逻辑
#define SPD_CCR_MAX 980  // 占空比值最大值
#define SPD_CCR_MIN 20  // 占空比值最小值
#define SPD_CCR_P_F 0.294f  // P系数
#define SPD_CCR_P_N 16  // P系数放大倍数
#define SPD_CCR_I_F 0.0294f  // I系数
#define SPD_CCR_I_N 18  // I系数放大倍数
#define SPD_RPM_ERR_SUM_MAX 33333  // 转速差积分上下限绝对值

typedef struct
{
    uint8_t cplt : 1;  // PI预占空比值更新完毕事件
    uint8_t ccr_i : 1;  // PI是否允许积分
} SPDFlag;

typedef uint8_t SPDState;
enum
{
    SPD_IDLE,
    SPD_CALC,
    SPD_EXEC
};

void spd_init(void);
void spd_process(void);

extern SPDFlag spdFlag;
extern int32_t spd_ccr_out;
extern uint16_t spd_rpm_tar;

#endif