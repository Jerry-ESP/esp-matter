set(PRIV_REQUIRES_LIST device esp_matter esp_matter_console route_hook app_reset lcd bus screen console esp_adc_cal esp_lcd)

set(src_dirs        .
                    src)

set(include_dirs    include
                    .)

idf_component_register(SRC_DIRS          ${src_dirs}
                      PRIV_INCLUDE_DIRS  ${include_dirs}
                      PRIV_REQUIRES      ${PRIV_REQUIRES_LIST})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
