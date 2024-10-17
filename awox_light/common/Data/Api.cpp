#include "Api.hpp"
#include "BluetoothData.hpp"

constexpr uint8_t STORAGE_API_QUEUE_SIZE = 10;

namespace API {

Queue* Queue::getInstance()
{
    static Queue instance;
    return &instance;
}

Queue::Queue()
{
    // The variable used to hold the queue's data structure.
    static StaticQueue_t staticFlashQueue; /**< @brief Static queue for flash operations */

    // The array to use as the queue's storage area.
    static uint8_t queueStorageArea[STORAGE_API_QUEUE_SIZE * sizeof(attributeData_t)]; /**< @brief Storage area for flash */

    // Create the queues.
    flashQueue = xQueueCreateStatic(STORAGE_API_QUEUE_SIZE, sizeof(attributeData_t),
        queueStorageArea, &staticFlashQueue);
}

/**
 * @brief Implement the getter for the Flash Queue
 *
 * @return QueueHandle_t
 */
QueueHandle_t Queue::getFlashQueue()
{
    return flashQueue;
}

} // namespace API