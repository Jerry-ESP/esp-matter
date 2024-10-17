/**
 * @file BlePairingChr.hpp
 * @author AWOX
 * @brief Handle the BLE pairing proccess (APP)
 *
 */

#include "Api.hpp"
#include "BleDataCommandChr.hpp"
#include "BleDataNotificationChr.hpp"
#include "BluetoothData.hpp"
#include "nimble/ble.h"
// undefine min and max to avoid conflicts between definition in nimble and std
#undef min
#undef max
#pragma once

// Opcodes used during onboarding/pairing
#define BLE_PAIRING_NONCE             0x0C /**< Opcode for nonce */
#define BLE_PAIRING_READ_DEVICE_TOKEN 0x0D /**< Opcode for read device toke */
#define BLE_PAIRING_MESH_NAME         0x04 /**< Opcode for mesh name */
#define BLE_PAIRING_MESH_PASSWORD     0x05 /**< Opcode for  mesh password*/
#define BLE_PAIRING_READ_SESSION_KEY  0x07 /**< Opcode for session key */
#define BLE_PAIRING_ERROR             0x0E /**< Opcode for error */

#define AES_128_KEY_SIZE              128 /**< Bit size for key in AES 128 */

/**
 * @enum pairingState_t
 * @brief Possible states in pairing process
 */
enum pairingState_t {
    PAIRING_STATE_INIT, /**< Init state*/
    PAIRING_STATE_NONCE_RECEIVED, /**< Nonce received state*/
    PAIRING_STATE_SESSION_KEY_READ, /**< Session key read state*/
    PAIRING_STATE_MESH_NAME_RECEIVED, /**< Mesh name received state*/
    PAIRING_STATE_MESH_PWD_RECEIVED, /**< Mesh password received state*/
    PAIRING_STATE_SESSION_READY /**< Sesion ready state*/
};

/*	-------------------------------------------------------------------------------	*/

#define SIZE_NONCE           8 /**< NONCE size */
#define SIZE_CIPHER          8 /**< Cipher size */
#define SIZE_NONCE_EXTENDED  16 /**< Extended NONCE size */
#define SIZE_CIPHER_EXTENDED 16 /**< Extended Cipher size */
#define SIZE_NAME_EXTENDED   16 /**< Extended Name size */
#define SIZE_DATA_ENCRYPT    16 /**< Size of data to encrypt/decrypt size */

/*	-------------------------------------------------------------------------------	*/

/**
 * @enum pairingError_t
 * @brief Possible error during pairing process
 */
enum pairingError_t {
    PAIR_ERROR_SUCCESS, /**< No error*/
    PAIR_ERROR_CRYPTO, /**< Crypto error*/
    PAIR_ERROR_WRONG_STATE, /**< Wrong state error*/
    PAIR_ERROR_UNKNOW_CMD, /**< Unknow command error*/
    PAIR_ERROR_UNMATCHING_NONCE, /**< Unmatch nonce error*/
    PAIR_ERROR_WRONG_PARAM_SIZE /**< Param size error*/
};

/*	-------------------------------------------------------------------------------	*/

#define BLE_PAIRING_CHR_SIZE 17 /**< The size of data received in the BLE pairing characteristic*/

/**
 * @brief Structure for commands received by BLE in the pairing characteristic
 *
 */
struct blePairingCommand_t {
    uint8_t packetInfo; /**< OpCodes*/
    union {
        struct {
            uint8_t pairingNonce[SIZE_NONCE]; /**< Nonce*/
            uint8_t cipher[SIZE_CIPHER]; /**< Cipher*/
        }; /**< Pairing Nonce command*/
        uint8_t pairingMeshName[SIZE_NAME_EXTENDED]; /**< Pairing Mesh Name command*/
        uint8_t pairingPassword[DEVICE_PASSWORD_SIZE]; /**< Pairing Password command*/
    }; /**< Data received*/
};

/**
 * @brief Class for the Pairing Process with the APP
 *
 */
class BlePairingChr {
public:
    // Members

    // Functions
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    BlePairingChr(const BlePairingChr& obj) = delete;
    static BlePairingChr* getInstance();
    void resetPairing();

    static int gattPairingChrAccessWrapper(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);
    uint8_t bleDecryptIncommingCommand(bleCommandInfo_t* theData, uint8_t size);
    bool blePairingEncryptNotification(bleNotifyInfo_t* theNotifyInfo);
    void saveBleNameAndPassword(void) const;

protected:
    // Members

    // Functions

private:
    // Members
    uint8_t _gattPairingChr[BLE_PAIRING_CHR_SIZE] = { 0 }; /**< gatt chr */
    uint8_t _gattPairingDscVal[4] = { 'P', 'a', 'i', 'r' }; /**< gatt descriptor */
    pairingState_t _pairingState; /**< pairing state */
    uint8_t _pairingNonce[SIZE_NONCE_EXTENDED]; /**< pairing nonce */
    uint8_t _sessionKey[SIZE_CIPHER_EXTENDED]; /**< session key */
    uint8_t _password[DEVICE_PASSWORD_SIZE]; /**< password */
    char _deviceName[DEVICE_NAME_SIZE]; /**< device name */
    uint8_t _pairingPassword[DEVICE_PASSWORD_SIZE]; /**< pairing password */

    // Functions
    BlePairingChr(/* args */); // Constructor
    int gattPairingChrAccess(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);
    void blePairingWrite(void* packet);
    void blePairingRead(uint8_t* packet);
    void blePairingHash(uint8_t aData[SIZE_CIPHER_EXTENDED]);
    void blePairingReadKey(uint8_t* packet);
    void aesEncryption(uint8_t key[SIZE_CIPHER_EXTENDED], uint8_t data[SIZE_DATA_ENCRYPT], uint8_t result[SIZE_DATA_ENCRYPT]);
    void aesDecryption(uint8_t key[SIZE_CIPHER_EXTENDED], uint8_t data[SIZE_DATA_ENCRYPT], uint8_t result[SIZE_DATA_ENCRYPT]);
    uint8_t blePairingCRC8(const uint8_t* data, uint8_t length);
};
