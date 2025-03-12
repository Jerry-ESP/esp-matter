// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <esp_check.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_core.h>
#include <nvs.h>

#include <app/server/Dnssd.h>
#ifdef CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/server/Server.h>
#include <esp_matter_ota.h>
#include <esp_matter_nvs.h>
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
#include <esp_matter_data_model_priv.h>
#endif // CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
#endif // CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
#include <access/Privilege.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProviderImpl.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/DataModelTypes.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/ESP32/ESP32Utils.h>
#include <esp_matter_mem.h>
#include <esp_matter_providers.h>

using chip::DeviceLayer::ChipDeviceEvent;
using chip::DeviceLayer::ConfigurationMgr;
using chip::DeviceLayer::ConnectivityManager;
using chip::DeviceLayer::ConnectivityMgr;
using chip::DeviceLayer::PlatformMgr;
using chip::DeviceLayer::GetDiagnosticDataProvider;
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
using chip::DeviceLayer::ThreadStackMgr;
#endif

#define MAX_GROUPS_PER_FABRIC_PER_ENDPOINT CONFIG_MAX_GROUPS_PER_FABRIC_PER_ENDPOINT

static const char *TAG = "esp_matter_core";
bool esp_matter_started = false;

#ifndef CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
// If Matter Server is disabled, these functions are required by InteractionModelEngine but not linked
// as they are defined in other files. They will be never used if server is not enable. Define empty
// functions in esp_matter_core.cpp to make sure that they are linked
// TODO: Use dynamic server defined in upstream repo
void InitDataModelHandler() {}

namespace chip {
namespace app {
void DispatchSingleClusterCommand(const ConcreteCommandPath &command_path, TLVReader &tlv_data,
                                  CommandHandler *command_obj) {}
} // namespace app
} // namespace chip

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return false;
}

chip::Access::Privilege MatterGetAccessPrivilegeForReadAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return chip::Access::Privilege::kView;
}

chip::Access::Privilege MatterGetAccessPrivilegeForWriteAttribute(chip::ClusterId cluster, chip::AttributeId attribute)
{
    return chip::Access::Privilege::kOperate;
}

chip::Access::Privilege MatterGetAccessPrivilegeForInvokeCommand(chip::ClusterId cluster, chip::CommandId command)
{
    return chip::Access::Privilege::kOperate;
}

chip::Access::Privilege MatterGetAccessPrivilegeForReadEvent(chip::ClusterId cluster, chip::EventId event)
{
    return chip::Access::Privilege::kView;
}
#endif // !CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER

namespace esp_matter {
#ifdef CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
namespace {

void PostEvent(uint16_t eventType)
{
    chip::DeviceLayer::ChipDeviceEvent event;
    event.Type = eventType;
    CHIP_ERROR error = chip::DeviceLayer::PlatformMgr().PostEvent(&event);
    VerifyOrReturn(error == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to post event for event type:%" PRIu16 ", err:%" CHIP_ERROR_FORMAT, eventType, error.Format()));
}

class AppDelegateImpl : public AppDelegate
{
public:
    void OnCommissioningSessionStarted()
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted);
    }

    void OnCommissioningSessionStopped()
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped);
    }

    void OnCommissioningWindowOpened()
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened);
    }

    void OnCommissioningWindowClosed()
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed);
    }
};

class FabricDelegateImpl : public chip::FabricTable::Delegate
{
public:
    void FabricWillBeRemoved(const chip::FabricTable & fabricTable,chip::FabricIndex fabricIndex)
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved);
    }

    void OnFabricRemoved(const chip::FabricTable & fabricTable,chip::FabricIndex fabricIndex)
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kFabricRemoved);
    }

    void OnFabricCommitted(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex)
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kFabricCommitted);
    }

    void OnFabricUpdated(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex)
    {
        PostEvent(chip::DeviceLayer::DeviceEventType::kFabricUpdated);
    }
};

AppDelegateImpl s_app_delegate;

FabricDelegateImpl s_fabric_delegate;

}  // namespace
#endif // CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER

