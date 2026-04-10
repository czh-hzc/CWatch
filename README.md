# CWatch Project Structure

This project is a smartwatch application built on RT-Thread, Sifli SDK, and LVGL.

## Directory Layout

```text
CWatch/
|- src/                         # Core source code
|  |- main.c                    # Program entry
|  |- app/                      # App layer (data, tasks, BLE, UI update)
|  |- drivers/                  # Sensor and peripheral drivers
|  `- ui/                       # LVGL UI code (screens/assets/components)
|- project/                     # Build system and project config (SCons/Kconfig)
|  |- SConstruct                # Top-level build entry
|  |- SConscript                # Combines SDK and src build scripts
|  |- proj.conf                 # Project config
|  `- build_CWatch_hcpu/        # Build outputs (generated)
`- image/                       # Separate image scripts and ezip assets
```

## Main Startup Flow

`src/main.c` initializes modules in this order:
1. `DataHub_Init()`
2. `SensorTask_Start()`
3. `lvgl_start()`
4. `ble_app_init()`

## Module Responsibilities

### `src/app/`
- `data_hub.*`: shared runtime data model (`system_data`) and mutex (`data_mutex`)
- `sensor_task.*`: sensor collection thread (IMU, env, heart/spo2, magnetometer, RTC)
- `ui_update.*`: periodic UI refresh and per-screen sensor enable flags
- `app_ble.*` and `ble_rx.*`: BLE service, advertising, connection events, phone data receive
- `compass.*`: heading calculation from motion and magnetometer data

### `src/drivers/`
- `BME280/`: temperature, pressure, humidity
- `LSM6DS3TR/`: accelerometer and gyroscope
- `MAX30102/`: heart rate and SpO2
- `QMC5883/`: magnetometer
- `RTC/`: realtime clock
- `sensor_i2c/`: shared I2C abstraction for sensors

### `src/ui/`
- `ui.c/ui.h`, `ui_start.c`: UI setup and LVGL thread startup
- `screens/`: generated and custom screen files (`ui_Screen1...7`, weather screens)
- `images/`, `fonts/`: LVGL image/font resources in C arrays
- `components/`: UI component hooks
- `honeycomb.*`: honeycomb launcher layout logic

## Runtime Data Flow

1. Sensor thread collects data and writes into `system_data`.
2. UI timer checks active screen and enables needed sensor groups.
3. UI reads a data snapshot and updates widgets.
4. BLE receives phone-side data (for example weather/time) and updates `system_data` and RTC.

## Structure Notes

- `project/build_CWatch_hcpu/` is generated output and should be treated as build artifacts.
- `image/ezip` and `src/ui/images` both contain image resources; consider unifying one source of truth later.
- Suggested next organization steps:
  1. Add `docs/` for protocol/UI/sensor mapping notes.
  2. Add `tools/` for asset conversion scripts.
  3. Add CI build scripts for reproducible builds.
