/**
 * @file BleDataCommandChr.cpp
 * @author AWOX
 * @brief Class to handle the Command BLE characteristic
 *
 */

#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* BLE */
#include "BleDataCommandChr.hpp"
#include "platform/ESP32_custom/BleManager/BleGatt.hpp"
#include "platform/ESP32_custom/BleManager/BleManager.hpp"
#include "BlePairingChr.hpp"
#include "host/ble_hs.h"
#include "services/gatt/ble_svc_gatt.h"
#include "Product/product_util.h"

constexpr static const char* TAG = "awox_ble_data_command_chr"; /**< @brief Espressif tag for Log */

/**
 * @brief Constructe a new BleDataCommandChr
 *
 */
BleDataCommandChr::BleDataCommandChr(/* args */)
{
}

/**
 * @brief Get the Instance object
 *
 * @return BleDataCommandChr* the only instance
 */
BleDataCommandChr* BleDataCommandChr::getInstance()
{
    static BleDataCommandChr _instance = BleDataCommandChr();
    return &_instance;
}

/**
 * @brief Access callback whenever the characteristic/descriptor of command characteristic is read or written to
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
int BleDataCommandChr::gattDataCommandChrAccess(uint16_t connHandle, uint16_t attrHandle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
    int result = BLE_ATT_ERR_UNLIKELY;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "Characteristic read; connHandle=%d attrHandle=%d", connHandle, attrHandle);
        } else {
            ESP_LOGI(TAG, "Characteristic read by NimBLE stack; attrHandle=%d", attrHandle);
        }
        result = os_mbuf_append(ctxt->om, _gattDataCommandChr, sizeof(_gattDataCommandChr));
        if (result != BLE_ERR_SUCCESS) {
            result = BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "Characteristic write; connHandle=%d attrHandle=%d", connHandle, attrHandle);

        } else {
            ESP_LOGI(TAG, "Characteristic write by NimBLE stack; attrHandle=%d", attrHandle);
        }
        ESP_LOGI(TAG, "size %d", OS_MBUF_PKTLEN(ctxt->om));
        result = BleGatt::getInstance()->gattSvrWrite(ctxt->om, BLE_COMMAND_CHR_MIN_SIZE, BLE_COMMAND_CHR_SIZE, _gattDataCommandChr, NULL);

        if (result == BLE_ERR_SUCCESS) {
            bleDataCommandWrite(_gattDataCommandChr, OS_MBUF_PKTLEN(ctxt->om));
        } else {
            ESP_LOGE(TAG, "gattSvrWrite failed = %d", result);
        }
        break;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "Descriptor read; connHandle=%d attrHandle=%d", connHandle, attrHandle);
        } else {
            ESP_LOGI(TAG, "Descriptor read by NimBLE stack; attrHandle=%d", attrHandle);
        }
        result = os_mbuf_append(ctxt->om, &_gattDataCommandDscVal, sizeof(_gattDataCommandDscVal));
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
 * @brief We need this function because the structure ble_gatt_svc_def, defining services, requires a static function to
 * handle accesses of the command characteristic and that causes problem with cpp
 *
 * @param connHandle Connection handle
 * @param attrHandle Give the value handle of the attribute being accessed
 * @param ctxt ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor
 * @param arg unused
 * @return int Status
 */
int BleDataCommandChr::gattDataCommandChrAccessWrapper(uint16_t connHandle, uint16_t attrHandle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
    return BleDataCommandChr::getInstance()->gattDataCommandChrAccess(connHandle, attrHandle, ctxt, arg);
}

/**
 * @brief Manage data when a write is done in the DataCommand characteristic
 *
 * @param[in] packet Data written
 * @param[in] size Packet length
 *
 * @details In Zigbee section, data need to be retreated for fitting well with the received packet.
 * Some data send by the add are not sended in the right sense for being instantly usable.
 * It requires to change their position or to swap for being in Big Endian
 */
void BleDataCommandChr::bleDataCommandWrite(void* packet, uint8_t size) const
{
    ESP_LOGI(TAG, "RawCommand:");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, packet, size, ESP_LOG_DEBUG);

    // For alignment problem, the structure bleCommandInfo_t starts with a reserved byte. So we cast the structure to the byte just before packet
    bleCommandInfo_t* theData = reinterpret_cast<bleCommandInfo_t*>(((uint8_t*)(packet)) - 1); // -1 for reserved byte

    // Discard messages without payload or not decrypted correctly
    if ((size > BLE_MESSAGE_RECEIVED_BLE_PAYLOAD_START_INDEX) && (BlePairingChr::getInstance()->bleDecryptIncommingCommand(theData, size) == BLE_ERR_SUCCESS)) {
        if (theData->packetInfo == CMD_BLE) {
            ESP_LOGI(TAG, "COMMAND: This is a BLE command");
            bleHandlerCommand(&theData->ble, theData->payloadLength);
        } else {
            ESP_LOGE(TAG, "COMMAND: Unknown");
        }
    }
}

/**
 * @brief Manage BLE commands
 *
 * @param[in] command BLE command
 * @param[in] size Packet length
 */
void BleDataCommandChr::bleHandlerCommand(bleCommandHandler_t* command, uint8_t size) const
{
    ESP_LOGI(TAG, "COMMAND: Got BLE command 0x%X (length=%d)", static_cast<uint8_t>(command->cmdId), size);
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)command, size, ESP_LOG_DEBUG);

    switch (command->cmdId) {
    case CMD_BLE_ID::KICKOUT:
        kickout();
        break;
    default:
        ESP_LOGE(TAG, "BLE command not recognized, 0x%X", (uint8_t)command->cmdId);
        break;
    }
}

/**
 * @brief Handle kickout command(0x01)
 * @param networkAddress the target device
 */
void BleDataCommandChr::kickout(void) const
{
    ESP_LOGI(TAG, "KICKOUT");
    BleDataNotificationChr::getInstance()->bleNotificationSendGenericResult(
        CMD_BLE_ID::KICKOUT,
        0x0000,
        notificationResult_t::SUCCESS);
    // TODO: Add Matter factory reset
    product_factory_reset();
    QueueHandle_t flashQueue = Queue::getInstance()->getFlashQueue();
    attributeData_t data;
    data.id = ATTRIBUTE_ID::TOUCHLINK_FACTORY_RESET;
    INFORM_SEND_FAIL(xQueueSend(flashQueue, &data, AWOX_MAX_POSTING_DELAY))
}
