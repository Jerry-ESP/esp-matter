idf_component_register(SRCS "RebootCounterManager.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES Data VirtualManager FlashDriver
                    PRIV_REQUIRES TaskConfig
                    )

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
