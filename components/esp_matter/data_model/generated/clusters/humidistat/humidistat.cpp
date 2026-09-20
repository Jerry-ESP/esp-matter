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

#include <esp_log.h>
#include <esp_matter_core.h>
#include <esp_matter.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/callback.h>
#include <app/InteractionModelEngine.h>
#include <zap_common/app/PluginApplicationCallbacks.h>
#include <esp_matter_delegate_callbacks.h>
#include <humidistat.h>
#include <humidistat_ids.h>
#include <binding.h>
#include <esp_matter_data_model_priv.h>
#include <app/ClusterCallbacks.h>

using namespace chip::app::Clusters;
using chip::app::CommandHandler;
using chip::app::DataModel::Decode;
using chip::TLV::TLVReader;
using namespace esp_matter;
using namespace esp_matter::cluster;
using namespace esp_matter::cluster::delegate_cb;

static const char *TAG = "humidistat_cluster";
constexpr uint16_t cluster_revision = 1;

namespace esp_matter {
namespace cluster {
namespace humidistat {

namespace feature {
namespace humidifier {
uint32_t get_id()
{
    return Humidifier::Id;
}

esp_err_t add(cluster_t *cluster, config_t *config)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    VerifyOrReturnError(config, ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());
    attribute::create_mist_type(cluster, config->mist_type);

    return ESP_OK;
}
} /* humidifier */

namespace dehumidifier {
uint32_t get_id()
{
    return Dehumidifier::Id;
}

esp_err_t add(cluster_t *cluster)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());

    return ESP_OK;
}
} /* dehumidifier */

namespace continuous {
uint32_t get_id()
{
    return Continuous::Id;
}

esp_err_t add(cluster_t *cluster, config_t *config)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    VerifyOrReturnError(config, ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());
    attribute::create_continuous(cluster, config->continuous);

    return ESP_OK;
}
} /* continuous */

namespace sensor {
uint32_t get_id()
{
    return Sensor::Id;
}

esp_err_t add(cluster_t *cluster, config_t *config)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    VerifyOrReturnError(config, ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());
    attribute::create_user_setpoint(cluster, config->user_setpoint);
    attribute::create_min_setpoint(cluster, config->min_setpoint);
    attribute::create_max_setpoint(cluster, config->max_setpoint);
    attribute::create_step(cluster, config->step);

    return ESP_OK;
}
} /* sensor */

namespace fan_auto {
uint32_t get_id()
{
    return FanAuto::Id;
}

esp_err_t add(cluster_t *cluster)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnError(((has_feature(humidifier)) && (has_feature(dehumidifier)) && (has_feature(sensor))), ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());

    return ESP_OK;
}
} /* fan_auto */

namespace fan_only {
uint32_t get_id()
{
    return FanOnly::Id;
}

esp_err_t add(cluster_t *cluster)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());

    return ESP_OK;
}
} /* fan_only */

namespace optimal {
uint32_t get_id()
{
    return Optimal::Id;
}

esp_err_t add(cluster_t *cluster, config_t *config)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    VerifyOrReturnError(config, ESP_ERR_INVALID_ARG);
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnError(has_feature(sensor), ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());
    attribute::create_target_setpoint(cluster, config->target_setpoint);
    attribute::create_optimal(cluster, config->optimal);

    return ESP_OK;
}
} /* optimal */

namespace warm_mist {
uint32_t get_id()
{
    return WarmMist::Id;
}

esp_err_t add(cluster_t *cluster)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnError(has_feature(humidifier), ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());

    return ESP_OK;
}
} /* warm_mist */

namespace cold_mist {
uint32_t get_id()
{
    return ColdMist::Id;
}

esp_err_t add(cluster_t *cluster)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnError(has_feature(humidifier), ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());

    return ESP_OK;
}
} /* cold_mist */

namespace cond_pump {
uint32_t get_id()
{
    return CondPump::Id;
}

esp_err_t add(cluster_t *cluster, config_t *config)
{
    VerifyOrReturnError(cluster, ESP_ERR_INVALID_ARG);
    VerifyOrReturnError(config, ESP_ERR_INVALID_ARG);
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnError(has_feature(cond_pump), ESP_ERR_INVALID_ARG);
    update_feature_map(cluster, get_id());
    attribute::create_cond_pump_enabled(cluster, config->cond_pump_enabled);
    attribute::create_cond_run_count(cluster, 0);

    return ESP_OK;
}
} /* cold_mist */

} /* feature */

