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
namespace meter_identification {

namespace feature {
namespace power_threshold {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* power_threshold */

} /* feature */

namespace attribute {
attribute_t *create_meter_type(cluster_t *cluster, nullable<uint8_t> value);
attribute_t *create_point_of_delivery(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_meter_serial_number(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_protocol_version(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_power_threshold(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

typedef struct config {
    config() {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* meter_identification */
} /* cluster */
} /* esp_matter */
