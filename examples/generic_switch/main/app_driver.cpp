/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <esp_matter.h>
#include <app-common/zap-generated/attributes/Accessors.h>

#include <app_priv.h>
#include <iot_button.h>

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
#define BUTTON_GPIO_PIN GPIO_NUM_0
#else // CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32C2
#define BUTTON_GPIO_PIN GPIO_NUM_9
#endif

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

static const char *TAG = "app_driver";
extern uint16_t generic_switch_endpoint_id;

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    return err;
}

static void send_command_success_callback(void *context, const ConcreteCommandPath &command_path,
                                          const chip::app::StatusIB &status, TLVReader *response_data)
{
    ESP_LOGI(TAG, "Send command success");
}

static void send_command_failure_callback(void *context, CHIP_ERROR error)
{
    ESP_LOGI(TAG, "Send command failure: err :%" CHIP_ERROR_FORMAT, error.Format());
}

void app_driver_client_invoke_command_callback(client::peer_device_t *peer_device, client::request_handle_t *req_handle,
                                               void *priv_data)
{
    if (req_handle->type != esp_matter::client::INVOKE_CMD) {
        return;
    }
    char command_data_str[32];
    // on_off light switch should support on_off cluster and identify cluster commands sending.
    if (req_handle->command_path.mClusterId == OnOff::Id) {
        strcpy(command_data_str, "{}");
    } else if (req_handle->command_path.mClusterId == Identify::Id) {
        if (req_handle->command_path.mCommandId == Identify::Commands::Identify::Id) {
            if (((char *)req_handle->request_data)[0] != 1) {
                ESP_LOGE(TAG, "Number of parameters error");
                return;
            }
            sprintf(command_data_str, "{\"0:U16\": %ld}",
                    strtoul((const char *)(req_handle->request_data) + 1, NULL, 16));
        } else {
            ESP_LOGE(TAG, "Unsupported command");
            return;
        }
    } else {
        ESP_LOGE(TAG, "Unsupported cluster");
        return;
    }
    client::interaction::invoke::send_request(NULL, peer_device, req_handle->command_path, command_data_str,
                                              send_command_success_callback, send_command_failure_callback,
                                              chip::NullOptional);
}

#if CONFIG_GENERIC_SWITCH_TYPE_LATCHING
static uint8_t newPosition = 0;
static void app_driver_button_switch_latched(void *arg, void *data)
{
    ESP_LOGI(TAG, "Switch lached pressed");
    gpio_button * button = (gpio_button*)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
<<<<<<< HEAD
    // Press moves Position from 0 (idle) to 1 (press) and vice versa
    uint8_t newPosition = (latching_switch_previous_position == 1) ? 0 : 1;
    latching_switch_previous_position = newPosition;
        chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
        // SwitchLatched event takes newPosition as event data
    });

    client::request_handle_t req_handle;
    req_handle.type = esp_matter::client::INVOKE_CMD;
    req_handle.command_path.mClusterId = OnOff::Id;
    if(newPosition == 1)
    {
        req_handle.command_path.mCommandId = OnOff::Commands::On::Id;
    }
    else if (newPosition == 0)
    {
        req_handle.command_path.mCommandId = OnOff::Commands::Off::Id;
    }

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(generic_switch_endpoint_id, &req_handle);
    lock::chip_stack_unlock();
}
#endif
#if CONFIG_GENERIC_SWITCH_TYPE_MOMENTARY
static int current_number_of_presses_counted = 1;
static bool is_multipress = 0;
static uint8_t idlePosition    = 0;

static void app_driver_button_initial_pressed(void *arg, void *data)
{
    if(!is_multipress) {
        ESP_LOGI(TAG, "Initial button pressed");
        gpio_button * button = (gpio_button*)data;
        int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
        // Press moves Position from 0 (idle) to 1 (press)
        uint8_t newPosition     = 1;
        chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, newPosition]() {
            chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
            // InitialPress event takes newPosition as event data
            switch_cluster::event::send_initial_press(switch_endpoint_id, newPosition);
        });
        is_multipress = 1;
    }
}

static void app_driver_button_release(void *arg, void *data)
{

    gpio_button *button = (gpio_button *)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
    chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id]() {
        chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, idlePosition);
    });
}

