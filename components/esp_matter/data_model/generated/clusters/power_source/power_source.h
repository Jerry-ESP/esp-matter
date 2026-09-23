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
namespace power_source {

const uint8_t k_max_description_length = 60u;
const uint8_t k_max_bat_replacement_description_length = 60u;
const uint8_t k_max_bat_ansi_designation_length = 20u;
const uint8_t k_max_bat_iec_designation_length = 20u;
namespace feature {
namespace wired {
typedef struct config {
    uint8_t wired_current_type;
    config() : wired_current_type(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* wired */

namespace battery {
typedef struct config {
    uint8_t bat_charge_level;
    bool bat_replacement_needed;
    uint8_t bat_replaceability;
    config() : bat_charge_level(0), bat_replacement_needed(false), bat_replaceability(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* battery */

namespace rechargeable {
typedef struct config {
    uint8_t bat_charge_state;
    bool bat_functional_while_charging;
    config() : bat_charge_state(0), bat_functional_while_charging(false) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* rechargeable */

namespace replaceable {
typedef struct config {
    char bat_replacement_description[k_max_bat_replacement_description_length + 1];
    uint8_t bat_quantity;
    config() : bat_quantity(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* replaceable */

} /* feature */

namespace attribute {
attribute_t *create_status(cluster_t *cluster, uint8_t value);
attribute_t *create_order(cluster_t *cluster, uint8_t value);
attribute_t *create_description(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_wired_assessed_input_voltage(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_wired_assessed_input_frequency(cluster_t *cluster, nullable<uint16_t> value);
attribute_t *create_wired_current_type(cluster_t *cluster, uint8_t value);
attribute_t *create_wired_assessed_current(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_wired_nominal_voltage(cluster_t *cluster, uint32_t value);
attribute_t *create_wired_maximum_current(cluster_t *cluster, uint32_t value);
attribute_t *create_wired_present(cluster_t *cluster, bool value);
attribute_t *create_active_wired_faults(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_bat_voltage(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_bat_percent_remaining(cluster_t *cluster, nullable<uint8_t> value);
attribute_t *create_bat_time_remaining(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_bat_charge_level(cluster_t *cluster, uint8_t value);
attribute_t *create_bat_replacement_needed(cluster_t *cluster, bool value);
attribute_t *create_bat_replaceability(cluster_t *cluster, uint8_t value);
attribute_t *create_bat_present(cluster_t *cluster, bool value);
attribute_t *create_active_bat_faults(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_bat_replacement_description(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_bat_common_designation(cluster_t *cluster, uint8_t value);
attribute_t *create_bat_ansi_designation(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_bat_iec_designation(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_bat_approved_chemistry(cluster_t *cluster, uint8_t value);
attribute_t *create_bat_capacity(cluster_t *cluster, uint32_t value);
attribute_t *create_bat_quantity(cluster_t *cluster, uint8_t value);
attribute_t *create_bat_charge_state(cluster_t *cluster, uint8_t value);
attribute_t *create_bat_time_to_full_charge(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_bat_functional_while_charging(cluster_t *cluster, bool value);
attribute_t *create_bat_charging_current(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_active_bat_charge_faults(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_endpoint_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace event {
event_t *create_wired_fault_change(cluster_t *cluster);
event_t *create_bat_fault_change(cluster_t *cluster);
event_t *create_bat_charge_fault_change(cluster_t *cluster);
} /* event */

typedef struct config {
    uint8_t status;
    uint8_t order;
    char description[k_max_description_length + 1];
    struct {
        feature::wired::config_t wired;
        feature::battery::config_t battery;
        feature::rechargeable::config_t rechargeable;
        feature::replaceable::config_t replaceable;
    } features;
    uint32_t feature_flags;
    config() : status(0), order(0), description{0}, feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* power_source */
} /* cluster */
} /* esp_matter */
