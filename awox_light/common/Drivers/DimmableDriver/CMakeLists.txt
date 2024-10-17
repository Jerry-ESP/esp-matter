idf_component_register(SRCS "DimmableDriver.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES TwDriver
                    )
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
