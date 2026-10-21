/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <esp_err.h>
#include <esp_matter.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

typedef void *app_driver_handle_t;

/** Button used for factory reset. The stub returns NULL. */
app_driver_handle_t app_driver_button_init();

/** Attribute updates that still go through the esp-matter store, such as On/Off. */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);

/** Empty actuator hooks. A real driver replaces these. */
esp_err_t app_driver_set_on_off(bool on);
esp_err_t app_driver_set_mode(uint8_t mode);
esp_err_t app_driver_set_system_state(uint8_t system_state);
esp_err_t app_driver_set_user_setpoint(uint8_t setpoint);
esp_err_t app_driver_set_target_setpoint(uint8_t setpoint);
esp_err_t app_driver_set_mist_type(uint8_t mist_type);
esp_err_t app_driver_set_optimal(bool optimal);

/** Empty sensor hook. Writes a null humidity when no sensor is connected. */
esp_err_t app_driver_get_relative_humidity(nullable<uint16_t> *humidity);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif
