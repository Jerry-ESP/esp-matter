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

#include <string.h>

#include <esp_err.h>
#include <esp_log.h>

#include <lightbulb.h>
#include <indicator.h>

typedef struct {
    uint8_t color_select;
    uint8_t r;   /* range: 0-255 */
    uint8_t g;   /* range: 0-255 */
    uint8_t b;   /* range: 0-255 */
    uint16_t cct; /* range: 0-100 or min-max kelvin */
} __attribute__((packed)) color_custom_t;

typedef enum {
    INDICATOR_MODE_RESTORE = 0, /* restore to previous state */
    INDICATOR_MODE_BLINK, /* effect: blink */
    INDICATOR_MODE_BREATHE, /* effect: breathe */
    INDICATOR_MODE_MAX,
} indicator_mode_t;

typedef struct {
    indicator_mode_t mode;
    uint16_t speed;
    uint8_t min_brightness; /* range: 0-100 */
    uint8_t max_brightness; /* range: 0-100, also used in white_mode */
    color_custom_t color;
    int total_ms; /* if total_ms > 0, will enable auto-stop timer */
    /*
     * If interrupt_forbidden set to true, the auto-stop timer can only be stopped
     * by effect_stop/effect_start and led_pattern_*_stop/led_pattern_*_start interfaces or triggered by FreeRTOS.
     * Any set APIs will only save the status, the status will not be written.
     */
    bool interrupt_forbidden;
} __attribute__((packed)) indicator_config_t;

static const char *TAG = "led_indicator";

indicator_config_t commission_started = {
    .mode = INDICATOR_MODE_BLINK,
    .speed = 500,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 0,
        .g = 0,
        .b = 255,
    },
    .total_ms = 1500,
};

indicator_config_t commission_failed = {
    .mode = INDICATOR_MODE_BLINK,
    .speed = 1000,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 255,
        .g = 0,
        .b = 0,
    },
    .total_ms = 5000,
};

indicator_config_t commission_success = {
    .mode = INDICATOR_MODE_BREATHE,
    .speed = 1000,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 0,
        .g = 255,
        .b = 0,
    },
    .total_ms = 3000,
};

indicator_config_t identify_start = {
    .mode = INDICATOR_MODE_BLINK,
    .speed = 1000,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 255,
        .g = 0,
        .b = 0,
    },
    .total_ms = 0,
};

indicator_config_t identify_stop = {
    .mode = INDICATOR_MODE_RESTORE,
};

indicator_config_t identify_blink = {
    .mode = INDICATOR_MODE_BLINK,
    .speed = 1000,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 255,
        .g = 255,
        .b = 255,
    },
    .total_ms = 1000,
};

indicator_config_t identify_breath = {
    .mode = INDICATOR_MODE_BREATHE,
    .speed = 1000,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 255,
        .g = 255,
        .b = 255,
    },
    .total_ms = 15000,
};

indicator_config_t identify_okay = {
    .mode = INDICATOR_MODE_BLINK,
    .speed = 2000,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 0,
        .g = 255,
        .b = 0,
    },
    .total_ms = 1000,
};

indicator_config_t identify_channel_change = {
    .mode = INDICATOR_MODE_BLINK,
    .speed = 16000,
    .min_brightness = 20,
    .max_brightness = 100,
    .color = {
        .color_select = 1,
        .r = 255,
        .g = 165,
        .b = 0,
    },
    .total_ms = 8000,
};

indicator_config_t identify_finish = {
    .mode = INDICATOR_MODE_RESTORE,
};

static void led_indicator_lightbulb_effect_callback()
{
    ESP_LOGI(TAG, "Lightbulb effect complete, restore to previous state");
    lightbulb_basic_effect_stop_and_restore();
}

static esp_err_t indicator_trigger_lightbulb(indicator_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "lightbulb indicator config is NULL");
        return ESP_FAIL;
    }

    if (config->mode == INDICATOR_MODE_BREATHE || config->mode == INDICATOR_MODE_BLINK) {
        lightbulb_effect_config_t effect_config = {
            .effect_type = (lightbulb_effect_t)(config->mode == INDICATOR_MODE_BREATHE ? EFFECT_BREATH : EFFECT_BLINK),
            .mode = (lightbulb_works_mode_t)config->color.color_select,
            .cct = config->color.cct,
            .min_value_brightness = config->min_brightness,
            .max_value_brightness = config->max_brightness,
            .effect_cycle_ms = config->speed,
            .total_ms = config->total_ms,
            .user_cb = led_indicator_lightbulb_effect_callback,
            .interrupt_forbidden = config->interrupt_forbidden,
        };

        uint8_t calculate_value;
        lightbulb_rgb2hsv(config->color.r, config->color.g, config->color.b, &effect_config.hue, &effect_config.saturation, &calculate_value);
        get_light_driver().indicaceBreathOrBlink(&effect_config);
    } else if (config->mode == INDICATOR_MODE_RESTORE) {
        // Temporarily force power on when the lighting effect stops and restore.
        get_light_driver().indicaceRestore();
    }

    return ESP_OK;
}

esp_err_t identify_indicator(identify_type_t type)
{
    switch (type) {
        case IDENTIFY_START:
            indicator_trigger_lightbulb(&identify_start);
        break;
        case IDENTIFY_STOP:
            indicator_trigger_lightbulb(&identify_stop);
        break;
        case IDENTIFY_EFFECT_BLINK:
            indicator_trigger_lightbulb(&identify_blink);
        break;
        case IDENTIFY_EFFECT_BREATH:
            indicator_trigger_lightbulb(&identify_breath);
        break;
        case IDENTIFY_EFFECT_CHANGE:
            indicator_trigger_lightbulb(&identify_channel_change);
        break;
        case IDENTIFY_EFFECT_OK:
            indicator_trigger_lightbulb(&identify_okay);
        break;
        case IDENTIFY_EFFECT_FINISH:
            indicator_trigger_lightbulb(&identify_finish);
        break;
        case IDENTIFY_EFFECT_STOP:
            indicator_trigger_lightbulb(&identify_stop);
        break;
        default:
        break;
    }

    return ESP_OK;
}

esp_err_t commission_indicator(commission_state_t type)
{
    switch (type) {
        case COMMISSIONING_START:
            indicator_trigger_lightbulb(&commission_started);
        break;
        case COMMISSIONING_SUCCESS:
            indicator_trigger_lightbulb(&commission_success);
        break;
        case COMMISSIONING_FAILED:
            indicator_trigger_lightbulb(&commission_failed);
        break;
        default:
        break;
    }

    return ESP_OK;
}

esp_err_t indicator_callbacks_init()
{
    return ESP_OK;
}
