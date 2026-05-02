#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "sensor_i2c.h"
#include "BME280.h"
#include "BME280_regs.h"
#include "math.h"

/* Local altitude calibration: sensor 1013 hPa, actual 1011 hPa, altitude 58 m. */
#define BME280_ALTITUDE_PRESSURE_OFFSET_HPA     (-2.0f)
#define BME280_ALTITUDE_SEA_LEVEL_PRESSURE_HPA  (1017.98f)

bme280_calib_data_t bme280_calib;
static bme280_power_mode_t bme280_power_mode = BME280_POWER_MODE_OFF;
static rt_bool_t bme280_power_mode_ready = RT_FALSE;

void BME280_readid(void)
{
    rt_uint8_t id;
    if(sensor_i2c_readreg(BME280_I2C_ADDR_L, BME280_REG_ID, &id, 1) == RT_EOK)
    {
        if(id != BME280_CHIP_ID)
        {
            rt_kprintf("BME280 id error: 0x%02x\n", id);
        }
        else
        {
            rt_kprintf("BME280 id: 0x%02x\n", id);
            return;
        }
    }
    else
    {
        rt_kprintf("BME280 read id failed\n");
        return;
    }
}

void BME280_init(void)
{
    rt_uint8_t ctrl_value;
    rt_uint8_t bme280_calib_buf[26];
    rt_uint8_t bme280_calib_buf_h[7]; 

    sensor_i2c_init();
    BME280_readid();

    sensor_i2c_writereg(BME280_I2C_ADDR_L, BME280_REG_RESET, BME280_SOFT_RESET_CMD);//软复位
    rt_thread_mdelay(20); 

    ctrl_value = BME280_OVERSAMP_1X;//湿度过采样 x1
    sensor_i2c_writereg(BME280_I2C_ADDR_L, BME280_REG_CTRL_HUM, ctrl_value); 

    ctrl_value = (BME280_STANDBY_1000_MS << BME280_T_SB_POS) | (BME280_FILTER_COEFF_16 << BME280_FILTER_POS);//待机时间 0.5ms, IIR 滤波器系数 16
    sensor_i2c_writereg(BME280_I2C_ADDR_L, BME280_REG_CONFIG, ctrl_value);

    ctrl_value = (BME280_OVERSAMP_2X << BME280_OSRS_T_POS) | (BME280_OVERSAMP_16X << BME280_OSRS_P_POS) | BME280_MODE_SLEEP;//温度过采样 x2, 气压过采样 x16, 睡眠模式
    sensor_i2c_writereg(BME280_I2C_ADDR_L, BME280_REG_CTRL_MEAS, ctrl_value);

    sensor_i2c_readreg(BME280_I2C_ADDR_L, BME280_REG_CALIB_00, bme280_calib_buf, 26); // 读取 calib00...calib25
    bme280_calib.dig_T1 = (rt_uint16_t)(bme280_calib_buf[1] << 8) | bme280_calib_buf[0];
    bme280_calib.dig_T2 = (rt_int16_t)(bme280_calib_buf[3] << 8) | bme280_calib_buf[2];
    bme280_calib.dig_T3 = (rt_int16_t)(bme280_calib_buf[5] << 8) | bme280_calib_buf[4];
    
    bme280_calib.dig_P1 = (rt_uint16_t)(bme280_calib_buf[7] << 8) | bme280_calib_buf[6];
    bme280_calib.dig_P2 = (rt_int16_t)(bme280_calib_buf[9] << 8) | bme280_calib_buf[8];
    bme280_calib.dig_P3 = (rt_int16_t)(bme280_calib_buf[11] << 8) | bme280_calib_buf[10];
    bme280_calib.dig_P4 = (rt_int16_t)(bme280_calib_buf[13] << 8) | bme280_calib_buf[12];
    bme280_calib.dig_P5 = (rt_int16_t)(bme280_calib_buf[15] << 8) | bme280_calib_buf[14];
    bme280_calib.dig_P6 = (rt_int16_t)(bme280_calib_buf[17] << 8) | bme280_calib_buf[16];
    bme280_calib.dig_P7 = (rt_int16_t)(bme280_calib_buf[19] << 8) | bme280_calib_buf[18];
    bme280_calib.dig_P8 = (rt_int16_t)(bme280_calib_buf[21] << 8) | bme280_calib_buf[20];
    bme280_calib.dig_P9 = (rt_int16_t)(bme280_calib_buf[23] << 8) | bme280_calib_buf[22];
    bme280_calib.dig_H1 = bme280_calib_buf[25]; // 0xA1 [cite: 530]

    sensor_i2c_readreg(BME280_I2C_ADDR_L, BME280_REG_CALIB_26, bme280_calib_buf_h, 7); // 读取 calib26...calib32
    bme280_calib.dig_H2 = (rt_int16_t)(bme280_calib_buf_h[1] << 8) | bme280_calib_buf_h[0];
    bme280_calib.dig_H3 = bme280_calib_buf_h[2];
    bme280_calib.dig_H4 = (rt_int16_t)(bme280_calib_buf_h[3] << 4) | (bme280_calib_buf_h[4] & 0x0F);
    bme280_calib.dig_H5 = (rt_int16_t)(bme280_calib_buf_h[5] << 4) | (bme280_calib_buf_h[4] >> 4);
    bme280_calib.dig_H6 = (rt_int8_t)bme280_calib_buf_h[6]; // 0xE7 [cite: 530]
    BME280_SetPowerMode(BME280_POWER_MODE_OFF);

}

