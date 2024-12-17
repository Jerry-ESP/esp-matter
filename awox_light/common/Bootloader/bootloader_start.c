/**
 * @file bootloader_start.c
 * @author AWOX
 * @brief Custom bootloader for compressed OTA
 *
 */

#include "esp_log.h"

#include "bootloader_common.h"
#include "bootloader_init.h"
#include "bootloader_utility.h"

#if defined(CONFIG_BOOTLOADER_COMPRESSED_ENABLED)
#include "bootloader_custom_ota.h"
#endif

static const char* TAG = "boot"; /**< @brief Espressif tag for Log */

static int selectPartitionNumber(bootloader_state_t* bs);

/**
 * @brief We arrive here after the ROM bootloader finished loading this second stage bootloader from flash.
 * The hardware is mostly uninitialized, flash cache is down and the app CPU is in reset.
 * We do have a stack, so we can do the initialization in C.
 */
void __attribute__((noreturn)) call_start_cpu0(void)
{
    // 1. Hardware initialization
    if (bootloader_init() != ESP_OK) {
        bootloader_reset();
    }

#ifdef CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP
    // If this boot is a wake up from the deep sleep then go to the short way,
    // try to load the application which worked before deep sleep.
    // It skips a lot of checks due to it was done before (while first boot).
    bootloader_utility_load_boot_image_from_deep_sleep();
    // If it is not successful try to load an application as usual.
#endif

    // 2. Select the number of boot partition
    bootloader_state_t bs = { 0 };
    int bootIndex = selectPartitionNumber(&bs);
    if (bootIndex == INVALID_INDEX) {
        bootloader_reset();
    }

#if defined(CONFIG_BOOTLOADER_COMPRESSED_ENABLED)
    // 2.1 Call custom OTA routine
    bootIndex = bootloader_custom_ota_main(&bs, bootIndex);
#endif

    // 3. Load the app image for booting
    bootloader_utility_load_boot_image(&bs, bootIndex);
}

/**
 * @brief Select the number of boot partition
 *
 * @param bs Bootloader state structure used to save read data
 * @return int The index of the selected boot partition
 */
static int selectPartitionNumber(bootloader_state_t* bs)
{
    int bootIndex;
    // 1. Load partition table
    if (!bootloader_utility_load_partition_table(bs)) {
        ESP_LOGE(TAG, "load partition table error!");
        bootIndex = INVALID_INDEX;
    } else {
        // 2. Select the number of boot partition
        bootIndex = bootloader_utility_get_selected_boot_partition(bs);
    }
    return bootIndex;
}

/**
 * @brief Return global reent struct if any newlib functions are linked to bootloader
 *
 * @return struct _reent* global reent struct
 */
struct _reent* __getreent(void)
{
    return _GLOBAL_REENT;
}
