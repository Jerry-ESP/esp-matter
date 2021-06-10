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
#include "driver/ledc.h"
#include "hal/ledc_types.h"

static uint8_t last_brightness;
static bool current_onoff;

esp_err_t led_manager_init(uint8_t default_brightness)
{
    esp_err_t error;

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, // timer mode
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .timer_num = LEDC_TIMER_1, // timer index
        .freq_hz = 5000, // frequency of PWM signal
        .clk_cfg = LEDC_AUTO_CLK, // Auto select the source clock
    };
    error = ledc_timer_config(&ledc_timer);
    if (error != ESP_OK) {
        ESP_LOGE(APP_LOG_TAG, "led_timerc_config failed");
        return error;
    }
    ledc_channel_config_t ledc_channel = {.gpio_num = CONFIG_LED_PIN,
                                          .speed_mode = LEDC_LOW_SPEED_MODE,
                                          .channel = LEDC_CHANNEL_0,
                                          .intr_type = LEDC_INTR_DISABLE,
                                          .timer_sel = LEDC_TIMER_1,
                                          .duty = 0,
                                          .hpoint = 0,
                                          .flags = {}};
    ledc_channel.flags.output_invert = 0;
    error = ledc_channel_config(&ledc_channel);
    if (error != ESP_OK) {
        ESP_LOGE(APP_LOG_TAG, "ledc_channel_config failed");
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

    error = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    if (error != ESP_OK) {
        ESP_LOGE(APP_LOG_TAG, "ledc_set_duty failed");
    }

    error = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    if (error != ESP_OK) {
        ESP_LOGE(APP_LOG_TAG, "ledc_update_duty failed");
    }

    return ESP_OK;
}
