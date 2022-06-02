/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <esp_matter_attribute_utils.h>
#include <zboss_api.h>
#include <zboss_api_zcl.h>

#include <zb_ha.h>

void zigbee_bridge_match_bridged_onoff_light(zb_bufid_t bufid);

void zigbee_bridge_match_bridged_onoff_light_timeout(zb_bufid_t bufid);

void send_on_off_command_to_bound_matter_device(uint8_t on_off);

esp_err_t zigbee_bridge_attribute_update(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                                         esp_matter_attr_val_t *val);

#ifdef __cplusplus
}
#endif
