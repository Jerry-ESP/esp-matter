/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota_provider.h>

#include <app_reset.h>

#include <app/clusters/ota-provider/ota-provider.h>
#include <app/server/Server.h>
#include <credentials/FabricTable.h>

static const char *TAG = "app_main";
uint16_t switch_endpoint_id = 0;

char esp_matter::ota_provider::dcl_rest_url[50];

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace esp_matter::ota_provider;
using namespace chip::app::Clusters;
using chip::app::Clusters::OTAProviderDelegate;

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::PublicEventTypes::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;
    default:
        break;
    }
}
static bool manager_ip_is_set = false;
static esp_err_t get_manager_ip()
{
    nvs_handle handle;
    esp_err_t err;
    if ((err = nvs_open("manager",NVS_READONLY, &handle)) != ESP_OK) {
        ESP_LOGI(TAG, "Manager NVS open failed with error %d",err);
        return err;
    }
    size_t required_size = 0;
    if ((err = nvs_get_blob(handle, "manager_ip", NULL, &required_size)) != ESP_OK) {
        ESP_LOGI(TAG, "Failed to read manager_ip with error %d size %d", err, required_size);
        nvs_close(handle);
        return err;
    }
    void *value = calloc(required_size + 1, 1); /* + 1 for NULL termination */
    if (value) {
        nvs_get_blob(handle, "manager_ip", value, &required_size);
    }
    nvs_close(handle);
    ESP_LOGE(TAG,"\nManager-ip: %s\n",(char*)value);
    snprintf(dcl_rest_url,sizeof(dcl_rest_url),"http://%s:8000/dcl/model/versions",(char*)value);
    printf("\nLocal DCL URL: %s\n",dcl_rest_url);
    free(value);
    manager_ip_is_set = true;
    return err ;
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();
    // If there is no commissioner in the controller, we need a default node so that the controller can be commissioned
    // to a specific fabric.
    node::config_t node_config;
    node_t *node = node::create(&node_config, NULL, NULL);
    endpoint_t *root_node_endpoint = endpoint::get(node, 0);
    cluster::ota_provider::config_t config;
    cluster_t *ota_provider_cluster = cluster::ota_provider::create(root_node_endpoint, &config, CLUSTER_FLAG_SERVER);
    if (!node || !root_node_endpoint || !ota_provider_cluster) {
        ESP_LOGE(TAG, "Failed to create data model");
        return;
    }
    /* Matter start */
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Matter start failed: %d", err);
    }
    EspOtaProvider::GetInstance().Init(true);
    OTAProvider::SetDelegate(0, reinterpret_cast<OTAProviderDelegate *>(&EspOtaProvider::GetInstance()));
#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::manager_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::init();
#endif // CONFIG_ENABLE_CHIP_SHELL

    get_manager_ip();
    if(manager_ip_is_set==false)
    {
        ESP_LOGE(TAG,"FATAL ERROR:");
        ESP_LOGE(TAG,"Please set LOCAL DCL IP using \"matter esp manager set-ip <ip> \"command");
        ESP_LOGE(TAG,"Reboot after setting the manager ip");
        return;
    }

}
