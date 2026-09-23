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
namespace messages {

inline constexpr uint32_t Id = 0x0097;

namespace feature {
namespace ReceivedConfirmation {
inline constexpr uint32_t Id = 0x1;
} /* ReceivedConfirmation */
namespace ConfirmationResponse {
inline constexpr uint32_t Id = 0x2;
} /* ConfirmationResponse */
namespace ConfirmationReply {
inline constexpr uint32_t Id = 0x4;
} /* ConfirmationReply */
namespace ProtectedMessages {
inline constexpr uint32_t Id = 0x8;
} /* ProtectedMessages */
} /* feature */

namespace attribute {
namespace Messages {
inline constexpr uint32_t Id = 0x0000;
} /* Messages */
namespace ActiveMessageIDs {
inline constexpr uint32_t Id = 0x0001;
} /* ActiveMessageIDs */
} /* attribute */

namespace command {
namespace PresentMessagesRequest {
inline constexpr uint32_t Id = 0x00;
} /* PresentMessagesRequest */
namespace CancelMessagesRequest {
inline constexpr uint32_t Id = 0x01;
} /* CancelMessagesRequest */
} /* command */

namespace event {
namespace MessageQueued {
inline constexpr uint32_t Id = 0x00;
} /* MessageQueued */
namespace MessagePresented {
inline constexpr uint32_t Id = 0x01;
} /* MessagePresented */
namespace MessageComplete {
inline constexpr uint32_t Id = 0x02;
} /* MessageComplete */
} /* event */

} /* messages */
} /* cluster */
} /* esp_matter */
