/**
 * @file I2CDriverConfig.hpp
 * @author AWOX
 * @brief Configuration file for the I2C driver
 */
#pragma once

#include "TargetConfig.hpp"
#include "lightbulb.h"

/**
 * @brief Macro to safe check if a capability is enabled, will skip the function if not
 *
 */
#define CHECK_CAPABILITY(CAPABILITY) \
    do {                             \
        if (!(CAPABILITY)) {         \
            return;                  \
        }                            \
    } while (0)

#if CONFIG_CAPABILITY_COLOR
/* AUTO GENERATED COLOR TABLE RGBIC/Tools/I2CGenerator/script.py */
constexpr uint8_t COLOR_DATA_SIZE = 24;
lightbulb_color_mapping_data_t color_data[COLOR_DATA_SIZE] = {
    { .rgbcw_100 = { 1.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.5010f, 0.2495f, 0.2495f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3846f, 0.3077f, 0.3077f, 0.0000f, 0.0000f }, .hue = 0   },
    { .rgbcw_100 = { 0.7244f, 0.2756f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.4570f, 0.3154f, 0.2276f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3734f, 0.3280f, 0.2987f, 0.0000f, 0.0000f }, .hue = 22  },
    { .rgbcw_100 = { 0.5667f, 0.4333f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.4201f, 0.3707f, 0.2092f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3632f, 0.3462f, 0.2906f, 0.0000f, 0.0000f }, .hue = 45  },
    { .rgbcw_100 = { 0.4586f, 0.5414f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.3809f, 0.4133f, 0.2058f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3499f, 0.3612f, 0.2890f, 0.0000f, 0.0000f }, .hue = 68  },
    { .rgbcw_100 = { 0.3254f, 0.6746f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.3310f, 0.4466f, 0.2224f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3328f, 0.3706f, 0.2965f, 0.0000f, 0.0000f }, .hue = 90  },
    { .rgbcw_100 = { 0.0893f, 0.9107f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2682f, 0.4885f, 0.2433f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3129f, 0.3817f, 0.3054f, 0.0000f, 0.0000f }, .hue = 113 },
    { .rgbcw_100 = { 0.0000f, 1.0000f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2495f, 0.5010f, 0.2495f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3077f, 0.3846f, 0.3077f, 0.0000f, 0.0000f }, .hue = 119 },
    { .rgbcw_100 = { 0.0000f, 0.7798f, 0.2202f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2330f, 0.4679f, 0.2991f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3009f, 0.3761f, 0.3230f, 0.0000f, 0.0000f }, .hue = 136 },
    { .rgbcw_100 = { 0.0000f, 0.6818f, 0.3182f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2232f, 0.4482f, 0.3286f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.2969f, 0.3712f, 0.3319f, 0.0000f, 0.0000f }, .hue = 147 },
    { .rgbcw_100 = { 0.0000f, 0.6071f, 0.3929f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2145f, 0.4307f, 0.3547f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.2931f, 0.3664f, 0.3405f, 0.0000f, 0.0000f }, .hue = 158 },
    { .rgbcw_100 = { 0.0000f, 0.5183f, 0.4817f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2022f, 0.4061f, 0.3917f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.2873f, 0.3592f, 0.3535f, 0.0000f, 0.0000f }, .hue = 175 },
    { .rgbcw_100 = { 0.0000f, 0.4910f, 0.5090f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2009f, 0.3956f, 0.4035f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.2865f, 0.3553f, 0.3581f, 0.0000f, 0.0000f }, .hue = 181 },
    { .rgbcw_100 = { 0.0000f, 0.1774f, 0.8226f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2365f, 0.2886f, 0.4749f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3027f, 0.3190f, 0.3783f, 0.0000f, 0.0000f }, .hue = 226 },
    { .rgbcw_100 = { 0.0000f, 0.0000f, 1.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2495f, 0.2495f, 0.5010f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3077f, 0.3077f, 0.3846f, 0.0000f, 0.0000f }, .hue = 239 },
    { .rgbcw_100 = { 0.1021f, 0.0000f, 0.8979f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2710f, 0.2424f, 0.4866f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3139f, 0.3049f, 0.3812f, 0.0000f, 0.0000f }, .hue = 246 },
    { .rgbcw_100 = { 0.1414f, 0.0000f, 0.8586f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.2792f, 0.2396f, 0.4811f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3170f, 0.3036f, 0.3795f, 0.0000f, 0.0000f }, .hue = 249 },
    { .rgbcw_100 = { 0.3544f, 0.0000f, 0.6456f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.3402f, 0.2193f, 0.4404f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3357f, 0.2952f, 0.3690f, 0.0000f, 0.0000f }, .hue = 272 },
    { .rgbcw_100 = { 0.4231f, 0.0000f, 0.5769f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.3665f, 0.2106f, 0.4229f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3443f, 0.2914f, 0.3643f, 0.0000f, 0.0000f }, .hue = 283 },
    { .rgbcw_100 = { 0.4540f, 0.0000f, 0.5460f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.3789f, 0.2065f, 0.4146f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3489f, 0.2894f, 0.3617f, 0.0000f, 0.0000f }, .hue = 289 },
    { .rgbcw_100 = { 0.4775f, 0.0000f, 0.5225f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.3898f, 0.2029f, 0.4073f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3526f, 0.2877f, 0.3597f, 0.0000f, 0.0000f }, .hue = 294 },
    { .rgbcw_100 = { 0.5889f, 0.0000f, 0.4111f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.4264f, 0.2124f, 0.3612f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3653f, 0.2923f, 0.3424f, 0.0000f, 0.0000f }, .hue = 317 },
    { .rgbcw_100 = { 0.7612f, 0.0000f, 0.2388f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.4645f, 0.2313f, 0.3042f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3756f, 0.3004f, 0.3240f, 0.0000f, 0.0000f }, .hue = 340 },
    { .rgbcw_100 = { 0.9239f, 0.0000f, 0.0761f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.4904f, 0.2442f, 0.2654f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3823f, 0.3058f, 0.3118f, 0.0000f, 0.0000f }, .hue = 354 },
    { .rgbcw_100 = { 1.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f }, .rgbcw_50 = { 0.5010f, 0.2495f, 0.2495f, 0.0000f, 0.0000f }, .rgbcw_0 = { 0.3846f, 0.3077f, 0.3077f, 0.0000f, 0.0000f }, .hue = 360 },
};

