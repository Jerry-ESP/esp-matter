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
namespace scenes_management {

inline constexpr uint32_t Id = 0x0062;

namespace feature {
namespace SceneNames {
inline constexpr uint32_t Id = 0x1;
} /* SceneNames */
} /* feature */

namespace attribute {
namespace SceneTableSize {
inline constexpr uint32_t Id = 0x0001;
} /* SceneTableSize */
namespace FabricSceneInfo {
inline constexpr uint32_t Id = 0x0002;
} /* FabricSceneInfo */
} /* attribute */

namespace command {
namespace AddScene {
inline constexpr uint32_t Id = 0x00;
} /* AddScene */
namespace AddSceneResponse {
inline constexpr uint32_t Id = 0x00;
} /* AddSceneResponse */
namespace ViewScene {
inline constexpr uint32_t Id = 0x01;
} /* ViewScene */
namespace ViewSceneResponse {
inline constexpr uint32_t Id = 0x01;
} /* ViewSceneResponse */
namespace RemoveScene {
inline constexpr uint32_t Id = 0x02;
} /* RemoveScene */
namespace RemoveSceneResponse {
inline constexpr uint32_t Id = 0x02;
} /* RemoveSceneResponse */
namespace RemoveAllScenes {
inline constexpr uint32_t Id = 0x03;
} /* RemoveAllScenes */
namespace RemoveAllScenesResponse {
inline constexpr uint32_t Id = 0x03;
} /* RemoveAllScenesResponse */
namespace StoreScene {
inline constexpr uint32_t Id = 0x04;
} /* StoreScene */
namespace StoreSceneResponse {
inline constexpr uint32_t Id = 0x04;
} /* StoreSceneResponse */
namespace RecallScene {
inline constexpr uint32_t Id = 0x05;
} /* RecallScene */
namespace GetSceneMembership {
inline constexpr uint32_t Id = 0x06;
} /* GetSceneMembership */
namespace GetSceneMembershipResponse {
inline constexpr uint32_t Id = 0x06;
} /* GetSceneMembershipResponse */
namespace CopyScene {
inline constexpr uint32_t Id = 0x40;
} /* CopyScene */
namespace CopySceneResponse {
inline constexpr uint32_t Id = 0x40;
} /* CopySceneResponse */
} /* command */

} /* scenes_management */
} /* cluster */
} /* esp_matter */
