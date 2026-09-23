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
namespace icd_management {

const uint8_t k_max_user_active_mode_trigger_instruction_length = 128u;
namespace feature {
namespace check_in_protocol_support {
typedef struct config {
    uint32_t icd_counter;
    uint16_t clients_supported_per_fabric;
    uint32_t maximum_check_in_backoff;
    config() : icd_counter(0), clients_supported_per_fabric(1), maximum_check_in_backoff(1) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* check_in_protocol_support */

namespace user_active_mode_trigger {
typedef struct config {
    uint32_t user_active_mode_trigger_hint;
    config() : user_active_mode_trigger_hint(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* user_active_mode_trigger */

namespace long_idle_time_support {
typedef struct config {
    uint8_t operating_mode;
    config() : operating_mode(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* long_idle_time_support */

namespace dynamic_sit_lit_support {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* dynamic_sit_lit_support */

} /* feature */

namespace attribute {
attribute_t *create_idle_mode_duration(cluster_t *cluster, uint32_t value);
attribute_t *create_active_mode_duration(cluster_t *cluster, uint32_t value);
attribute_t *create_active_mode_threshold(cluster_t *cluster, uint16_t value);
attribute_t *create_registered_clients(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_icd_counter(cluster_t *cluster, uint32_t value);
attribute_t *create_clients_supported_per_fabric(cluster_t *cluster, uint16_t value);
attribute_t *create_user_active_mode_trigger_hint(cluster_t *cluster, uint32_t value);
attribute_t *create_user_active_mode_trigger_instruction(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_operating_mode(cluster_t *cluster, uint8_t value);
attribute_t *create_maximum_check_in_backoff(cluster_t *cluster, uint32_t value);
} /* attribute */

namespace command {
command_t *create_register_client(cluster_t *cluster);
command_t *create_register_client_response(cluster_t *cluster);
command_t *create_unregister_client(cluster_t *cluster);
command_t *create_stay_active_request(cluster_t *cluster);
command_t *create_stay_active_response(cluster_t *cluster);
} /* command */

typedef struct config {
    uint32_t idle_mode_duration;
    uint32_t active_mode_duration;
    uint16_t active_mode_threshold;
    config() : idle_mode_duration(1), active_mode_duration(300), active_mode_threshold(300) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* icd_management */
} /* cluster */
} /* esp_matter */
