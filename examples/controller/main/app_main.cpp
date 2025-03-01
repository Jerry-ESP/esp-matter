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
#include <esp_matter_controller_client.h>
#include <esp_matter_controller_console.h>
#include <esp_matter_controller_utils.h>
#include <esp_matter_ota.h>
#if CONFIG_OPENTHREAD_BORDER_ROUTER
#include <esp_openthread_border_router.h>
#include <esp_openthread_lock.h>
#include <esp_ot_config.h>
#include <esp_spiffs.h>
#include <platform/ESP32/OpenthreadLauncher.h>
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER
#include <app_reset.h>
#include <common_macros.h>

#include <app/server/Server.h>
#include <credentials/FabricTable.h>

#include <esp_matter_controller_cluster_command.h>
#include <esp_matter_controller_pairing_command.h>
#include <esp_matter_controller_read_command.h>
#include <esp_matter_controller_subscribe_command.h>
#include <esp_matter_controller_write_command.h>
#include <esp_matter_core.h>

#include "esp_wifi.h"
#include <esp_http_client.h>
#include <json_parser.h>

static const char *TAG = "app_main";
uint16_t switch_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace esp_matter::console;

#define MANAGER_IP "http://192.168.50.167"
#define SSID "ESP_TEST_2G"
#define PASSPHRASE "ESP3448India"
#define MANAGER_PORT 8081
#define NEW_SOFTWARE_VERSION 10010001
#define NEW_SOFTWARE_VERSION_STRING "1.1.0-280"

#define MAC_ADDR_SIZE 6
#define CONTROLLER_REGISTERED "Controller registered"

#define http_payload_size 2048
char *http_payload = NULL;

// Register
// Ready--polling repeat if 204
// if 200 got QR --> operate on device
// operate --> commission, OTA, custom cluster
// post success
static char mac[18];

typedef struct {
    char dac[1024];
    char pai[1024];
    uint64_t node_id;
    char qr_code[32];
    char mac[18];
} end_device_data_t;

static end_device_data_t s_end_device_data;
enum controller_status {
    kBootUpDone = 0,
    kRegistrationPending,
    kRegistered,
    kReady,
    kEndDeviceAssigned,
    kOngoingCommission,
    kEndDeviceCommissioned,
    kACLWritePending,
    kACLWriteDone,
    kOTAPending,
    kOTAOngoing,
    kOTAComplete,
    kDACWritePending,
    kDACWrite,
    kDACWriteDone,
    kPAIWritePending,
    kPAIWrite,
    kPAIWriteDone,
    kOperationComplete,
};

static controller_status s_current_state = controller_status::kBootUpDone;

void commissioning_success_callback(ScopedNodeId peer_id)
{
    printf("Successful Commissioning for .\n");
    s_current_state = controller_status::kEndDeviceCommissioned;
}
// Callback for the failure of commissioning
void commissioning_failure_callback(ScopedNodeId peer_id, CHIP_ERROR error, chip::Controller::CommissioningStage stage,
                                    std::optional<chip::Credentials::AttestationVerificationResult> addtional_err_info)
{
    printf("Commissioning failure\n");
}

static controller::pairing_command_callbacks_t s_pairing_callback{nullptr, commissioning_success_callback,
                                                                  commissioning_failure_callback};

static void get_mac_address(char *mac_addr)
{
    uint8_t mac[MAC_ADDR_SIZE];
    char mac_str[18]; // MAC address string format XX:XX:XX:XX:XX:XX (17 chars + null terminator)

    if (esp_wifi_get_mac(WIFI_IF_STA, mac) == ESP_OK) {
        snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4],
                 mac[5]);
        ESP_LOGI(TAG, "MAC Address: %s", mac_str);
        strcpy(mac_addr, mac_str);
    } else {
        ESP_LOGE(TAG, "Failed to get MAC address");
    }
}

void print_ip_address(char *ip_addr)
{
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"); // Change as needed

    if (netif == NULL) {
        ESP_LOGE(TAG, "Failed to get netif handle");
        return;
    }

    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_str[16]; // Buffer for storing the IP string
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));
        ESP_LOGI(TAG, "IP Address: %s", ip_str);
        strcpy(ip_addr, ip_str);

    } else {
        ESP_LOGE(TAG, "Failed to get IP address");
    }
}

