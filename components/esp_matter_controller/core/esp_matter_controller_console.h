// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
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

#pragma once
#include <esp_matter_console.h>
#include <lib/support/CHIPMem.h>

using chip::Platform::ScopedMemoryBufferWithSize;

namespace esp_matter {

esp_err_t string_to_uint16_array(const char *str, ScopedMemoryBufferWithSize<uint16_t> &uint16_array);
esp_err_t string_to_uint32_array(const char *str, ScopedMemoryBufferWithSize<uint32_t> &uint32_array);

namespace console {

esp_err_t controller_register_commands();

} // namespace console
} // namespace esp_matter
