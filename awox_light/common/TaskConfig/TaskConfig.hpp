/**
 * @file TaskConfig.hpp
 * @author AWOX
 * @brief Centralize information about the task created in the project
 *
 */

#include <array>
#pragma once

constexpr uint32_t defaultSizeTask = 4096; /**< size Task most used. If not specified, provide these parameters =*/

/**@{*/
/** @brief Reebot Tasks parameters */
constexpr static const char* rebootCounterTaskName = "REBOOT_COUNTER_MANAGER_TASK";
constexpr UBaseType_t rebootCounterPriority = 3;
/**@}*/

/**@{*/
/** @brief Storage Tasks parameters */
constexpr static const char* storageManagerTaskName = "STORAGE_MANAGER_TASK";
constexpr UBaseType_t storageManagerPriority = 3;
/**@}*/

/**@{*/
/** @brief BLE Tasks parameters */
constexpr static const char* bleManagerTaskName = "BLE_MANAGER_TASK";
constexpr UBaseType_t bleManagerPriority = 5;
/**@}*/

/**@{*/
/** @brief struct of parameters asked for a task */
struct __attribute__((packed)) taskDataParameters {
    const char* name; /**< Name of task*/
    uint32_t size; /**< Size of the task*/
    UBaseType_t priority; /**< Priority provided to the task*/
};
/**@}*/

/**@{*/
/** @brief Task parameters variables */
const struct taskDataParameters rebootTask = { rebootCounterTaskName, defaultSizeTask, rebootCounterPriority };
const struct taskDataParameters storageTask = { storageManagerTaskName, defaultSizeTask, storageManagerPriority };
const struct taskDataParameters bleManagerTask = { bleManagerTaskName, defaultSizeTask, bleManagerPriority };
/**@}*/
