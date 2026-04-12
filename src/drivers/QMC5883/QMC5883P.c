#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "sensor_i2c.h"
#include "QMC5883P.h"
#include "QMC3883P_regs.h"

#ifndef QMC5883P_RUNTIME_LOG
#define QMC5883P_RUNTIME_LOG 0
#endif

#if QMC5883P_RUNTIME_LOG
#define QMC_LOG(...) rt_kprintf(__VA_ARGS__)
#else
#define QMC_LOG(...) do { } while (0)
#endif

void QMC5883P_readid(void)
{
    rt_uint8_t id;
    if(sensor_i2c_readreg(QMC5883P_I2C_ADDR, QMC5883P_REG_CHIPID, &id, 1) == RT_EOK)
    {
        if(id != 0X80)
        {
            rt_kprintf("QMC5883P id error: 0x%02x\n", id);
        }
        else
        {
            rt_kprintf("QMC5883P id: 0x%02x\n", id);
            return;
        }
    }
    else
    {
        rt_kprintf("QMC5883P read id failed\n");
        return;
    }
}

void QMC5883P_init(void)
{
    rt_uint8_t ctrl_value;

    sensor_i2c_init();
    QMC5883P_readid();

    sensor_i2c_writereg(QMC5883P_I2C_ADDR, QMC5883P_REG_CTRL2, QMC5883P_SOFT_RST_EN); // 软复位
    rt_thread_mdelay(20); // 复位后等待100ms

    sensor_i2c_writereg(QMC5883P_I2C_ADDR, 0x29, 0x06);//定义 X Y Z 轴的符号

    ctrl_value = QMC5883P_SET_RESET_ON | QMC5883P_RNG_8G;//Set/Reset 开启，量程 ±8G
    sensor_i2c_writereg(QMC5883P_I2C_ADDR, QMC5883P_REG_CTRL2, ctrl_value);

    ctrl_value = QMC5883P_MODE_CONT | QMC5883P_ODR_100HZ | QMC5883P_OSR1_8 | QMC5883P_OSR2_1; // 连续测量模式，ODR 100Hz，过采样率 8， 下采样率 1
    sensor_i2c_writereg(QMC5883P_I2C_ADDR, QMC5883P_REG_CTRL1, ctrl_value);

}

rt_uint8_t QMC5883P_getdata(rt_int16_t *out_x, rt_int16_t *out_y, rt_int16_t *out_z)
{
    rt_uint8_t buf[6];
    rt_int16_t x, y, z;
    if(sensor_i2c_readreg(QMC5883P_I2C_ADDR, QMC5883P_REG_XOUT_LSB, buf, 6) == RT_EOK)
    {
        x = (rt_int16_t)(buf[1] << 8 | buf[0]);
        y = (rt_int16_t)(buf[3] << 8 | buf[2]);
        z = (rt_int16_t)(buf[5] << 8 | buf[4]);

        *out_x = x; // 量程 ±8G 时，分辨率约为 0.3mG/LSB
        *out_y = y;
        *out_z = z;

        QMC_LOG("QMC5883P data: x=%d, y=%d, z=%d\n", x, y, z);
        return 1;
    }
    else
    {
        rt_kprintf("QMC5883P get data failed\n");
        return 0;
    }
}
