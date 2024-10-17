idf_component_register(SRCS "Color.cpp" "AwoxScene.cpp" "Api.cpp" "BluetoothData.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES TargetConfig
                    )
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)