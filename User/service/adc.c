#include "main.h"

ADCFlag adcFlag = {0};
ADCState adcState = ADC_IDLE;
uint32_t adc_ms_vref = 0;
uint32_t adc_ms_vbus = 0;
uint32_t adc_ms_dm = 0;

uint16_t adc_buf[1] = {0};  // ADC码值数组
uint16_t adc_vref_cal = 0;  // 出厂校准值

uint16_t adc_vref_raw = 0;  // vref码值
uint16_t adc_vbus_raw = 0;  // vbus码值
uint16_t adc_dm_raw = 0;  // dm码值
uint16_t adc_dm_zero_raw = 0;  // dm的零点偏置码值
uint32_t adc_dm_zero_raw_sum = 0;  // dm的零点偏置码值总和

uint16_t adc_vref_mv = 0;  // vref电压值
uint16_t adc_vbus_mv = 0;  // vbus电压值
uint16_t adc_dm_ma = 0;  // dm电流值

void adc_process(void)
{
    switch (adcState)
    {
        case ADC_IDLE:
            if (g_ms - adc_ms_dm >= ADC_DM_UPD_MS)
            {
                if (!(ADC1->ADSTA & (1U << 2)))  // 非注入通道空闲时
                {
                    /* 切换通道队列[0]为DM->AIN1 */
                    ADC1->CHANY0 &= ~(0b1111 << 0);
                    ADC1->CHANY0 |= (0b0001 << 0);

                    ADC1->ADCR |= (1U << 8);  // ADC1开始采样
                    adcState = ADC_DM;
                }
            }
            else if (g_ms - adc_ms_vbus >= ADC_VBUS_UPD_MS)
            {
                if (!(ADC1->ADSTA & (1U << 2)))
                {
                    /* 切换通道队列[0]为VBUS->AIN2 */
                    ADC1->CHANY0 &= ~(0b1111 << 0);
                    ADC1->CHANY0 |= (0b0010 << 0);

                    ADC1->ADCR |= (1U << 8);
                    adcState = ADC_VBUS;
                }
            }
            else if (g_ms - adc_ms_vref >= ADC_VREF_UPD_MS)
            {
                if (!(ADC1->ADSTA & (1U << 2)))
                {
                    /* 切换通道队列[0]为VREF->AIN15 */
                    ADC1->CHANY0 &= ~(0b1111 << 0);
                    ADC1->CHANY0 |= (0b1111 << 0);

                    ADC1->ADCR |= (1U << 8);
                    adcState = ADC_VREF;
                }
            }
            break;

        case ADC_VREF:
            if (adcFlag.dma_cplt)
            {
                adcFlag.dma_cplt = 0;

                adc_vref_raw = adc_buf[0];
                adc_vref_mv = (uint32_t)(3300 * adc_vref_cal) / adc_vref_raw;

                adc_ms_vref = g_ms;
                adcState = ADC_IDLE;
            }
            break;
            
        case ADC_VBUS:
            if (adcFlag.dma_cplt)
            {
                adcFlag.dma_cplt = 0;

                adc_vbus_raw = adc_buf[0];

                /* ((adc_vbus_raw * adc_vref_mv / 4095) / 20000) * (100000 + 20000) */
                adc_vbus_mv = (uint32_t)(adc_vbus_raw * adc_vref_mv) / 683;

                adc_ms_vbus = g_ms;
                adcState = ADC_IDLE;
            }
            break;
            
        case ADC_DM:
            if (adcFlag.dma_cplt)
            {
                adcFlag.dma_cplt = 0;

                adc_dm_raw = adc_buf[0];

                /* ((adc_dm_raw - adc_dm_zero_raw) * adc_vref_mv / 4095) / 50 / 10 * 1000 */
                adc_dm_ma = (uint32_t)(abs(adc_dm_raw - adc_dm_zero_raw) * adc_vref_mv) / 2048;

                adc_ms_dm = g_ms;
                adcState = ADC_IDLE;
            }
            break;    
        
        default:
            adcState = ADC_IDLE;
            break;
    }
}

