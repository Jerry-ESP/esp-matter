/**
 * @file BleFastOtaChr.h
 * @author AWOX
 * @brief Handle the BLE OTA process
 *
 */

#pragma once

#include "platform/ESP32_custom/BleManager/BleGatt.hpp"
#include "esp_ota_ops.h"
#include "nimble/ble.h"

constexpr uint8_t BLE_FAST_OTA_CHR_NAME_SIZE = 4; /**< The size of the name of the BLE Fast OTA characteristic*/
constexpr uint16_t BLE_MAX_FAST_OTA_WRITE_SIZE = BLE_ATTR_MAX_LENGTH; /**< The max size of data wrote in the BLE Fast OTA characteristic*/
constexpr uint16_t BLE_MIN_FAST_OTA_WRITE_SIZE = 4; /**< The min size of data wrote in the BLE Fast OTA characteristic (index 2 bytes + CRC 2 bytes)*/
constexpr uint8_t MAX_ERROR_COUNT = 3; /**< The max number of error allowed during the OTA process*/
constexpr uint16_t SECTOR_SIZE = 4096; /**< The size of a flash sector. This is the fw size unit in the ota protocol.*/
constexpr uint16_t FAST_OTA_CONNECTION_PARAMETER_INTERVAL_MIN = 12; /**< Interval min for updating ble connection when doing OTA. 12*1.25ms = 15ms*/
constexpr uint16_t FAST_OTA_CONNECTION_PARAMETER_INTERVAL_MAX = 12; /**< Interval max for updating ble connection when doing OTA. 12*1.25ms = 15ms*/
constexpr uint16_t FAST_OTA_CONNECTION_PARAMETER_LATENCY = 0; /**< Latency for updating ble connection when doing OTA. */
constexpr uint16_t FAST_OTA_CONNECTION_PARAMETER_TIMEOUT = 200; /**< Timeout for updating ble connection when doing OTA. 200 * 10ms = 2s */
constexpr uint16_t FAST_OTA_CONNECTION_PARAMETER_MIN_LENGTH_CONNECTION_EVENT = 0; /**< default value :https://docs.silabs.com/bluetooth/4.0/a00058  */
constexpr uint16_t FAST_OTA_CONNECTION_PARAMETER_MAX_LENGTH_CONNECTION_EVENT = 0xFFFF; /**< default value :https://docs.silabs.com/bluetooth/4.0/a00058  */
constexpr uint8_t SIZE_INDEX = 2; /**< The size of the index in the ota protocol. */
constexpr uint8_t SIZE_INTEGRITY_CHECK = 2; /**< The size of the integrity check in the ota protocol. */
constexpr uint16_t FAST_OTA_TIMEOUT_MS = 10'000; /**< The timeout of the OTA process. If no command is received during this time, the OTA process is reset. */
constexpr uint16_t FAST_OTA_FINISH_DELAY = 1'000; /**< The timeout of the OTA process. If no command is received during this time, the OTA process is reset. */

constexpr uint8_t numberOfBitsInByte = 8; /**< The number of bits in a byte. To fix a SQ Warning without changing Telink function*/

/**
 * @enum fastOtaState_t
 * @brief This enum contains all the states of the OTA process.
 */
enum class fastOtaState_t : uint8_t {
    FAST_FWUPDATE_STATE_IDLING = 0, /**< The fw ota hasn't started yet */
    FAST_FWUPDATE_STATE_ERASING = 1, /**< The device is erasing it's flash during the first phase of the ota */
    FAST_FWUPDATE_STATE_ERASED = 2, /**< The erase step has finished */
    FAST_FWUPDATE_STATE_STARTED = 3, /**< The Ota itself has begun */
    FAST_FWUPDATE_STATE_COMPLETED = 4 /**< The Ota is finished. */
};

/**
 * @enum fastOtaErrorCode_t
 * @brief This enum contains all the possible error codes of the OTA process.
 */
enum class fastOtaErrorCode_t : uint8_t {
    FAST_FWUPDATE_RESULT_SUCCESS = 0, /**< The success result */
    FAST_FWUPDATE_RESULT_CRC_FAILED = 1, /**< The crc is wrong (integrity check) */
    FAST_FWUPDATE_RESULT_MISSING_PART = 2, /**< An index of the ota is missing */
    FAST_FWUPDATE_RESULT_FW_TOO_BIG = 3, /**< The received fw is too big / error during erasing phase */
    FAST_FWUPDATE_RESULT_WRONG_STATE = 4, /**< The Ota is in the wrong state to receive a given command. */
    FAST_FWUPDATE_RESULT_WRITE_FAILED = 5 /**< The write in flash have failed. */
};

/**
 * @brief This structure is the payload of a Erase Packet sent by the app to the device.
 *
 */
