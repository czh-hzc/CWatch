#ifndef __MAX30102_H__
#define __MAX30102_H__

#include "rtthread.h"

typedef enum
{
    MAX30102_POWER_MODE_OFF = 0,
    MAX30102_POWER_MODE_LOW,
    MAX30102_POWER_MODE_NORMAL
} max30102_power_mode_t;

void MAX30102_readid(void);
void MAX30102_init(void);
rt_err_t MAX30102_SetPowerMode(max30102_power_mode_t mode);
rt_uint8_t MAX30102_getdata(rt_int32_t *out_heart_rate, rt_int32_t *out_spo2);
void MAX30102_Clear_Buffer(void);

#endif /*__MAX30102_H__*/
