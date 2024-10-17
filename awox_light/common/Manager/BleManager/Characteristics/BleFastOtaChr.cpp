/**
 * @file BleFastOtaChr.cpp
 * @author AWOX
 * @brief Handle the BLE OTA process
 *
 */

// Link to documentation on fast OTA: https://awox365.sharepoint.com/:w:/s/Awox/EUsa-SoO3G1Fi2ienGsNDfsBC_T9OOyz_KB_pX7A0whDOQ?e=2WxsUN

#include "esp_log.h"

/* BLE */
#include "BleFastOtaChr.hpp"
#include "BleGatt.hpp"
#include "BleManager.hpp"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "host/ble_hs.h"
#include <cstddef>
/**
 * @brief Use the std namespace for bytes
 *
 */
using namespace std;

static const char* TAG = "awox_ble_fast_ota_chr"; /**< @brief Espressif tag for Log */

// Definition of the static members of the class
fastOtaState_t BleFastOtaChr::_fastOtaState = fastOtaState_t::FAST_FWUPDATE_STATE_IDLING;
fastOtaErrorCode_t BleFastOtaChr::_fastOtaErrorCode = fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS;
uint16_t BleFastOtaChr::_otaSuccessIndex = 0;
uint16_t BleFastOtaChr::_otaReceivedIndex = 0;
uint16_t BleFastOtaChr::_sizeOfChunk = 0;
uint8_t BleFastOtaChr::_otaErrorCount = 0;
esp_ota_handle_t BleFastOtaChr::_espOtaHandle = { 0 };
uint8_t BleFastOtaChr::numberOfSectors;
TimerHandle_t BleFastOtaChr::timerOta;

otaReadPayload_t BleFastOtaChr::_gattFastOtaChr;
uint8_t BleFastOtaChr::_gattFastOtaWriteData[BLE_MAX_FAST_OTA_WRITE_SIZE] = { 0 };
uint8_t BleFastOtaChr::_gattFastOtaDscVal[BLE_FAST_OTA_CHR_NAME_SIZE] = { 'F', 'O', 'T', 'A' };

/**
 * @brief Access callback whenever a characteristic/descriptor is read or written to
 *
 * @param[in] connHandle Connection handle
 * @param[in] attrHandle Give the value handle of the attribute being accessed
 * @param[in] ctxt ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor
 * ctxt->dsc->uuid tells which characteristic/descriptor is accessed
 * Accordingly do:
 *     Append the value to ctxt->om if the operation is READ
 *     Write ctxt->om to the value if the operation is WRITE
 * @param arg Unused
 * @return int Status 0 = success, others are specific fails
 */
