#ifndef __RTC_T_H
#define __RTC_T_H

#include "rtthread.h"

rt_uint8_t RTC_Init(void);
void RTC_SetTime(rt_uint16_t year, rt_uint8_t month, rt_uint8_t day, rt_uint8_t hour, rt_uint8_t minute, rt_uint8_t second);
void RTC_GetTime(rt_uint16_t *year, rt_uint8_t *month, rt_uint8_t *day, rt_uint8_t *hour, rt_uint8_t *minute, rt_uint8_t *second, rt_uint8_t *weekday);
void RTC_SetAlarm(rt_uint16_t year, rt_uint8_t month, rt_uint8_t day,rt_uint8_t hour, rt_uint8_t minute, rt_uint8_t second);

#endif