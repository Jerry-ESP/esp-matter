/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <app/clusters/humidistat-server/HumidistatCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Humidistat {

class ExampleHumidistatDelegate : public HumidistatDelegate {
public:
    void OnModeChanged(HumidistatModeEnum newMode) override;
    void OnSystemStateChanged(HumidistatSystemStateEnum newSystemState) override;
    void OnUserSetpointChanged(chip::Percent newUserSetpoint) override;
    void OnTargetSetpointChanged(chip::Percent newTargetSetpoint) override;
    void OnMistTypeChanged(chip::BitMask<MistTypeBitmap> newMistType) override;
    void OnOptimalChanged(bool newOptimal) override;
};

} // namespace Humidistat
} // namespace Clusters
} // namespace app
} // namespace chip
