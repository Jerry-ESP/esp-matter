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
namespace actions {

inline constexpr uint32_t Id = 0x0025;

namespace attribute {
namespace ActionList {
inline constexpr uint32_t Id = 0x0000;
} /* ActionList */
namespace EndpointLists {
inline constexpr uint32_t Id = 0x0001;
} /* EndpointLists */
namespace SetupURL {
inline constexpr uint32_t Id = 0x0002;
} /* SetupURL */
} /* attribute */

namespace command {
namespace InstantAction {
inline constexpr uint32_t Id = 0x00;
} /* InstantAction */
namespace InstantActionWithTransition {
inline constexpr uint32_t Id = 0x01;
} /* InstantActionWithTransition */
namespace StartAction {
inline constexpr uint32_t Id = 0x02;
} /* StartAction */
namespace StartActionWithDuration {
inline constexpr uint32_t Id = 0x03;
} /* StartActionWithDuration */
namespace StopAction {
inline constexpr uint32_t Id = 0x04;
} /* StopAction */
namespace PauseAction {
inline constexpr uint32_t Id = 0x05;
} /* PauseAction */
namespace PauseActionWithDuration {
inline constexpr uint32_t Id = 0x06;
} /* PauseActionWithDuration */
namespace ResumeAction {
inline constexpr uint32_t Id = 0x07;
} /* ResumeAction */
namespace EnableAction {
inline constexpr uint32_t Id = 0x08;
} /* EnableAction */
namespace EnableActionWithDuration {
inline constexpr uint32_t Id = 0x09;
} /* EnableActionWithDuration */
namespace DisableAction {
inline constexpr uint32_t Id = 0x0A;
} /* DisableAction */
namespace DisableActionWithDuration {
inline constexpr uint32_t Id = 0x0B;
} /* DisableActionWithDuration */
} /* command */

namespace event {
namespace StateChanged {
inline constexpr uint32_t Id = 0x00;
} /* StateChanged */
namespace ActionFailed {
inline constexpr uint32_t Id = 0x01;
} /* ActionFailed */
} /* event */

} /* actions */
} /* cluster */
} /* esp_matter */
