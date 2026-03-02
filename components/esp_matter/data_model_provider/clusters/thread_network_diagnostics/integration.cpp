// Copyright 2025 Espressif Systems (Shanghai) PTE LTD
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

#include "integration.h"
#include <app/ClusterCallbacks.h>
#include <data_model/esp_matter_data_model.h>
#include <data_model_provider/esp_matter_data_model_provider.h>
#include <esp_matter_data_model_priv.h>
#include <lib/support/CodeUtils.h>
#include <unordered_map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics;

namespace {

std::unordered_map<EndpointId, LazyRegisteredServerCluster<ThreadNetworkDiagnosticsCluster>> gServers;

uint32_t get_feature_map(EndpointId endpointId)
{
    esp_matter::cluster_t * cluster = esp_matter::cluster::get(endpointId, ThreadNetworkDiagnostics::Id);
    if (!cluster) {
        return 0;
    }
    esp_matter::attribute_t * attribute =
        esp_matter::attribute::get(cluster, chip::app::Clusters::Globals::Attributes::FeatureMap::Id);
    if (!attribute) {
        return 0;
    }
    esp_matter_attr_val_t val = esp_matter_invalid(nullptr);
    if (esp_matter::attribute::get_val_internal(attribute, &val) != ESP_OK ||
            val.type != ESP_MATTER_VAL_TYPE_BITMAP32) {
        return 0;
    }
    return val.val.u32;
}

} // namespace

void ESPMatterThreadNetworkDiagnosticsClusterServerInitCallback(EndpointId endpointId)
{
    if (gServers[endpointId].IsConstructed()) {
        return;
    }

    uint32_t rawFeatureMap = get_feature_map(endpointId);
    // Only minimal (0) or full (kFeaturesAll) are supported
    VerifyOrReturn(rawFeatureMap == 0 || rawFeatureMap == kFeaturesAll.Raw());

    ThreadNetworkDiagnosticsCluster::ClusterType clusterType =
        (rawFeatureMap == 0) ? ThreadNetworkDiagnosticsCluster::ClusterType::kMinimal
        : ThreadNetworkDiagnosticsCluster::ClusterType::kFull;

    gServers[endpointId].Create(endpointId, clusterType);
    CHIP_ERROR err =
        esp_matter::data_model::provider::get_instance().registry().Register(gServers[endpointId].Registration());
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer,
                     "Failed to register ThreadNetworkDiagnostics on endpoint %u - Error: %" CHIP_ERROR_FORMAT,
                     endpointId, err.Format());
    }
}

void ESPMatterThreadNetworkDiagnosticsClusterServerShutdownCallback(EndpointId endpointId,
                                                                    ClusterShutdownType shutdownType)
{
    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Unregister(
                         &gServers[endpointId].Cluster(), shutdownType);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer,
                     "Failed to unregister ThreadNetworkDiagnostics on endpoint %u - Error: %" CHIP_ERROR_FORMAT,
                     endpointId, err.Format());
    }
    gServers[endpointId].Destroy();
}

void MatterThreadNetworkDiagnosticsPluginServerInitCallback() {}

void MatterThreadNetworkDiagnosticsPluginServerShutdownCallback() {}
