#include "temperatures.hpp"
#include "widgets/lv_roller.h"
#include <algorithm>
#include <memory>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "ui.h"
#pragma GCC diagnostic pop

#include "display_ui.hpp"
#include "controller.hpp"
#include "resource_config.h"

#include <esp_err.h>
#include <esp_log.h>

#define LOG_TAG "home_scr"

#define LOG_PLACE() ESP_LOGI(LOG_TAG, "%s():%u", __func__, __LINE__)

static void set_temp_roller_value(lv_obj_t* roller, const uint32_t temp_desired, const uint32_t temp_min, const uint32_t temp_max);
static uint8_t get_temp_from_roller_value(lv_obj_t* roller, const uint32_t temp_min, const uint32_t temp_max);

void onTabChangedEventImpl(lv_event_t * e)
{
	// Your code here
    LOG_PLACE();
    ESP_LOGI(LOG_TAG, "target %p, current target %p, tab ID %u", e->target, e->current_target, lv_tabview_get_tab_act(ui_TabView1));

    if (e->target == ui_TabView1) {
        const uint16_t tab_id = lv_tabview_get_tab_act(e->target);
        ESP_LOGI(LOG_TAG, "Clicked tab id %u", tab_id);
        /* Unfortunately the tabview API doesnt provide getting selected tab
         * address, so need to hardcode by tab index (remember from the UI
         * design). */

        switch (tab_id) {
            case 0:
                /* Home */
                DisplayUI::instance().event_on_home_tab_entered();
                break;
            case 1:
                /* Settings */
                break;
            case 2:
                /* WiFi */
                break;
            default:
                /* not supported */
                break;
        }
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

void home_screen_set_temp_value(const float temp, const temp_sensor_t sensor)
{
    lv_obj_t* pump_status_comp = nullptr;
    switch (sensor)
    {
        case temp_sensor_t::GROUND_FLOOR:
            pump_status_comp = ui_homePumpStatusComponentFloor0;
            break;
        case temp_sensor_t::FLOOR_1:
            pump_status_comp = ui_homePumpStatusComponentFloor1;
            break;
        case temp_sensor_t::SENSORS_NUM:
            break;
    }

    if (nullptr != pump_status_comp)
    {
        /* Get label that shows the temp value */
        lv_obj_t* label = ui_comp_get_child( pump_status_comp, UI_COMP_HOMEPUMPSTATUSCOMPONENT_TEMPLABEL);
        lv_label_set_text_fmt(label, "%0.1f °C", temp);
    }
}

void home_screen_init_settings_rollers()
{
    /* NOTE: it doesn't support negative numbers */

    /* Calculate buffer length for roller settings for floor temps. Start from 1 to count the c-string null char. */
    size_t buf_size = 1u;
    for (uint32_t i = CONFIG_FLOOR_TEMP_SETTING_MIN; i <= CONFIG_FLOOR_TEMP_SETTING_MAX; ++i) {
        /* Append size for \n. Start after first element, and don't append after last. */
        if (i > CONFIG_FLOOR_TEMP_SETTING_MIN)
        {
            ++buf_size;
        }

        /* Append size required for given digit*/
        uint32_t digit = i;
        do {
            ++buf_size;
        } while (digit /= UINT32_C(10));

    }
    /* Allocate buffer for the options for rollers */
    auto options_buf = std::unique_ptr<char[]>(new char[buf_size]{});
    if (options_buf)
    {
        /* Fill the rollers for temperature */
        size_t wr_offset = 0u;
        for (uint32_t i = CONFIG_FLOOR_TEMP_SETTING_MIN; i <= CONFIG_FLOOR_TEMP_SETTING_MAX; ++i)
        {
            if (ESP_OK != ESP_ERROR_CHECK_WITHOUT_ABORT((wr_offset <= (buf_size - 1u)) ? ESP_OK : ESP_FAIL))
            {
                break;
            }
            const char* fmt = (i < CONFIG_FLOOR_TEMP_SETTING_MAX) ? "%u\n" : "%u";
            wr_offset += snprintf(&options_buf.get()[wr_offset], buf_size - wr_offset, fmt, i);
        }
        /* NULL char at the end */
        options_buf[buf_size - 1u] = '\0';

        /* Set the rollers options */
        lv_obj_t* roller = ui_comp_get_child(ui_settingsGroundFloor, UI_COMP_FLOORSETTINGSCOMPONENT_FLOORTEMPROLLER);
        lv_roller_set_options(roller, options_buf.get(), LV_ROLLER_MODE_INFINITE);
        roller = ui_comp_get_child(ui_settingsFloor1, UI_COMP_FLOORSETTINGSCOMPONENT_FLOORTEMPROLLER);
        lv_roller_set_options(roller, options_buf.get(), LV_ROLLER_MODE_INFINITE);
    }
    else
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(ESP_ERR_NO_MEM);
    }
}

void home_screen_set_ground_floor_temp_roller(const uint32_t temp)
{
    lv_obj_t* roller = ui_comp_get_child(ui_settingsGroundFloor, UI_COMP_FLOORSETTINGSCOMPONENT_FLOORTEMPROLLER);
    set_temp_roller_value(roller, temp, CONFIG_FLOOR_TEMP_SETTING_MIN, CONFIG_FLOOR_TEMP_SETTING_MAX);
}

void home_screen_set_floor1_temp_roller(const uint32_t temp)
{
    lv_obj_t* roller = ui_comp_get_child(ui_settingsFloor1, UI_COMP_FLOORSETTINGSCOMPONENT_FLOORTEMPROLLER);
    set_temp_roller_value(roller, temp, CONFIG_FLOOR_TEMP_SETTING_MIN, CONFIG_FLOOR_TEMP_SETTING_MAX);
}

void firstFloorValueTempSettChangedEventImpl(lv_event_t *e)
{
    const uint8_t temp = get_temp_from_roller_value(e->target, CONFIG_FLOOR_TEMP_SETTING_MIN, CONFIG_FLOOR_TEMP_SETTING_MAX);
    DisplayUI::instance().event_1st_floor_temp_changed(temp);
    ESP_LOGI(LOG_TAG, "first floor roller value changed to %u C", temp);
}

void groundFloorValueTempSettChangedEventImpl(lv_event_t *e)
{
    const uint8_t temp = get_temp_from_roller_value(e->target, CONFIG_FLOOR_TEMP_SETTING_MIN, CONFIG_FLOOR_TEMP_SETTING_MAX);
    DisplayUI::instance().event_gnd_floor_temp_changed(temp);
    ESP_LOGI(LOG_TAG, "ground floor roller value changed to %u C", temp);
}

static void set_temp_roller_value(lv_obj_t* roller, const uint32_t temp_desired, const uint32_t temp_min, const uint32_t temp_max)
{
    /* Calculate index of desired temp value. If temp is outside of bounds, just abort. */
    if ((nullptr != roller) && (temp_desired >= temp_min) && (temp_desired <= temp_max))
    {
        /* Convert to 0-based index */
        const uint16_t option_idx = temp_desired - temp_min;
        lv_roller_set_selected(roller, option_idx, LV_ANIM_OFF);
    }
}

static uint8_t get_temp_from_roller_value(lv_obj_t* roller, const uint32_t temp_min, const uint32_t temp_max)
{
    uint8_t temp = 20u; /* As default */

    /* Calculate index of desired temp value. If temp is outside of bounds, just abort. */
    if ((nullptr != roller) && (temp_min < temp_max))
    {
        const uint16_t sel_idx = lv_roller_get_selected(roller);
        /* As index is 0-based, but temp rollers might have first value greater
         * than zero, then just add the min temp value to the index to obtain a
         * temp value displayed in the roller. */
        temp = sel_idx + temp_min;
    }
    else
    {
        ESP_LOGE(LOG_TAG, "%s(): invalid arguments!", __func__);
    }

    return temp;
}
