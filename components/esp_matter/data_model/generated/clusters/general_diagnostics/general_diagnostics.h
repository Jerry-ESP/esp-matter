// Copyright 2026 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* THIS IS A GENERATED FILE, DO NOT EDIT */

#pragma once
#include <esp_matter_data_model.h>

namespace esp_matter {
namespace cluster {
namespace general_diagnostics {

namespace feature {
namespace data_model_test {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* data_model_test */

} /* feature */

namespace attribute {
attribute_t *create_network_interfaces(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_reboot_count(cluster_t *cluster, uint16_t value);
attribute_t *create_up_time(cluster_t *cluster, uint64_t value);
attribute_t *create_total_operational_hours(cluster_t *cluster, uint32_t value);
attribute_t *create_boot_reason(cluster_t *cluster, uint8_t value);
attribute_t *create_active_hardware_faults(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_active_radio_faults(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_active_network_faults(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_test_event_triggers_enabled(cluster_t *cluster, bool value);
attribute_t *create_device_load_status(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace command {
command_t *create_test_event_trigger(cluster_t *cluster);
command_t *create_time_snapshot(cluster_t *cluster);
command_t *create_time_snapshot_response(cluster_t *cluster);
command_t *create_payload_test_request(cluster_t *cluster);
command_t *create_payload_test_response(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_hardware_fault_change(cluster_t *cluster);
event_t *create_radio_fault_change(cluster_t *cluster);
event_t *create_network_fault_change(cluster_t *cluster);
event_t *create_boot_reason(cluster_t *cluster);
} /* event */

typedef struct config {
    uint16_t reboot_count;
    uint64_t up_time;
    bool test_event_triggers_enabled;
    config() : reboot_count(0), up_time(0), test_event_triggers_enabled(false) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* general_diagnostics */
} /* cluster */
} /* esp_matter */
