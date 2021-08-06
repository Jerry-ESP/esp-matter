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

#include "app_driver.h"
#include "app_matter.h"

#include "esp_err.h"
#include "esp_log.h"
#include "lighting_app_constants.hpp"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lib/shell/Engine.h"

#if CONFIG_ENABLE_CHIP_SHELL
void ChipShellTask(void *args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}
#endif // CONFIG_ENABLE_CHIP_SHELL

extern "C" void app_main()
{
    // Initialize the ESP NVS layer.
    ESP_ERROR_CHECK(nvs_flash_init());

    /* Initialize and set the default params */
    app_driver_init();

    ESP_LOGI(APP_LOG_TAG, "==================================================");
    ESP_LOGI(APP_LOG_TAG, "chip-esp32-lighting-example starting");
    ESP_LOGI(APP_LOG_TAG, "==================================================");

    /* Initialize chip */
    ESP_ERROR_CHECK(app_matter_init());

    app_driver_update_and_report_power(DEFAULT_POWER, APP_DRIVER_SRC_LOCAL);
    app_driver_update_and_report_brightness(DEFAULT_BRIGHTNESS, APP_DRIVER_SRC_LOCAL);
    app_driver_update_and_report_hue(DEFAULT_HUE, APP_DRIVER_SRC_LOCAL);
    app_driver_update_and_report_saturation(DEFAULT_SATURATION, APP_DRIVER_SRC_LOCAL);

#if CONFIG_ENABLE_CHIP_SHELL
    xTaskCreate(&ChipShellTask, "chip_shell", 2048, NULL, 5, NULL);
#endif
}
