#ifndef MAX30102_REGS_H
#define MAX30102_REGS_H

/* =========================================================================
 * MAX30102 I2C 设备地址与器件 ID
 * ========================================================================= */

// 器件 I2C 地址
#define MAX30102_I2C_ADDR                0x57

// 器件 ID 值
#define MAX30102_EXPECTED_PART_ID        0x15 // 寄存器 0xFF (Part ID) 的固定出厂值

/* =========================================================================
 * MAX30102 寄存器地址映射 [cite: 446, 773, 502, 505]
 * ========================================================================= */

// 状态寄存器
#define MAX30102_REG_INT_STAT1           0x00 // 中断状态 1 
#define MAX30102_REG_INT_STAT2           0x01 // 中断状态 2 
#define MAX30102_REG_INT_ENABLE1         0x02 // 中断使能 1 
#define MAX30102_REG_INT_ENABLE2         0x03 // 中断使能 2 

// FIFO 寄存器
#define MAX30102_REG_FIFO_WR_PTR         0x04 // FIFO 写指针 
#define MAX30102_REG_OVF_COUNTER         0x05 // 溢出计数器 
#define MAX30102_REG_FIFO_RD_PTR         0x06 // FIFO 读指针 
#define MAX30102_REG_FIFO_DATA           0x07 // FIFO 数据寄存器 

// 配置寄存器
#define MAX30102_REG_FIFO_CONFIG         0x08 // FIFO 配置 
#define MAX30102_REG_MODE_CONFIG         0x09 // 模式配置 
#define MAX30102_REG_SPO2_CONFIG         0x0A // SpO2 配置 

// LED 脉冲幅度寄存器
#define MAX30102_REG_LED1_PA             0x0C // LED1 (RED) 脉冲幅度 
#define MAX30102_REG_LED2_PA             0x0D // LED2 (IR) 脉冲幅度 

// 多 LED 模式控制寄存器
#define MAX30102_REG_MULTI_LED_CTRL1     0x11 // SLOT1 和 SLOT2 
#define MAX30102_REG_MULTI_LED_CTRL2     0x12 // SLOT3 和 SLOT4 

// 内部温度寄存器
#define MAX30102_REG_TEMP_INT            0x1F // 内部温度整数部分 [cite: 773]
#define MAX30102_REG_TEMP_FRAC           0x20 // 内部温度小数部分 [cite: 773]
#define MAX30102_REG_TEMP_CONFIG         0x21 // 内部温度配置 [cite: 773]

// 器件 ID 寄存器
#define MAX30102_REG_REV_ID              0xFE // 版本 ID [cite: 502]
#define MAX30102_REG_PART_ID             0xFF // 器件 ID [cite: 505]


/* =========================================================================
 * MAX30102 控制寄存器宏定义与位掩码
 * ========================================================================= */

/* --- 中断标志与使能掩码 (REG 0x00 ~ 0x03) [cite: 515, 540] --- */
#define MAX30102_INT_A_FULL              (1 << 7) // FIFO 将满 [cite: 515]
#define MAX30102_INT_PPG_RDY             (1 << 6) // 新 FIFO 数据就绪 [cite: 515]
#define MAX30102_INT_ALC_OVF             (1 << 5) // 环境光消除溢出 [cite: 515]
#define MAX30102_INT_PWR_RDY             (1 << 0) // 电源就绪 [cite: 515]
#define MAX30102_INT_DIE_TEMP_RDY        (1 << 1) // 内部温度就绪 [cite: 515]

/* --- FIFO 配置寄存器 (REG 0x08) [cite: 691, 696, 699, 705] --- */
// 采样平均 (SMP_AVE[2:0]) [cite: 696]
#define MAX30102_SMP_AVE_1               (0x00 << 5) // 不平均 [cite: 696]
#define MAX30102_SMP_AVE_2               (0x01 << 5) // 平均 2 个样本 [cite: 696]
#define MAX30102_SMP_AVE_4               (0x02 << 5) // 平均 4 个样本 [cite: 696]
#define MAX30102_SMP_AVE_8               (0x03 << 5) // 平均 8 个样本 [cite: 696]
#define MAX30102_SMP_AVE_16              (0x04 << 5) // 平均 16 个样本 [cite: 696]
#define MAX30102_SMP_AVE_32              (0x05 << 5) // 平均 32 个样本 [cite: 696]
// FIFO 满覆盖使能 [cite: 699]
#define MAX30102_FIFO_ROLLOVER_EN        (1 << 4) // [cite: 699]

