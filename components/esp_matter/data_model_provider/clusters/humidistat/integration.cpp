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
#include <clusters/Humidistat/Enums.h>
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

bool ReadU8(EndpointId endpointId, AttributeId attributeId, uint8_t &value)
{
    return read_attribute_raw_value(endpointId, Humidistat::Id, attributeId, value);
}

bool ReadBool(EndpointId endpointId, AttributeId attributeId, bool &value)
{
    return read_attribute_raw_value(endpointId, Humidistat::Id, attributeId, value);
}

HumidistatCluster::OptionalAttributeSet BuildOptionalSet(EndpointId endpointId)
{
    HumidistatCluster::OptionalAttributeSet optionalSet(0);
    if (attribute::get(endpointId, Humidistat::Id, Sleep::Id) != nullptr) {
        optionalSet.Set<Sleep::Id>();
    }
    if (attribute::get(endpointId, Humidistat::Id, TargetSetpoint::Id) != nullptr) {
        optionalSet.Set<TargetSetpoint::Id>();
    }
    return optionalSet;
}

void ValidateFeatureDependencies(BitFlags<Feature> features)
{
    VerifyOrDie(!features.Has(Feature::kOptimal) || features.Has(Feature::kSensor));
    VerifyOrDie(!features.HasAny(Feature::kColdMist, Feature::kWarmMist) || features.Has(Feature::kHumidifier));
    VerifyOrDie(!features.Has(Feature::kAuto) ||
                (features.Has(Feature::kHumidifier) && features.Has(Feature::kDehumidifier) &&
                 features.Has(Feature::kSensor)));
}

HumidistatCluster *GetClusterInstance(EndpointId endpointId)
{
    auto it = gServers.find(endpointId);
    VerifyOrReturnValue(it != gServers.end(), nullptr);
    VerifyOrReturnValue(it->second.IsConstructed(), nullptr);
    return &it->second.Cluster();
}

} // namespace

void ESPMatterHumidistatClusterServerInitCallback(EndpointId endpointId)
{
    cluster_t *cluster = cluster::get(endpointId, Humidistat::Id);
    VerifyOrReturn(cluster != nullptr,
                   ChipLogError(AppServer, "Humidistat: cluster missing in esp-matter data model for endpoint %u",
                                endpointId));

    if (!gServers[endpointId].IsConstructed()) {
        const uint32_t rawFeatureMap = read_feature_map_u32(endpointId, Humidistat::Id);
        BitFlags<Feature> features(rawFeatureMap);
        ValidateFeatureDependencies(features);

        const auto optionalSet = BuildOptionalSet(endpointId);

        HumidistatCluster::StartupConfiguration config;

        if (features.Has(Feature::kSensor)) {
            VerifyOrReturn(ReadU8(endpointId, MinSetpoint::Id, config.minSetpoint),
                           ChipLogError(AppServer, "Humidistat: MinSetpoint read failed on ep %u", endpointId));
            VerifyOrReturn(ReadU8(endpointId, MaxSetpoint::Id, config.maxSetpoint),
                           ChipLogError(AppServer, "Humidistat: MaxSetpoint read failed on ep %u", endpointId));
            VerifyOrReturn(ReadU8(endpointId, Step::Id, config.step),
                           ChipLogError(AppServer, "Humidistat: Step read failed on ep %u", endpointId));
            VerifyOrReturn(ReadU8(endpointId, UserSetpoint::Id, config.userSetpoint),
                           ChipLogError(AppServer, "Humidistat: UserSetpoint read failed on ep %u", endpointId));
            if (optionalSet.IsSet(TargetSetpoint::Id)) {
                VerifyOrReturn(ReadU8(endpointId, TargetSetpoint::Id, config.targetSetpoint),
                               ChipLogError(AppServer, "Humidistat: TargetSetpoint read failed on ep %u", endpointId));
            }
        }

        if (features.Has(Feature::kHumidifier)) {
            uint8_t mistTypeRaw = 0;
            VerifyOrReturn(ReadU8(endpointId, MistType::Id, mistTypeRaw),
                           ChipLogError(AppServer, "Humidistat: MistType read failed on ep %u", endpointId));
            config.mistType = BitMask<MistTypeBitmap>(mistTypeRaw);
        }

        if (features.Has(Feature::kContinuous)) {
            VerifyOrReturn(ReadBool(endpointId, Continuous::Id, config.continuous),
                           ChipLogError(AppServer, "Humidistat: Continuous read failed on ep %u", endpointId));
        }

        if (optionalSet.IsSet(Sleep::Id)) {
            if (!ReadBool(endpointId, Sleep::Id, config.sleep)) {
                config.sleep = false;
            }
        }

        if (features.Has(Feature::kOptimal)) {
            VerifyOrReturn(ReadBool(endpointId, Optimal::Id, config.optimal),
                           ChipLogError(AppServer, "Humidistat: Optimal read failed on ep %u", endpointId));
            VerifyOrReturn(ReadU8(endpointId, TargetSetpoint::Id, config.targetSetpoint),
                           ChipLogError(AppServer, "Humidistat: TargetSetpoint read failed on ep %u", endpointId));
        }

        HumidistatCluster::OptionalAttributeSet activeOptional = optionalSet;
        if (!features.Has(Feature::kSensor)) {
            activeOptional.Set<TargetSetpoint::Id>(false);
        }

        uint8_t modeRaw        = 0;
        uint8_t systemStateRaw = 0;
        VerifyOrReturn(ReadU8(endpointId, Mode::Id, modeRaw),
                       ChipLogError(AppServer, "Humidistat: Mode read failed on ep %u", endpointId));
        VerifyOrReturn(ReadU8(endpointId, SystemState::Id, systemStateRaw),
                       ChipLogError(AppServer, "Humidistat: SystemState read failed on ep %u", endpointId));
        config.mode        = static_cast<ModeEnum>(modeRaw);
        config.systemState = static_cast<SystemStateEnum>(systemStateRaw);

        gServers[endpointId].Create(endpointId, features, activeOptional, config);
    }

    HumidistatCluster *humidistatCluster = GetClusterInstance(endpointId);
    if (humidistatCluster != nullptr) {
        HumidistatDelegate *delegate = static_cast<HumidistatDelegate *>(cluster::get_delegate_impl(cluster));
        if (delegate != nullptr) {
            humidistatCluster->SetDelegate(delegate);
        }
    }

    CHIP_ERROR err =
        esp_matter::data_model::provider::get_instance().registry().Register(gServers[endpointId].Registration());
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Humidistat register failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void ESPMatterHumidistatClusterServerShutdownCallback(EndpointId endpointId, ClusterShutdownType shutdownType)
{
    auto it = gServers.find(endpointId);
    VerifyOrReturn(it != gServers.end());
    VerifyOrReturn(it->second.IsConstructed());
    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Unregister(&it->second.Cluster(),
                                                                                            shutdownType);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Humidistat unregister failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    if (shutdownType == ClusterShutdownType::kPermanentRemove) {
        it->second.Destroy();
        gServers.erase(it);
    }
}

void MatterHumidistatPluginServerInitCallback() {}

namespace chip::app::Clusters::Humidistat {

HumidistatCluster *FindClusterOnEndpoint(EndpointId endpointId)
{
    return GetClusterInstance(endpointId);
}

} // namespace Humidistat
