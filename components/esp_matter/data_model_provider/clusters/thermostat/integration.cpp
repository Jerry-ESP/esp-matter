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

#include "integration_internal.h"

#include <app/ClusterCallbacks.h>
#include <data_model_provider/esp_matter_data_model_provider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DefaultTimerDelegate.h>
#include <unordered_map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace esp_matter;
using namespace esp_matter::thermostat_integration;
using chip::Protocols::InteractionModel::Status;

namespace {

// The default delegates below are thin adapters over the esp-matter attribute store: reads
// fetch the stored value (presence in the store decides attribute support), writes go through
// set_val_internal so NONVOLATILE values persist to NVS and application attribute callbacks
// keep firing. Features whose state and semantics the application owns (presets, matter
// schedules) use application delegates registered via Thermostat::SetDelegates.

// The set of features a ThermostatCluster instance exposes is a compile-time property of the
// template (each enabled feature's attributes are published by template inclusion, not gated
// by the runtime feature map). The builder chain below therefore instantiates one cluster type
// per feature combination and selects it at init time from the endpoint's feature map and the
// application-provided delegates.

std::unordered_map<EndpointId, esp_matter::thermostat_integration::ThermostatEndpoint> gServers;

std::unique_ptr<ClusterHolder> BuildCluster(const BuildContext &ctx, ThermostatEndpoint &entry)
{
    if (ctx.features.HasAny(Feature::kPresets, Feature::kMatterScheduleConfiguration)) {
        // The presets / matter-schedules combinations live in integration_advanced.cpp and are
        // linked in by the Thermostat::SetDelegates call that provides their delegates.
        VerifyOrReturnValue(gAdvancedBuilder != nullptr, nullptr,
                            ChipLogError(AppServer, "Thermostat: presets/schedules need delegates registered via "
                                         "Thermostat::SetDelegates; endpoint %u not registered", ctx.endpointId));
        return gAdvancedBuilder(ctx, entry);
    }
    if (ctx.features.Has(Feature::kOccupancy)) {
        return MakeHolder(ctx, entry.core, entry.setpoints, entry.hold, entry.occupancy);
    }
    return MakeHolder(ctx, entry.core, entry.setpoints, entry.hold);
}

/** Validate that every requested feature has its application delegate. */
bool ValidateFeatureSupport(EndpointId endpointId, const BitFlags<Thermostat::Feature> &features,
                            const EndpointDelegates &app)
{
    const struct {
        Feature feature;
        const void *delegate;
        const char *name;
    } axes[] = {
        { Feature::kPresets, app.presets, "presets" },
        { Feature::kMatterScheduleConfiguration, app.schedules, "matter schedules" },
    };
    for (const auto &axis : axes) {
        if (!features.Has(axis.feature)) {
            continue;
        }
        if (axis.delegate == nullptr) {
            ChipLogError(AppServer, "Thermostat: %s feature needs an application delegate (Thermostat::SetDelegates); "
                         "endpoint %u not registered", axis.name, endpointId);
            return false;
        }
    }
    if (features.HasAny(Feature::kThermostatSuggestions, Feature::kThermostatSensors)) {
        ChipLogError(AppServer, "Thermostat: suggestions/sensors features are not supported yet; endpoint %u not registered",
                     endpointId);
        return false;
    }
    if (features.HasAny(Feature::kScheduleConfiguration, Feature::kSetback)) {
        ChipLogError(AppServer, "Thermostat: deprecated schedule-configuration/setback features are not supported by the "
                     "code-driven cluster; endpoint %u not registered", endpointId);
        return false;
    }
    return true;
}

OptionalAttributes BuildOptionalAttributes(EndpointId endpointId, const BitFlags<Thermostat::Feature> &features)
{
    cluster_t *cluster = cluster::get(endpointId, Thermostat::Id);
    OptionalAttributes optionalAttributes;
    VerifyOrReturnValue(cluster != nullptr, optionalAttributes);

    auto has = [cluster](AttributeId id) {
        return is_attribute_enabled(cluster, id);
    };

    optionalAttributes.AbsMinHeatSetpointLimit = features.Has(Feature::kHeating) && has(AbsMinHeatSetpointLimit::Id);
    optionalAttributes.AbsMaxHeatSetpointLimit = features.Has(Feature::kHeating) && has(AbsMaxHeatSetpointLimit::Id);
    optionalAttributes.AbsMinCoolSetpointLimit = features.Has(Feature::kCooling) && has(AbsMinCoolSetpointLimit::Id);
    optionalAttributes.AbsMaxCoolSetpointLimit = features.Has(Feature::kCooling) && has(AbsMaxCoolSetpointLimit::Id);

    optionalAttributes.LocalTemperatureCalibration =
        !features.Has(Feature::kLocalTemperatureNotExposed) && has(LocalTemperatureCalibration::Id);

    optionalAttributes.MinHeatSetpointLimit = features.Has(Feature::kHeating) && has(MinHeatSetpointLimit::Id);
    optionalAttributes.MaxHeatSetpointLimit = features.Has(Feature::kHeating) && has(MaxHeatSetpointLimit::Id);
    optionalAttributes.MinCoolSetpointLimit = features.Has(Feature::kCooling) && has(MinCoolSetpointLimit::Id);
    optionalAttributes.MaxCoolSetpointLimit = features.Has(Feature::kCooling) && has(MaxCoolSetpointLimit::Id);

    optionalAttributes.RemoteSensing         = has(RemoteSensing::Id);
    optionalAttributes.ThermostatRunningMode = features.Has(Feature::kAutoMode) && has(ThermostatRunningMode::Id);
    optionalAttributes.TemperatureSetpointHold         = has(TemperatureSetpointHold::Id);
    optionalAttributes.TemperatureSetpointHoldDuration = has(TemperatureSetpointHoldDuration::Id);
    optionalAttributes.SetpointHoldExpiryTimestamp     = has(SetpointHoldExpiryTimestamp::Id);
    optionalAttributes.ThermostatRunningState          = has(ThermostatRunningState::Id);
    optionalAttributes.SetpointChangeSource            = has(SetpointChangeSource::Id);
    optionalAttributes.SetpointChangeAmount            = has(SetpointChangeAmount::Id);
    optionalAttributes.SetpointChangeSourceTimestamp   = has(SetpointChangeSourceTimestamp::Id);
    optionalAttributes.OutdoorTemperature              = has(OutdoorTemperature::Id);
    optionalAttributes.CriticalFreezeProtection = features.Has(Feature::kHeating) && has(CriticalFreezeProtection::Id);
    optionalAttributes.CriticalOverheatProtection = features.Has(Feature::kCooling) && has(CriticalOverheatProtection::Id);

    return optionalAttributes;
}

TimerDelegate &GetTimerDelegate()
{
    static DefaultTimerDelegate sTimerDelegate;
    return sTimerDelegate;
}

} // namespace

void ESPMatterThermostatClusterServerInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(cluster::get(endpointId, Thermostat::Id) != nullptr,
                   ChipLogError(AppServer, "Thermostat: cluster missing in esp-matter data model for endpoint %u", endpointId));

    auto &entry = gServers[endpointId];
    if (!entry.holder) {
        const uint32_t featureMap = read_feature_map_u32(endpointId, Thermostat::Id);
        const BitFlags<Thermostat::Feature> features(featureMap);

        VerifyOrReturn(ValidateFeatureSupport(endpointId, features, entry.app));

        entry.core.Init(endpointId);
        entry.core.SetWrapped(entry.app.core);
        entry.setpoints.Init(endpointId);
        entry.hold.Init(endpointId);
        entry.occupancy.Init(endpointId);

        ThermostatClusterBase::Config config(BuildOptionalAttributes(endpointId, features), GetTimerDelegate());
        entry.holder = BuildCluster({ endpointId, features, config, entry.app }, entry);
        VerifyOrReturn(entry.holder != nullptr,
                       ChipLogError(AppServer, "Thermostat: failed to create cluster for endpoint %u", endpointId));
    }

    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Register(entry.holder->Registration());
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Thermostat register failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void ESPMatterThermostatClusterServerShutdownCallback(EndpointId endpointId, ClusterShutdownType shutdownType)
{
    auto it = gServers.find(endpointId);
    VerifyOrReturn(it != gServers.end());
    VerifyOrReturn(it->second.holder != nullptr);
    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Unregister(&it->second.holder->Cluster(),
                                                                                            shutdownType);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Thermostat unregister failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    if (shutdownType == ClusterShutdownType::kPermanentRemove) {
        it->second.holder.reset();
        gServers.erase(it);
    }
}

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void SetDefaultDelegate(EndpointId endpointId, Delegate *delegate)
{
    auto &entry = gServers[endpointId];
    entry.app.core = delegate;
    entry.core.SetWrapped(delegate);
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

namespace esp_matter {
namespace thermostat_integration {

AdvancedBuilderFn gAdvancedBuilder = nullptr;

ThermostatEndpoint &GetEndpoint(chip::EndpointId endpointId)
{
    return gServers[endpointId];
}

} // namespace thermostat_integration
} // namespace esp_matter
