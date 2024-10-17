/**
 * @file ModuleVersion.hpp
 * @author AWOX
 * @brief Information retrieved from Product Database about the hardware of the device
 */

#pragma once
#include "sdkconfig.h"
#include <cstdint>

/**
 * @brief Namespace for GPIOS
 *
 */
namespace ModuleGpio {
/**@{*/
/** Define gpios for different modules and applications*/

/*I2C GPIO*/
#if defined(CONFIG_CAPABILITY_I2C)
#if defined(CONFIG_MODULE_VERSION_WROOM_07) || (CONFIG_MODULE_VERSION_COB_GENERIC_RGBTW) || (CONFIG_MODULE_VERSION_COB_GENERIC_RGBW) || (CONFIG_MODULE_VERSION_DEVKIT)
constexpr uint8_t GPIO_I2C_CLK = 22;
constexpr uint8_t GPIO_I2C_DATA = 10;
#else
"error: I2C GPIO not defined"
#endif
#endif

/*PWM + controllers GPIO*/
#if defined(CONFIG_CAPABILITY_SWITCH)
#if !defined(CONFIG_CAPABILITY_I2C)
#if defined(CONFIG_MODULE_VERSION_MINI_1) || (CONFIG_MODULE_VERSION_MINI_1_CONTROLLER) || (CONFIG_MODULE_VERSION_COB_GENERIC_TW) || (CONFIG_MODULE_VERSION_DEVKIT)

constexpr uint8_t GPIO_RED_CHANNEL = 14;
constexpr uint8_t GPIO_GREEN_CHANNEL = 12;
constexpr uint8_t GPIO_BLUE_CHANNEL = 4;
constexpr uint8_t GPIO_WARM_CHANNEL = 11;
constexpr uint8_t GPIO_COLD_CHANNEL = 10;

#if defined(CONFIG_CAPABILITY_CONTROLLER_PLUG_IN)
/** Define GPIO for Controller */
constexpr uint8_t GPIO_CONTROLLER_BUTTON_0 = 22;
constexpr uint8_t GPIO_CONTROLLER_BUTTON_1 = 27;
constexpr uint8_t GPIO_CONTROLLER_BUTTON_2 = 13;
constexpr uint8_t GPIO_CONTROLLER_DETECTION = 25;
#endif // controller

#elif defined(CONFIG_MODULE_VERSION_WROOM_07)
constexpr uint8_t GPIO_WARM_CHANNEL = 10;
constexpr uint8_t GPIO_COLD_CHANNEL = 22;

#else
#error "A correct module version must be chosen for pwms/ controllers"
#endif
#endif
#endif

/*Remotes GPIO*/
#if defined(CONFIG_TARGET_WS) || (CONFIG_CAPABILITY_RCUGROUPS) || (CONFIG_CAPABILITY_RCU)
// todo correct modules and gpios for rcu rcu3groups and ws
#if defined(CONFIG_MODULE_VERSION_MINI_1) || (CONFIG_MODULE_VERSION_DEVKIT)

/** Define GPIO for Wall Switch */
constexpr uint8_t GPIO_WALL_SWITCH_BUTTON_ON_OFF = 14;
constexpr uint8_t GPIO_WALL_SWITCH_BUTTON_LEFT = 12;
constexpr uint8_t GPIO_WALL_SWITCH_BUTTON_RIGHT = 4;
constexpr uint8_t GPIO_WALL_SWITCH_BUTTON_RESET = 10;
constexpr uint8_t GPIO_WALL_SWITCH_LED_GPIO = 26;

/** Define GPIO for RCU 3 groups */
constexpr uint8_t GPIO_COL_0 = 13;
constexpr uint8_t GPIO_COL_1 = 1;
constexpr uint8_t GPIO_COL_2 = 0;
constexpr uint8_t GPIO_COL_3 = 3;
constexpr uint8_t GPIO_COL_4 = 2;
constexpr uint8_t GPIO_ROW_0 = 22;
constexpr uint8_t GPIO_ROW_1 = 25;
constexpr uint8_t GPIO_ROW_2 = 27;
constexpr uint8_t LED_GPIO = 26;

/** Define GPIO channels for RCU buttons attribution */
constexpr uint8_t GPIO_BUTTON_0 = 11;
constexpr uint8_t GPIO_BUTTON_1 = 25;
constexpr uint8_t GPIO_BUTTON_2 = 12;
constexpr uint8_t GPIO_BUTTON_3 = 8;
constexpr uint8_t GPIO_BUTTON_4 = 22;
constexpr uint8_t GPIO_BUTTON_5 = 9;

#else
#error "A correct module must be chosen for remotes"
#endif
#endif
/**@}*/
}

#ifndef CONFIG_HARDWARE_VERSION_MAJOR
#error "Requires a major number for HW version"
#endif
#define HARDWARE_VERSION_MAJOR CONFIG_HARDWARE_VERSION_MAJOR /**< @brief Major number of HW version */

#ifndef CONFIG_HARDWARE_VERSION_MINOR
#error "Requires a minor number for HW version"
#endif
#define HARDWARE_VERSION_MINOR CONFIG_HARDWARE_VERSION_MINOR /**< @brief Minor number of HW version */
