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
namespace temperature_alarm {

namespace feature {
namespace reset {
typedef struct config {
    uint32_t latch;
    config() : latch(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* reset */

namespace over_temperature {
typedef struct config {
    int16_t critical_over_temperature_threshold;
    int16_t major_over_temperature_threshold;
    int16_t minor_over_temperature_threshold;
    config() : critical_over_temperature_threshold(0), major_over_temperature_threshold(0), minor_over_temperature_threshold(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* over_temperature */

namespace under_temperature {
typedef struct config {
    int16_t minor_under_temperature_threshold;
    int16_t major_under_temperature_threshold;
    int16_t critical_under_temperature_threshold;
    config() : minor_under_temperature_threshold(0), major_under_temperature_threshold(0), critical_under_temperature_threshold(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* under_temperature */

namespace major_threshold {
typedef struct config {
    int16_t major_over_temperature_threshold;
    int16_t major_under_temperature_threshold;
    config() : major_over_temperature_threshold(0), major_under_temperature_threshold(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* major_threshold */

namespace minor_threshold {
typedef struct config {
    int16_t minor_over_temperature_threshold;
    int16_t minor_under_temperature_threshold;
    config() : minor_over_temperature_threshold(0), minor_under_temperature_threshold(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* minor_threshold */

namespace over_critical_adjustable {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* over_critical_adjustable */

namespace over_major_adjustable {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* over_major_adjustable */

namespace over_minor_adjustable {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* over_minor_adjustable */

namespace under_minor_adjustable {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* under_minor_adjustable */

namespace under_major_adjustable {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* under_major_adjustable */

namespace under_critical_adjustable {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* under_critical_adjustable */

} /* feature */

namespace attribute {
attribute_t *create_mask(cluster_t *cluster, uint32_t value);
attribute_t *create_latch(cluster_t *cluster, uint32_t value);
attribute_t *create_state(cluster_t *cluster, uint32_t value);
attribute_t *create_supported(cluster_t *cluster, uint32_t value);
attribute_t *create_critical_over_temperature_threshold(cluster_t *cluster, int16_t value);
attribute_t *create_major_over_temperature_threshold(cluster_t *cluster, int16_t value);
attribute_t *create_minor_over_temperature_threshold(cluster_t *cluster, int16_t value);
attribute_t *create_minor_under_temperature_threshold(cluster_t *cluster, int16_t value);
attribute_t *create_major_under_temperature_threshold(cluster_t *cluster, int16_t value);
attribute_t *create_critical_under_temperature_threshold(cluster_t *cluster, int16_t value);
} /* attribute */

namespace command {
command_t *create_reset(cluster_t *cluster);
command_t *create_modify_enabled_alarms(cluster_t *cluster);
command_t *create_set_temperature_alarm_thresholds(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_notify(cluster_t *cluster);
} /* event */

typedef struct config {
    uint32_t mask;
    uint32_t state;
    uint32_t supported;
    struct {
        feature::reset::config_t reset;
        feature::over_temperature::config_t over_temperature;
        feature::under_temperature::config_t under_temperature;
        feature::major_threshold::config_t major_threshold;
        feature::minor_threshold::config_t minor_threshold;
    } features;
    uint32_t feature_flags;
    config() : mask(0), state(0), supported(0), feature_flags(0) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* temperature_alarm */
} /* cluster */
} /* esp_matter */
