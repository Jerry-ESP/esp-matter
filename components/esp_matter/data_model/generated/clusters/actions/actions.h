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
namespace actions {

const uint16_t k_max_setup_url_length = 512u;
namespace attribute {
attribute_t *create_action_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_endpoint_lists(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_setup_url(cluster_t *cluster, char * value, uint16_t length);
} /* attribute */

namespace command {
command_t *create_instant_action(cluster_t *cluster);
command_t *create_instant_action_with_transition(cluster_t *cluster);
command_t *create_start_action(cluster_t *cluster);
command_t *create_start_action_with_duration(cluster_t *cluster);
command_t *create_stop_action(cluster_t *cluster);
command_t *create_pause_action(cluster_t *cluster);
command_t *create_pause_action_with_duration(cluster_t *cluster);
command_t *create_resume_action(cluster_t *cluster);
command_t *create_enable_action(cluster_t *cluster);
command_t *create_enable_action_with_duration(cluster_t *cluster);
command_t *create_disable_action(cluster_t *cluster);
command_t *create_disable_action_with_duration(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_state_changed(cluster_t *cluster);
event_t *create_action_failed(cluster_t *cluster);
} /* event */

typedef struct config {
    void *delegate;
    config() : delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* actions */
} /* cluster */
} /* esp_matter */