int BleFastOtaChr::gattFastOtaChrAccess(uint16_t connHandle, uint16_t attrHandle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
    int result = BLE_ATT_ERR_UNLIKELY;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        result = gattAccessReadChr(connHandle, attrHandle, ctxt->om);
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        result = gattAccessWriteChr(connHandle, attrHandle, ctxt->om);
        break;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        result = os_mbuf_append(ctxt->om, &_gattFastOtaDscVal, sizeof(_gattFastOtaDscVal));
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
 * @brief Operation done when there is a read request. Send a read response through om
 *
 * @param connHandle Connection handle
 * @param attrHandle Give the value handle of the attribute being accessed
 * @param om Append the value to om
 */
int BleFastOtaChr::gattAccessReadChr(uint16_t connHandle, uint16_t attrHandle, os_mbuf* om)
{
    int result = BLE_ATT_ERR_UNLIKELY;

    if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGD(TAG, "Characteristic read; connHandle=%d attrHandle=%d", connHandle, attrHandle);
    } else {
        ESP_LOGD(TAG, "Characteristic read by NimBLE stack; attrHandle=%d", attrHandle);
    }
    bleFastOtaRead();
    result = os_mbuf_append(om, (void*)&_gattFastOtaChr, sizeof(_gattFastOtaChr));
    if (result != BLE_ERR_SUCCESS) {
        result = BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return result;
}

/**
 * @brief Operation done when there is a write request. Copy data received to handle it
 *
 * @param connHandle Connection handle
 * @param attrHandle Give the value handle of the attribute being accessed
 * @param om The value written
 */
int BleFastOtaChr::gattAccessWriteChr(uint16_t connHandle, uint16_t attrHandle, os_mbuf* om)
{
    int result = BLE_ATT_ERR_UNLIKELY;
    uint16_t lengthWritten;

    if (connHandle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGD(TAG, "Characteristic write; connHandle=%d attrHandle=%d", connHandle, attrHandle);

    } else {
        ESP_LOGD(TAG, "Characteristic write by NimBLE stack; attrHandle=%d", attrHandle);
    }
    result = BleGatt::getInstance()->gattSvrWrite(om, BLE_MIN_FAST_OTA_WRITE_SIZE, BLE_MAX_FAST_OTA_WRITE_SIZE, _gattFastOtaWriteData, &lengthWritten);
    if (result == BLE_ERR_SUCCESS) {
        bleFastOtaWrite(lengthWritten);
    } else {
        ESP_LOGE(TAG, "gattSvrWrite failed = %X", result);
    }
    return result;
}

/**
 * @brief Manage data when a read is done in the Fast OTA characteristic
 * Called when a read request is received
 * Update the characteristic with the current state of the OTA process
 * If the OTA process is completed and successful, set the boot partition to the new firmware. This will reset the device.
 *
 */
void BleFastOtaChr::bleFastOtaRead(void)
{
    _gattFastOtaChr.fastOtaState = _fastOtaState;
    _gattFastOtaChr.fastOtaErrorCode = _fastOtaErrorCode;
    _gattFastOtaChr.latestIndexWritten = (uint16_t)(_otaSuccessIndex << 8 | _otaSuccessIndex >> 8);
    _gattFastOtaChr.latestIndexReceived = (uint16_t)(_otaReceivedIndex << 8 | _otaReceivedIndex >> 8);
    _gattFastOtaChr.errorCount = _otaErrorCount;
    _gattFastOtaChr.erasePercent = 0; // ErasePercent not implemented
    _gattFastOtaChr.debug = 0; // Debug not implemented

    ESP_LOGI(TAG, "Fast OTA read received. Current index = %d", _otaSuccessIndex);

    // If this is the laste read after ota completed, set the boot partition to the new firmware. This will reset the device.
    if (_fastOtaState == fastOtaState_t::FAST_FWUPDATE_STATE_COMPLETED && _fastOtaErrorCode == fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS) {
        const esp_partition_t* flashPartitionForOta = esp_ota_get_next_update_partition(nullptr);
        esp_err_t espError = esp_ota_set_boot_partition(flashPartitionForOta);
        if (espError != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_set_boot_partition failed. Error code : %d", espError);
        }
        xTimerStop(timerOta, 0);
        ESP_LOGI(TAG, "OTA process finished. Rebooting the device with a timer to ensure sending the read response to the app");
        timerOta = xTimerCreate("TimerOta", pdMS_TO_TICKS(FAST_OTA_FINISH_DELAY), pdFALSE, nullptr, otaTimerCallback);
        if (timerOta == nullptr) {
            ESP_LOGE(TAG, "xTimerCreate failed");
        }
        xTimerStart(timerOta, 0);
    }
}

/**
 * @brief Manage data when a write is done in the Fast OTA characteristic
 *
 * @param lengthWritten Number of bytes written to the characteristic
 */
void BleFastOtaChr::bleFastOtaWrite(uint16_t lengthWritten)
{
    ESP_LOGV(TAG, "bleFastOtaWrite");
    switch (_fastOtaState) {
    case fastOtaState_t::FAST_FWUPDATE_STATE_IDLING:
        handleCommandIdle(lengthWritten);
        break;
    case fastOtaState_t::FAST_FWUPDATE_STATE_ERASING:
        handleCommandErasing();
        break;
    case fastOtaState_t::FAST_FWUPDATE_STATE_ERASED:
        handleCommandErased(lengthWritten);
        break;
    case fastOtaState_t::FAST_FWUPDATE_STATE_STARTED:
        handleCommandStarted(lengthWritten);
        break;
    case fastOtaState_t::FAST_FWUPDATE_STATE_COMPLETED:
        handleCommandCompleted();
        break;
    default:
        ESP_LOGE(TAG, "bleFastOtaWrite should not happen");
        break;
    }
}

/**
 * @brief Handle the commands receivied while in idle state.
 * This is the first command of the OTA process.
 * Retrieve the number of sector to be erased.
 * Check if the size isn't too big. if so generate an error and return.
 * Start the erase process and go to erasing state.
 * Update slave latency parameter to 0
 * Update _sizeOfChunk corresponding to the size received in the first command.
 */
void BleFastOtaChr::handleCommandIdle(uint16_t lengthWritten)
{
    ESP_LOGI(TAG, "handleCommandIdle");
    otaErasePayload_t otaErasePayload;
    otaErasePayload.sizeInSectors = _gattFastOtaWriteData[offsetSizeErasePayload];
    numberOfSectors = otaErasePayload.sizeInSectors;

    ESP_LOGI(TAG, "Start erasing");
    _fastOtaState = fastOtaState_t::FAST_FWUPDATE_STATE_ERASING;
    _fastOtaErrorCode = fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS;

    const esp_partition_t* flashPartitionForOta = esp_ota_get_next_update_partition(nullptr);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html#over-the-air-updates-ota
    esp_err_t espErrorCode = esp_ota_begin(flashPartitionForOta, otaErasePayload.sizeInSectors * SECTOR_SIZE, &_espOtaHandle);
    if (espErrorCode != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed. Error code : %d", espErrorCode);
        ESP_LOGE(TAG, "_espOtaHandle : %p", (void*)_espOtaHandle);
        ESP_LOGE(TAG, "flashPartitionForOta : %p", (void*)flashPartitionForOta);
        ESP_LOGE(TAG, "otaErasePayload.sizeInSectors * SECTOR_SIZE : %d", (int)otaErasePayload.sizeInSectors * SECTOR_SIZE);
        if (flashPartitionForOta != nullptr) {
            ESP_LOGE(TAG, "flashPartitionForOta->size : %d", (int)flashPartitionForOta->size);
        }

        handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_FW_TOO_BIG);
        _fastOtaState = fastOtaState_t::FAST_FWUPDATE_STATE_IDLING;
        return;
    }

    ESP_LOGI(TAG, "Erase done");
    _fastOtaState = fastOtaState_t::FAST_FWUPDATE_STATE_ERASED;

    // Update slave latency parameter to 0
    if (updateConnectionParameterForOta() != 0) { // 0 means succes. No define in the stack.
        ESP_LOGW(TAG, "esp_ble_gap_update_params failed, OTA may be slow.");
    }

    // Store the size of the payload. All successive packet must have the same size.
    _sizeOfChunk = lengthWritten - SIZE_INDEX - SIZE_INTEGRITY_CHECK;
    handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS);
}

