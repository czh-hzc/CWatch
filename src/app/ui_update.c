#include "rtthread.h"
#include "lvgl.h"
#include "ui.h"
#include "data_hub.h"
#include "stdio.h"
#include "ui_update.h"
#include "compass.h"
#include "RTC_T.h"

static void ui_update_timer_cb(lv_timer_t * timer)
{
    char buf[32];

    lv_obj_t * current_screen = lv_scr_act();

    if (current_screen == ui_Screen3) 
    {
        lv_timer_set_period(timer, 20); // 航向角需要更频繁更新，20ms一次
    } 
    else 
    {
        lv_timer_set_period(timer, 500);// 其他屏幕500ms更新一次即可
    }

    rt_uint16_t year; 
    rt_uint8_t month, day, hour, minute, second, weekday;
    int local_heart_rate = 0, local_spo2 = 0;
    float local_temp = 0, local_press = 0, local_hum = 0;
    float local_ax = 0, local_ay = 0, local_az = 0;
    float local_gx = 0, local_gy = 0, local_gz = 0;
    float local_mx = 0, local_my = 0, local_mz = 0;
    char local_city[16] = {0};
    rt_int8_t local_cur_temp = 0, local_max_temp = 0, local_min_temp = 0;
    rt_uint16_t local_aqi = 0;
    rt_uint8_t local_weather_type = 0;
    daily_weather_t local_forecast[7];
    hourly_weather_t local_hourly[24];

    if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
    {
        system_data.enable_environment = 0;
        system_data.enable_heartrate = 0;
        system_data.enable_mag = 0;
        system_data.enable_imu = 0;
        system_data.enable_rtc = 0;
        
        if(current_screen == ui_Screen1)
        {
            system_data.enable_heartrate = 1;
            system_data.enable_rtc = 1;
        }
        else if(current_screen == ui_Screen2)
        {
            system_data.enable_heartrate = 1;
        }
        else if(current_screen == ui_Screen3)
        {
            system_data.enable_imu = 1;
            system_data.enable_mag = 1;
        }
        else if (current_screen == ui_Screen5) 
        {
            system_data.enable_environment = 1;
        }

        local_heart_rate = system_data.heart_rate;
        local_spo2 = system_data.spo2;
        local_temp = system_data.temperature;
        local_press = system_data.pressure;
        local_hum = system_data.humidity;
        
        local_ax = system_data.acc_x; local_ay = system_data.acc_y; local_az = system_data.acc_z;
        local_gx = system_data.gyro_x; local_gy = system_data.gyro_y; local_gz = system_data.gyro_z;
        local_mx = system_data.mag_x; local_my = system_data.mag_y; local_mz = system_data.mag_z;

        year = system_data.year;
        month = system_data.month;
        day = system_data.day;
        hour = system_data.hour; 
        minute = system_data.minute; 
        second = system_data.second; 
        weekday = system_data.weekday;

        rt_strncpy(local_city, system_data.city_name, sizeof(local_city));
        local_cur_temp = system_data.current_temp;
        local_max_temp = system_data.max_temp;
        local_min_temp = system_data.min_temp;
        local_aqi = system_data.aqi;
        local_weather_type = system_data.weather_type;

        memcpy(local_forecast, system_data.forecast, sizeof(local_forecast));
        memcpy(local_hourly, system_data.hourly_forecast, sizeof(local_hourly));

        rt_mutex_release(data_mutex);
    }

    if(current_screen == ui_Screen1)
    {
        const char *week_str[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};

        snprintf(buf, sizeof(buf), "%d", (int)local_heart_rate);
        lv_label_set_text(ui_heart_label, buf);
        snprintf(buf, sizeof(buf), "%d", (int)local_spo2);
        lv_label_set_text(ui_spo2_label, buf);
        snprintf(buf, sizeof(buf), "%02d", year);
        lv_label_set_text(ui_year, buf);
        snprintf(buf, sizeof(buf), "%02d", month);
        lv_label_set_text(ui_month, buf);
        snprintf(buf, sizeof(buf), "%02d", day);
        lv_label_set_text(ui_day, buf);
        snprintf(buf, sizeof(buf), "%02d", hour);
        lv_label_set_text(ui_hour, buf);
        snprintf(buf, sizeof(buf), "%02d", minute);
        lv_label_set_text(ui_min, buf);
        snprintf(buf, sizeof(buf), "%s", week_str[weekday]);
        lv_label_set_text(ui_week, buf);

    }
    else if(current_screen == ui_Screen2)
    {
        snprintf(buf, sizeof(buf), "%d", (int)local_heart_rate);
        lv_label_set_text(ui_heart_rate, buf);
        // lv_label_set_text_fmt(ui_heart_rate, "%d", (int)local_heart_rate);
    }
    else if (current_screen == ui_Screen3) 
    {
    // 传入局部变量数据，计算航向角
        float heading = Compass_Calculate(local_ax, local_ay, local_az, local_mx, local_my, local_mz);

        lv_img_set_angle(ui_Image7, (int16_t)(360.0f - heading * 10)); 

        snprintf(buf, sizeof(buf), "%d°", (int)heading);
        lv_label_set_text(ui_Label5, buf);
    }
    else if (current_screen == ui_Screen5) 
    {
        snprintf(buf, sizeof(buf), "%df°", (int)local_temp);
        lv_label_set_text(ui_temp_test, buf);
        snprintf(buf, sizeof(buf), "%dfhpa", (int)local_press);
        lv_label_set_text(ui_press_test, buf);
        snprintf(buf, sizeof(buf), "%d%%", (int)local_hum);
        lv_label_set_text(ui_hum_test, buf);
    
    }
    else if (current_screen == ui_weather) 
    {
        lv_label_set_text(ui_city, local_city);
        snprintf(buf, sizeof(buf), "%d", local_cur_temp);
        lv_label_set_text(ui_weathertemp, buf);
        snprintf(buf, sizeof(buf), "%d/%d", local_min_temp, local_max_temp);
        lv_label_set_text(ui_weathermin, buf);
        
        // 1. 判断 AQI 等级
        const char *aqi_level = "未知";
        if (local_aqi == 0) 
        {
            aqi_level = "获取中"; // 刚开机没连上蓝牙时显示
        } 
        else if (local_aqi <= 50) 
        {
            aqi_level = "优";
        } 
        else if (local_aqi <= 100) 
        {
            aqi_level = "良";
        } 
        else if (local_aqi <= 150) 
        {
            aqi_level = "轻度污染";
        } 
        else if (local_aqi <= 200) 
        {
            aqi_level = "中度污染";
        } 
        else if (local_aqi <= 300) 
        {
            aqi_level = "重度污染";
        } 
        else 
        {
            aqi_level = "严重污染";
        }

        // 2. 将数值和等级文字拼接到一起 (例如: "45 优" 或 "120 轻度污染")
        snprintf(buf, sizeof(buf), "%d %s", local_aqi, aqi_level);
        lv_label_set_text(ui_aqi, buf);

        // 根据天气类型设置天气图标
        switch (local_weather_type) 
        {
            case 0x00: // 晴天
                lv_img_set_src(ui_weather_image, &ui_img_wsun_png);
                snprintf(buf, sizeof(buf), "晴天");
                lv_label_set_text(ui_weathernow, buf);
                break;
            case 0x01: // 多云
                lv_img_set_src(ui_weather_image, &ui_img_wcloud_png);
                snprintf(buf, sizeof(buf), "多云");
                lv_label_set_text(ui_weathernow, buf);
                break;
            case 0x02: // 阴天
                lv_img_set_src(ui_weather_image, &ui_img_wcloud_png);
                snprintf(buf, sizeof(buf), "阴天");
                lv_label_set_text(ui_weathernow, buf);
                break;
            case 0x03: // 雨天
                lv_img_set_src(ui_weather_image, &ui_img_wrain_png);
                snprintf(buf, sizeof(buf), "雨天");
                lv_label_set_text(ui_weathernow, buf);
                break;
            // case 0x04: // 雷雨
            //     lv_img_set_src(ui_weather_image, &ui_img_wthunder_png);
            //     break;
            // case 0x05: // 雪天
            //     lv_img_set_src(ui_weather_image, &ui_img_wsnow_png);
            //     break;
        }
    }
    else if (current_screen == ui_weather2)
    {
        const char *week_str[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};

        lv_label_set_text(ui_city2, local_city);
        snprintf(buf, sizeof(buf), "%02d/%02d/%02d", year, month, day);
        lv_label_set_text(ui_year2, buf);
        snprintf(buf, sizeof(buf), "%s", week_str[weekday]);
        lv_label_set_text(ui_week2, buf);
        //未来12小时逐小时天气预报显示
        lv_obj_t * time_labels[12] = {ui_weathertime1, ui_weathertime2, ui_weathertime3, ui_weathertime4, ui_weathertime5, ui_weathertime6, ui_weathertime7, ui_weathertime8, ui_weathertime9, ui_weathertime10, ui_weathertime11, ui_weathertime12};
        lv_obj_t * temp_labels[12] = {ui_weather_temp1, ui_weather_temp2, ui_weather_temp3, ui_weather_temp4, ui_weather_temp5, ui_weather_temp6, ui_weather_temp7, ui_weather_temp8, ui_weather_temp9, ui_weather_temp10, ui_weather_temp11, ui_weather_temp12};
        lv_obj_t * icon_imgs[12]   = {ui_weathertype1, ui_weathertype2, ui_weathertype3, ui_weathertype4, ui_weathertype5, ui_weathertype6, ui_weathertype7, ui_weathertype8, ui_weathertype9, ui_weathertype10, ui_weathertype11, ui_weathertype12};

        for (int i = 0; i < 12; i++) 
        {
            hourly_weather_t *hourly = &local_hourly[i];
            snprintf(buf, sizeof(buf), "%02d:00", hourly->hour);
            lv_label_set_text(time_labels[i], buf);
            snprintf(buf, sizeof(buf), "%d°", hourly->temp);
            lv_label_set_text(temp_labels[i], buf);

            switch (hourly->weather_type) 
            {
                case 0x00: // 晴天
                    lv_img_set_src(icon_imgs[i], &ui_img_weather_sun_png);
                    break;
                case 0x01: // 多云
                    lv_img_set_src(icon_imgs[i], &ui_img_weather_cloud_png);
                    break;
                case 0x02: // 阴天
                    lv_img_set_src(icon_imgs[i], &ui_img_weather_cloud_png);
                    break;
                case 0x03: // 雨天
                    lv_img_set_src(icon_imgs[i], &ui_img_weather_cloud_png);
                    break;
                // case 0x04: // 雷雨
                //     lv_img_set_src(icon_imgs[i], &ui_img_wthunder_png);
                //     break;
                // case 0x05: // 雪天
                //     lv_img_set_src(icon_imgs[i], &ui_img_wsnow_png);
                //     break;
            }
        }

        //未来7天天气预报显示
        lv_obj_t * day_labels[7] = {ui_shijian1, ui_shijian2, ui_shijian3, ui_shijian4, ui_shijian5, ui_shijian6, ui_shijian7};
        lv_obj_t * temp1_labels[7] = {ui_weathermax1, ui_weathermax2, ui_weathermax3, ui_weathermax4, ui_weathermax5, ui_weathermax6, ui_weathermax7};
        lv_obj_t * forecast_icons[7] = {ui_weathertype13, ui_weathertype14, ui_weathertype15, ui_weathertype16, ui_weathertype17, ui_weathertype18, ui_weathertype19};

        for (int i = 0; i < 7; i++) 
        {
            if (i == 0) 
            {
                // 第 0 天强制显示为“今天”
                lv_label_set_text(day_labels[i], "今天");
            } 
            else 
            {
                // 后续日期：用今天的 weekday 加上偏移量 i，再对 7 取余数
                // 比如今天是周六(6)，明天(i=1)就是 (6+1)%7 = 0(周日)，完美循环！
                int target_weekday = (weekday + i) % 7;
                lv_label_set_text(day_labels[i], week_str[target_weekday]);
            }
            // 刷新温度区间文字 (例如: "22~28°C")
            snprintf(buf, sizeof(buf), "%d°/%d°", local_forecast[i].min_temp, local_forecast[i].max_temp);
            lv_label_set_text(temp1_labels[i], buf);

            // 刷新对应图标
            switch (local_forecast[i].weather_type) 
            {
                case 0x00: lv_img_set_src(icon_imgs[i], &ui_img_weather_sun_png); break;
                case 0x01: lv_img_set_src(icon_imgs[i], &ui_img_weather_cloud_png); break;
                case 0x02: lv_img_set_src(icon_imgs[i], &ui_img_weather_cloud_png); break;
                case 0x03: lv_img_set_src(icon_imgs[i], &ui_img_weather_cloud_png); break;
                default:   lv_img_set_src(icon_imgs[i], &ui_img_weather_sun_png); break;
            }
        }
    }

}

void UI_Updater_Start(void) 
{
    lv_timer_create(ui_update_timer_cb, 500, NULL);
}
