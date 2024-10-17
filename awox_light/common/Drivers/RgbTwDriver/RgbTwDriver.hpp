/**
 * @file RgbTwDriver.hpp
 * @author AWOX
 * @brief Light driver header file for a RGBTW light
 */

#include "AwoxScene.hpp"
#include "LightDriver.hpp"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <stdint.h>

#pragma once
namespace RgbTw {
/**
 * @brief RgbTw Light driver (PWM)
 * Modes: RGB & TW
 */
class Driver : public LightDriverInterface {
private:
    uint32_t _fadeTimeMs; /**< Fade Time parameter in ms, has a minimun value defined at FADE_TIME_MIN_MS and default FADE_TIME_DEFAULT_MS*/
protected:
    // Members
    HsvColor _color; /**< Current HSV color */
    XyColor _xy_color = XyColor(0,0); /**< Current XY color */
    TemperatureTw _white; /**< Current TW color + brightness */
    ledc_channel_t _coldChannel; /**< Cold white PWM */
    ledc_channel_t _warmChannel; /**< Warm white PWM */
    ledc_channel_t _redChannel; /**< Red PWM */
    ledc_channel_t _greenChannel; /**< Green PWM */
    ledc_channel_t _blueChannel; /**< Blue PWM */
    ledc_channel_config_t _ledc_channel; /**< PWM configuration */
    uint8_t _coldChannelGPIO;
    uint8_t _warmChannelGPIO;
    uint8_t _redChannelGPIO;
    uint8_t _greenChannelGPIO;
    uint8_t _blueChannelGPIO;
    // Functions
    /**
     * @brief Default constructor for a RGBTW driver
     */
    Driver() = default;
    void updateLight() {};
    void updateLightState();
    void initTimer();
    void initChannel(ledc_channel_t channel, uint8_t gpio);
    void _applyTunnableWhite();
    void _turnOffTunnableWhite();
    void _applyRgb();
    void _xy_applyRgb();
    void _turnOffRgb();
    void _writeChannel(ledc_channel_t channel, uint8_t value);

public:
    Driver(uint8_t coldChannelGPIO, uint8_t warmChannelGPIO, uint8_t redChannelGPIO, uint8_t greenChannelGPIO, uint8_t blueChannelGPIO);
    void start(AwoxScene scene);

    // ON/OFF
    void setOn();
    void setOff();
    bool isOn();

    //  DIM
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness();

    // TW
    TemperatureTw getTunnableWhite();
    uint16_t getTemperature();
    void setTunnableWhite(TemperatureTw white);
    void setTemperature(uint16_t temperature);
    // RGB
    HsvColor getColor();
    uint8_t getHue();
    uint8_t getSaturation();
    uint16_t getX();
    uint16_t getY();
    void setColor(lightDriverMode_t mode);
    void setHue(uint8_t hue);
    void setSaturation(uint8_t saturation);
    void setHueAndSaturation(uint8_t hue, uint8_t saturation);
    void setX(uint16_t x);
    void setY(uint16_t y);
    void setXY(uint16_t x, uint16_t y);
    lightDriverMode_t getMode();

    void setMode(lightDriverMode_t mode);

    // Smoothness
    LightDriverInterface* setFadeTime(uint32_t fadeTimeMs);
    /**
     * @brief Get the Fade Time
     *
     * @return uint32_t fade time in ms
     */
    uint32_t getFadeTime() { return _fadeTimeMs; }
};

}
