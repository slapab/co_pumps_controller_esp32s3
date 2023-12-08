#include <stdint.h>
#include "display_ui.hpp"
#include <freertos/FreeRTOS.h>
#include "freertos/semphr.h"
#include <freertos/task.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "lvgl.h"
#pragma GCC diagnostic pop

#include "ui.h"

#include <esp_log.h>
#define TAG "disp_ui"

/* A little above 60Hz */
#define EXAMPLE_LVGL_TASK_INTERVAL_MS UINT32_C(16)


static DisplayUI disp_ui;

static void call_disp_ui_tick();

void DisplayUI::tick()
{
    // TODO
}




static void call_disp_ui_tick() {
    disp_ui.tick();
}

extern "C" void display_ui_task(void *arg)
{
    lv_disp_t *disp = (lv_disp_t*)arg;
    (void)disp;

    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_INTERVAL_MS;
    uint32_t t_slicking_tp = UINT32_C(0);

    ui_init();

    while (true) {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        if ((xTaskGetTickCount() - t_slicking_tp) >= pdMS_TO_TICKS(task_delay_ms)) {
            t_slicking_tp = xTaskGetTickCount();
            task_delay_ms = lv_timer_handler();
        }

        call_disp_ui_tick();

        if (task_delay_ms > EXAMPLE_LVGL_TASK_INTERVAL_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_INTERVAL_MS;
            vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
        } else if (0u == task_delay_ms) {
            continue;
        } else {
            vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
        }
    }
}
