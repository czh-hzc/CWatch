#include "rtthread.h"
#include "heart_sensor_power.h"
#include "MAX30102.h"

void HeartSensorPower_SetRequest(systemdata_t *data, heart_sensor_power_mode_t mode)
{
    if(data == RT_NULL)
    {
        return;
    }

    mode = HeartSensorPower_NormalizeMode((rt_uint8_t)mode);
    data->heartrate_power_mode = (rt_uint8_t)mode;
    data->enable_heartrate = (mode != HEART_SENSOR_POWER_OFF) ? 1 : 0;
}

heart_sensor_power_mode_t HeartSensorPower_NormalizeMode(rt_uint8_t mode)
{
    if(mode == HEART_SENSOR_POWER_LOW)
    {
        return HEART_SENSOR_POWER_LOW;
    }
    if(mode == HEART_SENSOR_POWER_NORMAL)
    {
        return HEART_SENSOR_POWER_NORMAL;
    }

    return HEART_SENSOR_POWER_OFF;
}

rt_err_t HeartSensorPower_ApplyMode(heart_sensor_power_mode_t mode)
{
    rt_err_t ret;

    mode = HeartSensorPower_NormalizeMode((rt_uint8_t)mode);

    switch(mode)
    {
    case HEART_SENSOR_POWER_LOW:
        ret = MAX30102_SetPowerMode(MAX30102_POWER_MODE_LOW);
        break;

    case HEART_SENSOR_POWER_NORMAL:
        ret = MAX30102_SetPowerMode(MAX30102_POWER_MODE_NORMAL);
        break;

    case HEART_SENSOR_POWER_OFF:
    default:
        ret = MAX30102_SetPowerMode(MAX30102_POWER_MODE_OFF);
        break;
    }

    if(ret != RT_EOK)
    {
        rt_kprintf("MAX30102 power mode switch failed: %d\n", (int)mode);
    }

    return ret;
}
