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

#include <esp_matter_data_model_priv.h>

#include <memory>

#include <app/ClusterCallbacks.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/server/Server.h>
#include <lib/support/attribute-storage-null-handling.h>
#include <clusters/Thermostat/Attributes.h>
#include <clusters/Thermostat/ClusterId.h>
#include <data_model/esp_matter_attribute_helpers.h>
#include <data_model/esp_matter_data_model.h>
#include <data_model_provider/esp_matter_data_model_provider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DefaultTimerDelegate.h>
#include <unordered_map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace esp_matter;
using chip::Protocols::InteractionModel::Status;

namespace {

// The default delegates below are thin adapters over the esp-matter attribute store: reads
// fetch the stored value (presence in the store decides attribute support), writes go through
// set_val_internal so NONVOLATILE values persist to NVS and application attribute callbacks
// keep firing. Features whose state and semantics the application owns (presets, schedules,
// suggestions, sensors, hold) use application delegates registered via Thermostat::SetDelegates.

Status read_temp(EndpointId endpointId, AttributeId attributeId, int16_t &out)
{
    VerifyOrReturnValue(read_attribute_raw_value(endpointId, Thermostat::Id, attributeId, out),
                        Status::UnsupportedAttribute);
    return Status::Success;
}

Status read_u8(EndpointId endpointId, AttributeId attributeId, uint8_t &out)
{
    VerifyOrReturnValue(read_attribute_raw_value(endpointId, Thermostat::Id, attributeId, out),
                        Status::UnsupportedAttribute);
    return Status::Success;
}

Status write_val(EndpointId endpointId, AttributeId attributeId, esp_matter_attr_val_t val, bool &changed)
{
    changed          = false;
    attribute_t *attr = attribute::get(endpointId, Thermostat::Id, attributeId);
    VerifyOrReturnValue(attr != nullptr, Status::UnsupportedAttribute);
    esp_err_t err = attribute::set_val_internal(attr, &val);
    if (err == ESP_OK) {
        changed = true;
        return Status::Success;
    }
    if (err == ESP_ERR_NOT_FINISHED) {
        return Status::Success;
    }
    if (err == ESP_ERR_INVALID_ARG) {
        return Status::ConstraintError;
    }
    return Status::Failure;
}

/** Core thermostat delegate over the esp-matter attribute store, with optional forwarding to an
 *  application-provided delegate registered via Thermostat::SetDefaultDelegate(). */
class StoreCoreDelegate : public Thermostat::Delegate {
public:
    void Init(EndpointId endpointId)
    {
        mEndpoint = endpointId;
    }
    void SetWrapped(Thermostat::Delegate *delegate)
    {
        mWrapped = delegate;
    }

    FabricTable &GetFabricTable() const override
    {
        return mWrapped ? mWrapped->GetFabricTable() : Server::GetInstance().GetFabricTable();
    }

    CHIP_ERROR Startup(ServerClusterContext &context) override
    {
        return mWrapped ? mWrapped->Startup(context) : Delegate::Startup(context);
    }

    void Shutdown(ClusterShutdownType type) override
    {
        if (mWrapped) {
            mWrapped->Shutdown(type);
        } else {
            Delegate::Shutdown(type);
        }
    }

    DataModel::Nullable<int16_t> GetLocalTemperature() const override
    {
        if (mWrapped) {
            return mWrapped->GetLocalTemperature();
        }
        int16_t raw;
        if (ReadNullableTemp(LocalTemperature::Id, raw)) {
            return DataModel::MakeNullable(raw);
        }
        return DataModel::NullNullable;
    }

    Status SetLocalTemperature(DataModel::Nullable<int16_t> localTemperature, bool &changed) override
    {
        if (mWrapped) {
            return mWrapped->SetLocalTemperature(localTemperature, changed);
        }
        nullable<int16_t> val;
        if (!localTemperature.IsNull()) {
            val = nullable<int16_t>(localTemperature.Value());
        }
        return write_val(mEndpoint, LocalTemperature::Id, esp_matter_nullable_int16(val), changed);
    }

    Status GetOutdoorTemperature(DataModel::Nullable<int16_t> &outdoorTemp) const override
    {
        if (mWrapped) {
            return mWrapped->GetOutdoorTemperature(outdoorTemp);
        }
        VerifyOrReturnValue(attribute::get(mEndpoint, Thermostat::Id, OutdoorTemperature::Id) != nullptr,
                            Status::UnsupportedAttribute);
        int16_t raw;
        outdoorTemp = ReadNullableTemp(OutdoorTemperature::Id, raw) ? DataModel::MakeNullable(raw)
                      : DataModel::Nullable<int16_t>();
        return Status::Success;
    }

