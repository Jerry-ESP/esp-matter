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

#include <app_priv.h>
#include <app_reset.h>
#include <common_macros.h>
#include <humidistat_delegate.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/clusters/humidistat-server/CodegenIntegration.h>
#include <app/data-model/Nullable.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

static const char *TAG = "app_main";
uint16_t humidity_conditioner_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;

// Declared by the relative-humidity provider. The header lives next to that .cpp and is not on the public include path.
namespace chip::app::Clusters::RelativeHumidityMeasurement {
CHIP_ERROR SetMeasuredValue(chip::EndpointId endpointId, chip::app::DataModel::Nullable<uint16_t> measuredValue);
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

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG, "Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    if (type != PRE_UPDATE) {
        return ESP_OK;
    }
    return app_driver_attribute_update((app_driver_handle_t)priv_data, endpoint_id, cluster_id, attribute_id, val);
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    nvs_flash_init();

    app_driver_handle_t button_handle = app_driver_button_init();
    if (button_handle != nullptr) {
        app_reset_button_register(button_handle);
    }

    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

    static Humidistat::ExampleHumidistatDelegate humidistat_delegate;

    humidity_conditioner::config_t config;
    config.humidistat.mode = static_cast<uint8_t>(Humidistat::HumidistatModeEnum::kAuto);
    config.humidistat.system_state = static_cast<uint8_t>(Humidistat::HumidistatSystemStateEnum::kIdle);
    config.humidistat.features.sensor.user_setpoint = 50;
    config.humidistat.features.sensor.min_setpoint = 30;
    config.humidistat.features.sensor.max_setpoint = 70;
    config.humidistat.features.sensor.step = 5;
    config.humidistat.features.sensor.target_setpoint = 50;
    config.humidistat.features.optimal.target_setpoint = 50;
    config.humidistat.features.optimal.optimal = false;
    config.humidistat.features.humidifier.mist_type = 
        static_cast<uint8_t>(Humidistat::MistTypeBitmap::kMistCold);
    // fan_auto is the generated name of the spec Auto feature (0x10).
    config.humidistat.feature_flags = cluster::humidistat::feature::humidifier::get_id() |
                                      cluster::humidistat::feature::dehumidifier::get_id() |
                                      cluster::humidistat::feature::sensor::get_id() |
                                      cluster::humidistat::feature::fan_auto::get_id() |
                                      cluster::humidistat::feature::optimal::get_id() |
                                      cluster::humidistat::feature::cold_mist::get_id();

    endpoint_t *endpoint = humidity_conditioner::create(node, &config, ENDPOINT_FLAG_NONE, nullptr);
    ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create humidity conditioner endpoint"));

    cluster::relative_humidity_measurement::config_t humidity_config;
    humidity_config.min_measured_value = nullable<uint16_t>(0);
    humidity_config.max_measured_value = nullable<uint16_t>(10000);
    cluster_t *humidity_cluster =
        cluster::relative_humidity_measurement::create(endpoint, &humidity_config, CLUSTER_FLAG_SERVER);
    ABORT_APP_ON_FAILURE(humidity_cluster != nullptr, ESP_LOGE(TAG, "Failed to create relative humidity measurement cluster"));

    humidity_conditioner_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Humidity conditioner created with endpoint_id %d, features HUM|DEHUM|SENSOR|AUTO|OPT|COLD",
             humidity_conditioner_endpoint_id);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t ot_config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&ot_config);
#endif

    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

    HumidistatCluster *humidistat = Humidistat::FindClusterOnEndpoint(humidity_conditioner_endpoint_id);
    ABORT_APP_ON_FAILURE(humidistat != nullptr, ESP_LOGE(TAG, "Humidistat cluster was not registered"));
    humidistat->SetDelegate(&humidistat_delegate);

    nullable<uint16_t> measured_humidity;
    err = app_driver_get_relative_humidity(&measured_humidity);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to read relative humidity, err:%d", err));
    chip::app::DataModel::Nullable<uint16_t> reported_humidity;
    if (!measured_humidity.is_null()) {
        reported_humidity.SetNonNull(measured_humidity.value());
    }
    CHIP_ERROR humidity_err =
        RelativeHumidityMeasurement::SetMeasuredValue(humidity_conditioner_endpoint_id, reported_humidity);
    ABORT_APP_ON_FAILURE(humidity_err == CHIP_NO_ERROR,
                         ESP_LOGE(TAG, "Failed to set measured humidity, err:%" CHIP_ERROR_FORMAT, humidity_err.Format()));

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::attribute_register_commands();
#if CONFIG_OPENTHREAD_CLI
    esp_matter::console::otcli_register_commands();
#endif
    esp_matter::console::init();
#endif
}
