/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <esp_matter.h>
#include "bsp/esp-bsp.h"

#include <app_priv.h>

#include <lightbulb.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;

#ifdef CONFIG_ENABLE_PWM_DRIVER

#define PWM_RED_PIN   14
#define PWM_GREEN_PIN 12
#define PWM_BLUE_PIN  4
#define PWM_CW_PIN    10
#define PWM_WW_PIN    11

#endif

#ifdef CONFIG_ENABLE_BP57x8D_DRIVER

#define IIC_SDA 10
#define IIC_SCL 22

#endif

typedef struct {
    bool switch_fade;   /* Whether to enable the fade when the switch state changes. */
    bool color_fade;    /* Whether to enable the fade when the color/brightness state changes.*/
} __attribute__((packed)) light_config_t;

static light_config_t light_config = {
    .switch_fade = false,
    .color_fade = true,
};

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

static esp_err_t light_driver_init()
{
    lightbulb_gamma_config_t gamma_data = {
        .balance_coefficient = {(float)1.0, (float)1.0, (float)1.0, (float)1.0, (float)1.0},
        .curve_coefficient = (float)1.0,
    };

    lightbulb_power_limit_t power_limit_config = {
        .white_max_brightness = 100,
        .white_min_brightness = 10,
        .color_max_value = 100,
        .color_min_value = 10,
        .white_max_power = 100,
        .color_max_power = 100,
    };

    lightbulb_config_t led_config = {
#ifdef CONFIG_ENABLE_PWM_DRIVER
        .type = DRIVER_ESP_PWM,
        .driver_conf = {
            .pwm = {
                .freq_hz = 2000
            },
        },
#endif

#ifdef CONFIG_ENABLE_BP57x8D_DRIVER
        .type = DRIVER_BP57x8D,
        .driver_conf = {
            .bp57x8d = {
                .current = {31, 31, 31, 31, 31},
                .iic_clk = (gpio_num_t)IIC_SCL,
                .iic_sda = (gpio_num_t)IIC_SDA,
                .freq_khz = 300,
            },
        },
#endif

        .cct_mix_mode = {
            .standard = {
                .kelvin_min = 2200,
                .kelvin_max = 7000
            },
        },

        .gamma_conf = &gamma_data,
        .external_limit = &power_limit_config,

#ifdef CONFIG_ENABLE_PWM_DRIVER
        .io_conf = {
            .pwm_io = {
                .red = (gpio_num_t)PWM_RED_PIN,
                .green = (gpio_num_t)PWM_GREEN_PIN,
                .blue = (gpio_num_t)PWM_BLUE_PIN,
                .cold_cct = (gpio_num_t)PWM_CW_PIN,
                .warm_brightness = (gpio_num_t)PWM_WW_PIN,
            },
        },
#endif

#ifdef CONFIG_ENABLE_BP57x8D_DRIVER
        .io_conf = {
            .iic_io = {
                .red = OUT1,
                .green = OUT2,
                .blue = OUT3,
                .cold_white = OUT4,
                .warm_yellow = OUT5,
            },
        },
#endif

        .capability = {
            .fade_time_ms = 800,
            .led_beads = LED_BEADS_5CH_RGBCW,
            .enable_fade = true,
            .enable_lowpower = false,
            .enable_status_storage = false,
            .enable_hardware_cct = true,
            .enable_precise_cct_control = false,
            .sync_change_brightness_value = true,
            .disable_auto_on = true,
        },

        .init_status = {
            .mode = WORK_COLOR,
        },
    };

    esp_err_t err = lightbulb_init(&led_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "lightbulb driver init failed");
    }

    return err;
}

static esp_err_t light_driver_set_power(bool power)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set power: %d", power);

    lightbulb_status_t lightbulb_status;
    err |= lightbulb_get_all_detail(&lightbulb_status);
    err |= lightbulb_set_fades_function(light_config.switch_fade);
    lightbulb_status.on = power;
    err |= lightbulb_update_status(&lightbulb_status, true);

    return err;
}

static esp_err_t light_driver_set_brightness(uint16_t level)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set brightness: %d", level);

    lightbulb_status_t lightbulb_status;
    err |= lightbulb_get_all_detail(&lightbulb_status);
    err |= lightbulb_set_fades_function(light_config.color_fade);
    if (lightbulb_status.mode == WORK_COLOR) {
        lightbulb_status.value = level;
    } else if (lightbulb_status.mode == WORK_WHITE) {
        lightbulb_status.brightness = level;
    } else {
        lightbulb_status.brightness = level;
    }
    err |= lightbulb_update_status(&lightbulb_status, true);

    return err;
}

static esp_err_t light_driver_set_hue(uint16_t hue)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set hue %d", hue);

    lightbulb_status_t lightbulb_status;
    err |= lightbulb_get_all_detail(&lightbulb_status);
    err |= lightbulb_set_fades_function(light_config.color_fade);
    lightbulb_status.hue = hue;
    err |= lightbulb_update_status(&lightbulb_status, true);

    return err;
}

