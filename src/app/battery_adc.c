#include "battery_adc.h"

#include <rtdevice.h>

#define BATTERY_ADC_DEV_NAME           "bat1"
#define BATTERY_ADC_CHANNEL            7
#define BATTERY_ADC_FULL_SCALE_RAW     4095U
#define BATTERY_ADC_INPUT_MAX_MV       4700U
#define BATTERY_EMPTY_MV               3300U
#define BATTERY_FULL_MV                4160U
#define BATTERY_UI_LEVEL_MAX           10U
#define BATTERY_FILTER_OLD_WEIGHT      3U
#define BATTERY_FILTER_NEW_WEIGHT      1U
#define BATTERY_PERCENT_STEP           1U
#define BATTERY_PERCENT_RISE_INTERVAL  120U

static rt_uint8_t battery_percent_inited = 0;
static rt_uint32_t battery_filtered_mv = 0;
static rt_uint8_t battery_display_percent = 0;
static rt_uint16_t battery_rise_count = 0;

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

static rt_uint32_t battery_filter_voltage(rt_uint32_t voltage_mv)
{
    if (!battery_percent_inited)
    {
        battery_filtered_mv = voltage_mv;
        return battery_filtered_mv;
    }

    battery_filtered_mv = (battery_filtered_mv * BATTERY_FILTER_OLD_WEIGHT +
                           voltage_mv * BATTERY_FILTER_NEW_WEIGHT) /
                          (BATTERY_FILTER_OLD_WEIGHT + BATTERY_FILTER_NEW_WEIGHT);

    return battery_filtered_mv;
}

static rt_uint8_t battery_smooth_percent(rt_uint8_t target_percent)
{
    if (!battery_percent_inited)
    {
        battery_percent_inited = 1;
        battery_display_percent = target_percent;
        return battery_display_percent;
    }

    if (target_percent > battery_display_percent)
    {
        battery_rise_count++;
        if (battery_rise_count >= BATTERY_PERCENT_RISE_INTERVAL)
        {
            battery_rise_count = 0;
            battery_display_percent += BATTERY_PERCENT_STEP;
        }
        return battery_display_percent;
    }
    else if (battery_display_percent > target_percent + BATTERY_PERCENT_STEP)
    {
        battery_rise_count = 0;
        battery_display_percent -= BATTERY_PERCENT_STEP;
    }
    else
    {
        battery_rise_count = 0;
        battery_display_percent = target_percent;
    }

    return battery_display_percent;
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
    info->voltage_mv = battery_filter_voltage(battery_adc_raw_to_mv(raw_value));
    info->percent = battery_smooth_percent(battery_clamp_percent(info->voltage_mv));
    info->level = (rt_uint8_t)((info->percent * BATTERY_UI_LEVEL_MAX + 50U) / 100U);
    if (info->level > BATTERY_UI_LEVEL_MAX)
    {
        info->level = BATTERY_UI_LEVEL_MAX;
    }

    return RT_EOK;
}
