#include "rtthread.h"
#include "ui.h"
#include "stdlib.h"
#include <math.h> 
#include "honeycomb.h"

#define IMAGE_ICON_NUM  25  // 你的 15 个图标
#define IMAGE_ROW_NUM   5   // 每行排 3 个
#define IMAGE_SIZE      80  // 基础大小设定
#define SPACING_X   120 // 间距
#define SPACING_Y   104

//#define SCREEN_CENTER_X 230
//#define SCREEN_CENTER_Y 230

// 声明全局数组保存对象和初始坐标
static lv_obj_t * my_images[IMAGE_ICON_NUM];
static lv_point_t init_pos[IMAGE_ICON_NUM];
static lv_point_t point_final = {0, 0}; // 记录手指累计拖拽了多远

// 定义全局变量，用于记录动画的起点和终点
static lv_point_t anim_start_point;
static lv_point_t anim_target_point;

static bool is_dragged = false; // 核心：记录本次按下是否发生了滑动

static int SCREEN_CENTER_X = 0;
static int SCREEN_CENTER_Y = 0;

static void update_icon_positions(void)
{
    for(int i = 0; i < IMAGE_ICON_NUM; i++)
    {
        // 1. 根据初始木桩 + 总偏移量，算出当前绝对左上角坐标
        int pos_x = init_pos[i].x + point_final.x;
        int pos_y = init_pos[i].y + point_final.y;

        // 2. 算出图标的绝对正中心
        int center_x = pos_x + IMAGE_SIZE / 2;
        int center_y = pos_y + IMAGE_SIZE / 2;

        // 3. 计算距离屏幕中心的距离
        int32_t dx = center_x - SCREEN_CENTER_X;
        int32_t dy = center_y - SCREEN_CENTER_Y;
        uint32_t dist = (uint32_t)sqrt((float)(dx * dx + dy * dy));

        // 4. 计算缩放
        int32_t zoom = 512 - (dist * 1.25); 
        if(zoom < 64) zoom = 64;   
        if(zoom > 512) zoom = 512; 

        // 5. 应用位置和缩放
        lv_img_set_zoom(my_images[i], zoom);
        lv_obj_set_pos(my_images[i], pos_x, pos_y);
    }
}


// ================= 新增：整数四舍五入算法 (网格吸附核心) =================
// 作用：如果你滑动了 60 像素(超过一半)，就吸附到 100；滑动了 40 像素(没到一半)，就退回 0。
static int snap_to_grid(int value, int spacing) 
{
    int remainder = value % spacing;
    if (remainder > spacing / 2) 
    {
        return value - remainder + spacing;
    } 
    else if (remainder < -spacing / 2) 
    {
        return value - remainder - spacing;
    }
    return value - remainder;
}

// ================= 新增：LVGL 动画执行回调 =================
// 这个函数会在动画运行的 300 毫秒内，被高频调用，平滑改变 point_final
static void scroll_anim_cb(void * var, int32_t v) {
    // v 是从 0 到 1024 的进度值
    point_final.x = anim_start_point.x + ((anim_target_point.x - anim_start_point.x) * v) / 1024;
    point_final.y = anim_start_point.y + ((anim_target_point.y - anim_start_point.y) * v) / 1024;
    
    // 每次更新坐标后，刷新图标位置
    update_icon_positions();
}

static void screen_load_cb(lv_event_t * e)
{
    // 每次屏幕切换时重置拖动状态，防止坐标乱飞
    point_final.x = 0;
    point_final.y = 0;
    update_icon_positions();
}

