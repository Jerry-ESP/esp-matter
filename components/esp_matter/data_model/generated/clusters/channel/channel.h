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
namespace channel {

namespace feature {
namespace channel_list {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* channel_list */

namespace lineup_info {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* lineup_info */

namespace electronic_guide {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* electronic_guide */

namespace record_program {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* record_program */

} /* feature */

namespace attribute {
attribute_t *create_channel_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_lineup(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_channel(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace command {
command_t *create_change_channel(cluster_t *cluster);
command_t *create_change_channel_response(cluster_t *cluster);
command_t *create_change_channel_by_number(cluster_t *cluster);
command_t *create_skip_channel(cluster_t *cluster);
command_t *create_get_program_guide(cluster_t *cluster);
command_t *create_program_guide_response(cluster_t *cluster);
command_t *create_record_program(cluster_t *cluster);
command_t *create_cancel_record_program(cluster_t *cluster);
} /* command */

typedef struct config {
    void *delegate;
    config() : delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* channel */
} /* cluster */
} /* esp_matter */
