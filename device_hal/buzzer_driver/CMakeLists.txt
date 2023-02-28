include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

idf_component_register(SRC_DIRS ${buzzer_type}
                    INCLUDE_DIRS include
                    PRIV_REQUIRES driver)
