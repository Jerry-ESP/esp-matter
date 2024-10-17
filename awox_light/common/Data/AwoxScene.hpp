#pragma once
#include "Color.hpp"
#include "TargetConfig.hpp"
#include <list>
#include <stdint.h>

/**
 * @brief  Class used to define a scene
 *
 */
class AwoxScene {
public:
    AwoxScene() = default; /**< @brief Construct a new Awox Scene object */
    // Getters & setters
    /**
     * @brief On getter
     *
     * @return true
     * @return false
     */
    bool isOn() const { return _on; }
    /**
     * @brief Set the On object
     *
     * @param on
     */
    void setOn(bool on) { _on = on; }
    /**
     * @brief Set the Color Mode object
     *
     * @param color
     */
    void setColor(bool color) { _color = color; }
    /**
     * @brief Get the Color Mode
     *
     * @return true is in color mode
     * @return false is in CCT mode
     */
    bool isColor() const { return _color; }
    /**
     * @brief Set the Hue object
     *
     * @param hue
     */
    void setHue(uint8_t hue) { _hue = validate(hue); }
    /**
     * @brief Get the Hue object
     *
     * @return uint8_t
     */
    uint8_t getHue() const { return _hue; }
    /**
     * @brief Set the Saturation object
     *
     * @param sat
     */
    void setSat(uint8_t sat) { _sat = validate(sat); }
    /**
     * @brief Get the Saturation object
     *
     * @return uint8_t
     */
    uint8_t getSat() const { return _sat; }
    /**
     * @brief Set the Temperature object
     *
     * @param temperature
     */
    void setTemperature(uint16_t temperature) { _temperature = temperature; }
    /**
     * @brief Get the Temperature object
     *
     * @return uint16_t
     */
    uint16_t getTemperature() const { return _temperature; }
    /**
     * @brief Get the Level object
     *
     * @return uint8_t
     */
    uint8_t getLevel() const { return _level; }
    /**
     * @brief Get the Level (In Percentage) object
     *
     * @return uint8_t
     */
    uint8_t getLevelInPercentage() const { return levelToPercentage(_level); }
    /**
     * @brief Set the Level object
     *
     * @param level
     */
    void setLevel(uint8_t level) { _level = validate(level); }
    /**
     * @brief Set the Level (In Percentage) object
     *
     * @param level
     */
    void setLevelInPercentage(uint8_t level) { _level = percentageToLevel(level); }
#if (CAPABILITY_BACKLIGHT)
    /**
     * @brief Get the rgb Level object
     *
     * @return uint8_t
     */
    uint8_t getRgbLevel() const { return _rgbLevel; }
    /**
     * @brief set the rgb Level object
     *
     * @param rgbLevel
     */
    void setRgbLevel(uint8_t rgbLevel) { _rgbLevel = validate(rgbLevel); }
    /**
     * @brief set the rgb Level (In Percentage) object
     *
     * @param rgbLevel
     */
    void setRgbLevelInPercentage(uint8_t rgbLevel) { _rgbLevel = percentageToLevel(rgbLevel); }
    /**
     * @brief Get the rgb Level (In Percentage) object
     *
     * @return uint8_t
     */
    uint8_t getRgbLevelInPercentage() const { return levelToPercentage(_rgbLevel); }
    /**
     * @brief Get the White Level object
     *
     */
    uint8_t getWhiteLevel() const { return _whiteLevel; }
    /**
     * @brief Set the White Level object
     *
     * @param whiteLevel
     */
    void setWhiteLevel(uint8_t whiteLevel) { _whiteLevel = validate(whiteLevel); }
    /**
     * @brief Set the White Level (In Percentage) object
     *
     * @param whiteLevel
     */
    void setWhiteLevelInPercentage(uint8_t whiteLevel) { _whiteLevel = percentageToLevel(whiteLevel); }
    /**
     * @brief Get the White Level (In Percentage) object
     *
     * @return uint8_t
     */
    uint8_t getWhiteLevelInPercentage() const { return levelToPercentage(_whiteLevel); }
    /**
     * @brief Set the Mix object
     *
     * @param mix
     */
    void setMix(bool mix) { _mix = mix; }
    /**
     * @brief Get the Mix object
     *
     * @return true
     * @return false
     */
    bool isMix() const { return _mix; }
#endif
private:
    uint8_t _hue; /**< hue value */
    uint8_t _sat; /**< saturation value */
    uint8_t _level; /**< level value */
    uint16_t _temperature; /**< temperature value */
    bool _on; /**< on value */
    bool _color; /**< color value */
#if (CAPABILITY_BACKLIGHT)
    bool _mix; /**< mix value */
    uint8_t _rgbLevel; /**< rgb level value */
    uint8_t _whiteLevel; /**< white level value */
#endif
    const uint8_t sceneLevelIndex = 1; /**< where the level is in the Scene */
    const uint8_t sceneHueIndex = 2; /**< where the hue is in the Scene */
    const uint8_t sceneSatIndex = 3; /**< where the saturation is in the Scene */
    const uint8_t sceneTemperatureIndex = 2; /**< where temperature is in the Scene */
    uint8_t validate(uint8_t value) const;
};
