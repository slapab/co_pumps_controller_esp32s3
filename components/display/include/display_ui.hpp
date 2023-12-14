#pragma once

#include <cstddef>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "display_ui_messages.h"
#include "freertos/portmacro.h"

class DisplayUI
{
 public:
   static DisplayUI& instance();

   void init();
   void tick();
   bool send_msg(const disp_ui_msg_t& msg, const uint32_t timeout_ticks = portMAX_DELAY);

 protected:
   constexpr static size_t MSQ_Q_LEN = 10u;
   constexpr static size_t MSG_Q_ITEM_SIZE = sizeof(disp_ui_msg_t);
   /**
    * @brief Some additional initialization. e.g. widgets label or sth like this
    */
   void setup_ui();

   bool get_msg(disp_ui_msg_t& msg, const uint32_t timeout_ticks = UINT32_C(0));

   void process_msg();

  /* Generate the declarations of message handles */
  #define X(enum_name, data_name) \
      void handle_msg(const disp_ui_##data_name##_t& msg);

  DISPLAY_UI_MSGS_LIST_X
  #undef X

   QueueHandle_t msg_q_handle = nullptr;
};
