#include "rtthread.h"
#include "compass.h"
#include "math.h"

#define PI 3.14159265f
#define FILTER_COEFFICIENT 0.15f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

#ifndef COMPASS_RUNTIME_LOG
#define COMPASS_RUNTIME_LOG 1
#endif

#if COMPASS_RUNTIME_LOG
#define COMPASS_LOG(...) rt_kprintf(__VA_ARGS__)
#else
#define COMPASS_LOG(...) do { } while (0)
#endif

#define COMPASS_MOUNT_OFFSET_DEG    (-90.0f)
#define COMPASS_DECLINATION_DEG     (0.0f)
#define COMPASS_DEFAULT_DT_S        (0.02f)
#define COMPASS_MIN_DT_S            (0.005f)
#define COMPASS_MAX_DT_S            (0.10f)
#define COMPASS_ACCEL_NORM_MIN_MG   (700.0f)
#define COMPASS_ACCEL_NORM_MAX_MG   (1300.0f)
#define COMPASS_MAG_MIN_SPAN_LSB    (300.0f)
#define COMPASS_MAG_CAL_MIN_SAMPLES (250U)
#define COMPASS_MAG_CAL_MIN_SPAN_XY (1200.0f)
#define COMPASS_MAG_CAL_MIN_SPAN_Z  (600.0f)
#define COMPASS_MAG_CAL_MAX_RATIO   (2.5f)
#define COMPASS_LOG_PERIOD_MS       (200U)

typedef struct
{
    rt_uint8_t initialized;
    rt_uint8_t heading_ready;
    rt_uint8_t mag_calibrated;
    rt_uint32_t mag_samples;
    rt_tick_t last_tick;
    rt_tick_t last_log_tick;
    float roll_rad;
    float pitch_rad;
    float heading_deg;
    float heading_unwrapped_deg;
    float mag_min[3];
    float mag_max[3];
    float mag_offset[3];
    float mag_offset_active[3];
    float mag_span[3];
    float filt_xh;
    float filt_yh;
} compass_state_t;

static compass_state_t g_compass;

