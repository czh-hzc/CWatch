#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include <board.h>
#include "QMC5883P.h"
#include "BME280.h"
#include "lsm6ds3tr_c.h"
#include "MAX30102.h"
#include "lvgl.h"
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"
#include "ui.h"
#include "data_hub.h"
#include "sensor_task.h"
#include "ui_update.h"
#include "ui_start.h"
#include "app_ble.h"

int main(void)
{
    
    DataHub_Init();
    SensorTask_Start();
    lvgl_start();
    ble_app_init();
    
    while (1)
    {
        rt_thread_mdelay(1000);
    }

    return 0;
}
// LV_IMG_DECLARE(ui_img_weather_cloud_png);    
// LV_IMG_DECLARE(ui_img_weather_sun_png);    
// LV_IMG_DECLARE(ui_img_wrain_png);
// LV_IMG_DECLARE(ui_img_wcloud_png);
// LV_IMG_DECLARE(ui_img_compass_png);
// LV_IMG_DECLARE(ui_img_light_png);
// LV_IMG_DECLARE(ui_img_heart1_png);
// LV_IMG_DECLARE(ui_img_setting_png);
// LV_IMG_DECLARE(ui_img_env_png);