#include "main.h"

BLDCFlag bldcFlag = {0};
BLDCState bldcState = BLDC_IDLE;
uint32_t bldc_us = 0;

uint16_t bldc_ccr = 0;  // 上桥斩波占空比值
uint8_t bldc_open_cnt = 0;  // 开环强拖的换相计数

void bldc_process(void)
{
    if (!bldcFlag.sts)
    {
        bldcState = BLDC_STOP;
    }
    
    switch (bldcState)
    {
        case BLDC_IDLE:
            if (bldcFlag.sts)
            {
                /* 正反转控制 */
                bldcFlag.dir = BLDC_DIR_A;

                /* 预定位 */
                TIM1->CCER |= (1U << 0);  // UH
                TIM1->CCR1 = BLDC_ALIGN_CCR;

                TIM1->CCER |= (1U << 6);  // VL
                TIM1->CCR2 = 1000;

                TIM1->SR &= ~(1U << 0);  // 清除更新事件
                TIM1->EGR |= (1U << 0);  // 软件产生更新事件

                TIM1->BDTR |= (1U << 15);  // 总闸使能

                bldc_us = get_us();
                bldcState = BLDC_ALIGN;
            }
            break;

        case BLDC_ALIGN:
            if (get_us() - bldc_us >= BLDC_ALIGN_US)
            {
                /* 开环强拖 */
                comm_dir_tab_idx = 0;  // 指针归位
                bldc_open_cnt = 0;  // 计数重置
                commFlag.exec = 1;
                bldc_ccr = BLDC_OPEN_CCR;

                bldc_us = get_us();
                bldcState = BLDC_OPEN;
            }
            break;

        case BLDC_OPEN:
            if (get_us() - bldc_us >= BLDC_OPEN_US)
            {
                commFlag.exec = 1;
                bldc_us = get_us();
            }

            if (commFlag.cplt)
            {
                commFlag.cplt = 0;
                bldc_open_cnt++;

                if (bldc_open_cnt >= BLDC_OPEN_CNT_TAR)
                {
                    /* 闭环霍尔自转 */
                    spd_rpm_tar = SPD_DFT_RPM_TAR;
                    bldcFlag.close = 1;
                    bldcState = BLDC_CLOSE;
                }
            }
            break;
            
        case BLDC_CLOSE:
            if (hallFlag.comm)
            {
                hallFlag.comm = 0;
                commFlag.exec = 1;
            }

            if (spdFlag.cplt)
            {
                spdFlag.cplt = 0;
                bldc_ccr = spd_ccr_out;
            }

            if (adc_dm_ma >= BLDC_DM_MA_MAX)
            {
                bldcFlag.sts = 0;
                btnFlag.start = 1;
            }
            break;
            
        case BLDC_STOP:
            TIM1->BDTR &= ~(1U << 15);  // 总闸使能禁止
            TIM1->CCER &= ~((1U << 0) | (1U << 2) | (1U << 4) | (1U << 6) | (1U << 8) | (1U << 10));  // 上下桥全部通道使能禁止

            bldcState = BLDC_IDLE;
            break;    
        
        default:
            bldcState = BLDC_IDLE;
            break;
    }
}

void bldc_init(void)
{
    /* 上下桥全部用AF2：UH->PA8->OC1，VH->PA9->OC2，WH->PA10->OC3，UL->PB13->OC1N，VL->PB14->OC2N，WL->PB15->OC3N */
    RCC->AHBENR |= (1U << 17);  // GPIOA时钟使能
    GPIOA->BSRR |= (1U << 24) | (1U << 25) | (1U << 26);  // 防直通
    GPIOA->CRH &= ~((0b1111 << 0) | (0b1111 << 4) | (0b1111 << 8));
    GPIOA->CRH |= (0b1001 << 0) | (0b1001 << 4) | (0b1001 << 8);
    GPIOA->AFRH &= ~((0b1111 << 0) | (0b1111 << 4) | (0b1111 << 8));
    GPIOA->AFRH |= (0b0010 << 0) | (0b0010 << 4) | (0b0010 << 8);

    RCC->AHBENR |= (1U << 18);  // GPIOB时钟使能
    GPIOB->BSRR |= (1U << 29) | (1U << 30) | (1U << 31);  // 防直通
    GPIOB->CRH &= ~((0b1111 << 20) | (0b1111 << 24) | (0b1111 << 28));
    GPIOB->CRH |= (0b1001 << 20) | (0b1001 << 24) | (0b1001 << 28);
    GPIOB->AFRH &= ~((0b1111 << 20) | (0b1111 << 24) | (0b1111 << 28));
    GPIOB->AFRH |= (0b0010 << 20) | (0b0010 << 24) | (0b0010 << 28);
    
    RCC->APB2ENR |= (1U << 11);  // TIM1时钟使能
    
    /* tDTS = tINT_CK，ARR预装载使能，边沿对齐模式，计数器递增模式，单脉冲模式使能禁止，多源可触发更新事件，允许更新事件 */
    TIM1->CR1 &= ~((0b11 << 8) | (0b11 << 5) | (1U << 4) | (1U << 3) | (1U << 2) | (1U << 1));
    TIM1->CR1 |= (1U << 7);

    /* 输出通道预装载使能禁止，上下桥全部空闲电平设置为低电平 */
    TIM1->CR2 &= ((1U << 0) | (0b111111 << 8));

    /* 上下桥：设置为输出模式，快速比较使能禁止，CCR预装载使能，PWM1模式 */
    TIM1->CCMR1 &= ~((0b1111111 << 0) | (0b1111111 << 8));
    TIM1->CCMR1 |= (0b1101000 << 0) | (0b1101000 << 8);
    TIM1->CCMR2 &= ~(0b1111111 << 0);
    TIM1->CCMR2 |= (0b1101000 << 0);

    /* 上下桥：默认全部禁止输出，高电平有效 */
    TIM1->CCER &= ~((0b1111 << 0) | (0b1111 << 4) | (0b1111 << 8));

    /* APB2时钟频率 = 72mhz，PWM输出频率 = 12khz，目标Tick = 1000 */
    TIM1->PSC = 5;
    TIM1->ARR = 999;

    /* 默认上桥斩波占空比值 */
    TIM1->CCR1 = 500;
    TIM1->CCR2 = 500;
    TIM1->CCR3 = 500;

    /* 默认MOE = 0，OSSI = OSSR = 1 */
    TIM1->BDTR &= ~(1U << 15);
    TIM1->BDTR |= (1U << 11) | (1U << 10);

    /* tDTS = 13.9ns，MCU死区时间 = 24 x 13.9ns = 333.6ns */
    /* 总死区时间（估算） = 栅极驱动典型死区 + MCU死区 = 533.6ns */
    TIM1->BDTR &= ~(0b11111111 << 0);
    TIM1->BDTR |= (0b00011000 << 0);

    TIM1->SR &= ~(1U << 0);  // 清除更新事件
    TIM1->EGR |= (1U << 0);  // 软件产生更新事件

    TIM1->CR1 |= (1U << 0);  // TIM1使能
}