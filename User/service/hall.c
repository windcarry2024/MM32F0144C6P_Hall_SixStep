#include "main.h"

HallFlag hallFlag = {0};
HallState hallState = HALL_IDLE;
uint32_t hall_us = 0;
Hall hall = {0};

volatile uint32_t hall_trig_us = 0;  // 霍尔边沿触发的时刻
uint8_t hall_trig_idx = 0;  // 霍尔边沿触发时的索引值
uint8_t hall_cnt = 0;  // 霍尔消抖计数

uint8_t hall_idx = 0;  // 霍尔索引值
uint32_t hall_chg_us = 0;  // 霍尔索引值改变的时刻

uint8_t debug_hall_cnt = 0;  // 调试测极对数用
uint8_t debug_hall_tab[36] = {0};  // 调试测霍尔索引值的变化规律用，数组
uint8_t debug_hall_tab_idx = 0;  // 调试测霍尔索引值的变化规律用，指针

void hall_process(void)
{
    switch (hallState)
    {
        case HALL_IDLE:
            if (hallFlag.trig)
            {
                hallFlag.trig = 0;

                /* 组装霍尔ABC索引值 */
                hall.a = (GPIOC->IDR & (1U << 15)) ? 1 : 0;
                hall.b = (GPIOC->IDR & (1U << 14)) ? 1 : 0;
                hall.c = (GPIOC->IDR & (1U << 13)) ? 1 : 0;
                hall_trig_idx = (hall.a << 2) | (hall.b << 1) | (hall.c << 0);

                if (hall_trig_idx != hall_idx)
                {
                    hall_cnt = 0;

                    if (hall_trig_idx != 0 && hall_trig_idx != 7)
                    {
                        hall_us = get_us();
                        hallState = HALL_DEB_CHG;
                    }
                }
            }
            break;

        case HALL_DEB_CHG:
            if (get_us() - hall_us >= HALL_DEB_US)
            {
                hall.a = (GPIOC->IDR & (1U << 15)) ? 1 : 0;
                hall.b = (GPIOC->IDR & (1U << 14)) ? 1 : 0;
                hall.c = (GPIOC->IDR & (1U << 13)) ? 1 : 0;
                
                if (hall_trig_idx == ((hall.a << 2) | (hall.b << 1) | (hall.c << 0)))
                {
                    hall_cnt++;
                    if (hall_cnt >= HALL_DEB_CNT_TAR)
                    {
                        hallState = HALL_CHG;
                    }
                    else
                    {
                        hall_us = get_us();
                    }
                }
                else
                {
                    hallState = HALL_IDLE;
                }
            }
            break;
            
        case HALL_CHG:
            hall_chg_us = hall_trig_us;
            hall_idx = hall_trig_idx;
            hallFlag.comm = 1;
            hallFlag.chg = 1;

            /* DEBUG */
            if (debug_hall_cnt < 255)
            {
                debug_hall_cnt++;
            }
            if (debug_hall_tab_idx < 36)
            {
                debug_hall_tab[debug_hall_tab_idx] = hall_idx;
                debug_hall_tab_idx++;
            }
            
            hallState = HALL_IDLE;
            break;    
        
        default:
            hallState = HALL_IDLE;
            break;
    }
}

void hall_init(void)
{
    /* HA->PC15->EXTI15、HB->PC14->EXTI14、HC->PC13->EXTI13 */
    /* PC13/14/15设置为内部上拉 */
    RCC->AHBENR |= (1U << 19);  // GPIOC时钟使能
    GPIOC->ODR |= (1U << 13) | (1U << 14) | (1U << 15);
    GPIOC->CRH &= ~((0b1111 << 20) | (0b1111 << 24) | (0b1111 << 28));
    GPIOC->CRH |= (0b1000 << 20) | (0b1000 << 24) | (0b1000 << 28);

    /* EXTI13/14/15绑定PC13/14/15 */
    RCC->APB2ENR |= (1U << 0);  // SYSCFG时钟使能
    SYSCFG->EXTICR[3] &= ~((0b1111 << 4) | (0b1111 << 8) | (0b1111 << 12));
    SYSCFG->EXTICR[3] |= (0b0010 << 4) | (0b0010 << 8) | (0b0010 << 12);

    /* 设置EXTI13/14/15中断使能 */
    RCC->APB2ENR |= (1U << 29);  // EXTI时钟使能
    EXTI->PR |= (1U << 13) | (1U << 14) | (1U << 15);  // 清除EXTI13/14/15中断事件
    EXTI->IMR |= (1U << 13) | (1U << 14) | (1U << 15);  // EXTI13/14/15中断使能
    EXTI->EMR |= (1U << 13) | (1U << 14) | (1U << 15);  // EXTI13/14/15事件使能
    EXTI->RTSR |= (1U << 13) | (1U << 14) | (1U << 15);  // EXTI13/14/15中断上升沿触发事件使能
    EXTI->FTSR |= (1U << 13) | (1U << 14) | (1U << 15);  // EXTI13/14/15中断下降沿触发事件使能
    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 1);
}