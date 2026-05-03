#include "rtthread.h"
#include "imu_sensor_power.h"
#include "lsm6ds3tr_c.h"

rt_err_t ImuSensorPower_Apply(rt_uint8_t enable_step, rt_uint8_t enable_imu)
{
    rt_err_t ret;
    lsm6ds3tr_power_mode_t mode;

    if(enable_imu)
    {
        mode = LSM6DS3TR_POWER_MODE_NORMAL;
    }
    else if(enable_step)
    {
        mode = LSM6DS3TR_POWER_MODE_STEP;
    }
    else
    {
        mode = LSM6DS3TR_POWER_MODE_OFF;
    }

    ret = lsm6ds3tr_SetPowerMode(mode);
    if(ret != RT_EOK)
    {
        rt_kprintf("LSM6DS3TR power mode switch failed: %d\n", (int)mode);
    }

    return ret;
}
