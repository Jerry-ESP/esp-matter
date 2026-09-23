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
namespace service_area {

namespace feature {
namespace select_while_running {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* select_while_running */

namespace progress_reporting {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* progress_reporting */

namespace maps {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* maps */

} /* feature */

namespace attribute {
attribute_t *create_supported_areas(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_supported_maps(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_selected_areas(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_area(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_estimated_end_time(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_progress(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace command {
command_t *create_select_areas(cluster_t *cluster);
command_t *create_select_areas_response(cluster_t *cluster);
command_t *create_skip_area(cluster_t *cluster);
command_t *create_skip_area_response(cluster_t *cluster);
} /* command */

typedef struct config {
    void *delegate;
    config() : delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* service_area */
} /* cluster */
} /* esp_matter */
