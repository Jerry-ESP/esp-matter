/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "zigbee_bridge.h"
#include <app_zboss.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <zboss_api.h>
#include "ha/zb_ha_configuration_tool.h"

#if (!defined ZB_MACSPLIT_HOST)
#error "Zigbee host option should be enabled to use this example"
#endif

#define ZB_DEVICE_ENDPOINT 1

/* Basic cluster attributes data */
zb_uint8_t g_attr_zcl_version  = ZB_ZCL_VERSION;
zb_uint8_t g_attr_power_source = ZB_ZCL_BASIC_POWER_SOURCE_UNKNOWN;

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST(basic_attr_list, &g_attr_zcl_version, &g_attr_power_source);

/* Identify cluster attributes data */
zb_uint16_t g_attr_identify_time = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(identify_attr_list, &g_attr_identify_time);

ZB_HA_DECLARE_CONFIGURATION_TOOL_CLUSTER_LIST(bridge_clusters,
                                 basic_attr_list, identify_attr_list);

ZB_HA_DECLARE_CONFIGURATION_TOOL_EP(bridge_ep, ZB_DEVICE_ENDPOINT, bridge_clusters);

ZB_HA_DECLARE_CONFIGURATION_TOOL_CTX(bridge_ctx, bridge_ep);

static const char *TAG = "esp_zboss";

static void bdb_start_top_level_commissioning_cb(zb_uint8_t mode_mask)
{
    if (!bdb_start_top_level_commissioning(mode_mask)) {
        ESP_LOGE(TAG, "In BDB commissioning, an error occurred (for example: the device has already been running)");
    }
}

void zcl_process_attribute_reporting(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
    zb_uint8_t src_ep = ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).src_endpoint;
    zb_uint8_t dst_ep = ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint;

    zb_zcl_report_attr_req_t *rep_attr_req;

    do
    {
        ZB_ZCL_GENERAL_GET_NEXT_REPORT_ATTR_REQ(param, rep_attr_req);
        if (rep_attr_req)
        {
            ESP_LOGI(TAG, "Report command is received, src_ep %d, dst_ep %d, cluster_id 0x%x, attr_id 0x%x",
                    src_ep, dst_ep, cmd_info->cluster_id, rep_attr_req->attr_id);
            if (cmd_info->cluster_id == ZB_ZCL_CLUSTER_ID_ON_OFF &&
                rep_attr_req->attr_id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID)
            {
                ESP_LOGI(TAG, "receive report command, send command to the matter device");
                zb_uint8_t on_off_val = rep_attr_req->attr_value[0];
                send_on_off_command_to_bound_matter_device(on_off_val);
                // send command to matter device.
            }
        }
    } while (rep_attr_req);
}

zb_uint8_t zcl_custom_cmd_handler(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
    /* Store some values - cmd_info will be overwritten */
    zb_uint8_t processed = ZB_FALSE;

    if(cmd_info->is_common_command && cmd_info->cmd_id == ZB_ZCL_CMD_REPORT_ATTRIB)
    {
        zcl_process_attribute_reporting(param);
    }

  return processed;
}

/**
 * @brief Zigbee stack event handler.
 *
 * @param bufid   Reference to the Zigbee stack buffer used to pass signal.
 */

