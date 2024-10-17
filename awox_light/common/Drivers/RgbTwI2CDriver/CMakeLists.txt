idf_component_register(SRCS "RgbTwI2CDriver.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver VirtualDriver
                    PRIV_REQUIRES TargetConfig
                    )

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)