/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* It is recommended to copy this code in your example so that you can modify as per your application's needs,
 * especially for the indicator calbacks, button_factory_reset_pressed_cb() and button_factory_reset_released_cb().
 */

#include <esp_log.h>
#include <esp_matter.h>
#include "iot_button.h"
#include "esp_partition.h"

static const char *TAG = "app_reset";
static bool perform_factory_reset = false;

static void button_factory_reset_pressed_cb(void *arg, void *data)
{
    if (!perform_factory_reset) {
        ESP_LOGI(TAG, "Factory reset triggered. Release the button to start factory reset.");
        perform_factory_reset = true;
    }
}

static void factory_reset_zigbee()
{
    const esp_partition_t *fat_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,     // 分区类型为 DATA
        ESP_PARTITION_SUBTYPE_DATA_FAT, // 分区子类型为 FAT
        "zb_storage"                    // 分区标签（可选）
    );

    if (fat_partition != NULL) {
        printf("address:%ld ----- size:%ld\n", fat_partition->address, fat_partition->size);
        esp_err_t err = esp_partition_erase_range(fat_partition, 0, fat_partition->size);
        if (err != ESP_OK) {
            printf("Failed to erase FAT partition: %s\n", esp_err_to_name(err));
        } else {
            printf("FAT partition erased successfully.\n");
        }
    } else {
        printf("FAT partition not found.\n");
    }
}

static void button_factory_reset_released_cb(void *arg, void *data)
{
    if (perform_factory_reset) {
        ESP_LOGI(TAG, "Starting factory reset");
        factory_reset_zigbee();
        esp_matter::factory_reset();
        perform_factory_reset = false;
    }
}

esp_err_t app_reset_button_register(void *handle)
{
    if (!handle) {
        ESP_LOGE(TAG, "Handle cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    button_handle_t button_handle = (button_handle_t)handle;
    esp_err_t err = ESP_OK;
    err |= iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_HOLD, button_factory_reset_pressed_cb, NULL);
    err |= iot_button_register_cb(button_handle, BUTTON_PRESS_UP, button_factory_reset_released_cb, NULL);
    return err;
}
