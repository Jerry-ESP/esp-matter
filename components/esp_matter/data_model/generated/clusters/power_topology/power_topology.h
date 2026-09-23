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
namespace power_topology {

namespace feature {
namespace node_topology {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* node_topology */

namespace tree_topology {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* tree_topology */

namespace set_topology {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* set_topology */

namespace dynamic_power_flow {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* dynamic_power_flow */

} /* feature */

namespace attribute {
attribute_t *create_available_endpoints(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_active_endpoints(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

typedef struct config {
    void *delegate;
    uint32_t feature_flags;
    config() : delegate(nullptr), feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* power_topology */
} /* cluster */
} /* esp_matter */