rt_err_t BME280_SetPowerMode(bme280_power_mode_t mode)
{
    rt_uint8_t ctrl_value;

    if(bme280_power_mode_ready && mode == bme280_power_mode)
    {
        return RT_EOK;
    }

    if(mode == BME280_POWER_MODE_ON)
    {
        ctrl_value = BME280_OVERSAMP_1X;
        if(sensor_i2c_writereg(BME280_I2C_ADDR_L, BME280_REG_CTRL_HUM, ctrl_value) != RT_EOK)
        {
            return -RT_ERROR;
        }

        ctrl_value = (BME280_STANDBY_1000_MS << BME280_T_SB_POS) | (BME280_FILTER_COEFF_16 << BME280_FILTER_POS);
        if(sensor_i2c_writereg(BME280_I2C_ADDR_L, BME280_REG_CONFIG, ctrl_value) != RT_EOK)
        {
            return -RT_ERROR;
        }

        ctrl_value = (BME280_OVERSAMP_2X << BME280_OSRS_T_POS) |
                     (BME280_OVERSAMP_16X << BME280_OSRS_P_POS) |
                     BME280_MODE_NORMAL;
    }
    else
    {
        mode = BME280_POWER_MODE_OFF;
        ctrl_value = (BME280_OVERSAMP_2X << BME280_OSRS_T_POS) |
                     (BME280_OVERSAMP_16X << BME280_OSRS_P_POS) |
                     BME280_MODE_SLEEP;
    }

    if(sensor_i2c_writereg(BME280_I2C_ADDR_L, BME280_REG_CTRL_MEAS, ctrl_value) != RT_EOK)
    {
        return -RT_ERROR;
    }

    if(mode == BME280_POWER_MODE_ON)
    {
        rt_thread_mdelay(50);
    }

    bme280_power_mode = mode;
    bme280_power_mode_ready = RT_TRUE;

    return RT_EOK;
}

/* 全局变量，用于将高精度温度值传递给气压和湿度补偿公式  */
rt_int32_t t_fine;

/*
 * 温度补偿函数 [cite: 553]
 * 返回温度值，分辨率为 0.01 摄氏度。输出值 "5123" 等于 51.23 ℃ [cite: 551]
 */
