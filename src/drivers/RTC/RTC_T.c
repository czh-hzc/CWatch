#include <rtthread.h>
#include <board.h>
#include "time.h"

#define RTC_NAME "rtc"

rt_device_t rtc_dev;
rt_err_t res;

rt_uint8_t RTC_Init(void)
{
    rtc_dev = rt_device_find(RTC_NAME);
    if(rtc_dev == RT_NULL)
    {
        rt_kprintf("RTC device not found!\n");
        return RT_ERROR;
    }

    res = rt_device_open(rtc_dev, RT_DEVICE_OFLAG_RDWR);
    if(res != RT_EOK)
    {
        rt_kprintf("Failed to open RTC device! Error code: %d\n", res);
        return RT_ERROR;
    }
    return RT_EOK;

}


void RTC_SetTime(rt_uint16_t year, rt_uint8_t month, rt_uint8_t day, rt_uint8_t hour, rt_uint8_t minute, rt_uint8_t second)
{
    res = set_date(year, month, day);
    if(res != RT_EOK)
    {
        rt_kprintf("Failed to set RTC date! Error code: %d\n", res);
        return;
    }
    res = set_time(hour, minute, second);
    if(res != RT_EOK)
    {
        rt_kprintf("Failed to set RTC time! Error code: %d\n", res);
        return;
    }
}

void RTC_GetTime(rt_uint16_t *year, rt_uint8_t *month, rt_uint8_t *day, rt_uint8_t *hour, rt_uint8_t *minute, rt_uint8_t *second, rt_uint8_t *weekday)
{
    time_t time_now;
    time_now = time(RT_NULL);
    struct tm *tm_info = localtime(&time_now);
    *year = tm_info->tm_year + 1900;
    *month = tm_info->tm_mon + 1;
    *day = tm_info->tm_mday;
    *hour = tm_info->tm_hour;
    *minute = tm_info->tm_min;
    *second = tm_info->tm_sec;
    *weekday = tm_info->tm_wday;

}

void user_alarm_callback(rt_alarm_t alarm, time_t timestamp)
{
    rt_kprintf("user alarm callback function.\n");
}

void RTC_SetAlarm(rt_uint16_t year, rt_uint8_t month, rt_uint8_t day,
                  rt_uint8_t hour, rt_uint8_t minute, rt_uint8_t second)
{
    struct rt_alarm_setup Alarm_Set;

    Alarm_Set.flag = RT_ALARM_ONESHOT;
    Alarm_Set.wktime.tm_year = year -1900;
    Alarm_Set.wktime.tm_mon  = month - 1;
    Alarm_Set.wktime.tm_mday = day;
    Alarm_Set.wktime.tm_hour = hour;
    Alarm_Set.wktime.tm_min  = minute;
    Alarm_Set.wktime.tm_sec  = second;
    
    rt_alarm_t Alarm;

    Alarm = rt_alarm_create(user_alarm_callback, &Alarm_Set);
    if(Alarm == RT_NULL)
    {
        rt_kprintf("Failed to create alarm!\n");
        return;
    }

    rt_alarm_start(Alarm);

}