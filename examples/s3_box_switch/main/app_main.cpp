/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#ifdef CONFIG_IDF_TARGET_ESP32S3
#include <esp32s3/rom/rtc.h>
#endif

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <app_priv.h>
#include <app_reset.h>
#include <box_main.h>
#include <app/server/Server.h>

static const char *TAG = "app_main";
uint16_t switch_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
static bool isCommissioningComplete = false;

static SemaphoreHandle_t box_sem = NULL;

void start_box(void)
{
    xSemaphoreGive(box_sem);
}

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address Changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        isCommissioningComplete = true;
	start_box();
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
        if(isCommissioningComplete)
		start_box();
	break;

    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %d, effect: %d", type, effect_id);
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    if (type == PRE_UPDATE) {
        /* Handle the attribute updates here. */
    }

    return ESP_OK;
}

static esp_err_t box_switch_nvs_set_u8(uint8_t value)
{
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open_from_partition("nvs", "matter_box_app", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS: %d", err);
        return err;
    }
    err = nvs_set_u8(handle, "reboot_count", value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting in NVS: %d", err);
    }
    nvs_commit(handle);
    nvs_close(handle);
    return err;
}

static uint8_t box_switch_nvs_get_u8(void)
{
    uint8_t value = 0;
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open_from_partition("nvs", "matter_box_app", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS: %d", err);
        return value;
    }
    err = nvs_get_u8(handle, "reboot_count", &value);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error getting from NVS: %d", err);
    }
    nvs_commit(handle);
    nvs_close(handle);
    return value;
}

static void reset_reboot_count(TimerHandle_t timer)
{
    ESP_LOGW(TAG, "Resetting reboot count-------------------------");
    uint8_t reboot_count = box_switch_nvs_get_u8();
    box_switch_nvs_set_u8(0);
    if (reboot_count >= 5) {
        esp_matter::factory_reset();
    }
    xTimerDelete(timer, 0);
}

static void factory_reset_check(void)
{
    uint8_t reboot_count = box_switch_nvs_get_u8();
    reboot_count ++;
    ESP_LOGW(TAG, "Reboot count: %d-------------------------------", reboot_count);

    box_switch_nvs_set_u8(reboot_count);
    TimerHandle_t timer = xTimerCreate("reset_reboot_count", pdMS_TO_TICKS(5 * 1000), pdFALSE, NULL,
                                    &reset_reboot_count);
    if (!timer) {
        ESP_LOGE(TAG, "Could not initialize timer");
        return;
    }
    xTimerStart(timer, 0);
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    factory_reset_check();

    /* Initialize driver */
    app_driver_handle_t switch_handle = app_driver_switch_init();
    app_reset_button_register(switch_handle);

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);

    on_off_switch::config_t switch_config;
    endpoint_t *endpoint = on_off_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, switch_handle);

    /* These node and endpoint handles can be used to create/add other endpoints and clusters. */
    if (!node || !endpoint) {
        ESP_LOGE(TAG, "Matter node creation failed");
    }

    /* Add group cluster to the switch endpoint */
    cluster::groups::config_t groups_config;
    cluster::groups::create(endpoint, &groups_config, CLUSTER_FLAG_SERVER | CLUSTER_FLAG_CLIENT);

    switch_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Switch created with endpoint_id %d", switch_endpoint_id);

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Matter start failed: %d", err);
    }

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::init();
#endif
    box_sem = xSemaphoreCreateBinary();

    if(chip::Server::GetInstance().GetFabricTable().FabricCount() <= 0) {
printf("Fabric Count is zero\n");
        xSemaphoreTake(box_sem, portMAX_DELAY);
        xSemaphoreTake(box_sem, portMAX_DELAY);
        vSemaphoreDelete(box_sem);
        box_sem = NULL;
    }

    box_main();
}
