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
#include <driver/rmt.h>
#include <led_strip.h>

#include <led_driver.h>

static const char *TAG = "led_driver_rmt";
static bool current_power = false;
static uint8_t current_brightness = 0;
static led_strip_t *strip = NULL;

esp_err_t led_driver_init(led_driver_config_t *config)
{
    ESP_LOGI(TAG, "Initializing led driver");
    esp_err_t err = ESP_OK;

    rmt_config_t rmt_cfg = RMT_DEFAULT_CONFIG_TX(config->gpio, config->channel);
    rmt_cfg.clk_div = 2;
    err = rmt_config(&rmt_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_cfg failed");
    }
    err = rmt_driver_install(rmt_cfg.channel, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_driver_install failed");
    }

    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)rmt_cfg.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "W2812 driver install failed");
    }

    return err;
}

esp_err_t led_driver_set_power(bool power)
{
    current_power = power;
    return ESP_OK;
}

esp_err_t led_driver_set_brightness(uint8_t brightness)
{
    esp_err_t err = ESP_OK;

    if (brightness != 0) {
        current_brightness = brightness;
    }
    if (!current_power) {
        brightness = 0;
    }
    if (!strip) {
        ESP_LOGE(TAG, "can't find w2812 led_strip handle");
    } else {
        err = strip->set_pixel(strip, 0, brightness, 0, brightness);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "strip_set_pixel failed");
        }
        err = strip->refresh(strip, 100);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "strip_refresh failed");
        }
    }
    return err;
}

bool led_driver_get_power()
{
    return current_power;
}

uint8_t led_driver_get_brightness()
{
    return current_brightness;
}
