/**
 * @file BleManager.cpp
 * @author AWOX
 * @brief Manager that handles the Bluetooth Low Energy Gap connection and entry point of our Bluetooth component
 *
 */

#include "Data/BluetoothData.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* BLE */
#include "platform/ESP32_custom/BleManager/Characteristics/BleFastOtaChr.hpp"
#include "BleManager.hpp"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "platform/ESP32_custom/BLEManagerImpl.h"

using namespace chip::DeviceLayer::Internal;

// extern struct ble_instance_cb_register ble_instance_cb[BLE_ADV_INSTANCES];

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Just a template for later use, the implementation is in the ble stack
 *
 */
void ble_store_config_init(void);
#ifdef __cplusplus
}
#endif

// constexpr static const char* TAG = "awox_ble_gap_manager"; /**< @brief Espressif tag for Log */

/********************* Define functions **************************/

/**
 * @brief Get the Instance object
 *
 * @return BleManager* the only instance
 */
BleManager* BleManager::getInstance()
{
    static BleManager _instance;
    return &_instance;
}

/**
 * @brief BLE Manager task
 *
 */
void BleManager::managerTask()
{
    int result;
    esp_err_t ret;

    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init nimble %d ", ret);
        return;
    }
    BleGatt* bleGatt = BleGatt::getInstance();
    bleGatt->initGattServices();

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleGapOnReset;
    ble_hs_cfg.sync_cb = BleManager::bleGapOnSyncWrapper;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
    ble_hs_cfg.sm_sc = 0;

    xSemaphoreTake(BasicBle::getInstance()->getSemaphoreFlashReadDone(), portMAX_DELAY);

    /* Set the initial device name */
    char name[DEVICE_NAME_SIZE + 1] = { 0 };
    blePairingInfo_t blePairingInfo = BasicBle::getInstance()->getBlePairingInfo();
    memcpy(name, blePairingInfo.name, 8);
    ESP_LOGI(TAG, "BLE name: %s ", name);
    result = ble_svc_gap_device_name_set(name);
    assert(result == BLE_ERR_SUCCESS);

    /* XXX Need to have template for store */
    ble_store_config_init();
    ESP_LOGI(TAG, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

/**
 * @brief We need this function because the task create requires a static task and that cause problem with cpp
 *
 * @param pvParameters
 */
void BleManager::bleManagerWrapperTask(void* pvParameters)
{
    ((BleManager*)pvParameters)->managerTask();
}

/**
 * @brief We need this function because the function ble_gap_adv_start() requires a static function to handle gap events and that causes problem with cpp
 *
 * @param[in] event The type of event being signalled
 * @param arg Unused
 * @return int 0 if the application successfully handled the event; nonzero on failure
 */
int BleManager::bleGapEventWrapper(struct ble_gap_event* event, void* arg)
{
    return BleManager::getInstance()->bleGapEvent(event, arg);
}

/**
 * @brief We need this function because the structure ble_hs_cgg requires a static function to handle gap On Sync event and that causes problem with cpp
 *
 */
void BleManager::bleGapOnSyncWrapper(void)
{
    return BleManager::getInstance()->bleGapOnSync();
}

/**
 * @brief Manager task creation
 *
 * @param[out] handle Task handle
 * @return BaseType_t pdTRUE if success; pdFALSE otherwise
 *
 */
BaseType_t BleManager::createManagerTask(TaskHandle_t* handle)
{
    BaseType_t baseType;// = xTaskCreate(BleManager::bleManagerWrapperTask, bleManagerTask.name, bleManagerTask.size, this, bleManagerTask.priority, handle);
    //_taskHandle = *handle;
    return baseType;
}

/**
 * @brief Get the task handle of BleManager
 *
 * @return TaskHandle_t
 */
TaskHandle_t BleManager::getTaskHandle()
{
    return _taskHandle;
}

/**
 * @brief Update BLE pairing information: Store in flash and update advertising data
 *
 * @param updateNameNeeded Determine if the name in ble advertismenet needs to be updated
 */
void BleManager::updateBlePairingInfo(bool updateNameNeeded) const
{
    QueueHandle_t flashQueue = Queue::getInstance()->getFlashQueue();
    attributeData_t data;
    data.id = ATTRIBUTE_ID::FLASH_BLE_PAIRING_INFO;
    INFORM_SEND_FAIL(xQueueSend(flashQueue, &data, AWOX_MAX_POSTING_DELAY))
    if (updateNameNeeded) {
        blePairingInfo_t blePairingInfo = BasicBle::getInstance()->getBlePairingInfo();
        char name[DEVICE_NAME_SIZE + 1] = { 0 };
        memcpy(name, blePairingInfo.name, DEVICE_NAME_SIZE);
        ble_svc_gap_device_name_set(name);
    }
    updateBleAdv();
}

/**
 * @brief Update advertising data if advertising ongoing
 *
 */
void BleManager::updateBleAdv() const
{
    if (ble_gap_ext_adv_active(1) != 0) {
        ble_gap_ext_adv_stop(1);
        bleGapAdvertise(true);
    }
}

/**
 * @brief The nimble host executes this callback when a GAP event occurs
 *
 * @param[in] event The type of event being signalled
 * @param arg unused
 * @return int 0 if the application successfully handled the event; nonzero on failure
 */
int BleManager::bleGapEvent(struct ble_gap_event* event, const void* arg)
{
    struct ble_gap_conn_desc desc;
    int result = BLE_ERR_SUCCESS;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == BLE_ERR_SUCCESS) {
            result = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(result == BLE_ERR_SUCCESS);
            _connHandle = event->connect.conn_handle;
            bleGapPrintConnDesc(&desc);
            /* Ask for a DLE exchange */
            result = ble_gap_set_data_len(event->connect.conn_handle, LL_PACKET_LENGTH, LL_PACKET_TIME);
            if (result != BLE_ERR_SUCCESS) {
                ESP_LOGE(TAG, "Set packet length failed; result = %d", result);
            }
            BlePairingChr::getInstance()->resetPairing();
        } else {
            ESP_LOGE(TAG, "Connection failed; status=%d ", event->connect.status);
            /* Connection failed; resume advertising. */
            bleGapAdvertise(true);
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT: {
        ESP_LOGI(TAG, "Disconnect; reason=%d ", event->disconnect.reason);
        bleGapPrintConnDesc(&event->disconnect.conn);
        _connHandle = 0xFFFF;
        /* Connection terminated; resume advertising. */
        bleGapAdvertise(true);
        break;
    }

    case BLE_GAP_EVENT_CONN_UPDATE: {
        /* The central has updated the connection parameters. */
        ESP_LOGI(TAG, "Connection updated; status=%d ", event->conn_update.status);
        result = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(result == BLE_ERR_SUCCESS);
        bleGapPrintConnDesc(&desc);
    } break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertise complete; reason=%d", event->adv_complete.reason);
        bleGapAdvertise(true);
        break;
    case BLE_GAP_EVENT_SUBSCRIBE: {
        ESP_LOGI(TAG, "Subscribe event; conn_handle=%d attr_handle=%d reason=%d prevn=%d curn=%d previ=%d curi=%d",
            event->subscribe.conn_handle,
            event->subscribe.attr_handle,
            event->subscribe.reason,
            event->subscribe.prev_notify,
            event->subscribe.cur_notify,
            event->subscribe.prev_indicate,
            event->subscribe.cur_indicate);
        break;
    }
// Useful only with logs on
#if CONFIG_LOG_DEFAULT_LEVEL
    case BLE_GAP_EVENT_NOTIFY_TX:
        ESP_LOGI(TAG, "Notify_tx event; conn_handle=%d attr_handle=%d status=%d is_indication=%d",
            event->notify_tx.conn_handle,
            event->notify_tx.attr_handle,
            event->notify_tx.status,
            event->notify_tx.indication);
        break;
    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "mtu update event; conn_handle=%d cid=%d mtu=%d",
            event->mtu.conn_handle,
            event->mtu.channel_id,
            event->mtu.value);
        break;

#endif
    default:
        break;
    }
    return result;
}

