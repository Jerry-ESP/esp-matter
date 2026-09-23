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
namespace time_format_localization {

namespace feature {
namespace calendar_format {
typedef struct config {
    uint8_t active_calendar_type;
    config() : active_calendar_type(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* calendar_format */

} /* feature */

namespace attribute {
attribute_t *create_hour_format(cluster_t *cluster, uint8_t value);
attribute_t *create_active_calendar_type(cluster_t *cluster, uint8_t value);
attribute_t *create_supported_calendar_types(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

typedef struct config {
    uint8_t hour_format;
    config() : hour_format(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* time_format_localization */
} /* cluster */
} /* esp_matter */
