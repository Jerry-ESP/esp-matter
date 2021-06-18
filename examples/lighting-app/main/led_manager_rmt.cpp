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

#include "led_manager.hpp"

#include "esp_err.h"
#include "esp_log.h"
#include "lighting_app_constants.hpp"
#include "driver/rmt.h"
#include "led_strip.h"
#define RMT_TX_DEFAULT_GPIO GPIO_NUM_8
#define RMT_TX_DEFAULT_CHANNEL RMT_CHANNEL_0
static led_strip_t *strip = NULL;
static uint8_t last_brightness;
static bool current_onoff;

esp_err_t led_manager_init(uint8_t default_brightness)
{
    esp_err_t error;
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(RMT_TX_DEFAULT_GPIO, RMT_TX_DEFAULT_CHANNEL);
    config.clk_div = 2;
    error = rmt_config(&config);
    if (error != ESP_OK) {
        ESP_LOGE(APP_LOG_TAG, "rmt_config failed");
    }
    error = rmt_driver_install(config.channel, 0, 0);
    if (error != ESP_OK) {
        ESP_LOGE(APP_LOG_TAG, "rmt_driver_install failed");
    }
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(APP_LOG_TAG, "W2812 driver install failed");
    }

    last_brightness = default_brightness;

    return ESP_OK;
}

esp_err_t led_set_onoff(bool onoff)
{
    current_onoff = onoff;
    return ESP_OK;
}

esp_err_t led_set_brightness(uint8_t brightness)
{
    esp_err_t error;

    if (brightness != 0) {
        last_brightness = brightness;
    }
    if (!current_onoff) {
        brightness = 0;
    }
    if (!strip) {
        ESP_LOGE(APP_LOG_TAG, "can't find w2812 led_strip handle");
    } else {
        error = strip->set_pixel(strip, 0, brightness, 0, brightness);
        if (error != ESP_OK) {
            ESP_LOGE(APP_LOG_TAG, "strip_set_pixel failed");
        }
        error = strip->refresh(strip, 100);
        if (error != ESP_OK) {
            ESP_LOGE(APP_LOG_TAG, "strip_refresh failed");
        }
    }
    return ESP_OK;
}
