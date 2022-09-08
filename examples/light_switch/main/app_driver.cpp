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

static int color_loop = 0;

static esp_err_t app_driver_bound_console_handler(int argc, char **argv)
{
    if (argc == 1 && strncmp(argv[0], "help", sizeof("help")) == 0) {
        printf("Bound commands:\n"
               "\thelp: Print help\n"
               "\tinvoke: <local_endpoint_id> <cluster_id> <command_id>. "
               "Example: matter esp bound invoke 0x0001 0x0006 0x0002.\n"
               "\tinvoke-group: <local_endpoint_id> <cluster_id> <command_id>. "
               "Example: matter esp bound invoke-group 0x0001 0x0006 0x0002.\n");
    } else if (argc == 4 && strncmp(argv[0], "invoke", sizeof("invoke")) == 0) {
        client::command_handle_t *cmd_handle = (client::command_handle_t *)calloc(1, sizeof(client::command_handle_t));
        uint16_t local_endpoint_id = strtol((const char *)&argv[1][2], NULL, 16);
        cmd_handle->cluster_id = strtol((const char *)&argv[2][2], NULL, 16);
        cmd_handle->command_id = strtol((const char *)&argv[3][2], NULL, 16);
        cmd_handle->is_group = false;

        client::cluster_update(local_endpoint_id, cmd_handle);
    } else if (argc == 4 && strncmp(argv[0], "invoke-group", sizeof("invoke-group")) == 0) {
        client::command_handle_t *cmd_handle = (client::command_handle_t *)calloc(1, sizeof(client::command_handle_t));
        uint16_t local_endpoint_id = strtol((const char *)&argv[1][2], NULL, 16);
        cmd_handle->cluster_id = strtol((const char *)&argv[2][2], NULL, 16);
        cmd_handle->command_id = strtol((const char *)&argv[3][2], NULL, 16);
        cmd_handle->is_group = true;

        client::cluster_update(local_endpoint_id, cmd_handle);
    }
    else {
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

void app_driver_client_group_command_callback(uint8_t fabric_index, uint16_t group_id,
                                              client::command_handle_t *cmd_handle, void *priv_data)
{
    if (cmd_handle->cluster_id == OnOff::Id) {
        if (cmd_handle->command_id == OnOff::Commands::Off::Id) {
            on_off::command::group_send_off(fabric_index, group_id);
        } else if (cmd_handle->command_id == OnOff::Commands::On::Id) {
            on_off::command::group_send_on(fabric_index, group_id);
        } else if (cmd_handle->command_id == OnOff::Commands::Toggle::Id) {
            on_off::command::group_send_toggle(fabric_index, group_id);
        }
    } else if (cmd_handle->cluster_id == ColorControl::Id) {
        if (cmd_handle->command_id == ColorControl::Commands::StepHue::Id) {
            if (color_loop >= 5) {
                color_loop = 0;
            }
            ESP_LOGW(TAG, "color_loop:%d", color_loop);
            color_control::command::group_send_move_to_hue(fabric_index, group_id, (50 * color_loop), 0, 0, 0, 0);

            if (color_loop >= 4) {
                color_loop = 0;
            } else {
                color_loop++;
            }
        }
    } else if (cmd_handle->cluster_id == LevelControl::Id) {
        if (cmd_handle->command_id == LevelControl::Commands::Step::Id) {
            level_control::command::group_send_step(fabric_index, group_id, 0, 40, 0, 0, 0);
        } else if (cmd_handle->command_id == LevelControl::Commands::MoveToLevel::Id) {
            level_control::command::group_send_step(fabric_index, group_id, 1, 40, 0, 0, 0);
        }
    }
}

static void app_driver_button_toggle_cb(void *arg)
{
    ESP_LOGI(TAG, "Toggle button0 pressed");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = OnOff::Id;
    cmd_handle.command_id = OnOff::Commands::Toggle::Id;
    cmd_handle.is_group = true;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(switch_endpoint_id, &cmd_handle);
    lock::chip_stack_unlock();
}

static void app_driver_button1_toggle_cb(void *arg)
{
    ESP_LOGI(TAG, "Toggle button1 pressed");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = ColorControl::Id;
    cmd_handle.command_id = ColorControl::Commands::StepHue::Id;
    cmd_handle.is_group = true;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(switch_endpoint_id, &cmd_handle);
    lock::chip_stack_unlock();
}


static void app_driver_button2_toggle_cb(void *arg)
{
    ESP_LOGI(TAG, "Toggle button2 pressed");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = LevelControl::Id;
    cmd_handle.command_id = LevelControl::Commands::Step::Id;
    cmd_handle.is_group = true;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(switch_endpoint_id, &cmd_handle);
    lock::chip_stack_unlock();
}


static void app_driver_button3_toggle_cb(void *arg)
{
    ESP_LOGI(TAG, "Toggle button3 pressed");
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = LevelControl::Id;
    cmd_handle.command_id = LevelControl::Commands::MoveToLevel::Id;
    cmd_handle.is_group = true;

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(switch_endpoint_id, &cmd_handle);
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
    ESP_LOGI(TAG, "Initialising driver");

    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_OUTPUT;

    gpio_conf.pin_bit_mask = (1ULL << GPIO_NUM_14);
    gpio_config(&gpio_conf);
    gpio_set_level(GPIO_NUM_14, 1); //blue

    gpio_conf.pin_bit_mask = (1ULL << GPIO_NUM_26);
    gpio_config(&gpio_conf);
    gpio_set_level(GPIO_NUM_26, 1); //green

    gpio_conf.pin_bit_mask = (1ULL << GPIO_NUM_25);
    gpio_config(&gpio_conf);
    gpio_set_level(GPIO_NUM_25, 0); //power on

    // gpio_conf.pin_bit_mask = (1ULL << GPIO_NUM_27);
    // gpio_config(&gpio_conf);
    // gpio_set_level(GPIO_NUM_27, 1); //red
    /* Initialize led */
    led_driver_config_t led_config = led_driver_get_config();
    led_driver_init(&led_config);

    led_driver_handle_t handle = NULL;
    led_driver_set_brightness(handle, 255);

    /* Initialize button */
    button_config_t button_config0 = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = GPIO_NUM_34,
            .active_level = 1,
        }
    };
    button_config_t button_config1 = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = GPIO_NUM_39,
            .active_level = 1,
        }
    };
    button_config_t button_config2 = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = GPIO_NUM_32,
            .active_level = 1,
        }
    };
    button_config_t button_config3 = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = GPIO_NUM_35,
            .active_level = 1,
        }
    };

    button_handle_t handle0 = iot_button_create(&button_config0);
    button_handle_t handle1 = iot_button_create(&button_config1);
    button_handle_t handle2 = iot_button_create(&button_config2);
    button_handle_t handle3 = iot_button_create(&button_config3);

    iot_button_register_cb(handle0, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb);

    iot_button_register_cb(handle1, BUTTON_PRESS_DOWN, app_driver_button1_toggle_cb);
    //app_reset_button_register(handle1);

    iot_button_register_cb(handle2, BUTTON_PRESS_DOWN, app_driver_button2_toggle_cb);
    //app_reset_button_register(handle2);

    iot_button_register_cb(handle3, BUTTON_PRESS_DOWN, app_driver_button3_toggle_cb);
    //app_reset_button_register(handle3);

    /* Other initializations */
    app_driver_register_commands();
    client::set_command_callback(app_driver_client_command_callback, app_driver_client_group_command_callback, NULL);

    //led_driver_set_brightness(handle, 200);//red

    return (app_driver_handle_t)handle0;
}
