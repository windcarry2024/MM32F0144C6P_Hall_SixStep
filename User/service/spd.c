#include "main.h"

SPDFlag spdFlag = {0};
SPDState spdState = SPD_IDLE;
uint32_t spd_ms = 0;

uint32_t spd_us_a = 0;  // 霍尔时刻记录点A
uint32_t spd_us_b = 0;  // 霍尔时刻记录点B
uint16_t spd_cyc_us = 0;  // 一个完整周期耗时
uint16_t spd_sec_cnt = 0;  // 变化了多少次扇区计数
uint16_t spd_rpm = 0;  // 当前转速

uint16_t spd_rpm_tar = 0;  // 目标转速
int16_t spd_rpm_err = 0;  // 转速差
int32_t spd_rpm_err_sum = 0;  // 转速差积分
int32_t spd_ccr_p_mix = 0;  // 放大后的P系数
int32_t spd_ccr_i_mix = 0;  // 放大后的I系数
int32_t spd_ccr_out = 0;  // PI输出值

void spd_process(void)
{
    switch (spdState)
    {
        case SPD_IDLE:
            if (bldcFlag.sts)
            {
                if (g_ms - spd_ms >= SPD_RPM_MS)
                {
                    if (hallFlag.chg)
                    {
                        hallFlag.chg = 0;
                        spd_us_a = hall_chg_us;  // 记录A时刻
                        spd_sec_cnt = 0;  // 扇区变化计数器清零

                        spdState = SPD_CALC;
                    }
                }
            }
            break;

        case SPD_CALC:
            if (hallFlag.chg)
            {
                hallFlag.chg = 0;
                spd_sec_cnt++;
                if (spd_sec_cnt >= 6)
                {
                    spd_us_b = hall_chg_us;  // 记录B时刻
                    spdState = SPD_EXEC;
                }
            }
            break;
            
        case SPD_EXEC:
            spd_cyc_us = spd_us_b - spd_us_a;
            spd_rpm = 60000000 / (spd_cyc_us * SPD_POLE_PAIRS);

            if (bldcFlag.close)
            {
                if (spd_rpm > spd_rpm_tar + (uint32_t)spd_rpm_tar * SPD_RPM_DEAD / 100 || spd_rpm < spd_rpm_tar - (uint32_t)spd_rpm_tar * SPD_RPM_DEAD / 100)
                {
                    spdFlag.ccr_i = 1;

                    spd_rpm_err = spd_rpm_tar - spd_rpm;
                    spd_ccr_out = ((spd_rpm_err * spd_ccr_p_mix) >> SPD_CCR_P_N) + ((spd_rpm_err_sum * spd_ccr_i_mix) >> SPD_CCR_I_N);

                    /* 输出限幅 */
                    spd_ccr_out = spd_ccr_out > SPD_CCR_MAX ? SPD_CCR_MAX : spd_ccr_out;
                    spd_ccr_out = spd_ccr_out < SPD_CCR_MIN ? SPD_CCR_MIN : spd_ccr_out;

                    /* 遇限停积 */
                    if ((spd_ccr_out >= SPD_CCR_MAX && spd_rpm_err > 0) || (spd_ccr_out <= SPD_CCR_MIN && spd_rpm_err < 0))
                    {
                        spdFlag.ccr_i = 0;
                    }

                    if (spdFlag.ccr_i)
                    {
                        spd_rpm_err_sum += spd_rpm_err;

                        /* 积分限幅 */
                        spd_rpm_err_sum = spd_rpm_err_sum > SPD_RPM_ERR_SUM_MAX ? SPD_RPM_ERR_SUM_MAX : spd_rpm_err_sum;
                        spd_rpm_err_sum = spd_rpm_err_sum < -SPD_RPM_ERR_SUM_MAX ? -SPD_RPM_ERR_SUM_MAX : spd_rpm_err_sum;
                    }

                    spdFlag.cplt = 1;
                }
            }

            spd_ms = g_ms;
            spdState = SPD_IDLE;
            break;    
        
        default:
            spdState = SPD_IDLE;
            break;
    }
}

void spd_init(void)
{
    spd_ccr_p_mix = SPD_CCR_P_F * (float)(1U << SPD_CCR_P_N);
    spd_ccr_i_mix = SPD_CCR_I_F * (float)(1U << SPD_CCR_I_N);
}