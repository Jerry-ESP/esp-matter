/**
 * @file BlePairingChr.cpp
 * @author AWOX
 * @brief Handle the BLE pairing proccess (APP)
 *
 */
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/aes.h"
#include <algorithm>

/* BLE */
#include "platform/ESP32_custom/BleManager/BleGatt.hpp"
#include "platform/ESP32_custom/BleManager/BleManager.hpp"
#include "BlePairingChr.hpp"
#include "host/ble_hs.h"
#include "services/gatt/ble_svc_gatt.h"

static const char* TAG = "awox_ble_pairing_chr"; /**< @brief Espressif tag for Log */

/**
 * @brief Constructe a new BlePairingChr
 *
 */
BlePairingChr::BlePairingChr(/* args */)
{
    memset(&_pairingNonce, 0, sizeof _pairingNonce);
    memset(&_sessionKey, 0, sizeof _sessionKey);
    memset(&_password, 0, sizeof _password);
    memset(&_deviceName, 0, sizeof _deviceName);
}

/**
 * @brief Get the Instance object
 *
 * @return BlePairingChr* the only instance
 */
BlePairingChr* BlePairingChr::getInstance()
{
    static BlePairingChr _instance;
    return &_instance;
}

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
int BlePairingChr::gattPairingChrAccess(uint16_t connHandle, uint16_t attrHandle,
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
        blePairingRead(_gattPairingChr);
        result = os_mbuf_append(ctxt->om, _gattPairingChr, sizeof(_gattPairingChr));
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
        result = BleGatt::getInstance()->gattSvrWrite(ctxt->om, sizeof(_gattPairingChr), sizeof(_gattPairingChr), _gattPairingChr, NULL);
        if (result == BLE_ERR_SUCCESS) {
            blePairingWrite(_gattPairingChr);
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
        result = os_mbuf_append(ctxt->om, &_gattPairingDscVal, sizeof(_gattPairingDscVal));
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
 * handle accesses of the pairing characteristic and that causes problem with cpp
 *
 * @param[in] connHandle Connection handle
 * @param[in] attrHandle Give the value handle of the attribute being accessed
 * @param[in] ctxt ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor
 * @param arg Unused
 * @return int Status
 */
int BlePairingChr::gattPairingChrAccessWrapper(uint16_t connHandle, uint16_t attrHandle,
    struct ble_gatt_access_ctxt* ctxt, void* arg)
{
    return BlePairingChr::getInstance()->gattPairingChrAccess(connHandle, attrHandle, ctxt, arg);
}

/**
 * @brief Manage data when a write is done in the pairing characteristic
 *
 * @param[in] packet Data written
 */
void BlePairingChr::blePairingWrite(void* packet)
{
    uint8_t theResult = PAIR_ERROR_SUCCESS;
    blePairingCommand_t* thePacket = (blePairingCommand_t*)packet;

    ESP_LOGI(TAG, "opCode=%d (pairing state = %d)", thePacket->packetInfo, _pairingState);

    switch (thePacket->packetInfo) {
    case BLE_PAIRING_NONCE:
        if (_pairingState != PAIRING_STATE_INIT) {
            theResult = PAIR_ERROR_WRONG_STATE;
        } else {
            /* Start of pairing */
            uint8_t theCipher[SIZE_CIPHER];
            uint8_t theDecryptedData[SIZE_CIPHER_EXTENDED];
            uint8_t theNamePasswordHash[SIZE_CIPHER_EXTENDED];

            ESP_LOGI(TAG, "Getting nonce");

            memcpy(_pairingNonce, thePacket->pairingNonce, SIZE_NONCE);

            memcpy(theCipher, thePacket->cipher, SIZE_CIPHER);

            blePairingHash(theNamePasswordHash);
            aesEncryption(_pairingNonce, theNamePasswordHash, theDecryptedData);

            /* Verifying the Nonce is well received */
            if (memcmp(theDecryptedData, theCipher, SIZE_CIPHER) != 0) {
                ESP_LOGI(TAG, "  NamePassHash");
                ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)theNamePasswordHash, SIZE_CIPHER_EXTENDED, ESP_LOG_DEBUG);
                ESP_LOGI(TAG, " DecryptedData");
                ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)theDecryptedData, SIZE_CIPHER_EXTENDED, ESP_LOG_DEBUG);
                ESP_LOGI(TAG, "       Cipher");
                ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)theCipher, SIZE_CIPHER, ESP_LOG_DEBUG);
                memset(_pairingNonce, 0, SIZE_NONCE_EXTENDED);
                theResult = PAIR_ERROR_UNMATCHING_NONCE;
            } else {
                _pairingState = PAIRING_STATE_NONCE_RECEIVED;

                ESP_LOGI(TAG, "Nonce received (pairing state = %d)", _pairingState);
            }
        }
        break;

    case BLE_PAIRING_MESH_NAME:
        if (_pairingState != PAIRING_STATE_SESSION_KEY_READ) {
            theResult = PAIR_ERROR_WRONG_STATE;
        } else {
            /* Decrypt the mesh name received */
            uint8_t thePairingName[SIZE_NAME_EXTENDED];

            aesDecryption(_sessionKey, thePacket->pairingMeshName, thePairingName);

#if CONFIG_LOG_DEFAULT_LEVEL
            char theName[DEVICE_NAME_SIZE + 1];
            memcpy(theName, thePairingName, DEVICE_NAME_SIZE);
            theName[DEVICE_NAME_SIZE] = 0;
            ESP_LOGI(TAG, "Pairing name received = %s", (uint8_t*)theName);
#endif
            memcpy(_deviceName, thePairingName, DEVICE_NAME_SIZE);
            _pairingState = PAIRING_STATE_MESH_NAME_RECEIVED;
        }
        break;

    case BLE_PAIRING_MESH_PASSWORD:
        if (_pairingState != PAIRING_STATE_MESH_NAME_RECEIVED) {
            theResult = PAIR_ERROR_WRONG_STATE;
        } else {
            /* Decrypt the mesh password received */
            aesDecryption(_sessionKey, thePacket->pairingPassword, _pairingPassword);
            ESP_LOGI(TAG, "Pairing password received");
            ESP_LOG_BUFFER_HEX_LEVEL(TAG, _pairingPassword, SIZE_CIPHER_EXTENDED, ESP_LOG_DEBUG);
            _pairingState = PAIRING_STATE_SESSION_KEY_READ;
        }
        break;

    default:
        theResult = PAIR_ERROR_UNKNOW_CMD;
        break;
    }

    if (theResult != PAIR_ERROR_SUCCESS) {
        ESP_LOGE(TAG, "PAIRING: Error %d !", theResult);
    }
}

