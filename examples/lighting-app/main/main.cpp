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

#include "device_callbacks.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "led_manager.hpp"
#include "lighting_app_constants.hpp"
#include "nvs_flash.h"
#include "app/util/af-enums.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Matter includes
#include "app/common/gen/att-storage.h"
#include "app/common/gen/attribute-id.h"
#include "app/common/gen/attribute-type.h"
#include "app/common/gen/cluster-id.h"
#include "app/server/Server.h"
#include "app/util/af-types.h"
#include "app/util/af.h"
#include "core/CHIPError.h"
#include "lib/shell/Engine.h"
#include "lib/support/CHIPMem.h"
#include "platform/CHIPDeviceLayer.h"

using chip::DeviceLayer::ConnectivityMgr;
using chip::DeviceLayer::PlatformMgr;

static esp_err_t init_chip_stack()
{
    if (PlatformMgr().InitChipStack() != CHIP_NO_ERROR) {
        ESP_LOGE(APP_LOG_TAG, "Failed to initialize CHIP stack");
        return ESP_FAIL;
    }
    ConnectivityMgr().SetBLEAdvertisingEnabled(true);
    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR) {
        ESP_LOGE(APP_LOG_TAG, "Failed to initialize CHIP memory pool");
        return ESP_ERR_NO_MEM;
    }
    if (PlatformMgr().StartEventLoopTask() != CHIP_NO_ERROR) {
        chip::Platform::MemoryShutdown();
        ESP_LOGE(APP_LOG_TAG, "Failed to launch Matter main task");
        return ESP_FAIL;
    }
    PlatformMgr().AddEventHandler(on_device_event, static_cast<intptr_t>(NULL));

    return ESP_OK;
}

#if CONFIG_ENABLE_CHIP_SHELL
void ChipShellTask(void *args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}
#endif // CONFIG_ENABLE_CHIP_SHELL

extern "C" void app_main()
{
    // UINT8_MAX is reserved for undefined brightness
    uint8_t default_brightness_level = UINT8_MAX;
    // Initialize the ESP NVS layer.
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(led_manager_init(default_brightness_level));

    ESP_LOGI(APP_LOG_TAG, "==================================================");
    ESP_LOGI(APP_LOG_TAG, "chip-esp32-lighting-example starting");
    ESP_LOGI(APP_LOG_TAG, "==================================================");

    ESP_ERROR_CHECK(init_chip_stack());
    InitServer();
    update_current_brightness(default_brightness_level);
#if CONFIG_ENABLE_CHIP_SHELL
    xTaskCreate(&ChipShellTask, "chip_shell", 2048, NULL, 5, NULL);
#endif
}
