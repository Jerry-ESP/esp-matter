/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <esp_err.h>
#include "esp_check.h"
#include <esp_log.h>
#include <esp_zigbee_core.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <app_zboss.h>
extern "C" { 
#include "zboss_api.h"
}
#include <zigbee_bridge.h>

#if (!defined(ZB_MACSPLIT_HOST) && defined(ZB_MACSPLIT_DEVICE))
#error "Zigbee host option should be enabled to use this example"
#endif

static const char *TAG = "esp_zboss";

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

static esp_err_t zb_ias_zone_status_change_handler(const esp_zb_zcl_ias_zone_status_change_notification_message_t *message)
{
    esp_err_t ret = ESP_OK;
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);
    ESP_LOGI(TAG, "Received status information: zone status: 0x%x, zone id: 0x%x\n", message->zone_status, message->zone_id);

    bool state = (message->zone_status & 0x0001) ? false : true;

    zigbee_bridge_contact_state_change_handler(message->info.src_address.u.short_addr, message->info.src_endpoint, state);
    return ret;
}

static esp_err_t zb_attribute_report_handler(const esp_zb_zcl_report_attr_message_t *message)
{
    esp_err_t ret = ESP_OK;
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->status);
    ESP_LOGI(TAG, "Received attribute report Cluster: 0x%x, Attribute: 0x%x\n", message->cluster, message->attribute.id);

     if (message->cluster == ZB_ZCL_CLUSTER_ID_ON_OFF && message->attribute.id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
        ESP_LOGI(TAG, "receive report command, send command to the matter device");
        uint8_t *data = (uint8_t*)message->attribute.data.value;
        zigbee_bridge_on_off_report_handler(message->src_address.u.short_addr, message->src_endpoint, (bool)(data[0]));
    }
    return ret;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_CMD_IAS_ZONE_ZONE_STATUS_CHANGE_NOT_ID:
        ret = zb_ias_zone_status_change_handler((esp_zb_zcl_ias_zone_status_change_notification_message_t *)message);
        break;
    case ESP_ZB_CORE_REPORT_ATTR_CB_ID:
        /*report attribute data from device  esp_zb_zcl_report_attr_message_t */
        zb_attribute_report_handler((esp_zb_zcl_report_attr_message_t *) message);
        break;
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

void user_find_cb(esp_zb_zdp_status_t zdo_status, uint16_t addr, uint8_t endpoint, void *user_ctx)
{
    ESP_LOGI(TAG, "User find cb: response_status:%d, address:0x%x, endpoint:%d", zdo_status, addr, endpoint);
    if (zdo_status == ESP_ZB_ZDP_STATUS_SUCCESS) {
#if ENABLE_CONTACT_SENSOR_BRIDGE
        zigbee_bridge_find_bridged_contact_sensor_cb(zdo_status, addr, endpoint, user_ctx);
#endif
#if ENABLE_ONOFF_LIGHT_BRIDGE
        zigbee_bridge_find_bridged_on_off_light_cb(zdo_status, addr, endpoint, user_ctx);
#endif
    }

    return;
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
#if ENABLE_CONTACT_SENSOR_BRIDGE
        zb_set_node_descriptor_manufacturer_code_req(0x115f, NULL); /*0x115f is Aqara vendor id*/
#endif
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
    {
        dev_annce_params = (esp_zb_zdo_signal_device_annce_params_t *)esp_zb_app_signal_get_params(p_sg_p);
        ESP_LOGI(TAG, "New device commissioned or rejoined (short: 0x%04hx)", dev_annce_params->device_short_addr);

        esp_zb_zdo_match_desc_req_param_t find_req;
#if ENABLE_CONTACT_SENSOR_BRIDGE
        uint16_t cluster_list[] = {ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE};
#endif
#if ENABLE_ONOFF_LIGHT_BRIDGE
        uint16_t cluster_list[] = {ESP_ZB_ZCL_CLUSTER_ID_ON_OFF};
#endif
        find_req.dst_nwk_addr = dev_annce_params->device_short_addr;
        find_req.addr_of_interest = dev_annce_params->device_short_addr;
        find_req.profile_id = ESP_ZB_AF_HA_PROFILE_ID;
        find_req.num_in_clusters = 1;
        find_req.num_out_clusters = 0;
        find_req.cluster_list = cluster_list;
        esp_zb_zdo_match_cluster(&find_req, user_find_cb, NULL);
    }
        break;

    default:
        ESP_LOGI(TAG, "status: %d", err_status);
        break;
    }
}

static void zboss_task(void *pvParameters)
{
    /* initialize Zigbee stack with Zigbee coordinator config */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZC_CONFIG();
    esp_zb_init(&zb_nwk_cfg);
    /* initiate Zigbee Stack start without zb_send_no_autostart_signal auto-start */

    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();

    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
#if ENABLE_CONTACT_SENSOR_BRIDGE
    static esp_zb_ias_zone_cluster_cfg_t ias_zone_cfg = {
        .zone_state = ESP_ZB_ZCL_IAS_ZONE_ZONESTATE_NOT_ENROLLED,
        .zone_type = ESP_ZB_ZCL_IAS_ZONE_ZONETYPE_CONTACT_SWITCH,
        .zone_status = 0,
        .ias_cie_addr = ESP_ZB_ZCL_ZONE_IAS_CIE_ADDR_DEFAULT,
        .zone_id = 0,
        .zone_ctx = {NULL, NULL, 0, 0},
    };
    esp_zb_cluster_list_add_ias_zone_cluster(cluster_list, esp_zb_ias_zone_cluster_create(&ias_zone_cfg), ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
#endif
#if ENABLE_ONOFF_LIGHT_BRIDGE
    esp_zb_on_off_cluster_cfg_t on_off_cfg = {
        .on_off = true,
    };
    esp_zb_cluster_list_add_on_off_cluster(cluster_list, esp_zb_on_off_cluster_create(&on_off_cfg), ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
#endif
    esp_zb_ep_list_add_ep(ep_list, cluster_list, 1, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_ON_OFF_SWITCH_DEVICE_ID);
    esp_zb_device_register(ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);

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
