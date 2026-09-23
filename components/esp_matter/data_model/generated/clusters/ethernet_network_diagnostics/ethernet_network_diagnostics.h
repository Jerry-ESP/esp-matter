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
namespace ethernet_network_diagnostics {

namespace feature {
namespace packet_counts {
typedef struct config {
    uint64_t packet_rx_count;
    uint64_t packet_tx_count;
    config() : packet_rx_count(0), packet_tx_count(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* packet_counts */

namespace error_counts {
typedef struct config {
    uint64_t tx_err_count;
    uint64_t collision_count;
    uint64_t overrun_count;
    config() : tx_err_count(0), collision_count(0), overrun_count(0) {}
} config_t;
uint32_t get_id();
esp_err_t add(cluster_t *cluster, config_t *config);
} /* error_counts */

} /* feature */

namespace attribute {
attribute_t *create_phy_rate(cluster_t *cluster, nullable<uint8_t> value);
attribute_t *create_full_duplex(cluster_t *cluster, nullable<bool> value);
attribute_t *create_packet_rx_count(cluster_t *cluster, uint64_t value);
attribute_t *create_packet_tx_count(cluster_t *cluster, uint64_t value);
attribute_t *create_tx_err_count(cluster_t *cluster, uint64_t value);
attribute_t *create_collision_count(cluster_t *cluster, uint64_t value);
attribute_t *create_overrun_count(cluster_t *cluster, uint64_t value);
attribute_t *create_carrier_detect(cluster_t *cluster, nullable<bool> value);
attribute_t *create_time_since_reset(cluster_t *cluster, uint64_t value);
} /* attribute */

namespace command {
command_t *create_reset_counts(cluster_t *cluster);
} /* command */

typedef struct config {
    config() {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* ethernet_network_diagnostics */
} /* cluster */
} /* esp_matter */