/**
 * @brief Handle the commands receivied while in erasing state.
 * This is the second step of the OTA process.
 * Update the error code to SUCCESS and update the error percentage to report. Not implemented.
 */
void BleFastOtaChr::handleCommandErasing()
{
    // Get and update the erasePercent not inmplemented
    ESP_LOGI(TAG, "handleCommandErasing");
    ESP_LOGE(TAG, "Not used by espressif");
    handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS);
}

/**
 * @brief Handle the commands receivied while in erased state.
 * The erased state is transitory. It is only used to know when the erase process is done and change as soon as possible we receive the first chunk of fw.
 * Update the state to started.
 * Call handleCommandStarted to handle the first packet.
 * @param lengthWritten Number of bytes written to the characteristic
 */
void BleFastOtaChr::handleCommandErased(uint16_t lengthWritten)
{
    ESP_LOGI(TAG, "handleCommandErased");

    // start a timer to detect inactivity during the OTA process
    timerOta = xTimerCreate("TimerOta", pdMS_TO_TICKS(FAST_OTA_TIMEOUT_MS), pdFALSE, nullptr, otaTimerCallback);
    if (timerOta == nullptr) {
        ESP_LOGE(TAG, "xTimerCreate failed");
    }

    _fastOtaState = fastOtaState_t::FAST_FWUPDATE_STATE_STARTED;
    handleCommandStarted(lengthWritten);
}

/**
 * @brief Handle the commands receivied while in started state.
 * This is the main state of the OTA process.
 * Check if this is the last packet. If so go to state completed.
 * Check the size of the packet. If it is not the same as the first packet, generate an error and return.
 * Check the integrity of the packet. If it is not the same as the expected one, generate an error and return.
 * Check the index of the packet. If it is not the expected one, generate an error and return.
 * Write the packet in flash.
 * Update _otaSuccessIndex and _otaReceivedIndex.
 *  @param lengthWritten Number of bytes written to the characteristic
 */
