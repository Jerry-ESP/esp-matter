// Copyright 2021 Espressif Systems (Shanghai) CO LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#pragma once

#include "app/util/basic-types.h"
#include "gen/callback.h"
#include "platform/CHIPDeviceLayer.h"

void on_on_off_attribute_changed(chip::EndpointId endpoint, chip::AttributeId attribute, uint8_t *value, size_t size);

void on_level_control_atrribute_changed(chip::EndpointId endpoint, chip::AttributeId attribute, uint8_t *value,
                                        size_t size);

void on_device_event(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg);

void update_current_power(bool power);

void update_current_brightness(uint8_t level);

void device_callbacks_init();
