/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <esp_matter_attribute_utils.h>
#include <app_bridged_device.h>
#include <stdint.h>

esp_err_t rainmaker_bridge_attribute_update(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                                         esp_matter_attr_val_t *val, app_bridged_device_t *zigbee_device);

esp_err_t rainmaker_get_nodes(char *out_buf, size_t out_buf_len);

esp_err_t rainmaker_parse_node_details(char *out_buf)

void rainmaker_init();