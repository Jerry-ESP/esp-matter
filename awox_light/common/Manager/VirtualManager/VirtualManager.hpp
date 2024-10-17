/**
 * @file VirtualManager.hpp
 * @author AWOX
 * @brief Class Virtual Manager, managers inherit this class
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#pragma once

/**
 * @brief Virtual manager with only virtual functions
 *
 */
class VirtualManager {
public:
    // Members

    // Functions
    virtual TaskHandle_t getTaskHandle() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual BaseType_t createManagerTask(TaskHandle_t* handle) = 0; /**< Virtual function, need to be implemented in daughter classes */

protected:
    // Members
    TaskHandle_t _taskHandle; /**< Every Manager has a task running */

    // Functions
    virtual void managerTask() = 0; /**< Virtual function, need to be implemented in daughter classes */
    virtual ~VirtualManager() = default; /**< Virtual destructor*/
private:
    // Members

    // Functions
};