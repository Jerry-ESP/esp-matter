# Fan

This example creates a Fan device using the ESP Matter data model.

See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## 1. Additional Environment Setup

No additional setup is required.

## 2. Post Commissioning Setup

No additional setup is required.

## 3. Device Performance

### 3.1 Memory usage

The following is the Memory and Flash Usage.

-   `Bootup` == Device just finished booting up. Device is not
    commissionined or connected to wifi yet.
-   `After Commissioning` == Device is conneted to wifi and is also
    commissioned and is rebooted.
-   device used: esp32c3_devkit_m
-   tested on:
    [d947231](https://github.com/espressif/esp-matter/commit/d947231efe3fd0799aaa3cb0db809e79001f9f02)
    (2022-06-16)

|                         | Bootup | After Commissioning |
|:-                       |:-:     |:-:                  |
|**Free Internal Memory** |70KB    |135KB                |

**Flash Usage**: Firmware binary size: 1.4MB

This should give you a good idea about the amount of free memory that is
available for you to run your application's code.
