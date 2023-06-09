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
#include <led_driver.h>

#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;

static void app_driver_button_on_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Control button up");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = OnOff::Id;
    cmd_handle.command_id = OnOff::Commands::On::Id;
    cmd_handle.is_group = false;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(light_endpoint_id, &cmd_handle);
    lock::chip_stack_unlock();
}

static void app_driver_button_off_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Control button pressed");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = OnOff::Id;
    cmd_handle.command_id = OnOff::Commands::Off::Id;
    cmd_handle.is_group = false;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(light_endpoint_id, &cmd_handle);
    lock::chip_stack_unlock();
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    return err;
}

esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    return err;
}

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

app_driver_handle_t app_driver_light_init()
{
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_off_cb, NULL);
    iot_button_register_cb(handle, BUTTON_PRESS_UP, app_driver_button_on_cb, NULL);

    client::set_command_callback(app_driver_client_command_callback, app_driver_client_group_command_callback, NULL);

    return (app_driver_handle_t)handle;
}
