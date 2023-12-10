#include <esp_err.h>
#include "bsp.h"
#include "temperatures.hpp"
#include "ds18b20.h"
#include "freertos/projdefs.h"
#include "resource_config.h"

#include <esp_log.h>

#define LOG_TAG CONFIG_LOG_TAG_TEMPERATURE

static void temperatures_task_impl(void* param);

template <uint8_t NUM>
esp_err_t temperatures<NUM>::init() {
    esp_err_t err = ESP_OK;

    if (nullptr == bus) {
        err = install_onewire();
    }

    return err;
}

template <uint8_t NUM>
esp_err_t temperatures<NUM>::enumerate_sensors() {
    esp_err_t err = ESP_ERR_NOT_FOUND;

    found_sensors_count = 0u;

    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;

    // create 1-wire device iterator, which is used for device search
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(onewire_new_device_iter(bus, &iter));
    if (ESP_OK == err)
    {
        ESP_LOGI(LOG_TAG, "Device iterator created, start searching...");
        do {
            err = onewire_device_iter_get_next(iter, &next_onewire_device);
            if (ESP_OK == err) {
                /* Found a new device, let's check if we can upgrade it to a DS18B20 */
                ds18b20_config_t ds_cfg = {};
                /* Check if the device is a DS18B20, if so, return the ds18b20 handle */
                if (ds18b20_new_device(&next_onewire_device, &ds_cfg, &ds18b20s[found_sensors_count]) == ESP_OK) {
                    ESP_LOGI(LOG_TAG, "Found a DS18B20[%d], address: %016llX", found_sensors_count, next_onewire_device.address);
                    ds18b20s_addrs[found_sensors_count] = next_onewire_device.address;
                    ++found_sensors_count;
                } else {
                    ESP_LOGI(LOG_TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
                }
            }
        } while (ESP_ERR_NOT_FOUND != err);

        /* Cleanup */
        ESP_ERROR_CHECK_WITHOUT_ABORT(onewire_del_device_iter(iter));

        ESP_LOGI(LOG_TAG, "Searching done, %d DS18B20 device(s) found", found_sensors_count);

        /* Update result value */
        if (found_sensors_count >= NUM)
        {
            err = ESP_OK;
        } else if (0u == found_sensors_count)
        {
            err = ESP_ERR_NOT_FOUND;
        } else {
            err = ESP_FAIL;
        }
    }

    return err;
}

template <uint8_t NUM>
esp_err_t temperatures<NUM>::get_sample() {
    bool fail = false;
    for (uint8_t idx = 0u; idx < NUM; ++idx)
    {
        ESP_LOGD(LOG_TAG, "Triggering temp conversion for sensor %016llX", ds18b20s_addrs[idx]);

        /* Note this will also wait for temp conversion finished */
        esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(ds18b20_trigger_temperature_conversion(ds18b20s[idx]));
        if (ESP_OK == err)
        {
            err = ESP_ERROR_CHECK_WITHOUT_ABORT(ds18b20_get_temperature(ds18b20s[idx], &temps[idx]));
        }

        if (ESP_OK == err)
        {
            ESP_LOGD(LOG_TAG, "Temperature for sensor %016llX is %0.2f C", ds18b20s_addrs[idx], temps[idx]);
        }
        else
        {
            fail = true;
        }
    }

    return (false == fail) ? ESP_OK : ESP_FAIL;
}

template <uint8_t NUM>
esp_err_t temperatures<NUM>::install_onewire() {
    // install 1-wire bus
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = gpio_num,
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = ONEWIRE_MAX_RX_BYTES
    };

    return ESP_ERROR_CHECK_WITHOUT_ABORT(
        onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
}

extern "C" void temperatures_bootstrap(void) {
    /* Create temperatures module task. */
    if (pdPASS != xTaskCreate(temperatures_task_impl, "temps_task",
                                CONFIG_TEMPERATURES_TASK_DEPTH_BYTES, NULL,
                                CONFIG_TEMPERATURES_TASK_PRIORITY, NULL))
    {
        ESP_LOGE(LOG_TAG, "Failed to create a temperatures task!");
    }
}

static void temperatures_task_impl(void* param)
{
    (void)param;

    #warning FIXME update to 2 sensors
    temperatures<2u> temps(BSP_ONEWIRE_GPIO_NUM);

    esp_err_t err = ESP_ERROR_CHECK_WITHOUT_ABORT(temps.init());
    if (ESP_OK != err) {
        ESP_LOGE(LOG_TAG, "Failed to initialize one wire transport for temp sensors");
        vTaskDelete(NULL);
    }

    if (ESP_OK == err)
    {
        do {
            err = ESP_ERROR_CHECK_WITHOUT_ABORT(temps.enumerate_sensors());

            if (ESP_OK == err)
            {
                ESP_LOGI(LOG_TAG, "Temperatures task started");
            }
            else {
                /* Retry after 3seconds */
                vTaskDelay(pdMS_TO_TICKS(3000u));
            }
        } while (ESP_OK != err);
    }

    uint32_t last_time_sample = xTaskGetTickCount();
    /* Task loop */
    while (true)
    {
        temps.get_sample();
        // TODO push event to logic
        vTaskDelayUntil(&last_time_sample, pdMS_TO_TICKS(CONFIG_TEMPERATURES_UPDATE_INTERVAL_MS));

    }
}
