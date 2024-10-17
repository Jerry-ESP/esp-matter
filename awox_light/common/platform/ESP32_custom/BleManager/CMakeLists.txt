idf_component_register(SRCS "BleGatt.cpp" "BleManager.cpp" "Characteristics/BlePairingChr.cpp" "Characteristics/BleDataCommandChr.cpp"
 "Characteristics/BleDataNotificationChr.cpp" "Characteristics/BleFastOtaChr.cpp"
                       INCLUDE_DIRS "." "Characteristics"
                       REQUIRES TargetConfig VirtualManager bt esp_partition app_update
                       PRIV_REQUIRES Data mbedtls TaskConfig
                       )

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)