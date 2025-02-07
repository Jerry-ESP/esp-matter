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

#pragma once
#include <app/AttributePathParams.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/Provider.h>
#include <optional>

using chip::app::AttributePathParams;
using chip::app::AttributeValueDecoder;
using chip::app::AttributeValueEncoder;
using chip::app::CommandHandler;
using chip::app::ConcreteAttributePath;
using chip::app::ConcreteClusterPath;
using chip::app::ConcreteCommandPath;
using chip::app::DataModel::ActionReturnStatus;
using chip::app::DataModel::AttributeEntry;
using chip::app::DataModel::AttributeInfo;
using chip::app::DataModel::ClusterEntry;
using chip::app::DataModel::ClusterInfo;
using chip::app::DataModel::CommandEntry;
using chip::app::DataModel::CommandInfo;
using chip::app::DataModel::DeviceTypeEntry;
using chip::app::DataModel::EndpointEntry;
using chip::app::DataModel::EndpointInfo;
using chip::app::DataModel::InvokeRequest;
using chip::app::DataModel::ReadAttributeRequest;
using chip::app::DataModel::WriteAttributeRequest;
using chip::TLV::TLVReader;

namespace esp_matter {
namespace controller {
namespace data_model {

// TODO: The client-only controller has no data model, use an empty data model provider for it.
//       We should finish the functions after enabling dynamic server for the controller.
class provider : public chip::app::DataModel::Provider {
public:
    static provider &get_instance()
    {
        static provider instance;
        return instance;
    }

    CHIP_ERROR Shutdown() override { return CHIP_NO_ERROR; }

    ActionReturnStatus ReadAttribute(const ReadAttributeRequest &request, AttributeValueEncoder &encoder) override;

    ActionReturnStatus WriteAttribute(const WriteAttributeRequest &request, AttributeValueDecoder &decoder) override;

    std::optional<ActionReturnStatus> Invoke(const InvokeRequest &request, TLVReader &input_arguments,
                                             CommandHandler *handler) override;

    // metadata tree iteration
    // The dynamic server has only endpoint 0
    EndpointEntry FirstEndpoint() override;
    EndpointEntry NextEndpoint(chip::EndpointId before) override { return EndpointEntry::kInvalid; }
    std::optional<EndpointInfo> GetEndpointInfo(chip::EndpointId endpoint) override;
    bool EndpointExists(chip::EndpointId endpoint) override { return endpoint == 0 ? true : false; }

    // The dynamic server has only an OTA provider cluster server on endpoint 0 without descriptor cluster,
    // so return empty device type for all the endpoints
    std::optional<DeviceTypeEntry> FirstDeviceType(chip::EndpointId endpoint) override { return std::nullopt; }
    std::optional<DeviceTypeEntry> NextDeviceType(chip::EndpointId endpoint, const DeviceTypeEntry &previous) override
    {
        return std::nullopt;
    }

    // No semantic tag for all the endpoints
    std::optional<SemanticTag> GetFirstSemanticTag(chip::EndpointId endpoint) override { return std::nullopt; }
    std::optional<SemanticTag> GetNextSemanticTag(chip::EndpointId endpoint, const SemanticTag &previous) override
    {
        return std::nullopt;
    }

    ClusterEntry FirstServerCluster(chip::EndpointId endpoint) override;
    ClusterEntry NextServerCluster(const ConcreteClusterPath &before) override { return ClusterEntry::kInvalid; }
    std::optional<ClusterInfo> GetServerClusterInfo(const ConcreteClusterPath &path) override;

    // No client cluster for all the endpoints
    ConcreteClusterPath FirstClientCluster(chip::EndpointId endpoint) override { return ConcreteClusterPath(); }
    ConcreteClusterPath NextClientCluster(const ConcreteClusterPath &before) override { return ConcreteClusterPath(); }

    AttributeEntry FirstAttribute(const ConcreteClusterPath &cluster) override;
    AttributeEntry NextAttribute(const ConcreteAttributePath &before) override;
    std::optional<AttributeInfo> GetAttributeInfo(const ConcreteAttributePath &path) override;

    CommandEntry FirstAcceptedCommand(const ConcreteClusterPath &cluster) override;
    CommandEntry NextAcceptedCommand(const ConcreteCommandPath &before) override;
    std::optional<CommandInfo> GetAcceptedCommandInfo(const ConcreteCommandPath &path) override;

    ConcreteCommandPath FirstGeneratedCommand(const ConcreteClusterPath &cluster) override;
    ConcreteCommandPath NextGeneratedCommand(const ConcreteCommandPath &before) override;

    void Temporary_ReportAttributeChanged(const AttributePathParams &path) override {}

private:
    provider() = default;
    ~provider() = default;
};

} // namespace data_model
} // namespace controller
} // namespace esp_matter
