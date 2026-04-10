#ifndef __DATA_HUB_H__
#define __DATA_HUB_H__

#include "rtthread.h"

// 定义每一天的数据结构
typedef struct {
    rt_uint8_t weather_type; // 天气类型（0-晴天，1-多云，2-阴天，3-雨天，4-雷雨等）
    rt_int8_t  max_temp;
    rt_int8_t  min_temp;
} daily_weather_t;

// 定义逐小时数据结构
typedef struct {
    rt_uint8_t hour;       
    rt_uint8_t weather_type;       
    rt_int8_t  temp;       
} hourly_weather_t;

typedef struct 
{
    rt_int32_t heart_rate;
    rt_int32_t spo2;

    float temperature;
    float pressure;
    float humidity;

    rt_int16_t mag_x;
    rt_int16_t mag_y;
    rt_int16_t mag_z;

    float acc_x;
    float acc_y;
    float acc_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;

    rt_uint8_t enable_heartrate;
    rt_uint8_t enable_environment;
    rt_uint8_t enable_mag;
    rt_uint8_t enable_imu;
    rt_uint8_t enable_rtc;
    rt_uint8_t enable_weather;
    rt_uint8_t enable_forecastweather; 
    rt_uint8_t enable_hourlyweather;

    rt_uint16_t year;
    rt_uint8_t month;
    rt_uint8_t day;
    rt_uint8_t hour;
    rt_uint8_t minute;
    rt_uint8_t second;
    rt_uint8_t weekday;

    char city_name[16]; 
    rt_uint8_t weather_type; 
    rt_int8_t current_temp; 
    rt_int8_t max_temp; 
    rt_int8_t min_temp; 
    rt_uint16_t aqi;  
    
    daily_weather_t forecast[7];

    hourly_weather_t hourly_forecast[24];

} systemdata_t;

extern systemdata_t system_data;
extern rt_mutex_t data_mutex;

void DataHub_Init(void);

#endif /* __DATA_HUB_H__ */