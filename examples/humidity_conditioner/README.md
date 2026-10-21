# Humidity Conditioner

Humidity Conditioner 设备示例（设备类型 `0x007D`）。Humidistat 打开以下 feature：

| Feature | 生成代码里的名字 | 位 | 作用 |
|---|---|---|---|
| Humidifier | `humidifier` | `0x01` | 加湿，`MistType` |
| Dehumidifier | `dehumidifier` | `0x02` | 除湿 |
| Sensor | `sensor` | `0x08` | `UserSetpoint`、`MinSetpoint`、`MaxSetpoint`、`Step` |
| Auto | `fan_auto` | `0x10` | 在加湿和除湿之间自动切换 |
| Optimal | `optimal` | `0x40` | 按最佳湿度控制 |
| ColdMist | `cold_mist` | `0x100` | 冷雾 |

初始值：`Mode = Auto`，`SystemState = Idle`，设定点范围 30–70、步进 5、用户设定点 50，`MistType = Cold`。`Optimal` 属性初始为 false，此时 `TargetSetpoint` 跟随 `UserSetpoint`。

同一 endpoint 上还有 Relative Humidity Measurement。`MeasuredValue` 的单位是百分之一百分比（0–10000）。驱动里的 `app_driver_get_relative_humidity()` 是空实现，返回 null。

On/Off 带设备类型要求的 Dead Front Behavior。执行器接口在 `app_driver.cpp`，目前不操作硬件。

## 编译

需要打开 `CONFIG_ESP_MATTER_ENABLE_GENERATED_DATA_MODEL`。

```bash
cd examples/humidity_conditioner
idf.py set-target esp32c3
idf.py build
```
