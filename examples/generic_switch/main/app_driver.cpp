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
#include "bsp/esp-bsp.h"
#include "led_indicator_blink_default.h"
#include <esp_ota_ops.h>

#include <app_priv.h>
#include <iot_button.h>
#include <app_reset.h>

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
#define BUTTON_GPIO_PIN GPIO_NUM_0
#else // CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32C2
#define BUTTON_GPIO_PIN GPIO_NUM_9
#endif

static led_indicator_handle_t leds[CONFIG_BSP_LEDS_NUM];

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

static const char *TAG = "app_driver";

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    return err;
}

#if CONFIG_GENERIC_SWITCH_TYPE_LATCHING
static uint8_t latching_switch_previous_position = 0;
static void app_driver_button_switch_latched(void *arg, void *data)
{
    ESP_LOGI(TAG, "Switch lached pressed");
    gpio_button * button = (gpio_button*)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
    // Press moves Position from 0 (idle) to 1 (press) and vice versa
    uint8_t newPosition = (latching_switch_previous_position == 1) ? 0 : 1;
    latching_switch_previous_position = newPosition;
    chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, newPosition]() {
        chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
        // SwitchLatched event takes newPosition as event data
        switch_cluster::event::send_switch_latched(switch_endpoint_id, newPosition);
    });
}
#endif
#if CONFIG_GENERIC_SWITCH_TYPE_MOMENTARY
static int current_number_of_presses_counted = 1;
// static bool is_multipress = 0;
static uint8_t idlePosition    = 0;
static uint8_t is_long_press = 0;

static void app_driver_button_initial_pressed(void *arg, void *data)
{
    //if(!is_multipress) {
    ESP_LOGW(TAG, "Initial button pressed");
    gpio_button * button = (gpio_button*)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
    // Press moves Position from 0 (idle) to 1 (press)
    uint8_t newPosition     = 1;
    chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, newPosition]() {
        chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
        // InitialPress event takes newPosition as event data
        switch_cluster::event::send_initial_press(switch_endpoint_id, newPosition);
    });
        // is_multipress = 1;
//    }
}

static void app_driver_button_release(void *arg, void *data)
{

    gpio_button *button = (gpio_button *)data;
    int switch_endpoint_id = (button != NULL) ? get_endpoint(button) : 1;
    // chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id]() {
    //     chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, idlePosition);
    // });

    if (iot_button_get_ticks_time((button_handle_t)arg) < CONFIG_BUTTON_LONG_PRESS_TIME_MS || is_long_press == 0) {
        ESP_LOGW(TAG, "Short button release");
        // Release moves Position from 1 (press) to 0 (idle)
        uint8_t previousPosition = 1;
        uint8_t newPosition = 0;
        chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, previousPosition, newPosition]() {
             chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
             // ShortRelease event takes previousPosition as event data
             switch_cluster::event::send_short_release(switch_endpoint_id, previousPosition);
        });

        if (iot_button_get_ticks_time((button_handle_t)arg) >= CONFIG_BUTTON_LONG_PRESS_TIME_MS) {
            ESP_LOGW(TAG, "Multipress Complete");
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
        }
    } else {
        ESP_LOGW(TAG, "Long button release");
        // Release moves Position from 1 (press) to 0 (idle)
        uint8_t previousPosition = 1;
        uint8_t newPosition = 0;
        chip::DeviceLayer::SystemLayer().ScheduleLambda([switch_endpoint_id, previousPosition, newPosition]() {
            chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(switch_endpoint_id, newPosition);
            // LongRelease event takes previousPositionPosition as event data
            switch_cluster::event::send_long_release(switch_endpoint_id, previousPosition);
        });
    }

    is_long_press = 0;
}

static void app_driver_button_long_pressed(void *arg, void *data)
{
    is_long_press = 1;
    ESP_LOGW(TAG, "Long button pressed ");
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
    ESP_LOGW(TAG, "Multipress Ongoing");
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
    ESP_LOGW(TAG, "Multipress Complete");
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
    // is_multipress = 0;
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
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_switch_latched, button);
#endif

#if CONFIG_GENERIC_SWITCH_TYPE_MOMENTARY
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_initial_pressed, button);
    iot_button_register_cb(handle, BUTTON_PRESS_UP, app_driver_button_release, button);
    iot_button_register_cb(handle, BUTTON_PRESS_REPEAT, app_driver_button_multipress_ongoing, button);
    iot_button_register_cb(handle, BUTTON_LONG_PRESS_START, app_driver_button_long_pressed, button);
    iot_button_register_cb(handle, BUTTON_PRESS_REPEAT_DONE, app_driver_button_multipress_complete, button);
#endif
    return (app_driver_handle_t)handle;
}

static void rollback_trigger_callback(void *arg, void *data)
{
    static int count = 0;
    count++;
    printf("repeat click count: %d\n", count);
    if (count >= 5) {
        //roll back
        printf("roll back start---------------\n");
        count = 0;
        led_indicator_start(leds[0], BSP_LED_BLINK_FAST);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        if (esp_ota_mark_app_invalid_rollback_and_reboot() == ESP_ERR_OTA_ROLLBACK_FAILED) {
            printf("roll back failed\n");
            esp_restart();
        }
    }
}

void reset_rollback_button_init()
{
    button_config_t config = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = BUTTON_GPIO_PIN,
            .active_level = 0,
        }
    };

    button_handle_t handle = iot_button_create(&config);
    app_reset_button_register(handle);

    iot_button_register_cb(handle, BUTTON_PRESS_REPEAT, rollback_trigger_callback, NULL);
}

void app_driver_light_init()
{
#if CONFIG_BSP_LEDS_NUM > 0
    /* Initialize led */
    ESP_ERROR_CHECK(bsp_led_indicator_create(leds, NULL, CONFIG_BSP_LEDS_NUM));
    led_indicator_set_hsv(leds[0], SET_HSV(128, 254, 0));
#endif
}

void app_led_blink_start()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("blink start--\n");
    led_indicator_start(leds[0], BSP_LED_BLINK_FAST);
#endif
}

void app_led_blink_stop()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("blink stop--\n");
    led_indicator_stop(leds[0], BSP_LED_BLINK_FAST);
    led_indicator_set_hsv(leds[0], SET_HSV(128, 254, 0));
#endif
}
