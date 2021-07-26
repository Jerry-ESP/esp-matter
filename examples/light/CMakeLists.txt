#    Copyright 2021 Espressif Systems (Shanghai) CO LTD
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License

# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "Please set ESP_MATTER_PATH to the path of esp-matter repo")
endif(NOT DEFINED ENV{ESP_MATTER_PATH})
set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})

# This should be done before using the IDF_TARGET variable.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    "../common"
    "${ESP_MATTER_PATH}/connectedhomeip/config/esp32/components"
    "${ESP_MATTER_PATH}/components")

if(${IDF_TARGET} STREQUAL "esp32")
    list(APPEND EXTRA_COMPONENT_DIRS
        "${ESP_MATTER_PATH}/connectedhomeip/examples/common/m5stack-tft/repo/components/tft"
        "${ESP_MATTER_PATH}/connectedhomeip/examples/common/m5stack-tft/repo/components/spidriver"
    )
elseif(${IDF_TARGET} STREQUAL "esp32c3")
    list(APPEND EXTRA_COMPONENT_DIRS
        "$ENV{IDF_PATH}/examples/peripherals/rmt/led_strip/components"
    )
endif()

project(light)

idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++14;-Os;-DLWIP_IPV6_SCOPES=0;-DCHIP_HAVE_CONFIG_H" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os;-DLWIP_IPV6_SCOPES=0" APPEND)
