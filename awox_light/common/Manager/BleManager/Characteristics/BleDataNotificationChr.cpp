/**
 * @file BleDataNotificationChr.cpp
 * @author AWOX
 * @brief Class to handle the Notification BLE characteristic
 *
 */

#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* BLE */
#include "BleDataCommandChr.hpp"
#include "BleDataNotificationChr.hpp"
#include "BleManager.hpp"
#include "BlePairingChr.hpp"
#include "host/ble_hs.h"
#include "services/gatt/ble_svc_gatt.h"

constexpr static const char* TAG = "awox_ble_data_notification_chr"; /**< @brief Espressif tag for Log */

uint16_t BleDataNotificationChr::_dataNotificationChrAttrHandle;

/**
 * @brief Constructe a new BleDataNotificationChr
 *
 */
BleDataNotificationChr::BleDataNotificationChr(/* args */)
{
}

/**
 * @brief Get the Instance object
 *
 * @return BleDataNotificationChr* the only instance
 */
BleDataNotificationChr* BleDataNotificationChr::getInstance()
{
    static BleDataNotificationChr _instance = BleDataNotificationChr();
    return &_instance;
}

uint8_t BleDataNotificationChr::_gattDataNotificationChr[BLE_NOTIFICATION_SIZE] = { 0 };
uint8_t BleDataNotificationChr::_gattDataNotificationDscVal[6] = { 'S', 't', 'a', 't', 'u', 's' };

/**
 * @brief Access callback whenever a characteristic/descriptor is read or written to
 *
 * @param connHandle Connection handle
 * @param attrHandle Give the value handle of the attribute being accessed
 * @param ctxt ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor
 * ctxt->dsc->uuid tells which characteristic/descriptor is accessed
 * Accordingly do:
 *     Append the value to ctxt->om if the operation is READ
 *     Write ctxt->om to the value if the operation is WRITE
 * @param arg unused
 * @return int Status 0 is success, others are error codes
 */
int BleDataNotificationChr::gattDataNotificationChrAccess(uint16_t connHandle, uint16_t attrHandle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
    int result = BLE_ATT_ERR_UNLIKELY;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGD(TAG, "Characteristic read; connHandle=%d attrHandle=%d", connHandle, attrHandle);
        } else {
            ESP_LOGD(TAG, "Characteristic read by NimBLE stack; attrHandle=%d", attrHandle);
        }
        result = os_mbuf_append(ctxt->om, _gattDataNotificationChr, sizeof(_gattDataNotificationChr));
        if (result != BLE_ERR_SUCCESS) {
            result = BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGD(TAG, "Characteristic write; connHandle=%d attrHandle=%d", connHandle, attrHandle);

        } else {
            ESP_LOGD(TAG, "Characteristic write by NimBLE stack; attrHandle=%d", attrHandle);
        }
        ESP_LOGD(TAG, "size %d", OS_MBUF_PKTLEN(ctxt->om));
        ESP_LOGE(TAG, "gattSvrWrite failed: Should not write data in the notification characteristic");
        // Do Nothing, data should not be written in this characteristic
        break;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGD(TAG, "Descriptor read; connHandle=%d attrHandle=%d", connHandle, attrHandle);
        } else {
            ESP_LOGD(TAG, "Descriptor read by NimBLE stack; attrHandle=%d", attrHandle);
        }
        result = os_mbuf_append(ctxt->om, &_gattDataNotificationDscVal, sizeof(_gattDataNotificationDscVal));
        if (result != BLE_ERR_SUCCESS) {
            result = BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        break;

    default:
        break;
    }
    return result;
}

/**
 * @brief Encrypt and send BLE notification
 *
 * @param[in] theNotifyInfo Data to send
 */
void BleDataNotificationChr::bleNotificationSend(bleNotifyInfo_t* theNotifyInfo)
{
    ESP_LOGD(TAG, "NOTIF: Before Encryption, data size %d:", BLE_NOTIFICATION_SIZE);
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)theNotifyInfo, BLE_NOTIFICATION_SIZE, ESP_LOG_DEBUG);
    if (BlePairingChr::getInstance()->blePairingEncryptNotification(theNotifyInfo)) {
        memcpy(_gattDataNotificationChr, theNotifyInfo, BLE_NOTIFICATION_SIZE);
        ble_gatts_notify(BleManager::getInstance()->getConnHandle(), _dataNotificationChrAttrHandle);
    } else {
        ESP_LOGE(TAG, "Cannot push notification, no session established");
    }
}

/**
 * @brief Form and send notification for generic result
 *
 * @param[in] notificationType Command ID
 * @param[in] destAddress Zigbee Address answering
 * @param[in] result Status
 */
void BleDataNotificationChr::bleNotificationSendGenericResult(CMD_BLE_ID notificationType, uint16_t destAddress, notificationResult_t result)
{
    bleNotifyInfo_t theInfo;

    memset(&theInfo, 0, sizeof(bleNotifyInfo_t));
    theInfo.notificationType = notificationType;
    theInfo.payload.genericResult.result = (uint8_t)result;
    theInfo.payload.genericResult.targetAddr = destAddress;

    bleNotificationSend(&theInfo);
}
