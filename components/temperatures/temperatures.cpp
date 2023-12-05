#include <esp_err.h>
#include "bsp.h"
#include "temperatures.hpp"

template <typename NUM>
esp_err_t temperatures<NUM>::init() {
    esp_err_t err = ESP_OK;

    if (nullptr == bus) {
        err = install_onewire();
    }

    return err;
}

template <typename NUM>
esp_err_t temperatures<NUM>::enumerate_sensors() {
    esp_err_t err = ESP_ERR_NOT_FOUND;
    // TODO implement
    return err;
}

template <typename NUM>
esp_err_t temperatures<NUM>::install_onewire() {
    // install 1-wire bus
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = BSP_ONEWIRE_GPIO_NUM,
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10, // FIXME magic number! 1byte ROM command + 8byte ROM number + 1byte device command
    };

    return ESP_ERROR_CHECK_WITHOUT_ABORT(
        onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
}
