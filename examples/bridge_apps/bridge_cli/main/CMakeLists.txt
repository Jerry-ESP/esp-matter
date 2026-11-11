idf_component_register(SRC_DIRS          "."
                       PRIV_INCLUDE_DIRS  "." "${ESP_MATTER_PATH}/examples/common/utils"
                       PRIV_REQUIRES      esp_matter_bridge)

target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp_matter_console_bridge_register_commands")
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