    SystemModeEnum GetSystemMode() const override
    {
        if (mWrapped) {
            return mWrapped->GetSystemMode();
        }
        uint8_t raw = to_underlying(SystemModeEnum::kOff);
        read_u8(mEndpoint, SystemMode::Id, raw);
        return static_cast<SystemModeEnum>(raw);
    }

    Status SetSystemMode(SystemModeEnum systemMode, bool &changed) override
    {
        if (mWrapped) {
            return mWrapped->SetSystemMode(systemMode, changed);
        }
        return write_val(mEndpoint, SystemMode::Id, esp_matter_enum8(to_underlying(systemMode)), changed);
    }

    ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override
    {
        if (mWrapped) {
            return mWrapped->GetControlSequenceOfOperation();
        }
        uint8_t raw = to_underlying(ControlSequenceOfOperationEnum::kCoolingAndHeating);
        read_u8(mEndpoint, ControlSequenceOfOperation::Id, raw);
        return static_cast<ControlSequenceOfOperationEnum>(raw);
    }

    Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation, bool &changed) override
    {
        if (mWrapped) {
            return mWrapped->SetControlSequenceOfOperation(controlSequenceOfOperation, changed);
        }
        return write_val(mEndpoint, ControlSequenceOfOperation::Id,
                         esp_matter_enum8(to_underlying(controlSequenceOfOperation)), changed);
    }

    Status GetRunningMode(ThermostatRunningModeEnum &runningMode) const override
    {
        if (mWrapped) {
            return mWrapped->GetRunningMode(runningMode);
        }
        uint8_t raw;
        Status status = read_u8(mEndpoint, ThermostatRunningMode::Id, raw);
        VerifyOrReturnValue(status == Status::Success, status);
        runningMode = static_cast<ThermostatRunningModeEnum>(raw);
        return Status::Success;
    }

    Status SetRunningMode(ThermostatRunningModeEnum runningMode, bool &changed) override
    {
        if (mWrapped) {
            return mWrapped->SetRunningMode(runningMode, changed);
        }
        return write_val(mEndpoint, ThermostatRunningMode::Id, esp_matter_enum8(to_underlying(runningMode)), changed);
    }

    Status GetRunningState(BitMask<RelayStateBitmap> &runningState) const override
    {
        if (mWrapped) {
            return mWrapped->GetRunningState(runningState);
        }
        uint16_t raw;
        VerifyOrReturnValue(read_attribute_raw_value(mEndpoint, Thermostat::Id, ThermostatRunningState::Id, raw),
                            Status::UnsupportedAttribute);
        runningState = BitMask<RelayStateBitmap>(raw);
        return Status::Success;
    }

    Status SetRunningState(BitMask<RelayStateBitmap> runningState, bool &changed) override
    {
        if (mWrapped) {
            return mWrapped->SetRunningState(runningState, changed);
        }
        return write_val(mEndpoint, ThermostatRunningState::Id,
                         esp_matter_bitmap16(static_cast<uint16_t>(runningState.Raw())), changed);
    }

    int8_t GetLocalTemperatureCalibration() const override
    {
        if (mWrapped) {
            return mWrapped->GetLocalTemperatureCalibration();
        }
        int8_t raw = 0;
        read_attribute_raw_value(mEndpoint, Thermostat::Id, LocalTemperatureCalibration::Id, raw);
        return raw;
    }

    Status SetLocalTemperatureCalibration(int8_t localTemperatureCalibration, bool &changed) override
    {
        if (mWrapped) {
            return mWrapped->SetLocalTemperatureCalibration(localTemperatureCalibration, changed);
        }
        return write_val(mEndpoint, LocalTemperatureCalibration::Id, esp_matter_int8(localTemperatureCalibration),
                         changed);
    }

    Status GetRemoteSensing(BitMask<RemoteSensingBitmap> &remoteSensing) const override
    {
        if (mWrapped) {
            return mWrapped->GetRemoteSensing(remoteSensing);
        }
        uint8_t raw;
        Status status = read_u8(mEndpoint, RemoteSensing::Id, raw);
        VerifyOrReturnValue(status == Status::Success, status);
        remoteSensing = BitMask<RemoteSensingBitmap>(raw);
        return Status::Success;
    }

