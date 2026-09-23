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
namespace commodity_tariff {

inline constexpr uint32_t Id = 0x0700;

namespace feature {
namespace Pricing {
inline constexpr uint32_t Id = 0x1;
} /* Pricing */
namespace FriendlyCredit {
inline constexpr uint32_t Id = 0x2;
} /* FriendlyCredit */
namespace AuxiliaryLoad {
inline constexpr uint32_t Id = 0x4;
} /* AuxiliaryLoad */
namespace PeakPeriod {
inline constexpr uint32_t Id = 0x8;
} /* PeakPeriod */
namespace PowerThreshold {
inline constexpr uint32_t Id = 0x10;
} /* PowerThreshold */
namespace Randomization {
inline constexpr uint32_t Id = 0x20;
} /* Randomization */
} /* feature */

namespace attribute {
namespace TariffInfo {
inline constexpr uint32_t Id = 0x0000;
} /* TariffInfo */
namespace TariffUnit {
inline constexpr uint32_t Id = 0x0001;
} /* TariffUnit */
namespace StartDate {
inline constexpr uint32_t Id = 0x0002;
} /* StartDate */
namespace DayEntries {
inline constexpr uint32_t Id = 0x0003;
} /* DayEntries */
namespace DayPatterns {
inline constexpr uint32_t Id = 0x0004;
} /* DayPatterns */
namespace CalendarPeriods {
inline constexpr uint32_t Id = 0x0005;
} /* CalendarPeriods */
namespace IndividualDays {
inline constexpr uint32_t Id = 0x0006;
} /* IndividualDays */
namespace CurrentDay {
inline constexpr uint32_t Id = 0x0007;
} /* CurrentDay */
namespace NextDay {
inline constexpr uint32_t Id = 0x0008;
} /* NextDay */
namespace CurrentDayEntry {
inline constexpr uint32_t Id = 0x0009;
} /* CurrentDayEntry */
namespace CurrentDayEntryDate {
inline constexpr uint32_t Id = 0x000A;
} /* CurrentDayEntryDate */
namespace NextDayEntry {
inline constexpr uint32_t Id = 0x000B;
} /* NextDayEntry */
namespace NextDayEntryDate {
inline constexpr uint32_t Id = 0x000C;
} /* NextDayEntryDate */
namespace TariffComponents {
inline constexpr uint32_t Id = 0x000D;
} /* TariffComponents */
namespace TariffPeriods {
inline constexpr uint32_t Id = 0x000E;
} /* TariffPeriods */
namespace CurrentTariffComponents {
inline constexpr uint32_t Id = 0x000F;
} /* CurrentTariffComponents */
namespace NextTariffComponents {
inline constexpr uint32_t Id = 0x0010;
} /* NextTariffComponents */
namespace DefaultRandomizationOffset {
inline constexpr uint32_t Id = 0x0011;
} /* DefaultRandomizationOffset */
namespace DefaultRandomizationType {
inline constexpr uint32_t Id = 0x0012;
} /* DefaultRandomizationType */
} /* attribute */

namespace command {
namespace GetTariffComponent {
inline constexpr uint32_t Id = 0x00;
} /* GetTariffComponent */
namespace GetTariffComponentResponse {
inline constexpr uint32_t Id = 0x00;
} /* GetTariffComponentResponse */
namespace GetDayEntry {
inline constexpr uint32_t Id = 0x01;
} /* GetDayEntry */
namespace GetDayEntryResponse {
inline constexpr uint32_t Id = 0x01;
} /* GetDayEntryResponse */
} /* command */

} /* commodity_tariff */
} /* cluster */
} /* esp_matter */
