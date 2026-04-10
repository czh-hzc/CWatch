#include "rtthread.h"
#include "lvgl.h"
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"
#include "ui.h"
#include "ui_update.h"
#include "ui_start.h"
#include "honeycomb.h"

static rt_thread_t lvgl_thread = RT_NULL;

static void lvgl_thread_entry(void *parameter)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t ms;

    /* init littlevGL */
    ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK)
    {
        return;
    }
    lv_ex_data_pool_init();

    ui_init();
    ui_honeycomb_init();
    UI_Updater_Start();

    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }
}

void lvgl_start(void)
{
    lvgl_thread = rt_thread_create("lvgl_thread", lvgl_thread_entry, RT_NULL, 8192, 15, 10);
    if (lvgl_thread != RT_NULL) 
    {
        rt_thread_startup(lvgl_thread);
    }
}

