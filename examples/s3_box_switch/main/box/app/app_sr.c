/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_task_wdt.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "app_sr.h"
#include "bsp_codec.h"
#include "bsp_i2s.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_models.h"
#include "app_sr_handler.h"
#include "bsp_board.h"
#include "bsp_btn.h"
#include "settings.h"

static const char *TAG = "app_sr";

typedef struct {
    sr_language_t lang;
    model_iface_data_t *model_data;
    const esp_mn_iface_t *multinet;
    const esp_afe_sr_iface_t *afe_handle;
    esp_afe_sr_data_t *afe_data;
    int16_t *afe_in_buffer;
    int16_t *afe_out_buffer;
    SLIST_HEAD(sr_cmd_list_t, sr_cmd_t) cmd_list;
    uint8_t cmd_num;
    TaskHandle_t feed_task;
    TaskHandle_t detect_task;
    TaskHandle_t handle_task;
    QueueHandle_t result_que;
    EventGroupHandle_t event_group;

    FILE *fp;
    bool b_record_en;
} sr_data_t;

static sr_data_t *g_sr_data = NULL;

#define I2S_CHANNEL_NUM     (2)
#define NEED_DELETE BIT0
#define FEED_DELETED BIT1
#define DETECT_DELETED BIT2

/**
 * @brief all default commands
 */
static const sr_cmd_t g_default_cmd_info[] = {
    // English
    {SR_CMD_FAN_ON, SR_LANG_EN, 0, "Turn On the Fan", "TkN nN jc FaN", {NULL}},
    {SR_CMD_FAN_OFF, SR_LANG_EN, 0, "Turn Off the Fan", "TkN eF jc FaN", {NULL}},
    {SR_CMD_PLUG_ON, SR_LANG_EN, 0, "Turn On the Plug", "TkN nN jc PLcG", {NULL}},
    {SR_CMD_PLUG_OFF, SR_LANG_EN, 0, "Turn Off the Plug", "TkN eF jc PLcG", {NULL}},
    {SR_CMD_SET_RED, SR_LANG_EN, 0, "Turn Red", "TkN RfD", {NULL}},
    {SR_CMD_SET_GREEN, SR_LANG_EN, 0, "Turn Green", "TkN GRmN", {NULL}},
    {SR_CMD_SET_BLUE, SR_LANG_EN, 0, "Turn Blue", "TkN BLo", {NULL}},
    {SR_CMD_CUSTOMIZE_COLOR, SR_LANG_EN, 0, "Customize Color", "KcSTcMiZ KcLk", {NULL}},
    {SR_CMD_PLAY, SR_LANG_EN, 0, "Sing a song", "Sgl c Sel", {NULL}},
    {SR_CMD_PLAY, SR_LANG_EN, 0, "Play Music", "PLd MYoZgK", {NULL}},
    {SR_CMD_NEXT, SR_LANG_EN, 0, "Next Song", "NfKST Sel", {NULL}},
    {SR_CMD_PAUSE, SR_LANG_EN, 0, "Pause Playing", "PeZ PLdgl", {NULL}},

};

