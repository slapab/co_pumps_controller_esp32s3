#include <algorithm>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "ui.h"
#pragma GCC diagnostic pop

#include "controller.hpp"

#include <esp_log.h>

#define LOG_TAG "home_scr"

#define LOG_PLACE() ESP_LOGI(LOG_TAG, "%s():%u", __func__, __LINE__)

void onTabChangedEventImpl(lv_event_t * e)
{
	// Your code here
    ESP_LOGI(LOG_TAG, "target %p, current target %p, tab ID %u", e->target, e->current_target, lv_tabview_get_tab_act(ui_TabView1));
    LOG_PLACE();

    if (e->target == ui_TabView1) {
        uint16_t tab_id = lv_tabview_get_tab_act(ui_TabView1);
        lv_obj_t * btn_matrix = lv_tabview_get_tab_btns(ui_TabView1);
        const char* btn_text = lv_btnmatrix_get_btn_text(btn_matrix, tab_id);
        ESP_LOGI(LOG_TAG, "Clicked tab id %u, text: %s", tab_id, btn_text);
    }
}

void floor0ClickedEventImpl(lv_event_t * e)
{
	// Your code here
    LOG_PLACE();
    const bool is_checked = lv_obj_has_state(e->target, LV_STATE_CHECKED);
    ESP_LOGI(LOG_TAG, "Is checked? %d", is_checked);
    ctrl::ctrl_ground_floor_fsm::dispatch(ctrl::manual_pump_ctrl_evt(is_checked));
}

void floor1ClickedEventImpl(lv_event_t * e)
{
	// Your code here
    LOG_PLACE();
    const bool is_checked = lv_obj_has_state(e->target, LV_STATE_CHECKED);
    ESP_LOGI(LOG_TAG, "Is checked? %d", is_checked);
    ctrl::ctrl_floor1_fsm::dispatch(ctrl::manual_pump_ctrl_evt(is_checked));
}