/**
 * @brief Generate a hash from the pairing name and password
 *
 * @param[out] aData Hash generated, 16 bytes
 */
void BlePairingChr::blePairingHash(uint8_t aData[SIZE_CIPHER_EXTENDED])
{
    blePairingInfo_t blePairingInfo = BasicBle::getInstance()->getBlePairingInfo();
    const uint8_t* thePairingName = blePairingInfo.name;
    const uint8_t* thePairingPassword = blePairingInfo.password;

    printf("thePairingName: %s\n", thePairingName);
    printf("thePairingPassword: %s\n", thePairingPassword);

    if (memcmp(blePairingInfo.name, BLE_DEFAULT_DEVICE_NAME, DEVICE_NAME_SIZE) == 0) {
        thePairingPassword = bleDefaultPassword;
    }

    /* Get Hash */
    uint8_t theNetworkName[SIZE_CIPHER_EXTENDED];
    memset(theNetworkName, 0, SIZE_CIPHER_EXTENDED);
    memcpy(theNetworkName, thePairingName, DEVICE_NAME_SIZE);

    for (uint8_t i = 0; i < SIZE_CIPHER_EXTENDED; i++) {
        aData[i] = theNetworkName[i] ^ thePairingPassword[i];
    }
}

/**
 * @brief Manage data when a read is done in the pairing characteristic
 *
 * @param[out] packet Response for a read
 */
void BlePairingChr::blePairingRead(uint8_t* packet)
{
    if (_pairingState == PAIRING_STATE_NONCE_RECEIVED) {
        blePairingReadKey(packet);
        ESP_LOGI(TAG, "Session key computed, random data read by phone");
    } else if (_pairingState == PAIRING_STATE_SESSION_KEY_READ) {
        packet[0] = BLE_PAIRING_READ_SESSION_KEY;
        ESP_LOGI(TAG, "Confirming reception of key");
        saveBleNameAndPassword();
    } else {
        packet[0] = BLE_PAIRING_ERROR;
        ESP_LOGE(TAG, "Invalid pairing state at read (state = %d) !", _pairingState);
    }
}

