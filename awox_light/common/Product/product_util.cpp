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
#include "RebootCounterManager.hpp"
#include "StorageManager.hpp"
#include "FlashDriver.hpp"
#include <product_util.h>

static const char *TAG = "factory_reset";

esp_err_t factory_reset_init()
{
    ESP_LOGI(TAG, "FactoryReset Init");

    static auto storageManager = Storage::Manager();
    storageManager.readFlash();
    TaskHandle_t storageManagerHandle;
    storageManager.createManagerTask(&storageManagerHandle);

    static auto rebootCounterManager = RebootCounterManager::RebootCounterManager(storageManager.getRebootCounter());
    TaskHandle_t rebootManagerHandle;
    rebootCounterManager.createManagerTask(&rebootManagerHandle);

    return ESP_OK;
}

void product_factory_reset()
{
    Flash::Driver().erase();
    esp_matter::factory_reset();
}

void product_print_mem()
{
    printf("%s: Current Free Memory: %d, Minimum Ever Free Size: %d, Largest Free Block: %d\n", "Memory",
           heap_caps_get_free_size(MALLOC_CAP_8BIT) - heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
           heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
           heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
}
