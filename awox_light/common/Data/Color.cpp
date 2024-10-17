/**
 * @file Color.cpp
 * @author AWOX
 * @brief This file is gives all the tools to represents and manipulates the data of a state of a light(color, tw, mireds, lumen).
 *
 */

#include "Color.hpp"
#include "esp_log.h"
#include <algorithm>
/** Check usefull blogs on luminance correction:
 *  https://ledshield.wordpress.com/2012/11/13/led-brightness-to-your-eye-gamma-correction-no/
 *  https://jared.geek.nz/2013/feb/linear-led-pwm
 *  this table is the correction calculations of luminance and brightness described by CIE1931:
 *  Y = (L* / 903.3)             if L* <= 8
 *  Y = ((L* + 16) / 119)^3      if L* > 8
 *  Y is the luminance (output) between 0.0 and 1.0 and L* is the brightness input between 0 and 100
 *  This table maps the brightness (input) with the duty cycle (output) directly,
 *  making the LED luminance linear with brightness: duty_cycle = f(brightness)
 *  Execute python script from tools folder to generate table
 */

// clang-format off
constexpr static const char* TAG = "awox_color"; /**< @brief Espressif tag for Log */
constexpr uint8_t CCT_MAX_VALUE = 255; /**< @brief Maximum value for CCT */
constexpr uint8_t CCT_MIN_VALUE = 0; /**< @brief Minimum value for CCT */
/**
 * @brief This table gives a correspondance between brightness and the associated pwm duty cycle for the human eye perception.
 *
 */
static const uint16_t rgbLumenMap[SH_LIGHTNESS_MAX + 1] = {
    0, 284, 284, 284, 284, 284, 284, 284, 284, 284,
    284, 312, 340, 369, 397, 426, 454, 482, 511, 539,
    567, 595, 625, 655, 686, 719, 752, 786, 821, 858,
    895, 934, 973, 1014, 1056, 1098, 1143, 1188, 1234, 1282,
    1331, 1381, 1432, 1484, 1538, 1593, 1649, 1707, 1766, 1826,
    1888, 1951, 2016, 2082, 2149, 2218, 2288, 2359, 2433, 2507,
    2583, 2661, 2740, 2821, 2903, 2987, 3073, 3160, 3248, 3339,
    3431, 3525, 3620, 3717, 3816, 3917, 4019, 4123, 4229, 4337,
    4446, 4558, 4671, 4786, 4903, 5021, 5142, 5265, 5389, 5516,
    5644, 5775, 5907, 6042, 6178, 6317, 6457, 6600, 6745, 6891,
    7040, 7191, 7345, 7500, 7658, 7817, 7979, 8143, 8310, 8479,
    8649, 8823, 8998, 9176, 9356, 9539, 9724, 9911, 10100, 10292,
    10487, 10684, 10883, 11085, 11289, 11496, 11705, 11917, 12131, 12348,
    12568, 12790, 13014, 13241, 13471, 13704, 13939, 14177, 14417, 14661,
    14907, 15155, 15407, 15661, 15918, 16178, 16441, 16706, 16974, 17245,
    17519, 17796, 18076, 18359, 18645, 18933, 19225, 19519, 19817, 20117,
    20421, 20728, 21037, 21350, 21666, 21985, 22307, 22632, 22960, 23292,
    23626, 23964, 24305, 24650, 24997, 25348, 25702, 26059, 26420, 26784,
    27151, 27521, 27895, 28273, 28653, 29037, 29425, 29816, 30210, 30608,
    31009, 31414, 31823, 32234, 32650, 33069, 33491, 33917, 34347, 34780,
    35217, 35658, 36102, 36550, 37002, 37457, 37916, 38379, 38845, 39315,
    39789, 40267, 40749, 41234, 41724, 42217, 42714, 43215, 43720, 44229,
    44741, 45258, 45779, 46303, 46832, 47364, 47901, 48441, 48986, 49535,
    50088, 50645, 51206, 51771, 52340, 52914, 53491, 54073, 54659, 55250,
    55844, 56443, 57046, 57653, 58265, 58881, 59501, 60125, 60754, 61388,
    62025, 62667, 63314, 63965, 64620, 65280,
};
// clang-format on
/**
 * @brief this function uses the rgbLumenMap to convert a brightness to a pwm dutycycle
 * This function could be renamed brightnessToPwm but keeps it the same as old projects.
 * @param index [in] is the brightness on a u8.
 * @return uint16_t Returns the duty cycle that needs to be applied on the leds.
 */
