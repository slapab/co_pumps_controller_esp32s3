
idf_component_register(
                       SRCS "display_ui.cpp" "display_drv.c" "touch_drv.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver esp_lcd log lvgl esp_timer
                       PRIV_REQUIRES heap freertos bsp esp_lcd_touch_cst816s ui pumps controller config
                       )
