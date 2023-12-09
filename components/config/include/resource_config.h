#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_LOG_TAG_DISPLAY_UI "disp_ui"
#define CONFIG_LOG_TAG_TEMPERATURE "temps"


#define CONFIG_TEMPERATURES_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define CONFIG_TEMPERATURES_TASK_DEPTH_BYTES (UINT32_C(3) * UINT32_C(1024))
/** Temperatures update interval in milliseconds */
#define CONFIG_TEMPERATURES_UPDATE_INTERVAL_MS UINT32_C(4000)


#ifdef __cplusplus
}
#endif