static void honeycomb_scroll_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    // 获取我们在 init 传进来的图标索引 i
    int icon_index = (int)(intptr_t)lv_event_get_user_data(e);
    lv_indev_t * indev = lv_indev_get_act();

    if(code == LV_EVENT_PRESSED)
    {
        is_dragged = false; // 每次按下都重置拖动状态
        //drag_sum_x = 0;     // 清零累加器
        //drag_sum_y = 0;
    }

    // 【阶段 1】：手指正在拖拽
    if (code == LV_EVENT_PRESSING) 
    {
        // 如果手指刚按下去发现还有回弹动画在跑，立刻打断它！(非常重要的手感优化)
        lv_anim_del(&point_final, NULL);

        //lv_indev_t * indev = lv_indev_get_act(); 
        if(indev == NULL) return;

        lv_point_t vect;
        lv_indev_get_vect(indev, &vect);

        //drag_sum_x += vect.x;
        //drag_sum_y += vect.y;

        if (abs(vect.x) > 5 || abs(vect.y) > 5) 
        {
            is_dragged = true;
        }

        point_final.x += vect.x;
        point_final.y += vect.y;

            update_icon_positions();
        }

    // 【阶段 2】：手指松开屏幕 (或者滑动出屏幕意外丢失焦点)
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) 
    {
        if(!is_dragged && code == LV_EVENT_RELEASED) 
        {
            rt_kprintf("click %d \n", icon_index);
            switch(icon_index)
            {
                case 0: // 环境监测
                    _ui_screen_change(&ui_Screen5, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen5_screen_init);
                    break;
                case 1: // 设置
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 2: // 手电筒
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 3: // 罗盘
                    _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen3_screen_init);
                    break;
                case 4: // 心率
                    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen2_screen_init);
                    break;
                case 5: // 环境监测
                    _ui_screen_change(&ui_Screen5, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen5_screen_init);
                    break;
                case 6: // 设置
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 7: // 手电筒
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 8: // 罗盘
                    _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen3_screen_init);
                    break;
                case 9: // 心率
                    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen2_screen_init);
                    break;
                case 10: // 环境监测
                    _ui_screen_change(&ui_Screen5, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen5_screen_init);
                    break;
                case 11: // 设置
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 12: // 手电筒
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 13: // 罗盘
                    _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen3_screen_init);
                    break;
                case 14: // 心率
                    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen2_screen_init);
                    break;
                case 15: // 环境监测
                    _ui_screen_change(&ui_Screen5, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen5_screen_init);
                    break;
                case 16: // 设置
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 17: // 手电筒
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 18: // 罗盘
                    _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen3_screen_init);
                    break;
                case 19: // 心率
                    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen2_screen_init);
                    break;
                case 20: // 环境监测
                    _ui_screen_change(&ui_Screen5, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen5_screen_init);
                    break;
                case 21: // 设置
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 22: // 手电筒
                    _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen7_screen_init);
                    break;
                case 23: // 罗盘
                    _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen3_screen_init);
                    break;
                case 24: // 心率
                    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen2_screen_init);
                    break;
            }
            return; // 没有拖动就直接返回，不启动回弹动画
        }

        // 1. 计算吸附到最近的网格坐标
        // int target_x = snap_to_grid(point_final.x, SPACING_X);
        // int target_y = snap_to_grid(point_final.y, SPACING_Y);

        // 1. 先确定 Y 轴吸附到哪一行
        int target_y = snap_to_grid(point_final.y, SPACING_Y);

        // 2. 算出我们上下偏离了中心几行
        int row_offset = target_y / SPACING_Y;

        // 3. 决定 X 轴是否需要错开半个身位
        // 因为初始中心(row=2)是偶数行。如果拖拽偏离了奇数行（例如移到了原本的第1行），
        // 那么这行的图标天生就偏了 SPACING_X / 2，所以我们的 X 吸附点也必须补偿这个偏移。
        int offset_x = 0;
        if (abs(row_offset) % 2 == 1) 
        {
            offset_x = SPACING_X / 2;
        }

        // 4. 将 X 轴减去错位进行吸附，然后再把错位加回来
        int target_x = snap_to_grid(point_final.x - offset_x, SPACING_X) + offset_x;

        // 2. 边缘限制检查 (边界钳制)
        // 我们是 5行3列，中心在 (row=2, col=1)。
        // X轴：最多向左移1列 (-1 * SPACING_X)，向右移1列 (+1 * SPACING_X)
        // Y轴：最多向上移2行 (-2 * SPACING_Y)，向下移2行 (+2 * SPACING_Y)
        int MAX_X = 2 * SPACING_X;
        int MIN_X = -2 * SPACING_X;
        int MAX_Y = 2 * SPACING_Y;
        int MIN_Y = -2 * SPACING_Y;

        if (target_x > MAX_X) target_x = MAX_X;
        if (target_x < MIN_X) target_x = MIN_X;
        if (target_y > MAX_Y) target_y = MAX_Y;
        if (target_y < MIN_Y) target_y = MIN_Y;

        // 如果当前位置已经完全等于目标位置了，就不需要开启动画浪费 CPU
        if (point_final.x == target_x && point_final.y == target_y) return;

        // 3. 启动回弹阻尼动画
        anim_start_point = point_final;     // 起点：当前手指松开的地方
        anim_target_point.x = target_x;     // 终点：算好的网格中心点
        anim_target_point.y = target_y;

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, &point_final);   // 用 point_final 的地址作为动画的唯一标识
        lv_anim_set_values(&a, 0, 1024);     // 百分比进度：0 到 100% (用1024精度高)
        lv_anim_set_time(&a, 300);           // 回弹时间：300毫秒
        lv_anim_set_exec_cb(&a, scroll_anim_cb);
        
        // 使用 OVERSHOOT 曲线，能呈现出超棒的“弹簧拉扯再回弹”的物理阻尼感！
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot); 
        
        lv_anim_start(&a);
    }


}

