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

#pragma once

#include <app/clusters/thermostat-server/ThermostatCluster.h>

namespace chip::app::Clusters::Thermostat {

/**
 * Application-provided delegates for Thermostat features whose state and semantics the
 * application owns. The esp-matter integration provides attribute-store-backed defaults for
 * the core cluster, heating/cooling/auto setpoints and occupancy; the delegates below have no
 * usable default and must be supplied by the application when the corresponding feature is
 * enabled on the endpoint.
 */
struct EndpointDelegates {
    /** Optional override for the core delegate (local temperature, system mode, ...). */
    Delegate *core = nullptr;
    /** Enables TemperatureSetpointHold attributes; not tied to a feature bit. */
    ThermostatHold::Delegate *hold = nullptr;
    /** Required when the presets feature is enabled. */
    ThermostatPresets::Delegate *presets = nullptr;
    /** Required when the suggestions feature is enabled (implies presets). */
    ThermostatSuggestions::Delegate *suggestions = nullptr;
    /** Required when the matter-schedules feature is enabled. */
    ThermostatSchedules::Delegate *schedules = nullptr;
    /** Required when the sensors feature is enabled. */
    ThermostatSensors::Delegate *sensors = nullptr;
};

/**
 * Register application delegates for the endpoint. Must be called before the endpoint is
 * enabled (i.e. before esp_matter::start()); the delegates are consumed when the Thermostat
 * cluster instance is created.
 */
void SetDelegates(EndpointId endpointId, const EndpointDelegates &delegates);

/**
 * Register an application core Thermostat delegate for the endpoint. When set, core cluster
 * state (local temperature, system mode, running mode/state, ...) is served by the
 * application delegate instead of the default esp-matter attribute-store adapter. Setpoint,
 * auto and occupancy state keep using the esp-matter attribute store.
 */
void SetDefaultDelegate(EndpointId endpointId, Delegate * delegate);

} // namespace chip::app::Clusters::Thermostat
