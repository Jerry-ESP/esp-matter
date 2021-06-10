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

idf_component_register(PRIV_INCLUDE_DIRS 
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${ESP_MATTER_PATH}/connectedhomeip/src"
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      SRC_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_CURRENT_LIST_DIR}/gen"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/server"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/util"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/reporting"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/basic"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/bindings"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/general-commissioning-server"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/network-commissioning"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/on-off-server"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/operational-credentials-server"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/level-control"
                      "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/trusted-root-certificates-server"
                      PRIV_REQUIRES chip bt)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14) 
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")
