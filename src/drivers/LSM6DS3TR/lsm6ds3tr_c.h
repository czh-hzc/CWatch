#ifndef __LSM6DS3TR_C_H__
#define __LSM6DS3TR_C_H__

#include "rtthread.h"

void lsm6ds3tr_readid(void);
void lsm6ds3tr_init(void);
rt_uint8_t lsm6ds3tr_getdata(float *out_gx, float *out_gy, float *out_gz, float *out_ax, float *out_ay, float *out_az);


#endif /* __LSM6DS3TR_C_H__ */