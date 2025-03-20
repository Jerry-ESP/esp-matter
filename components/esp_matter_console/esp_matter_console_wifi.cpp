// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
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

#include <esp_check.h>
#include <esp_log.h>
#include <esp_matter_console.h>
#if CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION
#include <esp_wifi.h>
#endif // CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION
#include <string.h>
#include <nvs_flash.h>

namespace esp_matter {
namespace console {
#if CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION
static const char *TAG = "esp_matter_console_wifi";
static engine wifi_console;
static engine manager_console;

static esp_err_t wifi_connect_handler(int argc, char *argv[])
{
    ESP_RETURN_ON_FALSE(argc == 2, ESP_ERR_INVALID_ARG, TAG, "Incorrect arguments");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "Failed to initialize WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_stop(), TAG, "Failed to stop WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Failed to set WiFi mode");
    wifi_config_t wifi_cfg = {0};
    snprintf((char *)wifi_cfg.sta.ssid, sizeof(wifi_cfg.sta.ssid), "%s", argv[0]);
    snprintf((char *)wifi_cfg.sta.password, sizeof(wifi_cfg.sta.password), "%s", argv[1]);
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg), TAG, "Failed to set WiFi configuration");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG, "Failed to connect WiFi");
    return ESP_OK;
}

static esp_err_t manager_set_ip_handler(int argc, char *argv[])
{
    ESP_RETURN_ON_FALSE(argc == 1, ESP_ERR_INVALID_ARG, TAG, "Incorrect arguments");
    char new_manager_ip[16];  // Adjust size as needed
    strncpy(new_manager_ip, argv[0], sizeof(new_manager_ip) - 1);
    new_manager_ip[sizeof(new_manager_ip) - 1] = '\0';  // Null-terminate

    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open("manager", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS: %d", err);
        return err;
    }
    err = nvs_set_blob(handle, "manager_ip",new_manager_ip, sizeof(new_manager_ip));

    ESP_LOGI(TAG,"\nwriting Manager-IP: \n%s\n",new_manager_ip);

    nvs_commit(handle);

    if(err==ESP_OK)
        ESP_LOGI(TAG,"\nnew_manager_ip write success.\n");
    else
        ESP_LOGE(TAG,"\nnew_manager_ip write failure. %d\n",err);

    nvs_close(handle);

    return ESP_OK;

}

static esp_err_t manager_dispatch(int argc, char *argv[])
{
    if (argc <= 0) {
        manager_console.for_each_command(print_description, NULL);
        return ESP_OK;
    }
    return manager_console.exec_command(argc, argv);
}

static esp_err_t wifi_dispatch(int argc, char *argv[])
{
    if (argc <= 0) {
        wifi_console.for_each_command(print_description, NULL);
        return ESP_OK;
    }
    return wifi_console.exec_command(argc, argv);
}
#endif // CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION

esp_err_t wifi_register_commands()
{
#if CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION
    static const command_t command = {
        .name = "wifi",
        .description = "Wi-Fi commands. Usage: matter esp wifi <wifi_command>.",
        .handler = wifi_dispatch,
    };

    static const command_t wifi_commands[] = {
        {
            .name = "connect",
            .description = "Connect to AP. Usage: matter esp wifi connect ssid psk.",
            .handler = wifi_connect_handler,
        },
    };
    wifi_console.register_commands(wifi_commands, sizeof(wifi_commands) / sizeof(command_t));

    return add_commands(&command, 1);
#else
    return ESP_OK;
#endif // CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION
}

esp_err_t manager_register_commands()
{
#if CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION
    static const command_t command = {
        .name = "manager",
        .description = "Manager commands. Usage: matter esp manager <manager_command>.",
        .handler = manager_dispatch,
    };

    static const command_t manager_commands[] = {
        {
            .name = "set-ip",
            .description = "Connect to Manager. Usage: matter esp manager set-ip <ip-of-manager-machine>.",
            .handler = manager_set_ip_handler,
        },
    };
    manager_console.register_commands(manager_commands, sizeof(manager_commands) / sizeof(command_t));

    return add_commands(&command, 1);
#else
    return ESP_OK;
#endif // CONFIG_ENABLE_WIFI_AP || CONFIG_ENABLE_WIFI_STATION
}

} // namespace console
} // namespace esp_matter
