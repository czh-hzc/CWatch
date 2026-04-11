#include <rtthread.h>
#include <string.h>
#include "ble_rx.h"
#include "RTC_T.h"
#include "data_hub.h"

void ble_rx_handler(rt_uint8_t *data, rt_uint16_t len)
{
    if (data == RT_NULL || len == 0)
    {
        return;
    }

    if (len == 7)
    {
        rt_uint16_t year = (data[0] << 8) | data[1];
        rt_uint8_t month = data[2];
        rt_uint8_t day = data[3];
        rt_uint8_t hour = data[4];
        rt_uint8_t min = data[5];
        rt_uint8_t sec = data[6];

        rt_kprintf("[Protocol] Sync Time: %04d-%02d-%02d %02d:%02d:%02d\r\n", year, month, day, hour, min, sec);
        RTC_SetTime(year, month, day, hour, min, sec);
        rt_kprintf("[Protocol] RTC Updated successfully!\r\n");
    }
    else if (len == 3 && data[0] == 0x01)
    {
        rt_uint8_t temp = data[1];
        rt_uint8_t weather_type = data[2];
        rt_kprintf("[Protocol] Weather received -> Temp: %d, Type: %d\r\n", temp, weather_type);
    }
    else if (data[0] == 0x02 && len >= 7)
    {
        rt_uint8_t weather_type = data[1];
        rt_int8_t current_temp = (rt_int8_t)data[2];
        rt_int8_t max_temp = (rt_int8_t)data[3];
        rt_int8_t min_temp = (rt_int8_t)data[4];
        rt_uint16_t aqi = (data[5] << 8) | data[6];

        char city_name[16] = {0};
        rt_uint8_t city_len = len - 7;

        if (city_len > 0 && city_len < sizeof(city_name))
        {
            rt_memcpy(city_name, &data[7], city_len);
            city_name[city_len] = '\0';
        }
        else
        {
            strcpy(city_name, "unknown");
        }

        if (rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
        {
            strcpy(system_data.city_name, city_name);
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
    }
    else if (data[0] == 0x03 && len >= 11)
    {
        rt_uint8_t page = data[1];
        rt_uint8_t print_7day = 0;
        daily_weather_t local_forecast[7];

        if (rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (page == 0x00)
            {
                for (int i = 0; i < 4; i++)
                {
                    system_data.forecast[i].weather_type = data[2 + i * 3];
                    system_data.forecast[i].max_temp = (rt_int8_t)data[3 + i * 3];
                    system_data.forecast[i].min_temp = (rt_int8_t)data[4 + i * 3];
                }
            }
            else if (page == 0x01)
            {
                for (int i = 0; i < 3; i++)
                {
                    system_data.forecast[i + 4].weather_type = data[2 + i * 3];
                    system_data.forecast[i + 4].max_temp = (rt_int8_t)data[3 + i * 3];
                    system_data.forecast[i + 4].min_temp = (rt_int8_t)data[4 + i * 3];
                }

                system_data.enable_forecastweather = 1;
                memcpy(local_forecast, system_data.forecast, sizeof(local_forecast));
                print_7day = 1;
            }

            rt_mutex_release(data_mutex);
        }

        if (print_7day)
        {
            rt_kprintf("[BLE] 7-Day Forecast Assembly Complete!\r\n");
            rt_kprintf("\r\n====== 7 day weather ======\r\n");
            for (int j = 0; j < 7; j++)
            {
                rt_kprintf("day %d  | type: %d | %d C ~ %d C\r\n",
                           j + 1,
                           local_forecast[j].weather_type,
                           local_forecast[j].min_temp,
                           local_forecast[j].max_temp);
            }
            rt_kprintf("===============================\r\n\r\n");
        }
    }
    else if (data[0] == 0x04 && len >= 20)
    {
        rt_uint8_t page = data[1];
        rt_uint8_t print_24h = 0;
        hourly_weather_t local_hourly[24];

        if (rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (page <= 3)
            {
                for (int i = 0; i < 6; i++)
                {
                    int global_idx = page * 6 + i;
                    system_data.hourly_forecast[global_idx].hour = data[2 + i * 3];
                    system_data.hourly_forecast[global_idx].weather_type = data[3 + i * 3];
                    system_data.hourly_forecast[global_idx].temp = (rt_int8_t)data[4 + i * 3];
                }

                rt_kprintf("[BLE] Received 24h forecast page: %d\r\n", page + 1);

                if (page == 3)
                {
                    system_data.enable_hourlyweather = 1;
                    memcpy(local_hourly, system_data.hourly_forecast, sizeof(local_hourly));
                    print_24h = 1;
                }
            }

            rt_mutex_release(data_mutex);
        }

        if (print_24h)
        {
            rt_kprintf("\r\n====== 24 hour data ======\r\n");
            for (int j = 0; j < 24; j++)
            {
                rt_kprintf("%02d:00 | type: %d | temp: %d C\r\n",
                           local_hourly[j].hour,
                           local_hourly[j].weather_type,
                           local_hourly[j].temp);
            }
            rt_kprintf("==================================\r\n\r\n");
        }
    }
    else
    {
        rt_kprintf("[Protocol] Unknown data received, len: %d\r\nData: ", len);
        for (int i = 0; i < len; i++)
        {
            rt_kprintf("%02X ", data[i]);
        }
        rt_kprintf("\r\n");
    }
}