static esp_err_t register_controller();
static esp_err_t controller_complete();
static esp_err_t controller_ready();


static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::PublicEventTypes::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        if (s_current_state == controller_status::kBootUpDone)
            s_current_state = controller_status::kRegistrationPending;
        break;
    case chip::DeviceLayer::DeviceEventType::kESPSystemEvent:
        if (event->Platform.ESPSystemEvent.Base == IP_EVENT &&
            event->Platform.ESPSystemEvent.Id == IP_EVENT_STA_GOT_IP) {
#if CONFIG_OPENTHREAD_BORDER_ROUTER
            static bool sThreadBRInitialized = false;
            if (!sThreadBRInitialized) {
                esp_openthread_set_backbone_netif(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
                esp_openthread_lock_acquire(portMAX_DELAY);
                esp_openthread_border_router_init();
                esp_openthread_lock_release();
                sThreadBRInitialized = true;
            }
#endif
        }
        break;
    default:
        break;
    }
}
void attr_read_cb(uint64_t remote_node_id, const chip::app::ConcreteDataAttributePath &path, chip::TLV::TLVReader *data)
{
    // Todo:If OTA Status >90 state change to OTA Done kOTAComplete
    // If software version is NEW_SOFTWARE_VERSION - kDACWritePending
    ESP_LOGI(TAG, "Read attribute callback");
    if (path.mClusterId == 0x2A && path.mAttributeId == 0x3) {
        uint8_t value;
        chip::app::DataModel::Decode(*data, value);
        printf("OTA Progress : %d\n", value);
        if (value >= 96) {
            s_current_state = controller_status::kOTAComplete;
        }
    }

    // else if (path.mClusterId == 0x28 && path.mAttributeId == 0x9) {
    //     uint32_t value;
    //     chip::app::DataModel::Decode(*data, value);
    //     printf("New Software Version : %u\n", value);
    //     if (value == NEW_SOFTWARE_VERSION) {
    //         s_current_state = controller_status::kDACWritePending;
    //     }
    // }

    else if (path.mClusterId == 0x28 && path.mAttributeId == 0xA) {
        chip::CharSpan value;
        chip::app::DataModel::Decode(*data, value);
        printf("Software Version String : %.*s\n", value.size(),value.data());
        if (strncmp(value.data(),NEW_SOFTWARE_VERSION_STRING,value.size())==0)
        {
            printf("New Software version found\n");
            s_current_state = controller_status::kDACWritePending;
        }
    }

    else if (path.mClusterId == 0x131BFC05 && path.mAttributeId == 0x0) {
        int8_t value;
        chip::app::DataModel::Decode(*data, value);
        printf("End Device Status Value : %d\n", value);
        if ((s_current_state == controller_status::kDACWritePending) && (value == 1)) {
            s_current_state = controller_status::kDACWrite;
        }
        else if ((s_current_state==controller_status::kDACWriteDone) && (value == 2))
        {
            s_current_state = controller_status::kPAIWrite;
        }
        else if ((s_current_state==controller_status::kPAIWriteDone) && (value == 3))
        {
            s_current_state = controller_status::kOperationComplete;
        }
    }

    return;
}

static void attribute_data_cb(uint64_t remote_node_id, const chip::app::ConcreteDataAttributePath &path,
                              chip::TLV::TLVReader *data)
{
    return;
}

esp_err_t pairing_api(uint64_t node_id, const char *ssid, const char *pass, const char *payload)
{
    lock::chip_stack_lock(portMAX_DELAY);
    controller::pairing_command::get_instance().set_callbacks(s_pairing_callback);
    controller::pairing_code_wifi(node_id, ssid, pass, payload);
    lock::chip_stack_unlock();

    return ESP_OK;
}

esp_err_t invoke_cmd_api(uint64_t destination_id, uint16_t endpoint_id, uint32_t cluster_id, uint32_t command_id,
                         const char *command_data_field)
{
    lock::chip_stack_lock(portMAX_DELAY);
    controller::send_invoke_cluster_command(destination_id, endpoint_id, cluster_id, command_id, command_data_field);
    lock::chip_stack_unlock();

    return ESP_OK;
}

