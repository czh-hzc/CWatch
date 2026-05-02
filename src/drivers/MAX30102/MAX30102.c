#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "sensor_i2c.h"
#include "MAX30102.h"
#include "MAX30102_regs.h"
#include "MAX30102_al.h"

#define MAX30102_NORMAL_HR_DIVIDER       4
#define MAX30102_LOW_POWER_HR_DIVIDER    16

static max30102_power_mode_t max30102_power_mode = MAX30102_POWER_MODE_OFF;
static rt_bool_t max30102_power_mode_ready = RT_FALSE;
static rt_uint8_t max30102_hr_divider = MAX30102_NORMAL_HR_DIVIDER;

static rt_err_t max30102_write_reg(rt_uint8_t reg, rt_uint8_t value)
{
    return sensor_i2c_writereg(MAX30102_I2C_ADDR, reg, value);
}

static rt_err_t max30102_clear_fifo(void)
{
    if(max30102_write_reg(MAX30102_REG_FIFO_WR_PTR, 0x00) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_FIFO_RD_PTR, 0x00) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_OVF_COUNTER, 0x00) != RT_EOK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t max30102_config_measurement(rt_uint8_t fifo_config,
                                            rt_uint8_t spo2_config,
                                            rt_uint8_t led1_pa,
                                            rt_uint8_t led2_pa,
                                            rt_uint8_t hr_divider)
{
    if(max30102_write_reg(MAX30102_REG_MODE_CONFIG, MAX30102_MODE_SHDN) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_FIFO_CONFIG, fifo_config) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_SPO2_CONFIG, spo2_config) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_LED1_PA, led1_pa) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_LED2_PA, led2_pa) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_clear_fifo() != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_INT_ENABLE1, MAX30102_INT_A_FULL | MAX30102_INT_PPG_RDY) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_INT_ENABLE2, 0x00) != RT_EOK)
    {
        return -RT_ERROR;
    }
    if(max30102_write_reg(MAX30102_REG_MODE_CONFIG, MAX30102_MODE_SPO2) != RT_EOK)
    {
        return -RT_ERROR;
    }

    max30102_hr_divider = hr_divider;
    MAX30102_Clear_Buffer();

    return RT_EOK;
}

void MAX30102_readid(void)
{
    rt_uint8_t id;
    if(sensor_i2c_readreg(MAX30102_I2C_ADDR, MAX30102_REG_PART_ID, &id, 1) == RT_EOK)
    {
        if(id != MAX30102_EXPECTED_PART_ID)
        {
            rt_kprintf("MAX30102 id error: 0x%02x\n", id);
            return;
        }
        else
        {
            rt_kprintf("MAX30102 id: 0x%02x\n", id);
        }
    }
    else
    {
        rt_kprintf("MAX30102 read id failed\n");
        return;
    }
}

void MAX30102_init(void)
{
    rt_uint8_t ctrl_value;

    sensor_i2c_init();
    MAX30102_readid();

    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_MODE_CONFIG, MAX30102_MODE_RESET);
    rt_thread_mdelay(20); 

    ctrl_value = MAX30102_SMP_AVE_4 | MAX30102_FIFO_ROLLOVER_EN | 0x0f; // 采样平均 4 个样本，FIFO 满覆盖使能，FIFO 阈值 15
    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_CONFIG, ctrl_value);

    ctrl_value = MAX30102_SPO2_ADC_RGE_4096 | MAX30102_SPO2_SR_100 | MAX30102_LED_PW_411US_18BITS;// SpO2 ADC 范围 4096 nA，采样率 100 sps，LED 脉冲宽度 411 us，ADC 分辨率 18 bits
    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_SPO2_CONFIG, ctrl_value);

    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_LED1_PA, 0x40);// LED1 的脉冲幅度设置为 64 (0x40)，根据实际需求调整
    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_LED2_PA, 0x1F);// LED2 的脉冲幅度设置为 31 (0x1F)，根据实际需求调整

    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_WR_PTR, 0x00);
    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_RD_PTR, 0x00);
    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_OVF_COUNTER, 0x00);// 清除 FIFO 指针和溢出计数器

    ctrl_value = MAX30102_INT_A_FULL | MAX30102_INT_PPG_RDY;// 使能 FIFO 将满和新数据就绪中断
    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_INT_ENABLE1, ctrl_value);

    sensor_i2c_writereg(MAX30102_I2C_ADDR, MAX30102_REG_MODE_CONFIG, MAX30102_MODE_SPO2); // 设置为 SpO2 模式
    MAX30102_SetPowerMode(MAX30102_POWER_MODE_OFF);
}

