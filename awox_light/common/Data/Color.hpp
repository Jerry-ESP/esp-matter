/**
 * @file Color.hpp
 * @author AWOX
 * @brief This file is gives all the tools to represents and manipulates the data of a state of a light(color, tw, mireds, lumen).
 *
 */
#pragma once
#include "TargetConfig.hpp"
#include <list>
#include <stdint.h>

/**
 * @brief This Maccro permit to get the value of a variable on another scale
 * https://karbotronics.com/blog/2020-02-28-formule-changement-echelle-min-et-max/
 */
#if MAX_TEMPERATURE_MIREDS != MIN_TEMPERATURE_MIREDS
#define REMAP(in_s, in_e, out_s, out_e, val) (out_s + ((out_e - out_s) * (val - in_s)) / (in_e - in_s))
#else
#define REMAP(in_s, in_e, out_s, out_e, val) out_s
#endif
/**
 * @brief The CEIL Division of an int gives you the result of a divion approximated to the upper integer.
 * https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
 */
#define CEIL_DIV(A, B) (((A) + (B) - 1) / (B))

constexpr uint8_t MAX_ZIGBEE_U8 = 254; /**< @brief Maximum value of a Zigbee uint8_t*/

constexpr uint8_t BRIGHTNESS_MIN = 1; /**< @brief Gives the minimum value of brightness that the device can supports. 0 means OFF*/
constexpr uint8_t BRIGHTNESS_MAX = 254; /**< @brief Gives the maximum value of brightness that the device can supports.*/
constexpr uint8_t BRIGHTNESS_HALF = 128; /**< 50% of the maximum value of brightness that the device can supports.*/
constexpr uint8_t BRIGHTNESS_20 = 51; /**< 20% of the maximum value of brightness that the device can supports.*/
constexpr uint8_t BRIGHTNESS_30 = 77; /**< 30% of the maximum value of brightness that the device can supports.*/
constexpr uint8_t BRIGHTNESS_85 = 218; /**< 85% of the maximum value of brightness that the device can supports.*/
constexpr uint8_t BRIGHTNESS_RANGE = BRIGHTNESS_MAX - BRIGHTNESS_MIN; /**< @brief Gives the range of brightness that the device can supports.*/

constexpr uint16_t MAXIMUM_TEMPERATURE = MAX_TEMPERATURE_MIREDS; /**< @brief Gives the maximum value of temperature (in mired) that the device can supports.*/
constexpr uint16_t MINIMUM_TEMPERATURE = MIN_TEMPERATURE_MIREDS; /**< @brief Gives the minimum value of temperature (in mired) that the device can supports.*/
constexpr uint32_t KELVIN_TO_MIREDS_RATIO = 1000000; /**< @brief Gives the ratio to convert Kelvin to Mireds.*/
constexpr uint16_t MAXIMUM_TEMPERATURE_KELVIN = (uint16_t)(KELVIN_TO_MIREDS_RATIO / MINIMUM_TEMPERATURE); /**< @brief Gives the maximum value of temperature (in k)*/
constexpr uint16_t MINIMUM_TEMPERATURE_KELVIN = (uint16_t)(KELVIN_TO_MIREDS_RATIO / MAXIMUM_TEMPERATURE); /**< @brief Gives the minimum value of temperature (in k)*/

constexpr uint8_t SH_LIGHTNESS_MIN = 10; /**< @brief min value of the brightness range that gives the numbers of possible brightness. All device may not supports the whole possibilities.*/
constexpr uint8_t SH_LIGHTNESS_MAX = 255; /**< @brief max value of the brightness range that gives the numbers of possible brightness. All device may not supports the whole possibilities.*/
constexpr uint8_t SH_CT_MIN = 0; /**< @brief min value of the cold white channel. Warm white channel is 1 - cold.*/
constexpr uint8_t SH_CT_MAX = 255; /**< @brief max value of the cold white channel. */

constexpr uint32_t xyYRatio = 65'536; /**< @brief Current X/Y relation rate*/
constexpr uint8_t RgbMaxValue = 255; /**< @brief Max value of a RGB component*/
constexpr uint8_t HsvMaxValue = 254; /**< @brief Max value of a HSV component*/
constexpr uint16_t HueTheoricalMaxValue = 360; /**< @brief Theorical max value of a Hue component*/
constexpr uint8_t HueRegionSize = 60; /**< @brief Size of a Hue region*/
constexpr uint8_t PercentageMaxValue = 100; /**< @brief Max value of a percentage*/
constexpr double RgbRatio = 255.0; /**< @brief Current RGB relation rate*/
constexpr double HsvRatio = 254.0; /**< @brief Current HSV relation rate*/
constexpr double XyzMaxValue = 1.0; /**< @brief Max value of a XYZ component*/
constexpr double XyzMinValue = 0.0; /**< @brief Min value of a XYZ component*/
constexpr double RgbToHSVPrecision = 0.0001; /**< @brief Precision of the RGB to HSV conversion*/

