idf_component_register(SRCS "StorageManager.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES driver Data FlashDriver VirtualManager TargetConfig
                    PRIV_REQUIRES TaskConfig
                    )

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)