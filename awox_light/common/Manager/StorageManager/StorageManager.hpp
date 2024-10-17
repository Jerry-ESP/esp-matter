/**
 * @file StorageManager.hpp
 * @author AWOX
 * @brief Flash access manager.
 *
 */
#include "Api.hpp"
#include "FlashDriver.hpp"
#include "VirtualManager.hpp"
#include <list>
#pragma once

/**@{*/
/** @brief Default values when factory reset */
#define ON_OFF_DEFAULT_VALUE          1
#define START_UP_ON_OFF_DEFAULT_VALUE 1
#define LEVEL_DEFAULT_VALUE           254
#define SATURATION_DEFAULT_VALUE      254
#define TEMPERATURE_DEFAULT_VALUE     333
#define HUE_DEFAULT_VALUE             0xAA

constexpr uint16_t MIREDS_START_UP_VALUE = 0x0;
/**@}*/

using namespace API;

namespace Storage {
/**
 * @brief Manager to Storage the different data.
 *
 */
class Manager : public VirtualManager {
public:
    Manager();
    Manager(const Manager& obj) = delete; /**<Delete the constructor by copy for the "Singleton"*/
    BaseType_t createManagerTask(TaskHandle_t* handle);
    TaskHandle_t getTaskHandle() { return _taskHandle; }
    void readFlash();
    uint8_t getRebootCounter() const { return _flashDriver.getRebootCounter(); }

private:
    QueueHandle_t _flashQueue; /**< Flash queue (income)*/
    Flash::Driver _flashDriver = Flash::Driver(); /**< Flash driver*/
    std::list<ATTRIBUTE_ID> _forcedAttributes; /**< Attributes stored in flash not affected by power cut (so everything except bulb state) */
    static void coordinatorWrapperTask(void* pvParameters);
    void managerTask();
};

}