idf_component_register(SRCS "."
                    INCLUDE_DIRS "."
                    REQUIRES Data
                    )

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)