static float compass_clampf(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static float compass_absf(float value)
{
    return (value >= 0.0f) ? value : -value;
}

static float compass_wrap_360(float angle_deg)
{
    while (angle_deg >= 360.0f)
    {
        angle_deg -= 360.0f;
    }
    while (angle_deg < 0.0f)
    {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

static float compass_wrap_180(float angle_deg)
{
    while (angle_deg > 180.0f)
    {
        angle_deg -= 360.0f;
    }
    while (angle_deg <= -180.0f)
    {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

static float compass_maxf(float a, float b)
{
    return (a > b) ? a : b;
}

static float compass_minf(float a, float b)
{
    return (a < b) ? a : b;
}

static void compass_update_mag_calibration(compass_state_t *state, float mx, float my, float mz)
{
    float mag[3] = {mx, my, mz};
    float xy_ratio;
    rt_uint8_t i;

    if (state->mag_samples == 0U)
    {
        for (i = 0; i < 3; i++)
        {
            state->mag_min[i] = mag[i];
            state->mag_max[i] = mag[i];
            state->mag_offset[i] = mag[i];
            state->mag_span[i] = 0.0f;
        }
        state->mag_samples = 1U;
        state->mag_calibrated = 0U;
        return;
    }

    for (i = 0; i < 3; i++)
    {
        if (mag[i] < state->mag_min[i])
        {
            state->mag_min[i] = mag[i];
        }
        if (mag[i] > state->mag_max[i])
        {
            state->mag_max[i] = mag[i];
        }

        state->mag_offset[i] = (state->mag_max[i] + state->mag_min[i]) * 0.5f;
        state->mag_span[i] = state->mag_max[i] - state->mag_min[i];
    }

    state->mag_samples++;

    if ((state->mag_span[0] > COMPASS_MAG_MIN_SPAN_LSB) &&
        (state->mag_span[1] > COMPASS_MAG_MIN_SPAN_LSB))
    {
        xy_ratio = compass_maxf(state->mag_span[0], state->mag_span[1]) /
                   compass_minf(state->mag_span[0], state->mag_span[1]);

        if ((state->mag_samples >= COMPASS_MAG_CAL_MIN_SAMPLES) &&
            (state->mag_span[0] >= COMPASS_MAG_CAL_MIN_SPAN_XY) &&
            (state->mag_span[1] >= COMPASS_MAG_CAL_MIN_SPAN_XY) &&
            (state->mag_span[2] >= COMPASS_MAG_CAL_MIN_SPAN_Z) &&
            (xy_ratio <= COMPASS_MAG_CAL_MAX_RATIO))
        {
            for (i = 0; i < 3; i++)
            {
                state->mag_offset_active[i] = state->mag_offset[i];
            }
            state->mag_calibrated = 1U;
        }
    }
}

void Compass_Reset(void)
{
    rt_memset(&g_compass, 0, sizeof(g_compass));
}

const char *Compass_GetDirectionText(float heading_deg)
{
    static const char *direction_text[] =
    {
        "\xE5\x8C\x97",
        "\xE4\xB8\x9C\xE5\x8C\x97",
        "\xE4\xB8\x9C",
        "\xE4\xB8\x9C\xE5\x8D\x97",
        "\xE5\x8D\x97",
        "\xE8\xA5\xBF\xE5\x8D\x97",
        "\xE8\xA5\xBF",
        "\xE8\xA5\xBF\xE5\x8C\x97",
    };
    int index;

    heading_deg = compass_wrap_360(heading_deg);
    index = (int)((heading_deg + 22.5f) / 45.0f);
    index &= 0x07;

    return direction_text[index];
}

#if 0
float Compass_Calculate(float ax, float ay, float az, float mx, float my, float mz)
{
    // 1. 根据加速度计计算横滚角(Roll)和俯仰角(Pitch)
    float roll = atan2f(ay, az);
    float pitch = atan2f(-ax, sqrtf(ay * ay + az * az));

    // 2. 倾角补偿：将磁力计数据投影到水平面
    float Xh = mx * cosf(pitch) + my * sinf(roll) * sinf(pitch) + mz * cosf(roll) * sinf(pitch);
    float Yh = my * cosf(roll) - mz * sinf(roll);

    // 3. 计算航向角 (Heading)
    float heading = atan2f(-Yh, Xh) * (180.0f / PI);

    float offset_angle = -90.0f; // <--- 把这个数字改成你实际测试出来的偏差值！
    heading += offset_angle;

    // 4. 将角度转换为 0-360 度
    if (heading < 0.0f) 
    {
        heading += 360.0f;
    }

    // 4. 一阶低通滤波 (消除指针发抖)
    // 静态变量，作用域仅在此函数内，但生命周期贯穿程序全局，用于保存上一次的角度
    static float current_heading = -1.0f; 

    // 如果是第一次运行，直接赋值，不需要滤波过渡
    if (current_heading < 0.0f) {
        current_heading = heading;
    } else {
        // 计算目标角度和当前角度的差值
        float diff = heading - current_heading;
        
        // 处理 360 度过零点问题 (防止指针反转一圈)
        if (diff > 180.0f)  diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;
        
        // 应用滤波公式
        current_heading += diff * FILTER_COEFFICIENT;
        
        // 规整角度到 0-360 范围内
        if (current_heading >= 360.0f) current_heading -= 360.0f;
        if (current_heading < 0.0f)    current_heading += 360.0f;
    }

    return current_heading;
}
#endif

float Compass_Calculate(float ax, float ay, float az,
                        float gx, float gy, float gz,
                        float mx, float my, float mz)
{
    compass_state_t *state = &g_compass;
    rt_tick_t now_tick = rt_tick_get();
    float dt = COMPASS_DEFAULT_DT_S;
    float acc_norm;
    rt_uint8_t accel_valid;
    float roll_acc_rad;
    float pitch_acc_rad;
    float mx_corr;
    float my_corr;
    float mz_corr;
    float xh;
    float yh;
    float heading_meas_deg;
    float mag_alpha;
    float heading_error_deg;
    float heading_alpha;

    if (state->initialized)
    {
        rt_tick_t delta_tick = now_tick - state->last_tick;
        if (delta_tick > 0U)
        {
            dt = (float)delta_tick / (float)RT_TICK_PER_SECOND;
            dt = compass_clampf(dt, COMPASS_MIN_DT_S, COMPASS_MAX_DT_S);
        }
    }

    compass_update_mag_calibration(state, mx, my, mz);

    if (!state->initialized)
    {
        if ((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))
        {
            az = 1000.0f;
        }

        state->roll_rad = atan2f(ay, az);
        state->pitch_rad = atan2f(-ax, sqrtf(ay * ay + az * az));
        state->last_tick = now_tick;
        state->last_log_tick = now_tick;
        state->initialized = 1U;
    }
    else
    {
        state->roll_rad += gx * DEG2RAD * dt;
        state->pitch_rad += gy * DEG2RAD * dt;
    }

    acc_norm = sqrtf(ax * ax + ay * ay + az * az);
    accel_valid = (acc_norm >= COMPASS_ACCEL_NORM_MIN_MG) &&
                  (acc_norm <= COMPASS_ACCEL_NORM_MAX_MG);

    if (accel_valid)
    {
        float accel_blend = 0.06f;
        roll_acc_rad = atan2f(ay, az);
        pitch_acc_rad = atan2f(-ax, sqrtf(ay * ay + az * az));
        state->roll_rad += (roll_acc_rad - state->roll_rad) * accel_blend;
        state->pitch_rad += (pitch_acc_rad - state->pitch_rad) * accel_blend;
    }

    mx_corr = mx - state->mag_offset_active[0];
    my_corr = my - state->mag_offset_active[1];
    mz_corr = mz - state->mag_offset_active[2];

    xh = mx_corr * cosf(state->pitch_rad) +
         my_corr * sinf(state->roll_rad) * sinf(state->pitch_rad) +
         mz_corr * cosf(state->roll_rad) * sinf(state->pitch_rad);
    yh = my_corr * cosf(state->roll_rad) -
         mz_corr * sinf(state->roll_rad);

    if ((compass_absf(xh) < 1.0f) && (compass_absf(yh) < 1.0f))
    {
        state->last_tick = now_tick;
        return state->heading_deg;
    }

    if ((state->filt_xh == 0.0f) && (state->filt_yh == 0.0f))
    {
        state->filt_xh = xh;
        state->filt_yh = yh;
    }
    else
    {
        mag_alpha = 0.10f + compass_absf(gz) * 0.004f;
        if (!state->mag_calibrated)
        {
            mag_alpha += 0.05f;
        }
        mag_alpha = compass_clampf(mag_alpha, 0.08f, 0.45f);
        state->filt_xh += (xh - state->filt_xh) * mag_alpha;
        state->filt_yh += (yh - state->filt_yh) * mag_alpha;
    }

    heading_meas_deg = atan2f(-state->filt_yh, state->filt_xh) * RAD2DEG;
    heading_meas_deg += COMPASS_MOUNT_OFFSET_DEG + COMPASS_DECLINATION_DEG;
    heading_meas_deg = compass_wrap_360(heading_meas_deg);

    if (!state->heading_ready)
    {
        state->heading_unwrapped_deg = heading_meas_deg;
        state->heading_deg = heading_meas_deg;
        state->heading_ready = 1U;
    }
    else
    {
        heading_error_deg = compass_wrap_180(heading_meas_deg - state->heading_deg);
        heading_alpha = 0.12f +
                        compass_absf(heading_error_deg) / 90.0f * 0.30f +
                        compass_absf(gz) / 180.0f * 0.20f;
        heading_alpha = compass_clampf(heading_alpha, 0.10f, 0.60f);
        state->heading_unwrapped_deg += heading_error_deg * heading_alpha;
        state->heading_deg = compass_wrap_360(state->heading_unwrapped_deg);
    }

    if ((rt_tick_get() - state->last_log_tick) >= rt_tick_from_millisecond(COMPASS_LOG_PERIOD_MS))
    {
        COMPASS_LOG("COMPASS hdg=%.1f raw=%.1f roll=%.1f pitch=%.1f gz=%.1f acc=%.0f aok=%d cal=%d "
                    "mag=(%.0f,%.0f,%.0f) off=(%.0f,%.0f,%.0f) span=(%.0f,%.0f,%.0f)\n",
                    state->heading_deg,
                    heading_meas_deg,
                    state->roll_rad * RAD2DEG,
                    state->pitch_rad * RAD2DEG,
                    gz,
                    acc_norm,
                    (int)accel_valid,
                    (int)state->mag_calibrated,
                    mx, my, mz,
                    state->mag_offset_active[0], state->mag_offset_active[1], state->mag_offset_active[2],
                    state->mag_span[0], state->mag_span[1], state->mag_span[2]);
        state->last_log_tick = rt_tick_get();
    }

    state->last_tick = now_tick;
    return state->heading_deg;
}
