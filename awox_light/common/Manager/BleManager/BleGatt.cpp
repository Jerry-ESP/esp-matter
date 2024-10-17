/**
 * @file BleGatt.cpp
 * @author AWOX
 * @brief Manager for handle the Bluetooth Low Energy Gatt connection
 *
 */

#include "Api.hpp"
#include "TaskConfig.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* BLE */
#include "BleDataCommandChr.hpp"
#include "BleDataNotificationChr.hpp"
#include "BleFastOtaChr.hpp"
#include "BleManager.hpp"
#include "BlePairingChr.hpp"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

constexpr static const char* TAG = "awox_ble_gatt_manager"; /**< @brief Espressif tag for Log */

/** Service */
const ble_uuid128_t gattSvrSvcUuid
    = BLE_UUID128_INIT(0x10, 0x19, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00);

/** Pairing characteristic */
const ble_uuid128_t gattPairingChrUuid
    = BLE_UUID128_INIT(0x14, 0x19, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00);

/** Pairing descriptor */
const ble_uuid16_t gattPairingDscUuid = BLE_UUID16_INIT(0x2901);

/** Data Client to Server characteristic */
const ble_uuid128_t gattDataCommandChrUuid
    = BLE_UUID128_INIT(0x12, 0x19, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00);

/** Data Client to Server descriptor */
const ble_uuid16_t gattDataCommandDscUuid = BLE_UUID16_INIT(0x2901);

/** Data Notification (Server to Client) characteristic */
const ble_uuid128_t gattDataNotificationChrUuid
    = BLE_UUID128_INIT(0x11, 0x19, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00);

/** Data Notification (Server to Client) descriptor */
const ble_uuid16_t gattDataNotificationDscUuid = BLE_UUID16_INIT(0x2901);

/** Fast OTA characteristic */
const ble_uuid128_t gattFastOtaChrUuid
    = BLE_UUID128_INIT(0x15, 0x19, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00);

/** Fast OTA descriptor */
const ble_uuid16_t gattFastOtaDscUuid = BLE_UUID16_INIT(0x2901);

/* Warning ignored for structure fields not initialized */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
/**
 * @brief Gatt Service structure
 *
 */
const struct ble_gatt_svc_def gattSvrSvcs[] = {
    {
     /*** Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &gattSvrSvcUuid.u,
     .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &gattPairingChrUuid.u,
                .access_cb = BlePairingChr::gattPairingChrAccessWrapper,
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    {
                        .uuid = &gattPairingDscUuid.u,
                        .att_flags = BLE_ATT_F_READ,
                        .access_cb = BlePairingChr::gattPairingChrAccessWrapper,
                    },
                    {
                        nullptr, /* No more descriptors in this characteristic */
                    } },
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .val_handle = nullptr,
            },
            {
                .uuid = &gattDataCommandChrUuid.u,
                .access_cb = BleDataCommandChr::gattDataCommandChrAccessWrapper,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                                                                 .uuid = &gattDataCommandDscUuid.u,
                                                                 .att_flags = BLE_ATT_F_READ,
                                                                 .access_cb = BleDataCommandChr::gattDataCommandChrAccessWrapper,
                                                             },
                    {
                        nullptr, /* No more descriptors in this characteristic */
                    } },
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                .val_handle = nullptr,
            },
            {
                .uuid = &gattDataNotificationChrUuid.u,
                .access_cb = BleDataNotificationChr::gattDataNotificationChrAccess,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                                                                 .uuid = &gattDataNotificationDscUuid.u,
                                                                 .att_flags = BLE_ATT_F_READ,
                                                                 .access_cb = BleDataNotificationChr::gattDataNotificationChrAccess,
                                                             },
                    {
                        nullptr, /* No more descriptors in this characteristic */
                    } },
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &BleDataNotificationChr::_dataNotificationChrAttrHandle,
            },
            {
                .uuid = &gattFastOtaChrUuid.u,
                .access_cb = BleFastOtaChr::gattFastOtaChrAccess,
                .descriptors = (struct ble_gatt_dsc_def[]) { {
                                                                 .uuid = &gattFastOtaDscUuid.u,
                                                                 .att_flags = BLE_ATT_F_READ,
                                                                 .access_cb = BleFastOtaChr::gattFastOtaChrAccess,
                                                             },
                    {
                        nullptr, /* No more descriptors in this characteristic */
                    } },
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
                .val_handle = nullptr,
            },
            {
                nullptr, /* No more characteristics in this service. */
            } },
     },
    {
     0, /* No more services. */
    },
};
#pragma GCC diagnostic pop

/********************* Define functions **************************/

/**
 * @brief Constructe a new BleGatt
 *
 */
BleGatt::BleGatt()
{
}

/**
 * @brief Destroy the Ble Gatt Manager:: Ble Gatt Manager object (never used)
 *
 */
BleGatt::~BleGatt()
{
}

/**
 * @brief Get the Instance object
 *
 * @return BleGatt* the only instance
 */
BleGatt* BleGatt::getInstance()
{
    static BleGatt _instance;
    return &_instance;
}

/**
 * @brief Write data to an attribute
 *
 * @param[in] om Data
 * @param[in] min_len Min length
 * @param[in] max_len Max length
 * @param[in] dst Which attribute value is being written
 * @param[out] len Length writen
 * @return int Status
 */
int BleGatt::gattSvrWrite(struct os_mbuf* om, uint16_t min_len, uint16_t max_len,
    void* dst, uint16_t* len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        ESP_LOGE(TAG, "Invalid length.");
        ESP_LOGE(TAG, "om_len = %d", om_len);
        ESP_LOGE(TAG, "min_len = %d", min_len);
        ESP_LOGE(TAG, "max_len = %d", max_len);
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

/**
 * @brief Initialize gatt services
 *
 * @return int Status
 */
int BleGatt::gattSvrInit(void)
{
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gattSvrSvcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gattSvrSvcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

/**
 * @brief Initialization for the gatt services
 *
 * @details Used to be in the init part of the manager task but is is better initialize it before the task creation.
 */
void BleGatt::initGattServices()
{
    int rc;
    rc = gattSvrInit();
    assert(rc == 0);
}
