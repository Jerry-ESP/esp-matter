/**
 * @file DimmableDriver.cpp
 * @author AWOX
 * @brief Drivers source file for Dimmable lights
 */

#include "DimmableDriver.hpp"

/**
 * @brief Namespace for Dimmable drivers
 */
namespace Dimmable {

/**
 * @brief Constructor for a Dimmable driver
 * @param channelGPIO Light (only) GPIO
 */
Driver::Driver(uint8_t channelGPIO)
{
    initTimer();
    // WARM WHITE INIT
    initChannel(_warmChannel, channelGPIO);

    ledc_fade_func_install(0); // Needed in order to use the ledc library

    // The dimmable mode works internally as a TW with the maximum temperature (warm channel).
    // However, externally shows the state as Dimmable.
    _mode = LIGHT_DRIVER_MODE_TW;
    _white.setTemperature(MAXIMUM_TEMPERATURE);
}

/**
 * @brief Set TW (white + brightness) - This function does nothing for a Dimmable light, because it does not support TW PWM
 * @param white White
 */
void Driver::setTunnableWhite(TemperatureTw white)
{
    // Doest nothing for the moment!
}

/**
 * @brief Set TW - This function does nothing for a Dimmable light, because it does not support TW PWM
 * @param temperature Temperature
 */
void Driver::setTemperature(uint16_t temperature)
{
    // Doest nothing for the moment!
}

/**
 * @brief Get color mode - For a dimmable light, it only supports dimmable mode
 * @return Current color mode
 */
lightDriverMode_t Driver::getMode()
{
    return LIGHT_DRIVER_MODE_DIMMABLE;
}

}