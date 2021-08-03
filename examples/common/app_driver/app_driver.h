// Copyright 2021 Espressif Systems (Shanghai) CO LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <esp_err.h>

#define SRC_MAX_NAMELEN 20

typedef enum app_driver_param_type {
    PARAM_TYPE_POWER = 0,
    PARAM_TYPE_BRIGHTNESS,
    PARAM_TYPE_MAX,
} app_driver_param_type_t;

typedef struct app_driver_param_callback {
    void (*update_power)(bool power);
    void (*update_brightness)(uint8_t brightness);
} app_driver_param_callback_t;

esp_err_t app_driver_init();
esp_err_t app_driver_update_and_report_power(bool power, const char *src);
esp_err_t app_driver_update_and_report_brightness(uint8_t brightness, const char *src);
bool app_driver_get_power();
uint8_t app_driver_get_brightness();

esp_err_t app_driver_register_src(const char *name, app_driver_param_callback_t *callbacks);

#ifdef __cplusplus
}
#endif
