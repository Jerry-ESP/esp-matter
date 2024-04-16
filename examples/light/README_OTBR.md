# Light + OTBR

This example creates a Matter Over Wifi Light using the ESP Matter data model and an Openthread Border Router.


See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## 1. Additional Environment Setup

### The idf version

Please use the idf branch [support/light_otbr_5_2_1](https://glab.espressif.cn/esp-idf-preview/esp-idf-h2/tree/support/light_otbr_5_2_1)


## 2. OpenThread Border Router (OTBR) feature

### 2.1 Hardware Platform

See the [docs](https://github.com/espressif/esp-thread-br#hardware-platforms) for more information about the hardware platform.

### 2.2 Build

The sdkconfig file `sdkconfig.defaults.otbr` is provided to enable the OTBR feature on the light.
Build and flash the example with the sdkconfig file 'sdkconfig.defaults.otbr'

```
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults.otbr" set-target esp32s3 build
idf.py -p <PORT> erase-flash flash monitor
```

*Notes*: The Thread Border Router DevKit Board uses USB port.

### 2.3 Pair and Control

- Pairing the light via ble-wifi method with chip-tool command

```
./chip-tool pairing ble-wifi 1 wifi_ssid wifi_password 20202021 3840
```

- Control the Wifi Light via chip-tool command (On/Off cluster Toggle command)

```
./chip-tool onoff toggle 1 1
```

- Initializing a new Thread network dataset and commit it as the active one

```
matter esp ot_cli dataset init new
matter esp ot_cli dataset commit active
```

- Getting the operational dataset TLV-encoded string. The `<dataset_tlvs>` will be printed.

```
matter esp ot_cli dataset active -x

Done
0e080000000000010000000300000d35060004001fffe00208c62a4c5f80346de00708fd9ae261c39015460510874e399792fcb276c571dee6f71c260a030f4f70656e5468726561642d3861363301028a630410a6676a0f60ca6dd3af116bdefd2a21270c0402a0f7f8
Done

```

- Starting the Thread network

```
matter esp ot_cli ifconfig up
matter esp ot_cli thread start
```

- Pairing the Thread end-device(You can setup a matter over thread light on esp32h2 dev kit)

```
./chip-tool pairing ble-thread 2 hex:0e080000000000010000000300000d35060004001fffe00208c62a4c5f80346de00708fd9ae261c39015460510874e399792fcb276c571dee6f71c260a030f4f70656e5468726561642d3861363301028a630410a6676a0f60ca6dd3af116bdefd2a21270c0402a0f7f8 20202021 3840
```

- Control the Thread end-device via chip-tool command (On/Off cluster Toggle command)

```
./chip-tool onoff toggle 2 1
```

## 3. Setup a Matter over Thread light on esp32h2

### Build

The sdkconfig file `sdkconfig.defaults.esp32h2` is provided to enable the thread light.

```
idf.py set-target esp32h2
idf.py build
idf.py -p <PORT> erase-flash flash monitor
```
