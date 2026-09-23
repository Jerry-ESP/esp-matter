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
namespace hepa_filter_monitoring {

namespace feature {
namespace condition {
typedef struct config {
    uint8_t condition;
    uint8_t degradation_direction;
    config() : condition(0), degradation_direction(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* condition */

namespace warning {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* warning */

namespace replacement_product_list {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* replacement_product_list */

} /* feature */

namespace attribute {
attribute_t *create_condition(cluster_t *cluster, uint8_t value);
attribute_t *create_degradation_direction(cluster_t *cluster, uint8_t value);
attribute_t *create_change_indication(cluster_t *cluster, uint8_t value);
attribute_t *create_in_place_indicator(cluster_t *cluster, bool value);
attribute_t *create_last_changed_time(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_replacement_product_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace command {
command_t *create_reset_condition(cluster_t *cluster);
} /* command */

typedef struct config {
    uint8_t change_indication;
    config() : change_indication(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* hepa_filter_monitoring */
} /* cluster */
} /* esp_matter */
