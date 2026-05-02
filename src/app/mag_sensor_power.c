#include "rtthread.h"
#include "mag_sensor_power.h"
#include "QMC5883P.h"

rt_err_t MagSensorPower_Apply(rt_uint8_t enable)
{
    rt_err_t ret;
    qmc5883p_power_mode_t mode;

    mode = enable ? QMC5883P_POWER_MODE_ON : QMC5883P_POWER_MODE_OFF;
    ret = QMC5883P_SetPowerMode(mode);
    if(ret != RT_EOK)
    {
        rt_kprintf("QMC5883P power mode switch failed: %d\n", (int)mode);
    }

    return ret;
}
