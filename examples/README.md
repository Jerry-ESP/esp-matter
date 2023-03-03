## Demo Box

| Description                     | Example                 | Revision     |
| ------------------------------- | ----------------------- | ------------ |
| ESP32-C3 based light            | light                   | idf:v4.4.3   |
| ESP32 based 4 switch            | light_switch            | idf:v4.4.3   |
| ------------------------------- | ----------------------- | ------------ |
| ESP32 Fan                       | fan                     | idf:v4.4.3   |
| ESP32-S3 Fan Switch             | s3_box_switch           | idf:v4.4.3   |
| ------------------------------- | ----------------------- | ------------ |
| Zigbee Bridge                   | binaries/zigbee         |              |
| ESP32 Buzzer                    | buzzer                  | idf:v4.4.3   |
| ------------------------------- | ----------------------- | ------------ |
| Openthread Border Router        | binaries/thread         |              |
| ESP32-S3 LCD Screen             | lcd_screen              | idf:v4.4.3   |
| ESP32-H2-BETA2 Occupancy Sensor | occupancy_sensor_switch | fefb3a9b17   |
| ------------------------------- | ----------------------- | ------------ |

#### NOTE: Thread-Border-Router V1.0 can be compiled on below environment
- repo: https://github.com/espressif/esp-thread-br
- example: examples/basic_thread_border_router

- esp-idf: fefb3a9b17
- esp-thread-br: 6dd4ed5 

### Light and Light Switch

#### Build and Flash
```
# Light
cd light && idf.py set-target esp32c3 build
cd build && esptool.py --chip esp32c3 merge_bin `cat flash_args` -o demo_box_light.bin
esptool.py -p $ESPPORT write_flash 0 demo_box_light.bin

# Switch
cd light_switch && idf.py set-target esp32 build
cd build && esptool.py --chip esp32 merge_bin `cat flash_args` -o demo_box_light_switch.bin
esptool.py -p $ESPPORT write_flash 0 demo_box_light_switch.bin
```

#### Commission, groups, and binding 
Power on one device at a time and commission them, this is just to avoid commissioning specific device as we are not
using different discriminator.

NOTE: Also, after commissioning you can try toggling the device just to confirm that you commissioned the correct node.

```
# Light 1
./chip-tool pairing ble-wifi 31 $SSID $PASS 20202021 3840
# Light 2
./chip-tool pairing ble-wifi 32 $SSID $PASS 20202021 3840
# Light 3
./chip-tool pairing ble-wifi 33 $SSID $PASS 20202021 3840

# Switch
./chip-tool pairing ble-wifi 34 $SSID $PASS 20202021 3840

# Add them all to group
./chip-tool tests TestGroupDemoConfig --nodeId 31
./chip-tool tests TestGroupDemoConfig --nodeId 32
./chip-tool tests TestGroupDemoConfig --nodeId 33
./chip-tool tests TestGroupDemoConfig --nodeId 34

# Bind group to switch
./chip-tool binding write binding '[{"fabricIndex": 1, "group": 257}]' 34 1
```

### Fan and S3 Box Switch

#### Build and Flash
```
# Fan
cd fan && idf.py set-target esp32 build
cd build && esptool.py --chip esp32 merge_bin `cat flash_args` -o demo_box_fan.bin
esptool.py -p $ESPPORT write_flash 0 demo_box_fan.bin

# S3 Box Switch
cd s3_box_switch && idf.py set-target esp32s3 build
cd build && esptool.py --chip esp32s3 merge_bin `cat flash_args` -o demo_box_s3_box_switch.bin
esptool.py -p $ESPPORT write_flash 0 demo_box_s3_box_switch.bin
```

#### Commission, binding, and ACL
```
# Fan
./chip-tool pairing ble-wifi 21 $SSID $PASS 20202021 3840
./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode":2, "subjects": [ 112233, 22 ], "targets": null}]' 21 0x0

# Switch
./chip-tool pairing ble-wifi 22 $SSID $PASS 20202021 3840
./chip-tool binding write binding '[{"fabricIndex": 1, "node":21, "endpoint":1, "cluster":6}]' 22 0x1
```

### Zigbee Bridge and Buzzer

#### Build and Flash
```
# Zigbee RCP
esptool.py -p $ESPPORT write_flash 0 demo_box_zigbee_rcp.bin
# If writing blob gets stuck try writing with --no-stub option
esptool.py -p $ESPPORT --no-stub write_flash 0 demo_box_zigbee_rcp.bin

# Zigbee bridge
esptool.py -p $ESPPORT write_flash 0 demo_box_zigbee_bridge.bin

# Buzzer
cd buzzer && idf.py set-target esp32c3 build
cd build && esptool.py --chip esp32c3 merge_bin `cat flash_args` -o demo_box_buzzer.bin
esptool.py -p $ESPPORT write_flash 0 demo_box_buzzer.bin

# You may have to associate the door lock with bridge if it does not work
# Hold down the button on the door lock for few seconds till it starts blinking and then release it
# then probably it will associate with bridge
```

#### Commission, binding, and ACL
```
# Buzzer
./chip-tool pairing ble-wifi 11 $SSID $PASS 20202021 3840
./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode":2, "subjects": [ 112233, 12 ], "targets": null}]' 11 0x0

# Zigbee bridge
./chip-tool pairing ble-wifi 12 $SSID $PASS 20202021 3840
./chip-tool binding write binding '[{"fabricIndex": 1, "node":11, "endpoint":1, "cluster":6}]' 12 0x1
```

### LCD, OT-BR, and Occupancy Sensor

#### Build and Flash
```
# OT-BR
esptool.py -p $ESPPORT write_flash 0 demo_box_ot_br.bin

# LCD Screen
cd lcd_screen && idf.py set-target esp32s3 build
cd build && esptool.py --chip esp32s3 merge_bin `cat flash_args` -o demo_box_lcd_screen.bin
esptool -p $ESPPORT write_flash 0 demo_box_lcd_screen.bin

# Occupancy Sensor Switch
cd occupancy_light_switch && idf.py set-target esp32h2 build
cd build && esptool.py --chip esp32h2beta2 merge_bin `cat flash_args` -o demo_box_occupancy_light_switch.bin
esptool.py -p $ESPPORT write_flash 0 demo_box_occupancy_light_switch.bin
```

#### Commission, binding, and ACL
```
# LCD Screen
./chip-tool pairing ble-wifi 41 $SSID $PASS 20202021 3840
./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode":2, "subjects": [ 112233, 42 ], "targets": null}]' 41 0x0

# OT-BR
# Get the active dataset from the ot-br console using minicom/screen
minicom -D $ESPPORT -b 115200
# type in below command to get thread dataset 
dataset active -x

# Occupancy sensor
chip-tool pairing ble-thread 42 hex:<hex-string-active-dataset> 20202021 3840
./chip-tool binding write binding '[{"fabricIndex": 1, "node":41, "endpoint":1, "cluster":6}]' 42 0x1
```
