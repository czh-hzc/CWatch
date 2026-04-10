/**
 * @file lsm6ds3tr_c_regs.h
 * @brief LSM6DS3TR-C 6轴 IMU 寄存器映射及配置宏定义
 * @cite 
 */

#ifndef __LSM6DS3TR_C_REGS_H__
#define __LSM6DS3TR_C_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 1. I2C 设备地址及 ID 
 * ========================================================================= */
/* SDO/SA0 引脚接地(GND) -> 0x6A, SDO/SA0 引脚接电源(VCC) -> 0x6B [cite: 676, 677] */
#define LSM6DS3TR_C_I2C_ADDR_L              0x6A 
#define LSM6DS3TR_C_I2C_ADDR_H              0x6B
#define LSM6DS3TR_C_WHO_AM_I_ID             0x6A  // 芯片固定 ID [cite: 1148]

/* =========================================================================
 * 2. 寄存器地址映射 (Register Map) [cite: 960, 967, 973, 979]
 * ========================================================================= */
#define LSM6DS3TR_C_FUNC_CFG_ACCESS         0x01 // 嵌入式功能配置访问
#define LSM6DS3TR_C_SENSOR_SYNC_TIME_FRAME  0x04 // 传感器同步时间帧
#define LSM6DS3TR_C_SENSOR_SYNC_RES_RATIO   0x05 // 传感器同步分辨率比率
#define LSM6DS3TR_C_FIFO_CTRL1              0x06 // FIFO 控制 1
#define LSM6DS3TR_C_FIFO_CTRL2              0x07 // FIFO 控制 2
#define LSM6DS3TR_C_FIFO_CTRL3              0x08 // FIFO 控制 3
#define LSM6DS3TR_C_FIFO_CTRL4              0x09 // FIFO 控制 4
#define LSM6DS3TR_C_FIFO_CTRL5              0x0A // FIFO 控制 5
#define LSM6DS3TR_C_DRDY_PULSE_CFG_G        0x0B // 数据就绪脉冲配置
#define LSM6DS3TR_C_INT1_CTRL               0x0D // INT1 引脚控制
#define LSM6DS3TR_C_INT2_CTRL               0x0E // INT2 引脚控制
#define LSM6DS3TR_C_WHO_AM_I                0x0F // 设备 ID

/* 核心控制寄存器 */
#define LSM6DS3TR_C_CTRL1_XL                0x10 // 加速度计控制 1
#define LSM6DS3TR_C_CTRL2_G                 0x11 // 陀螺仪控制 2
#define LSM6DS3TR_C_CTRL3_C                 0x12 // 系统控制 3
#define LSM6DS3TR_C_CTRL4_C                 0x13 // 系统控制 4
#define LSM6DS3TR_C_CTRL5_C                 0x14 // 系统控制 5
#define LSM6DS3TR_C_CTRL6_C                 0x15 // 系统控制 6
#define LSM6DS3TR_C_CTRL7_G                 0x16 // 陀螺仪控制 7
#define LSM6DS3TR_C_CTRL8_XL                0x17 // 加速度计控制 8
#define LSM6DS3TR_C_CTRL9_XL                0x18 // 加速度计控制 9
#define LSM6DS3TR_C_CTRL10_C                0x19 // 系统控制 10

/* 主机与中断配置 */
#define LSM6DS3TR_C_MASTER_CONFIG           0x1A // I2C 主机配置
#define LSM6DS3TR_C_WAKE_UP_SRC             0x1B // 唤醒中断源状态
#define LSM6DS3TR_C_TAP_SRC                 0x1C // 敲击中断源状态
#define LSM6DS3TR_C_D6D_SRC                 0x1D // 6D/4D姿态中断源状态
#define LSM6DS3TR_C_STATUS_REG              0x1E // 状态寄存器 (数据就绪)

