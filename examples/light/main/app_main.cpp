/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_touchlink_light.h"
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif
#include "esp_openthread.h"

#include "esp_system.h"
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include "esp_vfs_dev.h"
#include "esp_vfs_eventfd.h"

#include "openthread/platform/settings.h"
#include "esp_touchlink_light.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_ieee802154.h"
#include "nvs_flash.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ha/esp_zigbee_ha_standard.h"

#include "esp_mac.h"

#include "esp_pm.h"

static const char *TAG = "app_main";
uint16_t light_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;

#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char decryption_key_start[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char decryption_key_end[] asm("_binary_esp_image_encryption_key_pem_end");

static const char *s_decryption_key = decryption_key_start;
static const uint16_t s_decryption_key_len = decryption_key_end - decryption_key_start;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

#if ! defined ZB_ROUTER_ROLE
#error define ZB_ROUTER_ROLE to compile
#endif /* defined ZB_ROUTER_ROLE */

static const char *ZB_TAG = "ESP_TL_ON_OFF_LIGHT";
static TaskHandle_t s_zb_task = NULL;
static esp_err_t deferred_driver_init(void)
{
    light_driver_init(LIGHT_DEFAULT_OFF);
    return ESP_OK;
}

static void free_heap_print()
{
    printf("---------%s: Current Free Memory: %d, Minimum Ever Free Size: %d, Largest Free Block: %d------------\n", TAG,
            heap_caps_get_free_size(MALLOC_CAP_8BIT) - heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
            heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
            heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p     = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = (esp_zb_app_signal_type_t)*p_sg_p;
    esp_zb_zdo_signal_device_annce_params_t *dev_annce_params = NULL;
    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(ZB_TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(ZB_TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
            if (esp_zb_bdb_is_factory_new()) {
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_TOUCHLINK_TARGET);
            } else {
                ESP_LOGI(ZB_TAG, "Device rebooted");
            }
            free_heap_print();
        } else {
            ESP_LOGW(ZB_TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_TARGET:
        ESP_LOGI(ZB_TAG, "Touchlink target is ready, awaiting commissioning");
        break;
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_NWK:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(ZB_TAG,
                     "Commissioning successfully, network information (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, "
                     "Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4], extended_pan_id[3], extended_pan_id[2],
                     extended_pan_id[1], extended_pan_id[0], esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        }
        break;
    case ESP_ZB_BDB_SIGNAL_TOUCHLINK_TARGET_FINISHED:
        if (err_status == ESP_OK) {
            ESP_LOGI(ZB_TAG, "Touchlink target commissioning finished");
        } else {
            ESP_LOGI(ZB_TAG, "Touchlink target commissioning failed");
        }
        break;
    case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE:
        dev_annce_params = (esp_zb_zdo_signal_device_annce_params_t *)esp_zb_app_signal_get_params(p_sg_p);
        ESP_LOGI(ZB_TAG, "New device commissioned or rejoined (short: 0x%04hx)", dev_annce_params->device_short_addr);
        free_heap_print();
        break;
    case ESP_ZB_NWK_SIGNAL_PERMIT_JOIN_STATUS:
        if (err_status == ESP_OK) {
            if (*(uint8_t *)esp_zb_app_signal_get_params(p_sg_p)) {
                ESP_LOGI(ZB_TAG, "Network(0x%04hx) is open for %d seconds", esp_zb_get_pan_id(), *(uint8_t *)esp_zb_app_signal_get_params(p_sg_p));
            } else {
                ESP_LOGW(ZB_TAG, "Network(0x%04hx) closed, devices joining not allowed.", esp_zb_get_pan_id());
            }
        }
        break;
    default:
        ESP_LOGI(ZB_TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));
        break;
    }
}

static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool light_state = 0;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, ZB_TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, ZB_TAG, "Received message: error status(%d)",
                        message->info.status);
    ESP_LOGI(ZB_TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);
    if (message->info.dst_endpoint == HA_ESP_LIGHT_ENDPOINT) {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                ESP_LOGI(ZB_TAG, "Light sets to %s", light_state ? "On" : "Off");
                light_driver_set_power(light_state);
            }
        }
    }
    return ret;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    default:
        ESP_LOGW(ZB_TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

void esp_zb_task(void *pvParameters)
{

    esp_zb_zdo_touchlink_target_set_timeout(TOUCHLINK_TARGET_TIMEOUT);

    esp_zb_attribute_list_t *touchlink_cluster = esp_zb_touchlink_commissioning_cluster_create();

    esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    /* Create a standard HA on-off light cluster list */
    esp_zb_cluster_list_t *cluster_list = esp_zb_on_off_light_clusters_create(&light_cfg);

    /* Add touchlink commissioning cluster */
    esp_zb_cluster_list_add_touchlink_commissioning_cluster(cluster_list, touchlink_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    esp_zb_ep_list_t *esp_zb_ep_list = esp_zb_ep_list_create();
    /* Add created endpoint (cluster_list) to endpoint list */
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = HA_ESP_LIGHT_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_ON_OFF_LIGHT_DEVICE_ID,
        .app_device_version = 0
    };
    esp_zb_ep_list_add_ep(esp_zb_ep_list, cluster_list, endpoint_config);

    esp_zb_device_register(esp_zb_ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);

    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_main_loop_iteration();
}

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        free_heap_print();
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        //otThreadSetEnabled(esp_openthread_get_instance(), false);
        if (esp_ieee802154_get_ot_started()) {
            otThreadSetEnabled(esp_openthread_get_instance(), false);
            esp_ieee802154_set_ot_started(false);
            otPlatSettingsWipe(NULL);
        }
        if (s_zb_task) {
            printf("resume zigbee task-----------\n");
            vTaskResume(s_zb_task);
        }
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        esp_ieee802154_set_ot_started(true);
        if (s_zb_task) {
            printf("suspend zigbee task-----------\n");
            vTaskSuspend(s_zb_task);
        }
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        //otThreadSetEnabled(esp_openthread_get_instance(), false);
        if (esp_ieee802154_get_ot_started()) {
            otThreadSetEnabled(esp_openthread_get_instance(), false);
            esp_ieee802154_set_ot_started(false);
            otPlatSettingsWipe(NULL);
        }
        if (s_zb_task) {
            printf("resume zigbee task-----------\n");
            vTaskResume(s_zb_task);
        }
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        {
            ESP_LOGI(TAG, "Fabric removed successfully");
            if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
            {
                otPlatSettingsWipe(NULL);
                esp_restart();
            }
        break;
        }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE deinitialized and memory reclaimed");
        free_heap_print();
        break;

    default:
        break;
    }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