namespace lock {
#define DEFAULT_TICKS (500 / portTICK_PERIOD_MS) /* 500 ms in ticks */
status_t chip_stack_lock(uint32_t ticks_to_wait)
{
#if CHIP_STACK_LOCK_TRACKING_ENABLED
    VerifyOrReturnValue(!PlatformMgr().IsChipStackLockedByCurrentThread(), ALREADY_TAKEN);
#endif
    VerifyOrReturnValue(ticks_to_wait != portMAX_DELAY, SUCCESS, PlatformMgr().LockChipStack());
    uint32_t ticks_remaining = ticks_to_wait;
    uint32_t ticks = DEFAULT_TICKS;
    while (ticks_remaining > 0) {
        VerifyOrReturnValue(!PlatformMgr().TryLockChipStack(), SUCCESS);
        ticks = ticks_remaining < DEFAULT_TICKS ? ticks_remaining : DEFAULT_TICKS;
        ticks_remaining -= ticks;
        ESP_LOGI(TAG, "Did not get lock yet. Retrying...");
        vTaskDelay(ticks);
    }
    ESP_LOGE(TAG, "Could not get lock");
    return FAILED;
}

esp_err_t chip_stack_unlock()
{
    PlatformMgr().UnlockChipStack();
    return ESP_OK;
}
} /* lock */

#ifdef CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
static void deinit_ble_if_commissioned(intptr_t unused)
{
#if CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING
    if(chip::Server::GetInstance().GetFabricTable().FabricCount() > 0) {
        chip::DeviceLayer::Internal::BLEMgr().Shutdown();
    }
#endif /* CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING */
}

static void esp_matter_chip_init_task(intptr_t context)
{
    TaskHandle_t task_to_notify = reinterpret_cast<TaskHandle_t>(context);
    static chip::CommonCaseDeviceServerInitParams initParams;

    initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.appDelegate = &s_app_delegate;
    initParams.dataModelProvider = chip::app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
    // Group data provider injection for dynamic data model
    {
        uint8_t groups_server_cluster_count = cluster::groups::get_server_cluster_count();
        uint16_t max_groups_per_fabric = groups_server_cluster_count * MAX_GROUPS_PER_FABRIC_PER_ENDPOINT;

        // since groupDataProvider is a static variable, it won't be released.
        static chip::Credentials::GroupDataProviderImpl groupDataProvider(max_groups_per_fabric, CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC);

        groupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
        groupDataProvider.SetSessionKeystore(initParams.sessionKeystore);
        groupDataProvider.Init();
        initParams.groupDataProvider = &groupDataProvider;
    }
#endif // CONFIG_ESP_MATTER_ENABLE_DATA_MODEL

    CHIP_ERROR ret = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&s_fabric_delegate);
    if (ret != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to add fabric delegate, err:%" CHIP_ERROR_FORMAT, ret.Format());
    }
    chip::Server::GetInstance().Init(initParams);
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#ifdef CONFIG_ESP_MATTER_ENABLE_OPENTHREAD
    VerifyOrReturn(ThreadStackMgr().InitThreadStack() == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to initialize Thread stack"));
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    VerifyOrReturn(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice) == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to set the Thread device type"));

#elif CHIP_DEVICE_CONFIG_THREAD_FTD
    VerifyOrReturn(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router) == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to set the Thread device type"));
#else
    VerifyOrReturn(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice) == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to set the Thread device type"));
#endif
    VerifyOrReturn(ThreadStackMgr().StartThreadTask() == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to launch Thread task"));
    // If Thread is Provisioned, publish the dns service
    if (chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned() &&
        (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0)) {
        chip::app::DnssdServer::Instance().StartServer();
    }
#endif // CONFIG_ESP_MATTER_ENABLE_OPENTHREAD
#endif
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
    if (endpoint::enable_all() != ESP_OK) {
        ESP_LOGE(TAG, "Enable all endpoints failure");
    }
    // The following two events can't be recorded when we start the server because the endpoints are not enabled.
    // TODO: Find a better way to record the events which should be recorded in matter server init
    // Record start up event in basic information cluster.
    PlatformMgr().HandleServerStarted();
    // Record boot reason evnet in general diagnostics cluster.
    chip::app::Clusters::GeneralDiagnostics::BootReasonEnum bootReason;
    if (GetDiagnosticDataProvider().GetBootReason(bootReason) == CHIP_NO_ERROR) {
        chip::app::Clusters::GeneralDiagnosticsServer::Instance().OnDeviceReboot(bootReason);
    }
#endif
    PlatformMgr().ScheduleWork(deinit_ble_if_commissioned, reinterpret_cast<intptr_t>(nullptr));
    xTaskNotifyGive(task_to_notify);
}
#endif // CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER

static void device_callback_internal(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
        if (event->InterfaceIpAddressChanged.Type == chip::DeviceLayer::InterfaceIpChangeType::kIpV6_Assigned ||
                event->InterfaceIpAddressChanged.Type == chip::DeviceLayer::InterfaceIpChangeType::kIpV4_Assigned) {
            chip::app::DnssdServer::Instance().StartServer();
        }
#endif
        break;
#ifdef CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
    case chip::DeviceLayer::DeviceEventType::kDnssdInitialized:
        esp_matter_ota_requestor_start();
        /* Initialize binding manager */
        client::binding_manager_init();
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning Complete");
        PlatformMgr().ScheduleWork(deinit_ble_if_commissioned, reinterpret_cast<intptr_t>(nullptr));
        break;

    case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionClosed:
        ESP_LOGI(TAG, "BLE Disconnected");
        break;
#endif
    default:
        break;
    }
}

