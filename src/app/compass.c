#include "rtthread.h"
#include "compass.h"
#include "math.h"

#define PI 3.14159265f
#define FILTER_COEFFICIENT 0.15f

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