esp_err_t read_attr_api(uint64_t node_id, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id)
{
    ScopedMemoryBufferWithSize<uint16_t> endpoint_ids;
    ScopedMemoryBufferWithSize<uint32_t> cluster_ids;
    ScopedMemoryBufferWithSize<uint32_t> attribute_ids;
    endpoint_ids.Alloc(1);
    cluster_ids.Alloc(1);
    attribute_ids.Alloc(1);
    endpoint_ids[0] = endpoint_id;
    cluster_ids[0] = cluster_id;
    attribute_ids[0] = attribute_id;
    lock::chip_stack_lock(portMAX_DELAY);
    controller::send_read_attr_command(node_id, endpoint_ids, cluster_ids, attribute_ids, attr_read_cb);
    lock::chip_stack_unlock();

    return ESP_OK;
}

esp_err_t write_attr_api(uint64_t node_id, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                         const char *attr_val_json_str)
{
    ScopedMemoryBufferWithSize<uint16_t> endpoint_ids;
    ScopedMemoryBufferWithSize<uint32_t> cluster_ids;
    ScopedMemoryBufferWithSize<uint32_t> attribute_ids;
    endpoint_ids.Alloc(1);
    cluster_ids.Alloc(1);
    attribute_ids.Alloc(1);
    endpoint_ids[0] = endpoint_id;
    cluster_ids[0] = cluster_id;
    attribute_ids[0] = attribute_id;
    lock::chip_stack_lock(portMAX_DELAY);
    controller::send_write_attr_command(node_id, endpoint_id, cluster_id, attribute_id, attr_val_json_str);
    lock::chip_stack_unlock();

    return ESP_OK;
}

esp_err_t subscribe_attr_api(uint64_t node_id, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id)
{
    ScopedMemoryBufferWithSize<uint16_t> endpoint_ids;
    ScopedMemoryBufferWithSize<uint32_t> cluster_ids;
    ScopedMemoryBufferWithSize<uint32_t> attribute_ids;
    endpoint_ids.Alloc(1);
    cluster_ids.Alloc(1);
    attribute_ids.Alloc(1);
    endpoint_ids[0] = endpoint_id;
    cluster_ids[0] = cluster_id;
    attribute_ids[0] = attribute_id;
    uint16_t min_interval = 0;
    uint16_t max_interval = 10;
    lock::chip_stack_lock(portMAX_DELAY);
    controller::send_subscribe_attr_command(node_id, endpoint_ids, cluster_ids, attribute_ids, min_interval,
                                            max_interval, attribute_data_cb, true);
    lock::chip_stack_unlock();

    return ESP_OK;
}

