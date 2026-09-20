/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>

#include <app/clusters/humidistat-server/HumidistatCluster.h>
#include <lib/support/logging/CHIPLogging.h>
#include <clusters/Humidistat/Enums.h>
#include <device.h>
#include <esp_matter.h>
#include <app/clusters/humidistat-server/CodegenIntegration.h>

#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Humidistat;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t humidifier_endpoint_id;

static void update_system_state(uint16_t endpoint_id, ModeEnum mode)
{
    HumidistatCluster *cluster = FindClusterOnEndpoint(endpoint_id);
    if (cluster == nullptr) {
        ESP_LOGW(TAG, "Humidistat cluster not found on endpoint %u", endpoint_id);
        return;
    }

    SystemStateEnum system_state = SystemStateEnum::kIdle;
    switch (mode) {
    case ModeEnum::kHumidifier:
        system_state = SystemStateEnum::kHumidifying;
        break;
    case ModeEnum::kDehumidifier:
        system_state = SystemStateEnum::kDehumidifying;
        break;
    case ModeEnum::kFanOnly:
        system_state = SystemStateEnum::kFan;
        break;
    case ModeEnum::kAuto:
    default:
        system_state = SystemStateEnum::kIdle;
        break;
    }

    CHIP_ERROR err = cluster->SetSystemState(system_state);
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "Failed to update system state: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void app_driver_on_mode_changed(uint16_t endpoint_id, ModeEnum mode)
{
    ESP_LOGI(TAG, "Mode changed to %u", static_cast<uint8_t>(mode));
    update_system_state(endpoint_id, mode);
}

void app_driver_on_user_setpoint_changed(uint16_t endpoint_id, chip::Percent setpoint)
{
    ESP_LOGI(TAG, "User setpoint changed to %u", static_cast<uint8_t>(setpoint));
    (void) endpoint_id;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    (void) driver_handle;

    if (endpoint_id != humidifier_endpoint_id || cluster_id != Humidistat::Id) {
        return ESP_OK;
    }

    if (cluster_id == Humidistat::Id && attribute_id == Attributes::Mode::Id) {
        app_driver_on_mode_changed(endpoint_id, static_cast<ModeEnum>(val->val.u8));
    }

    return ESP_OK;
}

app_driver_handle_t app_driver_button_init()
{
    button_handle_t handle = NULL;
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t btn_gpio_cfg = button_driver_get_config();

    if (iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button device");
        return NULL;
    }

    return (app_driver_handle_t) handle;
}
