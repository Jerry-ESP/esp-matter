# ESP Matter

## 1. Development Setup

This sections talks about setting up your development host, fetching the git repositories, and instructions for build and flash.

### 1.1 Host Setup

You should install drivers and support packages for your development host. Windows, Linux and Mac OS-X, are supported development hosts. Please see [Get Started](https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/index.html) for the host setup instructions.

### 1.2 Getting the Repositories

This only needs to be done once:
```
$ git clone --recursive https://github.com/espressif/esp-idf.git
$ cd esp-idf; git checkout v4.3; git submodule init; git submodule update --init --recursive;
$ ./install.sh
$ cd ..

$ git clone https://github.com/espressif/esp-matter.git
$ cd esp-matter; git submodule init; git submodule update --init --recursive;
$ source connectedhomeip/scripts/bootstrap.sh
$ cd ..
```

### 1.3 Configuring Environment

This needs to be done everytime a new terminal is opened:
```
$ cd esp-matter/examples/light/

$ export ESPPORT=/dev/cu.SLAB_USBtoUART (or /dev/ttyUSB0 or /dev/ttyUSB1 on Linux or COMxx on MinGW)
$ export IDF_PATH=/path/to/esp-idf
$ export ESP_MATTER_PATH=/path/to/esp-matter

$ . $IDF_PATH/export.sh
$ . $ESP_MATTER_PATH/export.sh
```

### 1.4 Building and Flashing the Firmware

Selecting board:
```
$ idf.py menuconfig
```
*   menuconfig -> ESP Matter Board Selection -> Select the supported board
    *   The boards here are dependent on the IDF_TARGET. If your board uses a different chip, set the correct target and try again.
    ```
    idf.py set-target esp32c3 (or esp32 or other supported targets)
    ```
    *   The other peripheral components like led_driver, button_driver, etc are selected based on the board selected.
    *   The configuration of the peripheral components can be found in `esp-matter/components/board/<board_name>/board.c`.
    *   If the board that you have is of a different revision, and is not working as expected, the gpio and other configuration can be changed in the `board.c` file.

Build and flash:
```
$ idf.py build
$ idf.py flash monitor
```

*   Note: If you are getting build errors like:
    ```
    ERROR: This script was called from a virtual environment, can not create a virtual environment again
    ```
    Run:
    ```
    pip install -r $IDF_PATH/requirements.txt
    ```

## 2. Test setup (Device controller setup)

### 2.1 Environment setup

This only needs to be done once:
```
$ cd esp-matter
$ ./connectedhomeip/scripts/build_python.sh -m platform
```

### 2.2 Begin

Start controller:
```
$ connectedhomeip/out/python_env/bin/chip-device-ctrl
```

*   The host machine should be connected to the same network.
*   The pairing code and descriminator can be set via menuconfig the options `Use Test Setup Pin Code` and `Use Test Setup discriminator`. By default they are `20202021` and `3840`.
*   The node_id is explicitly being set here to `12344321`. If this is not done, then the id will need to be read from the device logs. A different node_id can be used here. (This is represented in hexadecimal on the device and decimal i
n the controller.)

### 2.2 Commissioning

Connect:
```
chip-device-ctrl > ble-scan
chip-device-ctrl > connect -ble 3840 20202021 12344321
```

Commission: (Currently, this needs to be done on every reboot)
```
chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 12344321 0 0 ssid=str:${WIFI_SSID} credentials=str:${WIFI_PSK} breadcrumb=0 timeoutMs=1000
chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 12344321 0 0 networkID=str:${WIFI_SSID} breadcrumb=0 timeoutMs=1000
```

Close connection:
```
chip-device-ctrl > close-ble
```

### 2.3 Controlling

Check Device:
```
chip-device-ctrl > resolve 0 12344321
```

Control the device:
```
chip-device-ctrl > zcl OnOff On 12344321 1 1
chip-device-ctrl > zcl LevelControl MoveToLevel 12344321 1 1 level=10 transitionTime=0 optionMask=0 optionOverride=0
chip-device-ctrl > zcl LevelControl MoveToLevel 12344321 1 1 level=100 transitionTime=0 optionMask=0 optionOverride=0
chip-device-ctrl > zcl OnOff Toggle 12344321 1 1
```

### 2.4 End

Stop controller:
```
chip-device-ctrl > quit
```
