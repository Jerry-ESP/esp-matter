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

#include "integration.h"
#include <app/ClusterCallbacks.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data_model/esp_matter_data_model.h>
#include <esp_matter_data_model_priv.h>
#include <data_model_provider/esp_matter_data_model_provider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitLocalization;

namespace {

LazyRegisteredServerCluster<UnitLocalizationCluster> gServer;

uint32_t get_feature_map(chip::EndpointId endpointId)
{
    esp_matter::cluster_t * cluster = esp_matter::cluster::get(endpointId, UnitLocalization::Id);
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

void ESPMatterUnitLocalizationClusterServerInitCallback(EndpointId endpointId)
{
    // UnitLocalization may only be present on the root endpoint.
    VerifyOrReturn(endpointId == kRootEndpointId);
    esp_matter::cluster_t * cluster = esp_matter::cluster::get(endpointId, UnitLocalization::Id);
    if (!cluster) {
        return;
    }
    gServer.Create(endpointId, BitFlags<UnitLocalization::Feature>(get_feature_map(endpointId)));
    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Failed to register UnitLocalization - Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ESPMatterUnitLocalizationClusterServerShutdownCallback(EndpointId endpointId, ClusterShutdownType shutdownType)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Unregister(&gServer.Cluster(), shutdownType);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Failed to unregister UnitLocalization - Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
    gServer.Destroy();
}

void MatterUnitLocalizationPluginServerInitCallback() {}

void MatterUnitLocalizationPluginServerShutdownCallback() {}
