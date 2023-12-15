#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_LOG_TAG_DISPLAY_UI "disp_ui"
#define CONFIG_LOG_TAG_TEMPERATURE "temps"
#define CONFIG_LOG_TAG_PUMPS "pumps"
#define CONFIG_LOG_TAG_CTRL "ctrl"


#define CONFIG_TEMPERATURES_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define CONFIG_TEMPERATURES_TASK_DEPTH_BYTES (UINT32_C(3) * UINT32_C(1024))
/** Temperatures update interval in milliseconds */
#define CONFIG_TEMPERATURES_UPDATE_INTERVAL_MS UINT32_C(4000)

/* UI configuration */
#define CONFIG_FLOOR_TEMP_SETTING_MIN 16u
#define CONFIG_FLOOR_TEMP_SETTING_MAX 64u
#define CONFIG_FLOOR_TEMP_SETTING_DEFAULT 32u


#ifdef __cplusplus
}
#endif
