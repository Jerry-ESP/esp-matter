/**
 * @file RgbTwI2CDriver.hpp
 * @author AWOX
 * @brief Header file for I2C driver
 */
#include "LightDriver.hpp"
#include "lightbulb.h"

#pragma once

/**
 * @brief Namespace for the I2C driver
 *
 */
namespace RgbTwI2C {
/**
 * @brief RgbTwI2C Light driver. It will adapt to Dimmable, TW, RGBW or RGBTW according to the capabilities
 *
 */
class Driver : public LightDriverInterface {
public:
    ~Driver() = default; /**< Destructor */
    explicit Driver();
    void init();
    /*---SWITCH---*/
    void setOn() final;
    void setOff() final;
    bool isOn() final;
    /*---MODE---*/
    lightDriverMode_t getMode() final;
    void setMode(lightDriverMode_t mode) final;
    void updateLight() final {}; // Not needed for I2C Driver
    /*---DIMMABLE---*/
    void setBrightness(uint8_t brightness) final;
    uint8_t getBrightness() final;
    /*---COLOR---*/
    HsvColor getColor() final;
    uint8_t getHue() final;
    uint8_t getSaturation() final;
    void setColor(lightDriverMode_t mode);
    void setHue(uint8_t hue) final;
    void setSaturation(uint8_t saturation) final;
    void setHueAndSaturation(uint8_t hue, uint8_t saturation) final;
    void setXYy(uint16_t x, uint16_t y, uint8_t level);
    /*---FADE TIME---*/
    uint32_t getFadeTime() final;
    LightDriverInterface* setFadeTime(uint32_t fadeTimeMs) final;
    /*--- WHITE ---*/
    TemperatureTw getTunnableWhite() final;
    uint16_t getTemperature() final;
    void setTunnableWhite(TemperatureTw white) final;
    void setTemperature(uint16_t temperature) final;

private:
    lightbulb_config_t _config = lightbulb_config_t(); /**< Configuration for the driver */
    uint8_t _level; /**< Brightness common level */
    bool _isOn = false; /**< On/Off state */
    uint32_t _fadeTimeMs = FADE_TIME_DEFAULT_MS; /**< Fade Time parameter in ms */
};
}