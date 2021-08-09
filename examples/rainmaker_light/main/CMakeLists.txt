set(SRC_DIRS_LIST          "${CMAKE_CURRENT_LIST_DIR}"
                           "${CMAKE_CURRENT_LIST_DIR}/gen"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/server"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/util"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/reporting"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/basic"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/general-commissioning-server"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/network-commissioning"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/on-off-server"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/operational-credentials-server"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/level-control"
                           "${ESP_MATTER_PATH}/connectedhomeip/src/app/clusters/color-control-server")

set(INCLUDE_DIRS_LIST      "${CMAKE_CURRENT_LIST_DIR}"
                           "${ESP_MATTER_PATH}/connectedhomeip/src")

set(PRIV_REQUIRES_LIST chip bt esp32_mbedtls app_driver esp_rainmaker)

idf_component_register(SRC_DIRS         ${SRC_DIRS_LIST}
                      INCLUDE_DIRS      ${INCLUDE_DIRS_LIST}
                      PRIV_REQUIRES     ${PRIV_REQUIRES_LIST})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14) 
target_compile_options(${COMPONENT_LIB} PRIVATE "-DLWIP_IPV6_SCOPES=0" "-DCHIP_HAVE_CONFIG_H")
