idf_component_register(SRCS "FlashDriver.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash Data Product
)
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
