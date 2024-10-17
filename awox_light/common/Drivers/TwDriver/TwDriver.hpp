/**
 * @file TwDriver.hpp
 * @author AWOX
 * @brief Light driver header file for a TW light
 */

#include "RgbTwDriver.hpp"

#pragma once

namespace Tw {

/**
 * @brief  TW Light driver (PWM)
 * Modes: Tw
 */
class Driver : public RgbTw::Driver {
protected:
    /**
     * @brief Default constructor for a TW driver
     */
    Driver() {};

public:
    Driver(uint8_t coldChannelGPIO, uint8_t warmChannelGPIO);
    void setColor(HsvColor color);
    void setHue(uint8_t hue);
    void setSaturation(uint8_t saturation);
    void setHueAndSaturation(uint8_t hue, uint8_t saturation);
    void setMode(lightDriverMode_t mode);
};
}