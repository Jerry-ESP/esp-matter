set(requires_list )

# Chip specific components. Try and remove this later.
if(${IDF_TARGET} STREQUAL "esp32")
    list(APPEND requires_list tft spidriver)
elseif(${IDF_TARGET} STREQUAL "esp32c3")
    list(APPEND requires_list led_strip)
endif()

idf_component_register(SRCS ${CONFIG_LED_DRIVER}/led_driver.c
                    INCLUDE_DIRS include
                    REQUIRES ${requires_list})