/* --- 模式配置寄存器 (REG 0x09) [cite: 711, 723] --- */
#define MAX30102_MODE_SHDN               (1 << 7) // 关断模式 [cite: 711]
#define MAX30102_MODE_RESET              (1 << 6) // 软件复位 [cite: 711]
// 工作模式选型 (MODE[2:0]) [cite: 723]
#define MAX30102_MODE_HR                 0x02     // 心率模式 (仅红光) [cite: 723]
#define MAX30102_MODE_SPO2               0x03     // SpO2 模式 (红光 + 红外) [cite: 723]
#define MAX30102_MODE_MULTI_LED          0x07     // 多 LED 模式 [cite: 723]

/* --- SpO2 配置寄存器 (REG 0x0A) [cite: 725, 729, 739, 745] --- */
// ADC 满量程范围 (SPO2_ADC_RGE[1:0]) [cite: 729]
#define MAX30102_SPO2_ADC_RGE_2048       (0x00 << 5) // 2048 nA [cite: 729]
#define MAX30102_SPO2_ADC_RGE_4096       (0x01 << 5) // 4096 nA [cite: 729]
#define MAX30102_SPO2_ADC_RGE_8192       (0x02 << 5) // 8192 nA [cite: 729]
#define MAX30102_SPO2_ADC_RGE_16384      (0x03 << 5) // 16384 nA [cite: 729]
// SpO2 采样率 (SPO2_SR[2:0]) [cite: 739]
#define MAX30102_SPO2_SR_50              (0x00 << 2) // 50 sps [cite: 739]
#define MAX30102_SPO2_SR_100             (0x01 << 2) // 100 sps [cite: 739]
#define MAX30102_SPO2_SR_200             (0x02 << 2) // 200 sps [cite: 739]
#define MAX30102_SPO2_SR_400             (0x03 << 2) // 400 sps [cite: 739]
#define MAX30102_SPO2_SR_800             (0x04 << 2) // 800 sps [cite: 739]
#define MAX30102_SPO2_SR_1000            (0x05 << 2) // 1000 sps [cite: 739]
#define MAX30102_SPO2_SR_1600            (0x06 << 2) // 1600 sps [cite: 739]
#define MAX30102_SPO2_SR_3200            (0x07 << 2) // 3200 sps [cite: 739]
// LED 脉冲宽度控制与 ADC 分辨率 (LED_PW[1:0]) [cite: 745]
#define MAX30102_LED_PW_69US_15BITS      (0x00)      // 69 us, 15 bits [cite: 745]
#define MAX30102_LED_PW_118US_16BITS     (0x01)      // 118 us, 16 bits [cite: 745]
#define MAX30102_LED_PW_215US_17BITS     (0x02)      // 215 us, 17 bits [cite: 745]
#define MAX30102_LED_PW_411US_18BITS     (0x03)      // 411 us, 18 bits [cite: 745]

/* --- 多 LED 模式配置 (REG 0x11, 0x12) [cite: 765] --- */
// Slot 激活控制掩码 [cite: 765]
#define MAX30102_SLOT_NONE               0x00 // 禁用 [cite: 765]
#define MAX30102_SLOT_LED1_RED           0x01 // 激活 LED1 (红光) [cite: 765]
#define MAX30102_SLOT_LED2_IR            0x02 // 激活 LED2 (红外) [cite: 765]

/* --- 温度使能寄存器 (REG 0x21) [cite: 773, 785] --- */
#define MAX30102_TEMP_EN                 (1 << 0) // 开启单次温度转换 [cite: 785]

#endif /* MAX30102_REGS_H */