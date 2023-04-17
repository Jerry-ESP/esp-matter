set(PRIV_REQUIRES_LIST device esp_matter esp_matter_console esp_matter_controller route_hook app_reset spiffs esp_lcd lvgl esp-sr libhelix-mp3 wifi_provisioning led_strip)

idf_component_register(SRC_DIRS          "." "./box" "./box/app" "./box/gui" "./box/gui/font" "./box/gui/image"
                      PRIV_INCLUDE_DIRS  "." "./box" "./box/app" "./box/gui"
                      PRIV_REQUIRES      ${PRIV_REQUIRES_LIST})

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
