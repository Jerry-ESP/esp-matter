# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "Please set ESP_MATTER_PATH to the path of esp-matter repo")
endif(NOT DEFINED ENV{ESP_MATTER_PATH})

set(PROJECT_VER "1.0")
set(PROJECT_VER_NUMBER 1)

set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

# This should be done before using the IDF_TARGET variable.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)

set(EXTRA_COMPONENT_DIRS
    "${ESP_MATTER_PATH}/examples/common"
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    "${ESP_MATTER_PATH}/awox_light/common/Data"
    "${ESP_MATTER_PATH}/awox_light/common/Drivers/FlashDriver"
    "${ESP_MATTER_PATH}/awox_light/common/Drivers/RgbTwDriver"
    "${ESP_MATTER_PATH}/awox_light/common/Drivers/TwDriver"
    "${ESP_MATTER_PATH}/awox_light/common/Drivers/DimmableDriver"
    "${ESP_MATTER_PATH}/awox_light/common/Drivers/VirtualDriver"
    "${ESP_MATTER_PATH}/awox_light/common/Manager/BleManager"
    "${ESP_MATTER_PATH}/awox_light/common/Manager/RebootCounterManager"
    "${ESP_MATTER_PATH}/awox_light/common/Manager/StorageManager"
    "${ESP_MATTER_PATH}/awox_light/common/Manager/VirtualManager"
    "${ESP_MATTER_PATH}/awox_light/common/TargetConfig"
    "${ESP_MATTER_PATH}/awox_light/common/TaskConfig"
    "${ESP_MATTER_PATH}/awox_light/common/Product"
    ${extra_components_dirs_append})

project(dimmable_pwm)

# WARNING: This is just an example for using key for decrypting the encrypted OTA image
# Please do not use it as is.
if(CONFIG_ENABLE_ENCRYPTED_OTA)
    target_add_binary_data(light.elf "esp_image_encryption_key.pem" TEXT)
endif()

if(CONFIG_IDF_TARGET_ESP32C2)
    include(relinker)
endif()

idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
# For RISCV chips, project_include.cmake sets -Wno-format, but does not clear various
# flags that depend on -Wformat
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
