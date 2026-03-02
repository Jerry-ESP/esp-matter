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
#include <credentials/GroupDataProvider.h>
#include <data_model/esp_matter_data_model.h>
#include <data_model_provider/esp_matter_data_model_provider.h>
#include <lib/support/CodeUtils.h>
#include <unordered_map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Groups;

namespace {

std::unordered_map<EndpointId, LazyRegisteredServerCluster<GroupsCluster>> gServers;

} // namespace

void ESPMatterGroupsClusterServerInitCallback(EndpointId endpointId)
{
    if (gServers[endpointId].IsConstructed()) {
        return;
    }

    Credentials::GroupDataProvider * groupDataProvider = Credentials::GetGroupDataProvider();
    VerifyOrReturn(groupDataProvider != nullptr);

    // Optional integrations (scenes, identify) can be wired when those clusters are code-driven on the endpoint.
    GroupsCluster::Context context{
        .groupDataProvider    = *groupDataProvider,
        .scenesIntegration   = nullptr,
        .identifyIntegration = nullptr,
    };

    gServers[endpointId].Create(endpointId, context);
    CHIP_ERROR err =
        esp_matter::data_model::provider::get_instance().registry().Register(gServers[endpointId].Registration());
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Failed to register Groups on endpoint %u - Error: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void ESPMatterGroupsClusterServerShutdownCallback(EndpointId endpointId, ClusterShutdownType shutdownType)
{
    CHIP_ERROR err =
        esp_matter::data_model::provider::get_instance().registry().Unregister(&gServers[endpointId].Cluster(), shutdownType);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Failed to unregister Groups on endpoint %u - Error: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServers[endpointId].Destroy();
}

void MatterGroupsPluginServerInitCallback() {}

void MatterGroupsPluginServerShutdownCallback() {}