    Status SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing, bool &changed) override
    {
        if (mWrapped) {
            return mWrapped->SetRemoteSensing(remoteSensing, changed);
        }
        return write_val(mEndpoint, RemoteSensing::Id, esp_matter_bitmap8(remoteSensing.Raw()), changed);
    }

private:
    bool ReadNullableTemp(AttributeId attributeId, int16_t &out) const
    {
        attribute_t *attr = attribute::get(mEndpoint, Thermostat::Id, attributeId);
        VerifyOrReturnValue(attr != nullptr, false);
        esp_matter_attr_val_t val;
        VerifyOrReturnValue(get_stored_attr_val(const_cast<cluster_t *>(cluster::get(mEndpoint, Thermostat::Id)),
                                                attributeId, val) == ESP_OK, false);
        VerifyOrReturnValue(val.type == ESP_MATTER_VAL_TYPE_NULLABLE_INT16, false);
        VerifyOrReturnValue(!NumericAttributeTraits<int16_t>::IsNullValue(val.val.i16), false);
        out = val.val.i16;
        return true;
    }

    EndpointId mEndpoint              = kInvalidEndpointId;
    Thermostat::Delegate *mWrapped    = nullptr;
};

/** Heating/cooling/auto setpoint delegate over the esp-matter attribute store. */
class StoreSetpointsDelegate : public ThermostatHeatingSetpoints::Delegate,
    public ThermostatCoolingSetpoints::Delegate,
    public ThermostatAutoSetpoints::Delegate {
public:
    void Init(EndpointId endpointId)
    {
        mEndpoint = endpointId;
    }

    // ThermostatHeatingSetpoints::Delegate
    Status GetOccupiedHeatingSetpoint(int16_t &v) const override
    {
        return read_temp(mEndpoint, OccupiedHeatingSetpoint::Id, v);
    }
    Status SetOccupiedHeatingSetpoint(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, OccupiedHeatingSetpoint::Id, esp_matter_int16(v), changed);
    }
    Status GetUnoccupiedHeatingSetpoint(int16_t &v) const override
    {
        return read_temp(mEndpoint, UnoccupiedHeatingSetpoint::Id, v);
    }
    Status SetUnoccupiedHeatingSetpoint(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, UnoccupiedHeatingSetpoint::Id, esp_matter_int16(v), changed);
    }
    Status GetAbsMinHeatSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, AbsMinHeatSetpointLimit::Id, v);
    }
    Status GetAbsMaxHeatSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, AbsMaxHeatSetpointLimit::Id, v);
    }
    Status GetMinHeatSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, MinHeatSetpointLimit::Id, v);
    }
    Status SetMinHeatSetpointLimit(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, MinHeatSetpointLimit::Id, esp_matter_int16(v), changed);
    }
    Status GetMaxHeatSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, MaxHeatSetpointLimit::Id, v);
    }
    Status SetMaxHeatSetpointLimit(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, MaxHeatSetpointLimit::Id, esp_matter_int16(v), changed);
    }

    // ThermostatCoolingSetpoints::Delegate
    Status GetOccupiedCoolingSetpoint(int16_t &v) const override
    {
        return read_temp(mEndpoint, OccupiedCoolingSetpoint::Id, v);
    }
    Status SetOccupiedCoolingSetpoint(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, OccupiedCoolingSetpoint::Id, esp_matter_int16(v), changed);
    }
    Status GetUnoccupiedCoolingSetpoint(int16_t &v) const override
    {
        return read_temp(mEndpoint, UnoccupiedCoolingSetpoint::Id, v);
    }
    Status SetUnoccupiedCoolingSetpoint(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, UnoccupiedCoolingSetpoint::Id, esp_matter_int16(v), changed);
    }
    Status GetAbsMinCoolSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, AbsMinCoolSetpointLimit::Id, v);
    }
    Status GetAbsMaxCoolSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, AbsMaxCoolSetpointLimit::Id, v);
    }
    Status GetMinCoolSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, MinCoolSetpointLimit::Id, v);
    }
    Status SetMinCoolSetpointLimit(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, MinCoolSetpointLimit::Id, esp_matter_int16(v), changed);
    }
    Status GetMaxCoolSetpointLimit(int16_t &v) const override
    {
        return read_temp(mEndpoint, MaxCoolSetpointLimit::Id, v);
    }
    Status SetMaxCoolSetpointLimit(int16_t v, bool &changed) override
    {
        return write_val(mEndpoint, MaxCoolSetpointLimit::Id, esp_matter_int16(v), changed);
    }

    // ThermostatAutoSetpoints::Delegate
    Status GetMinDeadband(int16_t &minDeadband) const override
    {
        // Stored as SignedTemperature: int8_t in tenths of a degree; delegate reports hundredths.
        int8_t raw;
        VerifyOrReturnValue(read_attribute_raw_value(mEndpoint, Thermostat::Id, MinSetpointDeadBand::Id, raw),
                            Status::UnsupportedAttribute);
        minDeadband = static_cast<int16_t>(raw * 10);
        return Status::Success;
    }

