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
namespace service_area {

inline constexpr uint32_t Id = 0x0150;

namespace feature {
namespace SelectWhileRunning {
inline constexpr uint32_t Id = 0x1;
} /* SelectWhileRunning */
namespace ProgressReporting {
inline constexpr uint32_t Id = 0x2;
} /* ProgressReporting */
namespace Maps {
inline constexpr uint32_t Id = 0x4;
} /* Maps */
} /* feature */

namespace attribute {
namespace SupportedAreas {
inline constexpr uint32_t Id = 0x0000;
} /* SupportedAreas */
namespace SupportedMaps {
inline constexpr uint32_t Id = 0x0001;
} /* SupportedMaps */
namespace SelectedAreas {
inline constexpr uint32_t Id = 0x0002;
} /* SelectedAreas */
namespace CurrentArea {
inline constexpr uint32_t Id = 0x0003;
} /* CurrentArea */
namespace EstimatedEndTime {
inline constexpr uint32_t Id = 0x0004;
} /* EstimatedEndTime */
namespace Progress {
inline constexpr uint32_t Id = 0x0005;
} /* Progress */
} /* attribute */

namespace command {
namespace SelectAreas {
inline constexpr uint32_t Id = 0x00;
} /* SelectAreas */
namespace SelectAreasResponse {
inline constexpr uint32_t Id = 0x01;
} /* SelectAreasResponse */
namespace SkipArea {
inline constexpr uint32_t Id = 0x02;
} /* SkipArea */
namespace SkipAreaResponse {
inline constexpr uint32_t Id = 0x03;
} /* SkipAreaResponse */
} /* command */

} /* service_area */
} /* cluster */
} /* esp_matter */
