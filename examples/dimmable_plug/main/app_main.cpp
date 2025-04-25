/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

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
#include <app/util/attribute-storage.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

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

namespace {
// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceSwitches = 0x43;
// Common Number Namespace: 7, tag 0 (Zero)
constexpr const uint8_t kTagSwitchOn = 0;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagSwitchOff = 1;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagSwitchToggle = 2;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagSwitchUp = 3;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagSwitchDown = 4;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagSwitchNext = 5;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagSwitchPrevious = 6;
// Common Number Namespace: 7, tag 1 (One)
constexpr const uint8_t kTagSwitchOK = 7;


constexpr const uint8_t kNamespaceNumber = 7;
// Common Number Namespace: 7, tag: 0 (zero)
constexpr const uint8_t kTagNumberZero = 0;
// Common Number Namespace: 7, tag: 1 (one)
constexpr const uint8_t kTagNumberOne = 1;
// Common Number Namespace: 7, tag: 1 (one)
constexpr const uint8_t kTagNumberTwo = 2;
// Common Number Namespace: 7, tag: 1 (one)
constexpr const uint8_t kTagNumberThreee = 3;
// Common Number Namespace: 7, tag: 1 (one)
constexpr const uint8_t kTagNumberFour = 4;
// Common Number Namespace: 7, tag: 1 (one)
constexpr const uint8_t kTagNumberFive = 5;
// Common Number Namespace: 7, tag: 1 (one)
constexpr const uint8_t kTagNumberSix = 6;
// Common Number Namespace: 7, tag: 1 (one)
constexpr const uint8_t kTagNumberSeven = 7;

const Descriptor::Structs::SemanticTagStruct::Type gEp1TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 0}};
const Descriptor::Structs::SemanticTagStruct::Type gEp2TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 1}};
const Descriptor::Structs::SemanticTagStruct::Type gEp3TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 2}};
const Descriptor::Structs::SemanticTagStruct::Type gEp4TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 3}};
const Descriptor::Structs::SemanticTagStruct::Type gEp5TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 4}};
const Descriptor::Structs::SemanticTagStruct::Type gEp6TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 5}};
const Descriptor::Structs::SemanticTagStruct::Type gEp7TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 6}};
const Descriptor::Structs::SemanticTagStruct::Type gEp8TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 7}};
const Descriptor::Structs::SemanticTagStruct::Type gEp9TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 8}};
const Descriptor::Structs::SemanticTagStruct::Type gEp10TagList[] = {
    {.namespaceID = kNamespaceNumber, .tag = 9}};

}

static void memory_profiler_dump_heap_stat()
{
    printf("========== HEAP-DUMP-START ==========\n");
    printf("\tDescription\tInternal\n");
    printf("Current Free Memory\t%d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    printf("Largest Free Block\t%d\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
    printf("Min. Ever Free Size\t%d\n", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
    printf("========== HEAP-DUMP-END ==========\n");
}

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
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
                chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
                constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
                if (!commissionMgr.IsCommissioningWindowOpen())
                {
                    /* After removing last fabric, this example does not remove the Wi-Fi credentials
                     * and still has IP connectivity so, only advertising on DNS-SD.
                     */
                    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                    chip::CommissioningWindowAdvertisement::kDnssdOnly);
                    if (err != CHIP_NO_ERROR)
                    {
                        ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                    }
                }
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
    esp_err_t err = ESP_OK;
    switch (type) {
        case identification::START:
            app_led_blink_start();
            break;
        case identification::STOP:
            app_led_active_stop();
            break;
        case identification::EFFECT:
            switch (effect_id) {
                case (int)Identify::EffectIdentifierEnum::kBlink:
                    app_led_blink_fast_start();
                    break;
                case (int)Identify::EffectIdentifierEnum::kBreathe:
                    app_led_breath_fast_start();
                    break;
                case (int)Identify::EffectIdentifierEnum::kOkay:
                    app_led_blink_slow_start();
                    break;
                case (int)Identify::EffectIdentifierEnum::kChannelChange:
                    app_led_breath_slow_start();
                    break;
                case (int)Identify::EffectIdentifierEnum::kFinishEffect:
                    app_led_active_stop();
                    break;
                case (int)Identify::EffectIdentifierEnum::kStopEffect:
                    app_led_active_stop();
                    break;
                default:
                    break;
            }
            break;
        default:
            ESP_LOGE(TAG, "Identification type not handled");
            err = ESP_FAIL;
            break;
    }
    return err;
}

// This callback is called for every attribute update. The callback implementation shall
// handle the desired attributes and return an appropriate error code. If the attribute
// is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    memory_profiler_dump_heap_stat();

    if (type == PRE_UPDATE) {
        /* Driver update */
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    /* Initialize driver */
    app_driver_handle_t light_handle = app_driver_light_init();
    app_driver_handle_t button_handle = app_driver_button_init();
    reset_rollback_button_init();
    app_reset_button_register(button_handle);

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;

    // node handle can be used to add/modify other endpoints.
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

    dimmable_plugin_unit::config_t plug_config;
    plug_config.on_off.on_off = DEFAULT_POWER;
    plug_config.on_off.lighting.start_up_on_off = nullptr;

    // endpoint handles can be used to add/modify clusters.
    endpoint_t *endpoint;
    cluster_t* descriptor;

    for (uint8_t i = 1; i < 11; i++) {
        endpoint = dimmable_plugin_unit::create(node, &plug_config, ENDPOINT_FLAG_NONE, light_handle);
        ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create plug %d endpoint", i));
        descriptor = cluster::get(endpoint,Descriptor::Id);
        cluster::descriptor::feature::taglist::add(descriptor);
    }

    light_endpoint_id = 1;
    ESP_LOGI(TAG, "Light created with endpoint_id %d", light_endpoint_id);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    // Enable secondary network interface
    secondary_network_interface::config_t secondary_network_interface_config;
    endpoint = endpoint::secondary_network_interface::create(node, &secondary_network_interface_config, ENDPOINT_FLAG_NONE, nullptr);
    ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create secondary network interface endpoint"));
#endif


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
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

    SetTagList(1, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp1TagList));
    SetTagList(2, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp2TagList));
    SetTagList(3, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp3TagList));
    SetTagList(4, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp4TagList));
    SetTagList(5, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp5TagList));
    SetTagList(6, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp6TagList));
    SetTagList(7, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp7TagList));
    SetTagList(8, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp8TagList));
    SetTagList(9, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp9TagList));
    SetTagList(10, chip::Span<const Descriptor::Structs::SemanticTagStruct::Type>(gEp10TagList));

    /* Starting driver with default values */
    app_driver_light_set_defaults(light_endpoint_id);

#if CONFIG_ENABLE_ENCRYPTED_OTA
    err = esp_matter_ota_requestor_encrypted_init(s_decryption_key, s_decryption_key_len);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialized the encrypted OTA, err: %d", err));
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
#if CONFIG_OPENTHREAD_CLI
    esp_matter::console::otcli_register_commands();
#endif
    esp_matter::console::init();
#endif
}
