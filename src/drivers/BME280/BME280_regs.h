#ifndef BME280_REGS_H
#define BME280_REGS_H

/* ========================================================================== */
/* BME280 I2C 设备(从机)地址                                                  */
/* ========================================================================== */
/* 根据 SDO 引脚的电平状态选择 */
#define BME280_I2C_ADDR_L       0x76    /* SDO 接 GND (默认) */
#define BME280_I2C_ADDR_H       0x77    /* SDO 接 VDDIO */
/* ========================================================================== */
/* 1. BME280 寄存器地址映射 (Memory Map)                                      */
/* ========================================================================== */
#define BME280_REG_HUM_LSB      0xFF    /* 湿度数据 LSB [cite: 2288] */
#define BME280_REG_HUM_MSB      0xFD    /* 湿度数据 MSB [cite: 2288] */
#define BME280_REG_TEMP_XLSB    0xFC    /* 温度数据 XLSB [cite: 2288] */
#define BME280_REG_TEMP_LSB     0xFB    /* 温度数据 LSB [cite: 2288] */
#define BME280_REG_TEMP_MSB     0xFA    /* 温度数据 MSB [cite: 2288] */
#define BME280_REG_PRESS_XLSB   0xF9    /* 气压数据 XLSB [cite: 2288] */
#define BME280_REG_PRESS_LSB    0xF8    /* 气压数据 LSB [cite: 2288] */
#define BME280_REG_PRESS_MSB    0xF7    /* 气压数据 MSB [cite: 2288] */

#define BME280_REG_CONFIG       0xF5    /* 配置寄存器 [cite: 2288] */
#define BME280_REG_CTRL_MEAS    0xF4    /* 测量控制寄存器 [cite: 2288] */
#define BME280_REG_STATUS       0xF3    /* 状态寄存器 [cite: 2288] */
#define BME280_REG_CTRL_HUM     0xF2    /* 湿度控制寄存器 [cite: 2288] */

#define BME280_REG_RESET        0xE0    /* 软复位寄存器 [cite: 2288] */
#define BME280_REG_ID           0xD0    /* 芯片ID寄存器 [cite: 2288] */

/* 校准数据寄存器基地址 */
#define BME280_REG_CALIB_00     0x88    /* calib00 寄存器起始地址 (0x88...0xA1) [cite: 2288] */
#define BME280_REG_CALIB_26     0xE1    /* calib26 寄存器起始地址 (0xE1...0xF0) [cite: 2288] */

/* ========================================================================== */
/* 2. BME280 芯片ID 与 复位命令                                               */
/* ========================================================================== */
#define BME280_CHIP_ID          0x60    /* 默认芯片 ID 值 [cite: 2292] */
#define BME280_SOFT_RESET_CMD   0xB6    /* 触发软复位的命令字 [cite: 2296] */

/* ========================================================================== */
/* 3. BME280 寄存器位掩码 (Bit Masks) 与 偏移量 (Pos)                         */
/* ========================================================================== */

/* status 寄存器 (0xF3) */
#define BME280_STATUS_MEASURING_MASK  0x08  /* bit 3: 测量状态 [cite: 2312] */
#define BME280_STATUS_IM_UPDATE_MASK  0x01  /* bit 0: NVM 数据更新状态 [cite: 2312] */

/* ctrl_hum 寄存器 (0xF2) */
#define BME280_OSRS_H_MASK            0x07  /* bit 2:0: 湿度过采样掩码 [cite: 2306] */

/* ctrl_meas 寄存器 (0xF4) */
#define BME280_OSRS_T_MASK            0xE0  /* bit 7:5: 温度过采样掩码 [cite: 2320] */
#define BME280_OSRS_T_POS             5
#define BME280_OSRS_P_MASK            0x1C  /* bit 4:2: 气压过采样掩码 [cite: 2320] */
#define BME280_OSRS_P_POS             2
#define BME280_MODE_MASK              0x03  /* bit 1:0: 传感器模式掩码 [cite: 2320] */

/* config 寄存器 (0xF5) */
#define BME280_T_SB_MASK              0xE0  /* bit 7:5: 待机时间掩码 [cite: 2336] */
#define BME280_T_SB_POS               5
#define BME280_FILTER_MASK            0x1C  /* bit 4:2: IIR 滤波器掩码 [cite: 2336] */
#define BME280_FILTER_POS             2
#define BME280_SPI3W_EN_MASK          0x01  /* bit 0: 3线SPI使能掩码 [cite: 2336] */

/* ========================================================================== */
/* 4. BME280 配置选项宏定义 (Settings Macros)                                 */
/* ========================================================================== */

/* 过采样设置 (Oversampling) - 适用于 T, P, H [cite: 2308, 2322, 2325] */
#define BME280_OVERSAMP_SKIPPED 0x00    /* 跳过测量 */
#define BME280_OVERSAMP_1X      0x01    /* x1 过采样 */
#define BME280_OVERSAMP_2X      0x02    /* x2 过采样 */
#define BME280_OVERSAMP_4X      0x03    /* x4 过采样 */
#define BME280_OVERSAMP_8X      0x04    /* x8 过采样 */
#define BME280_OVERSAMP_16X     0x05    /* x16 过采样 */

/* 传感器工作模式 (Sensor Mode) [cite: 2327] */
#define BME280_MODE_SLEEP       0x00    /* 睡眠模式 */
#define BME280_MODE_FORCED      0x01    /* 强制模式 (01 或 10 均为强制模式) */
#define BME280_MODE_NORMAL      0x03    /* 正常模式 */

/* IIR 滤波器系数设置 (Filter Coefficient) [cite: 2340] */
#define BME280_FILTER_OFF       0x00    /* 滤波器关闭 */
#define BME280_FILTER_COEFF_2   0x01    /* 系数 2 */
#define BME280_FILTER_COEFF_4   0x02    /* 系数 4 */
#define BME280_FILTER_COEFF_8   0x03    /* 系数 8 */
#define BME280_FILTER_COEFF_16  0x04    /* 系数 16 */

/* 待机时间设置 (Standby Time t_sb) - 正常模式下 [cite: 2338] */
#define BME280_STANDBY_0_5_MS   0x00    /* 0.5 ms */
#define BME280_STANDBY_62_5_MS  0x01    /* 62.5 ms */
#define BME280_STANDBY_125_MS   0x02    /* 125 ms */
#define BME280_STANDBY_250_MS   0x03    /* 250 ms */
#define BME280_STANDBY_500_MS   0x04    /* 500 ms */
#define BME280_STANDBY_1000_MS  0x05    /* 1000 ms */
#define BME280_STANDBY_10_MS    0x06    /* 10 ms */
#define BME280_STANDBY_20_MS    0x07    /* 20 ms */

#endif /* BME280_REGS_H */