#include <stdint.h>
#include "display_ui.hpp"
#include <freertos/FreeRTOS.h>
#include "display_ui_messages.h"
#include "esp_err.h"
#include "freertos/projdefs.h"
#include "freertos/semphr.h"
#include <freertos/task.h>
#include "pumps.hpp"
#include "controller.hpp"
#include "temperatures.hpp"
#include "screen_home_events_actions.hpp"

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


static void call_disp_ui_tick();

DisplayUI& DisplayUI::instance()
{
    static DisplayUI disp_ui;
    return disp_ui;
}

void DisplayUI::init()
{
    /* Create Queue for messages */
    if (nullptr == msg_q_handle)
    {
        msg_q_handle = xQueueCreate(MSQ_Q_LEN, MSG_Q_ITEM_SIZE);
        if (nullptr == msg_q_handle)
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(ESP_ERR_NO_MEM);
        }
    }

    setup_ui();
}

bool DisplayUI::send_msg(const disp_ui_msg_t& msg, const uint32_t timeout_ticks)
{
    return pdTRUE == xQueueSend(msg_q_handle, &msg, timeout_ticks);
}

void DisplayUI::tick()
{
    // TODO
    // static uint32_t stp = 0u;
    // static bool state = false;
    // const uint32_t curr_tp = xTaskGetTickCount();
    // const uint32_t TICK_INTERVAL_MS = 1000u;

    // if ((curr_tp - stp) >= pdMS_TO_TICKS(TICK_INTERVAL_MS))
    // {
    //     stp = curr_tp;
    //     if (state) {
    //         pumps::pump_floor1_fsm::dispatch(pumps::off_evt());
    //         pumps::pump_ground_floor_fsm::dispatch(pumps::off_evt());
    //     } else {
    //         pumps::pump_floor1_fsm::dispatch(pumps::on_evt());
    //         pumps::pump_ground_floor_fsm::dispatch(pumps::on_evt());
    //     }
    //     state ^= true;
    // }
    process_msg();
}

void DisplayUI::setup_ui()
{
    /* Set label for floor 0 widget */
    lv_obj_t *obj = ui_comp_get_child(ui_homePumpStatusComponentFloor0, UI_COMP_HOMEPUMPSTATUSCOMPONENT_NAMELABEL);
    lv_label_set_text(obj, "Parter");
    obj = ui_comp_get_child(ui_homePumpStatusComponentFloor1, UI_COMP_HOMEPUMPSTATUSCOMPONENT_NAMELABEL);
    lv_label_set_text(obj, "Pietro 1");

    /* Set icons for tabs instead of texts */
    static const char* tabs_icons_map[] = {LV_SYMBOL_HOME, LV_SYMBOL_SETTINGS, LV_SYMBOL_WIFI, ""};
    lv_btnmatrix_set_map(lv_tabview_get_tab_btns(ui_TabView1), tabs_icons_map);
}

bool DisplayUI::get_msg(disp_ui_msg_t& msg, const uint32_t timeout_ticks)
{
    return pdTRUE == xQueueReceive(msg_q_handle, &msg, timeout_ticks);
}

void DisplayUI::process_msg()
{
    disp_ui_msg_t msg = {};
    if (true == get_msg(msg))
    {
        switch (msg.msg_id)
        {
            #define X(enum_name, data_name)\
                case display_ui_msg_id_t::enum_name:\
                    handle_msg(msg.data_name);
                    break;

            DISPLAY_UI_MSGS_LIST_X
            #undef X

            case display_ui_msg_id_t::DISP_UI_MSG_NUM:
                break;
        }
    }
}

void DisplayUI::handle_msg(const disp_ui_msg_temp_upd_t& msg)
{
    bool update_widget = true;
    /* Call appropriate controller state machines with temp update */
    switch (static_cast<temp_sensor_t>(msg.temp_sensor_id))
    {
        case temp_sensor_t::GROUND_FLOOR:
            ctrl::ctrl_ground_floor_fsm::dispatch(ctrl::temp_update_evt(msg.temp));
            break;
        case temp_sensor_t::FLOOR_1:
            ctrl::ctrl_floor1_fsm::dispatch(ctrl::temp_update_evt(msg.temp));
            break;
        case temp_sensor_t::SENSORS_NUM:
            /* Note expected. Just for compiler satisfaction */
            update_widget = false;
            break;
    }

    if (true == update_widget)
    {
        home_screen_set_temp_value(msg.temp, static_cast<temp_sensor_t>(msg.temp_sensor_id));
    }
}

static void call_disp_ui_tick()
{
    static uint32_t stp = 0u;
    const uint32_t curr_tp = xTaskGetTickCount();
    const uint32_t TICK_INTERVAL_MS = 20u;

    if ((curr_tp - stp) >= pdMS_TO_TICKS(TICK_INTERVAL_MS))
    {
        stp = curr_tp;
        DisplayUI::instance().tick();
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
    DisplayUI::instance().init();

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
