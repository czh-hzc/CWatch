#ifndef __IMU_SENSOR_POWER_H__
#define __IMU_SENSOR_POWER_H__

#include "rtthread.h"

rt_err_t ImuSensorPower_Apply(rt_uint8_t enable_step, rt_uint8_t enable_imu);

#endif /* __IMU_SENSOR_POWER_H__ */