void adc_init(void)
{
    /* DM->PA1->AIN1、VBUS->PA2->AIN2 */
    RCC->AHBENR |= (1U << 17);  // GPIOA时钟使能
    GPIOA->CRL &= ~((0b1111 << 4) | (0b1111 << 8));

    RCC->APB2ENR |= (1U << 9);  // ADC1时钟使能

    /* 预分频 = APB2(72mhz) / (4 + 2) = 12mhz，12位有效，内部电压传感器使能 */
    ADC1->ADCFG &= ~((1U << 14) | (0b111 << 7) | (0b111 << 4));
    ADC1->ADCFG |= (0b010 << 4) | (1U << 3);

    /* 通道队列从低到高扫描，右对齐格式，单次转换模式，DMA使能 */
    ADC1->ADCR &= ~((1U << 16) | (1U << 11) | (0b11 << 9));
    ADC1->ADCR |= (1U << 3);

    /* DM->AIN1、VBUS->AIN2、VREF->AIN15通道使能 */
    ADC1->ADCHS |= (1U << 1) | (1U << 2) | (1U << 15);

    /* 默认通道队列[0]为VREF->AIN15 */
    ADC1->CHANY0 &= ~(0b1111 << 0);
    ADC1->CHANY0 |= (0b1111 << 0);

    /* 只转换通道队列[0] */
    ADC1->ANYCFG &= ~(0b1111 << 0);

    /* 任意通道转换使能 */
    ADC1->ANYCR |= (1U << 0);

    /* 固定转化12.5个周期，转化时间为：83.3ns x 12.5 = 1041.25ns */
    /* DM源阻抗0.33kΩ，设置6.5个周期，采样时间为：83.3ns x 6.5 = 541.45ns */
    /* VBUS源阻抗17kΩ，设置29.5个周期，采样时间为：83.3ns x 29.5 = 2457.35ns */
    /* VREF设置240.5个周期，采样时间为：83.3ns x 240.5 = 20033.65ns */
    ADC1->SMPR1 &= ~((0b1111 << 4) | (0b1111 << 8));
    ADC1->SMPR1 |= (0b1011 << 4) | (0b0011 << 8);
    ADC1->SMPR2 &= ~(0b1111 << 28);
    ADC1->SMPR2 |= (0b0111 << 28);
    
    ADC1->ADCFG |= (1U << 0);  // ADC1使能
    delay_us(512);

    RCC->AHBENR |= (1U << 0);  // DMA1时钟使能
    
    /* 关闭M2M模式，通道优先级高，存储器数据宽度12位，外设数据宽度12位，关闭存储器地址递增，关闭外设地址递增，使能循环操作，从外设读，传输完成中断使能 */
    DMA1->IFCR |= (1U << 1);  // 清除通道1传输完成标志
    DMA1_Channel1->CCR &= ~((1U << 14) | (0b11 << 12) | (0b11 << 10) | (0b11 << 8) | (1U << 7) | (1U << 6) | (1U << 4));
    DMA1_Channel1->CCR |= (0b10 << 12) | (0b01 << 10) | (0b01 << 8) | (1U << 5) | (1U << 1);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    NVIC_SetPriority(DMA1_Channel1_IRQn, 1);

    DMA1_Channel1->CNDTR = 1;  // 传输数量为1
    DMA1_Channel1->CPAR = &(ADC1->ADDATA);  // 设置外设地址
    DMA1_Channel1->CMAR = adc_buf;  // 设置存储器地址

    DMA1_Channel1->CCR |= (1U << 0);  // DMA1的通道1使能
    delay_us(256);

    adc_vref_cal = *ADC_VREF_CAL_ADDR;  // 出厂校准值读取

    /* 上电先把第一次vref给测量出来 */
    ADC1->ADCR |= (1U << 8);  // ADC1开始采样
    while (!adcFlag.dma_cplt)
    {
        __NOP();
    }
    adcFlag.dma_cplt = 0;
    adc_vref_raw = adc_buf[0];
    adc_vref_mv = (uint32_t)(3300 * adc_vref_cal) / adc_vref_raw;

    /* 上电先把零点偏置码值给测量出来 */
    /* 切换通道队列[0]为DM->PA1->AIN1 */
    ADC1->CHANY0 &= ~(0b1111 << 0);
    ADC1->CHANY0 |= (0b0001 << 0);
    for (uint8_t i = 0; i < ADC_DM_ZERO_CNT_TAR; i++)
    {
        ADC1->ADCR |= (1U << 8);  // ADC1开始采样
        while (!adcFlag.dma_cplt)
        {
            __NOP();
        }
        adcFlag.dma_cplt = 0;
        adc_dm_zero_raw_sum += adc_buf[0];
    }
    adc_dm_zero_raw = adc_dm_zero_raw_sum / ADC_DM_ZERO_CNT_TAR;
}