static void audio_feed_task(void *pvParam)
{
    size_t bytes_read = 0;
    const esp_afe_sr_iface_t *afe_handle = g_sr_data->afe_handle;
    esp_afe_sr_data_t *afe_data = (esp_afe_sr_data_t *) pvParam;
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int feed_channel = 3;
    ESP_LOGI(TAG, "audio_chunksize=%d, feed_channel=%d", audio_chunksize, feed_channel);
    /* Allocate audio buffer and check for result */
    int16_t *audio_buffer = heap_caps_malloc(audio_chunksize * sizeof(int16_t) * feed_channel, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (NULL == audio_buffer) {
        esp_system_abort("No mem for audio buffer");
    }
    g_sr_data->afe_in_buffer = audio_buffer;

    while (true) {
        if (NEED_DELETE && xEventGroupGetBits(g_sr_data->event_group)) {
            xEventGroupSetBits(g_sr_data->event_group, FEED_DELETED);
            vTaskDelete(NULL);
        }

        /* Read audio data from I2S bus */
        i2s_read(I2S_NUM_0, audio_buffer, audio_chunksize * I2S_CHANNEL_NUM * sizeof(int16_t), &bytes_read, portMAX_DELAY);

        /* Save audio data to file if record enabled */
        if (g_sr_data->b_record_en && (NULL != g_sr_data->fp)) {
            fwrite(audio_buffer, 1, audio_chunksize * I2S_CHANNEL_NUM * sizeof(int16_t), g_sr_data->fp);
        }

        /* Channel Adjust */
        for (int  i = audio_chunksize - 1; i >= 0; i--) {
            audio_buffer[i * 3 + 2] = 0;
            audio_buffer[i * 3 + 1] = audio_buffer[i * 2 + 1];
            audio_buffer[i * 3 + 0] = audio_buffer[i * 2 + 0];
        }

        /* Feed samples of an audio stream to the AFE_SR */
        afe_handle->feed(afe_data, audio_buffer);
    }
}

static void audio_detect_task(void *pvParam)
{
    bool detect_flag = false;
    esp_afe_sr_data_t *afe_data = (esp_afe_sr_data_t *) pvParam;

    /* Allocate buffer for detection */
    size_t afe_chunk_size = g_sr_data->afe_handle->get_fetch_chunksize(afe_data);
    g_sr_data->afe_out_buffer = heap_caps_malloc(afe_chunk_size * sizeof(int16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (NULL == g_sr_data->afe_out_buffer) {
        ESP_LOGE(TAG, "Expect : %zu, avaliable : %zu",
                 afe_chunk_size * sizeof(int16_t),
                 heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
        esp_system_abort("No mem for detect buffer");
    }

    /* Check for chunk size */
    if (afe_chunk_size != g_sr_data->multinet->get_samp_chunksize(g_sr_data->model_data)) {
        esp_system_abort("Invalid chunk size");
    }

    while (true) {
        if (NEED_DELETE && xEventGroupGetBits(g_sr_data->event_group)) {
            xEventGroupSetBits(g_sr_data->event_group, DETECT_DELETED);
            vTaskDelete(g_sr_data->handle_task);
            vTaskDelete(NULL);
        }

        afe_fetch_mode_t ret_val = g_sr_data->afe_handle->fetch(afe_data, g_sr_data->afe_out_buffer);

        if (AFE_FETCH_WWE_DETECTED == ret_val) {
            ESP_LOGI(TAG, LOG_BOLD(LOG_COLOR_GREEN) "wakeword detected");
            sr_result_t result = {
                .fetch_mode = ret_val,
                .state = ESP_MN_STATE_DETECTING,
                .command_id = 0,
            };
            xQueueSend(g_sr_data->result_que, &result, 0);
        }

        if (AFE_FETCH_CHANNEL_VERIFIED == ret_val) {
            ESP_LOGI(TAG, LOG_BOLD(LOG_COLOR_GREEN) "Channel verified");
            detect_flag = true;
            g_sr_data->afe_handle->disable_wakenet(afe_data);
        }

        if (true == detect_flag) {
            /* Save audio data to file if record enabled */
            if (g_sr_data->b_record_en && (NULL != g_sr_data->fp)) {
                fwrite(g_sr_data->afe_out_buffer, 1, afe_chunk_size * sizeof(int16_t), g_sr_data->fp);
            }

            esp_mn_state_t mn_state = ESP_MN_STATE_DETECTING;
            if (false == sr_echo_is_playing()) {
                mn_state = g_sr_data->multinet->detect(g_sr_data->model_data, g_sr_data->afe_out_buffer);
            } else {
                continue;
            }

            if (ESP_MN_STATE_DETECTING == mn_state) {
                continue;
            }

            if (ESP_MN_STATE_TIMEOUT == mn_state) {
                ESP_LOGW(TAG, "Time out");
                sr_result_t result = {
                    .fetch_mode = ret_val,
                    .state = mn_state,
                    .command_id = 0,
                };
                xQueueSend(g_sr_data->result_que, &result, 0);
                g_sr_data->afe_handle->enable_wakenet(afe_data);
                detect_flag = false;
                continue;
            }

            if (ESP_MN_STATE_DETECTED <= mn_state) {
                esp_mn_results_t *mn_result = g_sr_data->multinet->get_results(g_sr_data->model_data);
                for (int i = 0; i < mn_result->num; i++) {
                    ESP_LOGI(TAG, "TOP %d, command_id: %d, phrase_id: %d, prob: %f",
                             i + 1, mn_result->command_id[i], mn_result->phrase_id[i], mn_result->prob[i]);
                }

                int sr_command_id = mn_result->command_id[0];
                ESP_LOGI(TAG, "Deteted command : %d", sr_command_id);
                sr_result_t result = {
                    .fetch_mode = ret_val,
                    .state = mn_state,
                    .command_id = sr_command_id,
                };
                xQueueSend(g_sr_data->result_que, &result, 0);
// #if !SR_CONTINUE_DET
                g_sr_data->afe_handle->enable_wakenet(afe_data);
                detect_flag = 0;
// #endif

                if (g_sr_data->b_record_en && (NULL != g_sr_data->fp)) {
                    ESP_LOGI(TAG, "File saved");
                    fclose(g_sr_data->fp);
                    g_sr_data->fp = NULL;
                }
                continue;
            }

            ESP_LOGE(TAG, "Exception unhandled");
        }
    }

    /* Task never returns */
    vTaskDelete(NULL);
}

esp_err_t app_sr_set_language(sr_language_t new_lang)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");

    if (new_lang == g_sr_data->lang) {
        ESP_LOGW(TAG, "nothing to do");
        return ESP_OK;
    } else {
        g_sr_data->lang = new_lang;
    }

    ESP_LOGW(TAG, "Set language to %s", SR_LANG_EN == g_sr_data->lang ? "EN" : "CN");
    if (g_sr_data->model_data) {
        g_sr_data->multinet->destroy(g_sr_data->model_data);
    }
    switch (g_sr_data->lang) {
    case SR_LANG_EN:
        g_sr_data->afe_handle->set_wakenet(g_sr_data->afe_data, FIRST_WAKE_WORD);
        g_sr_data->multinet = &MULTINET_MODEL_EN;
        g_sr_data->model_data = g_sr_data->multinet->create((const model_coeff_getter_t *) &MULTINET_COEFF_EN, 5760);
        break;
    case SR_LANG_CN:
        g_sr_data->afe_handle->set_wakenet(g_sr_data->afe_data, SECOND_WAKE_WORD);
        g_sr_data->multinet = &MULTINET_MODEL_CN;
        g_sr_data->model_data = g_sr_data->multinet->create((const model_coeff_getter_t *) &MULTINET_COEFF_CN, 5760);
        break;
    default:
        break;
    }

    // remove all command
    sr_cmd_t *it;
    while (!SLIST_EMPTY(&g_sr_data->cmd_list)) {
        it = SLIST_FIRST(&g_sr_data->cmd_list);
        SLIST_REMOVE_HEAD(&g_sr_data->cmd_list, next);
        heap_caps_free(it);
    }

    uint8_t cmd_number = 0;
    // count command number
    for (size_t i = 0; i < sizeof(g_default_cmd_info) / sizeof(sr_cmd_t); i++) {
        if (g_default_cmd_info[i].lang == g_sr_data->lang) {
            cmd_number++;
            app_sr_add_cmd(&g_default_cmd_info[i]);
        }
    }
    ESP_LOGI(TAG, "cmd_number=%d", cmd_number);
    return app_sr_update_cmds();/* Reset command list */
}

esp_err_t app_sr_start(bool record_en)
{
printf("In app_sr_start\n");
    esp_err_t ret = ESP_OK;
    ESP_RETURN_ON_FALSE(NULL == g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR already running");

    g_sr_data = heap_caps_calloc(1, sizeof(sr_data_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_NO_MEM, TAG, "Failed create sr data");

    g_sr_data->result_que = xQueueCreate(3, sizeof(sr_result_t));
    ESP_GOTO_ON_FALSE(NULL != g_sr_data->result_que, ESP_ERR_NO_MEM, err, TAG, "Failed create result queue");

    g_sr_data->event_group = xEventGroupCreate();
    ESP_GOTO_ON_FALSE(NULL != g_sr_data->event_group, ESP_ERR_NO_MEM, err, TAG, "Failed create event_group");

    SLIST_INIT(&g_sr_data->cmd_list);

    /* Create file if record to SD card enabled*/
    g_sr_data->b_record_en = record_en;
    if (record_en) {
        char file_name[32];
        for (size_t i = 0; i < 100; i++) {
            sprintf(file_name, "/sdcard/Record_%02d.pcm", i);
            g_sr_data->fp = fopen(file_name, "r");
            fclose(g_sr_data->fp);
            if (NULL == g_sr_data->fp) {
                break;
            }
        }
        g_sr_data->fp = fopen(file_name, "w");
        ESP_GOTO_ON_FALSE(NULL != g_sr_data->fp, ESP_FAIL, err, TAG, "Failed create record file");
        ESP_LOGI(TAG, "File created at %s", file_name);
    }
    esp_task_wdt_reset();
    g_sr_data->afe_handle = &ESP_AFE_HANDLE;
    afe_config_t afe_config = AFE_CONFIG_DEFAULT();
    afe_config.aec_init = false;
    // afe_config.vad_init = false;
    g_sr_data->afe_data = g_sr_data->afe_handle->create_from_config(&afe_config);
    const sys_param_t *param = settings_get_parameter();
    g_sr_data->lang = SR_LANG_MAX;
    ret = app_sr_set_language(param->sr_lang);
    ESP_GOTO_ON_FALSE(ESP_OK == ret, ESP_FAIL, err, TAG,  "Failed to set language");

    BaseType_t ret_val = xTaskCreatePinnedToCore(audio_feed_task, "Feed Task", 4 * 1024, g_sr_data->afe_data, 5, &g_sr_data->feed_task, 1);
    ESP_GOTO_ON_FALSE(pdPASS == ret_val, ESP_FAIL, err, TAG,  "Failed create audio feed task");

    ret_val = xTaskCreatePinnedToCore(audio_detect_task, "Detect Task", 6 * 1024, g_sr_data->afe_data, 5, &g_sr_data->detect_task, 1);
    ESP_GOTO_ON_FALSE(pdPASS == ret_val, ESP_FAIL, err, TAG,  "Failed create audio detect task");

    ret_val = xTaskCreatePinnedToCore(sr_handler_task, "SR Handler Task", 6 * 1024, NULL, configMAX_PRIORITIES - 3, &g_sr_data->handle_task, 0);
    ESP_GOTO_ON_FALSE(pdPASS == ret_val, ESP_FAIL, err, TAG,  "Failed create audio handler task");
    esp_task_wdt_reset();
printf("returning ok\n");

    return ESP_OK;

err:
    app_sr_stop();
printf("returning error\n");
    return ret;
}

esp_err_t app_sr_stop(void)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");

    /**
     * Waiting for all task stoped
     * TODO: A task creation failure cannot be handled correctly now
     * */
    xEventGroupSetBits(g_sr_data->event_group, NEED_DELETE);
    xEventGroupWaitBits(g_sr_data->event_group, NEED_DELETE | FEED_DELETED | DETECT_DELETED, 1, 1, portMAX_DELAY);

    if (g_sr_data->result_que) {
        vQueueDelete(g_sr_data->result_que);
        g_sr_data->result_que = NULL;
    }

    if (g_sr_data->event_group) {
        vEventGroupDelete(g_sr_data->event_group);
        g_sr_data->event_group = NULL;
    }

    if (g_sr_data->fp) {
        fclose(g_sr_data->fp);
        g_sr_data->fp = NULL;
    }

    if (g_sr_data->model_data) {
        g_sr_data->multinet->destroy(g_sr_data->model_data);
    }

    if (g_sr_data->afe_data) {
        g_sr_data->afe_handle->destroy(g_sr_data->afe_data);
    }

    sr_cmd_t *it;
    while (!SLIST_EMPTY(&g_sr_data->cmd_list)) {
        it = SLIST_FIRST(&g_sr_data->cmd_list);
        SLIST_REMOVE_HEAD(&g_sr_data->cmd_list, next);
        heap_caps_free(it);
    }

    if (g_sr_data->afe_in_buffer) {
        heap_caps_free(g_sr_data->afe_in_buffer);
    }

    if (g_sr_data->afe_out_buffer) {
        heap_caps_free(g_sr_data->afe_out_buffer);
    }

    heap_caps_free(g_sr_data);
    g_sr_data = NULL;
    return ESP_OK;
}

esp_err_t app_sr_get_result(sr_result_t *result, TickType_t xTicksToWait)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");

    xQueueReceive(g_sr_data->result_que, result, xTicksToWait);
    return ESP_OK;
}

esp_err_t app_sr_add_cmd(const sr_cmd_t *cmd)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");
    ESP_RETURN_ON_FALSE(NULL != cmd, ESP_ERR_INVALID_ARG, TAG, "pointer of cmd is invaild");
    ESP_RETURN_ON_FALSE(cmd->lang == g_sr_data->lang, ESP_ERR_INVALID_ARG, TAG, "cmd lang error");
    ESP_RETURN_ON_FALSE(200 >= g_sr_data->cmd_num, ESP_ERR_INVALID_STATE, TAG, "cmd is full");
    sr_cmd_t *item = (sr_cmd_t *)heap_caps_calloc(1, sizeof(sr_cmd_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_RETURN_ON_FALSE(NULL != item, ESP_ERR_NO_MEM, TAG, "memory for sr cmd is not enough");
    memcpy(item, cmd, sizeof(sr_cmd_t));
    item->next.sle_next = NULL;
#if 1 // insert after
    sr_cmd_t *last = SLIST_FIRST(&g_sr_data->cmd_list);
    if (last == NULL) {
        SLIST_INSERT_HEAD(&g_sr_data->cmd_list, item, next);
    } else {
        sr_cmd_t *it;
        while ((it = SLIST_NEXT(last, next)) != NULL) {
            last = it;
        }
        SLIST_INSERT_AFTER(last, item, next);
    }
#else  // insert head
    SLIST_INSERT_HEAD(&g_sr_data->cmd_list, it, next);
#endif
    g_sr_data->cmd_num++;
    return ESP_OK;
}

esp_err_t app_sr_modify_cmd(uint32_t id, const sr_cmd_t *cmd)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");
    ESP_RETURN_ON_FALSE(NULL != cmd, ESP_ERR_INVALID_ARG, TAG, "pointer of cmd is invaild");
    ESP_RETURN_ON_FALSE(id < g_sr_data->cmd_num, ESP_ERR_INVALID_ARG, TAG, "cmd id out of range");
    ESP_RETURN_ON_FALSE(cmd->lang == g_sr_data->lang, ESP_ERR_INVALID_ARG, TAG, "cmd lang error");

    sr_cmd_t *it;
    SLIST_FOREACH(it, &g_sr_data->cmd_list, next) {
        if (it->id == id) {
            ESP_LOGI(TAG, "modify cmd [%d] from %s to %s", id, it->str, cmd->str);
            memcpy(it, cmd, sizeof(sr_cmd_t));
            break;
        }
    }
    ESP_RETURN_ON_FALSE(NULL != it, ESP_ERR_NOT_FOUND, TAG, "can't find cmd id:%d", cmd->id);
    return ESP_OK;
}

esp_err_t app_sr_remove_cmd(uint32_t id)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");
    ESP_RETURN_ON_FALSE(id < g_sr_data->cmd_num, ESP_ERR_INVALID_ARG, TAG, "cmd id out of range");
    sr_cmd_t *it;
    SLIST_FOREACH(it, &g_sr_data->cmd_list, next) {
        if (it->id == id) {
            ESP_LOGI(TAG, "remove cmd id [%d]", it->id);
            SLIST_REMOVE(&g_sr_data->cmd_list, it, sr_cmd_t, next);
            heap_caps_free(it);
            g_sr_data->cmd_num--;
            break;
        }
    }
    ESP_RETURN_ON_FALSE(NULL != it, ESP_ERR_NOT_FOUND, TAG, "can't find cmd id:%d", id);
    return ESP_OK;
}

esp_err_t app_sr_remove_all_cmd(void)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");
    sr_cmd_t *it;
    while (!SLIST_EMPTY(&g_sr_data->cmd_list)) {
        it = SLIST_FIRST(&g_sr_data->cmd_list);
        SLIST_REMOVE_HEAD(&g_sr_data->cmd_list, next);
        heap_caps_free(it);
    }
    SLIST_INIT(&g_sr_data->cmd_list);
    return ESP_OK;
}

esp_err_t app_sr_update_cmds(void)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, ESP_ERR_INVALID_STATE, TAG, "SR is not running");

    char *cmd_str = heap_caps_calloc(g_sr_data->cmd_num, SR_CMD_PHONEME_LEN_MAX, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    ESP_RETURN_ON_FALSE(NULL != cmd_str, ESP_ERR_NO_MEM, TAG, "memory for sr cmd str is not enough");

    uint32_t count = 0;
    sr_cmd_t *it;
    SLIST_FOREACH(it, &g_sr_data->cmd_list, next) {
        it->id = count++;
        strcat(cmd_str, it->phoneme);
        strcat(cmd_str, ";");
    }

    ESP_LOGI(TAG, "New %d command set to :[%s]", count, cmd_str);

    esp_mn_phrase_err_id_t *err_id = g_sr_data->multinet->reset(g_sr_data->model_data, cmd_str, -1);
    for (int i = 0; i < err_id->err_id_num; i++) {
        ESP_LOGE(TAG, "err cmd id:%d", err_id->err_id[i]);
    }
    heap_caps_free(cmd_str);
    return ESP_OK;
}

