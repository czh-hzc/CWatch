#include "rtthread.h"
#include "data_hub.h"
#include "sensor_task.h"
#include "QMC5883P.h"
#include "BME280.h"
#include "lsm6ds3tr_c.h"
#include "MAX30102.h"
#include "RTC_T.h"
#include "step_counter.h"
#include "heart_sensor_power.h"

static rt_thread_t sensor_thread = RT_NULL;

static void sensor_thread_entry(void *parameter)
{
    lsm6ds3tr_init();
    StepCounter_Init();
    BME280_init();
    MAX30102_init();
    QMC5883P_init();
    RTC_Init();

    while (1)
    {
        rt_uint8_t run_heart = 0, run_env = 0, run_mag = 0, run_imu = 0, run_step = 0, run_rtc = 0;
        rt_uint8_t heart_power_mode = HEART_SENSOR_POWER_OFF;
        if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
        {
            run_heart = system_data.enable_heartrate;
            heart_power_mode = system_data.heartrate_power_mode;
            run_env = system_data.enable_environment;
            run_mag = system_data.enable_mag;
            run_imu = system_data.enable_imu;
            run_step = system_data.enable_step;
            run_rtc = system_data.enable_rtc;
            rt_mutex_release(data_mutex);
        }

        if(HeartSensorPower_ApplyMode((heart_sensor_power_mode_t)heart_power_mode) != RT_EOK)
        {
            run_heart = 0;
        }

        if(run_heart)
        {
            rt_int32_t heart_rate, spo2;
            if(MAX30102_getdata(&heart_rate, &spo2) == 1)
            {
                if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
                {
                    system_data.heart_rate = heart_rate;
                    system_data.spo2 = spo2;
                    rt_mutex_release(data_mutex);
                }
            } 
            // else 
            // {
            //     MAX30102_Clear_Buffer();
            // }
        }

        if(run_env)
        {
            float temp, press, hum, altitude;
            if(BME280_getdata(&temp, &press, &hum, &altitude) == 1)
            {
                if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
                {
                    system_data.temperature = temp;
                    system_data.pressure = press;
                    system_data.humidity = hum;
                    system_data.altitude = altitude;
                    rt_mutex_release(data_mutex);
                }
            }
        }

        if(run_imu)
        {
            float gx, gy, gz, ax, ay, az;
            if(lsm6ds3tr_getdata(&gx, &gy, &gz, &ax, &ay, &az) == 1)
            {
                if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
                {
                    system_data.gyro_x = gx;
                    system_data.gyro_y = gy;
                    system_data.gyro_z = gz;
                    system_data.acc_x = ax;
                    system_data.acc_y = ay;
                    system_data.acc_z = az;
                    rt_mutex_release(data_mutex);
                }
            }
        }

        if(run_step)
        {
            rt_uint32_t step_count;
            if(StepCounter_Read(&step_count) == RT_EOK)
            {
                if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
                {
                    system_data.step_count = step_count;
                    rt_mutex_release(data_mutex);
                }
            }
        }

        if(run_mag)
        {
            rt_int16_t mx, my, mz;
            if(QMC5883P_getdata(&mx, &my, &mz) == 1)
            {
                if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
                {
                    system_data.mag_x = mx;
                    system_data.mag_y = my;
                    system_data.mag_z = mz;
                    rt_mutex_release(data_mutex);
                }
            }
        }

        if(run_rtc)
        {
            rt_uint16_t year;
            rt_uint8_t month, day, hour, minute, second, weekday;
            RTC_GetTime(&year, &month, &day, &hour, &minute, &second, &weekday);
            if(rt_mutex_take(data_mutex, RT_WAITING_FOREVER) == RT_EOK)
            {
                system_data.year = year;
                system_data.month = month;
                system_data.day = day;
                system_data.hour = hour;
                system_data.minute = minute;
                system_data.second = second;
                system_data.weekday = weekday;
                rt_mutex_release(data_mutex);
            }
        }

        if (run_mag || run_imu) 
        {
            rt_thread_mdelay(20); 
        }
        else 
        {
            // 其他不需要高实时性的传感器，1秒测一次省电
            rt_thread_mdelay(1000); 
        }
    }
}

void SensorTask_Start(void) 
{
    sensor_thread = rt_thread_create("sensor_task", sensor_thread_entry, RT_NULL, 4096, 15, 10);
    if (sensor_thread != RT_NULL) 
    {
        rt_thread_startup(sensor_thread);
    }
}
