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
namespace meter_identification {

inline constexpr uint32_t Id = 0x0B06;

namespace feature {
namespace PowerThreshold {
inline constexpr uint32_t Id = 0x1;
} /* PowerThreshold */
} /* feature */

namespace attribute {
namespace MeterType {
inline constexpr uint32_t Id = 0x0000;
} /* MeterType */
namespace PointOfDelivery {
inline constexpr uint32_t Id = 0x0001;
} /* PointOfDelivery */
namespace MeterSerialNumber {
inline constexpr uint32_t Id = 0x0002;
} /* MeterSerialNumber */
namespace ProtocolVersion {
inline constexpr uint32_t Id = 0x0003;
} /* ProtocolVersion */
namespace PowerThreshold {
inline constexpr uint32_t Id = 0x0004;
} /* PowerThreshold */
} /* attribute */

} /* meter_identification */
} /* cluster */
} /* esp_matter */