namespace attribute {
attribute_t *create_supported_modes(cluster_t *cluster, uint8_t *value, uint16_t length, uint16_t count)
{
    return esp_matter::attribute::create(cluster, SupportedModes::Id, ATTRIBUTE_FLAG_MANAGED_INTERNALLY, esp_matter_attr_val(value, length, count));
}

attribute_t *create_mode(cluster_t *cluster, uint8_t value)
{
    attribute_t *attribute = esp_matter::attribute::create(cluster, Mode::Id, ATTRIBUTE_FLAG_NONE, esp_matter_enum8(value));
    esp_matter::attribute::add_bounds(attribute, esp_matter_enum8(0), esp_matter_enum8(4));
    return attribute;
}

attribute_t *create_system_state(cluster_t *cluster, uint8_t value)
{
    attribute_t *attribute = esp_matter::attribute::create(cluster, SystemState::Id, ATTRIBUTE_FLAG_NONE, esp_matter_enum8(value));
    esp_matter::attribute::add_bounds(attribute, esp_matter_enum8(0), esp_matter_enum8(4));
    return attribute;
}

attribute_t *create_user_setpoint(cluster_t *cluster, uint8_t value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(sensor), NULL);
    attribute_t *attribute = esp_matter::attribute::create(cluster, UserSetpoint::Id, ATTRIBUTE_FLAG_NONE, esp_matter_uint8(value));
    esp_matter::attribute::add_bounds(attribute, esp_matter_uint8(0), esp_matter_uint8(254));
    return attribute;
}

attribute_t *create_min_setpoint(cluster_t *cluster, uint8_t value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(sensor), NULL);
    attribute_t *attribute = esp_matter::attribute::create(cluster, MinSetpoint::Id, ATTRIBUTE_FLAG_NONE, esp_matter_uint8(value));
    esp_matter::attribute::add_bounds(attribute, esp_matter_uint8(0), esp_matter_uint8(99));
    return attribute;
}

attribute_t *create_max_setpoint(cluster_t *cluster, uint8_t value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(sensor), NULL);
    attribute_t *attribute = esp_matter::attribute::create(cluster, MaxSetpoint::Id, ATTRIBUTE_FLAG_NONE, esp_matter_uint8(value));
    esp_matter::attribute::add_bounds(attribute, esp_matter_uint8(1), esp_matter_uint8(100));
    return attribute;
}

attribute_t *create_step(cluster_t *cluster, uint8_t value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(sensor), NULL);
    attribute_t *attribute = esp_matter::attribute::create(cluster, Step::Id, ATTRIBUTE_FLAG_NONE, esp_matter_uint8(value));
    esp_matter::attribute::add_bounds(attribute, esp_matter_uint8(1), esp_matter_uint8(100));
    return attribute;
}

attribute_t *create_target_setpoint(cluster_t *cluster, uint8_t value)
{
    attribute_t *attribute = esp_matter::attribute::create(cluster, TargetSetpoint::Id, ATTRIBUTE_FLAG_NONE, esp_matter_uint8(value));
    esp_matter::attribute::add_bounds(attribute, esp_matter_uint8(0), esp_matter_uint8(254));
    return attribute;
}

attribute_t *create_mist_type(cluster_t *cluster, uint8_t value)
{
    printf("create_mist_type attribute value: %d!\n", value);
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(humidifier), NULL);
    attribute_t *attribute = esp_matter::attribute::create(cluster, MistType::Id, ATTRIBUTE_FLAG_NONE, esp_matter_bitmap8(value));
    printf("create_mist_type attribute success!\n");
    esp_matter::attribute::add_bounds(attribute, esp_matter_bitmap8(0), esp_matter_bitmap8(3));
    return attribute;
}

attribute_t *create_continuous(cluster_t *cluster, bool value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(continuous), NULL);
    return esp_matter::attribute::create(cluster, Continuous::Id, ATTRIBUTE_FLAG_NONE, esp_matter_bool(value));
}

attribute_t *create_sleep(cluster_t *cluster, bool value)
{
    return esp_matter::attribute::create(cluster, Sleep::Id, ATTRIBUTE_FLAG_NONE, esp_matter_bool(value));
}

attribute_t *create_optimal(cluster_t *cluster, bool value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(optimal), NULL);
    return esp_matter::attribute::create(cluster, Optimal::Id, ATTRIBUTE_FLAG_NONE, esp_matter_bool(value));
}

attribute_t *create_cond_pump_enabled(cluster_t *cluster, bool value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(cond_pump), NULL);
    return esp_matter::attribute::create(cluster, CondPumpEnabled::Id, ATTRIBUTE_FLAG_NONE, esp_matter_bool(value));
}

