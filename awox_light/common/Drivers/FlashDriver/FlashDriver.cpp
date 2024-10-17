/**
 * @file FlashDriver.cpp
 * @author AWOX
 * @brief Drivers source file for flash memory
 */

#include "FlashDriver.hpp"
#include "BluetoothData.hpp"
#include "esp_system.h"
#include <algorithm>
#include <cstring>
#include <product_util.h>

using namespace API;
/**
 * @brief Flash driver namespace
 */
namespace Flash {

constexpr static const char* TAG = "awox_flash_driver"; /**< @brief Espressif tag for Log */

/**@{*/
/** Flash driver keys */
constexpr static const char* eraseKey = "reset";
constexpr static const char* blePairingInfoKey = "blePairingInfo";
constexpr static const char* rebootCounterKey = "rebootCounter";

/**@}*/

/**
 * @brief Constructor for a Flash driver
 */
Driver::Driver()
{
    nvs_flash_init();
    nvs_open(CURRENT_STATE_FLASH_NAMESPACE, NVS_READWRITE, &_currentHandle);
    nvs_open(BLE_FLASH_NAMESPACE, NVS_READWRITE, &_bleHandle);
}

/**
 * @brief Store data into flash
 * @param data
 */
void Driver::writeData(attributeData_t data)
{
    switch (data.id) {
    case ATTRIBUTE_ID::TOUCHLINK_FACTORY_RESET:
        // erase();
        // esp_restart(); // Restart the device, may be deleted when Matter is implemented
        product_factory_reset();
        break;
    case ATTRIBUTE_ID::FLASH_BLE_PAIRING_INFO:
        writeBlePairingInfo();
        break;
    case ATTRIBUTE_ID::REBOOT_COUNTER:
        setRebootCounter(static_cast<uint8_t>(data.value));
        break;
    default:
        ESP_LOGV(TAG, "WRONG MAP KEY!, 0x%lx", (uint32_t)data.id);
    }
}

/**
 * @brief Read data from flash
 * @param attributeId from API
 * @return uint64_t the stored value
 */
uint64_t Driver::readData(ATTRIBUTE_ID attributeId)
{
    uint16_t value = FLASH_DEFAULT_VALUE_U16;
    if (attributeId == ATTRIBUTE_ID::REBOOT_COUNTER) {
        value = getRebootCounter();
    } else {
        ESP_LOGE(TAG, "WRONG MAP KEY!");
    }
    return value;
}

/**
 * @brief Check if the memory has been erased
 * @return True if flash comes from factory reset, False if flash has been used
 */
bool Driver::isErased() const
{
    uint16_t value = FLASH_DEFAULT_VALUE_U16;
    nvs_get_u16(_currentHandle, eraseKey, &value);
    return (value != 0);
}

/**
 * @brief Set the erase flag to 0
 */
void Driver::unsetErase() const
{
    uint16_t value = 0;
    nvs_set_u16(_currentHandle, eraseKey, value);
    ESP_LOGI(TAG, "ERASED FLAG UNSET");
}

/**
 * @brief Erase the memory and set the erase flag to 1
 */
void Driver::erase() const
{
    nvs_erase_all(_currentHandle);
    nvs_commit(_currentHandle);
    nvs_erase_all(_bleHandle);
    nvs_commit(_bleHandle);
}

/**
 * @brief Retrieve blePairingInfo structure from flash
 */
void Driver::readBlePairingInfo() const
{
    esp_err_t errorCode;
    blePairingInfo_t blePairingInfo {};
    size_t size = sizeof(blePairingInfo_t);
    errorCode = nvs_get_blob(_bleHandle, blePairingInfoKey, &blePairingInfo, &size);
    if (errorCode == ESP_OK) {
        BasicBle::getInstance()->setBlePairingInfo(&blePairingInfo);
    } else {
        ESP_LOGE(TAG, "BlePairingInfo structure doesn't exist in NVS, error = %d", errorCode);
    }
}

/**
 * @brief Store blePairingInfo structure in flash
 */
void Driver::writeBlePairingInfo() const
{
    blePairingInfo_t blePairingInfo = BasicBle::getInstance()->getBlePairingInfo();
    nvs_set_blob(_bleHandle, blePairingInfoKey, &blePairingInfo, sizeof(blePairingInfo_t));
}

/**
 * @brief Get reboot counter
 * @return uint8_t the number of reboots (0xFF if not set)
 */
uint8_t Driver::getRebootCounter() const
{
    uint8_t value = FLASH_DEFAULT_VALUE_U8;
    nvs_get_u8(_currentHandle, rebootCounterKey, &value);
    ESP_LOGD(TAG, "REBOOT COUNTER Get (value = %d)", value);
    ESP_LOGW(TAG, "REBOOT COUNTER Get (value = %d)", value);
    return value;
}

/**
 * @brief Set reboot counter
 * @param value the number of reboots
 */
void Driver::setRebootCounter(uint8_t value) const
{
    nvs_set_u8(_currentHandle, rebootCounterKey, value);
    ESP_LOGD(TAG, "REBOOT COUNTER Set to %d", value);
    ESP_LOGW(TAG, "REBOOT COUNTER Set to %d", value);
}

} // namespace Flash