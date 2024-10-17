/**
 * @file BleDataNotificationChr.hpp
 * @author AWOX
 * @brief Class to handle the Notification BLE characteristic
 *
 */

#include "Api.hpp"
#include "BleGatt.hpp"
#include "BluetoothData.hpp"
#include "nimble/ble.h"

#pragma once

#define BLE_NOTIFICATION_SIZE 20 /**< The size of data send in the BLE notification characteristic*/

/**
 * @struct bleNotifyInfo_t
 * @brief Structure of notifications to send in the notification characteristic
 */
struct bleNotifyInfo_t {
    CMD_BLE_ID notificationType; /**< Notification command ID*/
    union {
        bleGenericResult_t genericResult; /**< @ref bleGenericResult_t struct*/
    } payload; /**< Data of the notification*/
} __attribute__((packed));

/**
 * @brief Class to handle the Notification BLE characteristic
 *
 */
class BleDataNotificationChr {
public:
    // Members
    static uint16_t _dataNotificationChrAttrHandle; /**< Attribute handle of the notification characteristic*/

    // Functions
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    BleDataNotificationChr(const BleDataNotificationChr& obj) = delete;
    static BleDataNotificationChr* getInstance();
    static int gattDataNotificationChrAccess(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);
    void bleNotificationSendGenericResult(CMD_BLE_ID notificationType, uint16_t destAddress, notificationResult_t result);

protected:
    // Members

    // Functions

private:
    // Members
    static uint8_t _gattDataNotificationChr[BLE_NOTIFICATION_SIZE]; /**< Notification characteristic value*/
    static uint8_t _gattDataNotificationDscVal[6]; /**< Notification characteristic descriptor value*/

    // Functions
    BleDataNotificationChr(/* args */); // Constructor
    void bleNotificationSend(bleNotifyInfo_t* theNotifyInfo);
};