/**
 * @brief Compute session key and response to a read in the pairing characteristic
 *
 * @param[out] packet Contains a key that can be encrypted to find the session key
 */
void BlePairingChr::blePairingReadKey(uint8_t* packet)
{
    uint8_t theData[SIZE_CIPHER_EXTENDED];
    uint8_t theKey[SIZE_CIPHER_EXTENDED];

    esp_fill_random(theKey, SIZE_CIPHER);

    // Send the first 8 bytes to the phone
    packet[0] = BLE_PAIRING_READ_DEVICE_TOKEN;
    memcpy(packet + 1, theKey, SIZE_CIPHER);
    ESP_LOGI(TAG, "  The Key");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)theKey, SIZE_CIPHER, ESP_LOG_DEBUG);
    // Calculate the session key
    blePairingHash(theData);

    memcpy(theKey + SIZE_NONCE, theKey, SIZE_CIPHER);
    memcpy(theKey, _pairingNonce, SIZE_NONCE);
    aesEncryption(theData, theKey, _sessionKey);

    ESP_LOGI(TAG, "Computed session key");
    ESP_LOGI(TAG, "  The Data");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)theData, SIZE_CIPHER_EXTENDED, ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "  Nonce");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)_pairingNonce, SIZE_CIPHER_EXTENDED, ESP_LOG_DEBUG);
    ESP_LOGI(TAG, " SessionKey");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, (uint8_t*)_sessionKey, SIZE_CIPHER_EXTENDED, ESP_LOG_DEBUG);

    _pairingState = PAIRING_STATE_SESSION_KEY_READ;
}

/**
 * @brief Reset pairing information to be able to start a new pairing
 *
 */
void BlePairingChr::resetPairing()
{
    _pairingState = PAIRING_STATE_INIT;
}

/**
 * @brief Function to encrypt data
 *
 * @param[in] key Encryption key
 * @param[in] data Data to encrypt
 * @param[out] result Data encrypted
 */
void BlePairingChr::aesEncryption(uint8_t key[SIZE_CIPHER_EXTENDED], uint8_t data[SIZE_DATA_ENCRYPT], uint8_t result[SIZE_DATA_ENCRYPT])
{
    uint8_t revKey[SIZE_CIPHER_EXTENDED];
    uint8_t revData[SIZE_DATA_ENCRYPT];
    memcpy(revKey, key, SIZE_CIPHER_EXTENDED);
    memcpy(revData, data, SIZE_DATA_ENCRYPT);
    std::reverse(revKey, revKey + SIZE_CIPHER_EXTENDED);
    std::reverse(revData, revData + SIZE_DATA_ENCRYPT);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char*)revKey, AES_128_KEY_SIZE);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)revData, result);
    std::reverse(result, result + SIZE_DATA_ENCRYPT);
    mbedtls_aes_free(&aes);
}

/**
 * @brief Function to decrypt data
 *
 * @param[in] key Decryption key
 * @param[in] data Data to decrypt
 * @param[out] result Data decrypted
 */
void BlePairingChr::aesDecryption(uint8_t key[SIZE_CIPHER_EXTENDED], uint8_t data[SIZE_DATA_ENCRYPT], uint8_t result[SIZE_DATA_ENCRYPT])
{
    uint8_t revKey[SIZE_CIPHER_EXTENDED];
    uint8_t revData[SIZE_DATA_ENCRYPT];
    memcpy(revKey, key, SIZE_CIPHER_EXTENDED);
    memcpy(revData, data, SIZE_DATA_ENCRYPT);
    std::reverse(revKey, revKey + SIZE_CIPHER_EXTENDED);
    std::reverse(revData, revData + SIZE_DATA_ENCRYPT);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, (const unsigned char*)revKey, AES_128_KEY_SIZE);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)revData, result);
    std::reverse(result, result + SIZE_DATA_ENCRYPT);
    mbedtls_aes_free(&aes);
}

/**
 * @brief Function to decrypt the messages send by the App to the BLE
 *
 * @param[in,out] theData Data to decrypt
 * @param[in] size Packet size
 * @return uint8_t result 0 = Success, others are error codes
 */
