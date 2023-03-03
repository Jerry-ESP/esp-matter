include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

set(requires led_driver)

# fan_driver is only added for C3 and ESP32
if ("${IDF_TARGET}" STREQUAL "esp32c3") 
    list(APPEND requires buzzer_driver)
endif()

if ("${IDF_TARGET}" STREQUAL "esp32" OR "${IDF_TARGET}" STREQUAL "")
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
