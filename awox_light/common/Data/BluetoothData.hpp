#include "Api.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#pragma once

namespace API {
/* BLE values */
constexpr uint8_t DEVICE_NAME_SIZE = 8; /**< Size of device name */
constexpr uint8_t DEVICE_PASSWORD_SIZE = 16; /**< Size of pairing password */
constexpr uint8_t BLE_STATE_PAIRED = 1; /**< The device is paired to a phone */
constexpr const char* BLE_DEFAULT_DEVICE_NAME = "unpaired"; /**< Default name, used in advertisement */
constexpr uint8_t FULL_MAC_ADDRESS_SIZE = 6; /**< Mac address size*/
constexpr uint8_t PARTIAL_MAC_ADDRESS_SIZE = 4; /**< Mac address size*/
constexpr uint8_t ZB_PROVISIONED_NONE = 0xA0; /**< Zigbee provisioned state (none) */
// clang-format off
#define BLE_DEFAULT_DEVICE_NAME_INIT        {'u', 'n', 'p', 'a', 'i', 'r', 'e', 'd'} /**< Default name, used to init blePairingInfo structure */
#define BLE_DEFAULT_DEVICE_PASSWORD_INIT    {'1', '2', '3', '4', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /**< Default password, used to init blePairingInfo structure */
// clang-format on
const uint8_t bleDefaultPassword[DEVICE_PASSWORD_SIZE] = BLE_DEFAULT_DEVICE_PASSWORD_INIT; /**< Default pairing password */

/**
 * @struct blePairingInfo_t
 * @brief Structure containing BLE pairing information (pairing state, zigbee provisioned mode, device name and password)
 *
 */
struct blePairingInfo_t {
    uint8_t blePairingState; /**< BLE pairing state (paired or unpaired)*/
    uint8_t zbProvisionedState; /**< Zigbee provisioned states (none, Awox, third party or Awox remote)*/
    uint8_t name[DEVICE_NAME_SIZE]; /**< Devide name used in BLE advertising*/
    uint8_t password[DEVICE_PASSWORD_SIZE]; /**< Device password used for pairing*/
};

/**@{*/
/** @brief BLE commands IDs */
enum class CMD_BLE_ID : uint8_t {
    KICKOUT = 0x01, /**< id of the kickout command*/
};
/**@}*/

/**
 * @brief Kickout command
 *
 */
struct kickoutInfo_t {
    uint16_t networkAddress; /**< The address of the device to kickout*/
};

/**
 * @brief Structure to receive BLE commands
 *
 */
struct bleCommandHandler_t {
    CMD_BLE_ID cmdId; /**< Command ID*/
    union {
        kickoutInfo_t kickoutInfo; /**<@ref kickoutInfo_t struct*/
    }; /**< Payload of the command*/
} __attribute__((packed));

/**
 * @struct bleGenericResult_t
 * @brief Generic command response
 */
struct bleGenericResult_t {
    uint16_t targetAddr; /**< The address of the targeted bulb with the initial command*/
    uint8_t result; /**< An ID indicating the status of the command. 0x00 is success 0x01 generic failure, others can be warnings or more specific fails*/
};

/**
 * @brief Notification result
 *
 */
enum class notificationResult_t : uint8_t {
    SUCCESS = 0x0, /**< Success code*/
    GENERAL_FAIL = 0x1, /**< General fail code*/
};

class BasicBle {
public:
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    BasicBle(const BasicBle& obj) = delete;
    static BasicBle* getInstance();
    blePairingInfo_t getBlePairingInfo() const;
    void setBlePairingInfo(blePairingInfo_t* blePairingInfo) const;
    void cleanBlePairingInfo() const;
    SemaphoreHandle_t getSemaphoreFlashReadDone() const;

private:
    static blePairingInfo_t _blePairingInfo; /**< BLE pairing information */
    /**
     * @brief Given by the Flash Driver when the first read is done
     * Ble Gap manager is waiting for it to start advertising
     *
     */
    static SemaphoreHandle_t _semaphoreFlashReadDone;
    BasicBle();
};

} // namespace API