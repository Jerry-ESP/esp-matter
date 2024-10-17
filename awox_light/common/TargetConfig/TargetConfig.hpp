/**
 * @file TargetConfig.hpp
 * @author AWOX
 * @brief Information retrieved from Product Database and Gitlad (for FW version) about the device (device identification, capabilities...)
 */

#pragma once

// Include the module version from here to include only this file from everywhere
#include "ModuleVersion.hpp"

#ifndef CONFIG_SKIP_PRODUCTION
#define SKIP_PRODUCTION false /**< @brief Not skip production mode */
#else
#define SKIP_PRODUCTION CONFIG_SKIP_PRODUCTION /**< @brief Skip production mode from sdkconfig */
#endif

#ifndef CONFIG_USE_REGISTERED_MAC
#define USE_REGISTERED_MAC false /**< @brief No use a registered MAC address from sdkconfig */
#else
#define USE_REGISTERED_MAC CONFIG_USE_REGISTERED_MAC /**< @brief Use a registered MAC address from sdkconfig */
#endif

// Check that the compiled target has a model name and recast into shorter name ("CONFIG" automatically added by Espressif menuconfig)
#ifndef CONFIG_MODEL_NAME
#error "Requires a model name"
#endif
#define MODEL_NAME CONFIG_MODEL_NAME /**< @brief Model name */

// Check that the compiled target has a product id and recast into shorter name ("CONFIG" automatically added by Espressif menuconfig)
#ifndef CONFIG_PRODUCT_ID
#error "Requires a product ID"
#endif
#define PRODUCT_ID CONFIG_PRODUCT_ID /**< @brief Product ID */

#ifdef CONFIG_EMULATE_TELINK_ADDR
#define USE_TELINK_ADDR CONFIG_EMULATE_TELINK_ADDR
#endif

// Check that the compiled target has a Firmware version and recast into shorter name ("CONFIG" automatically added by Espressif menuconfig)
#ifndef CONFIG_FIRMWARE_VERSION
#error "Requires a FW version"
#endif
#define FIRMWARE_VERSION CONFIG_FIRMWARE_VERSION /**< @brief Full FW version */

#ifndef CONFIG_FIRMWARE_VERSION_MAJOR
#error "Requires a major number for FW version"
#endif
#define FIRMWARE_VERSION_MAJOR CONFIG_FIRMWARE_VERSION_MAJOR /**< @brief Major number of FW version */

#ifndef CONFIG_FIRMWARE_VERSION_MINOR
#error "Requires a minor number for FW version"
#endif
#define FIRMWARE_VERSION_MINOR CONFIG_FIRMWARE_VERSION_MINOR /**< @brief Minor number of FW version */

#ifndef CONFIG_FIRMWARE_VERSION_PATCH
#error "Requires a patch number for FW version"
#endif
#define FIRMWARE_VERSION_PATCH CONFIG_FIRMWARE_VERSION_PATCH /**< @brief Patch number of FW version */

#ifndef CONFIG_FIRMWARE_VERSION_BUILD
#error "Requires a build number for FW version"
#endif
#define FIRMWARE_VERSION_BUILD CONFIG_FIRMWARE_VERSION_BUILD /**< @brief Build number of FW version */

// Check that the compiled target has min and max mired temperatures and recast into shorter name ("CONFIG" automatically added by Espressif menuconfig)
#ifndef CONFIG_MIN_TEMPERATURE_MIREDS
#error "Requires a min temperature"
#endif
#define MIN_TEMPERATURE_MIREDS CONFIG_MIN_TEMPERATURE_MIREDS /**< @brief Minimum value for mireds color temperature */

#ifndef CONFIG_MAX_TEMPERATURE_MIREDS
#error "Requires a max temperature"
#endif
#define MAX_TEMPERATURE_MIREDS CONFIG_MAX_TEMPERATURE_MIREDS /**< @brief Maximum value for mireds color temperature */

