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
#include "bsp/esp-bsp.h"
#include "bsp/esp_bsp_devkit.h"

#include "led_indicator_blink_default.h"
#include <esp_ota_ops.h>

#include <app_priv.h>
#include <iot_button.h>
#include <app_reset.h>

#define BUTTON_GPIO_PIN GPIO_NUM_9

static led_indicator_handle_t leds[CONFIG_BSP_LEDS_NUM];

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;

/* Do any conversions/remapping for the actual value here */
static esp_err_t app_driver_light_set_power(led_indicator_handle_t handle, esp_matter_attr_val_t *val)
{
#if CONFIG_BSP_LEDS_NUM > 0
    esp_err_t err = ESP_OK;
    if (val->val.b) {
        err = led_indicator_start(handle, BSP_LED_ON);
    } else {
        err = led_indicator_start(handle, BSP_LED_OFF);
    }
    return err;
#else
    ESP_LOGI(TAG, "LED set power: %d", val->val.b);
    return ESP_OK;
#endif
}

static esp_err_t app_driver_light_set_brightness(led_indicator_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_BRIGHTNESS, STANDARD_BRIGHTNESS);
#if CONFIG_BSP_LEDS_NUM > 0
    return led_indicator_set_brightness(handle, value);
#else
    ESP_LOGI(TAG, "LED set brightness: %d", value);
    return ESP_OK;
#endif
}

static esp_err_t app_driver_light_set_hue(led_indicator_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_HUE, STANDARD_HUE);
#if CONFIG_BSP_LEDS_NUM > 0
    led_indicator_ihsv_t hsv;
    hsv.value = led_indicator_get_hsv(handle);
    hsv.h = value;
    return led_indicator_set_hsv(handle, hsv.value);
#else
    ESP_LOGI(TAG, "LED set hue: %d", value);
    return ESP_OK;
#endif
}

static esp_err_t app_driver_light_set_saturation(led_indicator_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_SATURATION, STANDARD_SATURATION);
#if CONFIG_BSP_LEDS_NUM > 0
    led_indicator_ihsv_t hsv;
    hsv.value = led_indicator_get_hsv(handle);
    hsv.s = value;
    return led_indicator_set_hsv(handle, hsv.value);
#else
    ESP_LOGI(TAG, "LED set saturation: %d", value);
    return ESP_OK;
#endif
}

static esp_err_t app_driver_light_set_temperature(led_indicator_handle_t handle, esp_matter_attr_val_t *val)
{
    uint32_t value = REMAP_TO_RANGE_INVERSE(val->val.u16, STANDARD_TEMPERATURE_FACTOR);
#if CONFIG_BSP_LEDS_NUM > 0
    return led_indicator_set_color_temperature(handle, value);
#else
    ESP_LOGI(TAG, "LED set temperature: %ld", value);
    return ESP_OK;
#endif
}

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_endpoint_id) {
        led_indicator_handle_t handle = (led_indicator_handle_t)driver_handle;
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                err = app_driver_light_set_power(handle, val);
            }
        } else if (cluster_id == LevelControl::Id) {
            if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
                err = app_driver_light_set_brightness(handle, val);
            }
        }
    }
    return err;
}

esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    void *priv_data = endpoint::get_priv_data(endpoint_id);
    led_indicator_handle_t handle = (led_indicator_handle_t)priv_data;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    /* Setting brightness */
    attribute_t *attribute = attribute::get(endpoint_id, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_brightness(handle, &val);

    /* Setting power */
    attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_power(handle, &val);

    return err;
}

app_driver_handle_t app_driver_light_init()
{
#if CONFIG_BSP_LEDS_NUM > 0
    /* Initialize led */
    ESP_ERROR_CHECK(bsp_led_indicator_create(leds, NULL, CONFIG_BSP_LEDS_NUM));
    led_indicator_set_hsv(leds[0], SET_HSV(DEFAULT_HUE, DEFAULT_SATURATION, DEFAULT_BRIGHTNESS));

    return (app_driver_handle_t)leds[0];
#else
    return NULL;
#endif
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_handle_t btns[BSP_BUTTON_NUM];
    ESP_ERROR_CHECK(bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM));
    ESP_ERROR_CHECK(iot_button_register_cb(btns[0], BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL));

    return (app_driver_handle_t)btns[0];
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

static esp_timer_handle_t led_active_stop_timer;

static void led_active_stop(void *priv)
{
    ESP_LOGW(TAG, "Stop led breath or blink.");
    app_led_active_stop();
    if (led_active_stop_timer) {
        esp_timer_stop(led_active_stop_timer);
        esp_timer_delete(led_active_stop_timer);
        led_active_stop_timer = NULL;
    }
}

esp_err_t app_led_start_timer(uint8_t time_s)
{
    if (led_active_stop_timer) {
        esp_timer_stop(led_active_stop_timer);
        esp_timer_delete(led_active_stop_timer);
        led_active_stop_timer = NULL;
    }
    if (time_s == 0) {
        return ESP_OK;
    }
    esp_timer_create_args_t led_stop_timer_conf = {.callback = led_active_stop,
                                                    .arg = NULL,
                                                    .dispatch_method = ESP_TIMER_TASK,
                                                    .name = "led_active_stop_tm"};
    if (esp_timer_create(&led_stop_timer_conf, &led_active_stop_timer) == ESP_OK) {
        esp_timer_start_once(led_active_stop_timer, time_s * 1000000);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Failed to create led stop timer.");
    }
    return ESP_FAIL;
}

void app_led_blink_start()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("--blink fast start--\n");
    app_led_active_stop();
    led_indicator_start(leds[0], BSP_LED_BLINK_FAST);
#endif
}

void app_led_blink_fast_start()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("--blink fast start--\n");
    app_led_active_stop();
    led_indicator_start(leds[0], BSP_LED_BLINK_FAST);
    app_led_start_timer(15);
#endif
}

void app_led_blink_slow_start()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("--blink slow start--\n");
    app_led_active_stop();
    led_indicator_start(leds[0], BSP_LED_BLINK_SLOW);
    app_led_start_timer(5);
#endif
}

void app_led_breath_fast_start()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("--breath fast start--\n");
    app_led_active_stop();
    led_indicator_start(leds[0], BSP_LED_BREATHE_FAST);
    app_led_start_timer(15);
#endif
}

void app_led_breath_slow_start()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("--breath slow start--\n");
    app_led_active_stop();
    led_indicator_start(leds[0], BSP_LED_BREATHE_SLOW);
    app_led_start_timer(10);
#endif
}

void app_led_active_stop()
{
#if CONFIG_BSP_LEDS_NUM > 0
    printf("--active stop----\n");
    led_indicator_stop(leds[0], BSP_LED_BLINK_FAST);
    led_indicator_stop(leds[0], BSP_LED_BLINK_SLOW);
    led_indicator_stop(leds[0], BSP_LED_BREATHE_FAST);
    led_indicator_stop(leds[0], BSP_LED_BREATHE_SLOW);
    led_indicator_set_hsv(leds[0], SET_HSV(128, 254, 0));
#endif
}
