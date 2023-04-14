# Notes
esp-box matter 
esp-idf 4.4 head 4c7d97e2bdbd26b1ad6adc6de8051888e1feec10
esp-matter head at ec70d32ecceed77e751b8cc42c69af4e674f2e21

## Chip-tool
Goto the folder of path 
/home/ali/work/learning/mattttttttt/esp-matter/connectedhomeip/connectedhomeip/scripts/examples

## steps to follow

### Flash all devices

- For BOX Node Id = 22
```
cd s3_box_switch && idf.py set-target esp32s3 build
cd build && esptool.py --chip esp32s3 merge_bin `cat flash_args` -o demo_box_s3_box_switch.bin
esptool.py -p /dev/ttyACM0 write_flash 0 demo_box_s3_box_switch.bin
idf.py -p /dev/ttyACM0 monitor

```

- For Fan Node Id = 21
```
cd fan && idf.py set-target esp32 build
cd build && esptool.py --chip esp32 merge_bin `cat flash_args` -o demo_box_fan.bin
esptool.py -p /dev/ttyUSB0 write_flash 0 demo_box_fan.bin
idf.py -p /dev/ttyUSB0 monitor


```

- For Light Node Id = 23
```
cd light && idf.py set-target esp32c3 build
cd build && esptool.py --chip esp32c3 merge_bin `cat flash_args` -o demo_box_light.bin
esptool.py -p /dev/ttyUSB1 write_flash 0 demo_box_light.bin
idf.py -p /dev/ttyUSB1 monitor

```

### Pairing them to esp-matter fabric
```   
out/debug/chip-tool pairing ble-wifi 22 Redmi_AE espressif 20202021 3840

```
```
out/debug/chip-tool pairing ble-wifi 21 Redmi_AE espressif 20202021 3840

```
```
out/debug/chip-tool pairing ble-wifi 23 Redmi_AE espressif 20202021 3840

```
### Write acl to the devices     
```
out/debug/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode":2, "subjects": [ 112233, 22 ], "targets": null}]' 23 0x0

```
```
out/debug/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode":2, "subjects": [ 112233, 22 ], "targets": null}]' 21 0x0 
```
### Binding 

```
out/debug/chip-tool binding write binding '[{"fabricIndex": 1, "node":21, "endpoint":1, "cluster":6}]' 22 2

```
```
out/debug/chip-tool binding write binding '[{"fabricIndex": 1, "node":23, "endpoint":1, "cluster":6}]' 22 1   

```
### Open commissioning for  HomePod
```
out/debug/chip-tool pairing open-commissioning-window 23 1 300 1000 3840

```
```
out/debug/chip-tool pairing open-commissioning-window 21 1 300 1000 3840 

```