// Check that the compiled target has PWM configuration and recast into shorter name ("CONFIG" automatically added by Espressif menuconfig)
#ifndef CONFIG_PWM_FREQUENCY
#error "Requires a PWM frequency"
#endif
#define PWM_FREQUENCY CONFIG_PWM_FREQUENCY /**< @brief PWM frequency for the LEDs */

/**@{*/
/** Minimum PWM required for the LEDs */
#ifndef CONFIG_MIN_PWM_CW
#error "Requires a minimum value for cold white PWM"
#endif
#define MIN_PWM_CW CONFIG_MIN_PWM_CW

#ifndef CONFIG_MIN_PWM_WW
#error "Requires a minimum value for warm white PWM"
#endif
#define MIN_PWM_WW CONFIG_MIN_PWM_WW

#ifndef CONFIG_MIN_PWM_R
#error "Requires a minimum value for red PWM"
#endif
#define MIN_PWM_R CONFIG_MIN_PWM_R

#ifndef CONFIG_MIN_PWM_G
#error "Requires a minimum value for green PWM"
#endif
#define MIN_PWM_G CONFIG_MIN_PWM_G

#ifndef CONFIG_MIN_PWM_B
#error "Requires a minimum value for blue PWM"
#endif
#define MIN_PWM_B CONFIG_MIN_PWM_B
/**@}*/

/**@{*/
/** Indicate if PWM is inverted */
#ifdef CONFIG_INVERTED_PWM_CW
#define INVERTED_PWM_CW true
#else
#define INVERTED_PWM_CW false
#endif

#ifdef CONFIG_INVERTED_PWM_WW
#define INVERTED_PWM_WW true
#else
#define INVERTED_PWM_WW false
#endif

#ifdef CONFIG_INVERTED_PWM_R
#define INVERTED_PWM_R true
#else
#define INVERTED_PWM_R false
#endif

#ifdef CONFIG_INVERTED_PWM_G
#define INVERTED_PWM_G true
#else
#define INVERTED_PWM_G false
#endif

#ifdef CONFIG_INVERTED_PWM_B
#define INVERTED_PWM_B true
#else
#define INVERTED_PWM_B false
#endif
/**@}*/

// Recast capabilities into shorter name ("CONFIG" automatically added by Espressif menuconfig)
/**@{*/
/** Define capabilities for the target */
#ifdef CONFIG_CAPABILITY_SWITCH
#define CAPABILITY_SWITCH true
#else
#define CAPABILITY_SWITCH false
#endif

#ifdef CONFIG_CAPABILITY_WHITE
#define CAPABILITY_WHITE true
#else
#define CAPABILITY_WHITE false
#endif

#ifdef CONFIG_CAPABILITY_DIMMING
#define CAPABILITY_DIMMING true
#else
#define CAPABILITY_DIMMING false
#endif

#ifdef CONFIG_CAPABILITY_TEMPERATURE
#define CAPABILITY_TEMPERATURE true
#else
#define CAPABILITY_TEMPERATURE false
#endif

#ifdef CONFIG_CAPABILITY_COLOR
#define CAPABILITY_RGB_COLOR true
#else
#define CAPABILITY_RGB_COLOR false
#endif

#ifdef CONFIG_CAPABILITY_CONTROLLER_PLUG_IN
#define CAPABILITY_CONTROLLER_PLUG_IN true
#else
#define CAPABILITY_CONTROLLER_PLUG_IN false
#endif

#ifdef CONFIG_CAPABILITY_RCUGROUPS
#define CAPABILITY_RCUGROUPS true
#else
#define CAPABILITY_RCUGROUPS false
#endif

#ifdef CONFIG_CAPABILITY_RCU
#define CAPABILITY_RCU true
#else
#define CAPABILITY_RCU false
#endif

#ifdef CONFIG_CAPABILITY_MOTION
#define CAPABILITY_MOTION true
#else
#define CAPABILITY_MOTION false
#endif

#ifdef CONFIG_CAPABILITY_BACKLIGHT
#define CAPABILITY_BACKLIGHT true
#else
#define CAPABILITY_BACKLIGHT false
#endif
/**@}*/
