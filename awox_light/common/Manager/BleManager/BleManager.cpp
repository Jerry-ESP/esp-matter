/**
 * @file BleManager.cpp
 * @author AWOX
 * @brief Manager that handles the Bluetooth Low Energy Gap connection and entry point of our Bluetooth component
 *
 */

#include "Api.hpp"
#include "BluetoothData.hpp"
#include "TargetConfig.hpp"
#include "TaskConfig.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* BLE */
#include "BleFastOtaChr.hpp"
#include "BleManager.hpp"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

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

constexpr static const char* TAG = "awox_ble_gap_manager"; /**< @brief Espressif tag for Log */

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
    ESP_LOGD(TAG, "BLE Host Task Started");
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
    BaseType_t baseType = xTaskCreate(BleManager::bleManagerWrapperTask, bleManagerTask.name, bleManagerTask.size, this, bleManagerTask.priority, handle);
    _taskHandle = *handle;
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
    if (ble_gap_adv_active() != 0) {
        ble_gap_adv_stop();
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
        ESP_LOGD(TAG, "Connection updated; status=%d ", event->conn_update.status);
        result = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(result == BLE_ERR_SUCCESS);
        bleGapPrintConnDesc(&desc);
    } break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGD(TAG, "Advertise complete; reason=%d", event->adv_complete.reason);
        bleGapAdvertise(true);
        break;
    case BLE_GAP_EVENT_SUBSCRIBE: {
        ESP_LOGD(TAG, "Subscribe event; conn_handle=%d attr_handle=%d reason=%d prevn=%d curn=%d previ=%d curi=%d",
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
        ESP_LOGD(TAG, "Notify_tx event; conn_handle=%d attr_handle=%d status=%d is_indication=%d",
            event->notify_tx.conn_handle,
            event->notify_tx.attr_handle,
            event->notify_tx.status,
            event->notify_tx.indication);
        break;
    case BLE_GAP_EVENT_MTU:
        ESP_LOGD(TAG, "mtu update event; conn_handle=%d cid=%d mtu=%d",
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

/**
 * @brief Enables advertising with the following parameters:
 *     - Limited discoverable mode
 *     - Undirected connectable mode
 *
 * @param advertise bool true: start advertising; false: stop advertising
 */
void BleManager::bleGapAdvertise(bool advertise) const
{
    if (advertise && (ble_gap_adv_active() == 0)) {
        ble_gap_adv_params advParams;
        ble_hs_adv_fields fields;
        const char* name;
        int result;

        /**
         *  Set the advertisement data included in our advertisements:
         *     o Flags (Limited discoverability and BLE-only (BR/EDR unsupported))
         *     o 16-bit service UUIDs
         *     o Device name
         *     o Manufacturer Data (Product ID + Mini MAC)
         */

        memset(&fields, 0, sizeof fields);

        fields.flags = BLE_HS_ADV_F_DISC_LTD | BLE_HS_ADV_F_BREDR_UNSUP;

        /* Set UUIDs list */
        ble_uuid16_t uuid16[1] = {
            {
             .u = {
                    .type = BLE_UUID_TYPE_16,
                },
             .value = GATT_SVR_GENERIC_ACCESS_SVC,
             }
        };
        fields.uuids16 = uuid16;
        fields.num_uuids16 = 1;
        fields.uuids16_is_complete = 0;

        /* Set complete name */
        name = ble_svc_gap_device_name();
        fields.name = (uint8_t*)name;
        fields.name_len = DEVICE_NAME_SIZE;
        fields.name_is_complete = 1;

        /* Set manufacturer data */
        fields.mfg_data_len = 8;
        auto manufData = manufacturerData_t();
        memcpy(manufData.macAddress, _macAddr, MAC_ADDRESS_SIZE);
        fields.mfg_data = (uint8_t*)&manufData;

        result = ble_gap_adv_set_fields(&fields);
        if (result != BLE_ERR_SUCCESS) {
            ESP_LOGE(TAG, "error setting advertisement data; result=%d", result);
            return;
        }

        /* Set Scan response data */
        memset(&fields, 0, sizeof fields);
        auto scanRespData = scanRespData_t();

        /* Set Manufacturer Data */
        memcpy(&scanRespData.manufData, &manufData, sizeof(manufacturerData_t));

        /* Set Zigbee Address */
        scanRespData.zigbeeAddress = _zbAddress;

        /* Set Provisioned State */
        scanRespData.zbProvisionedState = BasicBle::getInstance()->getBlePairingInfo().zbProvisionedState;

        scanRespData.rtcState = 0;

        fields.mfg_data_len = 29;
        fields.mfg_data = (uint8_t*)&scanRespData;
        result = ble_gap_adv_rsp_set_fields(&fields);
        if (result != BLE_ERR_SUCCESS) {
            ESP_LOGE(TAG, "error setting scan resp data; result=%d", result);
            return;
        }

        /* Begin advertising */
        memset(&advParams, 0, sizeof advParams);
        advParams.conn_mode = BLE_GAP_CONN_MODE_UND;
        advParams.disc_mode = BLE_GAP_DISC_MODE_LTD;
        advParams.itvl_min = BLE_GAP_ADV_ITVL_MS(300);
        advParams.itvl_max = BLE_GAP_ADV_ITVL_MS(305);

        result = ble_gap_adv_start(_ownAddrType, NULL, BLE_HS_FOREVER,
            &advParams, BleManager::bleGapEventWrapper, NULL);
        if (result != BLE_ERR_SUCCESS) {
            ESP_LOGE(TAG, "error enabling advertisement; result=%d", result);
            return;
        }
        ESP_LOGI(TAG, "ADVERTISE");
    } else if (!advertise) {
        ble_gap_adv_stop();
    } else {
        ESP_LOGI(TAG, "Advertisment already started");
    }
}

/**
 * @brief Logs information about a bluetooth connection
 *
 * @param desc Structure containing connexion parameters
 */
void BleManager::bleGapPrintConnDesc(const struct ble_gap_conn_desc* desc) const
{
    ESP_LOGD(TAG, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
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