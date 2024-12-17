/**
 * @file BleGatt.h
 * @author AWOX
 * @brief Manager for handle the Bluetooth Low Energy Gatt connection
 *
 */
#include "Data/Api.hpp"
#include "nimble/ble.h"
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ble_hs_cfg;
#ifdef __cplusplus
}
#endif

constexpr uint8_t BLE_ATT_HEADER = 3; /**< The size of the ATT header in an ATT packet*/
constexpr uint16_t BLE_ATTR_MAX_LENGTH = 512; /**< The maximum length of an attribute*/
constexpr uint16_t BLE_MAX_ATT_MTU_SIZE = BLE_ATTR_MAX_LENGTH + BLE_ATT_HEADER; /**< The maximum size of an ATT packet*/
constexpr uint8_t MASK_1_BIT_ = 0x01; /**< Mask to keep only 1 bit */
constexpr uint8_t MASK_8_BITS = 0xFF; /**< Mask to keep only 8 bits */
constexpr uint16_t MASK_16_BITS = 0xFFFF; /**< Mask to keep only 16 bits */

/** GATT server. */
#define GATT_SVR_GENERIC_ACCESS_SVC 0x1800

using namespace API;

/**
 * @brief Class to manage the Gatt Connection (manager)
 *
 */
class BleGatt {
public:
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    BleGatt(const BleGatt& obj) = delete;
    static BleGatt* getInstance();
    int gattSvrWrite(struct os_mbuf* om, uint16_t min_len, uint16_t max_len, void* dst, uint16_t* len);
    void initGattServices();

private:
    BleGatt();
    ~BleGatt();
    int gattSvrInit(void);
};
