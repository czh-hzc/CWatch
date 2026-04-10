#ifndef __SENSOR_I2C_H__
#define __SENSOR_I2C_H__

#include "rtthread.h"

extern struct rt_i2c_bus_device *sensor_i2c_bus;
rt_err_t sensor_i2c_init(void);
rt_err_t sensor_i2c_readreg(rt_uint16_t slave_addr, rt_uint8_t reg, rt_uint8_t *buf, rt_uint8_t len);
rt_err_t sensor_i2c_writereg(rt_uint16_t slave_addr, rt_uint8_t reg, rt_uint8_t data);

#endif /* __SENSOR_I2C_H__ */