uint8_t BlePairingChr::bleDecryptIncommingCommand(bleCommandInfo_t* theData, uint8_t size)
{
    uint8_t result = BLE_ERR_NO_PAIRING;
    if (_pairingState == PAIRING_STATE_SESSION_KEY_READ) {
        uint8_t theDecryptedData[SIZE_DATA_ENCRYPT];
        uint8_t theCRC8;
        uint8_t theDataLength = size;
        if (theDataLength > SIZE_DATA_ENCRYPT) {
            theDataLength = SIZE_DATA_ENCRYPT;
        }
        // &TheData->crc8 is necessarily of size >= SIZE_DATA_ENCRYPT (16), the check is done by gattSvrWrite() in gattDataCommandChrAccess()
        // which verifies that the received command is at least BLE_COMMAND_CHR_MIN_SIZE 17 bytes (16 + 1 for packetInfo)
        // so we can use it as a buffer for decryption
        aesDecryption(_sessionKey, &theData->crc8, theDecryptedData);
        memcpy(&theData->crc8, theDecryptedData, theDataLength);
        ESP_LOGI(TAG, "Full decrypted data (payloadlength=%d) (datalength=%d):", size, theDataLength);
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, &theData->packetInfo, size, ESP_LOG_DEBUG);

        if (theData->payloadLength > size - 3) {
            ESP_LOGE(TAG, "Invalid payload length: %d > %d", theData->payloadLength, size - 3);
        } else {
            theCRC8 = blePairingCRC8(&theData->payloadLength, theData->payloadLength + 1);
            if (theCRC8 != theData->crc8) {
                ESP_LOGE(TAG, "SESSION: CRC8 is NOT OK, Computed=0x%X != Command=0x%X !", theCRC8, theData->crc8);
            } else {
                result = BLE_ERR_SUCCESS;
            }
        }
    } else {
        ESP_LOGE(TAG, "Invalid pairing state (state=%d) !", _pairingState);
    }
    return result;
}

/**
 * @brief Function to calculate the CRC8 of a buffer
 *
 * @param[in] data Buffer
 * @param[in] length Buffer length
 * @return uint8_t CRC8 value
 */
uint8_t BlePairingChr::blePairingCRC8(const uint8_t* data, uint8_t length)
{
    uint8_t crc = 0x00;
    uint8_t extract;
    uint8_t sum;

    for (uint32_t i = 0; i < length; i++) {
        extract = *data;
        for (uint32_t tempI = 8; tempI; tempI--) {
            sum = (crc ^ extract) & 0x01;
            crc >>= 1;
            if (sum)
                crc ^= 0x6C;
            extract >>= 1;
        }
        data++;
    }
    return crc;
}

/**
 * @brief Function to encrypt data (if dataLength > 16, then the last bytes are not encrypted)
 *
 * @param[in,out] theNotifyInfo Data to encrypt
 * @return bool Status 0 = fail wrong pairing state, 1 = success
 */
bool BlePairingChr::blePairingEncryptNotification(bleNotifyInfo_t* theNotifyInfo)
{
    if (_pairingState == PAIRING_STATE_SESSION_KEY_READ) {
        uint8_t theEncryptedData[SIZE_DATA_ENCRYPT];
        aesEncryption(_sessionKey, (uint8_t*)&theNotifyInfo->payload, theEncryptedData);
        memcpy(&theNotifyInfo->payload, theEncryptedData, SIZE_DATA_ENCRYPT);
    }
    return (_pairingState == PAIRING_STATE_SESSION_KEY_READ);
}

/**
 * @brief If provisioning completed, save and store the BLE name and Password sent during BLE provisioning
 *
 */
void BlePairingChr::saveBleNameAndPassword(void) const
{
    /* Mesh name and password received -> update ble pairing information and advertising data if needed */
    blePairingInfo_t blePairingInfo = BasicBle::getInstance()->getBlePairingInfo();
    blePairingInfo.blePairingState = BLE_STATE_PAIRED;
    memcpy(blePairingInfo.name, _deviceName, DEVICE_NAME_SIZE);
    memcpy(blePairingInfo.password, _pairingPassword, DEVICE_PASSWORD_SIZE);
    BasicBle::getInstance()->setBlePairingInfo(&blePairingInfo);
    /* Update the ble pairing info in flash and advertisement */
    BleManager::getInstance()->updateBlePairingInfo(true);
}