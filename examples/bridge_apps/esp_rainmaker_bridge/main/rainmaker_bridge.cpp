/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <app_bridged_device.h>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_bridge.h>
#include <rainmaker_bridge.h>
#include <json_parser.h>
#include <json_generator.h>

static const char *TAG = "rainmaker_bridge";

using namespace chip::app::Clusters;
using namespace esp_matter;
using namespace esp_matter::cluster;

extern uint16_t aggregator_endpoint_id;

static char new_node_list[20][24] = {0};
static int new_node_count = 0;

// char nodes_buffer[1024] = "{"node":["1234","2345","4567"]}";
char nodes_buffer[1024] = "{\"nodes\":[\"1234\",\"2345\",\"4567\"]}";

// char params_buffer[1024] = "{"type":"esp.device.lightbulb","parameters":[{"name":"esp.param.brightness","flags":1,"value":100,},{"name":"esp.param.power","flags":1,"value":1,},{"name":"esp.param.hue","flags":1,"value":100,},{"name":"esp.param.saturation","flags":1,"value": 254,}]}";
char params_buffer[1024] = "{\"type\":\"esp.device.lightbulb\",\"parameters\":[{\"name\":\"esp.param.brightness\",\"flags\":1,\"value\":100},{\"name\":\"esp.param.power\",\"flags\":1,\"value\":1},{\"name\":\"esp.param.hue\",\"flags\":1,\"value\":100},{\"name\":\"esp.param.saturation\",\"flags\":1,\"value\":254}]}";

esp_err_t rainmaker_bridge_attribute_update(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id,
                                         esp_matter_attr_val_t *val, app_bridged_device_t *zigbee_device)
{
    return ESP_OK;
}

esp_err_t rainmaker_get_nodes(char *out_buf, size_t out_buf_len)
{
    jparse_ctx_t jctx;
    int total_count;
    char node[24];
    if (json_parse_start(&jctx, out_buf, out_buf_len) != 0) {
        return ESP_FAIL;
    }

    if (json_obj_get_array(&jctx, "nodes", &total_count) == 0) {
        printf("Array has %d elements\n", total_count);
        for (int i = 0; i < total_count; i++) {
            if (json_arr_get_string(&jctx, i, node, sizeof(node)) == 0) {
                printf("node id %d: %s\n", i, node);
                if (check_new_node()) {
                    snprintf(&new_node_list[new_node_count][0], 24, "%s", node);
                    new_node_list++;
                }
            }
        }
        json_obj_leave_array(&jctx);
    } else {
        ESP_LOGE(TAG, "No node found in json ");
        json_obj_leave_array(&jctx);
    }

    json_parse_end(&jctx);
    return ESP_OK;
}

esp_err_t rainmaker_bridge_add_new_device()
{
    char rev_buffer[1024] = {0};
    int rev_length = 0;
    for (int i = 0; i < new_node_list; i++) {
        if (new_node_list[i][0] != 0) {
            rainmaker_get_node_details(&new_node_list[i][0], rev_buffer, &rev_length);
            rainmaker_parse_node_details(rev_buffer);
        }
    }
}

esp_err_t rainmaker_parse_node_details(char *out_buf)
{
    jparse_ctx_t jctx;
    int param_count;
    char str_date[32];
    int flag;
    int value;

    if (json_parse_start(&jctx, out_buf, sizeof(out_buf)) != 0) {
        return ESP_FAIL;
    }

    if (json_obj_get_string(&jctx, "type", str_date, sizeof(str_date)) == 0) {
        //get device type
        printf("type: %s\n", str_date);
    } else {
        ESP_LOGE(TAG, "No type found in json ");
        return ESP_FAIL;
    }

    if (json_obj_get_array(&jctx, "parameters", &param_count) == 0) {
        for (int i = 0; i < param_count; i++) {
            if (json_arr_get_object(&jctx, i) == 0) {
                printf("param index: %d\n", i);
                if (json_obj_get_string(&jctx, "name", str_date, sizeof(str_date)) == 0) {
                    //get param name
                    printf("param name: %s\n", str_date);
                }
                if (json_obj_get_int(&jctx, "flags", &flag) == 0) {
                    //get param flag
                    printf("param flag: %d\n", flag);
                }
                if (json_obj_get_int(&jctx, "value", &value) == 0) {
                    //get param value
                    printf("param value: %d\n", value);
                }

                json_arr_leave_object(&jctx);
            }
        }
        json_obj_leave_array(&jctx);
    } else {
        ESP_LOGE(TAG, "No parameters found in json ");
        return ESP_FAIL;
    }

    json_obj_leave_object(&jctx);

    return ESP_OK;
}

esp_err_t rainmaker_write_param(const char *node_id, char *out_buf, size_t *out_buf_len)
{
    //
    return ESP_OK;
}

static bool check_new_node(const char *node_id)
{
    return (app_bridge_get_matter_endpointid_by_rainmaker_node_id(node_id) == chip::kInvalidEndpointId);
}

static uint32_t matter_get_device_type_from_rainmaker_device()
{
    return 0;
}

esp_err_t rainmaker_bridge_match_device(const char *node_id, uint32_t device_type_id)
{
    node_t *node = node::get();

    if (check_new_node(node_id)) {
        app_bridged_device_t *bridged_device = app_bridge_create_bridged_device(node, aggregator_endpoint_id, device_type_id,
                                                                                ESP_MATTER_BRIDGED_DEVICE_TYPE_RAINMAKER,
                                                                                app_bridge_rainmaker_adress(node_id), NULL);
        ESP_RETURN_ON_FALSE(bridged_device, ESP_FAIL, TAG, "Failed to create bridged device (rainmaker device)");
    } else {
        ESP_LOGI(TAG, "Bridged node for %s rainmaker device has been created", node_id);
    }

    return ESP_OK;
}

esp_err_t rainmaker_bridge_delete_device(uint16_t endpoint_id)
{
    const char* node_id = app_bridge_get_rainmaker_node_id_by_matter_endpointid(endpoint_id);

    if (node_id == NULL) {
        ESP_LOGI(TAG, "Can't find rainmaker device from ep: %d", endpoint_id);
    }

    app_bridged_device_t *bridged_device = app_bridge_get_device_by_rainmaker_node_id(node_id);

    if (bridged_device) {
        app_bridge_remove_device(bridged_device);
        ESP_LOGI(TAG, "Bridged rainmaker device removed: %s", node_id);
    } else {
        ESP_LOGI(TAG, "Bridged rainmaker device not found: %s", node_id);
    }
    return ESP_OK;
}

static void rainmaker_bridge_task(void *pvParameters)
{
    while(1) {
        //
        vTaskDelay(pdMS_TO_TICKS(5000));
        rainmaker_get_nodes(nodes_buffer, strlen(nodes_buffer));
        // rainmaker_get_node_details(NULL, params_buffer, strlen(params_buffer));
    }
}

void rainmaker_init()
{
    //rainmaker init
    //create task to get node and params from rainmaker side
    xTaskCreate(rainmaker_bridge_task, "rainmaker_main", 10240, xTaskGetCurrentTaskHandle(), 5, NULL);
}