/* 传感器输出数据寄存器 */
#define LSM6DS3TR_C_OUT_TEMP_L              0x20 // 温度输出低字节
#define LSM6DS3TR_C_OUT_TEMP_H              0x21 // 温度输出高字节
#define LSM6DS3TR_C_OUTX_L_G                0x22 // 陀螺仪 X 轴低字节
#define LSM6DS3TR_C_OUTX_H_G                0x23 // 陀螺仪 X 轴高字节
#define LSM6DS3TR_C_OUTY_L_G                0x24 // 陀螺仪 Y 轴低字节
#define LSM6DS3TR_C_OUTY_H_G                0x25 // 陀螺仪 Y 轴高字节
#define LSM6DS3TR_C_OUTZ_L_G                0x26 // 陀螺仪 Z 轴低字节
#define LSM6DS3TR_C_OUTZ_H_G                0x27 // 陀螺仪 Z 轴高字节
#define LSM6DS3TR_C_OUTX_L_XL               0x28 // 加速度计 X 轴低字节
#define LSM6DS3TR_C_OUTX_H_XL               0x29 // 加速度计 X 轴高字节
#define LSM6DS3TR_C_OUTY_L_XL               0x2A // 加速度计 Y 轴低字节
#define LSM6DS3TR_C_OUTY_H_XL               0x2B // 加速度计 Y 轴高字节
#define LSM6DS3TR_C_OUTZ_L_XL               0x2C // 加速度计 Z 轴低字节
#define LSM6DS3TR_C_OUTZ_H_XL               0x2D // 加速度计 Z 轴高字节

/* FIFO 与步数寄存器 */
#define LSM6DS3TR_C_FIFO_STATUS1            0x3A // FIFO 状态 1
#define LSM6DS3TR_C_FIFO_STATUS2            0x3B // FIFO 状态 2
#define LSM6DS3TR_C_FIFO_STATUS3            0x3C // FIFO 状态 3
#define LSM6DS3TR_C_FIFO_STATUS4            0x3D // FIFO 状态 4
#define LSM6DS3TR_C_FIFO_DATA_OUT_L         0x3E // FIFO 数据输出低字节
#define LSM6DS3TR_C_FIFO_DATA_OUT_H         0x3F // FIFO 数据输出高字节
#define LSM6DS3TR_C_TIMESTAMP0_REG          0x40 // 时间戳输出 0
#define LSM6DS3TR_C_TIMESTAMP1_REG          0x41 // 时间戳输出 1
#define LSM6DS3TR_C_TIMESTAMP2_REG          0x42 // 时间戳输出 2
#define LSM6DS3TR_C_STEP_TIMESTAMP_L        0x49 // 计步器时间戳低字节
#define LSM6DS3TR_C_STEP_TIMESTAMP_H        0x4A // 计步器时间戳高字节
#define LSM6DS3TR_C_STEP_COUNTER_L          0x4B // 计步器输出低字节
#define LSM6DS3TR_C_STEP_COUNTER_H          0x4C // 计步器输出高字节

/* 中断源与高级配置 */
#define LSM6DS3TR_C_FUNC_SRC1               0x53 // 功能中断源 1
#define LSM6DS3TR_C_FUNC_SRC2               0x54 // 功能中断源 2
#define LSM6DS3TR_C_WRIST_TILT_IA           0x55 // 翻腕中断状态
#define LSM6DS3TR_C_TAP_CFG                 0x58 // 敲击配置
#define LSM6DS3TR_C_TAP_THS_6D              0x59 // 敲击与 6D 阈值
#define LSM6DS3TR_C_INT_DUR2                0x5A // 中断持续时间 2
#define LSM6DS3TR_C_WAKE_UP_THS             0x5B // 唤醒阈值
#define LSM6DS3TR_C_WAKE_UP_DUR             0x5C // 唤醒持续时间
#define LSM6DS3TR_C_FREE_FALL               0x5D // 自由落体配置
#define LSM6DS3TR_C_MD1_CFG                 0x5E // INT1 路由配置
#define LSM6DS3TR_C_MD2_CFG                 0x5F // INT2 路由配置
#define LSM6DS3TR_C_MASTER_CMD_CODE         0x60 // 主机命令代码
#define LSM6DS3TR_C_SENS_SYNC_SPI_ERR_CODE  0x61 // 传感器同步错误代码