constexpr bool ENABLE_COLOR_CONTROL = true; /**< Enable precise color control */

#else
constexpr uint8_t COLOR_DATA_SIZE = 0; /**< Size of the color data table */
constexpr bool ENABLE_COLOR_CONTROL = false; /**< Enable precise color control */
lightbulb_color_mapping_data_t* color_data = nullptr;
#endif

// Hardware version minor: Mayor is 6 for all H2 products
#define HWV_MINOR_A60      30 /**< Hardware version minor for A60 */
#define HWV_MINOR_G95      31 /**< Hardware version minor for G95 */
#define HWV_MINOR_GU10     32 /**< Hardware version minor for GU10 */
#define HWV_MINOR_E14      33 /**< Hardware version minor for E14 */
#define HWV_MINOR_FILAMENT 00 /**< Hardware version minor for FILAMENT */
#define HWV_MINOR_SMDW     50 /**< Hardware version minor for SMDW (NOT USED)*/

#if HARDWARE_VERSION_MINOR == HWV_MINOR_A60
constexpr uint8_t I2C_RGB_CURRENT = 8; /**< Default RGB current for the driver */
constexpr uint8_t I2C_CCT_CURRENT = 26; /**< Default CCT current for the driver */
#elif HARDWARE_VERSION_MINOR == HWV_MINOR_G95
constexpr uint8_t I2C_RGB_CURRENT = 10; /**< Default RGB current for the driver */
constexpr uint8_t I2C_CCT_CURRENT = 41; /**< Default CCT current for the driver */
#elif HARDWARE_VERSION_MINOR == HWV_MINOR_GU10
constexpr uint8_t I2C_RGB_CURRENT = 8; /**< Default RGB current for the driver */
constexpr uint8_t I2C_CCT_CURRENT = 13; /**< Default CCT current for the driver */
#elif HARDWARE_VERSION_MINOR == HWV_MINOR_E14
constexpr uint8_t I2C_RGB_CURRENT = 8; /**< Default RGB current for the driver */
constexpr uint8_t I2C_CCT_CURRENT = 13; /**< Default CCT current for the driver */
#elif HARDWARE_VERSION_MINOR == HWV_MINOR_FILAMENT
// Two products with same hardware version:
#if (CONFIG_CAPABILITY_COLOR && CONFIG_CAPABILITY_TEMPERATURE) // RGBTW (0x6196)
//----------ATTENTION EGLO IS NOT SURE IF IT WILL BE 16 OR 20 (6/7w)------------//
constexpr uint8_t I2C_RGB_CURRENT = 20; /**< Default RGB current for the driver */
constexpr uint8_t I2C_CCT_CURRENT = 20; /**< Default CCT current for the driver */
#else // RGBW (0x61C0)
constexpr uint8_t I2C_RGB_CURRENT = 20; /**< Default RGB current for the driver */
constexpr uint8_t I2C_CCT_CURRENT = 20; /**< Default CCT current for the driver */
#endif
#elif HARDWARE_VERSION_MINOR == HWV_MINOR_SMDW // SMDW (0x61C0) THIS IS NOT USED,
// BUT ADDED TO AVOID COMPILATION ERRORS
constexpr uint8_t I2C_RGB_CURRENT = 20; /**< Default RGB current for the driver */
constexpr uint8_t I2C_CCT_CURRENT = 20; /**< Default CCT current for the driver */
// NO DEFAULT CASE: Will not compile if no hardware version is defined
#endif

#if (CONFIG_CAPABILITY_COLOR && CONFIG_CAPABILITY_TEMPERATURE)
constexpr lightbulb_led_beads_comb_t LED_BEADS = LED_BEADS_5CH_RGBCW; /**< LED Beads combination for RGBTW */
#define CURRENT_ARRAY { I2C_RGB_CURRENT, I2C_RGB_CURRENT, I2C_RGB_CURRENT, I2C_CCT_CURRENT, I2C_CCT_CURRENT } /**< Current array for RGBTW */
#elif (CONFIG_CAPABILITY_COLOR && CONFIG_CAPABILITY_WHITE)
constexpr lightbulb_led_beads_comb_t LED_BEADS = LED_BEADS_4CH_RGBW; /**< LED Beads combination for RGBW */
#define CURRENT_ARRAY { I2C_RGB_CURRENT, I2C_RGB_CURRENT, I2C_RGB_CURRENT, I2C_CCT_CURRENT, 0 } /**< Current array for RGBW */
#elif (CONFIG_CAPABILITY_TEMPERATURE)
constexpr lightbulb_led_beads_comb_t LED_BEADS = LED_BEADS_2CH_CW; /**< LED Beads combination for TW */
#else
constexpr lightbulb_led_beads_comb_t LED_BEADS = LED_BEADS_1CH_C; /**< LED Beads combination for Dimmable */
#endif