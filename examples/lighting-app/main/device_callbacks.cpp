// Copyright 2021 Espressif Systems (Shanghai) CO LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#include "device_callbacks.hpp"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "led_manager.hpp"
#include "lighting_app_constants.hpp"

#include "app/common/gen/att-storage.h"
#include "app/common/gen/attribute-id.h"
#include "app/common/gen/attribute-type.h"
#include "app/common/gen/cluster-id.h"
#include "app/server/Mdns.h"
#include "app/util/af.h"
#include "app/util/basic-types.h"
#include "platform/CHIPDeviceLayer.h"

using chip::AttributeId;
using chip::ClusterId;
using chip::EndpointId;
using chip::DeviceLayer::ChipDeviceEvent;
using chip::DeviceLayer::DeviceEventType::PublicEventTypes;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId cluster, AttributeId attribute, uint8_t mask,
                                        uint16_t manufacturer, uint8_t type, uint16_t size, uint8_t *value)
{
    ESP_LOGI(APP_LOG_TAG, "Handle cluster ID: %d", cluster);
    if (cluster == ZCL_ON_OFF_CLUSTER_ID) {
        on_on_off_attribute_changed(endpoint, attribute, value, size);
    } else if (cluster == ZCL_LEVEL_CONTROL_CLUSTER_ID) {
        on_level_control_atrribute_changed(endpoint, attribute, value, size);
    }
}

void on_on_off_attribute_changed(chip::EndpointId endpoint, chip::AttributeId attribute, uint8_t *value, size_t size)
{
    if (attribute == ZCL_ON_OFF_ATTRIBUTE_ID) {
        ESP_LOGI(APP_LOG_TAG, "On/Off set to: %d", *value);
        led_set_onoff(*value);
    } else {
        ESP_LOGW(APP_LOG_TAG, "Unknown attribute in On/Off cluster: %d", attribute);
    }
}

void on_level_control_atrribute_changed(chip::EndpointId endpoint, chip::AttributeId attribute, uint8_t *value,
                                        size_t size)
{
    if (attribute == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID) {
        ESP_LOGI(APP_LOG_TAG, "Brightness set to: %d", *value);
        led_set_brightness(*value);
    } else {
        ESP_LOGW(APP_LOG_TAG, "Unknown attribute in level control cluster: %d", attribute);
    }
}

void on_device_event(const ChipDeviceEvent *event, intptr_t arg)
{
    if (event->Type == PublicEventTypes::kInterfaceIpAddressChanged) {
        chip::app::Mdns::StartServer();
    }

    ESP_LOGI(APP_LOG_TAG, "Current free heap: %zu", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void update_current_brightness(uint8_t level)
{
    EmberAfStatus status;

    status = emberAfWriteAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                   &level, ZCL_INT8U_ATTRIBUTE_TYPE);
    assert(status == EMBER_ZCL_STATUS_SUCCESS);
}
