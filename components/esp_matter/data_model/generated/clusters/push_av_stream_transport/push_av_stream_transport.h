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
namespace push_av_stream_transport {

namespace feature {
namespace per_zone_sensitivity {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* per_zone_sensitivity */

} /* feature */

namespace attribute {
attribute_t *create_supported_formats(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_connections(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace command {
command_t *create_allocate_push_transport(cluster_t *cluster);
command_t *create_allocate_push_transport_response(cluster_t *cluster);
command_t *create_deallocate_push_transport(cluster_t *cluster);
command_t *create_modify_push_transport(cluster_t *cluster);
command_t *create_set_transport_status(cluster_t *cluster);
command_t *create_manually_trigger_transport(cluster_t *cluster);
command_t *create_find_transport(cluster_t *cluster);
command_t *create_find_transport_response(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_push_transport_begin(cluster_t *cluster);
event_t *create_push_transport_end(cluster_t *cluster);
} /* event */

typedef struct config {
    void *delegate;
    config() : delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* push_av_stream_transport */
} /* cluster */
} /* esp_matter */