rt_int32_t bme280_compensate_T_int32(rt_int32_t adc_T)
{
    rt_int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((rt_int32_t)bme280_calib.dig_T1 << 1))) * ((rt_int32_t)bme280_calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((rt_int32_t)bme280_calib.dig_T1)) * ((adc_T >> 4) - ((rt_int32_t)bme280_calib.dig_T1))) >> 12) * ((rt_int32_t)bme280_calib.dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

/*
 * 气压补偿函数 (64位整数版本以保证最佳精度) [cite: 547, 568]
 * 返回无符号 32 位整数格式的气压值，采用 Q24.8 格式（24个整数位和8个小数位） [cite: 566]
 * 输出值 "24674867" 代表 24674867/256 = 96386.2 Pa = 963.862 hPa [cite: 567]
 */
rt_uint32_t bme280_compensate_P_int64(rt_int32_t adc_P)
{
    rt_int64_t var1, var2, p;
    
    var1 = ((rt_int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (rt_int64_t)bme280_calib.dig_P6;
    var2 = var2 + ((var1 * (rt_int64_t)bme280_calib.dig_P5) << 17);
    var2 = var2 + (((rt_int64_t)bme280_calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (rt_int64_t)bme280_calib.dig_P3) >> 8) + ((var1 * (rt_int64_t)bme280_calib.dig_P2) << 12);
    var1 = (((((rt_int64_t)1) << 47) + var1)) * ((rt_int64_t)bme280_calib.dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0; /* 避免除以零的异常 [cite: 582] */
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((rt_int64_t)bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((rt_int64_t)bme280_calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((rt_int64_t)bme280_calib.dig_P7) << 4);
    
    return (rt_uint32_t)p;
}

/*
 * 湿度补偿函数 [cite: 593]
 * 返回无符号 32 位整数格式的湿度值，采用 Q22.10 格式（22个整数位和10个小数位） [cite: 591]
 * 输出值 "47445" 代表 47445/1024 = 46.333 %RH [cite: 592]
 */
rt_uint32_t bme280_compensate_H_int32(rt_int32_t adc_H)
{
    rt_int32_t v_x1_u32r;
    
    v_x1_u32r = (t_fine - ((rt_int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((rt_int32_t)bme280_calib.dig_H4) << 20) - (((rt_int32_t)bme280_calib.dig_H5) * v_x1_u32r)) + ((rt_int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((rt_int32_t)bme280_calib.dig_H6)) >> 10) * (((v_x1_u32r * ((rt_int32_t)bme280_calib.dig_H3)) >> 11) + ((rt_int32_t)32768))) >> 10) + ((rt_int32_t)2097152)) * ((rt_int32_t)bme280_calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((rt_int32_t)bme280_calib.dig_H1)) >> 4));
    
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    
    return (rt_uint32_t)(v_x1_u32r >> 12);
}

static float BME280_calc_altitude(float press_hpa)
{
    float altitude_press_hpa = press_hpa + BME280_ALTITUDE_PRESSURE_OFFSET_HPA;

    if (altitude_press_hpa <= 0.0f)
    {
        return 0.0f;
    }

    return 44330.0f * (1.0f - powf(altitude_press_hpa / BME280_ALTITUDE_SEA_LEVEL_PRESSURE_HPA, 0.1903f));
}

rt_uint8_t BME280_getdata(float *out_temp, float *out_press, float *out_hum, float *out_altitude)
{
    rt_uint8_t buf[8];
    rt_int32_t raw_T, raw_P, raw_H;
    if(sensor_i2c_readreg(BME280_I2C_ADDR_L, BME280_REG_PRESS_MSB, buf, 8) == RT_EOK)
    {
        raw_P = ((rt_int32_t)buf[0] << 12) | ((rt_int32_t)buf[1] << 4) | (buf[2] >> 4);
        raw_T = ((rt_int32_t)buf[3] << 12) | ((rt_int32_t)buf[4] << 4) | (buf[5] >> 4);
        raw_H = ((rt_int32_t)buf[6] << 8) | buf[7];

        rt_int32_t comp_temp   = bme280_compensate_T_int32(raw_T);
        rt_uint32_t comp_press = bme280_compensate_P_int64(raw_P);
        rt_uint32_t comp_hum   = bme280_compensate_H_int32(raw_H);

        /* 转换为真实物理单位的浮点数:
         * 1. 温度: 除以 100.0f 得到 ℃ 
         * 2. 气压: 除以 256.0f 得到 Pa，再除以 100.0f 得到 hPa
         * 3. 湿度: 除以 1024.0f 得到 %RH
         */
        float temp_f  = (float)comp_temp / 100.0f;
        float press_f = (float)comp_press / 256.0f / 100.0f;
        float hum_f   = (float)comp_hum / 1024.0f;
        float altitude_f = BME280_calc_altitude(press_f);

        *out_temp = temp_f;
        *out_press = press_f;
        *out_hum = hum_f;
        *out_altitude = altitude_f;

        rt_kprintf("  Temp  = %.2f degC\n", temp_f);
        rt_kprintf("  Press = %.2f hPa\n", press_f);
        rt_kprintf("  Hum   = %.2f %%RH\n", hum_f);
        rt_kprintf("  Alt   = %.2f m\n", altitude_f);

        return 1;
    }
    else
    {
        rt_kprintf("BME280 get data failed\n");
        return 0;
    }
}


