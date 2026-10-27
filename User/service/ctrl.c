#include "main.h"

CTRLFlag ctrlFlag = {0};
CTRLState ctrlState = CTRL_IDLE;
uint32_t ctrl_ms = 0;

uint16_t ctrl_rpm_tar_tab[CTRL_RPM_TAR_CNT + 1] = {0};  // 电机速度档表
uint8_t ctrl_rpm_tar_tab_idx = 0;  // 电机速度档表指针

void ctrl_process(void)
{
    switch (ctrlState)
    {
        case CTRL_IDLE:
            if (btnFlag.start)
            {
                btnFlag.start = 0;

                /* 电池欠压保护 */
                if (adc_vbus_mv >= CTRL_BAT_STOP_MV)
                {
                    btnFlag.ctrl = 0;  // 防累积
                    bldcFlag.sts = 1;
                    ledFlag.grn = 1;
                    ctrl_rpm_tar_tab_idx = 0;

                    ctrlState = CTRL_EXEC;
                }
            }
            break;

        case CTRL_EXEC:
            if (btnFlag.ctrl)
            {
                if (!ctrlFlag.keep)
                {
                    /* CTRL按键调速 */
                    ctrl_rpm_tar_tab_idx++;
                    ctrl_rpm_tar_tab_idx = ctrl_rpm_tar_tab_idx > 2 ? 0 : ctrl_rpm_tar_tab_idx;
                    spd_rpm_tar = ctrl_rpm_tar_tab[ctrl_rpm_tar_tab_idx];
                    
                    ctrlFlag.keep = 1;
                    ledFlag.red = 1;
                    ctrl_ms = g_ms;
                }
                else
                {
                    if (g_ms - ctrl_ms >= 100)
                    {
                        ctrlFlag.keep = 0;
                        ledFlag.red = 0;
                        btnFlag.ctrl = 0;
                    }
                }
            }

            if (adc_vbus_mv < CTRL_BAT_STOP_MV)
            {
                ctrlState = CTRL_STOP;
            }

            if (btnFlag.start)
            {
                btnFlag.start = 0;

                ctrlState = CTRL_STOP;
            }
            break;
            
        case CTRL_STOP:
            bldcFlag.sts = 0;
            ledFlag.grn = 0;
            ledFlag.red = 0;
            btnFlag.start = 0;
            btnFlag.ctrl = 0;

            ctrlState = CTRL_IDLE;
            break;    
        
        default:
            ctrlState = CTRL_IDLE;
            break;
    }
}

void ctrl_init(void)
{
    for (uint8_t i = 0; i < CTRL_RPM_TAR_CNT + 1; i++)
    {
        ctrl_rpm_tar_tab[i] = SPD_DFT_RPM_TAR + i * CTRL_RPM_TAR_STEP;
    }
}