private:
    EndpointId mEndpoint = kInvalidEndpointId;
};

/** Occupancy delegate over the esp-matter attribute store. */
class StoreOccupancyDelegate : public ThermostatOccupancy::Delegate {
public:
    void Init(EndpointId endpointId)
    {
        mEndpoint = endpointId;
    }

    BitMask<OccupancyBitmap> GetOccupancy() const override
    {
        uint8_t raw = to_underlying(OccupancyBitmap::kOccupied);
        read_u8(mEndpoint, Occupancy::Id, raw);
        return BitMask<OccupancyBitmap>(raw);
    }

    Status SetOccupancy(BitMask<OccupancyBitmap> occupied, bool &changed) override
    {
        return write_val(mEndpoint, Occupancy::Id, esp_matter_bitmap8(occupied.Raw()), changed);
    }

private:
    EndpointId mEndpoint = kInvalidEndpointId;
};

// The set of features a ThermostatCluster instance exposes is a compile-time property of the
// template (each enabled feature's attributes are published by template inclusion, not gated
// by the runtime feature map). The builder chain below therefore instantiates one cluster type
// per feature combination and selects it at init time from the endpoint's feature map and the
// application-provided delegates.
class ClusterHolder {
public:
    virtual ~ClusterHolder() = default;
    virtual ServerClusterRegistration &Registration() = 0;
    virtual ServerClusterInterface &Cluster() = 0;
};

template <typename CLUSTER>
class ClusterHolderImpl final : public ClusterHolder {
public:
    template <typename... Args>
    ClusterHolderImpl(Args &&... args) : mCluster(std::forward<Args>(args)...), mRegistration(mCluster) {}
    ServerClusterRegistration &Registration() override
    {
        return mRegistration;
    }
    ServerClusterInterface &Cluster() override
    {
        return mCluster;
    }

private:
    CLUSTER mCluster;
    ServerClusterRegistration mRegistration;
};

struct BuildContext {
    EndpointId endpointId;
    BitFlags<Thermostat::Feature> features;
    const ThermostatClusterBase::Config &config;
    const EndpointDelegates &app;
};

template <typename... Ds>
std::unique_ptr<ClusterHolder> MakeHolder(const BuildContext &ctx, Ds &... ds)
{
    using Cluster = ThermostatCluster<std::decay_t<Ds>...>;
    return std::make_unique<ClusterHolderImpl<Cluster>>(ctx.endpointId, ctx.features, ctx.config, ds...);
}

template <typename... Ds>
std::unique_ptr<ClusterHolder> BuildSensors(const BuildContext &ctx, Ds &... ds)
{
    if (ctx.features.Has(Feature::kThermostatSensors)) {
        return MakeHolder(ctx, ds..., *ctx.app.sensors);
    }
    return MakeHolder(ctx, ds...);
}

template <typename... Ds>
std::unique_ptr<ClusterHolder> BuildSchedules(const BuildContext &ctx, Ds &... ds)
{
    if (ctx.features.Has(Feature::kMatterScheduleConfiguration)) {
        return BuildSensors(ctx, ds..., *ctx.app.schedules);
    }
    return BuildSensors(ctx, ds...);
}

template <typename... Ds>
std::unique_ptr<ClusterHolder> BuildPresets(const BuildContext &ctx, Ds &... ds)
{
    if (ctx.features.Has(Feature::kPresets)) {
        // Upstream static-asserts that suggestions require presets, so this axis only exists
        // inside the presets branch.
        if (ctx.features.Has(Feature::kThermostatSuggestions)) {
            return BuildSchedules(ctx, ds..., *ctx.app.presets, *ctx.app.suggestions);
        }
        return BuildSchedules(ctx, ds..., *ctx.app.presets);
    }
    return BuildSchedules(ctx, ds...);
}

