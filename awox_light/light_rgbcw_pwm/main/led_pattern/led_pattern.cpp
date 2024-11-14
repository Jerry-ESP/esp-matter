// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
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

#include <esp_err.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>

#include <led_pattern.h>

static const char *TAG = "led_pattern";

static led_pattern_handle_t g_pattern_handle = NULL;
static SemaphoreHandle_t g_semaphore_handle = NULL;

led_pattern_handle_t get_pattern_handle(void)
{
    return g_pattern_handle;
}

static void led_pattern_change_state(TimerHandle_t timer)
{
    led_pattern_t *pattern = (led_pattern_t *)g_pattern_handle;//pvTimerGetTimerID(timer);
    pattern->state = !pattern->state;

    ESP_LOGD(TAG, "Pattern state change: %d", pattern->state);
    //led_driver_set_power(pattern->handle, pattern->state);
    if (pattern->state) {
        pattern->led_driver.setOn();
    } else {
        pattern->led_driver.setOff();
    }

    if (pattern->state && pattern->count > 0) {
        pattern->count--;
    }

    /* Stop is called */
    if (pattern->end_pattern) {
        xTimerDelete(timer, portMAX_DELAY);
        pattern->end_pattern = false;
        return;
    }

    /* Auto stop. */
    if (pattern->count == 0) {
        // led_driver_set_power(pattern->handle, pattern->restore_state);
        if (pattern->restore_state) {
            pattern->led_driver.setOn();
        } else {
            pattern->led_driver.setOff();
        }
        ESP_LOGW(TAG, "Pattern auto stop, state restore: %d", pattern->restore_state);
        if (pattern->callback) {
            pattern->callback((led_pattern_handle_t)pattern);
        }
        if (xSemaphoreTake(g_semaphore_handle, 0) == pdTRUE) {
            xTimerDelete(timer, portMAX_DELAY);
            xSemaphoreGive(g_semaphore_handle);
        } else {
            /* Stop has already taken the lock. Start the timer again to handle it in the next cycle */
            xTimerStart(timer, portMAX_DELAY);
        }
        return;
    }

    /* Restart the timer. */
    xTimerStart(timer, portMAX_DELAY);
}

esp_err_t led_pattern_blink_start(RgbTw::Driver& led_driver, int count, int delay_ms,
                                             led_pattern_blink_callback_t callback)
{
    if (!g_semaphore_handle) {
        g_semaphore_handle = xSemaphoreCreateMutex();
        if (!g_semaphore_handle) {
            ESP_LOGE(TAG, "Unable to create semaphore mutex");
            return ESP_FAIL;
        }
    }
    if (xSemaphoreTake(g_semaphore_handle, portMAX_DELAY) == pdTRUE) {
        led_pattern_t *pattern = (led_pattern_t *)calloc(1, sizeof(led_pattern_t));
        if (!pattern) {
            ESP_LOGE(TAG, "Could not allocate pattern");
            xSemaphoreGive(g_semaphore_handle);
            return ESP_ERR_NO_MEM;
        }
        pattern->led_driver = led_driver;
        pattern->state = true;
        pattern->count = count;
        pattern->restore_state = 1;
        pattern->callback = callback;
        int half_blink_delay_ms = delay_ms / 2;

        TimerHandle_t timer = xTimerCreate("led_pattern", pdMS_TO_TICKS(half_blink_delay_ms), pdFALSE, pattern, &led_pattern_change_state);
        xTimerStart(timer, portMAX_DELAY);
        g_pattern_handle = (led_pattern_handle_t)pattern;
        xSemaphoreGive(g_semaphore_handle);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Unable to get the led_pattern lock to start the pattern");
    }
    return ESP_FAIL;
}

esp_err_t led_pattern_blink_stop_restore(led_pattern_handle_t handle, bool restore)
{
    if (!g_semaphore_handle) {
        ESP_LOGE(TAG, "Semaphore handle is null");
        return ESP_FAIL;
    }
    if (!handle) {
        ESP_LOGE(TAG, "Handle can't be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    if (xSemaphoreTake(g_semaphore_handle, 0) == pdTRUE) {
        if (g_pattern_handle && g_pattern_handle == handle) {
            led_pattern_t *pattern = (led_pattern_t *)handle;
            bool onoff_state = true;
            if (restore) {
                onoff_state = pattern->restore_state;
            }
            xSemaphoreGive(g_semaphore_handle);
            return led_pattern_blink_stop_with_power(handle, onoff_state);
        } else {
            /* NOTE: Code should not reach here, this print means that there is a memory leak */
            ESP_LOGE(TAG, "Pattern handle does not match the g_pattern_handle");
        }
        xSemaphoreGive(g_semaphore_handle);
    } else {
        ESP_LOGE(TAG, "Unable to get the led_pattern lock");
    }
    return ESP_FAIL;
}

esp_err_t led_pattern_blink_stop_with_power(led_pattern_handle_t handle, bool on_off)
{
    if (!g_semaphore_handle) {
        ESP_LOGE(TAG, "Semaphore handle is null");
        return ESP_FAIL;
    }
    if (!handle) {
        ESP_LOGE(TAG, "Handle can't be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    if (xSemaphoreTake(g_semaphore_handle, 0) == pdTRUE) {
        if (g_pattern_handle && g_pattern_handle == handle) {
            led_pattern_t *pattern = (led_pattern_t *)g_pattern_handle;
            pattern->end_pattern = true;
            while (pattern->end_pattern) {
                ESP_LOGI(TAG, "Waiting for pattern to end");
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            if (on_off) {
                pattern->led_driver.setOn();
            } else {
                pattern->led_driver.setOff();
            }
        } else {
            /* NOTE: Code should not reach here, this print means that there is a memory leak */
            ESP_LOGE(TAG, "Pattern handle does not match the g_pattern_handle");
        }
        xSemaphoreGive(g_semaphore_handle);
    } else {
        /* Wait to return in case second task tries to stop the led_pattern while the semaphore is taken by first task
         and return only when the handle is freed i.e. equal to null. This prevents the double free issue.*/
        while(g_pattern_handle) {
            ESP_LOGE(TAG, "Waiting for the previous pattern handle to be freed");
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    return ESP_OK;
}
