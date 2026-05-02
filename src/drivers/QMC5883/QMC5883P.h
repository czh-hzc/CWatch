#ifndef __QMC5883P_H__
#define __QMC5883P_H__

#include "rtthread.h"

typedef enum
{
    QMC5883P_POWER_MODE_OFF = 0,
    QMC5883P_POWER_MODE_ON
} qmc5883p_power_mode_t;

void QMC5883P_readid(void);
void QMC5883P_init(void);
rt_err_t QMC5883P_SetPowerMode(qmc5883p_power_mode_t mode);
rt_uint8_t QMC5883P_getdata(rt_int16_t *out_x, rt_int16_t *out_y, rt_int16_t *out_z);

#endif /*__QMC5883P_H__*/
