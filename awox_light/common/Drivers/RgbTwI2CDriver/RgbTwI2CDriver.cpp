/**
 * @file RgbTwI2CDriver.cpp
 * @author AWOX
 * @brief Source code file for I2C driver
 */
#include "RgbTwI2CDriver.hpp"
#include "I2CDriverConfig.hpp"
#include "esp_log.h"

constexpr static const char* TAG = "awox_Rgb_Tw_I2C_Driver"; /**< Tag for logs */
constexpr uint16_t DRIVER_FREQ_KHZ = 300; /**< Frequency in KHz for the driver */
constexpr uint8_t DEFAULT_CURRENT = 50; /**< Default current for the driver */
namespace RgbTwI2C {
/**
 * @brief Construct a new Driver object
 *
 */
Driver::Driver()
{
    // Driver configuration
    _config.type = DRIVER_BP57x8D;
    _config.driver_conf.bp57x8d = driver_bp57x8d_t {
        .current = CURRENT_ARRAY,
        .iic_clk = (gpio_num_t)ModuleGpio::GPIO_I2C_CLK,
        .iic_sda = (gpio_num_t)ModuleGpio::GPIO_I2C_DATA,
        .enable_iic_queue = true,
        .freq_khz = DRIVER_FREQ_KHZ
    };
    // Capability configuration
    _config.capability = lightbulb_capability_t {
        .fade_time_ms = FADE_TIME_DEFAULT_MS,
        .storage_delay_ms = 0,
        .led_beads = LED_BEADS,
        .storage_cb = nullptr,
        .enable_fade = true,
        .enable_lowpower = false,
        .enable_status_storage = false,
        .enable_hardware_cct = false,
        .enable_precise_cct_control = false,
        .enable_precise_color_control = ENABLE_COLOR_CONTROL,
        .sync_change_brightness_value = true,
        .disable_auto_on = true,
    };
    _config.capability.enable_precise_cct_control = false;
    // Color order configuration
    _config.io_conf.iic_io.red = OUT1;
    _config.io_conf.iic_io.green = OUT2;
    _config.io_conf.iic_io.blue = OUT3;
    _config.io_conf.iic_io.cold_white = OUT5;
    _config.io_conf.iic_io.warm_yellow = OUT4;

    _config.color_mix_mode.precise.table = color_data;
    _config.color_mix_mode.precise.table_size = COLOR_DATA_SIZE;

    // Temperature Configuration
    _config.cct_mix_mode.standard.kelvin_max = MAXIMUM_TEMPERATURE_KELVIN;
    _config.cct_mix_mode.standard.kelvin_min = MINIMUM_TEMPERATURE_KELVIN;
    // Other configuration    lightbulb_works_mode_t mode = WORK_WHITE;
    _config.external_limit = new lightbulb_power_limit_t();
    _config.external_limit->white_max_brightness = 100;
    _config.external_limit->white_min_brightness = 0;
    _config.external_limit->color_max_value = 100;
    _config.external_limit->color_min_value = 0;
    _config.external_limit->white_max_power = 100;
    _config.external_limit->color_max_power = 100;
    _config.gamma_conf = nullptr;
    // Initial values

    _config.init_status = {
        .mode = WORK_WHITE,
    };
    // Initialize the lightbulb
    lightbulb_init(&_config);
}

/**
 * @brief Set the light on
 *
 */
void Driver::setOn()
{
    _isOn = true;
    setBrightness(_level);
}

/**
 * @brief Set the light off
 *
 */
void Driver::setOff()
{
    _isOn = false;
    lightbulb_set_switch(false);
}

/**
 * @brief Check if the light is on
 *
 * @return true if the light is on
 * @return false if the light is off
 */
bool Driver::isOn()
{
    return _isOn;
}

/**
 * @brief Get the mode of the light
 *
 * @return lightDriverMode_t mode of the light
 */
lightDriverMode_t Driver::getMode()
{
    return _mode;
}

/**
 * @brief Set the mode of the light (RGB or TW)
 * @note If the mode is already set, it will not change. Also, if the mode is not supported, it will not change.
 * @param newMode RGB or TW mode
 */
void Driver::setMode(lightDriverMode_t newMode)
{
    _mode = newMode;
    lightbulb_works_mode_t mode = lightbulb_get_mode();
    uint32_t fade = getFadeTime();
    if (newMode == LIGHT_DRIVER_MODE_RGB_HSV ||
         newMode == LIGHT_DRIVER_MODE_RGB_XY) {
        CHECK_CAPABILITY(CAPABILITY_RGB_COLOR);
        if (mode == WORK_COLOR) {
            return;
        } else {
            setFadeTime(FADE_TIME_COLOR_MODE_MS);
            setHue(getHue());
            setFadeTime(fade);
        }
    } else if (newMode == LIGHT_DRIVER_MODE_TW) {
        CHECK_CAPABILITY(CONFIG_CAPABILITY_WHITE);
        if (mode == WORK_WHITE) {
            return;
        } else {
            setFadeTime(FADE_TIME_COLOR_MODE_MS);
            setTemperature(getTemperature());
            setFadeTime(fade);
        }
    } else {
        ESP_LOGW(TAG, "Invalid mode");
        return;
    }
    ESP_LOGD(TAG, "Mode changed to %d", newMode);
}

/**
 * @brief Get the brightness level
 *
 * @return uint8_t brightness level
 */
uint8_t Driver::getBrightness()
{
    return _level;
}

/**
 * @brief Set the brightness level
 *
 * @param brightness level
 */
void Driver::setBrightness(uint8_t brightness)
{
    // Convert brightness from brightness range [1-254] to PWM range [10-255]
    _level = SH_LIGHTNESS_MIN + brightness * (SH_LIGHTNESS_MAX - SH_LIGHTNESS_MIN) / BRIGHTNESS_RANGE;
    ESP_LOGV(TAG, "Corrected Brightness: %d", _level);
    if (brightness == 0) {
        lightbulb_set_switch(false);
    } else {
        if (lightbulb_get_mode() == WORK_WHITE) {
            lightbulb_set_brightness(levelToPercentage(_level));
        } else {
            lightbulb_set_value(levelToPercentage(_level));
        }
        lightbulb_set_switch(true);
    }
}

/**
 * @brief Get the hue value
 *
 * @return uint8_t hue
 */
uint8_t Driver::getHue()
{
    return (uint8_t)((uint16_t)lightbulb_get_hue() * HsvMaxValue / HueTheoricalMaxValue);
}

/**
 * @brief Get the saturation value
 *
 * @return uint8_t saturation
 */
uint8_t Driver::getSaturation()
{
    return percentageToLevel((uint8_t)lightbulb_get_saturation());
}

/**
 * @brief Get the color
 *
 * @return HsvColor color
 */
HsvColor Driver::getColor()
{
    HsvColor color;
    color.setHue(getHue());
    color.setSaturation(getSaturation());
    color.setValue(getBrightness());
    return color;
}

/**
 * @brief Set the hue value
 * @note If the light is in TW mode, it will change to RGB mode. If the change is not supported, it will not change.
 * @param hue value [0, 254]
 */
void Driver::setHue(uint8_t hue)
{
    CHECK_CAPABILITY(CAPABILITY_RGB_COLOR);
    if (lightbulb_get_mode() == WORK_COLOR) {
        lightbulb_set_hue(hue * HueTheoricalMaxValue / HsvMaxValue);
        ESP_LOGW(TAG, "Hue set to %d", (hue * HueTheoricalMaxValue / HsvMaxValue));
    } else {
        uint32_t fade = getFadeTime();
        setFadeTime(FADE_TIME_COLOR_MODE_MS);
        lightbulb_set_hue(hue * HueTheoricalMaxValue / HsvMaxValue);
        setFadeTime(fade);
    }
}

/**
 * @brief Set the saturation value
 * @note If the light is in TW mode, it will change to RGB mode. If the capability is not supported, it will not change.
 * @param saturation value [0, 254]
 */
void Driver::setSaturation(uint8_t saturation)
{
    CHECK_CAPABILITY(CAPABILITY_RGB_COLOR);
    lightbulb_set_saturation(levelToPercentage(saturation));
}

/**
 * @brief Set the hue and saturation values
 * @note If the light is in TW mode, it will change to RGB mode. If the capability is not supported, it will not change.
 * @param hue value [0, 254]
 * @param saturation value [0, 254]
 */
void Driver::setHueAndSaturation(uint8_t hue, uint8_t saturation)
{
    setHue(hue);
    setSaturation(saturation);
}

void Driver::setXYy(uint16_t x, uint16_t y, uint8_t level)
{
    ESP_LOGI(TAG, "Set xyy %d, %d, %d", x, y, level);
    float current_x = ((float)x) / 65535.0f;
    float current_y = ((float)y) / 65535.0f;
    _level = SH_LIGHTNESS_MIN + level * (SH_LIGHTNESS_MAX - SH_LIGHTNESS_MIN) / BRIGHTNESS_RANGE;
    lightbulb_set_xyy(current_x, current_y, levelToPercentage(_level));
}

/**
 * @brief Set the Color
 * @param mode
 */
void Driver::setColor(lightDriverMode_t mode)
{
}


/**
 * @brief Get the fade time
 *
 * @return uint32_t fade time in ms
 */
uint32_t Driver::getFadeTime()
{
    return _fadeTimeMs;
}

/**
 * @brief Set the fade time
 *
 * @param fadeTimeMs fade time in ms
 * @return LightDriverInterface* pointer to the driver
 */
LightDriverInterface* Driver::setFadeTime(uint32_t fadeTimeMs)
{
    _fadeTimeMs = fadeTimeMs;
    lightbulb_set_fade_time(fadeTimeMs);
    return this;
}

/**
 * @brief Get the tunnable white temperature
 *
 * @return TemperatureTw temperature and brightness
 */
TemperatureTw Driver::getTunnableWhite()
{
    return TemperatureTw(getTemperature(), getBrightness());
}

/**
 * @brief Set the tunnable white temperature
 * @note If the light is in RGB mode, it will change to TW mode. If the capability is not supported, it will not change.
 * @param white TemperatureTw
 */
void Driver::setTunnableWhite(TemperatureTw white)
{
    setTemperature(white.getTemperature());
    setBrightness(white.getBrightness());
}

/**
 * @brief Get the temperature
 *
 * @return uint16_t temperature
 */
uint16_t Driver::getTemperature()
{
    return (uint16_t)(KELVIN_TO_MIREDS_RATIO / (uint16_t)lightbulb_get_cct_kelvin());
}

/**
 * @brief Set the temperature
 * @note If the light is in RGB mode, it will change to TW mode. If the capability is not supported, it will not change.
 * @param temperature
 */
void Driver::setTemperature(uint16_t temperature)
{
    CHECK_CAPABILITY(CONFIG_CAPABILITY_WHITE);
    uint16_t mireds;
    if (temperature > MAXIMUM_TEMPERATURE) {
        mireds = MAXIMUM_TEMPERATURE;
    } else if (temperature < MINIMUM_TEMPERATURE) {
        mireds = MINIMUM_TEMPERATURE;
    } else {
        mireds = temperature;
    }
    if (lightbulb_get_mode() == WORK_WHITE) {
        lightbulb_set_cct((uint16_t)(KELVIN_TO_MIREDS_RATIO / mireds));
    } else {
        uint32_t fade = getFadeTime();
        setFadeTime(FADE_TIME_COLOR_MODE_MS);
        lightbulb_set_cct((uint16_t)(KELVIN_TO_MIREDS_RATIO / mireds));
        setFadeTime(fade);
    }
}

/**
 * @brief light bulb indicate blink or breate effect
 * @param *config
 */
void Driver::indicaceBreathOrBlink(lightbulb_effect_config_t *config)
{
    lightbulb_effect_config_t effect_config = {
        .effect_type = config->effect_type,
        .mode = config->mode,
        .hue = config->hue,
        .saturation = config->saturation,
        .cct = config->cct,
        .min_value_brightness = config->min_value_brightness,
        .max_value_brightness = config->max_value_brightness,
        .effect_cycle_ms = config->effect_cycle_ms,
        .total_ms = config->total_ms,
        .user_cb = config->user_cb,
        .interrupt_forbidden = config->interrupt_forbidden,
    };

    lightbulb_basic_effect_start(&effect_config);
}

/**
 * @brief light bulb indicator restore to previous state
 */
void Driver::indicaceRestore()
{
    bool light_status = true;
    lightbulb_basic_effect_stop_and_restore();

    ESP_LOGD(TAG, "lightbulb restore to previous state");
    lightbulb_set_switch(light_status);
}

/**
 * @brief indicator user call back to restore after effect finish
 */
void Driver::indicatorEffectCallback()
{
    ESP_LOGI(TAG, "lightbulb effect complete, restore to previous state");
    lightbulb_basic_effect_stop_and_restore();
}

} // namespace RgbTwI2C