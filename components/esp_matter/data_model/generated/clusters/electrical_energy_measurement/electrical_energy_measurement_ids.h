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
namespace electrical_energy_measurement {

inline constexpr uint32_t Id = 0x0091;

namespace feature {
namespace ImportedEnergy {
inline constexpr uint32_t Id = 0x1;
} /* ImportedEnergy */
namespace ExportedEnergy {
inline constexpr uint32_t Id = 0x2;
} /* ExportedEnergy */
namespace CumulativeEnergy {
inline constexpr uint32_t Id = 0x4;
} /* CumulativeEnergy */
namespace PeriodicEnergy {
inline constexpr uint32_t Id = 0x8;
} /* PeriodicEnergy */
} /* feature */

namespace attribute {
namespace Accuracy {
inline constexpr uint32_t Id = 0x0000;
} /* Accuracy */
namespace CumulativeEnergyImported {
inline constexpr uint32_t Id = 0x0001;
} /* CumulativeEnergyImported */
namespace CumulativeEnergyExported {
inline constexpr uint32_t Id = 0x0002;
} /* CumulativeEnergyExported */
namespace PeriodicEnergyImported {
inline constexpr uint32_t Id = 0x0003;
} /* PeriodicEnergyImported */
namespace PeriodicEnergyExported {
inline constexpr uint32_t Id = 0x0004;
} /* PeriodicEnergyExported */
namespace CumulativeEnergyReset {
inline constexpr uint32_t Id = 0x0005;
} /* CumulativeEnergyReset */
} /* attribute */

namespace event {
namespace CumulativeEnergyMeasured {
inline constexpr uint32_t Id = 0x00;
} /* CumulativeEnergyMeasured */
namespace PeriodicEnergyMeasured {
inline constexpr uint32_t Id = 0x01;
} /* PeriodicEnergyMeasured */
} /* event */

} /* electrical_energy_measurement */
} /* cluster */
} /* esp_matter */
