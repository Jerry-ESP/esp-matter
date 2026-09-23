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
namespace content_control {

const uint8_t k_max_on_demand_rating_threshold_length = 8u;
const uint8_t k_max_scheduled_content_rating_threshold_length = 8u;
namespace feature {
namespace screen_time {
typedef struct config {
    uint32_t screen_daily_time;
    uint32_t remaining_screen_time;
    config() : screen_daily_time(0), remaining_screen_time(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* screen_time */

namespace pin_management {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* pin_management */

namespace block_unrated {
typedef struct config {
    bool block_unrated;
    config() : block_unrated(false) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* block_unrated */

namespace on_demand_content_rating {
typedef struct config {
    char on_demand_rating_threshold[k_max_on_demand_rating_threshold_length + 1];
    config() {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* on_demand_content_rating */

namespace scheduled_content_rating {
typedef struct config {
    char scheduled_content_rating_threshold[k_max_scheduled_content_rating_threshold_length + 1];
    config() {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* scheduled_content_rating */

namespace block_channels {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* block_channels */

namespace block_applications {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* block_applications */

namespace block_content_time_window {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* block_content_time_window */

} /* feature */

namespace attribute {
attribute_t *create_enabled(cluster_t *cluster, bool value);
attribute_t *create_on_demand_ratings(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_on_demand_rating_threshold(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_scheduled_content_ratings(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_scheduled_content_rating_threshold(cluster_t *cluster, char * value, uint16_t length);
attribute_t *create_screen_daily_time(cluster_t *cluster, uint32_t value);
attribute_t *create_remaining_screen_time(cluster_t *cluster, uint32_t value);
attribute_t *create_block_unrated(cluster_t *cluster, bool value);
attribute_t *create_block_channel_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_block_application_list(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_block_content_time_window(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace command {
command_t *create_update_pin(cluster_t *cluster);
command_t *create_reset_pin(cluster_t *cluster);
command_t *create_reset_pin_response(cluster_t *cluster);
command_t *create_enable(cluster_t *cluster);
command_t *create_disable(cluster_t *cluster);
command_t *create_add_bonus_time(cluster_t *cluster);
command_t *create_set_screen_daily_time(cluster_t *cluster);
command_t *create_block_unrated_content(cluster_t *cluster);
command_t *create_unblock_unrated_content(cluster_t *cluster);
command_t *create_set_on_demand_rating_threshold(cluster_t *cluster);
command_t *create_set_scheduled_content_rating_threshold(cluster_t *cluster);
command_t *create_add_block_channels(cluster_t *cluster);
command_t *create_remove_block_channels(cluster_t *cluster);
command_t *create_add_block_applications(cluster_t *cluster);
command_t *create_remove_block_applications(cluster_t *cluster);
command_t *create_set_block_content_time_window(cluster_t *cluster);
command_t *create_remove_block_content_time_window(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_remaining_screen_time_expired(cluster_t *cluster);
event_t *create_entering_block_content_time_window(cluster_t *cluster);
} /* event */

typedef struct config {
    bool enabled;
    void *delegate;
    config() : enabled(false), delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* content_control */
} /* cluster */
} /* esp_matter */
