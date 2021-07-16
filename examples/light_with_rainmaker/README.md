## 1. Adding ESP RainMaker

You can use this rainmaker_light example for RainMaker + Matter. Make sure to follow these additional steps along with the steps in the top level readme.

### 1.1 Getting the Repositories

This only needs to be done once:
```
$ git clone --recursive https://github.com/espressif/esp-rainmaker.git
```
Setup the RainMaker CLI from here: https://rainmaker.espressif.com/docs/cli-setup.html

### 1.2 Configuring Environment

This needs to be done everytime a new terminal is opened:
```
cd esp-matter/examples/rainmaker_light/

export ESP_RMAKER_PATH=/path/to/esp-rainmaker
```

### 1.3 RainMaker Claiming

This need to be done before flashing the firmware. Note the mac address of the device.

RainMaker CLI:
```
$ cd $ESP_RMAKER_PATH/cli
$ rainmaker.py claim --platform esp32 --mac <12-digit-mac-all-caps> --addr 0x3E0000 $ESPPORT
```

### 1.4 RainMaker User Node Association

This need to be done after commissioning.

RainMaker CLI:
```
$ rainmaker.py test --addnode <node-id>
```

This will print the console command to be run on the device:
```
add-user <user-id> <secret-key>
```

Copy-paste this command on the device console.
