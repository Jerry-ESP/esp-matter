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
namespace closure_control {

namespace feature {
namespace positioning {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* positioning */

namespace motion_latching {
typedef struct config {
    uint8_t latch_control_modes;
    config() : latch_control_modes(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* motion_latching */

namespace instantaneous {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* instantaneous */

namespace speed {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* speed */

namespace ventilation {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* ventilation */

namespace pedestrian {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* pedestrian */

namespace calibration {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* calibration */

namespace protection {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* protection */

namespace manually_operable {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* manually_operable */

} /* feature */

namespace attribute {
attribute_t *create_countdown_time(cluster_t *cluster, nullable<uint32_t> value);
attribute_t *create_main_state(cluster_t *cluster, uint8_t value);
attribute_t *create_current_error_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_overall_current_state(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_overall_target_state(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_latch_control_modes(cluster_t *cluster, uint8_t value);
} /* attribute */

namespace command {
command_t *create_stop(cluster_t *cluster);
command_t *create_move_to(cluster_t *cluster);
command_t *create_calibrate(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_operational_error(cluster_t *cluster);
event_t *create_movement_completed(cluster_t *cluster);
event_t *create_engage_state_changed(cluster_t *cluster);
event_t *create_secure_state_changed(cluster_t *cluster);
} /* event */

typedef struct config {
    uint8_t main_state;
    void *delegate;
    struct {
        feature::motion_latching::config_t motion_latching;
    } features;
    uint32_t feature_flags;
    config() : main_state(0), delegate(nullptr), feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* closure_control */
} /* cluster */
} /* esp_matter */