static void custom_ota_event_handler()
{
    // ESP_LOGI(TAG, "Custom Event Received: id=%ld", event_id);

    switch (s_current_state) {
    case controller_status::kRegistrationPending: {
        printf("Registering to manager\n");
        register_controller();
    } break;
    case kRegistered: {
        ESP_LOGW(TAG, "Event: Registered");
        s_current_state = controller_status::kReady;
    } break;
    case kReady: {
        vTaskDelay(5000 / portMAX_DELAY);
        memset((void *)&s_end_device_data, 0, sizeof(s_end_device_data));
        ESP_LOGI(TAG, "Event: Ready");
        controller_ready();
    } break;
    case kEndDeviceAssigned: {
        ESP_LOGI(TAG, "Event: End Device Assigned");
        pairing_api(s_end_device_data.node_id, SSID, PASSPHRASE, s_end_device_data.qr_code);
        s_current_state = controller_status::kOngoingCommission;
        // Block
    } break;
    case kOngoingCommission:
        ESP_LOGI(TAG, "Event: Ongoing Commissioning");
        break;
    case kEndDeviceCommissioned: {
        ESP_LOGI(TAG, "Event: End Device Commissioned");
        s_current_state = controller_status::kOTAPending;
        vTaskDelay(5000 / portMAX_DELAY);
    } break;

    // case kACLWritePending:
    //     ESP_LOGI(TAG, "Event: ACL Write Pending");
    //     break;
    // case kACLWriteDone:
    //     ESP_LOGI(TAG, "Event: ACL Write Done");
    //     break;
    case kOTAPending: {
        ESP_LOGI(TAG, "Event: OTA Pending");
        char cmd_data[] = "{\"0:U64\": 56026, \"1:U64\": 65521, \"2:U8\": 0, \"4:U16\": 0}";
        invoke_cmd_api(s_end_device_data.node_id, 0x0, 0x2A, 0x0, cmd_data);
        s_current_state = controller_status::kOTAOngoing;
    } break;

    case kOTAOngoing: {
        ESP_LOGI(TAG, "Event: OTA Ongoing");
        vTaskDelay(15000 / portMAX_DELAY);
        read_attr_api(s_end_device_data.node_id, 0x0, 0x2A, 0x3);

    } break;
    case kOTAComplete: {
        ESP_LOGI(TAG, "Event: OTA Complete");
        vTaskDelay(7500 / portMAX_DELAY);
        read_attr_api(s_end_device_data.node_id, 0x0, 0x28, 0xA);

    } break;
    case kDACWritePending: {
        ESP_LOGI(TAG, "Event: DAC Write Pending");
        read_attr_api(s_end_device_data.node_id, 0x0, 0x131BFC05, 0x0);
    } break;
    case kDACWrite: {
        ESP_LOGI(TAG, "Event: DAC Write");
        printf("Sending DAC command data as : \n%s\n", s_end_device_data.dac);
        invoke_cmd_api(s_end_device_data.node_id, 0x0, 0x131BFC05, 0x0, s_end_device_data.dac);
        s_current_state = controller_status::kDACWriteDone;
    } break;
    case kDACWriteDone: {
        ESP_LOGI(TAG, "Event: DAC Write Done");
        vTaskDelay(3000 / portMAX_DELAY);
        read_attr_api(s_end_device_data.node_id, 0x0, 0x131BFC05, 0x0);
    } break;
    // case kPAIWritePending: {
    //     ESP_LOGI(TAG, "Event: PAI Write Pending");
    //     uint64_t node_id_64 = string_to_int64(s_end_device_data.node_id);
    //     read_attr_api(node_id_64, 0x0, 0x131BFC05, 0x0);
    // } break;
    case kPAIWrite: {
        ESP_LOGI(TAG, "Event: PAI Write");
        printf("Sending PAI command data as : \n%s\n", s_end_device_data.pai);
        invoke_cmd_api(s_end_device_data.node_id, 0x0, 0x131BFC05, 0x1, s_end_device_data.pai);
        vTaskDelay(3000 / portMAX_DELAY);
        s_current_state = controller_status::kPAIWriteDone;
    } break;
    case kPAIWriteDone: {
        ESP_LOGI(TAG, "Event: PAI Write Done");
        read_attr_api(s_end_device_data.node_id, 0x0, 0x131BFC05, 0x0);

    } break;
    case kOperationComplete: {
        ESP_LOGI(TAG, "Event: Operation Complete");
        invoke_cmd_api(s_end_device_data.node_id, 0x0, 0x131BFC05, 0x2, NULL);
        vTaskDelay(3000 / portMAX_DELAY);
        controller_complete();
        vTaskDelay(3000 / portMAX_DELAY);
        esp_restart();
    } break;
    default:
        ESP_LOGW(TAG, "Unknown event received : %d",s_current_state);
        break;
    }
}

