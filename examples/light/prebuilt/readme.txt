The bin files flash address:
bootloader.bin       0x0
partition-table.bin  0xc000
ota_data_initial.bin 0x1d000
light.bin            0x20000

The flash command in linux:
esptool.py -p /dev/ttyUSB0 -b 1152000 write_flash 0x0 bootloader.bin 0xc000 partition-table.bin 0x1d000 ota_data_initial.bin 0x20000 light.bin

The zigbee touchlink switch flash command:
esptool.py -p /dev/ttyUSB0 -b 1152000 write_flash 0x0 esp32h2-zb-touchlink-merge.bin