uint8_t app_sr_search_cmd_from_user_cmd(sr_user_cmd_t user_cmd, uint8_t *id_list, uint16_t max_len)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, 0, TAG, "SR is not running");

    uint8_t cmd_num = 0;
    sr_cmd_t *it;
    SLIST_FOREACH(it, &g_sr_data->cmd_list, next) {
        if (user_cmd == it->cmd) {
            if (id_list) {
                id_list[cmd_num] = it->id;
            }
            if (++cmd_num >= max_len) {
                break;
            }
        }
    }
    return cmd_num;
}

uint8_t app_sr_search_cmd_from_phoneme(const char *phoneme, uint8_t *id_list, uint16_t max_len)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, 0, TAG, "SR is not running");

    uint8_t cmd_num = 0;
    sr_cmd_t *it;
    SLIST_FOREACH(it, &g_sr_data->cmd_list, next) {
        if (0 == strcmp(phoneme, it->phoneme)) {
            if (id_list) {
                id_list[cmd_num] = it->id;
            }
            if (++cmd_num >= max_len) {
                break;
            }
        }
    }
    return cmd_num;
}

const sr_cmd_t *app_sr_get_cmd_from_id(uint32_t id)
{
    ESP_RETURN_ON_FALSE(NULL != g_sr_data, NULL, TAG, "SR is not running");
    ESP_RETURN_ON_FALSE(id < g_sr_data->cmd_num, NULL, TAG, "cmd id out of range");

    sr_cmd_t *it;
    SLIST_FOREACH(it, &g_sr_data->cmd_list, next) {
        if (id == it->id) {
            return it;
        }
    }
    ESP_RETURN_ON_FALSE(NULL != it, NULL, TAG, "can't find cmd id:%d", id);
    return NULL;
}
