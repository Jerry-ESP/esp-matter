#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#pragma once

/**
 * @brief This function is an overlay of the FreeRTOS function xQueueSend in order to inform the developer if the message has been sent or not.
 *
 */
#define INFORM_SEND_FAIL(f)                                                                                                \
    if (f != pdTRUE) {                                                                                                     \
        ESP_LOGW("QUEUE", "Could not send message from line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__); \
    }

namespace API {
constexpr uint16_t AWOX_MAX_POSTING_DELAY = 100; /**< Maximum delay authorized for a task to block for posting a message when a task is full */

/**@{*/
/** @brief List of all attributes used/implemented in the project.
 *  @note Each define is composed of 0x[ClusterID]'[AttributeID].
 */
enum class ATTRIBUTE_ID : uint32_t {
    TOUCHLINK_FACTORY_RESET = 0x1000'0007, /**< factory reset from touchlink cluster command*/
    REBOOT_COUNTER = 0x7777'2000, /**< Reboot counter*/
    AWOX_CUSTOM_GENERIC_RESPONSE = 0x7777'3000, /**< Awox generic response command*/
    FLASH_BLE_PAIRING_INFO = 0x8888'7776, /**< command to update in flash the BLE pairing info*/
};
/**@}*/

/**
 * @struct attributeData_t
 * @brief set of argument necessary for raw command by targeting id and provides values within.
 */
struct attributeData_t {
    ATTRIBUTE_ID id; /**< Indicate the command id used*/
    uint64_t value; /**< Store the data related to the command id*/
};

class Queue {
public:
    /**
     * @brief Delete the constructor by copy for the "Singleton"
     *
     * @param obj
     */
    Queue(const Queue& obj) = delete;
    static Queue* getInstance();
    QueueHandle_t getFlashQueue();

private:
    /**@{*/
    /** @brief Queues to pass messages between the managers */
    QueueHandle_t flashQueue; /**< Queue for Storage Manager */
    /**@}*/
    Queue();
};

}