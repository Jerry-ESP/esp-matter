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
namespace push_av_stream_transport {

inline constexpr uint32_t Id = 0x0555;

namespace feature {
namespace PerZoneSensitivity {
inline constexpr uint32_t Id = 0x1;
} /* PerZoneSensitivity */
} /* feature */

namespace attribute {
namespace SupportedFormats {
inline constexpr uint32_t Id = 0x0000;
} /* SupportedFormats */
namespace CurrentConnections {
inline constexpr uint32_t Id = 0x0001;
} /* CurrentConnections */
} /* attribute */

namespace command {
namespace AllocatePushTransport {
inline constexpr uint32_t Id = 0x00;
} /* AllocatePushTransport */
namespace AllocatePushTransportResponse {
inline constexpr uint32_t Id = 0x01;
} /* AllocatePushTransportResponse */
namespace DeallocatePushTransport {
inline constexpr uint32_t Id = 0x02;
} /* DeallocatePushTransport */
namespace ModifyPushTransport {
inline constexpr uint32_t Id = 0x03;
} /* ModifyPushTransport */
namespace SetTransportStatus {
inline constexpr uint32_t Id = 0x04;
} /* SetTransportStatus */
namespace ManuallyTriggerTransport {
inline constexpr uint32_t Id = 0x05;
} /* ManuallyTriggerTransport */
namespace FindTransport {
inline constexpr uint32_t Id = 0x06;
} /* FindTransport */
namespace FindTransportResponse {
inline constexpr uint32_t Id = 0x07;
} /* FindTransportResponse */
} /* command */

namespace event {
namespace PushTransportBegin {
inline constexpr uint32_t Id = 0x00;
} /* PushTransportBegin */
namespace PushTransportEnd {
inline constexpr uint32_t Id = 0x01;
} /* PushTransportEnd */
} /* event */

} /* push_av_stream_transport */
} /* cluster */
} /* esp_matter */
