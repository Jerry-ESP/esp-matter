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
namespace ota_software_update_provider {

inline constexpr uint32_t Id = 0x0029;

namespace command {
namespace QueryImage {
inline constexpr uint32_t Id = 0x00;
} /* QueryImage */
namespace QueryImageResponse {
inline constexpr uint32_t Id = 0x01;
} /* QueryImageResponse */
namespace ApplyUpdateRequest {
inline constexpr uint32_t Id = 0x02;
} /* ApplyUpdateRequest */
namespace ApplyUpdateResponse {
inline constexpr uint32_t Id = 0x03;
} /* ApplyUpdateResponse */
namespace NotifyUpdateApplied {
inline constexpr uint32_t Id = 0x04;
} /* NotifyUpdateApplied */
} /* command */

} /* ota_software_update_provider */
} /* cluster */
} /* esp_matter */
