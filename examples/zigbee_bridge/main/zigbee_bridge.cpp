/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <app_bridged_device.h>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_bridge.h>
#include <zigbee_bridge.h>

static const char *TAG = "zigbee_bridge";

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

extern uint16_t aggregator_endpoint_id;
extern uint16_t switch_endpoint_id;

void zigbee_bridge_find_bridged_on_off_light_cb(esp_zb_zdp_status_t zdo_status, uint16_t addr, uint8_t endpoint, void *user_ctx)
{
    ESP_LOGI(TAG, "on_off_light found: address:0x%" PRIx16 ", endpoint:%" PRId8 ", response_status:%d", addr, endpoint, zdo_status);
    if (zdo_status == ESP_ZB_ZDP_STATUS_SUCCESS) {
        node_t *node = node::get();
        if (!node) {
            ESP_LOGE(TAG, "Could not find esp_matter node");
            return;
        }
        if (app_bridge_get_device_by_zigbee_shortaddr(addr)) {
            ESP_LOGI(TAG, "Bridged node for 0x%04" PRIx16 " zigbee device on endpoint %" PRId16 " has been created", addr,
                     app_bridge_get_matter_endpointid_by_zigbee_shortaddr(addr));
        } else {
            app_bridged_device_t *bridged_device =
                app_bridge_create_bridged_device(node, aggregator_endpoint_id, ESP_MATTER_ON_OFF_LIGHT_DEVICE_TYPE_ID,
                                                 ESP_MATTER_BRIDGED_DEVICE_TYPE_ZIGBEE,
                                                 app_bridge_zigbee_address(endpoint, addr));
            if (!bridged_device) {
                ESP_LOGE(TAG, "Failed to create zigbee bridged device (on_off light)");
                return;
            }
            ESP_LOGI(TAG, "Create/Update bridged node for 0x%04" PRIx16 " zigbee device on endpoint %" PRId16 "", addr,
                     app_bridge_get_matter_endpointid_by_zigbee_shortaddr(addr));
        }
    }
}

esp_err_t zigbee_bridge_attribute_update(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                                         esp_matter_attr_val_t *val, app_bridged_device_t *zigbee_device)
{
    if (zigbee_device && zigbee_device->dev && zigbee_device->dev->endpoint) {
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                ESP_LOGD(TAG, "Update Bridged Device, ep: %" PRId16 ", cluster: %" PRId32 ", att: %" PRId32 "", endpoint_id, cluster_id,
                         attribute_id);
                ESP_LOGW(TAG, "Update Bridged Device, ep: %" PRId16 ", cluster: %" PRId32 ", att: %" PRId32 "", endpoint_id, cluster_id,
                         attribute_id);
                esp_zb_zcl_on_off_cmd_t cmd_req;
                cmd_req.zcl_basic_cmd.dst_addr_u.addr_short = zigbee_device->dev_addr.zigbee_shortaddr;
                cmd_req.zcl_basic_cmd.dst_endpoint = zigbee_device->dev_addr.zigbee_endpointid;
                cmd_req.zcl_basic_cmd.src_endpoint = esp_matter::endpoint::get_id(zigbee_device->dev->endpoint);
                cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
                cmd_req.on_off_cmd_id = val->val.b ? ESP_ZB_ZCL_CMD_ON_OFF_ON_ID : ESP_ZB_ZCL_CMD_ON_OFF_OFF_ID;
                esp_zb_zcl_on_off_cmd_req(&cmd_req);
            }
        }
    }
    else{
        ESP_LOGE(TAG, "Unable to Update Bridge Device, ep: %" PRId16 ", cluster: %" PRId32 ", att: %" PRId32 "", endpoint_id, cluster_id, attribute_id);
    }
    return ESP_OK;
}

void app_driver_client_command_callback(client::peer_device_t *peer_device, client::command_handle_t *cmd_handle,
                                        void *priv_data)
{
    // on_off light switch should support on_off cluster and identify cluster commands sending.
    if (cmd_handle->cluster_id == OnOff::Id) {
        switch (cmd_handle->command_id) {
        case OnOff::Commands::Off::Id: {
            on_off::command::send_off(peer_device, cmd_handle->endpoint_id);
            break;
        };
        case OnOff::Commands::On::Id: {
            on_off::command::send_on(peer_device, cmd_handle->endpoint_id);
            break;
        };
        case OnOff::Commands::Toggle::Id: {
            on_off::command::send_toggle(peer_device, cmd_handle->endpoint_id);
            break;
        };
        default:
            break;
        }
    } else if (cmd_handle->cluster_id == Identify::Id) {
        if (cmd_handle->command_id == Identify::Commands::Identify::Id) {
            if (((char *)cmd_handle->command_data)[0] != 1) {
                ESP_LOGE(TAG, "Number of parameters error");
                return;
            }
            identify::command::send_identify(peer_device, cmd_handle->endpoint_id,
                                             strtol((const char *)(cmd_handle->command_data) + 1, NULL, 16));
        } else {
            ESP_LOGE(TAG, "Unsupported command");
        }
    } else {
        ESP_LOGE(TAG, "Unsupported cluster");
    }
}

void app_driver_client_group_command_callback(uint8_t fabric_index, client::command_handle_t *cmd_handle,
                                              void *priv_data)
{
    // on_off light switch should support on_off cluster and identify cluster commands sending.
    if (cmd_handle->cluster_id == OnOff::Id) {
        switch (cmd_handle->command_id) {
        case OnOff::Commands::Off::Id: {
            on_off::command::group_send_off(fabric_index, cmd_handle->group_id);
            break;
        }
        case OnOff::Commands::On::Id: {
            on_off::command::group_send_on(fabric_index, cmd_handle->group_id);
            break;
        }
        case OnOff::Commands::Toggle::Id: {
            on_off::command::group_send_toggle(fabric_index, cmd_handle->group_id);
            break;
        }
        default:
            break;
        }
    } else if (cmd_handle->cluster_id == Identify::Id) {
        if (cmd_handle->command_id == Identify::Commands::Identify::Id) {
            if (((char *)cmd_handle->command_data)[0] != 1) {
                ESP_LOGE(TAG, "Number of parameters error");
                return;
            }
            identify::command::group_send_identify(fabric_index, cmd_handle->group_id,
                                                   strtol((const char *)(cmd_handle->command_data) + 1, NULL, 16));
        } else {
            ESP_LOGE(TAG, "Unsupported command");
        }
    } else {
        ESP_LOGE(TAG, "Unsupported cluster");
    }
}

void door_sensor_state_change(bool state)
{
    ESP_LOGI(TAG, "Door sensor state: %d\n", state);
    client::command_handle_t cmd_handle;
    cmd_handle.cluster_id = OnOff::Id;
    cmd_handle.is_group = false;

    if (state == true) {
        cmd_handle.command_id = OnOff::Commands::On::Id;
    } else {
        cmd_handle.command_id = OnOff::Commands::Off::Id;
    }

    lock::chip_stack_lock(portMAX_DELAY);
    client::cluster_update(switch_endpoint_id, &cmd_handle);
    lock::chip_stack_unlock();
}

void switch_init()
{
    client::set_command_callback(app_driver_client_command_callback, app_driver_client_group_command_callback, NULL);
}