void ui_honeycomb_init(void) 
{
    const void* icon_srcs[] = 
    {
        &ui_img_env_png, &ui_img_setting_png, &ui_img_light_png, 
        &ui_img_compass_png, &ui_img_heart1_png
    };

    // 重置拖拽记录（非常重要，防止二次进入时坐标乱飞）
    point_final.x = 0;
    point_final.y = 0;

    // 获取我们要画图的屏幕对象
    lv_obj_t * parent = ui_Screen6; 

    
    // 【暴力模式】：直接关闭容器自带的所有滚动功能！

    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(parent, LV_DIR_NONE); 
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    //动态获取这个容器(屏幕)的真实物理分辨率，算出真正的中心
    SCREEN_CENTER_X = lv_disp_get_hor_res(NULL) / 2;
    SCREEN_CENTER_Y = lv_disp_get_ver_res(NULL) / 2;

    int center_row = 2;
    int center_col = 2;

    int target_x = SCREEN_CENTER_X - (IMAGE_SIZE / 2);
    int target_y = SCREEN_CENTER_Y - (IMAGE_SIZE / 2);

    int start_x = target_x - (center_col * SPACING_X);
    int start_y = target_y - (center_row * SPACING_Y);

    for(int i = 0; i < IMAGE_ICON_NUM; i++)
    {
        my_images[i] = lv_img_create(parent);
        lv_img_set_src(my_images[i], icon_srcs[i % 5]);
        
        // 缩放中心设置
        lv_obj_set_style_transform_pivot_x(my_images[i], IMAGE_SIZE / 2, 0);
        lv_obj_set_style_transform_pivot_y(my_images[i], IMAGE_SIZE / 2, 0);

        // 计算蜂窝错位排列的初始坐标
        int row = i / IMAGE_ROW_NUM;
        int col = i % IMAGE_ROW_NUM;
        
        int hex_offset = (row % 2 != 0) ? (SPACING_X / 2) : 0;

        int x = start_x + col * SPACING_X + hex_offset;
        int y = start_y + row * SPACING_Y;

        // 记录初始坐标，这非常关键！
        init_pos[i].x = x;
        init_pos[i].y = y;

        // 赋予图片点击响应能力
        lv_obj_add_flag(my_images[i], LV_OBJ_FLAG_CLICKABLE);
        
        // 【核心大招】：不挂 SCROLL，把拖拽事件挂载为 LV_EVENT_PRESSING (持续按压拖动)
        lv_obj_add_event_cb(my_images[i], honeycomb_scroll_cb, LV_EVENT_ALL, (void *)(intptr_t)i);
    }

    // 【新增 1】：获取装载这个菜单的真正屏幕 (Screen)
    //lv_obj_t * current_screen = lv_obj_get_screen(parent);
    
    // 【新增 2】：给这个屏幕挂载“即将显示”的事件
    lv_obj_add_event_cb(parent, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);

    // 【新增 3】：开机第一次手动刷一遍位置
    point_final.x = 0;
    point_final.y = 0;
    update_icon_positions();
}
