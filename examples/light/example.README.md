This example demonstrates the process of commissioning Matter over Thread with a Zigbee Touchlink target device on the ESP32H2. Both Matter and Zigbee Touchlink commissioning are enabled upon device boot. The device can be commissioned using any protocols of Matter and Zigbee Touchlink at one moment. To re-commission the device, please long press the boot button more than 5 seconds, then release, the device will factoryreset.

In this documentation, the chip-tool and an ESP Thread Border Router (BR) are utilized for Matter commissioning, while an ESP Touchlink Initiator Switch device is employed for Zigbee Touchlink. However, please note that these methods are optional, and alternative approaches for commissioning the device can also be utilized.

Notice, if you need the QR code to commission the device, you can get the QR code [here](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html#commissioning).


## 0. Use the pre-built binary file to quick start

The pre-built binary files are [here](https://glab.espressif.cn/esp-matter-preview/esp-matter/-/tree/support/matter_over_thread_with_zigbee_tl_h4b3/examples/light/prebuilt), you can flash these bin files to ESP32H2 without compiling the project.

Install the esptool
```
python3 -m pip install esptool
```

Put the device into download mode
```
Press boot button and hold, then click the reset button
```
Download the binary files on your local host and then flash them to the boards.
```
cd <your-local-down-load-prebuilt-file-dir>
python3 -m esptool -p <your-local-port> -b 2000000 --before default_reset --after hard_reset --chip esp32h2  write_flash --flash_mode dio --flash_size 4MB --flash_freq 48m 0x0 bootloader.bin 0xc000 partition-table.bin 0x1d000 ota_data_initial.bin 0x20000 light.bin
```

Open a serial tool(For example, `screen`)
```
screen <your-local-port> 115200
```

## 1.Compile, flash and run the matter over thread with zigbee touchlink target device.

### Get the codes.
Totally two repos are needed:

* [esp-idf](https://glab.espressif.cn/esp-idf-preview/esp-idf-h2) is the development framework for the ESP boards, contains many common components.
* [esp-matter](https://glab.espressif.cn/esp-matter-preview/esp-matter/) is the official Matter development framework. It is used for compiling the firmware for the combined matter over thread and zigbee commissioning application.

1. Get esp-idf code

```
cd <your-local-workspace>

git clone ssh://git@glab.espressif.cn:8266/esp-idf-preview/esp-idf-h2.git
```
Run the `set-submodules-to-github` located inside the tools folder and update the submodules:

```
cd esp-idf-h2
git checkout support/matter_over_thread_with_zigbee_tl_h4b3_idf
./tools/set-submodules-to-github.sh
git submodule update --init --recursive --progress
./install.sh
. ./export.sh
```

Install the idf env, please refer to this [docs](https://glab.espressif.cn/esp-idf-preview/esp-idf-h2/wikis/01_Installing-ESP-IDF-H2-Preview-Version)
Notice: the source code has already been cloned, please skip the related step.

2. Get esp-matter code

```
cd <your-local-workspace>
git clone ssh://git@glab.espressif.cn:8266/esp-matter-preview/esp-matter.git
cd esp-matter
git submodule update --init --depth 1
cd ./connectedhomeip/connectedhomeip
./scripts/checkout_submodules.py --platform esp32 linux --shallow
cd ../..
./install.sh
cd ..
```

Install the matter env, please refer to this [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html#getting-the-repositories).
The chiptool will be installed in this step.

Notice: the source code has already been cloned, therefore, the related step can be skipped.


### Build, flash and run matter over thread and zigbee touchlink target device:

Please Open a new terminal(T-1) and run these commands:
```
cd <your-local-workspace>/esp-idf-h2
git checkout support/mot_zb_commissioning
git submodule update --init --recursive --progress
./install.sh
. ./export.sh
```

```
cd <your-local-workspace>/esp-matter
source ./export.sh
git checkout support/matter_over_thread_with_zigbee_tl_h4b3
cd examples/light
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.optimizeflash.esp32h2;sdkconfig.defaults" set-target esp32h2
idf.py build
idf.py -p <your-local-port> flash monitor
```
After booting, these logs will output from the device:
```
I (27) boot: ESP-IDF v5.3-dev-3462-g697c84314f 2nd stage bootloader
I (28) boot: compile time Oct 11 2024 16:55:13
I (29) boot: chip revision: v0.0
I (32) boot: Enabling RNG early entropy source...
I (37) boot: Partition Table:
I (41) boot: ## Label            Usage          Type ST Offset   Length
I (48) boot:  0 esp_secure_cert  unknown          3f 06 0000d000 00002000
I (55) boot:  1 nvs              WiFi data        01 02 00010000 0000c000
I (63) boot:  2 nvs_keys         NVS keys         01 04 0001c000 00001000
I (70) boot:  3 otadata          OTA data         01 00 0001d000 00002000
I (78) boot:  4 phy_init         RF data          01 01 0001f000 00001000
I (85) boot:  5 ota_0            OTA app          00 10 00020000 001c0000
I (93) boot:  6 fctry            WiFi data        01 02 001e0000 00006000
I (100) boot:  7 coredump         Unknown data     01 03 001e6000 00010000
I (108) boot:  8 zb_storage       Unknown data     01 81 001f6000 00004000
I (115) boot:  9 zb_fct           Unknown data     01 81 001fa000 00000400
I (123) boot: End of partition table
I (128) esp_image: segment 0: paddr=00020020 vaddr=42170020 size=26660h (157280) map
I (218) esp_image: segment 1: paddr=00046688 vaddr=40800000 size=09990h ( 39312) load
I (243) esp_image: segment 2: paddr=00050020 vaddr=42000020 size=16421ch (1458716) map
I (1002) esp_image: segment 3: paddr=001b4244 vaddr=40809990 size=03500h ( 13568) load
I (1012) esp_image: segment 4: paddr=001b774c vaddr=4080cf00 size=02338h (  9016) load
I (1019) esp_image: segment 5: paddr=001b9a8c vaddr=50000000 size=00004h (     4) load
I (1028) boot: Loaded app from partition at offset 0x20000
I (1029) boot: Disabling RNG early entropy source...
I (1042) cpu_start: Unicore app
I (1052) cpu_start: Pro cpu start user code
I (1053) cpu_start: cpu freq: 96000000 Hz
I (1053) app_init: Application information:
I (1056) app_init: Project name:     light
I (1060) app_init: App version:      1.0
I (1065) app_init: Compile time:     Oct 11 2024 16:55:02
I (1071) app_init: ELF file SHA256:  c548be5df...
I (1076) app_init: ESP-IDF:          v5.3-dev-3462-g697c84314f
I (1083) efuse_init: Min chip rev:     v0.0
I (1088) efuse_init: Max chip rev:     v0.99
I (1093) efuse_init: Chip rev:         v0.0
I (1098) heap_init: Initializing. RAM available for dynamic allocation:
I (1105) heap_init: At 40825520 len 00027E60 (159 KiB): RAM
I (1111) heap_init: At 4084D380 len 00002B60 (10 KiB): RAM
I (1120) spi_flash: detected chip: generic
I (1122) spi_flash: flash io: dio
W (1126) spi_flash: Detected size(8192k) larger than the size in the binary image header(4096k). Using the size in the binary image header.
W (1350) i2c: This driver is an old driver, please migrate your application code to adapt `driver/i2c_master.h`
I (1363) sleep: Configure to isolate all GPIO pins in sleep state
I (1368) sleep: Enable automatic switching of GPIO sleep configuration
I (1375) coexist: coex firmware version: 7fff68fa5
I (1381) main_task: Started on CPU0
I (1381) main_task: Calling app_main()
I (1411) phy: phy_version: 101,0, e523c93, May 16 2024, 10:21:10
I (1411) phy: libbtbb version: cdc72df, May 16 2024, 10:21:22
W (1421) rmt: channel resolution loss, real=10666666
I (1421) button: IoT Button Version: 3.3.1
I (1471) app_main: Light created with endpoint_id 1
I (1491) chip[DL]: NVS set: chip-counters/reboot-count = 1 (0x1)
I (1491) chip[DL]: NVS set: chip-counters/total-hours = 0 (0x0)
I (1501) chip[DL]: NVS set: chip-config/unique-id = "CFA3F6591D31BCCA"
I (1501) chip[DL]: Real time clock set to ld (0001/946684800/00 100:00:01 UTC)
I (1501) BLE_INIT: Using main XTAL as clock source
I (1511) BLE_INIT: ble controller commit:[39c6e05]
I (1521) OPENTHREAD: Host connection mode none
I (1531) CHIP[DL]: BLE host-controller synced
I (1551) OPENTHREAD: OpenThread attached to netif
I (1561) chip[DL]: OpenThread started: OK
I (1561) chip[DL]: Setting OpenThread device type to ROUTER
I (2051) chip[SVR]: Initializing subscription resumption storage...
I (2061) chip[SVR]: Server initializing...
I (2061) chip[TS]: Last Known Good Time: [unknown]
I (2061) chip[TS]: Setting Last Known Good Time to firmware build time 2023-10-14T01:16:48
I (2071) chip[DMG]: AccessControl: initializing
I (2071) chip[DMG]: Examples::AccessControlDelegate::Init
I (2081) chip[DMG]: AccessControl: setting
I (2081) chip[DMG]: DefaultAclStorage: initializing
I (2091) chip[DMG]: DefaultAclStorage: 0 entries loaded
I (2111) chip[ZCL]: Using ZAP configuration...
I (2111) esp_matter_cluster: Cluster plugin init common callback
I (2111) chip[DMG]: AccessControlCluster: initializing
I (2111) chip[ZCL]: 0x4217417c ep 0 clus 0x0000_0030 attr 0x0000_0000 not supported
I (2121) chip[ZCL]: Initiating Admin Commissioning cluster.
I (2131) chip[DIS]: Updating services using commissioning mode 1
E (2131) chip[DIS]: Failed to remove advertised services: 3
I (2141) chip[DIS]: Advertise commission parameter vendorID=65521 productID=32768 discriminator=3840/15 cm=1 cp=0
E (2151) chip[DIS]: Failed to advertise commissionable node: 3
E (2161) chip[DIS]: Failed to finalize service update: 3
I (2161) chip[IN]: CASE Server enabling CASE session setups
I (2171) chip[SVR]: Joining Multicast groups
I (2181) chip[SVR]: Server Listening...
I (2181) esp_matter_core: Dynamic endpoint 0 added
I (2191) esp_matter_attribute: ********** W : Endpoint 0x0001's Cluster 0x00000003's Attribute 0x00000001 is 0 **********
---------app_main: Current Free Memory: 71972, Minimum Ever Free Size: 71944, Largest Free Block: 59392------------
---------app_main: Current Free Memory: 71972, Minimum Ever Free Size: 71944, Largest Free Block: 59392------------
I (2221) esp_matter_attribute: ********** W : Endpoint 0x0001's Cluster 0x00000004's Attribute 0x00000000 is 128 **********
---------app_main: Current Free Memory: 71972, Minimum Ever Free Size: 71944, Largest Free Block: 59392------------
---------app_main: Current Free Memory: 71972, Minimum Ever Free Size: 71944, Largest Free Block: 59392------------
I (2251) esp_matter_attribute: ********** W : Endpoint 0x0001's Cluster 0x00000004's Attribute 0x0000FFFC is <invalid type: 0> **********
---------app_main: Current Free Memory: 71972, Minimum Ever Free Size: 71944, Largest Free Block: 59392------------
I (2271) chip[ZCL]: 0x4217417c ep 1 clus 0x0000_0062 attr 0x0000_0000 not supported
I (2281) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000006's Attribute 0x0000FFFC is 1 **********
I (2291) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000006's Attribute 0x00004003 is null **********
I (2301) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000006's Attribute 0x00000000 is 1 **********
I (2321) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000006's Attribute 0x00000000 is 1 **********
I (2331) chip[ZCL]: Endpoint 1 On/off already set to new value
I (2331) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000008's Attribute 0x00000002 is 1 **********
I (2351) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000008's Attribute 0x00000003 is 254 **********
I (2361) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000008's Attribute 0x0000FFFC is 3 **********
I (2371) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000008's Attribute 0x00000000 is 64 **********
I (2381) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000008's Attribute 0x00004000 is 64 **********
I (2391) esp_matter_attribute: ********** W : Endpoint 0x0001's Cluster 0x00000008's Attribute 0x00000000 is 64 **********
---------app_main: Current Free Memory: 71972, Minimum Ever Free Size: 71944, Largest Free Block: 59392------------
---------app_main: Current Free Memory: 71972, Minimum Ever Free Size: 71944, Largest Free Block: 59392------------
I (2421) esp_matter_attribute: ********** R : Endpoint 0x0001's Cluster 0x00000300's Attribute 0x00004010 is null **********
I (2441) esp_matter_core: Dynamic endpoint 1 added
I (2451) esp_matter_core: Cannot find minimum unused endpoint_id, try to find in the previous namespace
I (2451) esp_matter_core: Failed to open node namespace
I (2461) chip[DL]: Configuring CHIPoBLE advertising (interval 25 ms, connectable)
---------app_main: Current Free Memory: 65628, Minimum Ever Free Size: 65164, Largest Free Block: 53248------------
I (2571) main_task: Returned from app_main()
I (2571) chip[DL]: CHIPoBLE advertising started
I (2611) app_main: Commissioning window opened
I (2621) ESP_TL_ON_OFF_LIGHT: ZDO signal: ZDO Config Ready (0x17), status: UNKNOWN ERROR
I (2621) ESP_TL_ON_OFF_LIGHT: Initialize Zigbee stack
I (2641) ESP_TL_ON_OFF_LIGHT: Device started up in  factory-reset mode
---------app_main: Current Free Memory: 68812, Minimum Ever Free Size: 64332, Largest Free Block: 53248------------
I (2661) ESP_TL_ON_OFF_LIGHT: Touchlink target is ready, awaiting commissioning
I (5411) esp_matter_core: Store the deferred attribute 0x0 of cluster 0x8 on endpoint 0x1
```


## 2.Matter Comissioning Using Chiptool
Notice: make sure your host does have a Bluetooth dongle.

### Get the BR codes

* [esp-thread-br](https://github.com/espressif/esp-thread-br) is the official ESP Thread Border Router SDK. It is used for compiling the BR firmware.

```
cd <your-local-workspace>
git clone git@github.com:espressif/esp-thread-br.git
```
### Build, flash and run thread BR:

[ESP Thread Border Router Board](https://github.com/espressif/esp-thread-br?tab=readme-ov-file#esp-thread-border-router-board) is needed.


Please Open a new terminal(T-2) and run these commands:
Tips, before starting, you can firstly take a look at our BR [codelabs](https://docs.espressif.com/projects/esp-thread-br/en/latest/dev-guide/build_and_run.html#)
```
cd <your-local-workspace>/esp-idf-h2
git checkout v5.1.2
git submodule update --init --recursive
./install.sh
. ./export.sh
cd examples/openthread/ot_rcp
idf.py set-target esp32h2
idf.py build

cd <your-local-workspace>/esp-thread-br
git checkout v1.0
cd examples/basic_thread_border_router
```

Tips, if there is an error when you try to switch the idf branch:
```
error: pathspec v5.1.2' did not match any file(s) known to git
```
The github remote might be needed, you can add it via these commands:

```
cd <your-local-workspace>/esp-idf-h2
git remote add github git@github.com:espressif/esp-idf.git
git fetch github
```

For detailed instructions on enabling auto-start mode and configuring Wi-Fi settings, please refer to the documentation provided [here](https://docs.espressif.com/projects/esp-thread-br/en/latest/dev-guide/build_and_run.html#wi-fi-based-thread-border-router)
Notice: make sure your host connects the same wifi network with BR.
```
idf.py set-target esp32s3
idf.py menuconfig
```
then build and flash the BR:

```
idf.py build
idf.py -p <your-local-port> flash monitor
```

You will see these logs if all things go as expected
```
I (0) cpu_start: App cpu up.
I (422) cpu_start: Pro cpu start user code
I (422) cpu_start: cpu freq: 160000000 Hz
I (422) cpu_start: Application information:
I (422) cpu_start: Project name:     esp_ot_br
I (423) cpu_start: App version:      v1.0-28-g423449e
I (423) cpu_start: Compile time:     Mar 19 2024 20:09:11
I (423) cpu_start: ELF file SHA256:  1fb6a6a82bb13515...
I (424) cpu_start: ESP-IDF:          v5.1.3
I (424) cpu_start: Min chip rev:     v0.0
I (424) cpu_start: Max chip rev:     v0.99
I (424) cpu_start: Chip rev:         v0.2
I (424) heap_init: Initializing. RAM available for dynamic allocation:
I (425) heap_init: At 3FCAAF50 len 0003E7C0 (249 KiB): DRAM
I (425) heap_init: At 3FCE9710 len 00005724 (21 KiB): STACK/DRAM
I (425) heap_init: At 3FCF0000 len 00008000 (32 KiB): DRAM
I (426) heap_init: At 600FE010 len 00001FD8 (7 KiB): RTCRAM
I (427) spi_flash: detected chip: generic
I (427) spi_flash: flash io: dio
I (427) sleep: Configure to isolate all GPIO pins in sleep state
I (428) sleep: Enable automatic switching of GPIO sleep configuration
I (428) app_start: Starting scheduler on CPU0
I (429) app_start: Starting scheduler on CPU1
I (429) main_task: Started on CPU0
I (439) main_task: Calling app_main()
I (509) RCP_UPDATE: RCP: using update sequence 0
I (509) uart: ESP_INTR_FLAG_IRAM flag not set while CONFIG_UART_ISR_IN_IRAM is enabled, flag updated
I (509) OPENTHREAD: spinel UART interface initialization completed
I (509) main_task: Returned from app_main()
I(519) OPENTHREAD:[I] P-RadioSpinel-: RCP reset: RESET_POWER_ON
I(519) OPENTHREAD:[I] P-RadioSpinel-: Software reset RCP successfully
I(559) OPENTHREAD:[I] ChildSupervsn-: Timeout: 0 -> 190
I(579) OPENTHREAD:[I] Settings------: Read NetworkInfo {rloc:0x2c00, extaddr:a2a088531af9502b, role:leader, mode:0x0f, version:4, keyseq:0x0, ...
I(579) OPENTHREAD:[I] Settings------: ... pid:0x511fcd0b, mlecntr:0xacb8, maccntr:0x469e, mliid:e43079509d8147ba}
I(589) OPENTHREAD:[I] Settings------: Read ChildInfo {rloc:0x2c01, extaddr:b6e310141d0ea456, timeout:240, mode:0x0c, version:4}
I (589) esp_ot_br: Internal RCP Version: openthread-esp32/e7771c75bd-456c44828; esp32h2;  2024-03-19 10:13:56 UTC
I (589) esp_ot_br: Running  RCP Version: openthread-esp32/e7771c75bd-456c44828; esp32h2;  2024-03-19 10:13:56 UTC
I (599) OPENTHREAD: OpenThread attached to netif
> I (599) example_connect: Start example_connect.
I (599) pp: pp rom version: e7ae62f
I (599) net80211: net80211 rom version: e7ae62f
I (609) wifi:wifi driver task: 3fcae240, prio:23, stack:6144, core=0
I (609) wifi:wifi firmware version: 0016c4d
I (609) wifi:wifi certification version: v7.0
I (609) wifi:config NVS flash: enabled
I (609) wifi:config nano formating: enabled
I (609) wifi:Init data frame dynamic rx buffer num: 32
I (609) wifi:Init static rx mgmt buffer num: 5
I (609) wifi:Init management short buffer num: 32
I (609) wifi:Init dynamic tx buffer num: 32
I (609) wifi:Init static tx FG buffer num: 2
I (609) wifi:Init static rx buffer size: 1600
I (609) wifi:Init static rx buffer num: 10
I (609) wifi:Init dynamic rx buffer num: 32
I (609) wifi_init: rx ba win: 6
I (609) wifi_init: tcpip mbox: 32
I (609) wifi_init: udp mbox: 6
I (609) wifi_init: tcp mbox: 6
I (609) wifi_init: tcp tx win: 5760
I (609) wifi_init: tcp rx win: 5760
I (609) wifi_init: tcp mss: 1440
I (609) wifi_init: WiFi IRAM OP enabled
I (609) wifi_init: WiFi RX IRAM OP enabled
I (619) phy_init: phy_version 640,cd64a1a,Jan 24 2024,17:28:12
I (679) wifi:mode : sta (48:27:e2:14:4d:3c)
I (679) wifi:enable tsf
I (679) example_connect: Connecting to matter1_1...
I (689) example_connect: Waiting for IP(s)
I (3089) wifi:new:<6,2>, old:<1,0>, ap:<255,255>, sta:<6,2>, prof:1
I (3569) wifi:state: init -> auth (b0)
I (3579) wifi:state: auth -> assoc (0)
I (3599) wifi:state: assoc -> run (10)
I (3609) wifi:<ba-add>idx:0 (ifx:0, 04:f9:f8:5e:83:0a), tid:5, ssn:125, winSize:64
I (3699) wifi:connected with matter1_1, aid = 4, channel 6, 40D, bssid = 04:f9:f8:5e:83:0a
I (3709) wifi:security: WPA2-PSK, phy: bgn, rssi: -39
I (3709) wifi:pm start, type: 1
I (3709) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (3709) wifi:set rx beacon pti, rx_bcn_pti: 0, bcn_timeout: 25000, mt_pti: 0, mt_time: 10000
I (3799) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (4709) esp_netif_handlers: example_netif_sta ip: 192.168.0.104, mask: 255.255.255.0, gw: 192.168.0.1
I (4709) example_connect: Got IPv4 event: Interface "example_netif_sta" address: 192.168.0.104
I (5509) example_connect: Got IPv6 event: Interface "example_netif_sta" address: fe80:0000:0000:0000:4a27:e2ff:fe14:4d3c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (5509) example_common: Connected to example_netif_sta
I (5509) example_common: - IPv4 address: 192.168.0.104,
I (5509) example_common: - IPv6 address: fe80:0000:0000:0000:4a27:e2ff:fe14:4d3c, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (5509) wifi:Set ps type: 0, coexist: 0
I(5519) OPENTHREAD:[N] RoutingManager: BR ULA prefix: fd8f:e9a2:bfcc::/48 (loaded)
I(5529) OPENTHREAD:[N] RoutingManager: Local on-link prefix: fdfb:ef67:cd94:f351::/64
I (5539) OPENTHREAD: Platform UDP bound to port 49153
I(5539) OPENTHREAD:[N] Mle-----------: Role disabled -> detached
I (5579) OT_STATE: netif up
I (5579) OPENTHREAD: NAT64 ready
> I (5729) wifi:<ba-add>idx:1 (ifx:0, 04:f9:f8:5e:83:0a), tid:0, ssn:472, winSize:64
I(32599) OPENTHREAD:[N] Mle-----------: RLOC16 2c00 -> fffe
I(33339) OPENTHREAD:[N] Mle-----------: Attach attempt 1, AnyPartition reattaching with Active Dataset
I(39919) OPENTHREAD:[N] RouterTable---: Allocate router id 11
I(39929) OPENTHREAD:[N] Mle-----------: RLOC16 fffe -> 2c00
I(39929) OPENTHREAD:[N] Mle-----------: Role detached -> leader
I(39929) OPENTHREAD:[N] Mle-----------: Partition ID 0x1ab5923e
```
Get the dataset from BR, this is used for commissioning the matter over thread device.
Notice, this command should be executed on the BR.
```
> dataset active -x

0e080000000000010000000300000d35060004001fffe00208fbef67cd9424f3510708fd5187beb6538eef05103b286eabfd1bfe1bd9ecd3aa78847832030f4f70656e5468726561642d646265340102dbe40410391246ea559f61fafedee495fad587b80c0402a0f7f8
```

### Use chiptool to comission the device.

Please Open a new terminal(T-3) and run these commands:
```
cd <your-local-workspace>/esp-matter/connectedhomeip/connectedhomeip/out/host
./chip-tool pairing ble-thread 1 hex:<the-dataset-get-from-BR> 20202021 3840
```

```
for example:
./chip-tool pairing ble-thread 1 hex:0e080000000000010000000300000d35060004001fffe00208fbef67cd9424f3510708fd5187beb6538eef05103b286eabfd1bfe1bd9ecd3aa78847832030f4f70656e5468726561642d646265340102dbe40410391246ea559f61fafedee495fad587b80c0402a0f7f8 20202021 3840
```
You will see
```
[1710928172.584092][6252:6257] CHIP:EM: Found matching exchange: 39127i, Delegate: 0x7fc56001f7a8
[1710928172.584103][6252:6257] CHIP:EM: Rxd Ack; Removing MessageCounter:133358555 from Retrans Table on exchange 39127i
[1710928172.584111][6252:6257] CHIP:DMG: ICR moving to [ResponseRe]
[1710928172.584124][6252:6257] CHIP:DMG: InvokeResponseMessage =
[1710928172.584128][6252:6257] CHIP:DMG: {
[1710928172.584132][6252:6257] CHIP:DMG:        suppressResponse = false,
[1710928172.584136][6252:6257] CHIP:DMG:        InvokeResponseIBs =
[1710928172.584143][6252:6257] CHIP:DMG:        [
[1710928172.584146][6252:6257] CHIP:DMG:                InvokeResponseIB =
[1710928172.584153][6252:6257] CHIP:DMG:                {
[1710928172.584157][6252:6257] CHIP:DMG:                        CommandDataIB =
[1710928172.584162][6252:6257] CHIP:DMG:                        {
[1710928172.584166][6252:6257] CHIP:DMG:                                CommandPathIB =
[1710928172.584171][6252:6257] CHIP:DMG:                                {
[1710928172.584175][6252:6257] CHIP:DMG:                                        EndpointId = 0x0,
[1710928172.584182][6252:6257] CHIP:DMG:                                        ClusterId = 0x30,
[1710928172.584186][6252:6257] CHIP:DMG:                                        CommandId = 0x5,
[1710928172.584191][6252:6257] CHIP:DMG:                                },
[1710928172.584195][6252:6257] CHIP:DMG:[1710928172.584200][6252:6257] CHIP:DMG:CommandFields =
[1710928172.584204][6252:6257] CHIP:DMG:                                {
[1710928172.584208][6252:6257] CHIP:DMG:                                        0x0 = 0,
[1710928172.584212][6252:6257] CHIP:DMG:                                        0x1 = "" (0 chars),
[1710928172.584216][6252:6257] CHIP:DMG:                                },
[1710928172.584219][6252:6257] CHIP:DMG:                        },
[1710928172.584224][6252:6257] CHIP:DMG:
[1710928172.584227][6252:6257] CHIP:DMG:                },
[1710928172.584231][6252:6257] CHIP:DMG:
[1710928172.584234][6252:6257] CHIP:DMG:        ],
[1710928172.584239][6252:6257] CHIP:DMG:
[1710928172.584242][6252:6257] CHIP:DMG:        InteractionModelRevision = 11
[1710928172.584245][6252:6257] CHIP:DMG: },
[1710928172.584268][6252:6257] CHIP:DMG: Received Command Response Data, Endpoint=0 Cluster=0x0000_0030 Command=0x0000_0005
[1710928172.584279][6252:6257] CHIP:CTL: Received CommissioningComplete response, errorCode=0
[1710928172.584288][6252:6257] CHIP:CTL: Successfully finished commissioning step 'SendComplete'
[1710928172.584292][6252:6257] CHIP:CTL: Commissioning stage next step: 'SendComplete' -> 'Cleanup'
[1710928172.584300][6252:6257] CHIP:CTL: Performing next commissioning step 'Cleanup'
[1710928172.584305][6252:6257] CHIP:DIS: Closing all BLE connections
[1710928172.584311][6252:6257] CHIP:IN: Clearing BLE pending packets.
[1710928172.584401][6252:6257] CHIP:BLE: Auto-closing end point's BLE connection.
[1710928172.584406][6252:6257] CHIP:DL: Closing BLE GATT connection (con 0x7fc5580391a0)
[1710928172.584423][6252:6256] CHIP:DL: BluezDisconnect peer=CF:05:58:FB:B2:49
[1710928175.407772][6252:6257] CHIP:IN: SecureSession[0x7fc560005dc0]: MarkForEviction Type:1 LSID:3025
[1710928175.408011][6252:6256] CHIP:DL: Bluez disconnected
[1710928175.408892][6252:6256] CHIP:DL: Bluez notify CHIPoBluez connection disconnected
[1710928175.408878][6252:6257] CHIP:SC: SecureSession[0x7fc560005dc0, LSID:3025]: State change 'kActive' --> 'kPendingEviction'
[1710928175.408922][6252:6257] CHIP:IN: SecureSession[0x7fc560005dc0]: Released - Type:1 LSID:3025
[1710928175.408936][6252:6257] CHIP:CTL: Successfully finished commissioning step 'Cleanup'
[1710928175.408944][6252:6257] CHIP:TOO: Device commissioning completed with success
[1710928175.408969][6252:6257] CHIP:DMG: ICR moving to [AwaitingDe]
[1710928175.409039][6252:6257] CHIP:EM: <<< [E:39127i S:3026 M:133358556 (Ack:205927119)] (S) Msg TX to 1:0000000000000001 [C026] [UDP:[fd8f:e9a2:bfcc:1:3a4d:e5d7:1e2b:fcda%wlx0013eff81090]:5540] --- Type 0000:10 (SecureChannel:StandaloneAck)
[1710928175.409090][6252:6257] CHIP:EM: Flushed pending ack for MessageCounter:205927119 on exchange 39127i
[1710928175.409101][6252:6257] CHIP:DL: HandlePlatformSpecificBLEEvent 16388
[1710928175.409106][6252:6257] CHIP:BLE: no endpoint for unsub complete
[1710928175.409172][6252:6252] CHIP:CTL: Shutting down the commissioner
[1710928175.409179][6252:6252] CHIP:CTL: Shutting down the controller
[1710928175.409185][6252:6252] CHIP:IN: Expiring all sessions for fabric 0x1!!
[1710928175.409189][6252:6252] CHIP:IN: SecureSession[0x7fc560029ca0]: MarkForEviction Type:2 LSID:3026
[1710928175.409193][6252:6252] CHIP:SC: SecureSession[0x7fc560029ca0, LSID:3026]: State change 'kActive' --> 'kPendingEviction'
[1710928175.409197][6252:6252] CHIP:IN: SecureSession[0x7fc560029ca0]: Released - Type:2 LSID:3026
[1710928175.409208][6252:6252] CHIP:FP: Forgetting fabric 0x1
[1710928175.409226][6252:6252] CHIP:TS: Pending Last Known Good Time: 2023-10-14T01:16:48
[1710928175.409274][6252:6252] CHIP:TS: Previous Last Known Good Time: 2023-10-14T01:16:48
[1710928175.409278][6252:6252] CHIP:TS: Reverted Last Known Good Time to previous value
[1710928175.409287][6252:6252] CHIP:CTL: Shutting down the commissioner
[1710928175.409291][6252:6252] CHIP:CTL: Shutting down the controller
[1710928175.409295][6252:6252] CHIP:CTL: Shutting down the System State, this will teardown the CHIP Stack
[1710928175.409338][6252:6252] CHIP:DMG: All ReadHandler-s are clean, clear GlobalDirtySet
[1710928175.409353][6252:6252] CHIP:FP: Shutting down FabricTable
[1710928175.409358][6252:6252] CHIP:TS: Pending Last Known Good Time: 2023-10-14T01:16:48
[1710928175.409378][6252:6252] CHIP:TS: Previous Last Known Good Time: 2023-10-14T01:16:48
[1710928175.409381][6252:6252] CHIP:TS: Reverted Last Known Good Time to previous value
[1710928175.409438][6252:6252] CHIP:DL: writing settings to file (/tmp/chip_counters.ini-mh10xU)
[1710928175.409538][6252:6252] CHIP:DL: renamed tmp file to file (/tmp/chip_counters.ini)
[1710928175.409549][6252:6252] CHIP:DL: NVS set: chip-counters/total-operational-hours = 0 (0x0)
[1710928175.409558][6252:6252] CHIP:DL: Inet Layer shutdown
[1710928175.409565][6252:6252] CHIP:DL: BLE shutdown
[1710928175.410112][6252:6252] CHIP:DL: System Layer shutdown
```

And toggle the light:
```
./chip-tool onoff toggle 1 1
```
You will see the logs:

```
[1710928186.526226][9180:9183] CHIP:DMG: InvokeResponseMessage =
[1710928186.526230][9180:9183] CHIP:DMG: {
[1710928186.526233][9180:9183] CHIP:DMG:        suppressResponse = false,
[1710928186.526236][9180:9183] CHIP:DMG:        InvokeResponseIBs =
[1710928186.526241][9180:9183] CHIP:DMG:        [
[1710928186.526286][9180:9183] CHIP:DMG:                InvokeResponseIB =
[1710928186.526353][9180:9183] CHIP:DMG:                {
[1710928186.526377][9180:9183] CHIP:DMG:                        CommandStatusIB =
[1710928186.526405][9180:9183] CHIP:DMG:                        {
[1710928186.526409][9180:9183] CHIP:DMG:                                CommandPathIB =
[1710928186.526414][9180:9183] CHIP:DMG:                                {
[1710928186.526419][9180:9183] CHIP:DMG:                                        EndpointId = 0x1,
[1710928186.526424][9180:9183] CHIP:DMG:                                        ClusterId = 0x6,
[1710928186.526428][9180:9183] CHIP:DMG:                                        CommandId = 0x2,
[1710928186.526432][9180:9183] CHIP:DMG:                                },
[1710928186.526441][9180:9183] CHIP:DMG:
[1710928186.526455][9180:9183] CHIP:DMG:                                StatusIB =
[1710928186.526470][9180:9183] CHIP:DMG:                                {
[1710928186.526504][9180:9183] CHIP:DMG:                                        status = 0x00 (SUCCESS),
[1710928186.526520][9180:9183] CHIP:DMG:                                },
[1710928186.526526][9180:9183] CHIP:DMG:
[1710928186.526538][9180:9183] CHIP:DMG:                        },
[1710928186.526553][9180:9183] CHIP:DMG:
[1710928186.526564][9180:9183] CHIP:DMG:                },
[1710928186.526576][9180:9183] CHIP:DMG:
[1710928186.526579][9180:9183] CHIP:DMG:        ],
[1710928186.526585][9180:9183] CHIP:DMG:
[1710928186.526589][9180:9183] CHIP:DMG:        InteractionModelRevision = 11
[1710928186.526593][9180:9183] CHIP:DMG: },
[1710928186.526609][9180:9183] CHIP:DMG: Received Command Response Status for Endpoint=1 Cluster=0x0000_0006 Command=0x0000_0002 Status=0x0
[1710928186.526626][9180:9183] CHIP:DMG: ICR moving to [AwaitingDe]
[1710928186.526687][9180:9183] CHIP:EM: <<< [E:27080i S:31401 M:111179352 (Ack:264833837)] (S) Msg TX to 1:0000000000000001 [C026] [UDP:[fd8f:e9a2:bfcc:1:3a4d
:e5d7:1e2b:fcda%wlx0013eff81090]:5540] --- Type 0000:10 (SecureChannel:StandaloneAck)
[1710928186.526733][9180:9183] CHIP:EM: Flushed pending ack for MessageCounter:264833837 on exchange 27080i
[1710928186.526800][9180:9180] CHIP:CTL: Shutting down the commissioner
[1710928186.526809][9180:9180] CHIP:CTL: Shutting down the controller
[1710928186.526819][9180:9180] CHIP:IN: Expiring all sessions for fabric 0x1!!
[1710928186.526824][9180:9180] CHIP:IN: SecureSession[0x7f9fb4005dc0]: MarkForEviction Type:2 LSID:31401
[1710928186.526828][9180:9180] CHIP:SC: SecureSession[0x7f9fb4005dc0, LSID:31401]: State change 'kActive' --> 'kPendingEviction'
[1710928186.526832][9180:9180] CHIP:IN: SecureSession[0x7f9fb4005dc0]: Released - Type:2 LSID:31401
[1710928186.526837][9180:9180] CHIP:FP: Forgetting fabric 0x1
[1710928186.526844][9180:9180] CHIP:TS: Pending Last Known Good Time: 2023-10-14T01:16:48
[1710928186.526882][9180:9180] CHIP:TS: Previous Last Known Good Time: 2023-10-14T01:16:48
[1710928186.526886][9180:9180] CHIP:TS: Reverted Last Known Good Time to previous value
[1710928186.526897][9180:9180] CHIP:CTL: Shutting down the commissioner
[1710928186.526900][9180:9180] CHIP:CTL: Shutting down the controller
[1710928186.526904][9180:9180] CHIP:CTL: Shutting down the System State, this will teardown the CHIP Stack
[1710928186.526966][9180:9180] CHIP:DMG: All ReadHandler-s are clean, clear GlobalDirtySet
[1710928186.526993][9180:9180] CHIP:FP: Shutting down FabricTable
[1710928186.526998][9180:9180] CHIP:TS: Pending Last Known Good Time: 2023-10-14T01:16:48
[1710928186.527018][9180:9180] CHIP:TS: Previous Last Known Good Time: 2023-10-14T01:16:48
[1710928186.527023][9180:9180] CHIP:TS: Reverted Last Known Good Time to previous value
[1710928186.527093][9180:9180] CHIP:DL: writing settings to file (/tmp/chip_counters.ini-3PheWl)
[1710928186.527190][9180:9180] CHIP:DL: renamed tmp file to file (/tmp/chip_counters.ini)
[1710928186.527207][9180:9180] CHIP:DL: NVS set: chip-counters/total-operational-hours = 0 (0x0)
[1710928186.527211][9180:9180] CHIP:DL: Inet Layer shutdown
[1710928186.527216][9180:9180] CHIP:DL: BLE shutdown
[1710928186.527220][9180:9180] CHIP:DL: System Layer shutdown
```
And on the matter over thread with zigbee touchlink target device, you will see the light is toggled.

### Finish the test

On the terminal(T-1), Quit the monitor by pressing Ctrl + `]` on the keyboard. And then erase the flash of the board to prepare for the next test.
```
idf.py -p <your-local-port> erase-flash
```

## 3.Zigbee Touchlink Commissioning uses Zigbee Touchlink Initiator:

### Get the ESP touchlink intiator code:

* [esp-zigbee-sdk](https://github.com/espressif/esp-zigbee-sdk) is the official Zigbee development framework. It is used for compiling the firmware for the touchlink switch device.

```
cd <your-local-workspace>
git clone git@github.com:espressif/esp-zigbee-sdk.git
```

### Build, flash and run the zigbee touchlink switch

An other ESP32-H2 board is needed.
Please Open a new terminal(T-4) and run these commands:

```
cd <your-local-workspace>/esp-idf-h2
git checkout v5.1.3
./install.sh
. ./export.sh
cd <your-local-workspace>/esp-zigbee-sdk/examples/esp_zigbee_touchlink/touchlink_switch
idf.py set-target esp32h2
idf.py build
idf.py -p <your-local-port> flash monitor
```

Tips, if there is an error when you try to switch the idf branch:
```
error: pathspec v5.1.3' did not match any file(s) known to git
```
The github remote might be needed, you can add it via these commands:

```
cd <your-local-workspace>/esp-idf-h2
git remote add github git@github.com:espressif/esp-idf.git
git fetch github
```


You will see the touchlink switch scans the target device periodically:
```
I (23) boot: ESP-IDF v5.3-dev-2305-gb32af596ab 2nd stage bootloader
I (24) boot: compile time Mar 20 2024 16:20:16
I (25) boot: chip revision: v0.1
I (28) boot.esp32h2: SPI Speed      : 64MHz
I (33) boot.esp32h2: SPI Mode       : DIO
I (38) boot.esp32h2: SPI Flash Size : 2MB
I (42) boot: Enabling RNG early entropy source...
I (48) boot: Partition Table:
I (51) boot: ## Label            Usage          Type ST Offset   Length
I (59) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (66) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (74) boot:  2 factory          factory app      00 00 00010000 000e1000
I (81) boot:  3 zb_storage       Unknown data     01 81 000f1000 00004000
I (89) boot:  4 zb_fct           Unknown data     01 81 000f5000 00000400
I (96) boot: End of partition table
I (100) esp_image: segment 0: paddr=00010020 vaddr=42070020 size=117c0h ( 71616) map
I (131) esp_image: segment 1: paddr=000217e8 vaddr=40800000 size=06830h ( 26672) load
I (141) esp_image: segment 2: paddr=00028020 vaddr=42000020 size=6c818h (444440) map
I (277) esp_image: segment 3: paddr=00094840 vaddr=40806830 size=0664ch ( 26188) load
I (287) esp_image: segment 4: paddr=0009ae94 vaddr=4080ce80 size=01780h (  6016) load
I (293) boot: Loaded app from partition at offset 0x10000
I (294) boot: Disabling RNG early entropy source...
I (307) cpu_start: Unicore app
W (316) clk: esp_perip_clk_init() has not been implemented yet
I (323) cpu_start: Pro cpu start user code
I (323) cpu_start: cpu freq: 96000000 Hz
I (324) heap_init: Initializing. RAM available for dynamic allocation:
I (328) heap_init: At 40817900 len 00035A80 (214 KiB): RAM
I (334) heap_init: At 4084D380 len 00002B60 (10 KiB): RAM
I (341) spi_flash: detected chip: generic
I (345) spi_flash: flash io: dio
W (349) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (362) app_init: Application information:
I (367) app_init: Project name:     esp_touchlink_switch
I (373) app_init: App version:      9606aa4
I (378) app_init: Compile time:     Mar 20 2024 16:20:06
I (384) app_init: ELF file SHA256:  25e75227b31e2230...
I (390) app_init: ESP-IDF:          v5.3-dev-2305-gb32af596ab
I (396) app_init: Min chip rev:     v0.0
I (401) app_init: Max chip rev:     v0.99
I (405) app_init: Chip rev:         v0.1
I (410) sleep: Configure to isolate all GPIO pins in sleep state
I (417) sleep: Enable automatic switching of GPIO sleep configuration
I (424) main_task: Started on CPU0
I (424) main_task: Calling app_main()
I (444) phy: phy_version: 230,2, 9aae6ea, Jan 15 2024, 11:17:12
I (444) phy: libbtbb version: 944f18e, Jan 15 2024, 11:17:25
I (454) main_task: Returned from app_main()
I (584) ESP_TL_ON_OFF_SWITCH: ZDO signal: ZDO Config Ready (0x17), status: ESP_FAIL
I (584) ESP_TL_ON_OFF_SWITCH: Initialize Zigbee stack
I (594) gpio: GPIO[9]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:2
I (594) ESP_TL_ON_OFF_SWITCH: Deferred driver initialization successful
I (604) ESP_TL_ON_OFF_SWITCH: Device started up in  factory-reset mode
I (614) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
I (2014) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (2014) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
I (3364) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (3364) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
I (4714) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (4714) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
I (6104) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (6104) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
I (7434) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (7434) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
I (8824) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (8824) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
```

On the terminal(T-2), reflash the H2 board.
```
idf.py -p <your-local-port> flash monitor
```

This time you will see:

```
Received Touchlink scan request from 74:4d:bd:ff:fe:60:30:8e, RSSI: -23 dBm
...
Received Touchlink scan request from 74:4d:bd:ff:fe:60:30:8e, RSSI: -22 dBm
...

I (2070) chip[DL]: CHIPoBLE advertising started
I (2070) app_main: Commissioning window opened
I (2080) esp_matter_core: Cannot find minimum unused endpoint_id, try to find in the previous namespace
I (2090) esp_matter_core: Failed to open node namespace
I (2090) main_task: Returned from app_main()
W (2970) ESP_TL_ON_OFF_LIGHT: Network(0x1f93) closed, devices joining not allowed.
I (2970) ESP_TL_ON_OFF_LIGHT: Commissioning successfully, network information (Extended PAN ID: 74:4d:bd:ff:fe:60:2e:de, PAN ID: 0x1f93, Channel:11, Short Address: 0xfcd4)
I (2980) ESP_TL_ON_OFF_LIGHT: Touchlink target commissioning finished
I (4660) ESP_TL_ON_OFF_LIGHT: ZDO signal: ZDO Device Update (0x30), status: ESP_OK
I (4670) ESP_TL_ON_OFF_LIGHT: New device commissioned or rejoined (short: 0x04c2)
I (5030) esp_matter_core: Store the deferred attribute 0x0 of cluster 0x8 on endpoint 0x1
```
On the touchlink switch device, you can see the commissioning is done and bound is successful:
```
I (11684) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (11684) ESP_TL_ON_OFF_SWITCH: Scanning as a Touchlink initiator...
I (15464) ESP_TL_ON_OFF_SWITCH: Touchlink initiator receives the response for started network
I (15464) ESP_TL_ON_OFF_SWITCH: Response is from profile: 0x0104, endpoint: 10, address: 0x744dbdfffe602eDE
I (15744) ESP_TL_ON_OFF_SWITCH: Touchlink commissioning as initiator done
I (15744) ESP_TL_ON_OFF_SWITCH: Commissioning successfully, network information (Extended PAN ID: 74:4d:bd:ff:fe:60:2e:de, PAN ID: 0x1f93, Channel:11, Short Address: 0x04c2)
I (15804) ESP_TL_ON_OFF_SWITCH: Found light
I (15804) ESP_TL_ON_OFF_SWITCH: Try to bind on/off light
I (15814) ESP_TL_ON_OFF_SWITCH: Bound successfully!
I (56304) ESP_TL_ON_OFF_SWITCH: ZDO signal: ZDO Device Unavailable (0x3c), status: ESP_OK

```

Additionally, on the touchlink switch device, the `BOOT` button can be used to control the matter over thread with zigbee touchlink target device. Each press of the button triggers the following actions:
```
I (214964) ESP_TL_ON_OFF_SWITCH: send 'on_off toggle' command
I (215764) ESP_TL_ON_OFF_SWITCH: send 'on_off toggle' command
I (216484) ESP_TL_ON_OFF_SWITCH: send 'on_off toggle' command
I (216964) ESP_TL_ON_OFF_SWITCH: send 'on_off toggle' command
```

### Finish the test

On the terminal(T-1) and terminal(T-4), Quit the monitor by pressing Ctrl + `]` on the keyboard. And then erase the flash of the board to prepare for the next test.
```
idf.py -p <your-local-port> erase-flash
```
