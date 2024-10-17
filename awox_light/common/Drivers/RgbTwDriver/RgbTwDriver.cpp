/**
 * @file RgbTwDriver.cpp
 * @author AWOX
 * @brief Light driver source file for a RGBTW light
 */

#include "RgbTwDriver.hpp"
#include "Color.hpp"
#include "TargetConfig.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PWM_8BITS_TO_12BITS(number)  ((uint32_t)number) * 16 /**< Macro to translate 8 bit PWM to 12 bits */
#define PWM_16BITS_TO_12BITS(number) ((uint32_t)number) / 16 /**< Macro to translate 16 bit PWM to 12 bits */

constexpr static const char* TAG = "awox_rgbtw_driver"; /**< @brief Espressif tag for Log */

/**
 * @brief RGBTW driver namespace
 */
namespace RgbTw {

/**
 * @brief Set initial values
 *
 * @param scene
 */
void Driver::start(AwoxScene scene)
{
    if (scene.isColor()) {
        setHue(scene.getHue());
        setSaturation(scene.getSat());
    } else {
        setTemperature(scene.getTemperature());
        // Setting hue and sat values in the driver to have the default values if only one of the two values is changed and it triggers a color mode change
        _color.setSaturation(scene.getSat());
        _color.setHue(scene.getHue());
    }
    setBrightness(scene.getLevel());
    if (scene.isOn()) {
        setFadeTime(FADE_ON_OFF_MS);
        setOn();
        setFadeTime(FADE_TIME_DEFAULT_MS);
    }
}

/**
 * @brief Internal function for setting up the timer and a common part of PWM channels
 */
void Driver::initTimer()
{
    setFadeTime(FADE_TIME_DEFAULT_MS); // init value
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32h2/api-reference/peripherals/ledc.html?highlight=ledc
    _coldChannel = LEDC_CHANNEL_0;
    _warmChannel = LEDC_CHANNEL_1;
    _redChannel = LEDC_CHANNEL_2;
    _greenChannel = LEDC_CHANNEL_3;
    _blueChannel = LEDC_CHANNEL_4;

    // init timer
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ledc_timer.duty_resolution = LEDC_TIMER_12_BIT;
    ledc_timer.freq_hz = PWM_FREQUENCY; // Set output frequency to common value defined in sdkconfig
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer.deconfigure = false;
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // LED COMMON CONFIG
    _ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    _ledc_channel.timer_sel = LEDC_TIMER_0;
    _ledc_channel.intr_type = LEDC_INTR_DISABLE;
    _ledc_channel.duty = 0; // Set initial duty to 0%
    _ledc_channel.hpoint = 0;
    _ledc_channel.flags.output_invert = 0; // TODO replace this value by CONFIG_INVERTED_PWM_xx if needed later (from sdkconfig)
}

/**
 * @brief Internal function for channel set up.
 * @param channel Channel define
 * @param gpio Pin used in the hardware
 */
void Driver::initChannel(ledc_channel_t channel, uint8_t gpio)
{
    _ledc_channel.channel = channel;
    _ledc_channel.gpio_num = gpio;
    ESP_ERROR_CHECK(ledc_channel_config(&_ledc_channel));
}

/**
 * @brief Internal function to write in a PWM channel.
 */
void Driver::_writeChannel(ledc_channel_t channel, uint8_t value)
{
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,
        channel,
        PWM_16BITS_TO_12BITS(rgbToLumen(value)),
        _fadeTimeMs,
        LEDC_FADE_NO_WAIT);
}

/**
 * @brief Internal function to apply the correct hue color.
 */
void Driver::_applyRgb()
{
    printf("_applyRgb----------\n");
    RgbColor rgb = _color.toRgb();
    ESP_LOGD(TAG, "Updating Rgb color: 0x%x, 0x%x, 0x%x", rgb.getRed(), rgb.getGreen(), rgb.getBlue());
    _writeChannel(_redChannel, rgb.getRed());
    _writeChannel(_greenChannel, rgb.getGreen());
    _writeChannel(_blueChannel, rgb.getBlue());
}

/**
 * @brief Internal function to apply the correct xy color.
 */
void Driver::_xy_applyRgb()
{
    printf("_xy_applyRgb----------\n");
    RgbColor rgb = _xy_color.toXyz(_white.getBrightness()).toRgb();
    ESP_LOGD(TAG, "Updating Rgb color: 0x%x, 0x%x, 0x%x", rgb.getRed(), rgb.getGreen(), rgb.getBlue());
    _writeChannel(_redChannel, rgb.getRed());
    _writeChannel(_greenChannel, rgb.getGreen());
    _writeChannel(_blueChannel, rgb.getBlue());
}

/**
 * @brief Internal function to turn off color.
 */
void Driver::_turnOffRgb()
{
    _writeChannel(_redChannel, 0);
    _writeChannel(_blueChannel, 0);
    _writeChannel(_greenChannel, 0);
}