uint16_t rgbToLumen(uint8_t index)
{
    return rgbLumenMap[index];
}

/**
 * @brief Conversion from percentage [0,100] to Brightness [MIN_BRIGHTNESS, MAX_BRIGHTNESS]
 *
 * @param percentage [0,100]
 * @return uint8_t [MIN_BRIGHTNESS, MAX_BRIGHTNESS]
 */
uint8_t percentageToLevel(uint8_t percentage)
{
    auto level = (uint8_t)(((uint16_t)(BRIGHTNESS_RANGE) * (uint16_t)percentage) / 100);
    level += BRIGHTNESS_MIN;
    return level;
}

/**
 * @brief Conversion from Mireds [MINIMUM_TEMPERATURE, MAXIMUM_TEMPERATURE] to percentage [0,100]
 *
 * @param  uint16_t [MINIMUM_TEMPERATURE, MAXIMUM_TEMPERATURE]
 * @return percentage [0,100]
 */
uint8_t miredToPercentage(uint16_t mired)
{
#if MAX_TEMPERATURE_MIREDS == MIN_TEMPERATURE_MIREDS // important: Has to be a #define
    return 0;
#else
    uint16_t kelvin = (uint16_t)(KELVIN_TO_MIREDS_RATIO / mired);
    auto percentage = (uint8_t)(((uint16_t)(kelvin - MINIMUM_TEMPERATURE_KELVIN) * (uint16_t)100)
        / (MAXIMUM_TEMPERATURE_KELVIN - MINIMUM_TEMPERATURE_KELVIN));
    return percentage;
#endif
}

/**
 * @brief Conversion from Brightness [0, MAX_BRIGHTNESS] to percentage [0,100]
 *
 * @param level [0, MAX_BRIGHTNESS]
 * @return uint8_t [0,100]
 */
uint8_t levelToPercentage(uint8_t level)
{
    auto percentage = (uint8_t)(((uint16_t)(level) * (uint16_t)100) / BRIGHTNESS_MAX);

    // If the level is not 0 and the percentage is 0, we return 1 (avoid having 0 brightness when level is not 0)
    if ((level != 0) && (percentage == 0)) {
        percentage = 1;
    }

    return percentage;
}

/**
 * @brief Construct a new Temperature Tw:: Temperature Tw object.
 *
 * @param temperature The Temperature of this white color. See @ref setTemperature to get out of bound values behaviour.
 * @param brightness The brightness of this white color.
 */
TemperatureTw::TemperatureTw(uint16_t temperature, uint8_t brightness)
    : _brightness(brightness)
{
    setTemperature(temperature);
}

/**
 * @brief Use this function to set a tw value. If the value is out of the supported range, the value assigned is the closest supported value.
 *
 * @param temperature temperature to set to the device.
 */
void TemperatureTw::setTemperature(uint16_t temperature)
{
    if (temperature > MAXIMUM_TEMPERATURE) {
        _temperature = MAXIMUM_TEMPERATURE;
    } else if (temperature < MINIMUM_TEMPERATURE) {
        _temperature = MINIMUM_TEMPERATURE;
    } else {
        _temperature = temperature;
    }
}

/**
 * @brief transform this TemperatureTW object to a WarmColdTw object. This is possible as they are a different representation of the same value.
 *
 * @return WarmColdTw The returned object.
 */
WarmColdTw TemperatureTw::toWarmColdTw() const
{
    if (_brightness == 0) {
        return WarmColdTw(0, 0);
    }
    uint8_t ct = cctConvertMiredToCt(_temperature);
    uint16_t totalTicks = rgbToLumen(_brightness);
    uint8_t lumWarm = CEIL_DIV(((SH_CT_MAX - ct) * _brightness), SH_LIGHTNESS_MAX);
    uint16_t ticksWw = (totalTicks * lumWarm) / _brightness;
    uint16_t ticksCw = totalTicks - ticksWw;
    return WarmColdTw(ticksWw / 256, ticksCw / 256);
}

