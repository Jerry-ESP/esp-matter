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
namespace media_playback {

inline constexpr uint32_t Id = 0x0506;

namespace feature {
namespace AdvancedSeek {
inline constexpr uint32_t Id = 0x1;
} /* AdvancedSeek */
namespace VariableSpeed {
inline constexpr uint32_t Id = 0x2;
} /* VariableSpeed */
namespace TextTracks {
inline constexpr uint32_t Id = 0x4;
} /* TextTracks */
namespace AudioTracks {
inline constexpr uint32_t Id = 0x8;
} /* AudioTracks */
namespace AudioAdvance {
inline constexpr uint32_t Id = 0x10;
} /* AudioAdvance */
} /* feature */

namespace attribute {
namespace CurrentState {
inline constexpr uint32_t Id = 0x0000;
} /* CurrentState */
namespace StartTime {
inline constexpr uint32_t Id = 0x0001;
} /* StartTime */
namespace Duration {
inline constexpr uint32_t Id = 0x0002;
} /* Duration */
namespace SampledPosition {
inline constexpr uint32_t Id = 0x0003;
} /* SampledPosition */
namespace PlaybackSpeed {
inline constexpr uint32_t Id = 0x0004;
} /* PlaybackSpeed */
namespace SeekRangeEnd {
inline constexpr uint32_t Id = 0x0005;
} /* SeekRangeEnd */
namespace SeekRangeStart {
inline constexpr uint32_t Id = 0x0006;
} /* SeekRangeStart */
namespace ActiveAudioTrack {
inline constexpr uint32_t Id = 0x0007;
} /* ActiveAudioTrack */
namespace AvailableAudioTracks {
inline constexpr uint32_t Id = 0x0008;
} /* AvailableAudioTracks */
namespace ActiveTextTrack {
inline constexpr uint32_t Id = 0x0009;
} /* ActiveTextTrack */
namespace AvailableTextTracks {
inline constexpr uint32_t Id = 0x000A;
} /* AvailableTextTracks */
} /* attribute */

namespace command {
namespace Play {
inline constexpr uint32_t Id = 0x00;
} /* Play */
namespace Pause {
inline constexpr uint32_t Id = 0x01;
} /* Pause */
namespace Stop {
inline constexpr uint32_t Id = 0x02;
} /* Stop */
namespace StartOver {
inline constexpr uint32_t Id = 0x03;
} /* StartOver */
namespace Previous {
inline constexpr uint32_t Id = 0x04;
} /* Previous */
namespace Next {
inline constexpr uint32_t Id = 0x05;
} /* Next */
namespace Rewind {
inline constexpr uint32_t Id = 0x06;
} /* Rewind */
namespace FastForward {
inline constexpr uint32_t Id = 0x07;
} /* FastForward */
namespace SkipForward {
inline constexpr uint32_t Id = 0x08;
} /* SkipForward */
namespace SkipBackward {
inline constexpr uint32_t Id = 0x09;
} /* SkipBackward */
namespace PlaybackResponse {
inline constexpr uint32_t Id = 0x0A;
} /* PlaybackResponse */
namespace Seek {
inline constexpr uint32_t Id = 0x0B;
} /* Seek */
namespace ActivateAudioTrack {
inline constexpr uint32_t Id = 0x0C;
} /* ActivateAudioTrack */
namespace ActivateTextTrack {
inline constexpr uint32_t Id = 0x0D;
} /* ActivateTextTrack */
namespace DeactivateTextTrack {
inline constexpr uint32_t Id = 0x0E;
} /* DeactivateTextTrack */
} /* command */

namespace event {
namespace StateChanged {
inline constexpr uint32_t Id = 0x00;
} /* StateChanged */
} /* event */

} /* media_playback */
} /* cluster */
} /* esp_matter */
