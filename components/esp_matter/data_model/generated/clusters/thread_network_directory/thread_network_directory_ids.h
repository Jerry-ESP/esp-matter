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
namespace thread_network_directory {

inline constexpr uint32_t Id = 0x0453;

namespace attribute {
namespace PreferredExtendedPanID {
inline constexpr uint32_t Id = 0x0000;
} /* PreferredExtendedPanID */
namespace ThreadNetworks {
inline constexpr uint32_t Id = 0x0001;
} /* ThreadNetworks */
namespace ThreadNetworkTableSize {
inline constexpr uint32_t Id = 0x0002;
} /* ThreadNetworkTableSize */
} /* attribute */

namespace command {
namespace AddNetwork {
inline constexpr uint32_t Id = 0x00;
} /* AddNetwork */
namespace RemoveNetwork {
inline constexpr uint32_t Id = 0x01;
} /* RemoveNetwork */
namespace GetOperationalDataset {
inline constexpr uint32_t Id = 0x02;
} /* GetOperationalDataset */
namespace OperationalDatasetResponse {
inline constexpr uint32_t Id = 0x03;
} /* OperationalDatasetResponse */
} /* command */

} /* thread_network_directory */
} /* cluster */
} /* esp_matter */
