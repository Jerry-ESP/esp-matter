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
namespace thread_border_router_management {

inline constexpr uint32_t Id = 0x0452;

namespace feature {
namespace PANChange {
inline constexpr uint32_t Id = 0x1;
} /* PANChange */
} /* feature */

namespace attribute {
namespace BorderRouterName {
inline constexpr uint32_t Id = 0x0000;
} /* BorderRouterName */
namespace BorderAgentID {
inline constexpr uint32_t Id = 0x0001;
} /* BorderAgentID */
namespace ThreadVersion {
inline constexpr uint32_t Id = 0x0002;
} /* ThreadVersion */
namespace InterfaceEnabled {
inline constexpr uint32_t Id = 0x0003;
} /* InterfaceEnabled */
namespace ActiveDatasetTimestamp {
inline constexpr uint32_t Id = 0x0004;
} /* ActiveDatasetTimestamp */
namespace PendingDatasetTimestamp {
inline constexpr uint32_t Id = 0x0005;
} /* PendingDatasetTimestamp */
} /* attribute */

namespace command {
namespace GetActiveDatasetRequest {
inline constexpr uint32_t Id = 0x00;
} /* GetActiveDatasetRequest */
namespace GetPendingDatasetRequest {
inline constexpr uint32_t Id = 0x01;
} /* GetPendingDatasetRequest */
namespace DatasetResponse {
inline constexpr uint32_t Id = 0x02;
} /* DatasetResponse */
namespace SetActiveDatasetRequest {
inline constexpr uint32_t Id = 0x03;
} /* SetActiveDatasetRequest */
namespace SetPendingDatasetRequest {
inline constexpr uint32_t Id = 0x04;
} /* SetPendingDatasetRequest */
} /* command */

} /* thread_border_router_management */
} /* cluster */
} /* esp_matter */
