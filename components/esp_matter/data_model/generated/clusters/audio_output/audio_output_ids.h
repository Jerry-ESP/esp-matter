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
namespace audio_output {

inline constexpr uint32_t Id = 0x050B;

namespace feature {
namespace NameUpdates {
inline constexpr uint32_t Id = 0x1;
} /* NameUpdates */
} /* feature */

namespace attribute {
namespace OutputList {
inline constexpr uint32_t Id = 0x0000;
} /* OutputList */
namespace CurrentOutput {
inline constexpr uint32_t Id = 0x0001;
} /* CurrentOutput */
} /* attribute */

namespace command {
namespace SelectOutput {
inline constexpr uint32_t Id = 0x00;
} /* SelectOutput */
namespace RenameOutput {
inline constexpr uint32_t Id = 0x01;
} /* RenameOutput */
} /* command */

} /* audio_output */
} /* cluster */
} /* esp_matter */