rt_err_t MAX30102_SetPowerMode(max30102_power_mode_t mode)
{
    rt_err_t ret = RT_EOK;

    if(max30102_power_mode_ready && mode == max30102_power_mode)
    {
        return RT_EOK;
    }

    switch(mode)
    {
    case MAX30102_POWER_MODE_NORMAL:
        ret = max30102_config_measurement(
            MAX30102_SMP_AVE_4 | MAX30102_FIFO_ROLLOVER_EN | 0x0f,
            MAX30102_SPO2_ADC_RGE_4096 | MAX30102_SPO2_SR_100 | MAX30102_LED_PW_411US_18BITS,
            0x40,
            0x1F,
            MAX30102_NORMAL_HR_DIVIDER);
        break;

    case MAX30102_POWER_MODE_LOW:
        ret = max30102_config_measurement(
            MAX30102_SMP_AVE_8 | MAX30102_FIFO_ROLLOVER_EN | 0x0f,
            MAX30102_SPO2_ADC_RGE_2048 | MAX30102_SPO2_SR_50 | MAX30102_LED_PW_215US_17BITS,
            0x10,
            0x08,
            MAX30102_LOW_POWER_HR_DIVIDER);
        break;

    case MAX30102_POWER_MODE_OFF:
    default:
        mode = MAX30102_POWER_MODE_OFF;
        if(max30102_write_reg(MAX30102_REG_INT_ENABLE1, 0x00) != RT_EOK)
        {
            ret = -RT_ERROR;
            break;
        }
        if(max30102_write_reg(MAX30102_REG_INT_ENABLE2, 0x00) != RT_EOK)
        {
            ret = -RT_ERROR;
            break;
        }
        if(max30102_write_reg(MAX30102_REG_LED1_PA, 0x00) != RT_EOK)
        {
            ret = -RT_ERROR;
            break;
        }
        if(max30102_write_reg(MAX30102_REG_LED2_PA, 0x00) != RT_EOK)
        {
            ret = -RT_ERROR;
            break;
        }
        if(max30102_clear_fifo() != RT_EOK)
        {
            ret = -RT_ERROR;
            break;
        }
        ret = max30102_write_reg(MAX30102_REG_MODE_CONFIG, MAX30102_MODE_SHDN);
        max30102_hr_divider = MAX30102_NORMAL_HR_DIVIDER;
        MAX30102_Clear_Buffer();
        break;
    }

    if(ret == RT_EOK)
    {
        max30102_power_mode = mode;
        max30102_power_mode_ready = RT_TRUE;
    }

    return ret;
}

// void MAX30102_getdata(void)
// {
//     rt_uint32_t red_buf, ir_buf;
//     rt_uint8_t wr_ptr, rd_ptr;
//     rt_uint8_t num_samples;
//     rt_uint8_t temp_buf[6];

//     sensor_i2c_readreg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_WR_PTR, &wr_ptr, 1);
//     sensor_i2c_readreg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_RD_PTR, &rd_ptr, 1);

//     if(wr_ptr >= rd_ptr)
//     {
//         num_samples = wr_ptr - rd_ptr;
//     }
//     else
//     {
//         num_samples = (wr_ptr + 32) - rd_ptr; // FIFO 大小为 32
//     }

//     for(rt_uint8_t i = 0; i < num_samples; i++)
//     {
//         sensor_i2c_readreg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_DATA, temp_buf, 6);
//         red_buf = (((rt_uint32_t)temp_buf[0] << 16) | ((rt_uint32_t)temp_buf[1] << 8) | temp_buf[2]) & 0x03FFFF;
//         ir_buf = (((rt_uint32_t)temp_buf[3] << 16) | ((rt_uint32_t)temp_buf[4] << 8) | temp_buf[5]) & 0x03FFFF;

//         //rt_kprintf("  [%d] RED: %u, IR: %u\n", i, red_buf, ir_buf);
//         Process_PPG_Sample(red_buf, ir_buf, 100.0f);
//     }

// }

// 根据 100Hz硬件采样 + 4次平均 = 实际 25Hz 数据输出
// 4秒的缓冲区大小 = 25 * 4 = 100
#define PPG_BUFFER_SIZE  100 
// 每次更新的数据量（1秒更新一次 = 25个点）
#define PPG_UPDATE_SIZE  25 

static uint32_t ir_buffer[PPG_BUFFER_SIZE];
static uint32_t red_buffer[PPG_BUFFER_SIZE];
static int32_t  buffer_index = 0;

