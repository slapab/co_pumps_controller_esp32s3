#include <stdint.h>
#include "display_ui.hpp"
#include <freertos/FreeRTOS.h>
#include "freertos/projdefs.h"
#include "freertos/semphr.h"
#include <freertos/task.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "lvgl.h"
#include "ui_comp.h"
#include "ui.h"
#pragma GCC diagnostic pop

#include <esp_log.h>
#define TAG "disp_ui"

/* A little above 60Hz */
#define EXAMPLE_LVGL_TASK_INTERVAL_MS UINT32_C(16)


static DisplayUI disp_ui;

static void call_disp_ui_tick();

void DisplayUI::init()
{
    setup_ui();
}

void DisplayUI::tick()
{
    // TODO
}

void DisplayUI::setup_ui()
{
    /* Set label for floor 0 widget */
    lv_obj_t *obj = ui_comp_get_child(ui_homePumpStatusComponentFloor0, UI_COMP_HOMEPUMPSTATUSCOMPONENT_NAMELABEL);
    lv_label_set_text(obj, "Parter");

    /* Set icons for tabs instead of texts */
    static const char* tabs_icons_map[] = {LV_SYMBOL_HOME, LV_SYMBOL_SETTINGS, LV_SYMBOL_WIFI, ""};
    lv_btnmatrix_set_map(lv_tabview_get_tab_btns(ui_TabView1), tabs_icons_map);
}


static void call_disp_ui_tick()
{
    static uint32_t stp = 0u;
    const uint32_t curr_tp = xTaskGetTickCount();
    const uint32_t TICK_INTERVAL_MS = 20u;

    if ((curr_tp - stp) >= pdMS_TO_TICKS(TICK_INTERVAL_MS))
    {
        stp = curr_tp;
        disp_ui.tick();
    }
}

extern "C" void display_ui_task(void *arg)
{
    lv_disp_t *disp = (lv_disp_t*)arg;
    (void)disp;

    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_INTERVAL_MS;
    uint32_t t_slicking_tp = UINT32_C(0);

    ui_init();
    disp_ui.init();

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
