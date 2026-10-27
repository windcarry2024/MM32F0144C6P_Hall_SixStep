#ifndef ADC_H
#define ADC_H

#include "mm32_device.h"

#define ADC_VREF_CAL_ADDR ((volatile uint16_t *)0x1FFFF7E0)  // 出厂校准值的flash存储地址
#define ADC_VREF_UPD_MS 100  // vref多久更新一次
#define ADC_VBUS_UPD_MS 10  // vbus多久更新一次
#define ADC_DM_UPD_MS 1  // dm多久更新一次
#define ADC_DM_ZERO_CNT_TAR 32  // dm的零点偏置码值取多少次平均值

typedef uint8_t ADCState;
enum
{
    ADC_IDLE,
    ADC_VREF,
    ADC_VBUS,
    ADC_DM
};

typedef struct
{
    volatile uint8_t dma_cplt : 1;  // DMA搬运完成事件
} ADCFlag;

extern ADCFlag adcFlag;
extern uint16_t adc_dm_ma;
extern uint16_t adc_vbus_mv;

#endif