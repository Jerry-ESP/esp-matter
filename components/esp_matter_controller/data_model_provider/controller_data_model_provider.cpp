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

#include <controller_data_model_provider.h>

#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/util/IMClusterCommandHandler.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip::app::Clusters;

namespace esp_matter {
namespace controller {
namespace data_model {

const AttributeInfo k_list_attribute = {
    .flags = chip::BitFlags<chip::app::DataModel::AttributeQualityFlags>(
        chip::app::DataModel::AttributeQualityFlags::kListAttribute),
    .readPrivilege = std::nullopt,
    .writePrivilege = std::nullopt,
};

const static AttributeEntry k_global_attributes[] = {
    {
        .path = ConcreteAttributePath(0, OtaSoftwareUpdateProvider::Id, Globals::Attributes::AttributeList::Id),
        .info = k_list_attribute,
    },
    {
        .path = ConcreteAttributePath(0, OtaSoftwareUpdateProvider::Id, Globals::Attributes::AcceptedCommandList::Id),
        .info = k_list_attribute,
    },
    {
        .path = ConcreteAttributePath(0, OtaSoftwareUpdateProvider::Id, Globals::Attributes::GeneratedCommandList::Id),
        .info = k_list_attribute,
    },
    {
        .path = ConcreteAttributePath(0, OtaSoftwareUpdateProvider::Id, Globals::Attributes::FeatureMap::Id),
        .info = AttributeInfo(),
    },
    {
        .path = ConcreteAttributePath(0, OtaSoftwareUpdateProvider::Id, Globals::Attributes::ClusterRevision::Id),
        .info = AttributeInfo(),
    },
};

const static CommandEntry k_accepted_commands[] = {
    {
        .path =
            ConcreteCommandPath(0, OtaSoftwareUpdateProvider::Id, OtaSoftwareUpdateProvider::Commands::QueryImage::Id),
        .info = CommandInfo(),
    },
    {
        .path = ConcreteCommandPath(0, OtaSoftwareUpdateProvider::Id,
                                    OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::Id),
        .info = CommandInfo(),
    },
    {
        .path = ConcreteCommandPath(0, OtaSoftwareUpdateProvider::Id,
                                    OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::Id),
        .info = CommandInfo(),
    },
};
const static ConcreteCommandPath k_generated_commands[] = {
    ConcreteCommandPath(0, OtaSoftwareUpdateProvider::Id, OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Id),
    ConcreteCommandPath(0, OtaSoftwareUpdateProvider::Id, OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Id),
};

ActionReturnStatus provider::ReadAttribute(const ReadAttributeRequest &request, AttributeValueEncoder &encoder)
{
    if (request.path.mEndpointId != 0) {
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    if (request.path.mClusterId != chip::app::Clusters::OtaSoftwareUpdateProvider::Id) {
        return chip::Protocols::InteractionModel::Status::UnsupportedCluster;
    }
    switch (request.path.mAttributeId) {
    case OtaSoftwareUpdateProvider::Attributes::FeatureMap::Id: {
        return encoder.Encode((uint32_t)0);
        break;
    }
    case OtaSoftwareUpdateProvider::Attributes::AttributeList::Id: {
        return encoder.EncodeList([](const auto &aEncoder) {
            for (const auto &attribute : k_global_attributes) {
                ReturnErrorOnFailure(aEncoder.Encode(attribute.path.mAttributeId));
            }
            return CHIP_NO_ERROR;
        });
        break;
    }
    case OtaSoftwareUpdateProvider::Attributes::AcceptedCommandList::Id: {
        return encoder.EncodeList([](const auto &aEncoder) {
            for (const auto &command : k_accepted_commands) {
                ReturnErrorOnFailure(aEncoder.Encode(command.path.mCommandId));
            }
            return CHIP_NO_ERROR;
        });
        break;
    }
    case OtaSoftwareUpdateProvider::Attributes::GeneratedCommandList::Id: {
        return encoder.EncodeList([](const auto &aEncoder) {
            for (const auto &command : k_generated_commands) {
                ReturnErrorOnFailure(aEncoder.Encode(command.mCommandId));
            }
            return CHIP_NO_ERROR;
        });
        break;
    }
    case OtaSoftwareUpdateProvider::Attributes::ClusterRevision::Id: {
        return encoder.Encode((uint8_t)0);
        break;
    }
    default:
        break;
    }
    return chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
}

ActionReturnStatus provider::WriteAttribute(const WriteAttributeRequest &request, AttributeValueDecoder &decoder)
{
    if (request.path.mEndpointId != 0) {
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }
    if (request.path.mClusterId != chip::app::Clusters::OtaSoftwareUpdateProvider::Id) {
        return chip::Protocols::InteractionModel::Status::UnsupportedCluster;
    }
    if (!chip::IsGlobalAttribute(request.path.mAttributeId)) {
        return chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
    return chip::Protocols::InteractionModel::Status::UnsupportedWrite;
}

std::optional<ActionReturnStatus> provider::Invoke(const InvokeRequest &request, TLVReader &input_arguments,
                                                   CommandHandler *handler)
{
    chip::app::DispatchSingleClusterCommand(request.path, input_arguments, handler);
    return std::nullopt;
}

EndpointEntry provider::FirstEndpoint()
{
    EndpointEntry ret = {
        .id = 0,
        .info = EndpointInfo(chip::kInvalidEndpointId),
    };
    return ret;
}

std::optional<EndpointInfo> provider::GetEndpointInfo(chip::EndpointId endpoint)
{
    if (endpoint == 0) {
        return EndpointInfo(chip::kInvalidEndpointId);
    }
    return std::nullopt;
}

static chip::DataVersion sDataVersion = 0;

ClusterEntry provider::FirstServerCluster(chip::EndpointId endpoint)
{
    if (endpoint == 0) {
        ClusterEntry ret = {
            .path = ConcreteClusterPath(0, OtaSoftwareUpdateProvider::Id),
            .info = ClusterInfo(sDataVersion),
        };
        return ret;
    }
    return ClusterEntry::kInvalid;
}

std::optional<ClusterInfo> provider::GetServerClusterInfo(const ConcreteClusterPath &path)
{
    if (path.mEndpointId == 0 && path.mClusterId == OtaSoftwareUpdateProvider::Id) {
        return ClusterInfo(sDataVersion);
    }
    return std::nullopt;
}

AttributeEntry provider::FirstAttribute(const ConcreteClusterPath &cluster)
{
    if (cluster.mEndpointId == 0 && cluster.mClusterId == OtaSoftwareUpdateProvider::Id) {
        return k_global_attributes[0];
    }
    return AttributeEntry::kInvalid;
}

AttributeEntry provider::NextAttribute(const ConcreteAttributePath &before)
{
    if (before.mEndpointId == 0 && before.mClusterId == OtaSoftwareUpdateProvider::Id) {
        size_t index = 0;
        for (index = 0; index < ArraySize(k_global_attributes); ++index) {
            if (before == k_global_attributes[index].path) {
                break;
            }
        }
        if (index < ArraySize(k_global_attributes) - 1) {
            return k_global_attributes[index + 1];
        }
    }
    return AttributeEntry::kInvalid;
}
std::optional<AttributeInfo> provider::GetAttributeInfo(const ConcreteAttributePath &path)
{
    if (path.mEndpointId == 0 && path.mClusterId == OtaSoftwareUpdateProvider::Id &&
        chip::IsGlobalAttribute(path.mAttributeId)) {
        return AttributeInfo();
    }
    return std::nullopt;
}

CommandEntry provider::FirstAcceptedCommand(const ConcreteClusterPath &cluster)
{
    if (cluster.mEndpointId == 0 && cluster.mClusterId == OtaSoftwareUpdateProvider::Id) {
        return k_accepted_commands[0];
    }
    return CommandEntry::kInvalid;
}
CommandEntry provider::NextAcceptedCommand(const ConcreteCommandPath &before)
{
    size_t index = 0;
    for (index = 0; index < ArraySize(k_accepted_commands); ++index) {
        if (before == k_accepted_commands[index].path) {
            break;
        }
    }
    if (index < ArraySize(k_accepted_commands) - 1) {
        return k_accepted_commands[index + 1];
    }
    return CommandEntry::kInvalid;
}

std::optional<CommandInfo> provider::GetAcceptedCommandInfo(const ConcreteCommandPath &path)
{
    size_t index = 0;
    for (index = 0; index < ArraySize(k_accepted_commands); ++index) {
        if (path == k_accepted_commands[index].path) {
            break;
        }
    }
    if (index < ArraySize(k_accepted_commands)) {
        return k_accepted_commands[index].info;
    }
    return std::nullopt;
}

ConcreteCommandPath provider::FirstGeneratedCommand(const ConcreteClusterPath &cluster)
{
    if (cluster.mEndpointId == 0 && cluster.mClusterId == OtaSoftwareUpdateProvider::Id) {
        return k_generated_commands[0];
    }
    return ConcreteCommandPath();
}

ConcreteCommandPath provider::NextGeneratedCommand(const ConcreteCommandPath &before)
{
    if (before == k_generated_commands[0]) {
        return k_generated_commands[1];
    }
    return ConcreteCommandPath();
}

} // namespace data_model
} // namespace controller
} // namespace esp_matter