/* 磁力计/外部传感器与补偿 */
#define LSM6DS3TR_C_OUT_MAG_RAW_X_L         0x66 // 磁力计原始数据 X 低字节
#define LSM6DS3TR_C_OUT_MAG_RAW_X_H         0x67 // 磁力计原始数据 X 高字节
#define LSM6DS3TR_C_OUT_MAG_RAW_Y_L         0x68 // 磁力计原始数据 Y 低字节
#define LSM6DS3TR_C_OUT_MAG_RAW_Y_H         0x69 // 磁力计原始数据 Y 高字节
#define LSM6DS3TR_C_OUT_MAG_RAW_Z_L         0x6A // 磁力计原始数据 Z 低字节
#define LSM6DS3TR_C_OUT_MAG_RAW_Z_H         0x6B // 磁力计原始数据 Z 高字节
#define LSM6DS3TR_C_X_OFS_USR               0x73 // 加速度计 X 轴用户偏移
#define LSM6DS3TR_C_Y_OFS_USR               0x74 // 加速度计 Y 轴用户偏移
#define LSM6DS3TR_C_Z_OFS_USR               0x75 // 加速度计 Z 轴用户偏移

/* =========================================================================
 * 3. 核心控制寄存器配置宏 (Configuration Macros)
 * ========================================================================= */

/* --- INT1_CTRL (0Dh) : INT1 引脚中断路由 [cite: 1130, 1132] --- */
#define LSM6DS3TR_C_INT1_STEP_DETECTOR      0x80  // 启用计步器中断
#define LSM6DS3TR_C_INT1_SIGN_MOT           0x40  // 启用有效运动中断
#define LSM6DS3TR_C_INT1_FULL_FLAG          0x20  // 启用 FIFO 满中断
#define LSM6DS3TR_C_INT1_FIFO_OVR           0x10  // 启用 FIFO 溢出中断
#define LSM6DS3TR_C_INT1_FTH                0x08  // 启用 FIFO 水位阈值中断
#define LSM6DS3TR_C_INT1_BOOT               0x04  // 启用启动状态中断
#define LSM6DS3TR_C_INT1_DRDY_G             0x02  // 启用陀螺仪数据就绪中断
#define LSM6DS3TR_C_INT1_DRDY_XL            0x01  // 启用加速度计数据就绪中断

/* --- INT2_CTRL (0Eh) : INT2 引脚中断路由 [cite: 1143, 1144] --- */
#define LSM6DS3TR_C_INT2_STEP_DELTA         0x80  // 启用计步器步数增量中断
#define LSM6DS3TR_C_INT2_STEP_COUNT_OV      0x40  // 启用计步器溢出中断
#define LSM6DS3TR_C_INT2_FULL_FLAG          0x20  // 启用 FIFO 满中断
#define LSM6DS3TR_C_INT2_FIFO_OVR           0x10  // 启用 FIFO 溢出中断
#define LSM6DS3TR_C_INT2_FTH                0x08  // 启用 FIFO 水位阈值中断
#define LSM6DS3TR_C_INT2_DRDY_TEMP          0x04  // 启用温度数据就绪中断
#define LSM6DS3TR_C_INT2_DRDY_G             0x02  // 启用陀螺仪数据就绪中断
#define LSM6DS3TR_C_INT2_DRDY_XL            0x01  // 启用加速度计数据就绪中断

/* --- CTRL1_XL (10h) : 加速度计控制 [cite: 1173, 1175] --- */
/* ODR: 输出数据速率 (Bit 7:4) */
#define LSM6DS3TR_C_XL_ODR_POWER_DOWN       (0x00 << 4) // 掉电模式
#define LSM6DS3TR_C_XL_ODR_12_5HZ           (0x01 << 4) // 12.5 Hz (高性能或低功耗)
#define LSM6DS3TR_C_XL_ODR_26HZ             (0x02 << 4) // 26 Hz
#define LSM6DS3TR_C_XL_ODR_52HZ             (0x03 << 4) // 52 Hz
#define LSM6DS3TR_C_XL_ODR_104HZ            (0x04 << 4) // 104 Hz
#define LSM6DS3TR_C_XL_ODR_208HZ            (0x05 << 4) // 208 Hz
#define LSM6DS3TR_C_XL_ODR_416HZ            (0x06 << 4) // 416 Hz
#define LSM6DS3TR_C_XL_ODR_833HZ            (0x07 << 4) // 833 Hz
#define LSM6DS3TR_C_XL_ODR_1666HZ           (0x08 << 4) // 1.66 kHz
#define LSM6DS3TR_C_XL_ODR_3332HZ           (0x09 << 4) // 3.33 kHz
#define LSM6DS3TR_C_XL_ODR_6664HZ           (0x0A << 4) // 6.66 kHz

