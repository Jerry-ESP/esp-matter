/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <app_priv.h>

#include <clusters/OnOff/AttributeIds.h>
#include <clusters/OnOff/ClusterId.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

app_driver_handle_t app_driver_button_init()
{
    return NULL;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    (void)driver_handle;
    (void)endpoint_id;
    if (val == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    if (cluster_id == OnOff::Id && attribute_id == OnOff::Attributes::OnOff::Id) {
        return app_driver_set_on_off(val->val.b);
    }
    return ESP_OK;
}

esp_err_t app_driver_set_on_off(bool on)
{
    (void)on;
    return ESP_OK;
}

esp_err_t app_driver_set_mode(uint8_t mode)
{
    (void)mode;
    return ESP_OK;
}

esp_err_t app_driver_set_system_state(uint8_t system_state)
{
    (void)system_state;
    return ESP_OK;
}

esp_err_t app_driver_set_user_setpoint(uint8_t setpoint)
{
    (void)setpoint;
    return ESP_OK;
}

esp_err_t app_driver_set_target_setpoint(uint8_t setpoint)
{
    (void)setpoint;
    return ESP_OK;
}

esp_err_t app_driver_set_mist_type(uint8_t mist_type)
{
    (void)mist_type;
    return ESP_OK;
}

esp_err_t app_driver_set_optimal(bool optimal)
{
    (void)optimal;
    return ESP_OK;
}

esp_err_t app_driver_get_relative_humidity(nullable<uint16_t> *humidity)
{
    if (humidity == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    *humidity = nullable<uint16_t>();
    return ESP_OK;
}