void BleFastOtaChr::handleCommandStarted(uint16_t lengthWritten)
{
    ESP_LOGV(TAG, "handleCommandStarted");
    xTimerReset(timerOta, pdMS_TO_TICKS(FAST_OTA_TIMEOUT_MS));

    otaFwChunkPayload_t otaFwChunkPayload;
    otaFwChunkPayload.index = (uint16_t)((uint16_t)byte(_gattFastOtaWriteData[offsetIndexMsb]) << 8 | (uint16_t)byte(_gattFastOtaWriteData[offsetIndexLsb]));
    otaFwChunkPayload.fwChunk = &_gattFastOtaWriteData[offsetChunk];
    otaFwChunkPayload.integrityCheck = (uint16_t)((uint16_t)byte(_gattFastOtaWriteData[lengthWritten - offsetIntegrityMsbEnd]) << 8
        | (uint16_t)byte(_gattFastOtaWriteData[lengthWritten - offsetIntegrityLsbEnd]));

    // If this is the last packet (only 2 bytes for index and 2 bytes for mac) go to state finish
    if (lengthWritten == SIZE_INDEX + SIZE_INTEGRITY_CHECK) {
        ESP_LOGI(TAG, "Last packet received");
        handleCommandCompleted();
        return;
    }

    // Check the size of the packet
    if (lengthWritten - SIZE_INDEX - SIZE_INTEGRITY_CHECK != _sizeOfChunk) {
        ESP_LOGE(TAG, "The received size of the payload is not the same as the first packet");
        ESP_LOGE(TAG, "sizeOfChunk received : %d", _sizeOfChunk);
        ESP_LOGE(TAG, "sizeOfChunk received : %d", lengthWritten - SIZE_INDEX - SIZE_INTEGRITY_CHECK);
        handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_WRITE_FAILED);
        return;
    }

    // Check the integrity of the packet. Integrity concerns the index and the fwChunk
    uint16_t calculatedIntegrityCheck = awTelinkCRC16(_gattFastOtaWriteData, lengthWritten - SIZE_INTEGRITY_CHECK);
    if (calculatedIntegrityCheck != otaFwChunkPayload.integrityCheck) {
        ESP_LOGE(TAG, "The received integrity check is not the expected one");
        ESP_LOGE(TAG, "Expected integrity check : %d", calculatedIntegrityCheck);
        ESP_LOGE(TAG, "Received integrity check : %d", otaFwChunkPayload.integrityCheck);
        handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_CRC_FAILED);
        return;
    }

    // Check the index of the packet
    _otaReceivedIndex = otaFwChunkPayload.index;
    if (((_otaReceivedIndex != _otaSuccessIndex + 1) || (_otaReceivedIndex * _sizeOfChunk > numberOfSectors * SECTOR_SIZE)) && (_otaSuccessIndex != 0)) {
        ESP_LOGE(TAG, "The received index is not the expected one");
        ESP_LOGE(TAG, "Expected index : %d", _otaSuccessIndex + 1);
        ESP_LOGE(TAG, "Received index : %d", otaFwChunkPayload.index);
        handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_MISSING_PART);
        return;
    }

    // Write the packet in flash
    esp_err_t espErrorCode = esp_ota_write(_espOtaHandle, otaFwChunkPayload.fwChunk, lengthWritten - 4);
    if (espErrorCode != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_write failed. Error code : %d", espErrorCode);
        handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_WRITE_FAILED);
        return;
    }
    _otaSuccessIndex = _otaReceivedIndex;
    handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS);
}

/**
 * @brief Handle the commands receivied while in completed state.
 * This is the last state of the OTA process.
 * Update the error code to SUCCESS
 * Set the boot partition to the new firmware.
 * Wait for the next read from the app to reset the device.
 */
void BleFastOtaChr::handleCommandCompleted()
{
    ESP_LOGI(TAG, "handleCommandCompleted");

    esp_err_t espError = esp_ota_end(_espOtaHandle);
    if (espError != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed. Error code : %d", espError);
        handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_WRITE_FAILED);
        return;
    }

    _fastOtaState = fastOtaState_t::FAST_FWUPDATE_STATE_COMPLETED;
    handleOtaError(fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS);
}

/**
 * @brief Handle the errors that can occur during the OTA process.
 * Update the error code.
 * Increment the error count if not success or fw too big (only during erasing state).
 * If the error count is too high, reset the OTA process.
 * @param errorCode The error code that occured
 */
