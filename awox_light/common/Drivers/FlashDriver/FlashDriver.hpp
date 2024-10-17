/**
 * @file FlashDriver.hpp
 * @author AWOX
 * @brief Drivers header file for flash memory
 */

#include "Api.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <map>
#include <vector>

#pragma once

/**@{*/
/** Flash driver namespaces */
#define CURRENT_STATE_FLASH_NAMESPACE "current"
#define BLE_FLASH_NAMESPACE           "ble"
/**@}*/

constexpr static const uint16_t FLASH_DEFAULT_VALUE_U16 = 0xFFFF; /**< @brief Default flash value for u16 */
constexpr static const uint8_t FLASH_DEFAULT_VALUE_U8 = 0xFF; /**< @brief Default flash value for u8*/

using namespace API;

namespace Flash {

/**
 * @brief Flash driver class
 */
class Driver {
public:
    // Constructor
    Driver();
    uint64_t readData(ATTRIBUTE_ID attributeId);
    void writeData(attributeData_t data);

    bool isErased() const;
    void unsetErase() const;
    void erase() const;

    void readBlePairingInfo() const;
    void writeBlePairingInfo() const;

    uint8_t getRebootCounter() const;
    void setRebootCounter(uint8_t value) const;

private:
    // Members
    nvs_handle_t _currentHandle; /**< @brief Flash current handler */
    nvs_handle_t _bleHandle; /**< @brief Flash handler for BLE */
};
}