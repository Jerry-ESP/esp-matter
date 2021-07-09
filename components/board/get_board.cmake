# Designed to be included from the app's CMakeLists.txt file
cmake_minimum_required(VERSION 3.5)

# Set the BOARD_NAME from environment, if exported. Set the default BOARD_NAME based on IDF_TARGET, if not exported.
if(DEFINED ENV{BOARD_NAME})
    message(STATUS "Setting BOARD_NAME from environment to $ENV{BOARD_NAME}")
    set (BOARD_NAME $ENV{BOARD_NAME} CACHE INTERNAL "")

    # Board specific check. Try and remove this later.
    if ((${BOARD_NAME} STREQUAL "esp32c3_devkit_m") AND ((${IDF_TARGET} STREQUAL "esp32") OR (${IDF_TARGET} STREQUAL "esp32s2")))
        message(FATAL_ERROR "Incorrect IDF_TARGET ${IDF_TARGET} set for the selected board ${BOARD_NAME}. Set the correct IDF_TARGET using: idf.py set-target <chip>")
    endif()

else()
    if(${IDF_TARGET} STREQUAL "esp32")
        message(STATUS "Setting default BOARD_NAME to esp32_devkit_c")
        set (BOARD_NAME "esp32_devkit_c" CACHE INTERNAL "")
    elseif(${IDF_TARGET} STREQUAL "esp32c3")
        message(STATUS "Setting default BOARD_NAME to esp32c3_devkit_m")
        set (BOARD_NAME "esp32c3_devkit_m" CACHE INTERNAL "")
    else()
        message(STATUS "Setting default BOARD_NAME to hollow")
        set (BOARD_NAME "hollow" CACHE INTERNAL "")
    endif()
endif(DEFINED ENV{BOARD_NAME})

# Chip specific components. Try and remove this later.
if(${IDF_TARGET} STREQUAL "esp32")
    list(APPEND EXTRA_COMPONENT_DIRS
        "${ESP_MATTER_PATH}/connectedhomeip/examples/common/m5stack-tft/repo/components/tft"
        "${ESP_MATTER_PATH}/connectedhomeip/examples/common/m5stack-tft/repo/components/spidriver")
elseif(${IDF_TARGET} STREQUAL "esp32c3")
    list(APPEND EXTRA_COMPONENT_DIRS
        "${ESP_MATTER_PATH}/esp-idf/examples/peripherals/rmt/led_strip/components")
endif()
