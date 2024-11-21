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
#include <app_priv.h>

#include "Color.hpp"
#include "DimmableDriver.hpp"
#include <product_util.h>

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace Dimmable;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;

#define PWM_PIN    22

typedef struct {
    bool      on_off;
    uint8_t   level;
    uint8_t   color_mode;
    uint16_t  color_temp;
} light_current_state_t;

static light_current_state_t light_current_state;

static Driver s_driver = Driver(PWM_PIN);

static esp_err_t light_driver_init()
{
    return ESP_OK;
}

Driver& get_light_driver()
{
    return s_driver;
}

static esp_err_t light_driver_set_power(bool power)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set power: %d", power);

    if (power) {
        s_driver.setOn();
    } else {
        s_driver.setOff();
    }

    return err;
}

static esp_err_t light_driver_set_brightness(uint16_t level)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set brightness: %d", level);

    s_driver.setBrightness(level);
    return err;
}

/* Do any conversions/remapping for the actual value here */
static esp_err_t light_matter_set_power(bool onoff)
{
    ESP_LOGI(TAG, "Set power");
    return light_driver_set_power(onoff);
}

static esp_err_t light_matter_set_brightness(uint8_t level)
{
    ESP_LOGI(TAG, "Set brightness");

    return light_driver_set_brightness(level);
}

static void print_current_light_state()
{
    printf("Current light state:\n");
    printf("----------onoff:      %s\n", light_current_state.on_off ? "on" : "off");
    printf("----------level:      %d\n", light_current_state.level);
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    product_print_mem();
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_endpoint_id) {
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                light_current_state.on_off = val->val.b;
                err = light_matter_set_power(val->val.b);
            }
        } else if (cluster_id == LevelControl::Id) {
            if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
                light_current_state.level = val->val.u8;
                err = light_matter_set_brightness(val->val.u8);
            }
        }
        print_current_light_state();
    }
    return err;
}

esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = NULL;
    attribute_t *attribute = NULL;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    /* get current level */
    cluster = cluster::get(endpoint, LevelControl::Id);
    attribute = attribute::get(cluster, LevelControl::Attributes::CurrentLevel::Id);
    attribute::get_val(attribute, &val);
    light_current_state.level = val.val.u8;

    /* get onoff */
    cluster = cluster::get(endpoint, OnOff::Id);
    attribute = attribute::get(cluster, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    light_current_state.on_off = val.val.b;

    print_current_light_state();

    /* setting brightness*/
    err |= light_matter_set_brightness(light_current_state.level);

    /* Setting power */
    err |= light_matter_set_power(light_current_state.on_off);

    return err;
}

esp_err_t app_driver_light_init()
{
    return light_driver_init();
}
