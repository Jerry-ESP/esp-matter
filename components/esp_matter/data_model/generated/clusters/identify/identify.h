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
namespace identify {

namespace attribute {
attribute_t *create_identify_time(cluster_t *cluster, uint16_t value);
attribute_t *create_identify_type(cluster_t *cluster, uint8_t value);
} /* attribute */

namespace command {
command_t *create_identify(cluster_t *cluster);
command_t *create_trigger_effect(cluster_t *cluster);
} /* command */

typedef struct config {
    uint16_t identify_time;
    uint8_t identify_type;
    config() : identify_time(0), identify_type(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* identify */
} /* cluster */
} /* esp_matter */
