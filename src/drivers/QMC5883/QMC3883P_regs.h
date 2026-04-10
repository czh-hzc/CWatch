#ifndef __QMC5883P_REGS_H__
#define __QMC5883P_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * I2C 地址定义
 * ========================================================================= */
#define QMC5883P_I2C_ADDR           0x2C    // QMC5883P 默认 7位 I2C 从机地址 [cite: 347]

/* =========================================================================
 * 寄存器地址映射表 [cite: 513]
 * ========================================================================= */
#define QMC5883P_REG_CHIPID         0x00    // 芯片 ID 寄存器 (只读，默认值通常为 0x80)
#define QMC5883P_REG_XOUT_LSB       0x01    // X 轴数据低 8 位
#define QMC5883P_REG_XOUT_MSB       0x02    // X 轴数据高 8 位
#define QMC5883P_REG_YOUT_LSB       0x03    // Y 轴数据低 8 位
#define QMC5883P_REG_YOUT_MSB       0x04    // Y 轴数据高 8 位
#define QMC5883P_REG_ZOUT_LSB       0x05    // Z 轴数据低 8 位
#define QMC5883P_REG_ZOUT_MSB       0x06    // Z 轴数据高 8 位
#define QMC5883P_REG_STATUS         0x09    // 状态寄存器
#define QMC5883P_REG_CTRL1          0x0A    // 控制寄存器 1 (配置模式、ODR、OSR)
#define QMC5883P_REG_CTRL2          0x0B    // 控制寄存器 2 (软复位、自检、量程等)

/* =========================================================================
 * 状态寄存器 (0x09) 位定义 [cite: 531]
 * ========================================================================= */
#define QMC5883P_STATUS_DRDY        0x01    // Bit 0: 数据就绪标志位 (1: 就绪, 0: 未就绪) [cite: 532, 534]
#define QMC5883P_STATUS_OVFL        0x02    // Bit 1: 数据溢出标志位 (1: 溢出, 0: 正常) [cite: 540, 541]

/* =========================================================================
 * 控制寄存器 1 (0x0A) 配置宏定义 [cite: 556]
 * ========================================================================= */
// 工作模式 (MODE: Bits [1:0])
#define QMC5883P_MODE_SUSPEND       0x00    // 挂起模式 (上电默认)
#define QMC5883P_MODE_NORMAL        0x01    // 正常模式
#define QMC5883P_MODE_SINGLE        0x02    // 单次测量模式
#define QMC5883P_MODE_CONT          0x03    // 连续测量模式

// 输出数据速率 (ODR: Bits [3:2])
#define QMC5883P_ODR_10HZ           (0x00 << 2) // 10Hz
#define QMC5883P_ODR_50HZ           (0x01 << 2) // 50Hz
#define QMC5883P_ODR_100HZ          (0x02 << 2) // 100Hz
#define QMC5883P_ODR_200HZ          (0x03 << 2) // 200Hz

// 过采样率 (OSR1: Bits [5:4]) 
#define QMC5883P_OSR1_8             (0x00 << 4) // Ratio = 8
#define QMC5883P_OSR1_4             (0x01 << 4) // Ratio = 4
#define QMC5883P_OSR1_2             (0x02 << 4) // Ratio = 2
#define QMC5883P_OSR1_1             (0x03 << 4) // Ratio = 1

// 下采样率 (OSR2: Bits [7:6])
#define QMC5883P_OSR2_1             (0x00 << 6) // Ratio = 1
#define QMC5883P_OSR2_2             (0x01 << 6) // Ratio = 2
#define QMC5883P_OSR2_4             (0x02 << 6) // Ratio = 4
#define QMC5883P_OSR2_8             (0x03 << 6) // Ratio = 8

/* =========================================================================
 * 控制寄存器 2 (0x0B) 配置宏定义 [cite: 567]
 * ========================================================================= */
// Set/Reset 模式控制 (SET/RESET MODE: Bits [1:0]) [cite: 570]
#define QMC5883P_SET_RESET_ON       0x00    // 开启 Set 和 Reset
#define QMC5883P_SET_ONLY_ON        0x01    // 仅开启 Set (偏移量不更新)
#define QMC5883P_SET_RESET_OFF      0x02    // 关闭 Set 和 Reset (也可以使用 0x03)

// 全量程范围 (RNG: Bits [4:3]) [cite: 570]
#define QMC5883P_RNG_30G            (0x00 << 3) // ±30 Gauss (分辨率最低)
#define QMC5883P_RNG_12G            (0x01 << 3) // ±12 Gauss
#define QMC5883P_RNG_8G             (0x02 << 3) // ±8 Gauss
#define QMC5883P_RNG_2G             (0x03 << 3) // ±2 Gauss (分辨率最高)

// 自检与软复位控制位 [cite: 570]
#define QMC5883P_SELF_TEST_EN       (1 << 6)    // Bit 6: 开启自检功能
#define QMC5883P_SOFT_RST_EN        (1 << 7)    // Bit 7: 触发软复位 (复位所有寄存器)

#ifdef __cplusplus
}
#endif

#endif /* __QMC5883P_REGS_H__ */