void zboss_signal_handler(zb_bufid_t bufid)
{
    // Read signal desription
    zb_zdo_app_signal_hdr_t *p_sg_p = NULL;
    zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &p_sg_p);
    zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
    zb_zdo_signal_device_annce_params_t *device_annce_params = NULL;

    switch (sig) {
    case ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Zigbee stack initialized");
        bdb_start_top_level_commissioning(ZB_BDB_INITIALIZATION);
        break;

    case ZB_MACSPLIT_DEVICE_BOOT:
        ESP_LOGI(TAG, "Zigbee rcp device booted");
        break;

    case ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        if (status == RET_OK) {
            ESP_LOGI(TAG, "Start network formation");
            bdb_start_top_level_commissioning(ZB_BDB_NETWORK_FORMATION);
        } else {
            ESP_LOGE(TAG, "Failed to initialize Zigbee stack (status: %d)", status);
        }
        break;

    case ZB_BDB_SIGNAL_FORMATION:
        if (status == RET_OK) {
            zb_ieee_addr_t ieee_address;
            zb_get_long_address(ieee_address);
            ESP_LOGI(TAG, "Formed network successfully");
            ESP_LOGI(TAG, "ieee extended address: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx)",
                     ieee_address[7], ieee_address[6], ieee_address[5], ieee_address[4], ieee_address[3],
                     ieee_address[2], ieee_address[1], ieee_address[0], ZB_PIBCACHE_PAN_ID());
            bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
        } else {
            ESP_LOGI(TAG, "Restart network formation (status: %d)", status);
            ZB_SCHEDULE_APP_ALARM((zb_callback_t)bdb_start_top_level_commissioning_cb, ZB_BDB_NETWORK_FORMATION,
                                  ZB_TIME_ONE_SECOND);
        }
        break;

    case ZB_BDB_SIGNAL_STEERING:
        if (status == RET_OK) {
            ESP_LOGI(TAG, "Network steering started");
        }
        break;

    case ZB_ZDO_SIGNAL_DEVICE_ANNCE:
        device_annce_params = ZB_ZDO_SIGNAL_GET_PARAMS(p_sg_p, zb_zdo_signal_device_annce_params_t);
        ESP_LOGI(TAG, "New device commissioned or rejoined (short: 0x%04hx)", device_annce_params->device_short_addr);
        /*
        status =
            ZB_SCHEDULE_APP_ALARM(zigbee_bridge_match_bridged_onoff_light, bufid, MATCH_BRIDGED_DEVICE_START_DELAY);
        if (status != RET_OK) {
            ESP_LOGD(TAG, "Could not start schedule alarm for matching bridged device");
        }
        status =
            ZB_SCHEDULE_APP_ALARM(zigbee_bridge_match_bridged_onoff_light_timeout, bufid, MATCH_BRIDGED_DEVICE_TIMEOUT);
        if (status != RET_OK) {
            ESP_LOGD(TAG, "Could not start schedule alarm for matching bridged device timeout");
        }
        // this buf will be free in zboss_match_bridged_device_callback/zboss_match_bridged_device_timeout later
        bufid = 0;
        */
        break;

    default:
        ESP_LOGI(TAG, "status: %d", status);
        break;
    }
    /* All callbacks should either reuse or free passed buffers. If bufid == 0, the buffer is invalid (not passed) */
    if (bufid) {
        zb_buf_free(bufid);
    }
}

void zboss_task(void *pvParameters)
{
    ZB_INIT("zigbee bridge");
    zb_set_network_coordinator_role(IEEE_CHANNEL_MASK);
    zb_bdb_set_legacy_device_support(ZB_TRUE); // disable TC Link exchange
    zb_set_nvram_erase_at_start(ERASE_PERSISTENT_CONFIG);
    zb_set_max_children(MAX_CHILDREN);

    ZB_AF_REGISTER_DEVICE_CTX(&bridge_ctx);
    ZB_AF_SET_ENDPOINT_HANDLER(ZB_DEVICE_ENDPOINT, zcl_custom_cmd_handler);
    /* initiate Zigbee Stack start without zb_send_no_autostart_signal auto-start */
    ESP_ERROR_CHECK(zboss_start_no_autostart());
    zigbee_bridge_add_door_sensor_endpoint(0x1234, 1);
    while (1) {
        zboss_main_loop_iteration();
    }
}

void launch_app_zboss(void)
{
    zb_esp_platform_config_t config = {
        .radio_config = ZB_ESP_DEFAULT_RADIO_CONFIG(),
        .host_config = ZB_ESP_DEFAULT_HOST_CONFIG(),
    };
    /* load Zigbee gateway platform config to initialization */
    ESP_ERROR_CHECK(zb_esp_platform_config(&config));
    xTaskCreate(zboss_task, "zboss_main", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}
