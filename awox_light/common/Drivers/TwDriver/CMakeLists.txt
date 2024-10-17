idf_component_register(SRCS "TwDriver.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES RgbTwDriver
                    )

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)