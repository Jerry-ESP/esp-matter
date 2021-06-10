# Minimal lighting exmaple

## Building and running the example application

### Build and flash firmware

The LED output pin is set by Kconfig option `LED_PIN`, by default it is PIN 12.

```bash
$ idf.py build
$ idf.py flash monitor
```

### Build the python device controller
```bash
$ cd ${HOME}/esp-matter/connnectedhomeip
$ ./scripts/build_python.sh --chip_mdns platform
```

## Pairing and controlling the device

### Connecting to the device with pairing code

The pairing code and descriminator is set via Kconfig option `Use Test Setup Pin Code` and `Use Test Setup discriminator`. By default they will be `20202021` and `3840`.

```bash
$ ${HOME}/esp-matter/connectedhomeip/out/python_env/bin/chip-device-ctrl
chip-device-ctrl > connect -ble 3840 20202021 12344321
```

### Commissioning the device to the WiFi network

The host machine shall be connected to the same network.

```bash
chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 12344321 0 0 ssid=str:${WIFI_SSID} credentials=str:${WIFI_PSK} breadcrumb=0 timeoutMs=1000

chip-device-ctrl > close-ble
```

### Controlling the lighting app

```bash
chip-device-ctrl > resolve 0 12344321

chip-device-ctrl > zcl OnOff On 12344321 1 1

chip-device-ctrl > zcl LevelControl MoveToLevel 12344321 1 1 level=10 transitionTime=0 optionMask=0 optionOverride=0

chip-device-ctrl > zcl LevelControl MoveToLevel 12344321 1 1 level=255 transitionTime=0 optionMask=0 optionOverride=0

chip-device-ctrl > zcl OnOff Toggle 12344321 1 1
```
