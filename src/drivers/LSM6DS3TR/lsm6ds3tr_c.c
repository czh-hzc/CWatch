#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "sensor_i2c.h"
#include "lsm6ds3tr_c.h"
#include "lsm6ds3tr_c_regs.h"

#ifndef SENSOR_RUNTIME_LOG
#define SENSOR_RUNTIME_LOG 0
#endif

#if SENSOR_RUNTIME_LOG
#define SENSOR_LOG(...) rt_kprintf(__VA_ARGS__)
#else
#define SENSOR_LOG(...) do { } while (0)
#endif

static lsm6ds3tr_power_mode_t lsm6ds3tr_power_mode = LSM6DS3TR_POWER_MODE_OFF;
static rt_bool_t lsm6ds3tr_power_mode_ready = RT_FALSE;

static rt_err_t lsm6ds3tr_set_pedometer(rt_bool_t enable)
{
    rt_uint8_t ctrl10;

    if(sensor_i2c_readreg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL10_C, &ctrl10, 1) != RT_EOK)
    {
        return -RT_ERROR;
    }

    if(enable)
    {
        ctrl10 |= LSM6DS3TR_C_FUNC_EN | LSM6DS3TR_C_PEDO_EN;
        ctrl10 &= (rt_uint8_t)(~LSM6DS3TR_C_PEDO_RST_STEP);
    }
    else
    {
        ctrl10 &= (rt_uint8_t)(~(LSM6DS3TR_C_PEDO_EN | LSM6DS3TR_C_PEDO_RST_STEP));
        ctrl10 &= (rt_uint8_t)(~LSM6DS3TR_C_FUNC_EN);
    }

    return sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL10_C, ctrl10);
}

void lsm6ds3tr_readid(void)
{
    rt_uint8_t id;
    if(sensor_i2c_readreg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_WHO_AM_I, &id, 1) == RT_EOK)
    {
        if(id != LSM6DS3TR_C_WHO_AM_I_ID)
        {
            rt_kprintf("lsm6ds3tr id error: 0x%02x\n", id);
        }
        else
        {
            rt_kprintf("lsm6ds3tr id: 0x%02x\n", id);
            return;
        }
    }
    else
    {
        rt_kprintf("lsm6ds3tr read id failed\n");
        return;
    }
}

void lsm6ds3tr_init(void)
{
    rt_uint8_t ctrl_value;

    sensor_i2c_init();
    lsm6ds3tr_readid();

    sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL3_C, 0x01); // 软复位
    rt_thread_mdelay(20); // 复位后等待 20ms

    ctrl_value = LSM6DS3TR_C_BDU | LSM6DS3TR_C_IF_INC;//开启数据防撕裂锁存(BDU) 和 寄存器地址连续读取自动递增(IF_INC)
    sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL3_C, ctrl_value);

    ctrl_value = LSM6DS3TR_C_XL_ODR_104HZ | LSM6DS3TR_C_XL_FS_2G;//加速度计104Hz 采样率, ±2g 量程, 默认带宽
    sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL1_XL, ctrl_value);

    ctrl_value = LSM6DS3TR_C_G_ODR_104HZ | LSM6DS3TR_C_G_FS_2000DPS;//陀螺仪104Hz 采样率, ±2000 dps 量程
    sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL2_G, ctrl_value);
    lsm6ds3tr_SetPowerMode(LSM6DS3TR_POWER_MODE_OFF);
}

