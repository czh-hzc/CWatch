#ifndef __LSM6DS3TR_C_H__
#define __LSM6DS3TR_C_H__

#include "rtthread.h"

typedef enum
{
    LSM6DS3TR_POWER_MODE_OFF = 0,
    LSM6DS3TR_POWER_MODE_STEP,
    LSM6DS3TR_POWER_MODE_NORMAL
} lsm6ds3tr_power_mode_t;

void lsm6ds3tr_readid(void);
void lsm6ds3tr_init(void);
rt_err_t lsm6ds3tr_SetPowerMode(lsm6ds3tr_power_mode_t mode);
rt_uint8_t lsm6ds3tr_getdata(float *out_gx, float *out_gy, float *out_gz, float *out_ax, float *out_ay, float *out_az);


#endif /* __LSM6DS3TR_C_H__ */