/**
 * @brief Internal function to turn off white.
 */
void Driver::_turnOffTunnableWhite()
{
    _writeChannel(_warmChannel, 0);
    _writeChannel(_coldChannel, 0);
}

/**
 * @brief Internal function to apply the correct white.
 */
void Driver::_applyTunnableWhite()
{
    uint8_t brightness = _white.getBrightness();
    // Convert mired to WW/CW PWM
    uint8_t ct = cctConvertMiredToCt(_white.getTemperature());
    // get total PWM ticks depending to brightness
    uint16_t totalTicks = rgbToLumen(brightness);
    // get warm brigthness depending of color temperature
    uint8_t lumWarm = CEIL_DIV(((SH_CT_MAX - ct) * brightness), SH_LIGHTNESS_MAX);
    // Split PWM ticks between WW and CW
    if (brightness == 0) {
        brightness = BRIGHTNESS_MIN;
    }
    uint16_t ticksWw = (totalTicks * lumWarm) / brightness;
    uint16_t ticksCw = totalTicks - ticksWw;
    // Apply the PWMs
    ESP_LOGW(TAG, "Updating white: Warm 0x%x, Cold 0x%x", ticksWw / 256, ticksCw / 256);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, _warmChannel, PWM_16BITS_TO_12BITS(ticksWw), _fadeTimeMs, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, _coldChannel, PWM_16BITS_TO_12BITS(ticksCw), _fadeTimeMs, LEDC_FADE_NO_WAIT);
}

/**
 * @brief Constructor for a RGBTW Driver
 * @param coldChannelGPIO Cold white GPIO
 * @param warmChannelGPIO Warm white GPIO
 * @param redChannelGPIO Red GPIO
 * @param greenChannelGPIO Green GPIO
 * @param blueChannelGPIO Blue GPIO
 */
Driver::Driver(uint8_t coldChannelGPIO, uint8_t warmChannelGPIO, uint8_t redChannelGPIO, uint8_t greenChannelGPIO, uint8_t blueChannelGPIO)
{
    _coldChannelGPIO = coldChannelGPIO;
    _warmChannelGPIO = warmChannelGPIO;
    _redChannelGPIO = redChannelGPIO;
    _greenChannelGPIO = greenChannelGPIO;
    _blueChannelGPIO = blueChannelGPIO;

    initTimer();
    // COLD WHITE INIT
    initChannel(_coldChannel, _coldChannelGPIO);

    // WARM WHITE INIT
    initChannel(_warmChannel, _warmChannelGPIO);

    // RED INIT
    initChannel(_redChannel, _redChannelGPIO);

    // GREEN INIT
    initChannel(_greenChannel, _greenChannelGPIO);

    // BLUE INIT
    initChannel(_blueChannel, _blueChannelGPIO);

    ledc_fade_func_install(0); // Needed in order to use the ledc library
}

// GETTERS

/**
 * @brief Get Color
 * @return HsvColor
 */
HsvColor Driver::getColor()
{
    return _color;
}

/**
 * @brief Get Hue value
 * @return uint8_t
 */
uint8_t Driver::getHue()
{
    return _color.getHue();
}

/**
 * @brief Get Saturation value
 * @return uint8_t
 */
uint8_t Driver::getSaturation()
{
    return _color.getSaturation();
}

/**
 * @brief Get X value
 * @return uint16_t
 */
uint16_t Driver::getX()
{
    return _xy_color.getX();
}

/**
 * @brief Get Y value
 * @return uint16_t
 */
uint16_t Driver::getY()
{
    return _xy_color.getY();
}

/**
 * @brief Check if the device is on or off
 * @return true: Is on;
 * @return false: Is off
 */
bool Driver::isOn()
{
    return _on;
}

/**
 * @brief Get the Mode
 * @return lightDriverMode_t
 */
lightDriverMode_t Driver::getMode()
{
    return _mode;
}

/**
 * @brief Get the Brightness object
 * @return uint8_t Brightness
 */
uint8_t Driver::getBrightness()
{
    return _color.getValue();
}

/**
 * @brief Get the Tunnable White object
 * @return TemperatureTw
 */
TemperatureTw Driver::getTunnableWhite()
{
    return _white;
}

/**
 * @brief Get the current Temperature
 * @return uint8_t
 */
uint16_t Driver::getTemperature()
{
    return _white.getTemperature();
}

// SETTERS

/**
 * @brief Set the Tunnable White object
 * @param white
 */
void Driver::setTunnableWhite(TemperatureTw white)
{
    _white = white;
    _mode = LIGHT_DRIVER_MODE_TW;
    updateLightState();
}

/**
 * @brief Set the Color
 * @param mode
 */
void Driver::setColor(lightDriverMode_t mode)
{
    if (mode == LIGHT_DRIVER_MODE_RGB_HSV) {
        _white.setBrightness(_color.getValue());
        _mode = LIGHT_DRIVER_MODE_RGB_HSV;
    } else if (mode == LIGHT_DRIVER_MODE_RGB_XY) {
        _mode = LIGHT_DRIVER_MODE_RGB_XY;
    }
    updateLightState();
}

