# CWatch 项目结构说明

本项目是基于 RT-Thread、Sifli SDK 和 LVGL 的智能手表应用工程。

## 目录结构

```text
CWatch/
|- src/                         # 核心源码
|  |- main.c                    # 程序入口
|  |- app/                      # 应用层（数据、任务、BLE、UI 刷新）
|  |- drivers/                  # 传感器与外设驱动
|  `- ui/                       # LVGL 界面代码（页面/资源/组件）
|- project/                     # 构建系统与工程配置（SCons/Kconfig）
|  |- SConstruct                # 顶层构建入口
|  |- SConscript                # 组织 SDK 与 src 的构建脚本
|  |- proj.conf                 # 工程配置
|  `- build_CWatch_hcpu/        # 构建输出（自动生成）
`- image/                       # 独立图像脚本与 ezip 资源
```

## 主启动流程

`src/main.c` 按如下顺序初始化：
1. `DataHub_Init()`
2. `SensorTask_Start()`
3. `lvgl_start()`
4. `ble_app_init()`

## 模块职责

### `src/app/`
- `data_hub.*`：共享运行时数据模型（`system_data`）与互斥锁（`data_mutex`）
- `sensor_task.*`：传感器采集线程（IMU、环境、心率血氧、磁力计、RTC）
- `ui_update.*`：周期刷新 UI，并按页面动态启停采集项
- `app_ble.*` 与 `ble_rx.*`：BLE 服务、广播、连接事件、手机数据接收
- `compass.*`：基于运动与磁力数据计算航向角

### `src/drivers/`
- `BME280/`：温度、气压、湿度
- `LSM6DS3TR/`：加速度计与陀螺仪
- `MAX30102/`：心率与血氧
- `QMC5883/`：磁力计
- `RTC/`：实时时钟
- `sensor_i2c/`：传感器共用 I2C 抽象层

### `src/ui/`
- `ui.c/ui.h`、`ui_start.c`：UI 初始化与 LVGL 线程启动
- `screens/`：生成页面与自定义页面（`ui_Screen1...7`、天气页等）
- `images/`、`fonts/`：LVGL 图片/字体 C 数组资源
- `components/`：UI 组件钩子
- `honeycomb.*`：蜂窝启动器布局逻辑

## 运行时数据流

1. 传感器线程采集数据并写入 `system_data`。
2. UI 定时器根据当前页面启用所需采集组。
3. UI 读取数据快照并刷新控件。
4. BLE 接收手机侧数据（如天气/时间）并更新 `system_data` 与 RTC。

## 结构说明与建议

- `project/build_CWatch_hcpu/` 属于构建产物，应视为生成目录。
- `image/ezip` 与 `src/ui/images` 同时存放图片资源，后续建议统一单一来源。
- 建议后续补充：
  1. `docs/`：协议、页面与传感器映射文档
  2. `tools/`：资源转换脚本
  3. CI 构建脚本：提高构建可复现性
