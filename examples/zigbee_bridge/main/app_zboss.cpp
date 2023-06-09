/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <app_zboss.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_zigbee_core.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <zigbee_bridge.h>
#include <zb_esp_cli.h>

#if (!defined(ZB_MACSPLIT_HOST) && defined(ZB_MACSPLIT_DEVICE))
#error "Zigbee host option should be enabled to use this example"
#endif

static const char *TAG = "esp_zboss";

//static esp_zb_ias_zone_status_change_notif_callback_t door_sensor_state_report_callback(esp_zb_zcl_status_t status, uint16_t zone_status, uint8_t extzone_status, uint8_t zone_id, uint16_t delay);

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

/**
 * @brief Zigbee stack event handler.
 *
 * @param bufid   Reference to the Zigbee stack buffer used to pass signal.
 */

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    // Read signal desription
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    esp_zb_zdo_signal_device_annce_params_t *dev_annce_params = NULL;
    esp_zb_zdo_signal_macsplit_dev_boot_params_t *rcp_version = NULL;

    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Zigbee stack initialized");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;

    case ESP_ZB_MACSPLIT_DEVICE_BOOT:
        ESP_LOGI(TAG, "Zigbee rcp device booted");
        rcp_version = (esp_zb_zdo_signal_macsplit_dev_boot_params_t*)esp_zb_app_signal_get_params(p_sg_p);
        ESP_LOGI(TAG, "Running RCP Version:%s", rcp_version->version_str);
        break;

    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Start network formation");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_FORMATION);
        } else {
            ESP_LOGE(TAG, "Failed to initialize Zigbee stack (status: %d)", err_status);
        }
        break;

    case ESP_ZB_BDB_SIGNAL_FORMATION:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t ieee_address;
            esp_zb_get_long_address(ieee_address);
            ESP_LOGI(TAG, "Formed network successfully");
            ESP_LOGI(TAG, "ieee extended address: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx)",
                     ieee_address[7], ieee_address[6], ieee_address[5], ieee_address[4], ieee_address[3],
                     ieee_address[2], ieee_address[1], ieee_address[0], esp_zb_get_pan_id());
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
        } else {
            ESP_LOGI(TAG, "Restart network formation (status: %d)", err_status);
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_FORMATION, 1000);
        }
        break;

    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Network steering started");
        }
        break;

    case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE:
        dev_annce_params = (esp_zb_zdo_signal_device_annce_params_t *)esp_zb_app_signal_get_params(p_sg_p);
        ESP_LOGI(TAG, "New device commissioned or rejoined (short: 0x%04hx)", dev_annce_params->device_short_addr);
        break;

    default:
        ESP_LOGI(TAG, "status: %d", err_status);
        break;
    }
}

static void door_sensor_state_report_callback(esp_zb_zcl_status_t status, uint16_t zone_status, uint8_t extzone_status, uint8_t zone_id, uint16_t delay)
{
    printf("door_sensor_state_report_callback--zone_status: %d \n", zone_status);
    door_sensor_state_change((bool)(zone_status & 0x0001));
    return;
}

static uint8_t esp_zb_cli_agent_handler(uint8_t bufid)
{
    if (esp_zb_cli_agent_ep_handler_report(bufid) == true) {
        return true;
    }

    return false;
}

static void zboss_task(void *pvParameters)
{
    uint8_t test_attr;
    test_attr = 0;
    /* initialize Zigbee stack with Zigbee coordinator config */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZC_CONFIG();
    esp_zb_init(&zb_nwk_cfg);
    esp_zb_attribute_list_t *esp_zb_basic_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_BASIC);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, &test_attr);
    esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, &test_attr);
    esp_zb_attribute_list_t *esp_zb_on_off_client_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
    esp_zb_cluster_list_t *esp_zb_cluster_list = esp_zb_zcl_cluster_list_create();
    esp_zb_cluster_list_add_basic_cluster(esp_zb_cluster_list, esp_zb_basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_on_off_cluster(esp_zb_cluster_list, esp_zb_on_off_client_cluster, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
    esp_zb_ep_list_t *esp_zb_ep_list = esp_zb_ep_list_create();
    esp_zb_ep_list_add_ep(esp_zb_ep_list, esp_zb_cluster_list, 1, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_ON_OFF_SWITCH_DEVICE_ID);
    esp_zb_device_register(esp_zb_ep_list);

    esp_zb_ias_zone_add_status_change_notify_cb(1, door_sensor_state_report_callback);

    /* initiate Zigbee Stack start without zb_send_no_autostart_signal auto-start */
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_main_loop_iteration();
}

void launch_app_zboss(void)
{
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    /* load Zigbee gateway platform config to initialization */
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));
    xTaskCreate(zboss_task, "zboss_main", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}
