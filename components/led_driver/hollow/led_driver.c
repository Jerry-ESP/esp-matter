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

#include <esp_log.h>
#include <led_driver.h>

static const char *TAG = "led_driver_hollow";

esp_err_t led_driver_init(led_driver_config_t *config)
{
    ESP_LOGI(TAG, "Initializing led driver");
    /* Initialize the driver here */

    return ESP_OK;
}

esp_err_t led_driver_set_power(bool power)
{
    ESP_LOGI(TAG, "Setting power to: %d", power);
    /* Set the power state here */

    return ESP_OK;
}

esp_err_t led_driver_set_brightness(uint8_t brightness)
{
    ESP_LOGI(TAG, "Setting brightness to: %d", brightness);
    /* Set the brightness level here */

    return ESP_OK;
}
