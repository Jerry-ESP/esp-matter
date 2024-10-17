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

#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#ifdef CONFIG_IDF_TARGET_ESP32H2
#include <esp32h2/rom/rtc.h>
#endif
#include <nvs_flash.h>

#include <esp_matter.h>
#include <factory_reset.h>

#define NVS_PART_NAME "nvs"
#define NVS_NAMESPACE "color_ligth"
#define REBOOT_COUNT_NVS_KEY "reboot_count"
#define FACTORY_RESET_COUNT 3

static const char *TAG = "factory_reset";
static bool immediately_trigger = false;
static bool perform_factory_reset = false;

static uint8_t reboot_count_get()
{
    uint8_t value = 0;
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open_from_partition(NVS_PART_NAME, NVS_NAMESPACE, NVS_READWRITE,
                                            &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS: %d", err);
        return value;
    }
    err = nvs_get_u8(handle, REBOOT_COUNT_NVS_KEY, &value);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error getting from NVS: %d", err);
    }
    nvs_commit(handle);
    nvs_close(handle);
    return value;
}

static esp_err_t reboot_count_set(uint8_t value)
{
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open_from_partition(NVS_PART_NAME, NVS_NAMESPACE, NVS_READWRITE,
                                            &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS: %d", err);
        return err;
    }
    err = nvs_set_u8(handle, REBOOT_COUNT_NVS_KEY, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting in NVS: %d", err);
    }
    nvs_commit(handle);
    nvs_close(handle);
    return err;
}

static void factory_reset_reset_reboot_count(TimerHandle_t timer)
{
    if (perform_factory_reset) {
        perform_factory_reset = false;
        esp_matter::factory_reset();
    } else {
        printf("%s: Resetting reboot count\n", TAG);
        reboot_count_set(0);
    }
    xTimerDelete(timer, 0);
}

static uint8_t factory_reset_get_reboot_count()
{
    RESET_REASON reset_reason = rtc_get_reset_reason(0);
    uint8_t reboot_count = reboot_count_get();

    if (reset_reason == POWERON_RESET || reset_reason == RTCWDT_RTC_RESET) {
        reboot_count++;
    } else {
        reboot_count = 1;
    }

    printf("%s: Reboot count: %d, reset reason: %d\n", TAG, reboot_count, reset_reason);
    reboot_count_set(reboot_count);

    TimerHandle_t timer = xTimerCreate("factory_reset", pdMS_TO_TICKS(5 * 1000), pdFALSE, NULL,
                                       &factory_reset_reset_reboot_count);
    if (!timer) {
        ESP_LOGE(TAG, "Could not initialize timer");
        return 0;
    }
    xTimerStart(timer, 0);
    return reboot_count;
}

esp_err_t factory_reset_init()
{
    if (factory_reset_get_reboot_count() == FACTORY_RESET_COUNT) {
        if (immediately_trigger) {
            ESP_LOGI(TAG, "Factory reset immediately triggered.");
            perform_factory_reset = false;
            esp_matter::factory_reset();
        } else {
            ESP_LOGI(TAG, "Factory reset triggered. It will start after the timer has expired.");
            perform_factory_reset = true;
        }
    }

    return ESP_OK;
}
