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

#include <esp_matter_data_model_priv.h>

#include <app/ClusterCallbacks.h>
#include <app/clusters/humidistat-server/HumidistatCluster.h>
#include <clusters/Humidistat/Attributes.h>
#include <clusters/Humidistat/ClusterId.h>
#include <clusters/Humidistat/EnumsCheck.h>
#include <data_model/esp_matter_attribute_helpers.h>
#include <data_model/esp_matter_data_model.h>
#include <data_model_provider/esp_matter_data_model_provider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <unordered_map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Humidistat;
using namespace chip::app::Clusters::Humidistat::Attributes;
using namespace esp_matter;

namespace {

std::unordered_map<EndpointId, LazyRegisteredServerCluster<HumidistatCluster>> gServers;

bool SetpointsValid(uint8_t minSetpoint, uint8_t maxSetpoint, uint8_t step)
{
    if (minSetpoint > 99 || maxSetpoint > 100 || maxSetpoint < static_cast<uint8_t>(minSetpoint + 1)) {
        return false;
    }
    const uint8_t span = static_cast<uint8_t>(maxSetpoint - minSetpoint);
    return step >= 1 && step <= span && (span % step) == 0;
}

HumidistatCluster::StartupConfiguration LoadStartupConfiguration(EndpointId endpointId, cluster_t *cluster,
                                                                 BitFlags<Feature> features)
{
    HumidistatCluster::StartupConfiguration config;

    uint8_t raw = 0;
    if (read_attribute_raw_value(endpointId, Humidistat::Id, Mode::Id, raw)) {
        auto mode = EnsureKnownEnumValue(static_cast<ModeEnum>(raw));
        if (mode != ModeEnum::kUnknownEnumValue) {
            config.mode = mode;
        }
    }
    if (read_attribute_raw_value(endpointId, Humidistat::Id, SystemState::Id, raw)) {
        auto systemState = EnsureKnownEnumValue(static_cast<SystemStateEnum>(raw));
        if (systemState != SystemStateEnum::kUnknownEnumValue) {
            config.systemState = systemState;
        }
    }

    if (features.Has(Feature::kSensor)) {
        uint8_t minSetpoint = config.minSetpoint;
        uint8_t maxSetpoint = config.maxSetpoint;
        uint8_t step = config.step;
        uint8_t userSetpoint = config.userSetpoint;
        uint8_t targetSetpoint = config.targetSetpoint;
        read_attribute_raw_value(endpointId, Humidistat::Id, MinSetpoint::Id, minSetpoint);
        read_attribute_raw_value(endpointId, Humidistat::Id, MaxSetpoint::Id, maxSetpoint);
        read_attribute_raw_value(endpointId, Humidistat::Id, Step::Id, step);
        read_attribute_raw_value(endpointId, Humidistat::Id, UserSetpoint::Id, userSetpoint);
        read_attribute_raw_value(endpointId, Humidistat::Id, TargetSetpoint::Id, targetSetpoint);
        if (SetpointsValid(minSetpoint, maxSetpoint, step)) {
            config.minSetpoint = minSetpoint;
            config.maxSetpoint = maxSetpoint;
            config.step = step;
            config.userSetpoint = userSetpoint;
            config.targetSetpoint = targetSetpoint;
        } else {
            ChipLogError(AppServer, "Humidistat: ignoring invalid setpoints min=%u max=%u step=%u", minSetpoint, maxSetpoint,
                         step);
        }
    }

    if (features.Has(Feature::kHumidifier)) {
        esp_matter_attr_val_t val = esp_matter_invalid(nullptr);
        if (get_stored_attr_val(cluster, MistType::Id, val) == ESP_OK && !val.is_null()) {
            BitMask<MistTypeBitmap> mistType = BitMask<MistTypeBitmap>(val.val.u8) &
                                               BitMask<MistTypeBitmap>(MistTypeBitmap::kMistCold, MistTypeBitmap::kMistWarm);
            if (!features.Has(Feature::kColdMist)) {
                mistType.Clear(MistTypeBitmap::kMistCold);
            }
            if (!features.Has(Feature::kWarmMist)) {
                mistType.Clear(MistTypeBitmap::kMistWarm);
            }
            config.mistType = mistType;
        }
    }

    if (features.Has(Feature::kContinuous)) {
        bool continuous = config.continuous;
        if (read_attribute_raw_value(endpointId, Humidistat::Id, Continuous::Id, continuous)) {
            config.continuous = continuous;
        }
    }
    if (is_attribute_enabled(cluster, Sleep::Id)) {
        bool sleep = config.sleep;
        if (read_attribute_raw_value(endpointId, Humidistat::Id, Sleep::Id, sleep)) {
            config.sleep = sleep;
        }
    }
    if (features.Has(Feature::kOptimal)) {
        bool optimal = config.optimal;
        if (read_attribute_raw_value(endpointId, Humidistat::Id, Optimal::Id, optimal)) {
            config.optimal = optimal;
        }
    }

    return config;
}

} // namespace

namespace chip::app::Clusters::Humidistat {

HumidistatCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    auto it = gServers.find(endpointId);
    if (it == gServers.end() || !it->second.IsConstructed()) {
        return nullptr;
    }
    return &it->second.Cluster();
}

} // namespace chip::app::Clusters::Humidistat

void ESPMatterHumidistatClusterServerInitCallback(EndpointId endpointId)
{
    cluster_t *cluster = cluster::get(endpointId, Humidistat::Id);
    VerifyOrReturn(cluster != nullptr,
                   ChipLogError(AppServer, "Humidistat: cluster missing in esp-matter data model for endpoint %u", endpointId));

    if (!gServers[endpointId].IsConstructed()) {
        const BitFlags<Feature> features(read_feature_map_u32(endpointId, Humidistat::Id));
        HumidistatCluster::OptionalAttributeSet optionalAttributes;
        if (is_attribute_enabled(cluster, TargetSetpoint::Id)) {
            optionalAttributes.Set<TargetSetpoint::Id>();
        }
        if (is_attribute_enabled(cluster, Sleep::Id)) {
            optionalAttributes.Set<Sleep::Id>();
        }
        gServers[endpointId].Create(endpointId, features, optionalAttributes,
                                    LoadStartupConfiguration(endpointId, cluster, features));
    }

    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Register(gServers[endpointId].Registration());
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Humidistat register failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void ESPMatterHumidistatClusterServerShutdownCallback(EndpointId endpointId, ClusterShutdownType shutdownType)
{
    auto it = gServers.find(endpointId);
    if (it == gServers.end() || !it->second.IsConstructed()) {
        return;
    }
    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Unregister(&it->second.Cluster(), shutdownType);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Humidistat unregister failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    if (shutdownType == ClusterShutdownType::kPermanentRemove) {
        it->second.Destroy();
        gServers.erase(it);
    }
}

void MatterHumidistatPluginServerInitCallback() {}