template <typename... Ds>
std::unique_ptr<ClusterHolder> BuildHold(const BuildContext &ctx, Ds &... ds)
{
    if (ctx.app.hold != nullptr) {
        return BuildPresets(ctx, ds..., *ctx.app.hold);
    }
    return BuildPresets(ctx, ds...);
}

struct ThermostatEndpoint {
    StoreCoreDelegate core;
    StoreSetpointsDelegate setpoints;
    StoreOccupancyDelegate occupancy;
    EndpointDelegates app;
    std::unique_ptr<ClusterHolder> holder;
};

std::unordered_map<EndpointId, ThermostatEndpoint> gServers;

std::unique_ptr<ClusterHolder> BuildCluster(const BuildContext &ctx, ThermostatEndpoint &entry)
{
    if (ctx.features.Has(Feature::kOccupancy)) {
        return BuildHold(ctx, entry.core, entry.setpoints, entry.occupancy);
    }
    return BuildHold(ctx, entry.core, entry.setpoints);
}

/** Validate that every requested feature has its application delegate. */
bool ValidateFeatureSupport(EndpointId endpointId, const BitFlags<Thermostat::Feature> &features,
                            const EndpointDelegates &app)
{
    const struct {
        Feature feature;
        const void *delegate;
        const char *name;
    } axes[] = {
        { Feature::kPresets, app.presets, "presets" },
        { Feature::kThermostatSuggestions, app.suggestions, "suggestions" },
        { Feature::kMatterScheduleConfiguration, app.schedules, "matter schedules" },
        { Feature::kThermostatSensors, app.sensors, "sensors" },
    };
    for (const auto &axis : axes) {
        if (!features.Has(axis.feature)) {
            continue;
        }
        if (axis.delegate == nullptr) {
            ChipLogError(AppServer, "Thermostat: %s feature needs an application delegate (Thermostat::SetDelegates); "
                         "endpoint %u not registered", axis.name, endpointId);
            return false;
        }
    }
    if (features.Has(Feature::kThermostatSuggestions) && !features.Has(Feature::kPresets)) {
        ChipLogError(AppServer, "Thermostat: suggestions feature requires presets; endpoint %u not registered", endpointId);
        return false;
    }
    if (features.HasAny(Feature::kScheduleConfiguration, Feature::kSetback)) {
        ChipLogError(AppServer, "Thermostat: deprecated schedule-configuration/setback features are not supported by the "
                     "code-driven cluster; endpoint %u not registered", endpointId);
        return false;
    }
    return true;
}

