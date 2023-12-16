#pragma once
/**
 * @file
 *
 * @note that events declarations clicked in the stored in auto generated file: @see ui_events.h
 */

#include "temperatures.hpp"

void home_screen_set_temp_value(const float temp, const temp_sensor_t sensor);

void home_screen_init_settings_rollers();

void home_screen_set_ground_floor_temp_roller(const uint32_t temp);
void home_screen_set_floor1_temp_roller(const uint32_t temp);

void home_screen_set_ground_floor_pump_status_progress(const uint8_t percentage);
void home_screen_set_floor1_pump_status_progress(const uint8_t percentage);
