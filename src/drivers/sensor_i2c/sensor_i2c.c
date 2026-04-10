#include "rtthread.h"
#include "board.h"
#include "sensor_i2c.h"

#define SENSOR_I2C_BUS_NAME "i2c3"

struct rt_i2c_bus_device *sensor_i2c_bus = RT_NULL;

rt_err_t sensor_i2c_init(void)
{
    HAL_PIN_Set(PAD_PA44, I2C3_SDA, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA41, I2C3_SCL, PIN_PULLUP, 1);

    sensor_i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(SENSOR_I2C_BUS_NAME);
    if (sensor_i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find %s device\n", SENSOR_I2C_BUS_NAME);
        return -RT_ERROR;
    }
    
    rt_kprintf("find %s device successfully\n", SENSOR_I2C_BUS_NAME);
    return RT_EOK;
}

rt_err_t sensor_i2c_readreg(rt_uint16_t slave_addr, rt_uint8_t reg, rt_uint8_t *buf, rt_uint8_t len)
{
    if (rt_i2c_master_send(sensor_i2c_bus, slave_addr, RT_I2C_WR, &reg, 1) == 1)
    {
        if (rt_i2c_master_recv(sensor_i2c_bus, slave_addr, RT_I2C_RD, buf, len) == len)
        {
            return RT_EOK;
        }
    }
    
    rt_kprintf("sensor i2c read failed (addr: 0x%02X, reg: 0x%02X)\n", slave_addr, reg);
    return -RT_ERROR;
}

rt_err_t sensor_i2c_writereg(rt_uint16_t slave_addr, rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;

    if (rt_i2c_master_send(sensor_i2c_bus, slave_addr, RT_I2C_WR, buf, 2) == 2)
    {
        return RT_EOK;
    }
    
    rt_kprintf("sensor i2c write failed (addr: 0x%02X, reg: 0x%02X)\n", slave_addr, reg);
    return -RT_ERROR;
}