void controller_task(void *pvParameter)
{
    while (1) {
        custom_ota_event_handler();
        printf("Controller Status Checking....\n");
        vTaskDelay(pdMS_TO_TICKS(2500)); // Delay for 1000 ms (1 second)
    }
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();
#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::init();
#if CONFIG_ESP_MATTER_CONTROLLER_ENABLE
    esp_matter::console::controller_register_commands();
#endif // CONFIG_ESP_MATTER_CONTROLLER_ENABLE
#ifdef CONFIG_OPENTHREAD_BORDER_ROUTER
    esp_matter::console::otcli_register_commands();
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER
#endif // CONFIG_ENABLE_CHIP_SHELL
#ifdef CONFIG_OPENTHREAD_BORDER_ROUTER
#ifdef CONFIG_AUTO_UPDATE_RCP
    esp_vfs_spiffs_conf_t rcp_fw_conf = {
        .base_path = "/rcp_fw", .partition_label = "rcp_fw", .max_files = 10, .format_if_mount_failed = false};
    if (ESP_OK != esp_vfs_spiffs_register(&rcp_fw_conf)) {
        ESP_LOGE(TAG, "Failed to mount rcp firmware storage");
        return;
    }
    esp_rcp_update_config_t rcp_update_config = ESP_OPENTHREAD_RCP_UPDATE_CONFIG();
    openthread_init_br_rcp(&rcp_update_config);
#endif
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER
    /* Matter start */
    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

#if CONFIG_ESP_MATTER_COMMISSIONER_ENABLE
    esp_matter::lock::chip_stack_lock(portMAX_DELAY);
    esp_matter::controller::matter_controller_client::get_instance().init(112233, 1, 5580);
    esp_matter::controller::matter_controller_client::get_instance().setup_commissioner();
    esp_matter::lock::chip_stack_unlock();
#endif // CONFIG_ESP_MATTER_COMMISSIONER_ENABLE

    http_payload = (char *)calloc(http_payload_size, sizeof(char));

    if(http_payload == NULL)
    {
        ESP_LOGE(TAG,"HTTP payload allocation failed");
        return;
    }

    xTaskCreate(controller_task, // Task function
                "Controllertask", // Name of the task
                6144, // Stack size (in words, not bytes)
                NULL, // Task parameters
                5, // Task priority
                NULL); // Task handle (optional)

    ESP_LOGI(TAG, "Controller Status Check Task Created!");
}

static esp_err_t register_controller()
{
    printf("send post request.\n");
    esp_err_t ret = ESP_OK;
    char url[256] = {0};
    char post_data[256] = {0};
    char ip_addr[16];

    int wlen = 0;
    // char *http_payload = NULL;
    // const size_t http_payload_size = 1024;

    const char *ip = MANAGER_IP;
    uint16_t port = MANAGER_PORT;
    const char *route = "/controller/register";

    snprintf(url, sizeof(url), "%s:%d/%s", ip, port, route);
    ESP_LOGE(TAG, "url: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .buffer_size = 1526,
        .buffer_size_tx = 2048,
    };

    int http_len, http_status_code;
    jparse_ctx_t jctx;

    int str_length = 0;

    get_mac_address(mac);
    print_ip_address(ip_addr);

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP Client.");
        return ESP_FAIL;
    }

    ret = esp_http_client_set_header(client, "Content-Type", "application/json");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP header accept");
        goto cleanup;
    }

    snprintf(post_data, sizeof(post_data),
             "{\"controller_id\": \"%s\",\"mac_address\": \"%s\",\"ip_address\": \"%s\", \"version\": \"1.0.0\"}", mac,
             mac, ip_addr);

    // HTTP POST
    ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP method");
        goto cleanup;
    }
    ret = esp_http_client_open(client, strlen(post_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        goto cleanup;
    }

    // http_payload = (char *)calloc(http_payload_size, sizeof(char));

    if (http_payload == NULL) {
        ESP_LOGE(TAG, "Failed to alloc memory for http_payload");
        ret = ESP_ERR_NO_MEM;
        goto close;
    }

    wlen = esp_http_client_write(client, post_data, strlen(post_data));
    if (wlen < 0) {
        ESP_LOGE(TAG, "Write failed");
    }
    http_len = esp_http_client_fetch_headers(client);
    http_status_code = esp_http_client_get_status_code(client);
    if ((http_len > 0) && (http_status_code == 201)) {
        http_len = esp_http_client_read_response(client, http_payload, http_payload_size - 1);
        http_payload[http_len] = 0;

    } else {
        http_len = esp_http_client_read_response(client, http_payload, http_payload_size - 1);
        http_payload[http_len] = 0;
        ESP_LOGE(TAG, "Invalid response for %s", url);
        ESP_LOGE(TAG, "Status = %d, Data = %s", http_status_code, http_len > 0 ? http_payload : "None");
        ret = http_status_code == 401 ? ESP_ERR_INVALID_STATE : ESP_FAIL;
        goto close;
    }

    // Parse the response payload
    ESP_LOGI(TAG, "HTTP response:\n%s\n", http_payload);

    // {
    //     "message": "Controller registered",
    //     "status": "success"
    // }

    if (json_parse_start(&jctx, http_payload, http_len) != 0) {
        ESP_LOGE(TAG, "Failed to parse the HTTP response json on json_parse_start");
        ret = ESP_FAIL;
        goto close;
    } else
        printf("json parse start successfull.\n");

    if (json_obj_get_strlen(&jctx, "message", &str_length) == 0 && str_length != 0) {
        char *message = (char *)calloc(str_length + 1, sizeof(char));
        if (json_obj_get_string(&jctx, "message", message, str_length + 1) == 0) {
            printf("message:\n%s\n", message);
            if (strcmp(message, CONTROLLER_REGISTERED) == 0) {
                ESP_LOGW(TAG, "Controller Successfully Registered");
                s_current_state = controller_status::kRegistered;

            }
            ret = ESP_OK;
        }
    }

    json_parse_end(&jctx);

close:
    esp_http_client_close(client);
cleanup:
    esp_http_client_cleanup(client);
    // if (http_payload) {
    //     free(http_payload);
    // }

    return ret;
}

