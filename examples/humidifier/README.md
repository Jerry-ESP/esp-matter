# Humidifier

This example creates a Humidifier/Dehumidifier device using the ESP Matter data model with the provisional Humidistat cluster.

The endpoint uses `endpoint::humidifier_dehumidifier::create()` and enables Humidifier, Sensor, and ColdMist features on the Humidistat cluster. A `HumidistatDelegate` simulates driver updates by reporting `SystemState` when `Mode` changes.

See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## 1. Additional Environment Setup

No additional setup is required.

## 2. Post Commissioning Setup

No additional setup is required.

## 3. Build

```bash
cd $ESP_MATTER_PATH/examples/humidifier
idf.py set-target esp32c6
idf.py build
```
