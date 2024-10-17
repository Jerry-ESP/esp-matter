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

typedef enum {
    IDENTIFY_START = 0,
    IDENTIFY_STOP,
    IDENTIFY_EFFECT_BLINK,
    IDENTIFY_EFFECT_BREATH,
    IDENTIFY_EFFECT_OK,
    IDENTIFY_EFFECT_CHANGE,
    IDENTIFY_EFFECT_FINISH,
    IDENTIFY_EFFECT_STOP,
} identify_type_t;

typedef enum {
    COMMISSIONING_START = 0,
    COMMISSIONING_SUCCESS,
    COMMISSIONING_FAILED,
} commission_state_t;

esp_err_t indicator_callbacks_init();
esp_err_t identify_indicator(identify_type_t type);
esp_err_t commission_indicator(commission_state_t type);
