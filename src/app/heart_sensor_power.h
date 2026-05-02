#ifndef __HEART_SENSOR_POWER_H__
#define __HEART_SENSOR_POWER_H__

#include "rtthread.h"
#include "data_hub.h"

typedef enum
{
    HEART_SENSOR_POWER_OFF = 0,
    HEART_SENSOR_POWER_LOW,
    HEART_SENSOR_POWER_NORMAL
} heart_sensor_power_mode_t;

void HeartSensorPower_SetRequest(systemdata_t *data, heart_sensor_power_mode_t mode);
heart_sensor_power_mode_t HeartSensorPower_NormalizeMode(rt_uint8_t mode);
rt_err_t HeartSensorPower_ApplyMode(heart_sensor_power_mode_t mode);

#endif /* __HEART_SENSOR_POWER_H__ */
