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

/**
 * @brief The param update callbacks, notify the registered sources that the param
 *        was updated by other source.
 *
 */
typedef struct app_driver_param_callback {
    void (*update_power)(bool power);               /* Update power (On/Off) */
    void (*update_brightness)(uint8_t brightness);  /* Update brightness (On/Off) */
} app_driver_param_callback_t;

/**
 * @brief Initializes the application driver layer.
 *
 * @return
 *      - ESP_OK on success
 *
 */
esp_err_t app_driver_init(void);

/**
 * @brief Register a control source.
 *
 * @param[in]  name       The source name.
 * @param[in]  callbacks  The param update callbacks.
 *
 * @note Each source can just register the param callbacks it cares about, and leave the others as NULL.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if any of the param is invalid
 *      - ESP_ERR_NO_MEM if register failed due to out of memory
 *
 */
esp_err_t app_driver_register_src(const char *name, app_driver_param_callback_t *callbacks);

/**
 * @brief Change the power (On/Off) param.
 *
 * @param[in]  power   The new power value.
 * @param[in]  src     The source that the change comes from.
 *
 * @return
 *      - ESP_OK on success
 *
 */
esp_err_t app_driver_update_and_report_power(bool power, const char *src);

/**
 * @brief Change the brightness param.
 *
 * @param[in]  brightness   The new brightness value.
 * @param[in]  src          The source that the change comes from.
 *
 * @return
 *      - ESP_OK on success
 *
 */
esp_err_t app_driver_update_and_report_brightness(uint8_t brightness, const char *src);

/**
 * @brief Get the power (On/Off) value.
 *
 * @return The current power (On/Off) value.
 *
 */
bool app_driver_get_power(void);

/**
 * @brief Get the brightness value.
 *
 * @return The current brightness value.
 *
 */
uint8_t app_driver_get_brightness(void);

#ifdef __cplusplus
}
#endif