/**
 * @brief This function is used to convert a cct value in mireds to a ct (cold temperature) value
 * This function is exported from connect.b
 * @param mired [in] The value to convert
 * @return uint8_t The converted value
 */
uint8_t cctConvertMiredToCt(uint16_t mired)
{
    uint8_t ct = 0;

    // Technically mired is always in product range because the check is done by cct_set_mired
    if (mired >= MAXIMUM_TEMPERATURE) {
        ct = CCT_MIN_VALUE;
    } else if (mired <= MINIMUM_TEMPERATURE) {
        ct = CCT_MAX_VALUE;
    } else {
        // Remap mired to telink ct value range
        // - MAXIMUM_TEMPERATURE -> CT_MIN_VALUE (0)
        // - MINIMUM_TEMPERATURE -> CT_MAX_VALUE (255)
        ct = REMAP(MINIMUM_TEMPERATURE, MAXIMUM_TEMPERATURE, SH_CT_MAX, SH_CT_MIN, mired);
    }

    return ct;
}
/**
 * @brief This function is used to convert a HSV object to a RGB object usable for pwms.
 *
 * @return RgbColor The returned RGB object.
 * @note source: https://cs.stackexchange.com/questions/64549/convert-hsv-to-rgb-colors
 */
RgbColor HsvColor::toRgb() const
{
    double saturation = _saturation / HsvRatio;
    double value = (double)_value / RgbRatio;
    uint16_t hue = ((uint16_t)_hue * HueTheoricalMaxValue + HsvMaxValue - 1) / HsvMaxValue; // 254-1 to avoid rounding errors
    double c = value * saturation;
    double h = hue / (double)HueRegionSize;
    double remainder = h - (uint8_t)(h / 2) * 2; // remainder = h % 2
    double x = c; // double x = c(1.0 - abs(remainder - 1.0)); : abs() gives problems
    if (remainder > 1.0) {
        x *= (2 - remainder);
    } else {
        x *= remainder;
    }
    double m = value - c;
    double r;
    double g;
    double b;
    switch ((uint8_t)h) { // REGION
    case 0:
        r = c;
        g = x;
        b = 0;
        break;
    case 1:
        r = x;
        g = c;
        b = 0;
        break;
    case 2:
        r = 0;
        g = c;
        b = x;
        break;
    case 3:
        r = 0;
        g = x;
        b = c;
        break;
    case 4:
        r = x;
        g = 0;
        b = c;
        break;
    default:
        r = c;
        g = 0;
        b = x;
        break;
    }
    return RgbColor((uint8_t)((r + m) * RgbMaxValue),
        (uint8_t)((g + m) * RgbMaxValue),
        (uint8_t)((b + m) * RgbMaxValue));
}

/**
 * @brief Round a double to the nearest integer
 *
 */
#define AWROUND(v) ((uint8_t)((v) + 0.5))

/**
 * @brief Check if a value is in a tolerance range.
 *
 * @param value the value to check
 * @param target the target value
 * @param tolerance the tolerance
 * @return true is in tolerance
 * @return false it isn't in tolerance
 */
bool isInTolerance(double value, double target, double tolerance)
{
    return (value >= target - tolerance) && (value <= target + tolerance);
}

/**
 * @brief Transform this RgbColor object to a HsvColor object.
 *
 * @return HsvColor The returned HsvColor object.
 */
