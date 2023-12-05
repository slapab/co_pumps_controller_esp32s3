#pragma once

#include <stdbool.h>
#include <esp_err.h>

esp_err_t display_drv_init(void);

// Move this to the display class
bool display_drv_lock(const uint32_t timeout_ms);
void display_drv_unlock(void);
