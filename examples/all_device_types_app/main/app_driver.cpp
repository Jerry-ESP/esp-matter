/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <esp_matter.h>

#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace chip::app::Clusters::FanControl;

static const char *TAG = "app_driver";
extern uint16_t app_endpoint_id;

static void get_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    attribute::get_val(attribute, val);
}

static esp_err_t set_attribute(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t val)
{
    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    return attribute::set_val(attribute, &val);
}

static bool check_if_mode_percent_match(uint8_t fan_mode, uint8_t percent)
{
    switch (fan_mode) {
        case chip::to_underlying(FanModeEnum::kHigh): {
            if (percent < HIGH_MODE_PERCENT_MIN) {
                return false;
            }
            break;
        }
        case chip::to_underlying(FanModeEnum::kMedium): {
            if ((percent < MED_MODE_PERCENT_MIN) || (percent > MED_MODE_PERCENT_MAX)) {
               return false;
            }
            break;
        }
        case chip::to_underlying(FanModeEnum::kLow): {
            if ((percent < LOW_MODE_PERCENT_MIN) || (percent > LOW_MODE_PERCENT_MAX)) {
                return false;
            }
            break;
        }
        default:
            return false;
    }

    return true;
}

static void app_driver_fan_percent_setting(esp_matter_attr_val_t val)
{
    /*this is just used to simulate fan driver*/
    set_attribute(app_endpoint_id, FanControl::Id, Attributes::PercentCurrent::Id, val);
    ESP_LOGI(TAG, "Call app_driver_fan_set_percent");
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == app_endpoint_id) {
        if (cluster_id == FanControl::Id) {
            if (attribute_id == FanControl::Attributes::FanMode::Id) {
                esp_matter_attr_val_t val_a = esp_matter_invalid(NULL);
                get_attribute(endpoint_id, FanControl::Id, Attributes::PercentSetting::Id, &val_a);
                /* When FanMode attribute change , should check the persent setting value, if this value not match
                   FanMode, need write the persent setting value to corresponding value. Now we set it to the max
                   value of the FanMode*/
                if (!check_if_mode_percent_match(val->val.u8, val_a.val.u8)) {
                    switch (val->val.u8) {
                        case chip::to_underlying(FanModeEnum::kHigh): {
                                val_a.val.u8 = HIGH_MODE_PERCENT_MAX;
                                attribute::update(endpoint_id, FanControl::Id, Attributes::PercentSetting::Id, &val_a);
                            break;
                        }
                        case chip::to_underlying(FanModeEnum::kMedium): {
                                val_a.val.u8 = MED_MODE_PERCENT_MAX;
                                attribute::update(endpoint_id, FanControl::Id, Attributes::PercentSetting::Id, &val_a);
                            break;
                        }
                        case chip::to_underlying(FanModeEnum::kLow): {
                                val_a.val.u8 = LOW_MODE_PERCENT_MAX;
                                attribute::update(endpoint_id, FanControl::Id, Attributes::PercentSetting::Id, &val_a);
                            break;
                        }
                        default:
                            break;
                    }
                }
            } else if (attribute_id == FanControl::Attributes::PercentSetting::Id) {
                /* When the Percent setting attribute change, should check the FanMode value, if not match, need write
                   the FanMode value to the corresponding FanMode.*/
                esp_matter_attr_val_t val_b = esp_matter_invalid(NULL);
                get_attribute(endpoint_id, FanControl::Id, Attributes::FanMode::Id, &val_b);
                if (!check_if_mode_percent_match(val_b.val.u8, val->val.u8)) {
                    if (val->val.u8 >= HIGH_MODE_PERCENT_MIN) {
                        /* set high mode */
                        val_b.val.u8 = chip::to_underlying(FanModeEnum::kHigh);
                        //set_attribute(endpoint_id, FanControl::Id, Attributes::FanMode::Id, val_b);
                        attribute::update(endpoint_id, FanControl::Id, Attributes::FanMode::Id, &val_b);
                    } else if (val->val.u8 >= MED_MODE_PERCENT_MIN) {
                        /* set med mode */
                        val_b.val.u8 = chip::to_underlying(FanModeEnum::kMedium);
                        //set_attribute(endpoint_id, FanControl::Id, Attributes::FanMode::Id, val_b);
                        attribute::update(endpoint_id, FanControl::Id, Attributes::FanMode::Id, &val_b);
                    } else if (val->val.u8 >= LOW_MODE_PERCENT_MIN) {
                        /* set low mode */
                        val_b.val.u8 = chip::to_underlying(FanModeEnum::kLow);
                        //set_attribute(endpoint_id, FanControl::Id, Attributes::FanMode::Id, val_b);
                        attribute::update(endpoint_id, FanControl::Id, Attributes::FanMode::Id, &val_b);
                    }
                }
                /* add percent setting driver */
                app_driver_fan_percent_setting(*val);
            }
        }
    }
    return err;
}
