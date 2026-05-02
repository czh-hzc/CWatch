#include "rtthread.h"
#include "env_sensor_power.h"
#include "BME280.h"

rt_err_t EnvSensorPower_Apply(rt_uint8_t enable)
{
    rt_err_t ret;
    bme280_power_mode_t mode;

    mode = enable ? BME280_POWER_MODE_ON : BME280_POWER_MODE_OFF;
    ret = BME280_SetPowerMode(mode);
    if(ret != RT_EOK)
    {
        rt_kprintf("BME280 power mode switch failed: %d\n", (int)mode);
    }

    return ret;
}
