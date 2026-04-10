#ifndef __QMC5883P_H__
#define __QMC5883P_H__

#include "rtthread.h"

void QMC5883P_readid(void);
void QMC5883P_init(void);
rt_uint8_t QMC5883P_getdata(rt_int16_t *out_x, rt_int16_t *out_y, rt_int16_t *out_z);

#endif /*__QMC5883P_H__*/