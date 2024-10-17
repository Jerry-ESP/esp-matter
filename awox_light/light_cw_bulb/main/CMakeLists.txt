idf_component_register(SRC_DIRS          "." "./led_pattern"
                       PRIV_INCLUDE_DIRS "." "./led_pattern" "${ESP_MATTER_PATH}/examples/common/utils")

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