// This callback is called for every attribute update. The callback implementation shall
// handle the desired attributes and return an appropriate error code. If the attribute
// is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        /* Driver update */
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }
    free_heap_print();

    return err;
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;
    esp_vfs_eventfd_config_t eventfd_config = {
    /**
        * The Uart interface file descriptor
        * The zboss scheduler event file descriptor
        * The ieee802154 radio event file decriptor
        */
        .max_fds = 7,
    };

    uint8_t mac_t[6] = {0x74, 0x4d, 0xbd, 0x60, 0x2d, 0xae};
    // esp_iface_mac_addr_set(mac_t, ESP_MAC_EFUSE_CUSTOM);
    esp_iface_mac_addr_set(mac_t, ESP_MAC_BASE);
#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_t pm_config = {
            .max_freq_mhz = 96,
            .min_freq_mhz = 8,

    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
#endif // CONFIG_PM_ENABLE

    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));

    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    /* Initialize Zigbee stack with Zigbee coordinator config */
    esp_zb_platform_config_t zbconfig = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&zbconfig));

    esp_zb_overall_network_size_set(8);
    esp_zb_io_buffer_size_set(48);
    esp_zb_aps_src_binding_table_size_set(2);
    esp_zb_aps_dst_binding_table_size_set(2);

    esp_zb_cfg_t zb_nwk_cfg;
    memset((void*)(&zb_nwk_cfg), 0, sizeof(esp_zb_cfg_t));
    zb_nwk_cfg.esp_zb_role = ESP_ZB_DEVICE_TYPE_ROUTER, zb_nwk_cfg.install_code_policy = INSTALLCODE_POLICY_ENABLE, zb_nwk_cfg.nwk_cfg.zczr_cfg .max_children = MAX_CHILDREN;
    esp_zb_init(&zb_nwk_cfg);
    esp_zb_set_channel_mask(ESP_ZB_TOUCHLINK_CHANNEL_MASK);
    esp_zb_set_rx_on_when_idle(true);

    deferred_driver_init();

    /* Initialize driver */
    app_driver_handle_t light_handle = app_driver_light_init();
    app_driver_handle_t button_handle = app_driver_button_init();
    app_reset_button_register(button_handle);

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;

    // node handle can be used to add/modify other endpoints.
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

    extended_color_light::config_t light_config;
    light_config.on_off.on_off = DEFAULT_POWER;
    light_config.on_off.lighting.start_up_on_off = nullptr;
    light_config.level_control.current_level = DEFAULT_BRIGHTNESS;
    light_config.level_control.lighting.start_up_current_level = DEFAULT_BRIGHTNESS;
    light_config.color_control.color_mode = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    light_config.color_control.enhanced_color_mode = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    light_config.color_control.color_temperature.startup_color_temperature_mireds = nullptr;

    // endpoint handles can be used to add/modify clusters.
    endpoint_t *endpoint = extended_color_light::create(node, &light_config, ENDPOINT_FLAG_NONE, light_handle);
    ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create extended color light endpoint"));

    light_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Light created with endpoint_id %d", light_endpoint_id);

    /* Mark deferred persistence for some attributes that might be changed rapidly */
    cluster_t *level_control_cluster = cluster::get(endpoint, LevelControl::Id);
    attribute_t *current_level_attribute = attribute::get(level_control_cluster, LevelControl::Attributes::CurrentLevel::Id);
    attribute::set_deferred_persistence(current_level_attribute);

    cluster_t *color_control_cluster = cluster::get(endpoint, ColorControl::Id);
    attribute_t *current_x_attribute = attribute::get(color_control_cluster, ColorControl::Attributes::CurrentX::Id);
    attribute::set_deferred_persistence(current_x_attribute);
    attribute_t *current_y_attribute = attribute::get(color_control_cluster, ColorControl::Attributes::CurrentY::Id);
    attribute::set_deferred_persistence(current_y_attribute);
    attribute_t *color_temp_attribute = attribute::get(color_control_cluster, ColorControl::Attributes::ColorTemperatureMireds::Id);
    attribute::set_deferred_persistence(color_temp_attribute);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0) {
        esp_ieee802154_set_ot_started(true);
    } else {
        if (esp_ieee802154_get_ot_started()) {
            otPlatSettingsWipe(NULL);
            esp_restart();
        }
        xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, &s_zb_task);
    }
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

    /* Starting driver with default values */
    app_driver_light_set_defaults(light_endpoint_id);

    free_heap_print();

#if CONFIG_ENABLE_ENCRYPTED_OTA
    err = esp_matter_ota_requestor_encrypted_init(s_decryption_key, s_decryption_key_len);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialized the encrypted OTA, err: %d", err));
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
#if CONFIG_OPENTHREAD_CLI
    esp_matter::console::otcli_register_commands();
#endif
    esp_matter::console::init();
#endif
}
