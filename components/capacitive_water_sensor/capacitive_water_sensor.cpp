#include "capacitive_water_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace capacitive_water_sensor {

static const char *TAG = "capacitive_water_sensor";

CapacitiveWaterSensor::CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval_ms)
    : PollingComponent(update_interval_ms), send_pin_(send_pin), receive_pin_(receive_pin) {}

void CapacitiveWaterSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Capacitive Water Sensor...");
    
    // Создаем экземпляр библиотеки
    sensor_ = new CapacitiveSensor(send_pin_, receive_pin_);
    
    // Настройка параметров как в оригинальном скетче
    sensor_->set_CS_Timeout_Millis(timeout_ms_);
    sensor_->set_CS_AutocaL_Millis(0xFFFFFFFF);  // Отключаем автокалибровку
    
    ESP_LOGCONFIG(TAG, "Sensor initialized with send pin GPIO%u, receive pin GPIO%u", 
                  send_pin_, receive_pin_);
}

void CapacitiveWaterSensor::update() {
    if (!sensor_) {
        ESP_LOGE(TAG, "Sensor not initialized!");
        return;
    }

    // Чтение сырого значения с датчика
    long reading_raw = sensor_->capacitiveSensorRaw(samples_);
    float mapped_value;

    // Обработка различных состояний датчика
    if (reading_raw == -2) {
        // Короткое замыкание (полное погружение/вода замкнула контакты)
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGD(TAG, "Sensor shorted (full water) - value: %.1f", mapped_value);
    } else if (reading_raw <= 0) {
        // Ошибка чтения или сухой датчик
        mapped_value = 0.0f;
        ESP_LOGD(TAG, "Sensor error or dry - raw: %ld", reading_raw);
    } else {
        // Нормальное измерение - маппинг значений в диапазон 0-120
        mapped_value = clamp(
            map(reading_raw, min_raw_, max_raw_, 0.0f, 120.0f),
            0.0f, 120.0f
        );
        ESP_LOGD(TAG, "Normal reading - raw: %ld, mapped: %.1f", reading_raw, mapped_value);
    }

    // Публикация значения
    publish_state(mapped_value);
}

void CapacitiveWaterSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "Capacitive Water Sensor:");
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
    ESP_LOGCONFIG(TAG, "  Number of samples: %u", samples_);
    ESP_LOGCONFIG(TAG, "  Timeout: %u ms", timeout_ms_);
    ESP_LOGCONFIG(TAG, "  Raw value range: %u - %u", min_raw_, max_raw_);
    ESP_LOGCONFIG(TAG, "  Shorted value: %u", shorted_value_);
    LOG_UPDATE_INTERVAL(this);
}

}  // namespace capacitive_water_sensor
}  // namespace esphome
