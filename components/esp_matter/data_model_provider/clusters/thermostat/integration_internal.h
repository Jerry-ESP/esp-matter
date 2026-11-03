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

// Internal to the thermostat provider integration: shared between integration.cpp (default
// store-backed delegates, basic feature combinations) and integration_advanced.cpp (presets /
// matter-schedules combinations, linked only into applications that call
// Thermostat::SetDelegates).

#pragma once

#include "integration.h"

#include <esp_matter_data_model_priv.h>

#include <memory>

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Server.h>
#include <lib/support/attribute-storage-null-handling.h>
#include <clusters/Thermostat/Attributes.h>
#include <clusters/Thermostat/ClusterId.h>
#include <data_model/esp_matter_attribute_helpers.h>
#include <data_model/esp_matter_data_model.h>

namespace esp_matter {
namespace thermostat_integration {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using chip::Protocols::InteractionModel::Status;

// The delegates below are thin adapters over the esp-matter attribute store: reads fetch the
// stored value (presence in the store decides attribute support), writes go through
// set_val_internal so NONVOLATILE values persist to NVS and application attribute callbacks
// keep firing.

inline Status read_temp(EndpointId endpointId, AttributeId attributeId, int16_t &out)
{
    VerifyOrReturnValue(read_attribute_raw_value(endpointId, Thermostat::Id, attributeId, out),
                        Status::UnsupportedAttribute);
    return Status::Success;
}

inline Status read_u8(EndpointId endpointId, AttributeId attributeId, uint8_t &out)
{
    VerifyOrReturnValue(read_attribute_raw_value(endpointId, Thermostat::Id, attributeId, out),
                        Status::UnsupportedAttribute);
    return Status::Success;
}

inline Status write_val(EndpointId endpointId, AttributeId attributeId, esp_matter_attr_val_t val, bool &changed)
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

/** Setpoint-hold delegate over the esp-matter attribute store; attribute presence in the store
 *  decides support, so no application delegate is needed for hold. */
class StoreHoldDelegate : public ThermostatHold::Delegate {
public:
    void Init(EndpointId endpointId)
    {
        mEndpoint = endpointId;
    }

    TemperatureSetpointHoldEnum GetTemperatureSetpointHold() const override
    {
        uint8_t raw = to_underlying(TemperatureSetpointHoldEnum::kSetpointHoldOff);
        read_u8(mEndpoint, TemperatureSetpointHold::Id, raw);
        return static_cast<TemperatureSetpointHoldEnum>(raw);
    }

    Status SetTemperatureSetpointHold(TemperatureSetpointHoldEnum temperatureSetpointHold, bool &changed) override
    {
        return write_val(mEndpoint, TemperatureSetpointHold::Id,
                         esp_matter_enum8(to_underlying(temperatureSetpointHold)), changed);
    }

    DataModel::Nullable<uint16_t> GetTemperatureSetpointHoldDuration() const override
    {
        attribute_t *attr = attribute::get(mEndpoint, Thermostat::Id, TemperatureSetpointHoldDuration::Id);
        VerifyOrReturnValue(attr != nullptr, DataModel::NullNullable);
        esp_matter_attr_val_t val;
        VerifyOrReturnValue(get_stored_attr_val(cluster::get(mEndpoint, Thermostat::Id),
                                                TemperatureSetpointHoldDuration::Id, val) == ESP_OK,
                            DataModel::NullNullable);
        VerifyOrReturnValue(val.type == ESP_MATTER_VAL_TYPE_NULLABLE_UINT16, DataModel::NullNullable);
        VerifyOrReturnValue(!NumericAttributeTraits<uint16_t>::IsNullValue(val.val.u16), DataModel::NullNullable);
        return DataModel::MakeNullable(val.val.u16);
    }

    Status SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> temperatureSetpointHoldDuration,
                                              bool &changed) override
    {
        nullable<uint16_t> val;
        if (!temperatureSetpointHoldDuration.IsNull()) {
            val = nullable<uint16_t>(temperatureSetpointHoldDuration.Value());
        }
        return write_val(mEndpoint, TemperatureSetpointHoldDuration::Id, esp_matter_nullable_uint16(val), changed);
    }

    DataModel::Nullable<uint32_t> GetSetpointHoldExpiryTimestamp() const override
    {
        attribute_t *attr = attribute::get(mEndpoint, Thermostat::Id, SetpointHoldExpiryTimestamp::Id);
        VerifyOrReturnValue(attr != nullptr, DataModel::NullNullable);
        esp_matter_attr_val_t val;
        VerifyOrReturnValue(get_stored_attr_val(cluster::get(mEndpoint, Thermostat::Id),
                                                SetpointHoldExpiryTimestamp::Id, val) == ESP_OK,
                            DataModel::NullNullable);
        VerifyOrReturnValue(val.type == ESP_MATTER_VAL_TYPE_NULLABLE_UINT32, DataModel::NullNullable);
        VerifyOrReturnValue(!NumericAttributeTraits<uint32_t>::IsNullValue(val.val.u32), DataModel::NullNullable);
        return DataModel::MakeNullable(val.val.u32);
    }

    Status SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> setpointHoldExpiryTimestamp,
                                          bool &changed) override
    {
        nullable<uint32_t> val;
        if (!setpointHoldExpiryTimestamp.IsNull()) {
            val = nullable<uint32_t>(setpointHoldExpiryTimestamp.Value());
        }
        return write_val(mEndpoint, SetpointHoldExpiryTimestamp::Id, esp_matter_nullable_uint32(val), changed);
    }

private:
    EndpointId mEndpoint = kInvalidEndpointId;
};

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

struct ThermostatEndpoint {
    StoreCoreDelegate core;
    StoreSetpointsDelegate setpoints;
    StoreOccupancyDelegate occupancy;
    StoreHoldDelegate hold;
    EndpointDelegates app;
    std::unique_ptr<ClusterHolder> holder;
};

/** Access the per-endpoint integration state (creates the entry on first use). */
ThermostatEndpoint &GetEndpoint(chip::EndpointId endpointId);

/** Builder for the presets / matter-schedules feature combinations; installed by
 *  Thermostat::SetDelegates (integration_advanced.cpp) so those template instantiations are
 *  only linked into applications that use them. */
using AdvancedBuilderFn = std::unique_ptr<ClusterHolder> (*)(const BuildContext &ctx, ThermostatEndpoint &entry);
extern AdvancedBuilderFn gAdvancedBuilder;

} // namespace thermostat_integration
} // namespace esp_matter
