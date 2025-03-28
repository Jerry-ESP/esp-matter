# 1.Purpose
This example intergrate Controller, OTBR(opnethread border router) and OTA provider, can run on the [Thread Border Router Board](https://docs.espressif.com/projects/esp-thread-br/en/latest/hardware_platforms.html) directly. It can be used to do OTA for matter over wifi device and matter over thread device.

# 2.Setup
### <1>. Set Target

```
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults.otbr" set-target esp32s3
```

### <2>. Set rework devices type
Set the rework devices type, matter over wifi or matter over thread, can set through menuconfig option:
```
CONFIG_WIFI_DEVICE=y or CONFIG_THREAD_DEVICE=y
```

### <3>.  Set the default wifi network params
```
CONFIG_DEFAULT_WIFI_SSID="matter1_1"
CONFIG_DEFAULT_WIFI_PASSWORD="espressif"
```

### <4>. Set the thread network params
Set the thread network params if you rework the thread devices, for different rework tools nearby, you shall not use the same network params and you'd better use different channels.
```
CONFIG_OPENTHREAD_NETWORK_CHANNEL=22
CONFIG_OPENTHREAD_NETWORK_PANID=0xab12
CONFIG_OPENTHREAD_NETWORK_EXTPANID="dead001234567890"
```

### <5>. Enable DAC Update
If need to update DAC, please enable below option:
```
CONFIG_ENABLE_DAC_UPDATE=y
```

### <6>. Config new software version and software version string
Config rework device new software version and new software version string
```
CONFIG_NEW_SOFTWARE_VERSION=10010001
CONFIG_NEW_SOFTWARE_VERSION_STRING="1.1.1-660"
```

# 3. Workflow
After the rework tool power up, you should use below command to set the Local DCL's ip address:
```
matter esp manager set-ip 192.168.1.100
```
