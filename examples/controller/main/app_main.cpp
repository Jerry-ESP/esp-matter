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
#include <esp_matter_core.h>

#include <esp_http_client.h>
#include <json_parser.h>


static const char *TAG = "app_main";
uint16_t switch_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

//Register
//Ready--polling repeat if 204
//if 200 got QR --> operate on device
//operate --> commission, OTA, custom cluster
//post success
static esp_err_t register_controller();
static int got_ip_connectivity=0;
static int got_end_device_assigned=0;
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::PublicEventTypes::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        got_ip_connectivity=1;
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

static esp_err_t register_controller()
{
    printf("send post request.\n");
    esp_err_t ret = ESP_OK;
    char url[256] = {0};

    int wlen=0;
    const char* post_data;
    char *http_payload = NULL;
    const size_t http_payload_size = 1024;

    const char* ip = "http://192.168.56.120";
    uint16_t port = 8081;
    const char* route = "/controller/register";
    // const char* rm_cn = "12005107-034b-4139-a3be-a2623fa7128e";
    snprintf(url, sizeof(url), "%s:%d/%s",ip, port,route);
    ESP_LOGE(TAG,"url: %s",url);

    esp_http_client_config_t config = {
    .url = url,
    .buffer_size = 1526,
    .buffer_size_tx = 2048,
    };

    int http_len, http_status_code;
    jparse_ctx_t jctx;

    char* mac = NULL;
    char* cn = NULL;
    char* cert = NULL;

    int str_length = 0;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if(client ==NULL)
    {
        ESP_LOGE(TAG,"Failed to initialise HTTP Client.");
        return ESP_FAIL;
    }

    ret = esp_http_client_set_header(client, "Content-Type", "application/json");
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to set HTTP header accept");
        goto cleanup;
    }

    // const char *
    post_data =
    // "{\"field1\":\"value1\"}";

    "{\"controller_id\": \"s3_ct_1\",\"mac_address\": \"0123456789AB\",\"ip_address\": \"192.168.1.100\", \"version\": \"1.0.0\"}";

    // HTTP POST
    ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to set HTTP method");
        goto cleanup;
    }
    ret = esp_http_client_open(client,strlen(post_data) );
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to open HTTP connection");
        goto cleanup;
    }

    http_payload = (char *)calloc(http_payload_size, sizeof(char));

    if(http_payload == NULL)
    {
        ESP_LOGE(TAG,"Failed to alloc memory for http_payload");
        ret = ESP_ERR_NO_MEM;
        goto close;
    }

    wlen = esp_http_client_write(client, post_data, strlen(post_data));
    if (wlen < 0) {
        ESP_LOGE(TAG, "Write failed");
    }
    http_len = esp_http_client_fetch_headers(client);
    http_status_code = esp_http_client_get_status_code(client);
    if ((http_len > 0) && (http_status_code == 201))
    {
        http_len = esp_http_client_read_response(client, http_payload,
                                                http_payload_size - 1);
        http_payload[http_len] = 0;


    }
    else
    {
        http_len = esp_http_client_read_response(client, http_payload,
                                                http_payload_size - 1);
        http_payload[http_len] = 0;
        ESP_LOGE(TAG, "Invalid response for %s", url);
        ESP_LOGE(TAG, "Status = %d, Data = %s", http_status_code,
                http_len > 0 ? http_payload : "None");
        ret = http_status_code == 401 ? ESP_ERR_INVALID_STATE : ESP_FAIL;
        goto close;
    }


    // Parse the response payload
    ESP_LOGI(TAG, "HTTP response:%s\n", http_payload);
    // if(json_parse_start(&jctx, http_payload, http_len) != 0)
    // {
    //     ESP_LOGE(TAG,"Failed to parse the HTTP response json on json_parse_start");
    //     ret = ESP_FAIL;
    //     goto close;
    // }
    // else
    //     printf("json parse start successfull.\n");


    // if (json_obj_get_strlen(&jctx, "mac", &str_length) == 0 && str_length !=0)
    // {
    //     mac = (char*) calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "mac", mac, str_length+1) == 0)
    //     {
    //         printf("MAC:\n%s\n",mac);
    //         ret = ESP_OK;
    //     }
    // }


    // if (json_obj_get_strlen(&jctx, "cn", &str_length) == 0 && str_length !=0)
    // {
    //     cn = (char*)calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "cn", cn, str_length+1) == 0)
    //     {
    //         if(cn)
    //         {   printf("CN:\n%s\n",cn);
    //             ret = ESP_OK;
    //         }
    //         else
    //             ret = ESP_FAIL;
    //     }
    // }

    // if (json_obj_get_strlen(&jctx, "cert", &str_length) == 0 && str_length !=0)
    // {
    //     cert = (char*) calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "cert", cert, str_length+1) == 0)
    //     {
    //         if(cert)
    //         {   printf("CERT:\n%s\n",cert);
    //             ret = ESP_OK;
    //         }
    //         else
    //             ret = ESP_FAIL;
    //     }
    // }

    // json_parse_end(&jctx);


    close:
        esp_http_client_close(client);
    cleanup:
        esp_http_client_cleanup(client);
        if (http_payload) {
            free(http_payload);
        }

  return ret;
}


