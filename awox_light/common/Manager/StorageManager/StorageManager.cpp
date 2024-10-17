/**
 * @file StorageManager.cpp
 * @author AWOX
 * @brief Flash access manager.
 *
 */
#include "StorageManager.hpp"
#include "BluetoothData.hpp"
#include "TargetConfig.hpp"
#include "TaskConfig.hpp"
#include <algorithm>
#include <set>

/**
 * @brief Storage Namespace
 *
 */
namespace Storage {

constexpr static const char* TAG = "awox_storage_manager"; /**< @brief Espressif tag for Log */

/** We need this function because the task create requires a static task and that cause problem with cpp */
void Manager::coordinatorWrapperTask(void* pvParameters)
{
    ((Manager*)pvParameters)->managerTask();
}

/**
 * @brief  Manager task creation
 *
 * @param handle the task handle object
 * @return BaseType_t the task creation result
 */
BaseType_t Manager::createManagerTask(TaskHandle_t* handle)
{
    BaseType_t baseType = xTaskCreate(Manager::coordinatorWrapperTask, storageTask.name, storageTask.size, this, storageTask.priority, handle);
    _taskHandle = *handle;
    return baseType;
}

/**
 * @brief Construct a new Storage Manager
 *
 */
Manager::Manager()
{
    _flashQueue = Queue::getInstance()->getFlashQueue();
    _forcedAttributes = { ATTRIBUTE_ID::FLASH_BLE_PAIRING_INFO,
        ATTRIBUTE_ID::REBOOT_COUNTER };
}

/**
 * @brief Read the flash storage, write initial values if it's empty and send it to the LightDriver
 *
 */
void Manager::readFlash()
{
    if (_flashDriver.isErased()) {
        ESP_LOGI(TAG, "MEMORY WAS ERASED: SETTING IT TO INITIAL VALUES");
        /* Store BLE pairing information with default values */
        _flashDriver.writeBlePairingInfo();
        _flashDriver.writeData({ API::ATTRIBUTE_ID::REBOOT_COUNTER, 0 });
        _flashDriver.unsetErase();
    } else {
        ESP_LOGI(TAG, "SETTING INITIAL VALUES ");
        /* Setting Ble pairing information */
        _flashDriver.readBlePairingInfo();
    }
    /* Unblock Ble Gap Manager */
    xSemaphoreGive(BasicBle::getInstance()->getSemaphoreFlashReadDone());
}

/**
 * @brief Main task (loop)
 *
 */
void Manager::managerTask()
{
    attributeData_t attribute;
    while (true) {
        xQueueReceive(_flashQueue, &attribute, portMAX_DELAY);
        _flashDriver.writeData(attribute);
    }
}

} // namespace Storage