static esp_err_t controller_ready()
{
    printf("send ready request.\n");
    esp_err_t ret = ESP_OK;
    char url[256] = {0};

    int wlen = 0;
    char post_data[256] = {0};
    // char *http_payload = NULL;
    // const size_t http_payload_size = 2048;

    const char *ip = MANAGER_IP;
    uint16_t port = MANAGER_PORT;
    const char *route = "/controller/ready";
    // const char* rm_cn = "12005107-034b-4139-a3be-a2623fa7128e";
    snprintf(url, sizeof(url), "%s:%d/%s", ip, port, route);
    ESP_LOGE(TAG, "url: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .buffer_size = 1526,
        .buffer_size_tx = 2048,
    };

    int http_len, http_status_code;
    jparse_ctx_t jctx;

    int str_length = 0;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP Client.");
        return ESP_FAIL;
    }

    ret = esp_http_client_set_header(client, "Content-Type", "application/json");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP header accept");
        goto cleanup;
    }

    snprintf(post_data, sizeof(post_data), "{\"controller_id\": \"%s\"}", mac);

    // HTTP POST
    ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP method");
        goto cleanup;
    }
    ret = esp_http_client_open(client, strlen(post_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        goto cleanup;
    }

    // http_payload = (char *)calloc(http_payload_size, sizeof(char));

    if (http_payload == NULL) {
        ESP_LOGE(TAG, "Failed to alloc memory for http_payload");
        ret = ESP_ERR_NO_MEM;
        goto close;
    }

    wlen = esp_http_client_write(client, post_data, strlen(post_data));
    if (wlen < 0) {
        ESP_LOGE(TAG, "Write failed");
    }
    http_len = esp_http_client_fetch_headers(client);
    http_status_code = esp_http_client_get_status_code(client);
    if ((http_len > 0) && (http_status_code == 200)) {
        http_len = esp_http_client_read_response(client, http_payload, http_payload_size - 1);
        http_payload[http_len] = 0;

        printf("Got an end device to operate on\n");


    } else if (http_status_code == 204) {
        printf("Wait for end device to be assigned\n");
        goto close;
    }

    else {
        http_len = esp_http_client_read_response(client, http_payload, http_payload_size - 1);
        http_payload[http_len] = 0;
        ESP_LOGE(TAG, "Invalid response for %s", url);
        ESP_LOGE(TAG, "Status = %d, Data = %s", http_status_code, http_len > 0 ? http_payload : "None");
        ret = http_status_code == 401 ? ESP_ERR_INVALID_STATE : ESP_FAIL;
        goto close;
    }

    // Parse the response payload
    ESP_LOGI(TAG, "HTTP response:%s\n", http_payload);
    // {
    //     "dac" : "dac_abc",
    //     "mac_address" : "E86BEA46CDB8",
    //     "pai" : "pai_abc",
    //     "qr_code_info" : "MT:YFZ010QV17-QO209K00",
    //     "matter_node_id" : "b3460846abb8963f44467a31d95f10b9"
    // }

    if (json_parse_start(&jctx, http_payload, http_len) != 0) {
        ESP_LOGE(TAG, "Failed to parse the HTTP response json on json_parse_start");
        ret = ESP_FAIL;
        goto close;
    } else
        printf("json parse start successfull.\n");

    if (json_obj_get_strlen(&jctx, "dac", &str_length) == 0 && str_length != 0) {
        char *DAC = (char *)calloc(str_length + 1, sizeof(char));
        if (json_obj_get_string(&jctx, "dac", DAC, str_length + 1) == 0) {
            printf("DAC:\n%s\n", DAC);
            // strcpy(s_end_device_data.dac, DAC); // Todo:Use strncpy 2. Remove calloc and put variable address
            snprintf(s_end_device_data.dac, sizeof(s_end_device_data.dac), "{\"0:STR\": \"%s\"}", DAC);
            ret = ESP_OK;
        }
        free(DAC);
    }

    if (json_obj_get_strlen(&jctx, "pai", &str_length) == 0 && str_length != 0) {
        char *PAI = (char *)calloc(str_length + 1, sizeof(char));
        if (json_obj_get_string(&jctx, "pai", PAI, str_length + 1) == 0) {
            if (PAI) {
                printf("PAI:\n%s\n", PAI);
                // strcpy(s_end_device_data.pai, PAI); // Todo:Use strncpy
                snprintf(s_end_device_data.dac, sizeof(s_end_device_data.pai), "{\"0:STR\": \"%s\"}", PAI);
                ret = ESP_OK;
            } else
                ret = ESP_FAIL;
        }
        free(PAI);
    }

    if (json_obj_get_strlen(&jctx, "mac", &str_length) == 0 && str_length != 0) {
        char *mac_id = (char *)calloc(str_length + 1, sizeof(char));
        if (json_obj_get_string(&jctx, "mac", mac_id, str_length + 1) == 0) {
            if (mac_id) {
                printf("MAC:\n%s\n", mac_id);
                strcpy(s_end_device_data.mac, mac_id); // Todo:Use strncpy
                ret = ESP_OK;
            } else
                ret = ESP_FAIL;
        }
        free(mac_id);
    }

    if (json_obj_get_strlen(&jctx, "qr_code_info", &str_length) == 0 && str_length != 0) {
        char *QR_code = (char *)calloc(str_length + 1, sizeof(char));
        if (json_obj_get_string(&jctx, "qr_code_info", QR_code, str_length + 1) == 0) {
            if (QR_code) {
                printf("QR Code:\n%s\n", QR_code);
                strcpy(s_end_device_data.qr_code, QR_code); // Todo:Use strncpy
                ret = ESP_OK;
            } else
                ret = ESP_FAIL;
        }
        free(QR_code);
    }

    if (json_obj_get_strlen(&jctx, "matter_node_id", &str_length) == 0 && str_length != 0) {
        char *Node_id = (char *)calloc(str_length + 1, sizeof(char));
        if (json_obj_get_string(&jctx, "matter_node_id", Node_id, str_length + 1) == 0) {
            if (Node_id) {
                printf("Node id:\n%s\n", Node_id);
                s_end_device_data.node_id = string_to_int64(Node_id);
                // strcpy(s_end_device_data.node_id, Node_id); // Todo:Use strncpy
                ret = ESP_OK;
            } else
                ret = ESP_FAIL;
        }
        free(Node_id);
    }

    json_parse_end(&jctx);

    s_current_state = controller_status::kEndDeviceAssigned;

close:
    esp_http_client_close(client);
cleanup:
    esp_http_client_cleanup(client);
    // if (http_payload) {
    //     free(http_payload);
    // }

    return ret;
}

