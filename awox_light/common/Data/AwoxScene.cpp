#include "AwoxScene.hpp"

/**
 * @brief function to verify that the zigbee value is valid
 *
 * @param value a uint8_t zigbee attibute as HUE or SAT
 * @return uint8_t a value in range [0x0,0xFF]
 */
uint8_t AwoxScene::validate(uint8_t value) const
{
    uint8_t validValue = value;
    if (value > MAX_ZIGBEE_U8) {
        validValue = MAX_ZIGBEE_U8;
    }
    return validValue;
}