rt_err_t lsm6ds3tr_SetPowerMode(lsm6ds3tr_power_mode_t mode)
{
    rt_uint8_t ctrl1_xl;
    rt_uint8_t ctrl2_g;
    rt_bool_t enable_pedometer = RT_FALSE;

    if(lsm6ds3tr_power_mode_ready && mode == lsm6ds3tr_power_mode)
    {
        return RT_EOK;
    }

    switch(mode)
    {
    case LSM6DS3TR_POWER_MODE_NORMAL:
        ctrl1_xl = LSM6DS3TR_C_XL_ODR_104HZ | LSM6DS3TR_C_XL_FS_2G;
        ctrl2_g = LSM6DS3TR_C_G_ODR_104HZ | LSM6DS3TR_C_G_FS_2000DPS;
        break;

    case LSM6DS3TR_POWER_MODE_STEP:
        ctrl1_xl = LSM6DS3TR_C_XL_ODR_26HZ | LSM6DS3TR_C_XL_FS_2G;
        ctrl2_g = LSM6DS3TR_C_G_ODR_POWER_DOWN | LSM6DS3TR_C_G_FS_2000DPS;
        enable_pedometer = RT_TRUE;
        break;

    case LSM6DS3TR_POWER_MODE_OFF:
    default:
        mode = LSM6DS3TR_POWER_MODE_OFF;
        ctrl1_xl = LSM6DS3TR_C_XL_ODR_POWER_DOWN | LSM6DS3TR_C_XL_FS_2G;
        ctrl2_g = LSM6DS3TR_C_G_ODR_POWER_DOWN | LSM6DS3TR_C_G_FS_2000DPS;
        break;
    }

    if(sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL1_XL, ctrl1_xl) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL2_G, ctrl2_g) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(lsm6ds3tr_set_pedometer(enable_pedometer) != RT_EOK)
    {
        return -RT_ERROR;
    }

    if(mode != LSM6DS3TR_POWER_MODE_OFF)
    {
        rt_thread_mdelay(20);
    }

    lsm6ds3tr_power_mode = mode;
    lsm6ds3tr_power_mode_ready = RT_TRUE;

    return RT_EOK;
}

rt_uint8_t lsm6ds3tr_getdata(float *out_gx, float *out_gy, float *out_gz, float *out_ax, float *out_ay, float *out_az)
{
    rt_uint8_t buf[14];
    rt_int16_t raw_gyro_x, raw_gyro_y, raw_gyro_z;
    rt_int16_t raw_acc_x, raw_acc_y, raw_acc_z;
    rt_int16_t raw_temp;
    float gyro_x, gyro_y, gyro_z;
    float acc_x, acc_y, acc_z;
    float temp;

    if(sensor_i2c_readreg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_OUT_TEMP_L, buf, 14) == RT_EOK)
    {
        raw_temp = (rt_int16_t)(buf[1] << 8 | buf[0]);
        raw_gyro_x = (rt_int16_t)(buf[3] << 8 | buf[2]);
        raw_gyro_y = (rt_int16_t)(buf[5] << 8 | buf[4]);
        raw_gyro_z = (rt_int16_t)(buf[7] << 8 | buf[6]);
        raw_acc_x = (rt_int16_t)(buf[9] << 8 | buf[8]);
        raw_acc_y = (rt_int16_t)(buf[11] << 8 | buf[10]);
        raw_acc_z = (rt_int16_t)(buf[13] << 8 | buf[12]);

        temp = raw_temp / 256.0f + 25.0f; // 转换为摄氏度

        gyro_x = raw_gyro_x * 70.0f / 1000.0f; // 转换为 dps
        gyro_y = raw_gyro_y * 70.0f / 1000.0f;
        gyro_z = raw_gyro_z * 70.0f / 1000.0f;

        acc_x = raw_acc_x * 0.061f; // 转换为 mg
        acc_y = raw_acc_y * 0.061f;
        acc_z = raw_acc_z * 0.061f;

        *out_gx = gyro_x;
        *out_gy = gyro_y;
        *out_gz = gyro_z;
        *out_ax = acc_x;
        *out_ay = acc_y;
        *out_az = acc_z;

        //rt_kprintf("raw gyro: (%d, %d, %d), raw acc: (%d, %d, %d)\n", raw_gyro_x, raw_gyro_y, raw_gyro_z, raw_acc_x, raw_acc_y, raw_acc_z);
        SENSOR_LOG("temp: %.2f C, tgyro: (%.2f, %.2f, %.2f) dps, acc: (%.2f, %.2f, %.2f) mg\n", temp, gyro_x, gyro_y, gyro_z, acc_x, acc_y, acc_z);

        return 1;
    }
    else 
    {
        rt_kprintf("lsm6ds3tr getdata failed\n");
        return 0;
    }
    
}
