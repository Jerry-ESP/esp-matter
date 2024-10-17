/**
 * @file BleManager.h
 * @author AWOX
 * @brief Manager that handles the Bluetooth Low Energy Gap connection and entry point of our Bluetooth component
 *
 */

#include "BleGatt.hpp"
#include "platform/ESP32_custom/BleManager/Characteristics/BlePairingChr.hpp"
#include "TargetConfig/TargetConfig.hpp"
#include "Manager/VirtualManager/VirtualManager.hpp"

#pragma once

constexpr uint16_t VENDOR_ID = 0x0160; /**< Device Info (Vendor ID) */
constexpr uint8_t ZIGBEE_SUCCESS = 0; /**< Success status of a Zigbee command*/
constexpr uint16_t LL_PACKET_LENGTH = 251; /**< Max Link Layer packet size (with DLE)*/
constexpr uint16_t LL_PACKET_TIME = 2120; /**< Transmission time of Link Layer packet*/
constexpr uint16_t SCAN_RESP_DATA_RESERVED_SIZE = 4; /**< Reserved size of the scan response data*/
constexpr uint8_t MAC_ADDRESS_SIZE = 6; /**< Mac address size*/

/**
 * @brief Manufacturer data used in advertisement
 *
 */
struct manufacturerData_t {
    uint16_t vendorId = VENDOR_ID; /**< Vendor ID*/
    uint16_t productId = PRODUCT_ID; /**< Product ID*/
    uint8_t macAddress[MAC_ADDRESS_SIZE]; /**< Mac address (6 bytes)*/
} __attribute__((packed));

/**
 * @brief Scan response data used after a scan request by a remote device during advertisement
 *
 */
struct scanRespData_t {
    manufacturerData_t manufData; /**< @ref manufacturerData_t*/
    uint8_t firmwareVersionMajor = FIRMWARE_VERSION_MAJOR; /**< Firmware version major number*/
    uint8_t firmwareVersionMinor = FIRMWARE_VERSION_MINOR; /**< Firmware version minor number*/
    uint8_t firmwareVersionPatch = FIRMWARE_VERSION_PATCH; /**< Firmware version patch number*/
    uint16_t zigbeeAddress; /**< Zigbee Address of the device*/
    uint8_t reservedBulbstate[6];
    uint8_t hardwareVersionMajor = HARDWARE_VERSION_MAJOR; /**< Hardware version major number*/
    uint8_t hardwareVersionMinor = HARDWARE_VERSION_MINOR; /**< Hardware version minor number*/
    uint8_t zbProvisionedState; /**< Zigbee provisioned state(AWOX secure, AWOX remote, 3rd party)*/
    uint8_t rtcState; /**< RTC state*/
    uint8_t reserved[SCAN_RESP_DATA_RESERVED_SIZE]; /**< 4 bytes reserved (TELINK IS 6, BUT 2 ARE FOR FULL MAC ADDRESS)*/
} __attribute__((packed));

/**
 * @brief Class to manage the Gap Connection (manager)
 *
 */
class BleManager : public VirtualManager {
public:
    // Members

    // Functions
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    BleManager(const BleManager& obj) = delete;
    static BleManager* getInstance();

    BaseType_t createManagerTask(TaskHandle_t* handle);
    TaskHandle_t getTaskHandle();

    void updateBlePairingInfo(bool updateNameNeeded) const;
    void updateBleAdv() const;

    uint16_t getConnHandle(void) const;
    void getMacAddress(uint8_t* macAddr) const;

    static void bleGapOnSyncWrapper(void);

    // Functions
    /**
     * @brief Construct a new Ble Manager object (private: singleton)
     *
     */
    BleManager() { }
    /**
     * @brief Destroy the Ble Manager object (private: singleton)
     *
     */
    ~BleManager() final { }
    static void bleManagerWrapperTask(void* pvParameters);
    int bleGapEvent(struct ble_gap_event* event, const void* arg);
    void bleGapPrintConnDesc(const struct ble_gap_conn_desc* desc) const;
    void bleGapAdvertise(bool advertise) const;
    static void bleGapOnReset(int reason);
    void bleGapOnSync(void);
    static int bleGapEventWrapper(struct ble_gap_event* event, void* arg);

protected:
    // Members

    // Functions
    void managerTask();

private:
    // Members
    /**
     * @brief The class instance, the only one (singleton).
     *
     */
    uint16_t _connHandle = 0xFFFF; /**< BLE connection handle */
    uint8_t _ownAddrType; /**< Address Type */
    uint8_t _macAddr[6]; /**< BLE mac address */
    uint16_t _zbAddress = 0x0000; /**< Zigbee address */

    // static void bleGapOnSyncWrapper(void);
};