static esp_err_t controller_complete()
{
    printf("send complete status.\n");
    esp_err_t ret = ESP_OK;
    char url[256] = {0};
    char post_data[256] = {0};
    int wlen = 0;

    // char *http_payload = NULL;
    // const size_t http_payload_size = 1024;

    const char *ip = MANAGER_IP;
    uint16_t port = MANAGER_PORT;
    const char *route = "/device/status";
    // const char* rm_cn = "12005107-034b-4139-a3be-a2623fa7128e";
    snprintf(url, sizeof(url), "%s:%d/%s", ip, port, route);
    ESP_LOGE(TAG, "url: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .buffer_size = 1526,
        .buffer_size_tx = 2048,
    };

    int http_len, http_status_code;
    jparse_ctx_t jctx;

    int str_length = 0;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP Client.");
        return ESP_FAIL;
    }

    ret = esp_http_client_set_header(client, "Content-Type", "application/json");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP header accept");
        goto cleanup;
    }

    snprintf(post_data, sizeof(post_data),
             "{\"mac_address\": \"%s\",\"controller_id\": \"%s\",\"status\": \"success\"}", s_end_device_data.mac, mac);
    // HTTP POST
    ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP method");
        goto cleanup;
    }
    ret = esp_http_client_open(client, strlen(post_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        goto cleanup;
    }

    // http_payload = (char *)calloc(http_payload_size, sizeof(char));

    if (http_payload == NULL) {
        ESP_LOGE(TAG, "Failed to alloc memory for http_payload");
        ret = ESP_ERR_NO_MEM;
        goto close;
    }

    wlen = esp_http_client_write(client, post_data, strlen(post_data));
    if (wlen < 0) {
        ESP_LOGE(TAG, "Write failed");
    }
    http_len = esp_http_client_fetch_headers(client);
    http_status_code = esp_http_client_get_status_code(client);
    if ((http_len > 0) && (http_status_code == 200)) {
        http_len = esp_http_client_read_response(client, http_payload, http_payload_size - 1);
        http_payload[http_len] = 0;

        printf("Device operation completed marked on manager.\n");

    } else {
        http_len = esp_http_client_read_response(client, http_payload, http_payload_size - 1);
        http_payload[http_len] = 0;
        ESP_LOGE(TAG, "Invalid response for %s", url);
        ESP_LOGE(TAG, "Status = %d, Data = %s", http_status_code, http_len > 0 ? http_payload : "None");
        ret = http_status_code == 401 ? ESP_ERR_INVALID_STATE : ESP_FAIL;
        goto close;
    }

    // Parse the response payload
    ESP_LOGI(TAG, "HTTP response:%s\n", http_payload);

close:
    esp_http_client_close(client);
cleanup:
    esp_http_client_cleanup(client);
    // if (http_payload) {
    //     free(http_payload);
    // }

    return ret;
}


