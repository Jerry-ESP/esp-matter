/**
 * @file TwDriver.cpp
 * @author AWOX
 * @brief Light driver source file for a TW light
 */

#include "TwDriver.hpp"

/**
 * @brief TW driver namespace
 */
namespace Tw {

/**
 * @brief Constructor for a TW driver
 * @param coldChannelGPIO Cold white GPIO
 * @param warmChannelGPIO Warm white GPIO
 */
Driver::Driver(uint8_t coldChannelGPIO, uint8_t warmChannelGPIO)
{
    initTimer();
    // COLD WHITE INIT
    initChannel(_coldChannel, coldChannelGPIO);

    // WARM WHITE INIT
    initChannel(_warmChannel, warmChannelGPIO);

    ledc_fade_func_install(0); // Needed in order to use the ledc library
    _mode = LIGHT_DRIVER_MODE_TW;
}

/**
 * @brief Set HSV color - This function does nothing for a TW light, because it does not support RGB PWM
 * @param color HSV color
 */
void Driver::setColor(HsvColor color)
{
    // Does nothing for the moment!
}

/**
 * @brief Set hue - This function does nothing for a TW light, because it does not support RGB PWM
 * @param hue Hue
 */
void Driver::setHue(uint8_t hue)
{
    // Does nothing for the moment!
}

/**
 * @brief Set saturation - This function does nothing for a TW light, because it does not support RGB PWM
 * @param saturation Saturation
 */
void Driver::setSaturation(uint8_t saturation)
{
    // Does nothing for the moment!
}

/**
 * @brief Set hue and saturation - This function does nothing for a TW light, because it does not support RGB PWM
 * @param hue Hue
 * @param saturation Saturation
 */
void Driver::setHueAndSaturation(uint8_t hue, uint8_t saturation)
{
    // Does nothing for the moment!
}

/**
 * @brief Set color mode - This function does nothing for a TW light, because it does not support RGB PWM
 * @param mode Color mode
 */
void Driver::setMode(lightDriverMode_t mode)
{
    // Does nothing for the moment!
}

}