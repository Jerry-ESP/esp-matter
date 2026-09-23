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
namespace keypad_input {

namespace feature {
namespace navigation_key_codes {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* navigation_key_codes */

namespace location_keys {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* location_keys */

namespace number_keys {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* number_keys */

} /* feature */

namespace command {
command_t *create_send_key(cluster_t *cluster);
command_t *create_send_key_response(cluster_t *cluster);
} /* command */

typedef struct config {
    void *delegate;
    config() : delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* keypad_input */
} /* cluster */
} /* esp_matter */
