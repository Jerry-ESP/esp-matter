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

/* THIS IS A GENERATED FILE, DO NOT EDIT */

#pragma once
#include <esp_matter_data_model.h>

namespace esp_matter {
namespace cluster {
namespace thermostat {

inline constexpr uint32_t Id = 0x0201;

namespace feature {
namespace Heating {
inline constexpr uint32_t Id = 0x1;
} /* Heating */
namespace Cooling {
inline constexpr uint32_t Id = 0x2;
} /* Cooling */
namespace Occupancy {
inline constexpr uint32_t Id = 0x4;
} /* Occupancy */
namespace ScheduleConfiguration {
inline constexpr uint32_t Id = 0x8;
} /* ScheduleConfiguration */
namespace Setback {
inline constexpr uint32_t Id = 0x10;
} /* Setback */
namespace AutoMode {
inline constexpr uint32_t Id = 0x20;
} /* AutoMode */
namespace LocalTemperatureNotExposed {
inline constexpr uint32_t Id = 0x40;
} /* LocalTemperatureNotExposed */
namespace MatterScheduleConfiguration {
inline constexpr uint32_t Id = 0x80;
} /* MatterScheduleConfiguration */
namespace Presets {
inline constexpr uint32_t Id = 0x100;
} /* Presets */
namespace ThermostatSuggestions {
inline constexpr uint32_t Id = 0x400;
} /* ThermostatSuggestions */
} /* feature */

namespace attribute {
namespace LocalTemperature {
inline constexpr uint32_t Id = 0x0000;
} /* LocalTemperature */
namespace OutdoorTemperature {
inline constexpr uint32_t Id = 0x0001;
} /* OutdoorTemperature */
namespace Occupancy {
inline constexpr uint32_t Id = 0x0002;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 1;
} /* Occupancy */
namespace AbsMinHeatSetpointLimit {
inline constexpr uint32_t Id = 0x0003;
} /* AbsMinHeatSetpointLimit */
namespace AbsMaxHeatSetpointLimit {
inline constexpr uint32_t Id = 0x0004;
} /* AbsMaxHeatSetpointLimit */
namespace AbsMinCoolSetpointLimit {
inline constexpr uint32_t Id = 0x0005;
} /* AbsMinCoolSetpointLimit */
namespace AbsMaxCoolSetpointLimit {
inline constexpr uint32_t Id = 0x0006;
} /* AbsMaxCoolSetpointLimit */
namespace PICoolingDemand {
inline constexpr uint32_t Id = 0x0007;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 100;
} /* PICoolingDemand */
namespace PIHeatingDemand {
inline constexpr uint32_t Id = 0x0008;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 100;
} /* PIHeatingDemand */
namespace HVACSystemTypeConfiguration {
inline constexpr uint32_t Id = 0x0009;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 63;
} /* HVACSystemTypeConfiguration */
namespace LocalTemperatureCalibration {
inline constexpr uint32_t Id = 0x0010;
} /* LocalTemperatureCalibration */
namespace OccupiedCoolingSetpoint {
inline constexpr uint32_t Id = 0x0011;
} /* OccupiedCoolingSetpoint */
namespace OccupiedHeatingSetpoint {
inline constexpr uint32_t Id = 0x0012;
} /* OccupiedHeatingSetpoint */
namespace UnoccupiedCoolingSetpoint {
inline constexpr uint32_t Id = 0x0013;
} /* UnoccupiedCoolingSetpoint */
namespace UnoccupiedHeatingSetpoint {
inline constexpr uint32_t Id = 0x0014;
} /* UnoccupiedHeatingSetpoint */
namespace MinHeatSetpointLimit {
inline constexpr uint32_t Id = 0x0015;
} /* MinHeatSetpointLimit */
namespace MaxHeatSetpointLimit {
inline constexpr uint32_t Id = 0x0016;
} /* MaxHeatSetpointLimit */
namespace MinCoolSetpointLimit {
inline constexpr uint32_t Id = 0x0017;
} /* MinCoolSetpointLimit */
namespace MaxCoolSetpointLimit {
inline constexpr uint32_t Id = 0x0018;
} /* MaxCoolSetpointLimit */
namespace MinSetpointDeadBand {
inline constexpr uint32_t Id = 0x0019;
inline constexpr int8_t Min = 0;
inline constexpr int8_t Max = 127;
} /* MinSetpointDeadBand */
namespace RemoteSensing {
inline constexpr uint32_t Id = 0x001A;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 7;
} /* RemoteSensing */
namespace ControlSequenceOfOperation {
inline constexpr uint32_t Id = 0x001B;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 5;
} /* ControlSequenceOfOperation */
namespace SystemMode {
inline constexpr uint32_t Id = 0x001C;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 8;
} /* SystemMode */
namespace StartOfWeek {
inline constexpr uint32_t Id = 0x0020;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 6;
} /* StartOfWeek */
namespace NumberOfWeeklyTransitions {
inline constexpr uint32_t Id = 0x0021;
} /* NumberOfWeeklyTransitions */
namespace NumberOfDailyTransitions {
inline constexpr uint32_t Id = 0x0022;
} /* NumberOfDailyTransitions */
namespace TemperatureSetpointHold {
inline constexpr uint32_t Id = 0x0023;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 1;
} /* TemperatureSetpointHold */
namespace TemperatureSetpointHoldDuration {
inline constexpr uint32_t Id = 0x0024;
inline constexpr uint16_t Min = 1;
inline constexpr uint16_t Max = 1440;
} /* TemperatureSetpointHoldDuration */
namespace ThermostatProgrammingOperationMode {
inline constexpr uint32_t Id = 0x0025;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 7;
} /* ThermostatProgrammingOperationMode */
namespace ThermostatRunningState {
inline constexpr uint32_t Id = 0x0029;
inline constexpr uint16_t Min = 0;
inline constexpr uint16_t Max = 65535;
} /* ThermostatRunningState */
namespace SetpointChangeSource {
inline constexpr uint32_t Id = 0x0030;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 2;
} /* SetpointChangeSource */
namespace SetpointChangeAmount {
inline constexpr uint32_t Id = 0x0031;
} /* SetpointChangeAmount */
namespace SetpointChangeSourceTimestamp {
inline constexpr uint32_t Id = 0x0032;
} /* SetpointChangeSourceTimestamp */
namespace OccupiedSetback {
inline constexpr uint32_t Id = 0x0034;
} /* OccupiedSetback */
namespace OccupiedSetbackMin {
inline constexpr uint32_t Id = 0x0035;
} /* OccupiedSetbackMin */
namespace OccupiedSetbackMax {
inline constexpr uint32_t Id = 0x0036;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 254;
} /* OccupiedSetbackMax */
namespace UnoccupiedSetback {
inline constexpr uint32_t Id = 0x0037;
} /* UnoccupiedSetback */
namespace UnoccupiedSetbackMin {
inline constexpr uint32_t Id = 0x0038;
} /* UnoccupiedSetbackMin */
namespace UnoccupiedSetbackMax {
inline constexpr uint32_t Id = 0x0039;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 254;
} /* UnoccupiedSetbackMax */
namespace EmergencyHeatDelta {
inline constexpr uint32_t Id = 0x003A;
} /* EmergencyHeatDelta */
namespace ACType {
inline constexpr uint32_t Id = 0x0040;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 4;
} /* ACType */
namespace ACCapacity {
inline constexpr uint32_t Id = 0x0041;
} /* ACCapacity */
namespace ACRefrigerantType {
inline constexpr uint32_t Id = 0x0042;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 3;
} /* ACRefrigerantType */
namespace ACCompressorType {
inline constexpr uint32_t Id = 0x0043;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 3;
} /* ACCompressorType */
namespace ACErrorCode {
inline constexpr uint32_t Id = 0x0044;
inline constexpr uint32_t Min = 0;
inline constexpr uint32_t Max = 4294967295;
} /* ACErrorCode */
namespace ACLouverPosition {
inline constexpr uint32_t Id = 0x0045;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 4;
} /* ACLouverPosition */
namespace ACCoilTemperature {
inline constexpr uint32_t Id = 0x0046;
} /* ACCoilTemperature */
namespace ACCapacityFormat {
inline constexpr uint32_t Id = 0x0047;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 0;
} /* ACCapacityFormat */
namespace PresetTypes {
inline constexpr uint32_t Id = 0x0048;
} /* PresetTypes */
namespace ScheduleTypes {
inline constexpr uint32_t Id = 0x0049;
} /* ScheduleTypes */
namespace NumberOfPresets {
inline constexpr uint32_t Id = 0x004A;
inline constexpr uint8_t Min = 1;
inline constexpr uint8_t Max = 255;
} /* NumberOfPresets */
namespace NumberOfSchedules {
inline constexpr uint32_t Id = 0x004B;
inline constexpr uint8_t Min = 1;
inline constexpr uint8_t Max = 255;
} /* NumberOfSchedules */
namespace NumberOfScheduleTransitions {
inline constexpr uint32_t Id = 0x004C;
inline constexpr uint8_t Min = 1;
inline constexpr uint8_t Max = 255;
} /* NumberOfScheduleTransitions */
namespace NumberOfScheduleTransitionPerDay {
inline constexpr uint32_t Id = 0x004D;
inline constexpr uint8_t Min = 1;
inline constexpr uint8_t Max = 255;
} /* NumberOfScheduleTransitionPerDay */
namespace ActivePresetHandle {
inline constexpr uint32_t Id = 0x004E;
} /* ActivePresetHandle */
namespace ActiveScheduleHandle {
inline constexpr uint32_t Id = 0x004F;
} /* ActiveScheduleHandle */
namespace Presets {
inline constexpr uint32_t Id = 0x0050;
} /* Presets */
namespace Schedules {
inline constexpr uint32_t Id = 0x0051;
} /* Schedules */
namespace SetpointHoldExpiryTimestamp {
inline constexpr uint32_t Id = 0x0052;
} /* SetpointHoldExpiryTimestamp */
namespace MaxThermostatSuggestions {
inline constexpr uint32_t Id = 0x0053;
inline constexpr uint8_t Min = 5;
inline constexpr uint8_t Max = 255;
} /* MaxThermostatSuggestions */
namespace ThermostatSuggestions {
inline constexpr uint32_t Id = 0x0054;
} /* ThermostatSuggestions */
namespace CurrentThermostatSuggestion {
inline constexpr uint32_t Id = 0x0055;
} /* CurrentThermostatSuggestion */
namespace ThermostatSuggestionNotFollowingReason {
inline constexpr uint32_t Id = 0x0056;
inline constexpr uint8_t Min = 0;
inline constexpr uint8_t Max = 255;
} /* ThermostatSuggestionNotFollowingReason */
} /* attribute */

namespace command {
namespace GetWeeklyScheduleResponse {
inline constexpr uint32_t Id = 0x00;
} /* GetWeeklyScheduleResponse */
namespace SetpointRaiseLower {
inline constexpr uint32_t Id = 0x00;
} /* SetpointRaiseLower */
namespace SetWeeklySchedule {
inline constexpr uint32_t Id = 0x01;
} /* SetWeeklySchedule */
namespace AddThermostatSuggestionResponse {
inline constexpr uint32_t Id = 0x02;
} /* AddThermostatSuggestionResponse */
namespace GetWeeklySchedule {
inline constexpr uint32_t Id = 0x02;
} /* GetWeeklySchedule */
namespace ClearWeeklySchedule {
inline constexpr uint32_t Id = 0x03;
} /* ClearWeeklySchedule */
namespace SetActiveScheduleRequest {
inline constexpr uint32_t Id = 0x05;
} /* SetActiveScheduleRequest */
namespace SetActivePresetRequest {
inline constexpr uint32_t Id = 0x06;
} /* SetActivePresetRequest */
namespace AddThermostatSuggestion {
inline constexpr uint32_t Id = 0x07;
} /* AddThermostatSuggestion */
namespace RemoveThermostatSuggestion {
inline constexpr uint32_t Id = 0x08;
} /* RemoveThermostatSuggestion */
} /* command */

} /* thermostat */
} /* cluster */
} /* esp_matter */
