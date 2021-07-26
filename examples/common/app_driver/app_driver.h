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

typedef enum app_driver_src {
    SRC_MATTER = 0,
    SRC_RAINMAKER,
    SRC_LOCAL,
    SRC_MAX,
} app_driver_src_t;

esp_err_t app_driver_init();
esp_err_t app_driver_update_and_report_power(bool power, app_driver_src_t src);
esp_err_t app_driver_update_and_report_brightness(uint8_t brightness, app_driver_src_t src);
bool app_driver_get_power();
uint8_t app_driver_get_brightness();
void app_driver_set_callbacks(app_driver_src_t src, void (*update_power)(bool power), void (*update_brightness)(uint8_t brightness));
esp_err_t app_driver_register_cli();

#ifdef __cplusplus
}
#endif