uint8_t percentageToLevel(uint8_t percentage);
uint8_t levelToPercentage(uint8_t level);
uint8_t miredToPercentage(uint16_t mired);

/**
 * @enum lightDriverMode_t
 * @brief Possible modes a light can implement
 */
enum lightDriverMode_t {
    LIGHT_DRIVER_MODE_RGB_HSV, /**< HSV RGB mode */
    LIGHT_DRIVER_MODE_RGB_XY, /**< XY RGB mode */
    LIGHT_DRIVER_MODE_TW, /**< TW mode */
    LIGHT_DRIVER_MODE_DIMMABLE, /**< Dimmable mode */
    LIGHT_DRIVER_MODE_MIX, /**< Mix RGB-TW mode */
    LIGHT_DRIVER_MODE_IC /**< RGBIC mode */
};

/**
 * @brief Predefined Colors (HUE value)
 *
 */
enum class HUE_COLOR : uint8_t {
    RED = 0, /**< @brief Hue value for pure Red color*/
    ORANGE = 12, /**< @brief Hue value for Orange color*/
    YELLOW = 43, /**< @brief Hue value for Yellow color*/
    GREEN = 85, /**< @brief Hue value for pure Green color*/
    CYAN = 137, /**< @brief Hue value for Cyan color*/
    BLUE = 170, /**< @brief Hue value for pure Blue color*/
    PURPLE = 215, /**< @brief Hue value for Purple color*/
    PINK = 244 /**< @brief Hue value for Pink color*/
};

class RgbColor;
class HsvColor;
class XyzColor;
class XyColor;
class WarmColdTw;

/**
 * @brief This class is the representation of a color in Red, Green and Blue components.
 *
 */
class RgbColor {
public:
    /**
     * @brief Construct a new Rgb Color object
     * Default constuctor initialize the struct to 0 in each components.
     */
    RgbColor()
        : _red(0)
        , _green(0)
        , _blue(0)
    {
    }
    /**
     * @brief Construct a new Rgb Color object
     * Constructor with argument to specify the init values.
     * @param red The given Red
     * @param green The given Red
     * @param blue The given Red
     */
    RgbColor(uint8_t red, uint8_t green, uint8_t blue)
        : _red(red)
        , _green(green)
        , _blue(blue)
    {
    }
    HsvColor toHsv() const;
    XyzColor toXyz() const;
    uint8_t getRed() { return _red; } /**< @brief Get the Red value */
    uint8_t getBlue() { return _blue; } /**< @brief Get the Blue value */
    uint8_t getGreen() { return _green; } /**< @brief Get the Green value */
    void setRed(uint8_t red) { _red = red; } /**< @brief Set the Red value */
    void setBlue(uint8_t blue) { _blue = blue; } /**< @brief Set the Blue value */
    void setGreen(uint8_t green) { _green = green; } /**< @brief Set the Green value */

private:
    uint8_t _red; /**< Red component of the RGB object*/
    uint8_t _green; /**< Green component of the RGB object*/
    uint8_t _blue; /**< Blue component of the RGB object*/
};

/**
 * @brief This class is the representation of a color in Hue, Saturation and Value (level or brightness) components.
 *
 */
class HsvColor {
public:
    /**
     * @brief Construct a new HSV Color object
     * Default constuctor initialize the struct to 0 in each components.
     */
    HsvColor()
        : _hue(0)
        , _saturation(0)
        , _value(0)
    {
    }
    /**
     * @brief Construct a new HSV Color object
     * Constructor with argument to specify the init values.
     * @param hue The given hue
     * @param saturation The given saturation
     * @param value The given value
     */
    HsvColor(uint8_t hue, uint8_t saturation, uint8_t value)
        : _hue(hue)
        , _saturation(saturation)
        , _value(value) {};
    RgbColor toRgb() const;
    uint8_t getHue() { return _hue; } /**< @brief Get the hue value */
    uint8_t getSaturation() { return _saturation; } /**< @brief Get the saturation value */
    uint8_t getValue() { return _value; } /**< @brief Get the value value */
    void setHue(uint8_t hue) { _hue = hue; } /**< @brief Set the hue value */
    void setSaturation(uint8_t saturation) { _saturation = saturation; } /**< @brief Set the saturation value */
    void setValue(uint8_t value) { _value = value; } /**< @brief Set the value value */

private:
    uint8_t _hue; /**< Hue component of the HSV object*/
    uint8_t _saturation; /**< Saturation component of the HSV object*//**< RGB mode */
    uint8_t _value; /**< Value component of the HSV object*/
};