struct __attribute__((packed)) otaErasePayload_t {
    uint16_t index; /**< Not used, data are random */
    uint8_t sizeInSectors; /**< Gives the number of free sectors that will be required to receive the new FW */
    uint8_t* fwChunk; /**< Not used, data are random */
    uint16_t integrityCheck; /**< Not used for backward compatibility with android.*/
    uint16_t fwChunkSize; /**< Not used. The size of the fw chunk. This is not part of the ble packet */
};
constexpr uint8_t offsetSizeErasePayload = 2; /**< The offset of the sizeInSectors in the otaErasePayload_t structure. */

/**
 * @brief This structure is the payload of a OTA packet giving a chunck of the fw.
 *
 */
struct __attribute__((packed)) otaFwChunkPayload_t {
    uint16_t index; /**< index of the received chunk of data */
    uint8_t* fwChunk; /**< This is a pointer to the raw binary of the fw chunk*/
    uint16_t integrityCheck; /**< 2 bytes to give the Mac Integrity check.  */
    uint16_t fwChunkSize; /**< The size of the fw chunk. This is not part of the ble packet */
};
constexpr uint8_t offsetIndexMsb = 1; /**< The offset of the msb of the index in the otaFwChunkPayload_t structure. */
constexpr uint8_t offsetIndexLsb = 0; /**< The offset of the lsb of the index in the otaFwChunkPayload_t structure. */
constexpr uint8_t offsetChunk = 2; /**< The offset of the chunk in the otaFwChunkPayload_t structure. */
constexpr uint8_t offsetIntegrityMsbEnd = 1; /**< The offset of the msb of the integrity check in the otaFwChunkPayload_t structure. */
constexpr uint8_t offsetIntegrityLsbEnd = 2; /**< The offset of the msb of the integrity check in the otaFwChunkPayload_t structure. */

/**
 * @brief This structure is the payload of a Read Packet sent by the app to the device.
 *
 */
struct __attribute__((packed)) otaReadPayload_t {
    fastOtaState_t fastOtaState; /**< Current state of the device */
    fastOtaErrorCode_t fastOtaErrorCode; /**< Error code of the latest action*/
    uint16_t latestIndexWritten; /**< index of the lastest successfully written chunk of fw */
    uint16_t latestIndexReceived; /**< index of the lastest received chunk of fw */
    uint8_t errorCount; /**< Number of errors during the ota process */
    uint8_t erasePercent; /**< Percentage of the flash erased */
    uint8_t debug; /**< Debug value */
};

/**
 * @brief Class for the OTA Process with the APP
 *
 */
class BleFastOtaChr {
public:
    // Members

    // Functions
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    BleFastOtaChr(const BleFastOtaChr& obj) = delete;
    static int gattFastOtaChrAccess(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);

private:
    // Members
    static otaReadPayload_t _gattFastOtaChr; /**< gatt chr */
    static uint8_t _gattFastOtaWriteData[BLE_MAX_FAST_OTA_WRITE_SIZE]; /**< gatt chr write */
    static uint8_t _gattFastOtaDscVal[BLE_FAST_OTA_CHR_NAME_SIZE]; /**< gatt descriptor */

    static esp_ota_handle_t _espOtaHandle; /**< The handle of the OTA process. */
    static fastOtaState_t _fastOtaState; /**< The state of the OTA process. By default it is in idle state*/
    static fastOtaErrorCode_t _fastOtaErrorCode; /**< Store the current error code to write in a read command */
    static uint16_t _otaSuccessIndex; /**< The index of the received chunk of data. By default it is 0*/
    static uint16_t _otaReceivedIndex; /**< The index of the received chunk of data. By default it is 0*/
    static uint16_t _sizeOfChunk; /**< The expected size of a received chunk of fw. It is initialized by the first "erase" command*/
    static uint8_t _otaErrorCount; /**< The number of errors during the ota process.*/
    static uint8_t numberOfSectors; /**< The number of sectors required for the OTA.*/
    static TimerHandle_t timerOta; /**< The timer used to reset the OTA process if no command is received during the OTA process.*/

    // Functions
    BleFastOtaChr(/* args */) = delete; /**< Deleting default constructor */
    static int gattAccessReadChr(uint16_t connHandle, uint16_t attrHandle, os_mbuf* om);
    static int gattAccessWriteChr(uint16_t connHandle, uint16_t attrHandle, os_mbuf* om);
    static void bleFastOtaRead(void);
    static void bleFastOtaWrite(uint16_t lengthWritten);
    static void handleCommandIdle(uint16_t lengthWritten);
    static void handleCommandErasing();
    static void handleCommandErased(uint16_t lengthWritten);
    static void handleCommandStarted(uint16_t lengthWritten);
    static void handleCommandCompleted();
    static void handleOtaError(fastOtaErrorCode_t errorCode);
    static uint32_t updateConnectionParameterForOta();
    static void resetOtaState();
    static uint16_t awTelinkCRC16(const uint8_t* aData, uint16_t aSize);
    static void otaTimerCallback(TimerHandle_t xTimer);
};
