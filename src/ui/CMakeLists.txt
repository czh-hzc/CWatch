SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    screens/ui_Screen5.c
    screens/ui_Screen6.c
    screens/ui_Screen7.c
    screens/ui_weather.c
    screens/ui_weather2.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_battery_png.c
    images/ui_img_1883903296.c
    images/ui_img_o2_png.c
    images/ui_img_foot_png.c
    images/ui_img_hot_png.c
    images/ui_img_wave2_png.c
    images/ui_img_wave1_png.c
    images/ui_img_heart_png.c
    images/ui_img_pngsucai_3128177_ef13f6_png.c
    images/ui_img_arrow_png.c
    images/ui_img_gemini_generated_image_fdcqbnfdcqbnfdcq_png.c
    images/ui_img_altitude_png.c
    images/ui_img_hum_png.c
    images/ui_img_press_png.c
    images/ui_img_temp_png.c
    images/ui_img_wsun_png.c
    images/ui_img_weather_sun_cloud_png.c
    fonts/ui_font_chinese.c
    fonts/ui_font_chinese2.c
    fonts/ui_font_large.c
    fonts/ui_font_large_promax.c)

add_library(ui ${SOURCES})