/**
 * @brief Turn on the device
 */
void Driver::setOn()
{
    _on = true;
    updateLightState();
}

/**
 * @brief Turn off the device
 */
void Driver::setOff()
{
    _turnOffRgb();
    _turnOffTunnableWhite();
    //_on = false;
}

void Driver::updateLightState()
{
    printf("updateLightState----------\n");
    if (_mode == LIGHT_DRIVER_MODE_RGB_HSV) {
        if (_on) {
            printf("LIGHT_DRIVER_MODE_RGB_HSV----------\n");
            _applyRgb();
            _turnOffTunnableWhite();
        }
    } else if (_mode == LIGHT_DRIVER_MODE_RGB_XY) {
        if (_on) {
            printf("LIGHT_DRIVER_MODE_RGB_XY----------\n");
            _xy_applyRgb();
            _turnOffTunnableWhite();
        }
    } else {
        if (_on) {
            _applyTunnableWhite();
            _turnOffRgb();
        }
    }
}

/**
 * @brief Set the Mode, mode also changes when other methods are used. For example set color changes mode to RGB.
 * @param mode
 */
void Driver::setMode(lightDriverMode_t mode)
{
    if (mode == LIGHT_DRIVER_MODE_RGB_HSV || mode == LIGHT_DRIVER_MODE_RGB_XY) {
        _mode = mode;
        updateLightState();
    } else if (mode == LIGHT_DRIVER_MODE_TW) {
        _mode = LIGHT_DRIVER_MODE_TW;
        updateLightState();
    }
}

/**
 * @brief Set the Hue
 * @param hue
 */
void Driver::setHue(uint8_t hue)
{
    _color.setHue(hue);
    printf("set hue --- hue: %d--saturation: %d---value:%d\n", _color.getHue(), _color.getSaturation(), _color.getValue());
    setColor(LIGHT_DRIVER_MODE_RGB_HSV);
}

/**
 * @brief Set the Saturation
 * @param saturation
 */
void Driver::setSaturation(uint8_t saturation)
{
    _color.setSaturation(saturation);
    printf("set saturation --- hue: %d--saturation: %d---value:%d\n", _color.getHue(), _color.getSaturation(), _color.getValue());
    setColor(LIGHT_DRIVER_MODE_RGB_HSV);
}

/**
 * @brief Set the Hue And Saturation
 * @param hue
 * @param saturation
 */
void Driver::setHueAndSaturation(uint8_t hue, uint8_t saturation)
{
    _color.setHue(hue);
    _color.setSaturation(saturation);
    printf("set saturation --- hue: %d--saturation: %d---value:%d\n", _color.getHue(), _color.getSaturation(), _color.getValue());
    setColor(LIGHT_DRIVER_MODE_RGB_HSV);
}

void Driver::setX(uint16_t x)
{
    _xy_color.setX(x);
    setColor(LIGHT_DRIVER_MODE_RGB_XY);
}

void Driver::setY(uint16_t y)
{
    _xy_color.setY(y);
    setColor(LIGHT_DRIVER_MODE_RGB_XY);
}

void Driver::setXY(uint16_t x, uint16_t y)
{
    _xy_color.setX(x);
    _xy_color.setY(y);
    setColor(LIGHT_DRIVER_MODE_RGB_XY);
}

/**
 * @brief Set the Brightness object
 * @param brightness
 */
void Driver::setBrightness(uint8_t brightness)
{
    // Convert brightness from brightness range [1-254] to PWM range [10-255]
    uint8_t correctedBrightness = SH_LIGHTNESS_MIN + brightness * (SH_LIGHTNESS_MAX - SH_LIGHTNESS_MIN) / BRIGHTNESS_RANGE;
    ESP_LOGV(TAG, "Corrected Brightness: %d", correctedBrightness);
    _color.setValue(correctedBrightness);
    _white.setBrightness(correctedBrightness);
    updateLightState();
}

/**
 * @brief Set the Temperature
 * @param temperature
 */
void Driver::setTemperature(uint16_t temperature)
{
    _white.setTemperature(temperature);
    _mode = LIGHT_DRIVER_MODE_TW;
    updateLightState();
}

/**
 * @brief Set the Fade Time transition time
 *
 * @param fadeTimeMs fade time in ms, min value is 100 ms
 * @return Driver* a pointer to the Driver so it's possible to contact instructions (Driver->setFade->setOn)
 */
LightDriverInterface* Driver::setFadeTime(uint32_t fadeTimeMs)
{
    if (fadeTimeMs < FADE_TIME_MIN_MS) {
        _fadeTimeMs = FADE_TIME_MIN_MS;
    } else {
        _fadeTimeMs = fadeTimeMs;
    }
    return this;
}

}
