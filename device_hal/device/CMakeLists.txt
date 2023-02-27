include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

set(requires led_driver)

# fan_driver is only added for C3 and ESP32
if (CONFIG_IDF_TARGET_ESP32C3 OR CONFIG_IDF_TARGET_ESP32)
    list(APPEND requires fan_driver)
endif()

if ("${button_type}" STREQUAL "iot")
    list(APPEND requires iot_button)
else()
    list(APPEND requires button)
endif()

idf_component_register(SRC_DIRS "${device_type}"
                        INCLUDE_DIRS include
                        REQUIRES "${requires}")
