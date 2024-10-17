/**
 * @file BleDataCommandChr.hpp
 * @author AWOX
 * @brief Class to handle the Command BLE characteristic
 *
 */
#include "Api.hpp"
#include "BluetoothData.hpp"
#include "nimble/ble.h"
#pragma once
/**@{*/
/** Define the type of command with the byte packetInfo */
constexpr static const uint8_t CMD_ZIGBEE = 0;
constexpr static const uint8_t CMD_ZIGBEE_GROUP = 1;
constexpr static const uint8_t CMD_BLE = 2;
/**@}*/
constexpr static const uint8_t BLE_COMMAND_CHR_SIZE = 27; /**< Maximum size of a command*/
constexpr static const uint8_t BLE_COMMAND_CHR_MIN_SIZE = 17; /**< Minimum size of a command*/

constexpr static const uint8_t BLE_MESSAGE_RECEIVED_BLE_PAYLOAD_START_INDEX = 3; /**< Starting position of BLE payload of a received BLE message*/
constexpr static const uint8_t BLE_MESSAGE_RECEIVED_PAYLOAD_START_INDEX = 9; /**< Starting position of payload of a received BLE message*/

constexpr static const uint16_t BROADCAST_ADDR = 0xFFFF; /**< Broadcast Address in Zigbee*/

using namespace API;

/**
 * @struct bleCommandInfo_t
 * @brief Structure to parse commands comming from BLE
 */
struct bleCommandInfo_t {
    uint8_t reserved; /**< add this byte to fix memory alignment issue. set to rnd value at reception*/
    uint8_t packetInfo; /**< Used to determine the type of the command (BLE, Zigbee or Zigbee Group)*/
    uint8_t crc8; /**< Checksum*/
    uint8_t payloadLength; /**< Size of the payload*/
    union {
        bleCommandHandler_t ble; /**< @ref bleCommandHandler_t struct*/
    }; /**< Payload union*/
};

/**
 * @brief Class to handle the Command BLE characteristic
 *
 */
class BleDataCommandChr {
public:
    // Members

    // Functions
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    BleDataCommandChr(const BleDataCommandChr& obj) = delete;
    static BleDataCommandChr* getInstance();

    static int gattDataCommandChrAccessWrapper(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);

protected:
    // Members

    // Functions

private:
    // Members
    uint8_t _gattDataCommandChr[BLE_COMMAND_CHR_SIZE] = { 0 }; /**< Command characteristic value*/
    uint8_t _gattDataCommandDscVal[7] = { 'C', 'o', 'm', 'm', 'a', 'n', 'd' }; /**< Command characteristic descriptor value*/

    // Functions
    BleDataCommandChr(/* args */); // Constructor
    int gattDataCommandChrAccess(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);
    void bleDataCommandWrite(void* packet, uint8_t size) const;
    void bleHandlerCommand(bleCommandHandler_t* command, uint8_t size) const;
    void kickout(void) const;
};
