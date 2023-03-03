/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <esp_matter.h>
#include <esp_matter_console.h>
#include <led_driver.h>

#include <app_priv.h>
#include <app_reset.h>

using chip::kInvalidClusterId;
static constexpr chip::CommandId kInvalidCommandId = 0xFFFF'FFFF;

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

static const char *TAG = "app_driver";
extern uint16_t switch_endpoint_id;

void app_driver_client_command_callback(client::peer_device_t *peer_device, client::command_handle_t *cmd_handle,
                                        void *priv_data)
{
    // on_off light switch should support on_off cluster and identify cluster commands sending.
    if (cmd_handle->cluster_id == OnOff::Id) {
        switch (cmd_handle->command_id) {
        case OnOff::Commands::Off::Id: {
            on_off::command::send_off(peer_device, cmd_handle->endpoint_id);
            break;
        };
        case OnOff::Commands::On::Id: {
            on_off::command::send_on(peer_device, cmd_handle->endpoint_id);
            break;
        };
        case OnOff::Commands::Toggle::Id: {
            on_off::command::send_toggle(peer_device, cmd_handle->endpoint_id);
            break;
        };
        default:
            break;
        }
    } else if (cmd_handle->cluster_id == Identify::Id) {
        if (cmd_handle->command_id == Identify::Commands::Identify::Id) {
            if (((char *)cmd_handle->command_data)[0] != 1) {
                ESP_LOGE(TAG, "Number of parameters error");
                return;
            }
            identify::command::send_identify(peer_device, cmd_handle->endpoint_id,
                                             strtol((const char *)(cmd_handle->command_data) + 1, NULL, 16));
        } else {
            ESP_LOGE(TAG, "Unsupported command");
        }
    } else {
        ESP_LOGE(TAG, "Unsupported cluster");
    }
}

void app_driver_client_group_command_callback(uint8_t fabric_index, client::command_handle_t *cmd_handle,
                                              void *priv_data)
{
    // on_off light switch should support on_off cluster and identify cluster commands sending.
    if (cmd_handle->cluster_id == OnOff::Id) {
        switch (cmd_handle->command_id) {
        case OnOff::Commands::Off::Id: {
            on_off::command::group_send_off(fabric_index, cmd_handle->group_id);
            break;
        }
        case OnOff::Commands::On::Id: {
            on_off::command::group_send_on(fabric_index, cmd_handle->group_id);
            break;
        }
        case OnOff::Commands::Toggle::Id: {
            on_off::command::group_send_toggle(fabric_index, cmd_handle->group_id);
            break;
        }
        default:
            break;
        }
    } else if (cmd_handle->cluster_id == Identify::Id) {
        if (cmd_handle->command_id == Identify::Commands::Identify::Id) {
            if (((char *)cmd_handle->command_data)[0] != 1) {
                ESP_LOGE(TAG, "Number of parameters error");
                return;
            }
            identify::command::group_send_identify(fabric_index, cmd_handle->group_id,
                                                   strtol((const char *)(cmd_handle->command_data) + 1, NULL, 16));
        } else {
            ESP_LOGE(TAG, "Unsupported command");
        }
    } else {
        ESP_LOGE(TAG, "Unsupported cluster");
    }
}

#define OCCUPANCY_SENSOR_GPIO GPIO_NUM_5
volatile static bool ledOn = false;

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = OnOff::Id;
    cmd_handle.command_id = OnOff::Commands::Toggle::Id;
    cmd_handle.is_group = false;
    client::cluster_update(switch_endpoint_id, &cmd_handle);
}

static void ClearOccupancyDetected(chip::System::Layer *layer, void * context)
{
    ESP_LOGI(TAG, "Clear occupancy detected");
    // AttributeUpdateOccupancy(false);
    app_driver_button_toggle_cb(NULL, NULL);
    ledOn = false;
}

static void HandleOccupancyDetected(intptr_t arg)
{
    ESP_LOGI(TAG, "Occupancy detected");
    // AttributeUpdateOccupancy(true);
    app_driver_button_toggle_cb(NULL, NULL);
    ledOn = true;
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(5), ClearOccupancyDetected, nullptr);
}

static void IRAM_ATTR OccupancyDetectedCallback(void *arg)
{
    if (!ledOn)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(HandleOccupancyDetected, reinterpret_cast<intptr_t>(nullptr));
        ledOn = true;
    }
}

static void OccupancySensorInit(void)
{
    gpio_reset_pin(OCCUPANCY_SENSOR_GPIO);
    gpio_set_intr_type(OCCUPANCY_SENSOR_GPIO, GPIO_INTR_POSEDGE);
    gpio_set_direction(OCCUPANCY_SENSOR_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(OCCUPANCY_SENSOR_GPIO, GPIO_PULLDOWN_ONLY);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(OCCUPANCY_SENSOR_GPIO, OccupancyDetectedCallback, NULL);
}

void app_driver_switch_init()
{
    /* Initialize occupancy sensor */
    OccupancySensorInit();

    client::set_command_callback(app_driver_client_command_callback, app_driver_client_group_command_callback, NULL);
}