OptionalAttributes BuildOptionalAttributes(EndpointId endpointId, const BitFlags<Thermostat::Feature> &features,
                                           bool holdActive)
{
    cluster_t *cluster = cluster::get(endpointId, Thermostat::Id);
    OptionalAttributes optionalAttributes;
    VerifyOrReturnValue(cluster != nullptr, optionalAttributes);

    auto has = [cluster](AttributeId id) {
        return is_attribute_enabled(cluster, id);
    };

    optionalAttributes.AbsMinHeatSetpointLimit = features.Has(Feature::kHeating) && has(AbsMinHeatSetpointLimit::Id);
    optionalAttributes.AbsMaxHeatSetpointLimit = features.Has(Feature::kHeating) && has(AbsMaxHeatSetpointLimit::Id);
    optionalAttributes.AbsMinCoolSetpointLimit = features.Has(Feature::kCooling) && has(AbsMinCoolSetpointLimit::Id);
    optionalAttributes.AbsMaxCoolSetpointLimit = features.Has(Feature::kCooling) && has(AbsMaxCoolSetpointLimit::Id);

    optionalAttributes.LocalTemperatureCalibration =
        !features.Has(Feature::kLocalTemperatureNotExposed) && has(LocalTemperatureCalibration::Id);

    optionalAttributes.MinHeatSetpointLimit = features.Has(Feature::kHeating) && has(MinHeatSetpointLimit::Id);
    optionalAttributes.MaxHeatSetpointLimit = features.Has(Feature::kHeating) && has(MaxHeatSetpointLimit::Id);
    optionalAttributes.MinCoolSetpointLimit = features.Has(Feature::kCooling) && has(MinCoolSetpointLimit::Id);
    optionalAttributes.MaxCoolSetpointLimit = features.Has(Feature::kCooling) && has(MaxCoolSetpointLimit::Id);

    optionalAttributes.RemoteSensing         = has(RemoteSensing::Id);
    optionalAttributes.ThermostatRunningMode = features.Has(Feature::kAutoMode) && has(ThermostatRunningMode::Id);
    // Hold attributes need an application ThermostatHold delegate.
    optionalAttributes.TemperatureSetpointHold         = holdActive && has(TemperatureSetpointHold::Id);
    optionalAttributes.TemperatureSetpointHoldDuration = holdActive && has(TemperatureSetpointHoldDuration::Id);
    optionalAttributes.SetpointHoldExpiryTimestamp     = holdActive && has(SetpointHoldExpiryTimestamp::Id);
    optionalAttributes.ThermostatRunningState          = has(ThermostatRunningState::Id);
    optionalAttributes.SetpointChangeSource            = has(SetpointChangeSource::Id);
    optionalAttributes.SetpointChangeAmount            = has(SetpointChangeAmount::Id);
    optionalAttributes.SetpointChangeSourceTimestamp   = has(SetpointChangeSourceTimestamp::Id);
    optionalAttributes.OutdoorTemperature              = has(OutdoorTemperature::Id);
    optionalAttributes.CriticalFreezeProtection = features.Has(Feature::kHeating) && has(CriticalFreezeProtection::Id);
    optionalAttributes.CriticalOverheatProtection = features.Has(Feature::kCooling) && has(CriticalOverheatProtection::Id);

    return optionalAttributes;
}

TimerDelegate &GetTimerDelegate()
{
    static DefaultTimerDelegate sTimerDelegate;
    return sTimerDelegate;
}

} // namespace

void ESPMatterThermostatClusterServerInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(cluster::get(endpointId, Thermostat::Id) != nullptr,
                   ChipLogError(AppServer, "Thermostat: cluster missing in esp-matter data model for endpoint %u", endpointId));

    auto &entry = gServers[endpointId];
    if (!entry.holder) {
        const uint32_t featureMap = read_feature_map_u32(endpointId, Thermostat::Id);
        const BitFlags<Thermostat::Feature> features(featureMap);

        VerifyOrReturn(ValidateFeatureSupport(endpointId, features, entry.app));

        entry.core.Init(endpointId);
        entry.core.SetWrapped(entry.app.core);
        entry.setpoints.Init(endpointId);
        entry.occupancy.Init(endpointId);

        const bool holdActive = entry.app.hold != nullptr;
        ThermostatClusterBase::Config config(BuildOptionalAttributes(endpointId, features, holdActive),
                                             GetTimerDelegate());
        entry.holder = BuildCluster({ endpointId, features, config, entry.app }, entry);
        VerifyOrReturn(entry.holder != nullptr,
                       ChipLogError(AppServer, "Thermostat: failed to create cluster for endpoint %u", endpointId));
    }

    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Register(entry.holder->Registration());
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Thermostat register failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void ESPMatterThermostatClusterServerShutdownCallback(EndpointId endpointId, ClusterShutdownType shutdownType)
{
    auto it = gServers.find(endpointId);
    VerifyOrReturn(it != gServers.end());
    VerifyOrReturn(it->second.holder != nullptr);
    CHIP_ERROR err = esp_matter::data_model::provider::get_instance().registry().Unregister(&it->second.holder->Cluster(),
                                                                                            shutdownType);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "Thermostat unregister failed ep %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    if (shutdownType == ClusterShutdownType::kPermanentRemove) {
        it->second.holder.reset();
        gServers.erase(it);
    }
}

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void SetDelegates(EndpointId endpointId, const EndpointDelegates &delegates)
{
    auto &entry = gServers[endpointId];
    entry.app = delegates;
    entry.core.SetWrapped(delegates.core);
    if (entry.holder) {
        ChipLogError(AppServer, "Thermostat: SetDelegates called after cluster creation on endpoint %u; only the core "
                     "delegate override takes effect", endpointId);
    }
}

void SetDefaultDelegate(EndpointId endpointId, Delegate *delegate)
{
    auto &entry = gServers[endpointId];
    entry.app.core = delegate;
    entry.core.SetWrapped(delegate);
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