/**
 * @brief xyY Color (CIE1931) representation, used in Zigbee X/Y color space.
 *
 */
class XyColor {
public:
    /**
     * @brief Construct a new Xy Color object
     *
     * @param x as Zigbee Standard
     * @param y as Zigbee Standard
     */
    XyColor(uint16_t x, uint16_t y)
        : _x(x)
        , _y(y) {};
    XyzColor toXyz(uint8_t brightness) const;
    uint16_t getX() const { return _x; } /**< @brief Get the X value */
    uint16_t getY() const { return _y; } /**< @brief Get the Y value */
    void setX(uint16_t x) { _x = x; }
    void setY(uint16_t y) { _y = y; }

private:
    uint16_t _x; /**< X component of the XY object*/
    uint16_t _y; /**< Y component of the XY object*/
};

/**
 * @brief This class is the representation of a color in X, Y and Z components as defined in the CIE1931.
 *
 */
class XyzColor {
public:
    /**
     * @brief Construct a new Xyz Color object
     * Constructor with argument to specify the init values.
     * @param X The given X
     * @param Y The given Y
     * @param Z The given Z
     */
    XyzColor(double X, double Y, double Z)
        : x_component(X)
        , y_component(Y)
        , z_component(Z) {};

    RgbColor toRgb() const;
    XyColor toXy() const;
    double getX() const { return x_component; } /**< @brief Get the X value */
    double getY() const { return y_component; } /**< @brief Get the Y value */
    double getZ() const { return z_component; } /**< @brief Get the Z value */

private:
    double x_component; /**< X component of the XYZ object*/
    double y_component; /**< Y component of the XYZ object*/
    double z_component; /**< Z component of the XYZ object*/
};

/**
 * @brief This class is the representation of a White in Brightness and Temperature components.
 *
 */
class TemperatureTw {
public:
    /**
     * @brief Construct a new TemperatureTW White object
     * Default constuctor initialize the struct to 0 in each components.
     */
    TemperatureTw()
        : _temperature(0)
        , _brightness(0)
    {
    }
    TemperatureTw(uint16_t temperature, uint8_t brightness);
    WarmColdTw toWarmColdTw() const;
    uint16_t getTemperature() { return _temperature; } /**< @brief Get the Temperature value */
    uint8_t getBrightness() { return _brightness; } /**< @brief Get the Brightness value */
    void setTemperature(uint16_t temperature); /**< @brief Set the Temperature value */
    void setBrightness(uint8_t brightness) { _brightness = brightness; } /**< @brief Set the Brightness value */

private:
    uint16_t _temperature; /**< temperature component of the TemperatureTW object*/
    uint8_t _brightness; /**< brightness component of the TemperatureTW object*/
};

/**
 * @brief This class is the representation of a White in Warm and Cold components.
 *
 */
class WarmColdTw {
public:
    /**
     * @brief Construct a new WarmColdTw White object
     * Default constuctor initialize the struct to 0 in each components.
     */
    WarmColdTw()
        : _warm(0)
        , _cold(0)
    {
    }
    /**
     * @brief Construct a new Warm Cold Tw object
     * Constructor with argument to specify the init values.
     * @param warm The given Warm
     * @param cold The given Cold
     */
    WarmColdTw(uint8_t warm, uint8_t cold)
        : _warm(warm)
        , _cold(cold)
    {
    }
    TemperatureTw toTemperatureTw(); /**< @brief NOT IMPLEMENTED*/
    uint8_t getWarm() { return _warm; } /**< @brief Get the Warm value */
    uint8_t getCold() { return _cold; } /**< @brief Get the Cold value */

private:
    uint8_t _warm; /**< warm component of the WarmColdTw object*/
    uint8_t _cold; /**< cold component of the WarmColdTw object*/
};

/**
 * @brief This class is the representation of the state of a bulb. It is composed of a White object and a Color  object.
 *
 */
class RgbTwColor {
public:
    TemperatureTw white; /**< The White part of a bulb state*/
    HsvColor color; /**< The Color part of a bulb state*/
};

/**
 * @brief Todo
 */
class Palette {
    RgbTwColor Color1; /**< @brief NOT IMPLEMENTED*/
    RgbTwColor Color2; /**< @brief NOT IMPLEMENTED*/
    RgbTwColor Color3; /**< @brief NOT IMPLEMENTED*/
};

/**
 * @brief Todo
 */
typedef std::list<RgbTwColor> StripLed;

uint16_t rgbToLumen(uint8_t index);
uint8_t cctConvertMiredToCt(uint16_t mired);