// matter esp controller pairing code-wifi

// matter esp controller invoke-cmd 0x2 0x0 0x131BFC05 0x0 "{\"0:STR\": \"-----BEGIN CERTIFICATE-----\nMIIBvDCCAWOgAwIBAgIIC6S3aD9evm4wCgYIKoZIzj0EAwIwNDEcMBoGA1UEAwwT\nRVNQIE1hdHRlciBQQUEgdGVzdDEUMBIGCisGAQQBgqJ8AgEMBDEzMUIwIBcNMjMw\nMzEwMDAwMDAwWhgPOTk5OTEyMzEyMzU5NTlaMCsxEzARBgNVBAMMCkVTUCBNYXR0\nZXIxFDASBgorBgEEAYKifAIBDAQxMzFCMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcD\nQgAEHQvGtYuLFltNTmaIaZu1VF4EmMX6ZOTzpyOd71iAARz8hkmo4zYf9AFqJoBj\n/i0thZmJ7ZQitfi7H5cc4+B1CaNmMGQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNV\nHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFBwfC8rTOwzd4FwtZYOIKdGaw95SMB8GA1Ud\nIwQYMBaAFBBXiQ7CHOd7WlZhCcoLOeraCCdxMAoGCCqGSM49BAMCA0cAMEQCIC+x\nNht5SJsdcnsCgnBOXYBqloa5zyQnRHp+3zjKGWsYAiAqipiFgrSd6348eB9vM+FQ\nojjYWhZ1AJuT2zZBXFP6Zg==\n-----END CERTIFICATE-----\"}"

// matter esp controller read-attr 0x2 0x0 0x28 0x9

// matter esp controller read-attr 0x4 0x0 0x131BFC05 0x0

// matter esp controller read-attr 0x4 0x0 0x28 0x9

// matter esp controller read-attr 0x4 0x0 0x29 0xFFFA

// matter esp controller pairing open-commissioning-window 0x4 0x0 0x29 0xFFF

// matter esp controller invoke-cmd 0x4 0x0 0x131BFC05 0x0 "{\"0:STR\": \"test-dac\"}"
