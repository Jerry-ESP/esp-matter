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
#include <esp_console.h>

#include <app_driver.h>
#include <board.h>
#include <led_driver.h>

typedef enum app_driver_param_type {
    PARAM_TYPE_POWER = 0,
    PARAM_TYPE_BRIGHTNESS,
    PARAM_TYPE_MAX,
} app_driver_param_type_t;

typedef struct {
    void (*update_power)(bool power);
    void (*update_brightness)(uint8_t brightness);
} app_driver_cb_t;

static const char* TAG = "app_driver";
static app_driver_cb_t app_driver_cb[SRC_MAX];

esp_err_t app_driver_init()
{
    return board_init();
}

void app_driver_set_callbacks(app_driver_src_t src, void (*update_power)(bool power), void (*update_brightness)(uint8_t brightness))
{
    app_driver_cb[src].update_power = update_power;
    app_driver_cb[src].update_brightness = update_brightness;
}

void app_driver_update(app_driver_src_t src, app_driver_param_type_t type, void *value)
{
    for (int i = 0; i < SRC_MAX; i++) {
        if (i == src) {
            continue;
        }
        if (type == PARAM_TYPE_POWER) {
            if (app_driver_cb[i].update_power) {
                app_driver_cb[i].update_power(*(bool *)value);
            }
        } else if (type == PARAM_TYPE_BRIGHTNESS) {
            if (app_driver_cb[i].update_brightness) {
                app_driver_cb[i].update_brightness(*(uint8_t *)value);
            }
        }
    }
}

esp_err_t app_driver_update_and_report_power(bool power, app_driver_src_t src)
{
    /* Update */
    led_driver_set_power(power);

    /* Report to other sources */
    app_driver_update(src, PARAM_TYPE_POWER, (void *)&power);

    return ESP_OK;
}

esp_err_t app_driver_update_and_report_brightness(uint8_t brightness, app_driver_src_t src)
{
    /* Update */
    led_driver_set_brightness(brightness);

    /* Report to other sources */
    app_driver_update(src, PARAM_TYPE_BRIGHTNESS, (void *)&brightness);

    return ESP_OK;
}

bool app_driver_get_power()
{
    return led_driver_get_power();
}

uint8_t app_driver_get_brightness()
{
    return led_driver_get_brightness();
}

static int app_driver_cli_handler(int argc, char *argv[])
{
    if (argc != 3) {
        ESP_LOGE(TAG, "Incorrect arguments");
        return 0;
    }
    app_driver_param_type_t param_type = (app_driver_param_type_t)atoi(argv[1]);
    int value = atoi(argv[2]);

    if (param_type == PARAM_TYPE_POWER) {
        app_driver_update_and_report_power(value, SRC_LOCAL);
    } else if (param_type == PARAM_TYPE_BRIGHTNESS) {
        app_driver_update_and_report_brightness(value, SRC_LOCAL);
    } else {
        ESP_LOGE(TAG, "Param type not handled: %d", param_type);
    }
    return 0;
}

static esp_console_cmd_t driver_cmds[] = {
    {
        .command = "driver",
        .help = "This can be used to simulate on-device control. Usage: driver <param_type> <value>",
        .func = app_driver_cli_handler,
    },
};

esp_err_t app_driver_register_cli()
{
    int cmds_num = sizeof(driver_cmds) / sizeof(esp_console_cmd_t);
    int i;
    for (i = 0; i < cmds_num; i++) {
        ESP_LOGI(TAG, "Registering command: %s", driver_cmds[i].command);
        esp_console_cmd_register(&driver_cmds[i]);
    }
    return ESP_OK;
}