static esp_err_t controller_ready()
{
    printf("send ready request.\n");
    esp_err_t ret = ESP_OK;
    char url[256] = {0};

    int wlen=0;
    const char* post_data;
    char *http_payload = NULL;
    const size_t http_payload_size = 1024;

    const char* ip = "http://192.168.56.120";
    uint16_t port = 8081;
    const char* route = "/controller/ready";
    // const char* rm_cn = "12005107-034b-4139-a3be-a2623fa7128e";
    snprintf(url, sizeof(url), "%s:%d/%s",ip, port,route);
    ESP_LOGE(TAG,"url: %s",url);

    esp_http_client_config_t config = {
    .url = url,
    .buffer_size = 1526,
    .buffer_size_tx = 2048,
    };

    int http_len, http_status_code;
    jparse_ctx_t jctx;

    char* mac = NULL;
    char* cn = NULL;
    char* cert = NULL;

    int str_length = 0;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if(client ==NULL)
    {
        ESP_LOGE(TAG,"Failed to initialise HTTP Client.");
        return ESP_FAIL;
    }

    ret = esp_http_client_set_header(client, "Content-Type", "application/json");
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to set HTTP header accept");
        goto cleanup;
    }

    // const char *
    post_data =
    // "{\"field1\":\"value1\"}";

    "{\"controller_id\": \"s3_ct_1\"}";
    // HTTP POST
    ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to set HTTP method");
        goto cleanup;
    }
    ret = esp_http_client_open(client,strlen(post_data) );
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to open HTTP connection");
        goto cleanup;
    }

    http_payload = (char *)calloc(http_payload_size, sizeof(char));

    if(http_payload == NULL)
    {
        ESP_LOGE(TAG,"Failed to alloc memory for http_payload");
        ret = ESP_ERR_NO_MEM;
        goto close;
    }

    wlen = esp_http_client_write(client, post_data, strlen(post_data));
    if (wlen < 0) {
        ESP_LOGE(TAG, "Write failed");
    }
    http_len = esp_http_client_fetch_headers(client);
    http_status_code = esp_http_client_get_status_code(client);
    if ((http_len > 0) && (http_status_code == 200))
    {
        http_len = esp_http_client_read_response(client, http_payload,
                                                http_payload_size - 1);
        http_payload[http_len] = 0;

        printf("Got an end device to operate on\n");

        got_end_device_assigned=1;

    }
    else if (http_status_code == 204)
    {
        printf("Wait for end device to be assigned\n");
    }

    else
    {
        http_len = esp_http_client_read_response(client, http_payload,
                                                http_payload_size - 1);
        http_payload[http_len] = 0;
        ESP_LOGE(TAG, "Invalid response for %s", url);
        ESP_LOGE(TAG, "Status = %d, Data = %s", http_status_code,
                http_len > 0 ? http_payload : "None");
        ret = http_status_code == 401 ? ESP_ERR_INVALID_STATE : ESP_FAIL;
        goto close;
    }


    // Parse the response payload
    ESP_LOGI(TAG, "HTTP response:%s\n", http_payload);
    // if(json_parse_start(&jctx, http_payload, http_len) != 0)
    // {
    //     ESP_LOGE(TAG,"Failed to parse the HTTP response json on json_parse_start");
    //     ret = ESP_FAIL;
    //     goto close;
    // }
    // else
    //     printf("json parse start successfull.\n");


    // if (json_obj_get_strlen(&jctx, "mac", &str_length) == 0 && str_length !=0)
    // {
    //     mac = (char*) calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "mac", mac, str_length+1) == 0)
    //     {
    //         printf("MAC:\n%s\n",mac);
    //         ret = ESP_OK;
    //     }
    // }


    // if (json_obj_get_strlen(&jctx, "cn", &str_length) == 0 && str_length !=0)
    // {
    //     cn = (char*)calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "cn", cn, str_length+1) == 0)
    //     {
    //         if(cn)
    //         {   printf("CN:\n%s\n",cn);
    //             ret = ESP_OK;
    //         }
    //         else
    //             ret = ESP_FAIL;
    //     }
    // }

    // if (json_obj_get_strlen(&jctx, "cert", &str_length) == 0 && str_length !=0)
    // {
    //     cert = (char*) calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "cert", cert, str_length+1) == 0)
    //     {
    //         if(cert)
    //         {   printf("CERT:\n%s\n",cert);
    //             ret = ESP_OK;
    //         }
    //         else
    //             ret = ESP_FAIL;
    //     }
    // }

    // json_parse_end(&jctx);


    close:
        esp_http_client_close(client);
    cleanup:
        esp_http_client_cleanup(client);
        if (http_payload) {
            free(http_payload);
        }

  return ret;
}

