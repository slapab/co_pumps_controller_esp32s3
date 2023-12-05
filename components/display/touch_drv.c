#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/semphr.h"
#include "driver/i2c.h"
#include "esp_lcd_touch_cst816s.h"
#include "bsp.h"
#include "touch_drv.h"

#include "esp_log.h"

static SemaphoreHandle_t touch_int_semphr = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static esp_lcd_panel_io_handle_t i2c_io_handle = NULL;

static esp_err_t init_i2c(void);

static void touch_interrupt_cb(esp_lcd_touch_handle_t tp);

esp_err_t touch_drv_init(esp_lcd_touch_handle_t* o_touch_handle) {
    esp_err_t err = ESP_OK;

    /* Create semaphore to notify when touch interrupt occurred. */
    if (NULL == touch_int_semphr) {
        touch_int_semphr = xSemaphoreCreateBinary();
        err = (NULL == touch_int_semphr) ? ESP_ERR_NO_MEM : ESP_OK;
    }

    // /* Create mutex to access data  */
    // if (ESP_OK == err) {

    // }

    /* Initialize the I2C peripheral and interrupt pin */
    if (ESP_OK == err) {
        err = init_i2c();
    }

    /* Create I2C io device */
    if ((ESP_OK == err) && (NULL == i2c_io_handle)) {
      const esp_lcd_panel_io_i2c_config_t io_config =
          ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
      /* Casting I2C number to esp_lcd_i2c_bus_handle_t to match type.
         Internally driver casts this bus parameter to uint32_t. So passing
         number as an address here is valid one (although strange!) because
         parameters is not referenced as an address but as a number! */
      err = ESP_ERROR_CHECK_WITHOUT_ABORT(
          esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)BSP_TOUCH_I2C_NUM,
                                   &io_config, &i2c_io_handle));
    }

    /* Create and start the touch driver instance */
    if ((ESP_OK == err) && (NULL == touch_handle)) {
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = BSP_LCD_V_RES,
            .y_max = BSP_LCD_H_RES,
            .rst_gpio_num = BSP_TOUCH_PIN_NUM_RST,
            .int_gpio_num = BSP_TOUCH_PIN_NUM_INT,
            .levels = {
                .reset = 0,
                .interrupt = 0,
            },
            .flags = {
                .swap_xy = 1,
                .mirror_x = 1,
                .mirror_y = 0,
            },
            .interrupt_callback = touch_interrupt_cb,
        };
        err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_touch_new_i2c_cst816s(
            i2c_io_handle, &tp_cfg, &touch_handle));

        if ((ESP_OK == err) && (NULL != o_touch_handle)) {
            *o_touch_handle = touch_handle;
        }
    }

    return err;
}

bool touch_drv_read(int16_t* x_coord, int16_t* y_coord) {
    bool pressed = false;
    // Read only when ISR was triggered
    if (pdTRUE == xSemaphoreTake(touch_int_semphr, 0u)) {
        esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_touch_read_data(touch_handle));

        if (ESP_OK == err) {
            uint16_t touch_x[1];
            uint16_t touch_y[1];
            uint16_t touch_strength[1];
            uint8_t touch_cnt = 0;

            pressed = esp_lcd_touch_get_coordinates(
                touch_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);

            if (true == pressed) {
                ESP_LOGD("touch", "Touch pressed: x: %u, y: %u, strength: %u, touch count: %u",
                    touch_x[0],
                    touch_y[0],
                    touch_strength[0],
                    touch_cnt
                );
            }
            /* Apply filter for detected finger count. */
            pressed &= touch_cnt > 0;

            if (true == pressed) {
                if (NULL != x_coord) {
                    *x_coord = (int16_t)touch_x[0];
                }
                if (NULL != y_coord) {
                    *y_coord = (int16_t)touch_y[0];
                }
            }
        }
    }
    return pressed;
}

static esp_err_t init_i2c(void) {
    esp_err_t err = ESP_FAIL;
    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BSP_TOUCH_PIN_NUM_SDA,
        .scl_io_num = BSP_TOUCH_PIN_NUM_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = BSP_TOUCH_I2C_SPEED,
        }
    };
    /* Initialize I2C */
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_param_config(BSP_TOUCH_I2C_NUM, &i2c_conf));
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_driver_install(BSP_TOUCH_I2C_NUM, i2c_conf.mode, 0, 0, 0));
    return err;
}

static void touch_interrupt_cb(esp_lcd_touch_handle_t tp) {
    (void)tp;

    if (NULL != touch_int_semphr) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(touch_int_semphr, &xHigherPriorityTaskWoken);

        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}
