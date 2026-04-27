# CWatch 项目说明

CWatch 是一个基于 RT-Thread、SiFli SDK 和 LVGL 的智能手表应用工程。仓库主体是手表固件源码与 SiFli/RT-Thread 工程配置，本地同时保留微信小程序工程用于 BLE 联调和天气、时间数据同步。

当前主要包含：

- 手表固件工程
- LVGL 手表 UI
- 传感器采集与共享数据中心
- BLE 数据同步链路
- 经典蓝牙音频连接模板
- 微信小程序联调工程

## 目录结构

```text
CWatch/
|- src/                         # 核心源码
|  |- main.c                    # 程序入口
|  |- SConscript                # 应用源码构建清单
|  |- app/                      # 应用层：数据、任务、蓝牙、UI 刷新
|  |- drivers/                  # 传感器与外设驱动
|  `- ui/                       # LVGL 界面代码、页面、资源、组件
|- project/                     # 构建系统与工程配置
|  |- SConstruct                # 顶层构建入口
|  |- SConscript                # 组织 SDK 与 src 的构建脚本
|  |- proj.conf                 # 工程配置
|  |- rtconfig.py               # RT-Thread/SiFli 构建配置
|  `- build_CWatch_hcpu/        # 构建产物目录，不应手动维护
|- CWatch_vxxcx/                # 微信小程序工程，用于 BLE 联调
|  |- app.js
|  |- app.json
|  |- app.wxss
|  |- project.config.json
|  |- sitemap.json
|  |- pages/
|  `- utils/
|- AGENTS.md                    # 本仓库协作约定
|- .gitignore
`- README.md
```

## 手表侧功能

- 传感器采集：心率、血氧、温度、气压、湿度、海拔、IMU、磁力计、RTC
- 电池信息读取：主表盘显示电量百分比和电量等级
- LVGL 本地界面显示：主表盘、心率页、指南针页、环境页、天气页等
- 页面驱动采集：根据当前页面动态启停对应传感器，降低无关采样开销
- BLE 广播、连接与 GATT 数据接收
- 接收手机侧时间、当天天气、7 天天气、24 小时天气数据
- 经典蓝牙音频连接模板，当前用于后续扩展预留

## 小程序侧功能

- 打开首页后自动初始化手机蓝牙
- 自动查找并连接名为 `CWatch` 的 BLE 设备
- BLE 通道建立后自动同步：
  - 当前时间
  - 当前城市天气、温度、AQI
  - 7 天天气预报
  - 24 小时逐小时天气
- 保留手动补发入口，便于自动同步失败后重试

## 主启动流程

`src/main.c` 当前按如下顺序初始化：

1. `DataHub_Init()`
2. `SensorTask_Start()`
3. `lvgl_start()`
4. `bt_classic_app_init()`
5. `ble_app_init()`

其中：

- `DataHub_Init()` 初始化共享运行时数据与互斥锁
- `SensorTask_Start()` 创建传感器采集线程
- `lvgl_start()` 启动 LVGL 和 UI 刷新流程
- `bt_classic_app_init()` 注册经典蓝牙相关事件，当前作为音频类连接模板保留
- `ble_app_init()` 负责 BLE 使能、服务初始化和广播启动

## 模块职责

### `src/app/`

- `data_hub.*`：共享运行时数据模型 `system_data` 与互斥锁 `data_mutex`
- `sensor_task.*`：传感器采集线程，负责 IMU、环境、心率血氧、磁力计、RTC 数据读取
- `ui_update.*`：周期刷新 UI，并按页面动态启停采集项
- `app_ble.*`：BLE 初始化、广播、连接事件与服务启动
- `ble_rx.*`：BLE 收包，解析时间、当天天气、7 天天气、24 小时天气
- `app_bt_classic.*`：经典蓝牙音频连接模板
- `battery_adc.*`：电池 ADC 读取、电压、电量百分比和等级换算
- `compass.*`：基于 IMU 与磁力计数据计算航向角和方向文本

### `src/drivers/`

