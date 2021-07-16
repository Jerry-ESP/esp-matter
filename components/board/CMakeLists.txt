idf_component_register(SRC_DIRS "${CONFIG_BOARD_NAME}"
                        INCLUDE_DIRS include
                        PRIV_INCLUDE_DIRS
                        REQUIRES button_driver led_driver)