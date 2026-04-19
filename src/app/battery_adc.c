#include "battery_adc.h"

#include <rtdevice.h>

#define BATTERY_ADC_DEV_NAME           "bat1"
#define BATTERY_ADC_CHANNEL            7
#define BATTERY_ADC_FULL_SCALE_RAW     4095U
#define BATTERY_ADC_INPUT_MAX_MV       4700U
#define BATTERY_EMPTY_MV               3300U
#define BATTERY_FULL_MV                4200U
#define BATTERY_UI_LEVEL_MAX           10U

static rt_uint32_t battery_adc_raw_to_mv(rt_uint32_t raw_value)
{
    if (raw_value >= 10000U)
    {
        return raw_value / 10U;
    }

    if (raw_value <= BATTERY_ADC_INPUT_MAX_MV)
    {
        return raw_value;
    }

    return (raw_value * BATTERY_ADC_INPUT_MAX_MV) / BATTERY_ADC_FULL_SCALE_RAW;
}

static rt_uint8_t battery_clamp_percent(rt_uint32_t voltage_mv)
{
    if (voltage_mv <= BATTERY_EMPTY_MV)
    {
        return 0;
    }

    if (voltage_mv >= BATTERY_FULL_MV)
    {
        return 100;
    }

    return (rt_uint8_t)(((voltage_mv - BATTERY_EMPTY_MV) * 100U) /
                        (BATTERY_FULL_MV - BATTERY_EMPTY_MV));
}

rt_err_t BatteryADC_ReadInfo(battery_adc_info_t *info)
{
    rt_adc_device_t adc_dev;
    rt_uint32_t raw_value;
    rt_err_t ret;

    if (info == RT_NULL)
    {
        return -RT_EINVAL;
    }

    adc_dev = (rt_adc_device_t)rt_device_find(BATTERY_ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        return -RT_ENOSYS;
    }

    ret = rt_adc_enable(adc_dev, BATTERY_ADC_CHANNEL);
    if (ret != RT_EOK)
    {
        return ret;
    }

    raw_value = rt_adc_read(adc_dev, BATTERY_ADC_CHANNEL);
    rt_adc_disable(adc_dev, BATTERY_ADC_CHANNEL);

    info->raw = raw_value;
    /* 文档说明 bat1/channel7 输入范围是 0-4.7V，这里按 12bit 原始值换算毫伏。 */
    info->voltage_mv = battery_adc_raw_to_mv(raw_value);
    info->percent = battery_clamp_percent(info->voltage_mv);
    info->level = (rt_uint8_t)((info->percent * BATTERY_UI_LEVEL_MAX + 50U) / 100U);
    if (info->level > BATTERY_UI_LEVEL_MAX)
    {
        info->level = BATTERY_UI_LEVEL_MAX;
    }

    return RT_EOK;
}
