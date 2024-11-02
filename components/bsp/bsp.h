#pragma once

#include <hal/gpio_types.h>
#include <hal/i2c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_BSP_HW_REV_ESP32C3_DEVKITC

#define BSP_ONEWIRE_GPIO_NUM GPIO_NUM_18

#elif CONFIG_BSP_HW_REV_LILYGO_TDISPLAY_S3

#define BSP_ONEWIRE_GPIO_NUM GPIO_NUM_13

#define BSP_PUMP_RELAY_GROUND_FLOOR_GPIO_NUM GPIO_NUM_1
#define BSP_PUMP_RELAY_FLOOR_1_GPIO_NUM GPIO_NUM_2

#define BSP_LCD_H_RES   320u
#define BSP_LCD_V_RES   170u

#define BSP_TOUCH_I2C_NUM                  I2C_NUM_0
#define BSP_TOUCH_I2C_SPEED                400000u

#define BSP_TOUCH_PIN_NUM_SDA              GPIO_NUM_18
#define BSP_TOUCH_PIN_NUM_SCL              GPIO_NUM_17
#define BSP_TOUCH_PIN_NUM_INT              GPIO_NUM_16
#define BSP_TOUCH_PIN_NUM_RST              GPIO_NUM_21

#define BSP_PWR_EN_PIN_NUM                 GPIO_NUM_15

#endif

#ifdef __cplusplus
}
#endif
