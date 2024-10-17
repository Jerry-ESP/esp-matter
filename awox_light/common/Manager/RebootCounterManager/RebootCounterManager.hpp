/**
 * @file RebootCounterManager.hpp
 * @author AWOX
 * @brief Handle the counter of electrical reboots and its following uses
 *
 */

#include "VirtualManager.hpp"
#pragma once

namespace RebootCounterManager {

constexpr static const uint8_t COUNTER_ON_TIME_SHORT = 0; /**< ON time to increment counter on short reboots */
constexpr static const uint8_t COUNTER_OFF_TIME_SHORT = 6; /**< OFF time to reset counter on short reboots */
constexpr static const uint8_t COUNTER_ON_TIME_LONG = 6; /**< ON time to increment counter on long reboots */
constexpr static const uint8_t COUNTER_OFF_TIME_LONG = 30; /**< OFF time to reset counter on short reboots */
constexpr static const uint8_t COUNTER_ON_TIME_POWER_CUT = 12; /**< ON time to start power cut reboot feature */
constexpr static const uint8_t COUNTER_OFF_TIME_POWER_CUT = 18; /**< OFF time to stop power cut reboot feature */

// constexpr static const uint8_t NB_REBOOTS_USER_MODE = 3; /**< Number of consecutive reboots to switch from production to user mode */
constexpr static const uint8_t NB_REBOOTS_FACTORY_RESET = 6; /**< Number of consecutive reboots to trigger factory reset */
// constexpr static const uint8_t NB_REBOOTS_CYCLING_MODE = 8; /**< Number of consecutive reboots to trigger cycling mode */
// constexpr static const uint8_t NB_REBOOTS_AGING_MODE = 10; /**< Number of consecutive reboots to trigger aging mode */
// constexpr static const uint8_t NB_REBOOTS_SUPER_RESET = 12; /**< Number of consecutive reboots to trigger super reset mode */
// constexpr static const uint8_t MAX_REBOOTS = NB_REBOOTS_SUPER_RESET; /**< Max number of reboots to trigger an action */
constexpr static const uint8_t MAX_REBOOTS = NB_REBOOTS_FACTORY_RESET; /**< Max number of reboots to trigger an action */

constexpr static const uint8_t PADDING_2ND_COLUMN = 2; /**< Padding to 2nd column of CONSECUTIVE_REBOOTS */

// clang-format off
/** @brief Table of counter of consecutive reboots. In raw are the consecutive reboots,
 * in 1st column, the time (in s) to increment the counter, in 2nd colum the time to reset the counter
 */
constexpr static const uint8_t CONSECUTIVE_REBOOTS[MAX_REBOOTS * PADDING_2ND_COLUMN] = { COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,
                                                                        COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,
                                                                        COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,
                                                                        COUNTER_ON_TIME_SHORT,       COUNTER_OFF_TIME_LONG,      // --> Go to user mode if in production mode
                                                                        COUNTER_ON_TIME_SHORT,       COUNTER_OFF_TIME_LONG,
                                                                        COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT};      // --> Factory reset
                                                                        // COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,
                                                                        // COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,      // --> Cycling mode
                                                                        // COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,
                                                                        // COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,     // --> Aging mode
                                                                        // COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT,
                                                                        // COUNTER_ON_TIME_SHORT,      COUNTER_OFF_TIME_SHORT };   // --> Super Factory Reset (SFR)
// clang-format on

/** @brief Stages for the reboot counter */
enum class stateReboots_t {
    START, /**< Start stage */
    ON_TIME, /**< ON time stage */
    OFF_TIME, /**< OFF time stage */
    ON_TIME_POWER_CUT, /**< ON time for power cut stage */
    OFF_TIME_POWER_CUT, /**< OFF time for power cut stage */
    FINISH /**< End stage */
};

/**
 * @brief Class used to cout number of consecutive reboots, and trigger associated features
 *
 */
class RebootCounterManager : public VirtualManager {
public:
    RebootCounterManager(const RebootCounterManager&) = delete;
    RebootCounterManager(uint8_t resets);
    BaseType_t createManagerTask(TaskHandle_t* handle) override;
    TaskHandle_t getTaskHandle() override;
    void setInproductionMode();

private:
    bool _inProductionMode = false; /**< Flag to know if the device is in production mode */
    uint8_t nbReboots = 0; /**< Number of consecutive reboots */
    stateReboots_t stage = stateReboots_t::START; /**< Current reboot stage */
    uint8_t currentTime = 0; /**< Current time (in s) */
    uint8_t nextStageTime = 0; /**< Next stage time (in s) */

    static void wrapperTask(void* pvParameters);
    void managerTask() override;
    void stageStart();
    void stageOnTime();
    void stageOffTime();
    void stageFinish() const;
    void incrementCounter();
    void resetCounterInFlash();
    void setDelay() const;
    void triggerAction();
};

}