static esp_err_t chip_init(event_callback_t callback, intptr_t callback_arg)
{
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, ESP_ERR_NO_MEM, ESP_LOGE(TAG, "Failed to initialize CHIP memory pool"));
    VerifyOrReturnError(PlatformMgr().InitChipStack() == CHIP_NO_ERROR, ESP_FAIL, ESP_LOGE(TAG, "Failed to initialize CHIP stack"));

    setup_providers();
    // ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Enabled);
    if (PlatformMgr().StartEventLoopTask() != CHIP_NO_ERROR) {
        chip::Platform::MemoryShutdown();
        ESP_LOGE(TAG, "Failed to launch Matter main task");
        return ESP_FAIL;
    }
    PlatformMgr().AddEventHandler(device_callback_internal, static_cast<intptr_t>(NULL));
    if(callback) {
       PlatformMgr().AddEventHandler(callback, callback_arg);
    }
#if CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
    // Add bounds to all attributes
    esp_matter::cluster::add_bounds_callback_common();
#endif // CONFIG_ESP_MATTER_ENABLE_DATA_MODEL

    PlatformMgr().ScheduleWork(esp_matter_chip_init_task, reinterpret_cast<intptr_t>(xTaskGetCurrentTaskHandle()));
    // Wait for the matter stack to be initialized
    xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
    // Initialise clusters which have delegate implemented
    esp_matter::cluster::delegate_init_callback_common();
#endif // CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
#endif // CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER

    return ESP_OK;
}

esp_err_t start(event_callback_t callback, intptr_t callback_arg)
{
    VerifyOrReturnError(!esp_matter_started, ESP_ERR_INVALID_STATE, ESP_LOGE(TAG, "esp_matter has started"));
    esp_err_t err = esp_event_loop_create_default();

    // In case create event loop returns ESP_ERR_INVALID_STATE it is not necessary to fail startup
    // as of it means that default event loop is already initialized and no additional actions should be done.
    VerifyOrReturnError((err == ESP_OK || err == ESP_ERR_INVALID_STATE), err, ESP_LOGE(TAG, "Error create default event loop"));
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    VerifyOrReturnError(chip::DeviceLayer::Internal::ESP32Utils::InitWiFiStack() == CHIP_NO_ERROR, ESP_FAIL, ESP_LOGE(TAG, "Error initializing Wi-Fi stack"));
#endif
#ifdef CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER
    esp_matter_ota_requestor_init();
#endif

    err = chip_init(callback, callback_arg);
    VerifyOrReturnError(err == ESP_OK, err, ESP_LOGE(TAG, "Error initializing matter"));
    esp_matter_started = true;
#if defined(CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER) && defined(CONFIG_ESP_MATTER_ENABLE_DATA_MODEL)
    err = node::read_min_unused_endpoint_id();
    // If the min_unused_endpoint_id is not found, we will write the current min_unused_endpoint_id in nvs.
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        err = node::store_min_unused_endpoint_id();
    }
#endif // defined(CONFIG_ESP_MATTER_ENABLE_MATTER_SERVER) && defined(CONFIG_ESP_MATTER_ENABLE_DATA_MODEL)
    return err;
}

esp_err_t factory_reset()
{
    esp_err_t err = ESP_OK;
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
    node_t *node = node::get();
    if (node) {
        /* ESP Matter data model is used. Erase all the data that we have added in nvs. */
        nvs_handle_t handle;
        err = nvs_open_from_partition(ESP_MATTER_NVS_PART_NAME, ESP_MATTER_KVS_NAMESPACE, NVS_READWRITE, &handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to open esp_matter nvs partition ");
        } else {
            err = nvs_erase_all(handle);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to erase esp_matter nvs namespace");
            } else {
                nvs_commit(handle);
            }
            nvs_close(handle);
        }
    }
#endif
    /* Submodule factory reset. This also restarts after completion. */
    ConfigurationMgr().InitiateFactoryReset();
    return err;
}
} /* esp_matter */
