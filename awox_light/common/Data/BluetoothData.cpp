#include "BluetoothData.hpp"
#include <cstring>

namespace API {

/**
 * @brief Init the object if it's null.
 *
 * @return BasicBle*
 */
BasicBle* BasicBle::getInstance()
{
    static BasicBle instance;
    return &instance;
}

blePairingInfo_t BasicBle::_blePairingInfo = {
    /* Initialize each field of the structure here */
    .blePairingState = 0,
    .zbProvisionedState = ZB_PROVISIONED_NONE,
    .name = BLE_DEFAULT_DEVICE_NAME_INIT,
    .password = BLE_DEFAULT_DEVICE_PASSWORD_INIT
};

SemaphoreHandle_t BasicBle::_semaphoreFlashReadDone = xSemaphoreCreateBinary();

/**
 * @brief Construct a new Basic Ble:: Basic Ble object
 *
 */
BasicBle::BasicBle()
{
}

/**
 * @brief Return ble pairing information structure
 *
 * @return blePairingInfo_t Structure containing pairing state, zigbee provisionned mode, device name and password
 */
blePairingInfo_t BasicBle::getBlePairingInfo() const
{
    return _blePairingInfo;
}

/**
 * @brief Copy the ble pairing information structure into the one stored in Api
 *
 * @param[in] blePairingInfo Structure containing pairing state, zigbee provisionned mode, device name and password
 */
void BasicBle::setBlePairingInfo(blePairingInfo_t* blePairingInfo) const
{
    memcpy(&_blePairingInfo, blePairingInfo, sizeof(blePairingInfo_t));
}

/**
 * @brief Clean ble pairing information structure to default values
 *
 */
void BasicBle::cleanBlePairingInfo() const
{
    memset(&_blePairingInfo, 0, sizeof(blePairingInfo_t));
    memcpy(_blePairingInfo.name, BLE_DEFAULT_DEVICE_NAME, DEVICE_NAME_SIZE);
    memcpy(_blePairingInfo.password, bleDefaultPassword, DEVICE_PASSWORD_SIZE);
    _blePairingInfo.zbProvisionedState = ZB_PROVISIONED_NONE;
}

/**
 * @brief Get the semaphore handle of Flash Read Done
 *
 * @return SemaphoreHandle_t
 */
SemaphoreHandle_t BasicBle::getSemaphoreFlashReadDone() const
{
    return _semaphoreFlashReadDone;
}

} // namespace API