- `BME280/`：温度、气压、湿度、海拔
- `LSM6DS3TR/`：加速度计与陀螺仪
- `MAX30102/`：心率与血氧
- `QMC5883/`：磁力计
- `RTC/`：实时时钟
- `sensor_i2c/`：传感器共用 I2C 抽象层

### `src/ui/`

- `ui.c`、`ui.h`、`ui_start.c`：UI 初始化与 LVGL 线程启动
- `screens/`：页面代码，包括主表盘、心率、指南针、环境、天气和关于页等
- `images/`、`fonts/`：LVGL 图片和字体 C 数组资源
- `components/`：UI 组件钩子
- `honeycomb.*`：蜂窝启动器布局逻辑

### `CWatch_vxxcx/`

- `pages/index/`：蓝牙连接与同步首页
- `pages/logs/`：小程序默认日志页
- `utils/util.js`：基础工具函数

## BLE 数据同步

当前工程采用 BLE 与经典蓝牙并存的方式：

- BLE 用于小程序数据通信，负责时间、天气等业务数据同步
- 经典蓝牙当前保留为音频类连接模板，不替代 BLE 数据通道

小程序侧当前使用：

- 设备名：`CWatch`
- Service UUID：`00000000-0000-0000-0000-000000003412`
- Characteristic UUID：`00000000-0000-0000-0000-00000000CDAB`

数据包约定：

- 7 字节无命令头：时间同步，格式为 `year(2) + month + day + hour + minute + second`
- `0x02`：当天天气，包含天气类型、当前温度、最高温、最低温、AQI、城市名
- `0x03`：7 天天气，分 2 包发送
- `0x04`：24 小时天气，分 4 包发送，每包 6 小时数据

注意：手机系统蓝牙连接成功，不等于小程序已经拿到 BLE 数据通道。小程序的数据同步仍依赖 BLE 服务与特征值发现成功。

## 运行时数据流

1. 传感器线程初始化各传感器和 RTC
2. UI 定时器根据当前页面设置 `system_data` 中的采集开关
3. 传感器线程按采集开关读取数据，并写入 `system_data`
4. UI 读取数据快照并刷新控件
5. BLE 接收手机侧数据，并更新天气、预报、RTC 等运行时数据
6. 小程序在 BLE 通道就绪后自动发起时间和天气同步

## 构建与运行

### 固件工程

构建依赖 SiFli SDK 与 RT-Thread/SCons 构建环境。`project/SConstruct` 会读取环境变量 `SIFLI_SDK`，如果未设置会提示先执行 SiFli SDK 根目录下的 `set_env.bat`。

常见流程：

```bat
cd /d D:\path\to\sifli_sdk
set_env.bat
cd /d D:\CWatch\code\CWatch\project
scons
```

构建输出位于 `project/build_CWatch_hcpu/`，其中 `main.bin`、`main.hex`、`download.bat`、`uart_download.bat` 等文件由构建过程生成。

### 微信小程序

使用微信开发者工具打开 `CWatch_vxxcx/` 目录即可。当前小程序用于本地联调，天气数据依赖定位权限和和风天气接口配置。

## 版本管理说明

当前 `.gitignore` 采用“默认忽略所有文件，再显式纳入核心工程文件”的策略：

- 默认纳入：`README.md`、`src/`、`project/` 下的核心配置和构建脚本
- 默认忽略：本地编辑器配置、构建产物、缓存、日志、临时文件
- `CWatch_vxxcx/` 当前未显式纳入版本管理，主要作为本地联调用工程

如果后续需要提交或单独开源小程序，需要同步调整 `.gitignore`。

## 维护建议

- 不要手动修改 `project/build_CWatch_hcpu/` 下的构建产物
- 新增传感器或页面时，优先复用 `system_data`、采集开关和 `ui_update.*` 的现有流程
- 新增源文件后，确认 `src/SConscript` 的 `Glob` 路径能够覆盖该文件
- 修改 BLE 协议时，需要同时更新 `src/app/ble_rx.c` 和 `CWatch_vxxcx/pages/index/index.js`
- 经典蓝牙当前只是连接模板；若要实现真正音频播放，还需要继续补齐音频输出链路