attribute_t *create_cond_run_count(cluster_t *cluster, uint16_t value)
{
    uint32_t feature_map = get_feature_map_value(cluster);
    VerifyOrReturnValue(has_feature(cond_pump), NULL);
    attribute_t *attribute = esp_matter::attribute::create(cluster, CondRunCount::Id, ATTRIBUTE_FLAG_NONE, esp_matter_uint16(value));
    return attribute;
}

} /* attribute */
namespace command {
command_t *create_set_settings(cluster_t *cluster)
{
    return esp_matter::command::create(cluster, SetSettings::Id, COMMAND_FLAG_ACCEPTED, NULL);
}

} /* command */

static void create_default_binding_cluster(endpoint_t *endpoint)
{
    binding::config_t config;
    binding::create(endpoint, &config, CLUSTER_FLAG_SERVER);
}

const function_generic_t *function_list = NULL;

const int function_flags = CLUSTER_FLAG_NONE;

cluster_t *create(endpoint_t *endpoint, config_t *config, uint8_t flags)
{
    cluster_t *cluster = esp_matter::cluster::create(endpoint, humidistat::Id, flags);
    VerifyOrReturnValue(cluster, NULL, ESP_LOGE(TAG, "Could not create cluster. cluster_id: 0x%08" PRIX32, humidistat::Id));
    if (flags & CLUSTER_FLAG_SERVER) {
        VerifyOrReturnValue(config != NULL, ABORT_CLUSTER_CREATE(cluster));
        if (config->delegate != nullptr) {
            static const auto delegate_init_cb = HumidistatDelegateInitCB;
            set_delegate_and_init_callback(cluster, delegate_init_cb, config->delegate);
        }
        static const auto plugin_server_init_cb = CALL_ONCE(MatterHumidistatPluginServerInitCallback);
        set_plugin_server_init_callback(cluster, plugin_server_init_cb);
        add_function_list(cluster, function_list, function_flags);

        /* Attributes managed internally */
        global::attribute::create_feature_map(cluster, config->feature_flags);

        /* Attributes not managed internally */
        global::attribute::create_cluster_revision(cluster, cluster_revision);

        attribute::create_supported_modes(cluster, NULL, 0, 0);
        attribute::create_mode(cluster, config->mode);
        attribute::create_system_state(cluster, config->system_state);

        uint32_t feature_map = config->feature_flags;
        if (feature_map & feature::humidifier::get_id()) {
            VALIDATE_FEATURES_AT_LEAST_ONE("WarmMist,ColdMist",
                                           feature::warm_mist::get_id(), feature::cold_mist::get_id());
            if (feature_map & feature::warm_mist::get_id()) {
                VerifyOrReturnValue(feature::warm_mist::add(cluster) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
            }
            if (feature_map & feature::cold_mist::get_id()) {
                VerifyOrReturnValue(feature::cold_mist::add(cluster) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
            }
        }
        VALIDATE_FEATURES_AT_LEAST_ONE("Humidifier,Dehumidifier",
                                       feature::humidifier::get_id(), feature::dehumidifier::get_id());
        if (feature_map & feature::humidifier::get_id()) {
            VerifyOrReturnValue(feature::humidifier::add(cluster, &(config->features.humidifier)) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        if (feature_map & feature::dehumidifier::get_id()) {
            VerifyOrReturnValue(feature::dehumidifier::add(cluster) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        VALIDATE_FEATURES_AT_LEAST_ONE("Continuous,Sensor",
                                       feature::continuous::get_id(), feature::sensor::get_id());
        if (feature_map & feature::continuous::get_id()) {
            VerifyOrReturnValue(feature::continuous::add(cluster, &(config->features.continuous)) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        if (feature_map & feature::sensor::get_id()) {
            VerifyOrReturnValue(feature::sensor::add(cluster, &(config->features.sensor)) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        if (feature_map & feature::fan_auto::get_id()) {
            VerifyOrReturnValue(feature::fan_auto::add(cluster) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        if (feature_map & feature::fan_only::get_id()) {
            VerifyOrReturnValue(feature::fan_only::add(cluster) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        if (feature_map & feature::optimal::get_id()) {
            VerifyOrReturnValue(feature::optimal::add(cluster, &(config->features.optimal)) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        if (feature_map & feature::cond_pump::get_id()) {
            VerifyOrReturnValue(feature::cond_pump::add(cluster, &(config->features.cond_pump)) == ESP_OK, ABORT_CLUSTER_CREATE(cluster));
        }
        command::create_set_settings(cluster);

        cluster::set_init_and_shutdown_callbacks(cluster, ESPMatterHumidistatClusterServerInitCallback,
                                                 ESPMatterHumidistatClusterServerShutdownCallback);
    }

    if (flags & CLUSTER_FLAG_CLIENT) {
        create_default_binding_cluster(endpoint);
    }
    return cluster;
}

} /* humidistat */
} /* cluster */
} /* esp_matter */
