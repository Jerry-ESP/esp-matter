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

#include <app_reset.h>
#include <app_priv.h>

using chip::kInvalidClusterId;
static constexpr chip::CommandId kInvalidCommandId = 0xFFFF'FFFF;

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

static const char *TAG = "app_driver";
extern uint16_t switch_endpoint_id;

static esp_err_t app_driver_bound_console_handler(int argc, char **argv)
{
    if (argc == 1 && strncmp(argv[0], "help", sizeof("help")) == 0) {
        printf("Bound commands:\n"
               "\thelp: Print help\n"
               "\tinvoke: <local_endpoint_id> <cluster_id> <command_id>. "
               "Example: matter esp bound invoke 0x0001 0x0006 0x0002.\n");
    } else if (argc == 4 && strncmp(argv[0], "invoke", sizeof("invoke")) == 0) {
        client::command_handle_t *cmd_handle = (client::command_handle_t *)calloc(1, sizeof(client::command_handle_t));
        uint16_t local_endpoint_id = strtol((const char *)&argv[1][2], NULL, 16);
        cmd_handle->cluster_id = strtol((const char *)&argv[2][2], NULL, 16);
        cmd_handle->command_id = strtol((const char *)&argv[3][2], NULL, 16);
        cmd_handle->is_group = false;

        client::cluster_update(local_endpoint_id, cmd_handle);
    } else {
        ESP_LOGE(TAG, "Incorrect arguments. Check help for more details.");
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

static esp_err_t app_driver_client_console_handler(int argc, char **argv)
{
    if (argc == 1 && strncmp(argv[0], "help", sizeof("help")) == 0) {
        printf("Client commands:\n"
               "\thelp: Print help\n"
               "\tinvoke: <fabric_index> <remote_node_id> <remote_endpoint_id> <cluster_id> <command_id>. "
               "Example: matter esp client invoke 0x0001 0xBC5C01 0x0001 0x0006 0x0002.\n");
    } else if (argc == 6 && strncmp(argv[0], "invoke", sizeof("invoke")) == 0) {
        client::command_handle_t *cmd_handle = (client::command_handle_t *)calloc(1, sizeof(client::command_handle_t));
        uint8_t fabric_index = strtol((const char *)&argv[1][2], NULL, 16);
        uint64_t node_id = strtol((const char *)&argv[2][2], NULL, 16);
        cmd_handle->endpoint_id = strtol((const char *)&argv[3][2], NULL, 16);
        cmd_handle->cluster_id = strtol((const char *)&argv[4][2], NULL, 16);
        cmd_handle->command_id = strtol((const char *)&argv[5][2], NULL, 16);

        client::connect(fabric_index, node_id, cmd_handle);
    } else {
        ESP_LOGE(TAG, "Incorrect arguments. Check help for more details.");
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

static void app_driver_register_commands()
{
    /* Add console command for bound devices */
    esp_matter_console_command_t bound_command = {
        .name = "bound",
        .description = "This can be used to simulate on-device control for bound devices."
                       "Usage: matter esp bound <bound_command>. "
                       "Bound commands: help, invoke",
        .handler = app_driver_bound_console_handler,
    };
    esp_matter_console_add_command(&bound_command);

    /* Add console command for client to control non-bound devices */
    esp_matter_console_command_t client_command = {
        .name = "client",
        .description = "This can be used to simulate on-device control for client devices."
                       "Usage: matter esp client <client_command>. "
                       "Client commands: help, invoke",
        .handler = app_driver_client_console_handler,
    };
    esp_matter_console_add_command(&client_command);
}

void app_driver_client_command_callback(client::peer_device_t *peer_device, uint16_t remote_endpoint_id,
                                        client::command_handle_t *cmd_handle, void *priv_data)
{
    if (cmd_handle->cluster_id == OnOff::Id) {
        if (cmd_handle->command_id == OnOff::Commands::Off::Id) {
            on_off::command::send_off(peer_device, remote_endpoint_id);
        } else if (cmd_handle->command_id == OnOff::Commands::On::Id) {
            on_off::command::send_on(peer_device, remote_endpoint_id);
        } else if (cmd_handle->command_id == OnOff::Commands::Toggle::Id) {
            on_off::command::send_toggle(peer_device, remote_endpoint_id);
        }
    }
}

static void app_driver_button_toggle_cb(void *arg)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    client::command_handle_t *cmd_handle = (client::command_handle_t *)calloc(1, sizeof(client::command_handle_t *));
    cmd_handle->cluster_id = OnOff::Id;
    cmd_handle->command_id = OnOff::Commands::Toggle::Id;
    cmd_handle->is_group = false;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(switch_endpoint_id, cmd_handle);
    lock::chip_stack_unlock();
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    /* Nothing to do here */
    return ESP_OK;
}

app_driver_handle_t app_driver_switch_init()
{
    /* Initialize button */
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb);

    /* Other initializations */
    app_driver_register_commands();
    client::set_command_callback(app_driver_client_command_callback, NULL, NULL);

    return (app_driver_handle_t)handle;
}
