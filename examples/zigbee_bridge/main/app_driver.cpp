/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_matter.h>
#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

static const char *TAG = "app_driver";

static void app_driver_client_command_callback(client::peer_device_t *peer_device, client::command_handle_t *cmd_handle,
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
    }
}

void send_onoff_command_to_binding_table(uint16_t local_endpoint, bool on_off)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = OnOff::Id;
    cmd_handle.is_group = false;
    cmd_handle.command_id = on_off ? (OnOff::Commands::On::Id) : (OnOff::Commands::Off::Id);

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(local_endpoint, &cmd_handle);
    lock::chip_stack_unlock();
}

void app_driver_switch_init()
{
    client::set_command_callback(app_driver_client_command_callback, NULL, NULL);
}
