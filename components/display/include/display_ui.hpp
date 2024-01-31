#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>  /* std::pair */
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "display_ui_messages.h"
#include "freertos/portmacro.h"
#include "resource_config.h"

class DisplayUINvsData
{
public:
  static constexpr const char* nvs_namespace = "controller";

  esp_err_t load_from_nvs();
  esp_err_t save_to_nvs();

  bool is_modified() const { return modified; }

  uint8_t get_gnd_floor_temp() const {  return temp_ground_floor.second; }
  uint8_t get_1st_floor_temp() const {  return temp_first_floor.second; }
  uint8_t get_temp_hysteresis() const { return hysteresis.second; }

  void set_gnd_floor_temp(const uint8_t temp) { set_if_different(temp, temp_ground_floor); }
  void set_1st_floor_temp(const uint8_t temp) { set_if_different(temp, temp_first_floor); }
  void set_temp_hysteresis(const uint8_t hyst) { set_if_different(hyst, hysteresis); }

private:
  template <typename T>
  void set_if_different(const T value, std::pair<const char*, T>& a_pair)
  {
      if (value != a_pair.second)
      {
          modified = true;
          a_pair.second = value;
      }
  }

  bool modified = false;

  std::pair<const char*, uint8_t> temp_ground_floor = std::make_pair("t_gnd_floor", CONFIG_FLOOR_TEMP_SETTING_DEFAULT);
  std::pair<const char*, uint8_t> temp_first_floor = std::make_pair("t_1st_floor", CONFIG_FLOOR_TEMP_SETTING_DEFAULT);
  /** On->Off->On dTemperature in degC. */
  std::pair<const char*, uint8_t> hysteresis = std::make_pair("hysteresis", CONFIG_FLOOR_TEMP_HYSTERESIS_DEFAULT);;
};

class DisplayUI
{
public:
    static DisplayUI &instance();

    void init();
    void tick();
    bool send_msg(const disp_ui_msg_t &msg, const uint32_t timeout_ticks = portMAX_DELAY);

    /* Methods to be accessed from UI events */
public:
    void event_gnd_floor_temp_changed(const uint8_t temp)
    {
        nvs_data.set_gnd_floor_temp(temp);
    }
    void event_1st_floor_temp_changed(const uint8_t temp)
    {
        nvs_data.set_1st_floor_temp(temp);
    }
    void event_on_home_tab_entered();

protected:
    constexpr static size_t MSQ_Q_LEN       = 10u;
    constexpr static size_t MSG_Q_ITEM_SIZE = sizeof(disp_ui_msg_t);

    /**
     * @brief Some additional initialization. e.g. widgets label or sth like this
     */
    void setup_ui();

    bool get_msg(disp_ui_msg_t &msg, const uint32_t timeout_ticks = UINT32_C(0));

    void process_msg();

    void update_ctrl_fsms_settings();

    /* Generate the declarations of message handles */
    #define X(enum_name, data_name) void handle_msg(const disp_ui_msg_##data_name##_t &msg);

    DISPLAY_UI_MSGS_LIST_X
    #undef X

    QueueHandle_t msg_q_handle = nullptr;
    DisplayUINvsData nvs_data;
};
