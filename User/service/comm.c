#include "main.h"

COMMFlag commFlag = {0};

/* A方向的换相表 */
const COMMStep comm_dir_a_tab[6] = 
{
    {COMM_UH, COMM_VL, 4},
	{COMM_UH, COMM_WL, 6},
	{COMM_VH, COMM_WL, 2},
	{COMM_VH, COMM_UL, 3},
	{COMM_WH, COMM_UL, 1},
	{COMM_WH, COMM_VL, 5}
};

/* B方向的换相表 */
const COMMStep comm_dir_b_tab[6] = 
{
    {COMM_UH, COMM_VL, 3},
    {COMM_WH, COMM_VL, 2},
    {COMM_WH, COMM_UL, 6},
    {COMM_VH, COMM_UL, 4},
    {COMM_VH, COMM_WL, 5},
    {COMM_UH, COMM_WL, 1}
};

COMMStep commStep = {0};  // 换相对象
COMMStep comm_dir_tab[6] = {0};  // 换相表
uint8_t comm_dir_tab_idx = 0;  // 换相表指针

void comm_process(void)
{
    if (bldcFlag.sts && commFlag.exec)
    {
        commFlag.exec = 0;

        TIM1->BDTR &= ~(1U << 15);  // 总闸使能禁止
        TIM1->CCER &= ~((1U << 0) | (1U << 2) | (1U << 4) | (1U << 6) | (1U << 8) | (1U << 10));  // 上下桥全部通道使能禁止

        switch (bldcFlag.dir)
        {
            case BLDC_DIR_A:
                memcpy(comm_dir_tab, comm_dir_a_tab, sizeof(comm_dir_tab));
                break;
        
            case BLDC_DIR_B:
                memcpy(comm_dir_tab, comm_dir_b_tab, sizeof(comm_dir_tab));
                break;
        }

        if (bldcFlag.close)
        {
            for (uint8_t i = 0; i < 6; i++)
            {
                if (comm_dir_tab[i].idx == hall_idx)
                {
                    commStep = comm_dir_tab[i];
                    break;
                }
            }
        }
        else
        {
            comm_dir_tab_idx++;
            comm_dir_tab_idx = comm_dir_tab_idx > 5 ? 0 : comm_dir_tab_idx;

            commStep = comm_dir_tab[comm_dir_tab_idx];
        }

        switch (commStep.high)
        {
            case COMM_UH:
                TIM1->CCER |= (1U << 0);  // UH
                TIM1->CCR1 = bldc_ccr;
                break;

            case COMM_VH:
                TIM1->CCER |= (1U << 4);  // VH
                TIM1->CCR2 = bldc_ccr;
                break;
                
            case COMM_WH:
                TIM1->CCER |= (1U << 8);  // WH
                TIM1->CCR3 = bldc_ccr;
                break;    
        }

        switch (commStep.low)
        {
            case COMM_UL:
                TIM1->CCER |= (1U << 2);  // UL
                TIM1->CCR1 = 1000;
                break;

            case COMM_VL:
                TIM1->CCER |= (1U << 6);  // VL
                TIM1->CCR2 = 1000;
                break;
                
            case COMM_WL:
                TIM1->CCER |= (1U << 10);  // WL
                TIM1->CCR3 = 1000;
                break;    
        }

        TIM1->SR &= ~(1U << 0);  // 清除更新事件
        TIM1->EGR |= (1U << 0);  // 软件产生更新事件

        TIM1->BDTR |= (1U << 15);  // 总闸使能

        commFlag.cplt = 1;
    }
}