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
namespace device_energy_management {

namespace feature {
namespace power_adjustment {
typedef struct config {
    uint8_t opt_out_state;
    config() : opt_out_state(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* power_adjustment */

namespace power_forecast_reporting {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* power_forecast_reporting */

namespace state_forecast_reporting {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* state_forecast_reporting */

namespace start_time_adjustment {
typedef struct config {
    uint8_t opt_out_state;
    config() : opt_out_state(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* start_time_adjustment */

namespace pausable {
typedef struct config {
    uint8_t opt_out_state;
    config() : opt_out_state(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* pausable */

namespace forecast_adjustment {
typedef struct config {
    uint8_t opt_out_state;
    config() : opt_out_state(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* forecast_adjustment */

namespace constraint_based_adjustment {
typedef struct config {
    uint8_t opt_out_state;
    config() : opt_out_state(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* constraint_based_adjustment */

} /* feature */

namespace attribute {
attribute_t *create_esa_type(cluster_t *cluster, uint8_t value);
attribute_t *create_esa_can_generate(cluster_t *cluster, bool value);
attribute_t *create_esa_state(cluster_t *cluster, uint8_t value);
attribute_t *create_abs_min_power(cluster_t *cluster, int64_t value);
attribute_t *create_abs_max_power(cluster_t *cluster, int64_t value);
attribute_t *create_power_adjustment_capability(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_forecast(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_opt_out_state(cluster_t *cluster, uint8_t value);
} /* attribute */

namespace command {
command_t *create_power_adjust_request(cluster_t *cluster);
command_t *create_cancel_power_adjust_request(cluster_t *cluster);
command_t *create_start_time_adjust_request(cluster_t *cluster);
command_t *create_pause_request(cluster_t *cluster);
command_t *create_resume_request(cluster_t *cluster);
command_t *create_modify_forecast_request(cluster_t *cluster);
command_t *create_request_constraint_based_forecast(cluster_t *cluster);
command_t *create_cancel_request(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_power_adjust_start(cluster_t *cluster);
event_t *create_power_adjust_end(cluster_t *cluster);
event_t *create_paused(cluster_t *cluster);
event_t *create_resumed(cluster_t *cluster);
} /* event */

typedef struct config {
    uint8_t esa_type;
    bool esa_can_generate;
    uint8_t esa_state;
    int64_t abs_min_power;
    int64_t abs_max_power;
    void *delegate;
    struct {
        feature::power_adjustment::config_t power_adjustment;
        feature::start_time_adjustment::config_t start_time_adjustment;
        feature::pausable::config_t pausable;
        feature::forecast_adjustment::config_t forecast_adjustment;
        feature::constraint_based_adjustment::config_t constraint_based_adjustment;
    } features;
    uint32_t feature_flags;
    config() : esa_type(0), esa_can_generate(false), esa_state(0), abs_min_power(0), abs_max_power(0), delegate(nullptr), feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* device_energy_management */
} /* cluster */
} /* esp_matter */
