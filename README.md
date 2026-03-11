# ESPHome Capacitive Water Sensor for Xiaomi Smartmi Humidifier

Внешний компонент ESPHome для емкостного датчика уровня воды увлажнителя Xiaomi Smartmi Air Humidifier 2 (CJXJSQ02ZM).

## Подключение

- send_pin (GPIO5) → контакт 1 датчика
- receive_pin (GPIO4) → контакт 2 датчика (через резистор 1МОм)
- GND → GND
- 5V → 5V (опционально, для питания датчика)

## Использование

```yaml
external_components:
  - source: github://YOUR_USERNAME/esphome-capacitive-water-sensor

sensor:
  - platform: capacitive_water_sensor
    name: "Water Level"
    send_pin: 5
    receive_pin: 4
    num_samples: 1000
    min_raw_value: 4200
    max_raw_value: 11000
    update_interval: 100ms
