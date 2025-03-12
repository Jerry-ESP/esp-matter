// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
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

#include <app/CASESessionManager.h>
#include <app/DeviceProxy.h>
#include <app/InteractionModelEngine.h>
#include <app/util/af-types.h>
#include <esp_err.h>
#include <esp_matter_attribute_utils.h>
#include <app/AttributePathParams.h>
#include <app/CommandPathParams.h>
#include <app/EventPathParams.h>

using chip::app::ConcreteCommandPath;
using chip::DeviceLayer::ChipDeviceEvent;
using chip::TLV::TLVReader;

namespace esp_matter {

/** Event callback
 *
 * @param[in] event event data pointer.
 * @param[in] arg Pointer to the private data passed while setting the callback.
 */
/** TODO: Change this */
typedef void (*event_callback_t)(const ChipDeviceEvent *event, intptr_t arg);

/** ESP Matter Start
 *
 * Initialize and start the matter thread.
 *
 * @param[in] callback event callback.
 * @param[in] callback_arg private data to pass to callback function, optional argument, by default set to NULL.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t start(event_callback_t callback, intptr_t callback_arg = static_cast<intptr_t>(NULL));

/** Factory reset
 *
 * Perform factory reset and erase the data stored in the non volatile storage. This also restarts the device.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t factory_reset();

namespace lock {

/** Lock status */
typedef enum status {
    /** Lock failed */
    FAILED,
    /** Lock was already taken */
    ALREADY_TAKEN,
    /** Lock success */
    SUCCESS,
} status_t;

/** Stack lock
 *
 * This API should be called before calling any upstream APIs.
 *
 * @param[in] ticks_to_wait number of ticks to wait for trying to take the lock. Accepted values: 0 to portMAX_DELAY.
 *
 * @return FAILED if the lock was not taken within the specified ticks.
 * @return ALREADY_TAKEN if the lock was already taken by the same task context.
 * @return SUCCESS if the lock was taken successfully.
 */
status_t chip_stack_lock(uint32_t ticks_to_wait);

/** Stack unlock
 *
 * This API should be called after the upstream APIs have been done calling.
 *
 * @note: This should only be called if `chip_stack_lock()` returns `SUCCESS`.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t chip_stack_unlock();

} /* lock */

} /* esp_matter */
