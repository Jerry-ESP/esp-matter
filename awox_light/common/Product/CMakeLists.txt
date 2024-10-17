idf_component_register(SRCS "product_util.cpp" "matter_dac_verify.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_matter RebootCounterManager StorageManager
)