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
namespace air_quality {

namespace feature {
namespace fair {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* fair */

namespace moderate {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* moderate */

namespace very_poor {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* very_poor */

namespace extremely_poor {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* extremely_poor */

} /* feature */

namespace attribute {
attribute_t *create_air_quality(cluster_t *cluster, uint8_t value);
} /* attribute */

typedef struct config {
    uint8_t air_quality;
    config() : air_quality(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* air_quality */
} /* cluster */
} /* esp_matter */
