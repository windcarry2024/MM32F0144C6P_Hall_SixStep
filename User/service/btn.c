#include "main.h"

BTNFlag btnFlag = {0};
BTNState btnState = BTN_IDLE;
uint32_t btn_ms = 0;

void btn_process(void)
{
    switch (btnState)
    {
        case BTN_IDLE:
            if (!(GPIOB->IDR & (1U << 12)))
            {
                btnFlag.start_trig = 1;
                btn_ms = g_ms;
                btnState = BTN_DEB_PRS;
            }
            else if (!(GPIOB->IDR & (1U << 11)))
            {
                btnFlag.ctrl_trig = 1;
                btn_ms = g_ms;
                btnState = BTN_DEB_PRS;
            }
            break;

        case BTN_DEB_PRS:
            if (btnFlag.start_trig)
            {
                if (g_ms - btn_ms >= BTN_START_DEB_MS)
                {
                    if (!(GPIOB->IDR & (1U << 12)))
                    {
                        btnFlag.start = 1;
                        btnFlag.start_prs = 1;
                        btnState = BTN_PRS;
                    }
                    else
                    {
                        btnState = BTN_IDLE;
                    }
                    btnFlag.start_trig = 0;
                }
            }
            else if (btnFlag.ctrl_trig)
            {
                if (g_ms - btn_ms >= BTN_CTRL_DEB_MS)
                {
                    if (!(GPIOB->IDR & (1U << 11)))
                    {
                        btnFlag.ctrl = 1;
                        btnFlag.ctrl_prs = 1;
                        btnState = BTN_PRS;
                    }
                    else
                    {
                        btnState = BTN_IDLE;
                    }
                    btnFlag.ctrl_trig = 0;
                }
            }
            break;
            
        case BTN_PRS:
            if (btnFlag.start_prs)
            {
                if (GPIOB->IDR & (1U << 12))
                {
                    btnFlag.start_trig = 1;
                    btn_ms = g_ms;
                    btnState = BTN_DEB_REL;
                }
            }
            else if (btnFlag.ctrl_prs)
            {
                if (GPIOB->IDR & (1U << 11))
                {
                    btnFlag.ctrl_trig = 1;
                    btn_ms = g_ms;
                    btnState = BTN_DEB_REL;
                }
            }
            break;

        case BTN_DEB_REL:
            if (btnFlag.start_trig)
            {
                if (g_ms - btn_ms >= BTN_START_DEB_MS)
                {
                    if (GPIOB->IDR & (1U << 12))
                    {
                        btnFlag.start_prs = 0;
                        btnState = BTN_IDLE;
                    }
                    else
                    {
                        btnState = BTN_PRS;
                    }
                    btnFlag.start_trig = 0;
                }
            }
            else if (btnFlag.ctrl_trig)
            {
                if (g_ms - btn_ms >= BTN_CTRL_DEB_MS)
                {
                    if (GPIOB->IDR & (1U << 11))
                    {
                        btnFlag.ctrl_prs = 0;
                        btnState = BTN_IDLE;
                    }
                    else
                    {
                        btnState = BTN_PRS;
                    }
                    btnFlag.ctrl_trig = 0;
                }
            }
            break;
        
        default:
            btnState = BTN_IDLE;
            break;
    }
}

void btn_init(void)
{
    /* PB12->START按键 */
    RCC->AHBENR |= (1U << 18);  // GPIOB时钟使能
    GPIOB->ODR |= (1U << 12);
    GPIOB->CRH &= ~(0b1111 << 16);
    GPIOB->CRH |= (0b1000 << 16);
    
    /* PB11->CTRL按键 */
    GPIOB->ODR |= (1U << 11);
    GPIOB->CRH &= ~(0b1111 << 12);
    GPIOB->CRH |= (0b1000 << 12);
}