#include <rtthread.h>
#include <string.h>
#include "ble_rx.h"
#include "RTC_T.h" // 包含你手表的 RTC 头文件
#include "data_hub.h" // 包含全局数据中心的头文件

// 数据处理中心
void ble_rx_handler(rt_uint8_t *data, rt_uint16_t len)
{
    // 安全性检查：如果数据为空或长度为0，直接退出
    if (data == RT_NULL || len == 0) return;

    /* * ==========================================
     * 业务一：时间同步处理 (长度为 7 字节)
     * ==========================================
     */
    if (len == 7) 
    {
        rt_uint16_t year = (data[0] << 8) | data[1]; 
        rt_uint8_t month = data[2];
        rt_uint8_t day   = data[3];
        rt_uint8_t hour  = data[4];
        rt_uint8_t min   = data[5];
        rt_uint8_t sec   = data[6];

        rt_kprintf("[Protocol] Sync Time: %04d-%02d-%02d %02d:%02d:%02d\r\n", year, month, day, hour, min, sec);

        // 调用底层的 RTC 函数更新手表时间
        RTC_SetTime(year, month, day, hour, min, sec);
        rt_kprintf("[Protocol] RTC Updated successfully!\r\n");
    }
    /* * ==========================================
     * 业务二：天气指令处理 (根据你之前的测试：3字节，包头为 0x01)
     * ==========================================
     */
    else if (len == 3 && data[0] == 0x01)
    {
        rt_uint8_t temp = data[1];       // 比如 0x1A 是 26度
        rt_uint8_t weather_type = data[2]; // 比如 0x02 是晴天

        rt_kprintf("[Protocol] Weather received -> Temp: %d, Type: %d\r\n", temp, weather_type);
        // 这里以后可以调用更新 UI 屏幕的函数，比如：
        // UI_Update_Weather(temp, weather_type);
    }
    /* * ==========================================
     * 业务三：豪华版今日天气 (包头为 0x02)
     * ==========================================
     */
   else if (data[0] == 0x02 && len >= 7)
    {
        rt_uint8_t weather_type = data[1];
        rt_int8_t current_temp  = (rt_int8_t)data[2]; // 当前实时温度！
        rt_int8_t max_temp      = (rt_int8_t)data[3];
        rt_int8_t min_temp      = (rt_int8_t)data[4];
        rt_uint16_t aqi         = (data[5] << 8) | data[6];

        // 提取中文城市名
        char city_name[16] = {0}; 
        rt_uint8_t city_len = len - 7; // 头长度变成了 7
        
        if (city_len > 0 && city_len < 16) {
            rt_memcpy(city_name, &data[7], city_len); 
            city_name[city_len] = '\0'; 
        } else 
        {
            strcpy(city_name, "未知");
        }

        if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
        {
            // 拷贝字符串
            strcpy(system_data.city_name, city_name);
            // 赋值各类天气参数
            system_data.weather_type = weather_type;
            system_data.current_temp = current_temp;
            system_data.max_temp = max_temp;
            system_data.min_temp = min_temp;
            system_data.aqi = aqi;
            
            system_data.enable_weather = 1; 

            rt_mutex_release(data_mutex);
        }

        rt_kprintf("[Protocol] Full Weather received:\r\n");
        rt_kprintf(" - City: %s\r\n", city_name);
        rt_kprintf(" - Icon Enum: %d\r\n", weather_type);
        rt_kprintf(" - Current Temp: %d C\r\n", current_temp);
        rt_kprintf(" - Range: %d ~ %d C\r\n", min_temp, max_temp);
        rt_kprintf(" - AQI: %d\r\n", aqi);
        
        // 此处调用你的 LVGL/SquareLine 更新界面的函数
    }
    /* * ==========================================
     * 业务四：未来7天天气分包接收 (包头 0x03)
     * ==========================================
     */
    else if (data[0] == 0x03 && len >= 11) 
    {
        rt_uint8_t page = data[1]; // 获取页码

        if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (page == 0x00) // 第 0 页 (包含第 0~3 天)
            {
                for(int i=0; i<4; i++) {
                    system_data.forecast[i].weather_type = data[2 + i*3];
                    system_data.forecast[i].max_temp = (rt_int8_t)data[3 + i*3];
                    system_data.forecast[i].min_temp = (rt_int8_t)data[4 + i*3];
                }
            }
            else if (page == 0x01) // 第 1 页 (包含第 4~6 天)
            {
                for(int i=0; i<3; i++) {
                    // 注意偏移量：数组下标从4开始
                    system_data.forecast[i+4].weather_type = data[2 + i*3];
                    system_data.forecast[i+4].max_temp = (rt_int8_t)data[3 + i*3];
                    system_data.forecast[i+4].min_temp = (rt_int8_t)data[4 + i*3];
                }
                
                // 收到最后一页，标记 7 天数据组装完毕！通知 UI 刷新！
                system_data.enable_forecastweather = 1; 
                rt_kprintf("[BLE] 7-Day Forecast Assembly Complete!\r\n");
                rt_kprintf("\r\n====== 7 day weather ======\r\n");
                for (int j = 0; j < 7; j++) 
                {
                    rt_kprintf("day %d  | type: %d | %d°C ~ %d°C\r\n", 
                                j + 1, 
                                system_data.forecast[j].weather_type, 
                                system_data.forecast[j].min_temp, 
                                system_data.forecast[j].max_temp);
                }
                rt_kprintf("===============================\r\n\r\n");
            }
            rt_mutex_release(data_mutex);
        }
    }
    /* * ==========================================
     * 业务五：24小时逐小时天气分包接收 (包头为 0x04)
     * ==========================================
     */
    else if (data[0] == 0x04 && len >= 20) 
    {
        rt_uint8_t page = data[1]; // 获取页码 (0, 1, 2, 3)

        if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
        {
            // 如果页码合法
            if (page >= 0 && page <= 3) 
            {
                // 一页包含 6 个小时的数据
                for(int i = 0; i < 6; i++) {
                    int global_idx = page * 6 + i;
                    system_data.hourly_forecast[global_idx].hour = data[2 + i*3];
                    system_data.hourly_forecast[global_idx].weather_type = data[3 + i*3];
                    system_data.hourly_forecast[global_idx].temp = (rt_int8_t)data[4 + i*3];
                }
                
                rt_kprintf("[BLE] 收到 24h 预报：第 %d 包\r\n", page + 1);

                // 当收到最后一包 (page == 3) 时，打印完整报告
                if (page == 3) 
                {
                    system_data.enable_hourlyweather = 1; // 标记全部接收完毕
                    
                    rt_kprintf("\r\n======  24 hour data ======\r\n");
                    for (int j = 0; j < 24; j++) 
                    {
                        rt_kprintf("%02d:00 | type: %d | temp: %d°C\r\n", 
                                    system_data.hourly_forecast[j].hour, 
                                    system_data.hourly_forecast[j].weather_type, 
                                    system_data.hourly_forecast[j].temp);
                    }
                    rt_kprintf("==================================\r\n\r\n");
                }
            }
            rt_mutex_release(data_mutex);
        }
    }
    /* * ==========================================
     * 其他未知数据（兜底打印，方便调试）
     * ==========================================
     */
    else 
    {
        rt_kprintf("[Protocol] Unknown data received, len: %d\r\nData: ", len);
        for(int i = 0; i < len; i++) {
            rt_kprintf("%02X ", data[i]);
        }
        rt_kprintf("\r\n");
    }
}