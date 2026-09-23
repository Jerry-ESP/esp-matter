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
namespace commissioner_control {

namespace attribute {
attribute_t *create_supported_device_categories(cluster_t *cluster, uint32_t value);
} /* attribute */

namespace command {
command_t *create_request_commissioning_approval(cluster_t *cluster);
command_t *create_commission_node(cluster_t *cluster);
command_t *create_reverse_open_commissioning_window(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_commissioning_request_result(cluster_t *cluster);
} /* event */

typedef struct config {
    uint32_t supported_device_categories;
    void *delegate;
    config() : supported_device_categories(0), delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* commissioner_control */
} /* cluster */
} /* esp_matter */
