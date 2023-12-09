#pragma once

#include <esp_err.h>
#include <cstdint>
#include <stdint.h>
#include <array>
#include "onewire_bus.h"
#include "ds18b20.h"

template <uint8_t NUM>
class temperatures {
public:
    temperatures(const int gpio_num)
        : gpio_num(gpio_num) { }

    esp_err_t init();
    esp_err_t enumerate_sensors();
    esp_err_t get_sample();
    uint8_t get_sensors_count() const { return found_sensors_count; }

protected:
    esp_err_t install_onewire();

protected:
    /** 1byte ROM command + 8byte ROM number + 1byte device command */
    constexpr static uint32_t ONEWIRE_MAX_RX_BYTES = 10u;

    onewire_bus_handle_t bus = nullptr;
    ds18b20_device_handle_t ds18b20s[NUM] = {};
    /** Storing addresses as separate data because cannot get addresses from the library */
    onewire_device_address_t ds18b20s_addrs[NUM] = {};
    float temps[NUM] = {};

    uint8_t found_sensors_count = 0u;

    const int gpio_num;
};
