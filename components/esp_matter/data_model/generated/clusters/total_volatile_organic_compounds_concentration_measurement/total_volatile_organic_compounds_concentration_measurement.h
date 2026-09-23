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
namespace total_volatile_organic_compounds_concentration_measurement {

namespace feature {
namespace numeric_measurement {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* numeric_measurement */

namespace level_indication {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* level_indication */

namespace medium_level {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* medium_level */

namespace critical_level {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* critical_level */

namespace peak_measurement {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* peak_measurement */

namespace average_measurement {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* average_measurement */

} /* feature */

namespace attribute {
attribute_t *create_measured_value(cluster_t *cluster, nullable<float> value);
attribute_t *create_min_measured_value(cluster_t *cluster, nullable<float> value);
attribute_t *create_max_measured_value(cluster_t *cluster, nullable<float> value);
attribute_t *create_peak_measured_value(cluster_t *cluster, nullable<float> value);
attribute_t *create_peak_measured_value_window(cluster_t *cluster, uint32_t value);
attribute_t *create_average_measured_value(cluster_t *cluster, nullable<float> value);
attribute_t *create_average_measured_value_window(cluster_t *cluster, uint32_t value);
attribute_t *create_uncertainty(cluster_t *cluster, float value);
attribute_t *create_measurement_unit(cluster_t *cluster, uint8_t value);
attribute_t *create_measurement_medium(cluster_t *cluster, uint8_t value);
attribute_t *create_level_value(cluster_t *cluster, uint8_t value);
} /* attribute */

typedef struct config {
    uint32_t feature_flags;
    config() : feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* total_volatile_organic_compounds_concentration_measurement */
} /* cluster */
} /* esp_matter */
