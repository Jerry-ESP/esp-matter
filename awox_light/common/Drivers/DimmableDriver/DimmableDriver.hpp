/**
 * @file DimmableDriver.hpp
 * @author AWOX
 * @brief Drivers header file for Dimmable lights
 */

#include "TwDriver.hpp"

#pragma once

namespace Dimmable {

/**
 * @brief Dimmable Light driver (PWM)
 * Modes: Dimmable
 */
class Driver : public Tw::Driver {
public:
    Driver(uint8_t channelGPIO);
    void setTunnableWhite(TemperatureTw white);
    void setTemperature(uint16_t temperature);
    lightDriverMode_t getMode();
};

}