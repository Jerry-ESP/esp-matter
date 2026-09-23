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

namespace feature {
namespace advanced_seek {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* advanced_seek */

namespace variable_speed {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* variable_speed */

namespace text_tracks {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* text_tracks */

namespace audio_tracks {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* audio_tracks */

namespace audio_advance {
uint32_t get_id();
esp_err_t add(cluster_t *cluster);
} /* audio_advance */

} /* feature */

namespace attribute {
attribute_t *create_current_state(cluster_t *cluster, uint8_t value);
attribute_t *create_start_time(cluster_t *cluster, nullable<uint64_t> value);
attribute_t *create_duration(cluster_t *cluster, nullable<uint64_t> value);
attribute_t *create_sampled_position(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_playback_speed(cluster_t *cluster, float value);
attribute_t *create_seek_range_end(cluster_t *cluster, nullable<uint64_t> value);
attribute_t *create_seek_range_start(cluster_t *cluster, nullable<uint64_t> value);
attribute_t *create_active_audio_track(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_available_audio_tracks(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_active_text_track(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
attribute_t *create_available_text_tracks(cluster_t *cluster, uint8_t * value, uint16_t length, uint16_t count);
} /* attribute */

namespace command {
command_t *create_play(cluster_t *cluster);
command_t *create_pause(cluster_t *cluster);
command_t *create_stop(cluster_t *cluster);
command_t *create_start_over(cluster_t *cluster);
command_t *create_previous(cluster_t *cluster);
command_t *create_next(cluster_t *cluster);
command_t *create_rewind(cluster_t *cluster);
command_t *create_fast_forward(cluster_t *cluster);
command_t *create_skip_forward(cluster_t *cluster);
command_t *create_skip_backward(cluster_t *cluster);
command_t *create_playback_response(cluster_t *cluster);
command_t *create_seek(cluster_t *cluster);
command_t *create_activate_audio_track(cluster_t *cluster);
command_t *create_activate_text_track(cluster_t *cluster);
command_t *create_deactivate_text_track(cluster_t *cluster);
} /* command */

namespace event {
event_t *create_state_changed(cluster_t *cluster);
} /* event */

typedef struct config {
    void *delegate;
    config() : delegate(nullptr) {}
} config_t;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags);

} /* media_playback */
} /* cluster */
} /* esp_matter */