HsvColor RgbColor::toHsv() const
{
    auto hsv = HsvColor();
    double saturation;
    double value;
    double hue = 0;
    double delta = 0;
    double precision = RgbToHSVPrecision;
    double min;
    double max;
    double red = (double)_red / RgbRatio;
    double green = (double)_green / RgbRatio;
    double blue = (double)_blue / RgbRatio;

    if (red > green) {
        max = std::max(red, blue);
        min = std::min(green, blue);
    } else {
        max = std::max(green, blue);
        min = std::min(red, blue);
    }

    delta = max - min;
    value = max * HsvMaxValue;
    hsv.setValue(AWROUND(value));

    if (isInTolerance(delta, 0, precision)) {
        hsv.setSaturation(0);
        hsv.setHue(0);
    } else {
        saturation = delta / max;
        hsv.setSaturation(AWROUND(saturation * HsvMaxValue));

        if (isInTolerance(red, max, precision)) {
            hue = (green - blue) / delta;
            if (green < blue) {
                hue += 6;
            }
        } else if (isInTolerance(green, max, precision)) {
            hue = 2.0 + ((blue - red) / delta);
        } else if (isInTolerance(blue, max, precision)) {
            hue = 4.0 + ((red - green) / delta);
        } else {
            ESP_LOGV(TAG, "NOTHING");
        }

        hue *= HueRegionSize;
        while (hue < 0) {
            hue += HueTheoricalMaxValue;
        }
        hsv.setHue(AWROUND(hue * HsvMaxValue / HueTheoricalMaxValue));
    }
    return hsv;
}

/**
 * @brief Transform this RgbColor object to a xyColor object.
 *
 * @param brightness The brightness of the color needed to pass to a 3D space.
 * @return XyzColor The returned xyColor object.
 * @note from Telink project (connect.z)
 */
XyzColor XyColor::toXyz(uint8_t brightness) const
{
    auto x = (double)_x / xyYRatio;
    auto y = (double)_y / xyYRatio;
    if (x + y > 1.0) {
        ESP_LOGE("xyColor", "Invalid xy values: x=%d, y=%d", _x, _y); // WTF we should do
    }
    double z = 1.0 - x - y;

    auto Y = (double)brightness / HsvRatio;
    double X = (Y * x) / y;
    double Z = (Y * z) / y;
    return XyzColor(X, Y, Z);
}

/**
 * @brief if x is out of range, saturate it to the limit value.
 *
 */
#define INRANGE(x, low, high) \
    if (x < low) {            \
        x = low;              \
    } else if (x > high) {    \
        x = high;             \
    }

/**
 * @brief Transform this XyzColor object to a RgbColor object.
 *
 * @return RgbColor The returned RgbColor object.
 * @note The transformation matrix is {{3.2410,-1.5374,-0.4986},{-0.9692,1.8758, 0.0416},{0.0556,-0.2040 , 1.0570}}
 */
RgbColor XyzColor::toRgb() const
{
    double r = x_component * 3.2406 + y_component * -1.5372 + z_component * -0.4986;
    double g = x_component * -0.9692 + y_component * 1.8758 + z_component * 0.0416;
    double b = x_component * 0.0556 + y_component * -0.2040 + z_component * 1.0570;
    INRANGE(r, XyzMinValue, XyzMaxValue)
    INRANGE(g, XyzMinValue, XyzMaxValue)
    INRANGE(b, XyzMinValue, XyzMaxValue)
    return RgbColor((uint8_t)(r * RgbMaxValue),
        (uint8_t)(g * RgbMaxValue),
        (uint8_t)(b * RgbMaxValue));
}

/**
 * @brief Transform this XyzColor object to a xyColor object.
 *
 * @return XyzColor The returned xyColor object.
 */
XyzColor RgbColor::toXyz() const
{
    double r = (double)_red / RgbRatio;
    double g = (double)_green / RgbRatio;
    double b = (double)_blue / RgbRatio;
    double X = r * 0.4124 + g * 0.3576 + b * 0.1805;
    double Y = r * 0.2126 + g * 0.7152 + b * 0.0722;
    double Z = r * 0.0193 + g * 0.1192 + b * 0.9505;
    return XyzColor(X, Y, Z);
}

/**
 * @brief Transform this XyzColor object to a xyColor object.
 *
 * @return XyzColor The returned xyColor object.
 */
XyColor XyzColor::toXy() const
{
    double x = x_component / (x_component + y_component + z_component);
    double y = y_component / (x_component + y_component + z_component);
    return XyColor((uint16_t)(x * xyYRatio), (uint16_t)(y * xyYRatio));
}
