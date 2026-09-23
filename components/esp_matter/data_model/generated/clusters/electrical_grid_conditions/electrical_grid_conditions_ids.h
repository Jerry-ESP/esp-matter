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
namespace electrical_grid_conditions {

inline constexpr uint32_t Id = 0x00A0;

namespace feature {
namespace Forecasting {
inline constexpr uint32_t Id = 0x1;
} /* Forecasting */
} /* feature */

namespace attribute {
namespace LocalGenerationAvailable {
inline constexpr uint32_t Id = 0x0000;
} /* LocalGenerationAvailable */
namespace CurrentConditions {
inline constexpr uint32_t Id = 0x0001;
} /* CurrentConditions */
namespace ForecastConditions {
inline constexpr uint32_t Id = 0x0002;
} /* ForecastConditions */
} /* attribute */

namespace event {
namespace CurrentConditionsChanged {
inline constexpr uint32_t Id = 0x00;
} /* CurrentConditionsChanged */
} /* event */

} /* electrical_grid_conditions */
} /* cluster */
} /* esp_matter */