void BleFastOtaChr::handleOtaError(fastOtaErrorCode_t errorCode)
{
    _fastOtaErrorCode = errorCode;

    switch (errorCode) {
    case fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_SUCCESS:
        ESP_LOGV(TAG, "handleOtaError. Success");
        break;
    case fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_CRC_FAILED:
        ESP_LOGE(TAG, "handleOtaError. CRC failed");
        _otaErrorCount++;
        break;
    case fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_MISSING_PART:
        ESP_LOGE(TAG, "handleOtaError. Missing part");
        _otaErrorCount++;
        break;
    case fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_FW_TOO_BIG:
        ESP_LOGE(TAG, "handleOtaError. FW too big");
        // No increment of _otaErrorCount because this is done during the erase phase that goes back to IDLE state in failure case.
        break;
    case fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_WRONG_STATE:
        ESP_LOGE(TAG, "handleOtaError. Wrong state");
        _otaErrorCount++;
        break;
    case fastOtaErrorCode_t::FAST_FWUPDATE_RESULT_WRITE_FAILED:
        ESP_LOGE(TAG, "handleOtaError. Write failed");
        _otaErrorCount++;
        break;
    default:
        ESP_LOGE(TAG, "Should not happen");
        break;
    }

    if (_otaErrorCount >= MAX_ERROR_COUNT) {
        ESP_LOGE(TAG, "handleOtaError. Too many errors during the OTA process. Aborting and reseting the OTA process");
        resetOtaState();
    }
}

/**
 * @brief Update connection parameter for the OTA process.
 *
 */
uint32_t BleFastOtaChr::updateConnectionParameterForOta()
{
    ESP_LOGI(TAG, "Updating connection parameters");
    ble_gap_upd_params connectionParameters;
    connectionParameters.itvl_min = FAST_OTA_CONNECTION_PARAMETER_INTERVAL_MIN;
    connectionParameters.itvl_max = FAST_OTA_CONNECTION_PARAMETER_INTERVAL_MAX;
    connectionParameters.latency = FAST_OTA_CONNECTION_PARAMETER_LATENCY;
    connectionParameters.supervision_timeout = FAST_OTA_CONNECTION_PARAMETER_TIMEOUT;
    connectionParameters.min_ce_len = FAST_OTA_CONNECTION_PARAMETER_MIN_LENGTH_CONNECTION_EVENT;
    connectionParameters.max_ce_len = FAST_OTA_CONNECTION_PARAMETER_MAX_LENGTH_CONNECTION_EVENT;

    const ble_gap_upd_params* pconnectionParameters = &connectionParameters;
    uint16_t connectionHandle = BleManager::getInstance()->getConnHandle();
    return (uint32_t)ble_gap_update_params(connectionHandle, pconnectionParameters);
}

/**
 * @brief Resets the Ota state to it's initial values by reboorting.
 * Also used when the OtA process is completed.
 *
 */
void __attribute__((noreturn)) BleFastOtaChr::resetOtaState()
{
    ESP_LOGI(TAG, "Resetting OTA state by rebooting");

    esp_ota_abort(_espOtaHandle);

    esp_restart();
}

/**
 * @brief Compute the CRC16 of the data received.
 *
 * @param aData The data to compute the CRC16
 * @param aSize The size of the data
 * @return uint16_t The CRC16
 */
uint16_t BleFastOtaChr::awTelinkCRC16(const uint8_t* aData, uint16_t aSize)
{
    static const uint16_t kPolynom[2] = { 0, 0xA001 };
    uint16_t theCRC16 = 0xFFFF;
    for (const uint8_t* theEnd = aData + aSize; aData < theEnd; aData++) {
        size_t theBitIdx;
        uint8_t theData;
        for (theBitIdx = 0, theData = *aData; theBitIdx < numberOfBitsInByte; theBitIdx++, theData >>= 1) {
            theCRC16 = (theCRC16 >> 1) ^ kPolynom[(theCRC16 ^ theData) & 1];
        }
    }
    return theCRC16;
}

/**
 * @brief Callback called when the OTA process timeout.
 * Reset the OTA process.
 *
 * @param xTimer The timer that called the callback
 */
void __attribute__((noreturn)) BleFastOtaChr::otaTimerCallback(TimerHandle_t)
{
    ESP_LOGI(TAG, "OTA process timeout. Resetting the OTA process");
    resetOtaState();
}
