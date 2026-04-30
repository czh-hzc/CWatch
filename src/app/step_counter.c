#include "step_counter.h"

#include "lsm6ds3tr_c_regs.h"
#include "sensor_i2c.h"

#define STEP_COUNTER_HW_RANGE 65536U

static rt_uint8_t step_counter_inited = 0;
static rt_uint8_t step_counter_has_last = 0;
static rt_uint16_t step_counter_last_raw = 0;
static rt_uint32_t step_counter_total = 0;

rt_err_t StepCounter_Init(void)
{
    rt_uint8_t ctrl10;
    rt_err_t ret;

    if (sensor_i2c_bus == RT_NULL)
    {
        return -RT_ERROR;
    }

    ret = sensor_i2c_readreg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL10_C, &ctrl10, 1);
    if (ret != RT_EOK)
    {
        return ret;
    }

    ctrl10 |= LSM6DS3TR_C_FUNC_EN | LSM6DS3TR_C_PEDO_EN | LSM6DS3TR_C_PEDO_RST_STEP;
    ret = sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL10_C, ctrl10);
    if (ret != RT_EOK)
    {
        return ret;
    }

    rt_thread_mdelay(10);

    ctrl10 &= (rt_uint8_t)(~LSM6DS3TR_C_PEDO_RST_STEP);
    ret = sensor_i2c_writereg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_CTRL10_C, ctrl10);
    if (ret != RT_EOK)
    {
        return ret;
    }

    step_counter_inited = 1;
    step_counter_has_last = 0;
    step_counter_last_raw = 0;
    step_counter_total = 0;

    return RT_EOK;
}

rt_err_t StepCounter_Read(rt_uint32_t *step_count)
{
    rt_uint8_t buf[2];
    rt_uint16_t raw_count;
    rt_uint32_t diff;
    rt_err_t ret;

    if (step_count == RT_NULL)
    {
        return -RT_EINVAL;
    }

    if (sensor_i2c_bus == RT_NULL)
    {
        return -RT_ERROR;
    }

    if (!step_counter_inited)
    {
        ret = StepCounter_Init();
        if (ret != RT_EOK)
        {
            return ret;
        }
    }

    ret = sensor_i2c_readreg(LSM6DS3TR_C_I2C_ADDR_L, LSM6DS3TR_C_STEP_COUNTER_L, buf, sizeof(buf));
    if (ret != RT_EOK)
    {
        return ret;
    }

    raw_count = (rt_uint16_t)(((rt_uint16_t)buf[1] << 8) | buf[0]);

    if (!step_counter_has_last)
    {
        step_counter_total = raw_count;
        step_counter_has_last = 1;
    }
    else
    {
        if (raw_count >= step_counter_last_raw)
        {
            diff = raw_count - step_counter_last_raw;
        }
        else
        {
            diff = STEP_COUNTER_HW_RANGE - step_counter_last_raw + raw_count;
        }

        step_counter_total += diff;
    }

    step_counter_last_raw = raw_count;
    *step_count = step_counter_total;

    return RT_EOK;
}
