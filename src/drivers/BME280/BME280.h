#ifndef __BME280_H__
#define __BME280_H__

#include "rtthread.h"

/* BME280 校准参数结构体 */
typedef struct {
    rt_uint16_t dig_T1;
    rt_int16_t  dig_T2;
    rt_int16_t  dig_T3;
    
    rt_uint16_t dig_P1;
    rt_int16_t  dig_P2;
    rt_int16_t  dig_P3;
    rt_int16_t  dig_P4;
    rt_int16_t  dig_P5;
    rt_int16_t  dig_P6;
    rt_int16_t  dig_P7;
    rt_int16_t  dig_P8;
    rt_int16_t  dig_P9;
    
    rt_uint8_t  dig_H1;
    rt_int16_t  dig_H2;
    rt_uint8_t  dig_H3;
    rt_int16_t  dig_H4;
    rt_int16_t  dig_H5;
    rt_int8_t   dig_H6;
} bme280_calib_data_t;

void BME280_readid(void);
void BME280_init(void);
rt_uint8_t BME280_getdata(float *out_temp, float *out_press, float *out_hum, float *out_altitude);

#endif /*__BME280_H__*/