rt_uint8_t MAX30102_getdata(rt_int32_t *out_heart_rate, rt_int32_t *out_spo2)
{
    rt_uint32_t red_buf, ir_buf;
    rt_uint8_t wr_ptr, rd_ptr;
    rt_uint8_t num_samples;
    rt_uint8_t temp_buf[6];

    rt_uint8_t has_new_data = 0;
    rt_uint8_t hr_divider = max30102_hr_divider;

    if(max30102_power_mode == MAX30102_POWER_MODE_OFF)
    {
        return 0;
    }

    if(hr_divider == 0)
    {
        hr_divider = 1;
    }

    sensor_i2c_readreg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_WR_PTR, &wr_ptr, 1);
    sensor_i2c_readreg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_RD_PTR, &rd_ptr, 1);

    if(wr_ptr >= rd_ptr)
    {
        num_samples = wr_ptr - rd_ptr;
    }
    else
    {
        num_samples = (wr_ptr + 32) - rd_ptr; // FIFO 大小为 32
    }

    for(rt_uint8_t i = 0; i < num_samples; i++)
    {
        sensor_i2c_readreg(MAX30102_I2C_ADDR, MAX30102_REG_FIFO_DATA, temp_buf, 6);
        red_buf = (((rt_uint32_t)temp_buf[0] << 16) | ((rt_uint32_t)temp_buf[1] << 8) | temp_buf[2]) & 0x03FFFF;
        ir_buf  = (((rt_uint32_t)temp_buf[3] << 16) | ((rt_uint32_t)temp_buf[4] << 8) | temp_buf[5]) & 0x03FFFF;

        // 1. 将数据存入全局滑动窗口缓冲区
        if (buffer_index < PPG_BUFFER_SIZE)
        {
            red_buffer[buffer_index] = red_buf;
            ir_buffer[buffer_index]  = ir_buf;
            buffer_index++;
        }

        // 2. 当缓冲区填满时，调用官方算法计算
        if (buffer_index == PPG_BUFFER_SIZE)
        {
            int32_t spo2, heart_rate;
            int8_t spo2_valid, hr_valid;

            // 调用官方算法（注意参数顺序：先 IR，后 RED）
            maxim_heart_rate_and_oxygen_saturation(
                ir_buffer, PPG_BUFFER_SIZE, red_buffer, 
                &spo2, &spo2_valid, 
                &heart_rate, &hr_valid
            );

            static float smoothed_hr = 0.0f;
            static float smoothed_spo2 = 0.0f;
            const float ALPHA = 0.2f; // 滤波系数：新数据占20%，历史数据占80% 

            if(hr_valid && spo2_valid)
            {
                // The divider follows the active MAX30102 output rate.
                int32_t real_heart_rate = heart_rate / hr_divider;

                // 1. 生理极限过滤：使用真实心率进行判断 (40~180) 和血氧 (70~100)
                if(real_heart_rate > 40 && real_heart_rate < 180 && spo2 > 70 && spo2 <= 100)
                {
                    // 2. 滤波器冷启动初始化
                    if(smoothed_hr == 0.0f) 
                    {
                        smoothed_hr = (float)real_heart_rate;
                        smoothed_spo2 = (float)spo2;
                    }
                    else 
                    {
                        // 3. 执行 EMA 滤波核心算法
                        smoothed_hr = (ALPHA * real_heart_rate) + ((1.0f - ALPHA) * smoothed_hr);
                        smoothed_spo2 = (ALPHA * spo2) + ((1.0f - ALPHA) * smoothed_spo2);
                    }

                    *out_heart_rate = (rt_int32_t)(smoothed_hr + 0.5f); 
                    *out_spo2 = (rt_int32_t)(smoothed_spo2 + 0.5f); 
                    has_new_data = 1; // 标记本次有新数据

                    // 4. 打印平滑后的最终结果 (四舍五入转为整数)
                    rt_kprintf("HR: %d bpm, SpO2: %d %%\n", (int)(smoothed_hr + 0.5f), (int)(smoothed_spo2 + 0.5f));
                }
                else
                {
                    // 如果算出了离谱数据，直接丢弃，维持上一次的平滑输出
                    if(smoothed_hr > 0) {
                        rt_kprintf("HR: %d bpm, SpO2: %d %% (Hold)\n", (int)(smoothed_hr + 0.5f), (int)(smoothed_spo2 + 0.5f));
                    } else {
                        rt_kprintf("Measuring...\n");
                    }
                }
            }
            else
            {
                // 信号无效时（比如手指离开或剧烈晃动），不清空历史记录，提示等待
                rt_kprintf("Calibrating or invalid signal...\n");
            }

            // 3. 滑动窗口操作：丢弃最旧的数据，为新数据腾出空间
            for (int j = PPG_UPDATE_SIZE; j < PPG_BUFFER_SIZE; j++)
            {
                ir_buffer[j - PPG_UPDATE_SIZE]  = ir_buffer[j];
                red_buffer[j - PPG_UPDATE_SIZE] = red_buffer[j];
            }
            buffer_index = PPG_BUFFER_SIZE - PPG_UPDATE_SIZE;
        }
    }

    return has_new_data;

}

void MAX30102_Clear_Buffer(void) 
{
    buffer_index = 0;
}
