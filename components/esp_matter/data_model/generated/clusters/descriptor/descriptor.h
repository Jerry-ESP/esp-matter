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
namespace descriptor {

const uint8_t k_max_endpoint_unique_id_length = 32u;
namespace feature {
namespace tag_list {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* tag_list */

} /* feature */

namespace attribute {
attribute_t *create_device_type_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_server_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_client_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_parts_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_tag_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
attribute_t *create_endpoint_unique_id(cluster_t *cluster, char * value, uint16_t length);
#endif // CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
} /* attribute */

typedef struct config {
    config() {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* descriptor */
} /* cluster */
} /* esp_matter */
