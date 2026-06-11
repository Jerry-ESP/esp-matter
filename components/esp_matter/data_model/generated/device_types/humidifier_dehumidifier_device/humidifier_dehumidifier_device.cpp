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

/* This is a Generated File */

#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_core.h>
#include <humidifier_dehumidifier_device.h>

using namespace esp_matter;
using namespace esp_matter::cluster;
using namespace esp_matter::endpoint;

namespace esp_matter {
namespace endpoint {
namespace humidifier_dehumidifier {
uint32_t get_device_type_id()
{
    return ESP_MATTER_HUMIDIFIER_DEHUMIDIFIER_DEVICE_TYPE_ID;
}

uint8_t get_device_type_version()
{
    return ESP_MATTER_HUMIDIFIER_DEHUMIDIFIER_DEVICE_TYPE_VERSION;
}

endpoint_t *create(node_t *node, config_t *config, uint8_t flags, void *priv_data)
{
    endpoint_t *endpoint = esp_matter::endpoint::create(node, flags, priv_data);
    VerifyOrReturnValue(endpoint != nullptr, NULL, ESP_LOGE("humidifier_dehumidifier", "Failed to create endpoint"));

    cluster_t *descriptor_cluster = descriptor::create(endpoint, &(config->descriptor), CLUSTER_FLAG_SERVER);
    VerifyOrReturnValue(descriptor_cluster != nullptr, NULL,
                        ESP_LOGE("humidifier_dehumidifier", "Failed to create descriptor cluster"));

    VerifyOrReturnValue(add(endpoint, config) == ESP_OK, NULL,
                        ESP_LOGE("humidifier_dehumidifier", "Failed to add device type"));
    return endpoint;
}

esp_err_t add(endpoint_t *endpoint, config_t *config)
{
    esp_err_t err = add_device_type(endpoint, get_device_type_id(), get_device_type_version());
    VerifyOrReturnError(err == ESP_OK, err);

    cluster::humidistat::create(endpoint, &(config->humidistat), CLUSTER_FLAG_SERVER);
    return ESP_OK;
}

} /* humidifier_dehumidifier */
} /* endpoint */
} /* esp_matter */
