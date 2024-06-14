/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class StaticSupportedModesManager : public chip::app::Clusters::ModeSelect::SupportedModesManager
{
private:
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;
    using SemanticTag          = Structs::SemanticTagStruct::Type;
    using storage_value_type   = const ModeOptionStructType;

    static ModeOptionsProvider epModeOptionsProviderList[FIXED_ENDPOINT_COUNT];

    void InitEndpointArray();

    void FreeSupportedModes(EndpointId endpointId) const;

    struct EndpointSpanPair
    {
        const EndpointId mEndpointId;
        const Span<storage_value_type> mSpan;

        EndpointSpanPair(const EndpointId aEndpointId, const Span<storage_value_type> && aSpan) :
            mEndpointId(aEndpointId), mSpan(aSpan)
        {}

        EndpointSpanPair() : mEndpointId(0), mSpan(Span<storage_value_type>()) {}
    };

    static storage_value_type coffeeOptions[];
    static storage_value_type lightOptions[];
    static const EndpointSpanPair supportedOptionsByEndpoints[2];

public:
    static const StaticSupportedModesManager instance;

    SupportedModesManager::ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                            const ModeOptionStructType ** dataPtr) const override;

    void CleanUp(EndpointId endpointId) const;

    StaticSupportedModesManager() { InitEndpointArray(); }

    ~StaticSupportedModesManager()
    {
        for (int i = 0; i < FIXED_ENDPOINT_COUNT; i++)
        {
            FreeSupportedModes(i);
        }
    }

    static inline const StaticSupportedModesManager & getStaticSupportedModesManagerInstance() { return instance; }
};

const SupportedModesManager * getSupportedModesManager();

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