/**
 * @brief Callback called when the controller reset (after a catastrophic error occurs)
 *
 * @param[out] reason an integer indicating the reason for the controller reset
 */
void BleManager::bleGapOnReset(int reason)
{
    ESP_LOGE(TAG, "Resetting state; reason=%d", reason);
}

/**
 * @brief Callback called once host and controller are synched (beginning of application)
 *
 */
void BleManager::bleGapOnSync(void)
{
    int result;

    /* Set the max ATT_MTU */
    result = ble_att_set_preferred_mtu(BLE_MAX_ATT_MTU_SIZE);
    if (result != BLE_ERR_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set preferred MTU; result = %d", result);
    }

    /* Make sure we have proper identity address set (public) */
    result = ble_hs_util_ensure_addr(BLE_ADDR_PUBLIC);
    assert(result == BLE_ERR_SUCCESS);

    /* Figure out address to use while advertising (no privacy for now) */
    result = ble_hs_id_infer_auto(BLE_HCI_PRIVACY_NETWORK, &_ownAddrType);
    if (result != BLE_ERR_SUCCESS) {
        ESP_LOGE(TAG, "error determining address type; result=%d", result);
        return;
    }

    /* Printing ADDR */
    result = ble_hs_id_copy_addr(_ownAddrType, _macAddr, NULL);
    assert(result == BLE_ERR_SUCCESS);

    // TODO getInstance()->setMacAddr(_macAddr);
    ESP_LOGI(TAG, "Device MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
        _macAddr[5], _macAddr[4], _macAddr[3], _macAddr[2], _macAddr[1], _macAddr[0]);

    /* Begin advertising. */
    bleGapAdvertise(true);
}

