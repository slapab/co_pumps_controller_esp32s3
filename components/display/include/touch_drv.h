#pragma once

#include "esp_err.h"
#include "esp_lcd_touch.h"

esp_err_t touch_drv_init(esp_lcd_touch_handle_t* touch_handle);

bool touch_drv_read(int16_t* x_coord, int16_t* y_coord);
