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

#include <driver/ledc.h>
#include <esp_log.h>
#include <hal/ledc_types.h>
#include <color_format.h>
#include <led_driver.h>

static const char *TAG = "led_driver_gpio";
static bool current_power = false;
static uint8_t current_brightness = 0;
static uint32_t current_temp = 6600;
static HS_color_t current_HS = {0, 0};
static RGB_color_t mRGB;

esp_err_t led_driver_init(led_driver_config_t *config)
{
    ESP_LOGI(TAG, "Initializing light driver");
    esp_err_t err = ESP_OK;

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, // timer mode
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .timer_num = LEDC_TIMER_1, // timer index
        .freq_hz = 5000, // frequency of PWM signal
        .clk_cfg = LEDC_AUTO_CLK, // Auto select the source clock
    };
    err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "led_timerc_config failed");
        return err;
    }

    ledc_channel_config_t ledc_channel_r = {
        .gpio_num = config->gpio_r,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = config->channel_r,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0,
    };
    err = ledc_channel_config(&ledc_channel_r);

    ledc_channel_config_t ledc_channel_g = {
        .gpio_num = config->gpio_g,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = config->channel_g,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0,
    };
    err = ledc_channel_config(&ledc_channel_g);

    ledc_channel_config_t ledc_channel_b = {
        .gpio_num = config->gpio_b,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = config->channel_b,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0,
    };
    err = ledc_channel_config(&ledc_channel_b);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed");
    }

    return err;
}

esp_err_t led_driver_set_power(bool power)
{
    current_power = power;
    return led_driver_set_brightness(current_brightness);
}

esp_err_t led_driver_set_RGB()
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, mRGB.red);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, mRGB.green);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, mRGB.blue);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

    return ESP_OK;
}

esp_err_t led_driver_set_brightness(uint8_t brightness)
{
    if (!current_power) {
        brightness = 0;
        current_brightness = 0;
    }
    if (brightness != 0) {
        current_brightness = brightness;
    }
    hsv_to_rgb(current_HS, brightness, &mRGB);
    return led_driver_set_RGB();
}

esp_err_t led_driver_set_hue(uint16_t hue)
{
    uint8_t brightness = current_power ? current_brightness : 0;
    current_HS.hue = hue;
    hsv_to_rgb(current_HS, brightness, &mRGB);
    return led_driver_set_RGB();
}

esp_err_t led_driver_set_saturation(uint8_t saturation)
{
    uint8_t brightness = current_power ? current_brightness : 0;
    current_HS.saturation = saturation;
    hsv_to_rgb(current_HS, brightness, &mRGB);
    return led_driver_set_RGB();
}

esp_err_t led_driver_set_temperature(uint32_t temperature)
{
    uint8_t brightness = current_power ? current_brightness : 0;
    current_temp = temperature;
    temp_to_hs(current_temp, &current_HS);
    hsv_to_rgb(current_HS, brightness, &mRGB);
    return led_driver_set_RGB();
}

bool led_driver_get_power()
{
    return current_power;
}

uint8_t led_driver_get_brightness()
{
    return current_brightness;
}

uint16_t led_driver_get_hue()
{
    return current_HS.hue;
}

uint8_t led_driver_get_saturation()
{
    return current_HS.saturation;
}

uint32_t led_driver_get_temperature()
{
    return current_temp;
}
