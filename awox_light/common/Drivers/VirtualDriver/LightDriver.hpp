/**
 * @file LightDriver.hpp
 * @author AWOX
 * @brief Virtual light driver header file that will be used for all the drivers (RGBTW, RGBW, TW, Dimmable...)
 */

#include "Color.hpp"
#include <stdint.h>

#pragma once

constexpr uint32_t FADE_TIME_MIN_MS = 1; /**< Minimum fade time possible*/
constexpr uint32_t FADE_TIME_DEFAULT_MS = 100; /**< Default fade time*/
constexpr uint32_t FADE_TIME_CANDLE_MODE_MS = FADE_TIME_DEFAULT_MS / 2; /**< Default fade time*/
constexpr uint32_t FADE_TIME_COLOR_MODE_MS = 300; /**< Default fade time*/
constexpr uint32_t FADE_ON_OFF_MS = 200; /**< Default fade time. Must be equal to onoff transition time value from level cluster.*/

/**
 * @brief Abstract class for a Light Driver. Contains ON/OFF - Dimmable - Temperature - Color interface.
 */
class LightDriverInterface {
protected:
    // Members
    lightDriverMode_t _mode; /**< Possible modes a light can implement */
    bool _on; /**< Light state ON/OFF */

    // Functions
    virtual void updateLight() = 0; /**< Virtual function, need to be implemented in daughter classes */

public:
    // Functions
    // ON - OFF CAP
    virtual void setOn() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setOff() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual bool isOn() = 0; /**< Virtual function, need to be implemented in daughter classes */

    //  DIM CAP
    virtual void setBrightness(uint8_t brightness) = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual uint8_t getBrightness() = 0; /**< Virtual function, need to be implemented in daughter classes */

    // TW CAP
    virtual TemperatureTw getTunnableWhite() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual uint16_t getTemperature() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setTunnableWhite(TemperatureTw white) = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setTemperature(uint16_t temperature) = 0; /**< Virtual function, need to be implemented in daughter classes */

    // RGB CAP
    virtual HsvColor getColor() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual uint8_t getHue() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual uint8_t getSaturation() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setColor(lightDriverMode_t mode) = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setHue(uint8_t hue) = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setSaturation(uint8_t saturation) = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setHueAndSaturation(uint8_t hue, uint8_t saturation) = 0; /**< Virtual function, need to be implemented in daughter classes */
    // OUTPUT MODE
    virtual lightDriverMode_t getMode() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual void setMode(lightDriverMode_t mode) = 0; /**< Virtual function, need to be implemented in daughter classes */
    // Smoothness
    virtual LightDriverInterface* setFadeTime(uint32_t fadeTimeMs) = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual uint32_t getFadeTime() = 0; /**< Virtual function, need to be implemented in daughter classes */
// Backlight exclusive functions
#if (CAPABILITY_BACKLIGHT)
    virtual void updateRgbBrightness(uint8_t value) = 0;
#endif
};
