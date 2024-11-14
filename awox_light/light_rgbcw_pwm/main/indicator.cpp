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

#include <indicator.h>
#include <led_pattern.h>

typedef struct {
    uint16_t blink_count;
    uint16_t delay_ms;
    bool if_restore;
} indicator_config_t;

static const char *TAG = "led_indicator";

indicator_config_t commission_started = {
    .blink_count = 3,
    .delay_ms = 500,
    .if_restore = true,
};

indicator_config_t commission_failed = {
    .blink_count = 5,
    .delay_ms = 1000,
    .if_restore = true,
};

indicator_config_t commission_success = {
    .blink_count = 3,
    .delay_ms = 1000,
    .if_restore = true,
};

indicator_config_t identify_start = {
    .blink_count = 0xfffe,
    .delay_ms = 1000,
    .if_restore = true,
};

indicator_config_t identify_stop = {
    .blink_count = 0,
    .delay_ms = 0,
    .if_restore = true,
};

indicator_config_t identify_blink = {
    .blink_count = 1,
    .delay_ms = 1000,
    .if_restore = true,
};

indicator_config_t identify_breath = {
    .blink_count = 15,
    .delay_ms = 1000,
    .if_restore = true,
};

indicator_config_t identify_okay = {
    .blink_count = 1,
    .delay_ms = 2000,
    .if_restore = true,
};

indicator_config_t identify_channel_change = {
    .blink_count = 1,
    .delay_ms = 16000,
    .if_restore = true,
};

indicator_config_t identify_finish = {
    .blink_count = 0,
    .delay_ms = 0,
    .if_restore = true,
};

static esp_err_t led_indicator_lightbulb_effect_callback(led_pattern_handle_t pattern_handle)
{
    ESP_LOGI(TAG, "Lightbulb effect complete, restore to previous state");
    led_pattern_t *pattern = (led_pattern_t *)pattern_handle;
    pattern->led_driver.setOn();
    return ESP_OK;
}

static esp_err_t indicator_trigger_lightbulb(indicator_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "lightbulb indicator config is NULL");
        return ESP_FAIL;
    }

    if (config->blink_count > 0) {
        led_pattern_blink_start(get_light_driver(), config->blink_count, config->delay_ms,
                                config->if_restore ? led_indicator_lightbulb_effect_callback : NULL);
    } else {
        led_pattern_blink_stop_restore(get_pattern_handle(), config->if_restore);
    }

    return ESP_FAIL;
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