// int BleManager::ble_multi_adv_set_addr(uint16_t instance)
// {
//     ble_addr_t addr;
//     int rc;

//     /* generate new non-resolvable private address */
//     rc = ble_hs_id_gen_rnd(1, &addr);
//     if (rc != 0) {
//         return rc;
//     }

//     /* Set generated address */
//     rc = ble_gap_ext_adv_set_addr(instance, &addr);
//     if (rc != 0) {
//         return rc;
//     }
//     memcpy(&ble_instance_cb[instance].addr, &addr, sizeof(addr));
//     return 0;
// }

// void BleManager::ble_multi_adv_conf_set_addr_test(uint16_t instance, struct ble_gap_ext_adv_params *params,
//                             uint8_t *pattern, int size_pattern, uint8_t *pattern_rsp, int size_pattern_rsp, int duration)
// {
//     struct os_mbuf *data;
//     struct os_mbuf *data_rsp;
//     int rc;

//     /* configure instance */
//     rc = ble_gap_ext_adv_configure(instance, params, NULL, bleGapEventWrapper, NULL);
//     assert (rc == 0);

//     rc = ble_multi_adv_set_addr(instance);
//     assert (rc == 0);

//     /* get mbuf for adv data */
//     data = os_msys_get_pkthdr(size_pattern, 0);
//     assert(data);

//     rc = ble_gap_ext_adv_set_data(instance, data);
//     assert (rc == 0);

//     /* get mbuf for adv rsp data */
//     data_rsp = os_msys_get_pkthdr(size_pattern_rsp, 0);
//     assert(data);

//     /* fill mbuf with adv rsp data */
//     rc = os_mbuf_append(data_rsp, pattern_rsp, size_pattern_rsp);
//     assert(rc == 0);

//     rc = ble_gap_ext_adv_rsp_set_data(instance, data_rsp);
//     assert (rc == 0);

//     /* start advertising */
//     rc = ble_gap_ext_adv_start(instance, duration, 0);
//     assert (rc == 0);

//     ESP_LOGI(TAG, "Instance %d started", instance);
// }


/**
 * @brief Enables advertising with the following parameters:
 *     - Limited discoverable mode
 *     - Undirected connectable mode
 *
 * @param advertise bool true: start advertising; false: stop advertising
 */
void BleManager::bleGapAdvertise(bool advertise) const
{
    BLEMgrImpl().AwoxBleAdvertising(advertise);
}

/**
 * @brief Logs information about a bluetooth connection
 *
 * @param desc Structure containing connexion parameters
 */
void BleManager::bleGapPrintConnDesc(const struct ble_gap_conn_desc* desc) const
{
    ESP_LOGI(TAG, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                  "encrypted=%d authenticated=%d bonded=%d",
        desc->conn_itvl, desc->conn_latency,
        desc->supervision_timeout,
        desc->sec_state.encrypted,
        desc->sec_state.authenticated,
        desc->sec_state.bonded);
}

/**
 * @brief Get the Connection Handle
 *
 * @return uint16_t connection handle
 */
uint16_t BleManager::getConnHandle(void) const
{
    return _connHandle;
}

/**
 * @brief Get the Mac Address
 *
 * @param[out] macAddr Mac address
 */
void BleManager::getMacAddress(uint8_t* macAddr) const
{
    memcpy(macAddr, _macAddr, FULL_MAC_ADDRESS_SIZE);
}