/* FS: 测量量程 (Bit 3:2) */
#define LSM6DS3TR_C_XL_FS_2G                (0x00 << 2) // ±2 g
#define LSM6DS3TR_C_XL_FS_16G               (0x01 << 2) // ±16 g
#define LSM6DS3TR_C_XL_FS_4G                (0x02 << 2) // ±4 g
#define LSM6DS3TR_C_XL_FS_8G                (0x03 << 2) // ±8 g

/* LPF1_BW_SEL & BW0_XL: 带宽选择 (Bit 1:0) */
#define LSM6DS3TR_C_XL_BW_400HZ             (0x01)      // 模拟带宽选择 400Hz (当 ODR>=1.67kHz)

/* --- CTRL2_G (11h) : 陀螺仪控制 [cite: 1189, 1191] --- */
/* ODR: 输出数据速率 (Bit 7:4) */
#define LSM6DS3TR_C_G_ODR_POWER_DOWN        (0x00 << 4) // 掉电模式
#define LSM6DS3TR_C_G_ODR_12_5HZ            (0x01 << 4) // 12.5 Hz
#define LSM6DS3TR_C_G_ODR_26HZ              (0x02 << 4) // 26 Hz
#define LSM6DS3TR_C_G_ODR_52HZ              (0x03 << 4) // 52 Hz
#define LSM6DS3TR_C_G_ODR_104HZ             (0x04 << 4) // 104 Hz
#define LSM6DS3TR_C_G_ODR_208HZ             (0x05 << 4) // 208 Hz
#define LSM6DS3TR_C_G_ODR_416HZ             (0x06 << 4) // 416 Hz
#define LSM6DS3TR_C_G_ODR_833HZ             (0x07 << 4) // 833 Hz
#define LSM6DS3TR_C_G_ODR_1666HZ            (0x08 << 4) // 1.66 kHz
#define LSM6DS3TR_C_G_ODR_3332HZ            (0x09 << 4) // 3.33 kHz
#define LSM6DS3TR_C_G_ODR_6664HZ            (0x0A << 4) // 6.66 kHz

/* FS: 测量量程 (Bit 3:2) */
#define LSM6DS3TR_C_G_FS_245DPS             (0x00 << 2) // ±245 dps (250 dps)
#define LSM6DS3TR_C_G_FS_500DPS             (0x01 << 2) // ±500 dps
#define LSM6DS3TR_C_G_FS_1000DPS            (0x02 << 2) // ±1000 dps
#define LSM6DS3TR_C_G_FS_2000DPS            (0x03 << 2) // ±2000 dps

/* FS_125: 125dps 特殊量程使能 (Bit 1) */
#define LSM6DS3TR_C_G_FS_125DPS_EN          (0x02)      // 启用 ±125 dps

/* --- CTRL3_C (12h) : 核心系统控制 [cite: 1201] --- */
#define LSM6DS3TR_C_BOOT                    (0x80) // 1: 重启内存内容
#define LSM6DS3TR_C_BDU                     (0x40) // 1: 块数据更新使能(防撕裂锁存)
#define LSM6DS3TR_C_H_LACTIVE               (0x20) // 1: 中断引脚低电平有效 (0为高电平有效)
#define LSM6DS3TR_C_PP_OD                   (0x10) // 1: 中断引脚开漏输出 (0为推挽)
#define LSM6DS3TR_C_SIM                     (0x08) // 1: 启用 3 线 SPI 模式 (0为 4 线)
#define LSM6DS3TR_C_IF_INC                  (0x04) // 1: 多字节读写时寄存器地址自动递增 (连读必备)
#define LSM6DS3TR_C_BLE                     (0x02) // 1: 大端模式 (0为小端模式)
#define LSM6DS3TR_C_SW_RESET                (0x01) // 1: 触发软件复位

/* --- CTRL4_C (13h) : 附加控制 [cite: 1215] --- */
#define LSM6DS3TR_C_DEN_XL_EN               (0x80) // 扩展 DEN 功能到加速度计
#define LSM6DS3TR_C_SLEEP                   (0x40) // 陀螺仪睡眠模式使能
#define LSM6DS3TR_C_INT2_ON_INT1            (0x20) // 将所有中断路由到 INT1 引脚 (逻辑或)
#define LSM6DS3TR_C_DEN_DRDY_INT1           (0x10) // DEN 数据就绪路由到 INT1
#define LSM6DS3TR_C_DRDY_MASK               (0x08) // 数据可用掩码使能
#define LSM6DS3TR_C_I2C_DISABLE             (0x04) // 1: 禁用 I2C (仅启用 SPI)
#define LSM6DS3TR_C_LPF1_SEL_G              (0x02) // 启用陀螺仪数字 LPF1 低通滤波器

