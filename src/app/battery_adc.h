#ifndef __BATTERY_ADC_H__
#define __BATTERY_ADC_H__

#include "rtthread.h"

typedef struct
{
    rt_uint32_t raw;
    rt_uint32_t voltage_mv;
    rt_uint8_t percent;
    rt_uint8_t level;
} battery_adc_info_t;

rt_err_t BatteryADC_ReadInfo(battery_adc_info_t *info);

#endif /* __BATTERY_ADC_H__ */
