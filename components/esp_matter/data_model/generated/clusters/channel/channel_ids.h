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
namespace channel {

inline constexpr uint32_t Id = 0x0504;

namespace feature {
namespace ChannelList {
inline constexpr uint32_t Id = 0x1;
} /* ChannelList */
namespace LineupInfo {
inline constexpr uint32_t Id = 0x2;
} /* LineupInfo */
namespace ElectronicGuide {
inline constexpr uint32_t Id = 0x4;
} /* ElectronicGuide */
namespace RecordProgram {
inline constexpr uint32_t Id = 0x8;
} /* RecordProgram */
} /* feature */

namespace attribute {
namespace ChannelList {
inline constexpr uint32_t Id = 0x0000;
} /* ChannelList */
namespace Lineup {
inline constexpr uint32_t Id = 0x0001;
} /* Lineup */
namespace CurrentChannel {
inline constexpr uint32_t Id = 0x0002;
} /* CurrentChannel */
} /* attribute */

namespace command {
namespace ChangeChannel {
inline constexpr uint32_t Id = 0x00;
} /* ChangeChannel */
namespace ChangeChannelResponse {
inline constexpr uint32_t Id = 0x01;
} /* ChangeChannelResponse */
namespace ChangeChannelByNumber {
inline constexpr uint32_t Id = 0x02;
} /* ChangeChannelByNumber */
namespace SkipChannel {
inline constexpr uint32_t Id = 0x03;
} /* SkipChannel */
namespace GetProgramGuide {
inline constexpr uint32_t Id = 0x04;
} /* GetProgramGuide */
namespace ProgramGuideResponse {
inline constexpr uint32_t Id = 0x05;
} /* ProgramGuideResponse */
namespace RecordProgram {
inline constexpr uint32_t Id = 0x06;
} /* RecordProgram */
namespace CancelRecordProgram {
inline constexpr uint32_t Id = 0x07;
} /* CancelRecordProgram */
} /* command */

} /* channel */
} /* cluster */
} /* esp_matter */
