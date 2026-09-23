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
namespace software_diagnostics {

inline constexpr uint32_t Id = 0x0034;

namespace feature {
namespace Watermarks {
inline constexpr uint32_t Id = 0x1;
} /* Watermarks */
} /* feature */

namespace attribute {
namespace ThreadMetrics {
inline constexpr uint32_t Id = 0x0000;
} /* ThreadMetrics */
namespace CurrentHeapFree {
inline constexpr uint32_t Id = 0x0001;
} /* CurrentHeapFree */
namespace CurrentHeapUsed {
inline constexpr uint32_t Id = 0x0002;
} /* CurrentHeapUsed */
namespace CurrentHeapHighWatermark {
inline constexpr uint32_t Id = 0x0003;
} /* CurrentHeapHighWatermark */
} /* attribute */

namespace command {
namespace ResetWatermarks {
inline constexpr uint32_t Id = 0x00;
} /* ResetWatermarks */
} /* command */

namespace event {
namespace SoftwareFault {
inline constexpr uint32_t Id = 0x00;
} /* SoftwareFault */
} /* event */

} /* software_diagnostics */
} /* cluster */
} /* esp_matter */