/* --- CTRL5_C (14h) : 循环读取与自检 [cite: 1222] --- */
#define LSM6DS3TR_C_ROUNDING_DISABLED       (0x00 << 5) // 禁用循环 burst 读
#define LSM6DS3TR_C_ROUNDING_XL             (0x01 << 5) // 仅循环读加速度计
#define LSM6DS3TR_C_ROUNDING_GYRO           (0x02 << 5) // 仅循环读陀螺仪
#define LSM6DS3TR_C_ROUNDING_GYRO_XL        (0x03 << 5) // 循环读加速度和陀螺仪
#define LSM6DS3TR_C_ST_G_NORMAL             (0x00 << 2) // 陀螺仪自检关闭
#define LSM6DS3TR_C_ST_XL_NORMAL            (0x00)      // 加速度计自检关闭

/* --- CTRL6_C (15h) : 陀螺仪滤波器与角速度配置 [cite: 1245] --- */
#define LSM6DS3TR_C_XL_HM_MODE              (0x10) // 1: 禁用加速度计高性能模式
#define LSM6DS3TR_C_USR_OFF_W               (0x08) // 加速度计用户偏移权重选择
/* 陀螺仪低通滤波器(LPF1)带宽 (Bit 1:0) */
#define LSM6DS3TR_C_G_FTYPE_00              (0x00) 
#define LSM6DS3TR_C_G_FTYPE_01              (0x01) 
#define LSM6DS3TR_C_G_FTYPE_10              (0x02) 
#define LSM6DS3TR_C_G_FTYPE_11              (0x03) 

/* --- CTRL7_G (16h) : 陀螺仪高性能与高通滤波 [cite: 1261] --- */
#define LSM6DS3TR_C_G_HM_MODE               (0x80) // 1: 禁用陀螺仪高性能模式
#define LSM6DS3TR_C_HP_EN_G                 (0x40) // 1: 启用陀螺仪高通滤波器
#define LSM6DS3TR_C_ROUNDING_STATUS         (0x04) // 1: 启用源寄存器循环读

/* --- CTRL8_XL (17h) : 加速度计滤波器 [cite: 1267] --- */
#define LSM6DS3TR_C_LPF2_XL_EN              (0x80) // 启用加速度计 LPF2
#define LSM6DS3TR_C_HP_REF_MODE             (0x10) // 启用 HP 滤波器参考模式
#define LSM6DS3TR_C_INPUT_COMPOSITE         (0x08) // 复合滤波器输入选择
#define LSM6DS3TR_C_HP_SLOPE_XL_EN          (0x04) // 启用斜率/高通滤波器
#define LSM6DS3TR_C_LOW_PASS_ON_6D          (0x01) // 6D 方向计算应用低通滤波

/* --- CTRL10_C (19h) : 嵌入式功能总开关 [cite: 1303] --- */
#define LSM6DS3TR_C_WRIST_TILT_EN           (0x80) // 启用翻腕算法
#define LSM6DS3TR_C_TIMER_EN                (0x20) // 启用时间戳计数器
#define LSM6DS3TR_C_PEDO_EN                 (0x10) // 启用计步器算法
#define LSM6DS3TR_C_TILT_EN                 (0x08) // 启用倾斜计算
#define LSM6DS3TR_C_FUNC_EN                 (0x04) // 启用全部嵌入式功能(计步, 倾斜, 有效运动)
#define LSM6DS3TR_C_PEDO_RST_STEP           (0x02) // 1: 重置计步器步数
#define LSM6DS3TR_C_SIGN_MOTION_EN          (0x01) // 启用有效运动检测

/* --- STATUS_REG (1Eh) : 数据状态查询 [cite: 1359] --- */
#define LSM6DS3TR_C_STS_TDA                 (0x04) // 1: 有新的温度数据
#define LSM6DS3TR_C_STS_GDA                 (0x02) // 1: 有新的陀螺仪数据
#define LSM6DS3TR_C_STS_XLDA                (0x01) // 1: 有新的加速度数据

#ifdef __cplusplus
}
#endif

#endif /* __LSM6DS3TR_C_REGS_H__ */