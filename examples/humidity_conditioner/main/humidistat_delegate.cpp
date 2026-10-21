/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <humidistat_delegate.h>

#include <app_priv.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Humidistat {

void ExampleHumidistatDelegate::OnModeChanged(HumidistatModeEnum newMode)
{
    app_driver_set_mode(static_cast<uint8_t>(newMode));
}

void ExampleHumidistatDelegate::OnSystemStateChanged(HumidistatSystemStateEnum newSystemState)
{
    app_driver_set_system_state(static_cast<uint8_t>(newSystemState));
}

void ExampleHumidistatDelegate::OnUserSetpointChanged(chip::Percent newUserSetpoint)
{
    app_driver_set_user_setpoint(newUserSetpoint);
}

void ExampleHumidistatDelegate::OnTargetSetpointChanged(chip::Percent newTargetSetpoint)
{
    app_driver_set_target_setpoint(newTargetSetpoint);
}

void ExampleHumidistatDelegate::OnMistTypeChanged(chip::BitMask<MistTypeBitmap> newMistType)
{
    app_driver_set_mist_type(newMistType.Raw());
}

void ExampleHumidistatDelegate::OnOptimalChanged(bool newOptimal)
{
    app_driver_set_optimal(newOptimal);
}

} // namespace Humidistat
} // namespace Clusters
} // namespace app
} // namespace chip