static esp_err_t controller_complete()
{
    printf("send complete status.\n");
    esp_err_t ret = ESP_OK;
    char url[256] = {0};

    int wlen=0;
    const char* post_data;
    char *http_payload = NULL;
    const size_t http_payload_size = 1024;

    const char* ip = "http://192.168.56.120";
    uint16_t port = 8081;
    const char* route = "/device/status";
    // const char* rm_cn = "12005107-034b-4139-a3be-a2623fa7128e";
    snprintf(url, sizeof(url), "%s:%d/%s",ip, port,route);
    ESP_LOGE(TAG,"url: %s",url);

    esp_http_client_config_t config = {
    .url = url,
    .buffer_size = 1526,
    .buffer_size_tx = 2048,
    };

    int http_len, http_status_code;
    jparse_ctx_t jctx;

    char* mac = NULL;
    char* cn = NULL;
    char* cert = NULL;

    int str_length = 0;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if(client ==NULL)
    {
        ESP_LOGE(TAG,"Failed to initialise HTTP Client.");
        return ESP_FAIL;
    }

    ret = esp_http_client_set_header(client, "Content-Type", "application/json");
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to set HTTP header accept");
        goto cleanup;
    }

    // const char *
    post_data =
    // "{\"field1\":\"value1\"}";

"{\"mac_address\": \"E86BEA46CDB8\",\"controller_id\": \"s3_ct_1\",\"status\": \"success\"}";
    // HTTP POST
    ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to set HTTP method");
        goto cleanup;
    }
    ret = esp_http_client_open(client,strlen(post_data) );
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG,"Failed to open HTTP connection");
        goto cleanup;
    }

    http_payload = (char *)calloc(http_payload_size, sizeof(char));

    if(http_payload == NULL)
    {
        ESP_LOGE(TAG,"Failed to alloc memory for http_payload");
        ret = ESP_ERR_NO_MEM;
        goto close;
    }

    wlen = esp_http_client_write(client, post_data, strlen(post_data));
    if (wlen < 0) {
        ESP_LOGE(TAG, "Write failed");
    }
    http_len = esp_http_client_fetch_headers(client);
    http_status_code = esp_http_client_get_status_code(client);
    if ((http_len > 0) && (http_status_code == 200))
    {
        http_len = esp_http_client_read_response(client, http_payload,
                                                http_payload_size - 1);
        http_payload[http_len] = 0;

        printf("Device operation completed marked on manager.\n");
        got_end_device_assigned=0;


    }
    else
    {
        http_len = esp_http_client_read_response(client, http_payload,
                                                http_payload_size - 1);
        http_payload[http_len] = 0;
        ESP_LOGE(TAG, "Invalid response for %s", url);
        ESP_LOGE(TAG, "Status = %d, Data = %s", http_status_code,
                http_len > 0 ? http_payload : "None");
        ret = http_status_code == 401 ? ESP_ERR_INVALID_STATE : ESP_FAIL;
        goto close;
    }


    // Parse the response payload
    ESP_LOGI(TAG, "HTTP response:%s\n", http_payload);
    // if(json_parse_start(&jctx, http_payload, http_len) != 0)
    // {
    //     ESP_LOGE(TAG,"Failed to parse the HTTP response json on json_parse_start");
    //     ret = ESP_FAIL;
    //     goto close;
    // }
    // else
    //     printf("json parse start successfull.\n");


    // if (json_obj_get_strlen(&jctx, "mac", &str_length) == 0 && str_length !=0)
    // {
    //     mac = (char*) calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "mac", mac, str_length+1) == 0)
    //     {
    //         printf("MAC:\n%s\n",mac);
    //         ret = ESP_OK;
    //     }
    // }


    // if (json_obj_get_strlen(&jctx, "cn", &str_length) == 0 && str_length !=0)
    // {
    //     cn = (char*)calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "cn", cn, str_length+1) == 0)
    //     {
    //         if(cn)
    //         {   printf("CN:\n%s\n",cn);
    //             ret = ESP_OK;
    //         }
    //         else
    //             ret = ESP_FAIL;
    //     }
    // }

    // if (json_obj_get_strlen(&jctx, "cert", &str_length) == 0 && str_length !=0)
    // {
    //     cert = (char*) calloc(str_length+1,sizeof(char));
    //     if(json_obj_get_string(&jctx, "cert", cert, str_length+1) == 0)
    //     {
    //         if(cert)
    //         {   printf("CERT:\n%s\n",cert);
    //             ret = ESP_OK;
    //         }
    //         else
    //             ret = ESP_FAIL;
    //     }
    // }

    // json_parse_end(&jctx);


    close:
        esp_http_client_close(client);
    cleanup:
        esp_http_client_cleanup(client);
        if (http_payload) {
            free(http_payload);
        }

  return ret;
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

    const char pai[] = "{\"0:STR\":\"-----BEGIN CERTIFICATE-----\nMIIBvDCCAWOgAwIBAgIIC6S3aD9evm4wCgYIKoZIzj0EAwIwNDEcMBoGA1UEAwwT\nRVNQIE1hdHRlciBQQUEgdGVzdDEUMBIGCisGAQQBgqJ8AgEMBDEzMUIwIBcNMjMw\nMzEwMDAwMDAwWhgPOTk5OTEyMzEyMzU5NTlaMCsxEzARBgNVBAMMCkVTUCBNYXR0\nZXIxFDASBgorBgEEAYKifAIBDAQxMzFCMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcD\nQgAEHQvGtYuLFltNTmaIaZu1VF4EmMX6ZOTzpyOd71iAARz8hkmo4zYf9AFqJoBj\n/i0thZmJ7ZQitfi7H5cc4+B1CaNmMGQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNV\nHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFBwfC8rTOwzd4FwtZYOIKdGaw95SMB8GA1Ud\nIwQYMBaAFBBXiQ7CHOd7WlZhCcoLOeraCCdxMAoGCCqGSM49BAMCA0cAMEQCIC+x\nNht5SJsdcnsCgnBOXYBqloa5zyQnRHp+3zjKGWsYAiAqipiFgrSd6348eB9vM+FQ\nojjYWhZ1AJuT2zZBXFP6Zg==\n-----END CERTIFICATE-----\"}";


