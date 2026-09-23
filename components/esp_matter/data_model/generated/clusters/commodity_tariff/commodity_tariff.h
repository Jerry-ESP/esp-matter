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
namespace commodity_tariff {

namespace feature {
namespace pricing {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* pricing */

namespace friendly_credit {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* friendly_credit */

namespace auxiliary_load {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* auxiliary_load */

namespace peak_period {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* peak_period */

namespace power_threshold {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* power_threshold */

namespace randomization {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* randomization */

} /* feature */

namespace attribute {
attribute_t *create_tariff_info(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_tariff_unit(cluster_t *cluster, nullable<uint8_t> value);
attribute_t *create_start_date(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_day_entries(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_day_patterns(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_calendar_periods(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_individual_days(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_day(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_next_day(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_day_entry(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_day_entry_date(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_next_day_entry(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_next_day_entry_date(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_tariff_components(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_tariff_periods(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_tariff_components(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_next_tariff_components(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_default_randomization_offset(cluster_t *cluster, nullable<int16_t> value);
attribute_t *create_default_randomization_type(cluster_t *cluster, nullable<uint8_t> value);
} /* attribute */

namespace command {
command_t *create_get_tariff_component(cluster_t *cluster);
command_t *create_get_tariff_component_response(cluster_t *cluster);
command_t *create_get_day_entry(cluster_t *cluster);
command_t *create_get_day_entry_response(cluster_t *cluster);
} /* command */

typedef struct config {
    void *delegate;
    uint32_t feature_flags;
    config() : delegate(nullptr), feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* commodity_tariff */
} /* cluster */
} /* esp_matter */
