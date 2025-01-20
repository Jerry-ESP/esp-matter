/**
 * @file RebootCounterManager.cpp
 * @author AWOX
 * @brief Handle the counter of electrical reboots and its following uses
 *
 */
#include "RebootCounterManager.hpp"
#include "Api.hpp"
#include "TaskConfig.hpp"
#include "esp_log.h"
#include <cmath>

/**
 * @brief Reboot counter Namespace
 *
 */
namespace RebootCounterManager {

constexpr static const char* TAG = "awox_reboot_counter"; /**< @brief Espressif tag for Log */

/** We need this function because the task create requires a static task and that cause problem with cpp */
void RebootCounterManager::wrapperTask(void* pvParameters)
{
    ((RebootCounterManager*)pvParameters)->managerTask();
}

/**
 * @brief  RebootCounterManager task creation
 *
 * @param handle the task handle object
 * @return BaseType_t the task creation result
 */
BaseType_t RebootCounterManager::createManagerTask(TaskHandle_t* handle)
{
    BaseType_t baseType = xTaskCreate(RebootCounterManager::wrapperTask, rebootTask.name, rebootTask.size, this, rebootTask.priority, handle);
    _taskHandle = *handle;
    return baseType;
}

/**
 * @brief Get the task handle of RebootCounterManager
 *
 * @return TaskHandle_t
 */
TaskHandle_t RebootCounterManager::getTaskHandle()
{
    return _taskHandle;
}

/**
 * @brief Construct a new Reboot Counter
 *
 */
RebootCounterManager::RebootCounterManager(uint8_t resets)
{
    ESP_LOGI(TAG, "Reboot Counter Manager created with %d reboots", resets);
    nbReboots = resets;
}

/**
 * @brief Main task (loop)
 *
 */
void RebootCounterManager::managerTask()
{
    while (true) {
        ESP_LOGD(TAG, "Reboot stage : %d", static_cast<uint8_t>(stage));
        currentTime = nextStageTime;
        switch (stage) {
        case stateReboots_t::START:
            stageStart();
            break;
        case stateReboots_t::ON_TIME:
            stageOnTime();
            break;
        case stateReboots_t::OFF_TIME:
            stageOffTime();
            break;
        case stateReboots_t::FINISH:
            stageFinish();
            break;
        default:
            stageFinish();
            break;
        }
    }
}

/**
 * @brief Starting stage of the reboot
 * Get reboot counter, then go to first stage (ON_TIME), with a delay or not according if it is a short or long reboot
 *
 */
void RebootCounterManager::stageStart()
{
    stage = stateReboots_t::ON_TIME;
    nextStageTime = CONSECUTIVE_REBOOTS[static_cast<uint8_t>(std::floor(nbReboots * PADDING_2ND_COLUMN))];
    ESP_LOGE(TAG, "stageStart--nbReboots %d---nextStageTime : %d", nbReboots, nextStageTime);
    if (nextStageTime > 0) {
        resetCounterInFlash();
        setDelay();
    }
}

/**
 * @brief ON time stage of the reboot
 * Increment reboot counter, and go to next stage with a delay
 *
 */
void RebootCounterManager::stageOnTime()
{
    nextStageTime = CONSECUTIVE_REBOOTS[static_cast<uint8_t>(std::floor(nbReboots * PADDING_2ND_COLUMN) + 1)];
    ESP_LOGE(TAG, "stageOnTime--nbReboots %d---nextStageTime : %d", nbReboots, nextStageTime);
    if (nextStageTime < COUNTER_ON_TIME_POWER_CUT) {
        stage = stateReboots_t::OFF_TIME;
    } else {
        stage = stateReboots_t::ON_TIME_POWER_CUT;
        nextStageTime = COUNTER_ON_TIME_POWER_CUT;
    }
    incrementCounter();
    setDelay();
}

/**
 * @brief OFF time stage of the reboot
 * Trigger an action if necessary, or reset the counter ; then go to the next stage with a delay, or end the task
 *
 */
void RebootCounterManager::stageOffTime()
{
    ESP_LOGE(TAG, "stageOffTime--nbReboots %d---nextStageTime : %d", nbReboots, nextStageTime);
    triggerAction();
    if (currentTime < COUNTER_ON_TIME_POWER_CUT) {
        stage = stateReboots_t::ON_TIME_POWER_CUT;
        nextStageTime = COUNTER_ON_TIME_POWER_CUT;
        setDelay();
    } else {
        stage = stateReboots_t::FINISH;
    }
}

/**
 * @brief Finish stage of the reboot
 * Delete the task
 */
void RebootCounterManager::stageFinish() const
{
    ESP_LOGE(TAG, "stageFinish--nbReboots %d---nextStageTime : %d", nbReboots, nextStageTime);
    ESP_LOGE(TAG, "Remove this task as it is not necessary anymore (counter is reseted)");
    vTaskDelete(nullptr);
}

/**
 * @brief Increment reboot counter, then store it in flash
 *
 */
void RebootCounterManager::incrementCounter()
{
    QueueHandle_t flashQueue = API::Queue::getInstance()->getFlashQueue();
    nbReboots++;
    API::attributeData_t data = { API::ATTRIBUTE_ID::REBOOT_COUNTER, nbReboots };
    ESP_LOGI(TAG, "Increment reboot counter to %d", nbReboots);
    INFORM_SEND_FAIL(xQueueSend(flashQueue, &data, API::AWOX_MAX_POSTING_DELAY))
}

/**
 * @brief Reset reboot counter to 0 in flash
 *
 */
void RebootCounterManager::resetCounterInFlash()
{
    ESP_LOGI(TAG, "Write reboot counter = 0 in flash, not changed in RAM");
    QueueHandle_t flashQueue = API::Queue::getInstance()->getFlashQueue();
    API::attributeData_t data = { API::ATTRIBUTE_ID::REBOOT_COUNTER, 0 };
    INFORM_SEND_FAIL(xQueueSend(flashQueue, &data, API::AWOX_MAX_POSTING_DELAY))
}

/**
 * @brief Set a delay for this task according to current time and next stage time
 *
 */
void RebootCounterManager::setDelay() const
{
    ESP_LOGD(TAG, "Next stage is %d, at %ds", static_cast<uint8_t>(stage), nextStageTime);
    ESP_LOGE(TAG, "Next stage is %d, at %ds------current time: %d", static_cast<uint8_t>(stage), nextStageTime, currentTime);
    // vTaskDelay(pdMS_TO_TICKS((nextStageTime - currentTime) * 1000));
    vTaskDelay(pdMS_TO_TICKS(5 * 1000));
}

/**
 * @brief Trigger an action if necessary (factory reset, cycling/aging mode), and reset the counter
 *
 */
void RebootCounterManager::triggerAction()
{
    ESP_LOGI(TAG, "Trigger action");
    // if (nbReboots == NB_REBOOTS_USER_MODE && _inProductionMode) {
    //     ESP_LOGI(TAG, "Trigger user mode from consecutive reboots");
    //     auto* queue = Api::getInstance()->getFlashQueue();
    //     auto data = attributeData_t();
    //     data.id = ATTRIBUTE_ID::ACTIVATE_USER_MODE_ID;
    //     INFORM_SEND_FAIL(xQueueSend(queue, &data, AWOX_MAX_POSTING_DELAY))
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    //     esp_restart();
    // }
    if (nbReboots == NB_REBOOTS_FACTORY_RESET) {
        ESP_LOGI(TAG, "Trigger factory reset from consecutive reboots");
        // SEND FACTORY RESET
        QueueHandle_t attrQueue = API::Queue::getInstance()->getFlashQueue();
        auto data = API::attributeData_t();
        data.id = API::ATTRIBUTE_ID::TOUCHLINK_FACTORY_RESET;
        INFORM_SEND_FAIL(xQueueSend(attrQueue, &data, API::AWOX_MAX_POSTING_DELAY));
    }
    // else if (nbReboots == NB_REBOOTS_CYCLING_MODE) {
    //     ESP_LOGI(TAG, "Trigger cycling mode from consecutive reboots");
    //     QueueHandle_t attrQueue = Api::getInstance()->getAttrQueue();
    //     auto data = attributeData_t();
    //     data.id = ATTRIBUTE_ID::SPECIAL_MODE_ATTRIBUTE_ID;
    //     data.value = (uint64_t)SPECIAL_MODE::CYCLING;
    //     INFORM_SEND_FAIL(xQueueSend(attrQueue, &data, AWOX_MAX_POSTING_DELAY))
    // } else if (nbReboots == NB_REBOOTS_AGING_MODE) {
    //     ESP_LOGI(TAG, "Trigger aging mode from consecutive reboots");
    //     QueueHandle_t attrQueue = Api::getInstance()->getAttrQueue();
    //     auto data = attributeData_t();
    //     data.id = ATTRIBUTE_ID::SPECIAL_MODE_ATTRIBUTE_ID;
    //     data.value = (uint64_t)SPECIAL_MODE::AGING;
    //     INFORM_SEND_FAIL(xQueueSend(attrQueue, &data, AWOX_MAX_POSTING_DELAY))
    // } else if (nbReboots >= NB_REBOOTS_SUPER_RESET) {
    //     ESP_LOGI(TAG, "Trigger super factory reset from consecutive reboots");
    //     auto sender = CommandRequestor(SRC_TYPE_REBOOTS);
    //     sender.requestSuperFactoryReset();
    // }
    else {
        /* No action triggered yet on other reboot counts */
    }
    nbReboots = 0;
    resetCounterInFlash();
}

/**
 * @brief Set the reboot counter manager in production mode.
 *
 */
void RebootCounterManager::setInproductionMode()
{
    ESP_LOGI(TAG, "Set in production mode");
    _inProductionMode = true;
}
}