// "-----BEGIN CERTIFICATE-----\nMIIBvDCCAWOgAwIBAgIIC6S3aD9evm4wCgYIKoZIzj0EAwIwNDEcMBoGA1UEAwwT\nRVNQIE1hdHRlciBQQUEgdGVzdDEUMBIGCisGAQQBgqJ8AgEMBDEzMUIwIBcNMjMw\nMzEwMDAwMDAwWhgPOTk5OTEyMzEyMzU5NTlaMCsxEzARBgNVBAMMCkVTUCBNYXR0\nZXIxFDASBgorBgEEAYKifAIBDAQxMzFCMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcD\nQgAEHQvGtYuLFltNTmaIaZu1VF4EmMX6ZOTzpyOd71iAARz8hkmo4zYf9AFqJoBj\n/i0thZmJ7ZQitfi7H5cc4+B1CaNmMGQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNV\nHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFBwfC8rTOwzd4FwtZYOIKdGaw95SMB8GA1Ud\nIwQYMBaAFBBXiQ7CHOd7WlZhCcoLOeraCCdxMAoGCCqGSM49BAMCA0cAMEQCIC+x\nNht5SJsdcnsCgnBOXYBqloa5zyQnRHp+3zjKGWsYAiAqipiFgrSd6348eB9vM+FQ\nojjYWhZ1AJuT2zZBXFP6Zg==\n-----END CERTIFICATE-----"

// while(1)
// {
//     vTaskDelay(5000/portTICK_PERIOD_MS);
//     lock::chip_stack_lock(portMAX_DELAY);

//     controller::send_invoke_cluster_command(0x1111,0x1 , 0x6, 0x2,NULL);

//     vTaskDelay(5000/portTICK_PERIOD_MS);

//     controller::send_invoke_cluster_command(0x1111,0x0 , 0x131BFC01, 0x1,pai);
//     lock::chip_stack_unlock();
// }
    while(!got_ip_connectivity)
    {
        vTaskDelay(2000/portTICK_PERIOD_MS);

    }
    printf("Registering to manager\n");
    register_controller();
    vTaskDelay(2000/portTICK_PERIOD_MS);

    while(1)
    {
        while(!got_end_device_assigned)
        {
            printf("Waiting for end device assignment.\n");
            controller_ready();
            vTaskDelay(10000/portTICK_PERIOD_MS);
        }

        vTaskDelay(20000/portTICK_PERIOD_MS);
        controller_complete();//todo: device processing else keep sending device completed
        vTaskDelay(15000/portTICK_PERIOD_MS);

    }






}