static esp_err_t light_driver_set_saturation(uint16_t saturation)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set saturation %d", saturation);

    lightbulb_status_t lightbulb_status;
    err |= lightbulb_get_all_detail(&lightbulb_status);
    err |= lightbulb_set_fades_function(light_config.color_fade);
    lightbulb_status.saturation = saturation;
    err |= lightbulb_update_status(&lightbulb_status, true);

    return err;
}

static esp_err_t light_driver_set_xyy(float x, float y, float level)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set xyy %f, %f, %f", x, y, level);

    err = lightbulb_set_xyy(x, y, level);
    uint8_t r, g, b, s, v = 0;
    uint16_t h = 0;
    lightbulb_status_t lightbulb_status;

    lightbulb_xyy2rgb(x, y, level, &r, &g, &b);
    lightbulb_rgb2hsv(r, g, b, &h, &s, &v);

    err |= lightbulb_get_all_detail(&lightbulb_status);
    err |= lightbulb_set_fades_function(light_config.color_fade);
    lightbulb_status.mode = WORK_COLOR;
    lightbulb_status.hue = h;
    lightbulb_status.saturation = s;
    lightbulb_status.value = v;
    err |= lightbulb_update_status(&lightbulb_status, true);

    return err;
}

static esp_err_t light_driver_set_cct(uint32_t cct)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set cct %ld", cct);

    lightbulb_status_t lightbulb_status;
    err |= lightbulb_get_all_detail(&lightbulb_status);
    err |= lightbulb_set_fades_function(light_config.color_fade);
    err |= lightbulb_kelvin2percentage(cct, &lightbulb_status.cct_percentage);
    printf("cct:%ld-----------cct percentage: %d-----\n", cct, lightbulb_status.cct_percentage);
    err |= lightbulb_update_status(&lightbulb_status, true);

    return err;
}

static esp_err_t light_driver_set_colormode(uint8_t color_mode)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Set colormode %d", color_mode);

    lightbulb_status_t lightbulb_status;
    err |= lightbulb_get_all_detail(&lightbulb_status);
    err |= lightbulb_set_fades_function(light_config.color_fade);
    lightbulb_status.mode = (lightbulb_works_mode_t)color_mode;
    err |= lightbulb_update_status(&lightbulb_status, true);

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
    double value = REMAP_TO_RANGE(level * 1.0, MATTER_BRIGHTNESS, STANDARD_BRIGHTNESS);
    uint16_t brightness = (uint16_t)ceil(value);

    return light_driver_set_brightness(brightness);
}

static esp_err_t light_matter_set_hue(uint8_t hue)
{
    ESP_LOGI(TAG, "Set hue");
    int value = (int)ceil((hue * 1.0 * STANDARD_HUE) / MATTER_HUE);

    return light_driver_set_hue(value);
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

    float current_x = ((float)color_x) / MATTER_CURRENT_XY_DIVISOR;
    float current_y = ((float)color_y) / MATTER_CURRENT_XY_DIVISOR;
    float level_f = REMAP_TO_RANGE(level, MATTER_BRIGHTNESS, STANDARD_BRIGHTNESS);

    return light_driver_set_xyy(current_x, current_y, level_f);
}

static esp_err_t light_matter_set_temperature(uint16_t color_temp)
{
    ESP_LOGI(TAG, "Set temperature");
    uint32_t value = REMAP_TO_RANGE_INVERSE(color_temp, STANDARD_TEMPERATURE_FACTOR);

    return light_driver_set_cct(value);
}

static esp_err_t light_matter_set_colormode(uint8_t color_mode)
{
    ESP_LOGI(TAG, "Set colormode");
    lightbulb_works_mode_t mode;

    if (color_mode== (uint8_t)ColorControl::ColorMode::kCurrentHueAndCurrentSaturation) {
        mode = WORK_COLOR;
    } else if (color_mode == (uint8_t)ColorControl::ColorMode::kColorTemperature) {
        mode = WORK_WHITE;
    } else if (color_mode == (uint8_t)ColorControl::ColorMode::kCurrentXAndCurrentY) {
        // The xyy color space is currently not supported by any ecosystem,
        // it is only used for certification testing
        mode = WORK_COLOR;
    }

    return light_driver_set_colormode((uint8_t)mode);
}

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
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
    if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kCurrentHueAndCurrentSaturation) {
        /* Setting hue */
        err |= light_matter_set_hue(light_current_state.hue);
        err |= light_matter_set_saturation(light_current_state.saturation);
    } else if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kColorTemperature) {
        /* Setting temperature */
        err |= light_matter_set_temperature(light_current_state.color_temp);
    } else if (val.val.u8 == (uint8_t)ColorControl::ColorMode::kCurrentXAndCurrentY) {
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