static void app_driver_button_long_pressed(void *arg, void *data)
{
    ESP_LOGI(TAG, "Long button pressed ");
    gpio_button *button = (gpio_button *)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
    // Press moves Position from 0 (idle) to 1 (press)
    uint8_t newPosition = 1;
    chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, newPosition]() {
        chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
        // LongPress event takes newPosition as event data
        switch_cluster::event::send_long_press(switch_endpoint_id, newPosition);
    });
}

static void app_driver_button_multipress_ongoing(void *arg, void *data)
{
    ESP_LOGI(TAG, "Multipress Ongoing");
    gpio_button * button = (gpio_button *)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
    // Press moves Position from 0 (idle) to 1 (press)
    uint8_t newPosition = 1;
    current_number_of_presses_counted++;
    uint16_t endpoint_id = switch_endpoint_id;
    uint32_t cluster_id = Switch::Id;
    uint32_t attribute_id = Switch::Attributes::FeatureMap::Id;

    attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);

    uint32_t feature_map = val.val.u32;
    uint32_t msm_feature_map = switch_cluster::feature::momentary_switch_multi_press::get_id();
    uint32_t as_feature_map = switch_cluster::feature::action_switch::get_id();
    if(((feature_map & msm_feature_map) == msm_feature_map) && ((feature_map & as_feature_map) != as_feature_map)) {
        chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, newPosition]() {
            chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
            // MultiPress Ongoing event takes newPosition and current_number_of_presses_counted as event data
            switch_cluster::event::send_multi_press_ongoing(switch_endpoint_id, newPosition, current_number_of_presses_counted);
        });
    }
}

static void app_driver_button_multipress_complete(void *arg, void *data)
{
    ESP_LOGI(TAG, "Multipress Complete");
    gpio_button * button = (gpio_button *)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
    // Press moves Position from 0 (idle) to 1 (press)
    uint8_t previousPosition = 1;
    uint16_t endpoint_id = switch_endpoint_id;
    uint32_t cluster_id = Switch::Id;
    uint32_t attribute_id = Switch::Attributes::MultiPressMax::Id;

    attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    uint8_t multipress_max = val.val.u8;
    int total_number_of_presses_counted = (current_number_of_presses_counted > multipress_max)? 0:current_number_of_presses_counted;
    chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, previousPosition, total_number_of_presses_counted]() {
        chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, idlePosition);
        // MultiPress Complete event takes previousPosition and total_number_of_presses_counted as event data
        switch_cluster::event::send_multi_press_complete(switch_endpoint_id, previousPosition, total_number_of_presses_counted);
        // Reset current_number_of_presses_counted to initial value
        current_number_of_presses_counted = 1;
    });
    is_multipress = 0;
}
#endif

app_driver_handle_t app_driver_button_init(gpio_button * button)
{
    button_config_t config = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = BUTTON_GPIO_PIN,
            .active_level = 0,
        }
    };

    if (button != NULL) {
        config.type =  button_type_t::BUTTON_TYPE_GPIO;
        config.gpio_button_config.gpio_num = button->GPIO_PIN_VALUE;
    }
    button_handle_t handle = iot_button_create(&config);


#if CONFIG_GENERIC_SWITCH_TYPE_LATCHING
<<<<<<< HEAD
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_switch_latched, button);
=======
    iot_button_register_cb(handle, BUTTON_DOUBLE_CLICK, app_driver_button_switch_latched, button);
    client::set_request_callback(app_driver_client_invoke_command_callback, NULL, NULL);
>>>>>>> e58e0259 (Added binding cluster to generic latching switch)
#endif

#if CONFIG_GENERIC_SWITCH_TYPE_MOMENTARY
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_initial_pressed, button);
    iot_button_register_cb(handle, BUTTON_PRESS_UP, app_driver_button_release, button);
    iot_button_register_cb(handle, BUTTON_PRESS_REPEAT, app_driver_button_multipress_ongoing, button);
    iot_button_register_cb(handle, BUTTON_PRESS_REPEAT_DONE, app_driver_button_multipress_complete, button);
#endif
    return (app_driver_handle_t)handle;
}
