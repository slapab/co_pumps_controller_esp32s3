#pragma once

#include <esp_err.h>
#include <cstdint>
#include <stdint.h>
#include <array>
#include "onewire_bus.h"

template <typename NUM>
class temperatures {
public:
    esp_err_t init();
    esp_err_t enumerate_sensors();
    uint8_t get_sensors_count() const { return found_sensors_count; }

protected:
    esp_err_t install_onewire();

protected:
    onewire_bus_handle_t bus = nullptr;
    uint8_t found_sensors_count = 0u;
};
