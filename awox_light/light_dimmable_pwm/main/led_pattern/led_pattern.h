// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <esp_err.h>
#include "DimmableDriver.hpp"

typedef void *led_pattern_handle_t;
typedef esp_err_t (*led_pattern_blink_callback_t)(led_pattern_handle_t pattern_handle);
typedef struct led_pattern {
    Dimmable::Driver led_driver;
    bool state;
    int count;
    bool end_pattern;
    bool restore_state;
    led_pattern_blink_callback_t callback;
} led_pattern_t;

esp_err_t led_pattern_blink_start(Dimmable::Driver& led_driver, int count, int delay_ms,
                                             led_pattern_blink_callback_t callback);
esp_err_t led_pattern_blink_stop_restore(led_pattern_handle_t handle, bool restore);
esp_err_t led_pattern_blink_stop_with_power(led_pattern_handle_t handle, bool on_off);
led_pattern_handle_t get_pattern_handle(void);
