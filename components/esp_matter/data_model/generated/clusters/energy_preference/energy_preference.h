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
namespace energy_preference {

namespace feature {
namespace energy_balance {
typedef struct config {
    uint8_t current_energy_balance;
    config() : current_energy_balance(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* energy_balance */

namespace low_power_mode_sensitivity {
typedef struct config {
    uint8_t current_low_power_mode_sensitivity;
    config() : current_low_power_mode_sensitivity(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* low_power_mode_sensitivity */

} /* feature */

namespace attribute {
attribute_t *create_energy_balances(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_energy_balance(cluster_t *cluster, uint8_t value);
attribute_t *create_energy_priorities(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_low_power_mode_sensitivities(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_current_low_power_mode_sensitivity(cluster_t *cluster, uint8_t value);
} /* attribute */

typedef struct config {
    void *delegate;
    struct {
        feature::energy_balance::config_t energy_balance;
        feature::low_power_mode_sensitivity::config_t low_power_mode_sensitivity;
    } features;
    uint32_t feature_flags;
    config() : delegate(nullptr), feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* energy_preference */
} /* cluster */
} /* esp_matter */
