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
#include "RgbTwI2CDriver.hpp"
#include <product_util.h>

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace RgbTwI2C;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;

typedef struct {
    bool      on_off;
    uint8_t   level;
    uint8_t   color_mode;
    uint8_t   hue;
    uint8_t   saturation;
    uint16_t  current_x;
    uint16_t  current_y;
    uint16_t  color_temp;
} light_current_state_t;

static light_current_state_t light_current_state;

static Driver s_driver = Driver();

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

static esp_err_t light_driver_set_hue(uint16_t hue)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set hue %d", hue);

    s_driver.setHue(hue);
    return err;
}

static esp_err_t light_driver_set_saturation(uint16_t saturation)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set saturation %d", saturation);

    s_driver.setSaturation(saturation);
    return err;
}

static esp_err_t light_driver_set_xyy(uint16_t x, uint16_t y, uint8_t level)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set color X %d, Y %d, level %d", x, y, level);

    s_driver.setXYy(x, y, level);
    return err;
}

static esp_err_t light_driver_set_cct(uint32_t cct)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set cct %ld", cct);

    s_driver.setTemperature(cct);
    return err;
}

static esp_err_t light_driver_set_colormode(lightDriverMode_t color_mode)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set colormode %d", color_mode);

    s_driver.setMode(color_mode);
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

static esp_err_t light_matter_set_hue(uint8_t hue)
{
    ESP_LOGI(TAG, "Set hue");

    return light_driver_set_hue(hue);
}

static esp_err_t light_matter_set_saturation(uint8_t saturation)
{
    ESP_LOGI(TAG, "Set saturation");
    uint16_t value = REMAP_TO_RANGE(saturation, MATTER_SATURATION, STANDARD_SATURATION);

    return light_driver_set_saturation(value);
}

static esp_err_t light_matter_set_current_xy(uint16_t color_x, uint16_t color_y, uint8_t level)
{
    ESP_LOGI(TAG, "Set current xy");

    return light_driver_set_xyy(color_x, color_y, level);
}

static esp_err_t light_matter_set_temperature(uint16_t color_temp)
{
    ESP_LOGI(TAG, "Set temperature");
    uint32_t value = color_temp;//REMAP_TO_RANGE_INVERSE(color_temp, STANDARD_TEMPERATURE_FACTOR);

    return light_driver_set_cct(value);
}

static esp_err_t light_matter_set_colormode(uint8_t color_mode)
{
    ESP_LOGI(TAG, "Set colormode");
    lightDriverMode_t mode;

    if (color_mode== (uint8_t)ColorControl::ColorMode::kCurrentHueAndCurrentSaturation) {
        mode = LIGHT_DRIVER_MODE_RGB_HSV;
    } else if (color_mode == (uint8_t)ColorControl::ColorMode::kColorTemperature) {
        mode = LIGHT_DRIVER_MODE_TW;
    } else if (color_mode == (uint8_t)ColorControl::ColorMode::kCurrentXAndCurrentY) {
        // The xyy color space is currently not supported by any ecosystem,
        // it is only used for certification testing
        mode = LIGHT_DRIVER_MODE_RGB_XY;
    }

    return light_driver_set_colormode(mode);
}

static void print_current_light_state()
{
    printf("Current light state:\n");
    printf("----------onoff:      %s\n", light_current_state.on_off ? "on" : "off");
    printf("----------level:      %d\n", light_current_state.level);
    printf("----------color mode: %d\n", light_current_state.color_mode);
    printf("----------color hue:  %d\n", light_current_state.hue);
    printf("----------color sat:  %d\n", light_current_state.saturation);
    printf("----------color x:    %d\n", light_current_state.current_x);
    printf("----------color y:    %d\n", light_current_state.current_y);
    printf("----------color temp: %d\n", light_current_state.color_temp);
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
        } else if (cluster_id == ColorControl::Id) {
            if (attribute_id == ColorControl::Attributes::CurrentHue::Id) {
                light_current_state.hue = val->val.u8;
                err = light_matter_set_hue(val->val.u8);
            } else if (attribute_id == ColorControl::Attributes::CurrentSaturation::Id) {
                light_current_state.saturation = val->val.u8;
                err = light_matter_set_saturation(val->val.u8);
            } else if (attribute_id == ColorControl::Attributes::ColorTemperatureMireds::Id) {
                light_current_state.color_temp = val->val.u16;
                err = light_matter_set_temperature(val->val.u16);
            } else if (attribute_id == ColorControl::Attributes::CurrentX::Id) {
                light_current_state.current_x = val->val.u16;
                err = light_matter_set_current_xy(val->val.u16, light_current_state.current_y, light_current_state.level);
            } else if (attribute_id == ColorControl::Attributes::CurrentY::Id) {
                light_current_state.current_y = val->val.u16;
                err = light_matter_set_current_xy(light_current_state.current_x, val->val.u16, light_current_state.level);
            } else if (attribute_id == ColorControl::Attributes::ColorMode::Id) {
                light_current_state.color_mode = val->val.u8;
                err = light_matter_set_colormode(val->val.u8);
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

    /* get color attributes*/
    cluster = cluster::get(endpoint, ColorControl::Id);
    attribute = attribute::get(cluster, ColorControl::Attributes::ColorMode::Id);
    attribute::get_val(attribute, &val);
    light_current_state.color_mode = val.val.u8;
    attribute = attribute::get(cluster, ColorControl::Attributes::CurrentHue::Id);
    attribute::get_val(attribute, &val);
    light_current_state.hue = val.val.u8;
    attribute = attribute::get(cluster, ColorControl::Attributes::CurrentSaturation::Id);
    attribute::get_val(attribute, &val);
    light_current_state.saturation = val.val.u8;
    attribute = attribute::get(cluster, ColorControl::Attributes::ColorTemperatureMireds::Id);
    attribute::get_val(attribute, &val);
    light_current_state.color_temp = val.val.u16;
    attribute = attribute::get(cluster, ColorControl::Attributes::CurrentX::Id);
    attribute::get_val(attribute, &val);
    light_current_state.current_x = val.val.u16;
    attribute = attribute::get(cluster, ColorControl::Attributes::CurrentY::Id);
    attribute::get_val(attribute, &val);
    light_current_state.current_y = val.val.u16;

    /* get onoff */
    cluster = cluster::get(endpoint, OnOff::Id);
    attribute = attribute::get(cluster, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    light_current_state.on_off = val.val.b;

    print_current_light_state();

    /* setting brightness*/
    err |= light_matter_set_brightness(light_current_state.level);

    /* Setting color */
    if (light_current_state.color_mode == (uint8_t)ColorControl::ColorMode::kCurrentHueAndCurrentSaturation) {
        /* Setting hue */
        err |= light_matter_set_hue(light_current_state.hue);
        err |= light_matter_set_saturation(light_current_state.saturation);
    } else if (light_current_state.color_mode == (uint8_t)ColorControl::ColorMode::kColorTemperature) {
        /* Setting temperature */
        err |= light_matter_set_temperature(light_current_state.color_temp);
    } else if (light_current_state.color_mode == (uint8_t)ColorControl::ColorMode::kCurrentXAndCurrentY) {
        /* Setting xy */
        err |= light_matter_set_current_xy(light_current_state.current_x, light_current_state.current_y, light_current_state.level);
    } else {
        ESP_LOGE(TAG, "Color mode not supported");
    }

    /* Setting power */
    err |= light_matter_set_power(light_current_state.on_off);

    return err;
}

esp_err_t app_driver_light_init()
{
    return light_driver_init();
}
