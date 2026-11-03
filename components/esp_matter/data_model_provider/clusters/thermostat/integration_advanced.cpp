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

// Presets / matter-schedules feature combinations for the Thermostat provider integration.
// This translation unit is pulled in by the Thermostat::SetDelegates reference, so the
// template instantiations below (and the upstream presets/schedules/atomic-write machinery
// they drag in) only cost flash in applications that actually register those delegates.

#include "integration_internal.h"

#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace esp_matter::thermostat_integration;

namespace {

template <typename... Ds>
std::unique_ptr<ClusterHolder> BuildSchedules(const BuildContext &ctx, Ds &... ds)
{
    if (ctx.features.Has(Feature::kMatterScheduleConfiguration)) {
        return MakeHolder(ctx, ds..., *ctx.app.schedules);
    }
    return MakeHolder(ctx, ds...);
}

template <typename... Ds>
std::unique_ptr<ClusterHolder> BuildPresets(const BuildContext &ctx, Ds &... ds)
{
    if (ctx.features.Has(Feature::kPresets)) {
        return BuildSchedules(ctx, ds..., *ctx.app.presets);
    }
    return BuildSchedules(ctx, ds...);
}

std::unique_ptr<ClusterHolder> BuildAdvanced(const BuildContext &ctx, ThermostatEndpoint &entry)
{
    if (ctx.features.Has(Feature::kOccupancy)) {
        return BuildPresets(ctx, entry.core, entry.setpoints, entry.hold, entry.occupancy);
    }
    return BuildPresets(ctx, entry.core, entry.setpoints, entry.hold);
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void SetDelegates(EndpointId endpointId, const EndpointDelegates &delegates)
{
    esp_matter::thermostat_integration::gAdvancedBuilder = BuildAdvanced;

    auto &entry = esp_matter::thermostat_integration::GetEndpoint(endpointId);
    entry.app = delegates;
    entry.core.SetWrapped(delegates.core);
    if (entry.holder) {
        ChipLogError(AppServer, "Thermostat: SetDelegates called after cluster creation on endpoint %u; only the core "
                     "delegate override takes effect", endpointId);
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
