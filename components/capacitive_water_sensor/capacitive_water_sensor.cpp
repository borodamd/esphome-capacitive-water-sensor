#include "capacitive_water_sensor.h"
#include "esphome/core/log.h"
#include <Arduino.h>

// Подключаем библиотеку ТОЛЬКО здесь
#include <CapacitiveSensor.h>

namespace esphome {
namespace capacitive_water_sensor {

static const char *TAG = "capacitive_water_sensor.sensor";

CapacitiveWaterSensor::CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval_ms)
    : PollingComponent(update_interval_ms), send_pin_(send_pin), receive_pin_(receive_pin) {}

void CapacitiveWaterSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Capacitive Water Sensor...");
    
    // Создаем экземпляр сенсора
    sensor_ = new CapacitiveSensor(send_pin_, receive_pin_);
    
    // Настраиваем параметры
    CapacitiveSensor *capSensor = static_cast<CapacitiveSensor*>(sensor_);
    capSensor->set_CS_Timeout_Millis(timeout_ms_);
    capSensor->set_CS_AutocaL_Millis(0xFFFFFFFF);
    
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
    ESP_LOGCONFIG(TAG, "  Samples: %u", samples_);
    ESP_LOGCONFIG(TAG, "  Timeout: %u ms", timeout_ms_);
}

void CapacitiveWaterSensor::update() {
    if (!sensor_) {
        ESP_LOGE(TAG, "Sensor not initialized!");
        return;
    }

    CapacitiveSensor *capSensor = static_cast<CapacitiveSensor*>(sensor_);
    
    // Читаем значение
    long reading_raw = capSensor->capacitiveSensorRaw(samples_);
    float mapped_value;

    // Обрабатываем результат
    if (reading_raw == -2) {
        // Короткое замыкание (полное погружение)
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGD(TAG, "Sensor shorted - value: %.1f", mapped_value);
    } else if (reading_raw <= 0) {
        // Ошибка или сухой датчик
        mapped_value = 0.0f;
        ESP_LOGD(TAG, "Sensor dry/error - raw: %ld", reading_raw);
    } else {
        // Нормальное измерение
        float raw_float = static_cast<float>(reading_raw);
        float min_float = static_cast<float>(min_raw_);
        float max_float = static_cast<float>(max_raw_);
        
        // Маппинг в диапазон 0-120
        float mapped = (raw_float - min_float) / (max_float - min_float) * 120.0f;
        mapped_value = std::max(0.0f, std::min(120.0f, mapped));
        
        ESP_LOGD(TAG, "Raw: %ld, Mapped: %.1f", reading_raw, mapped_value);
    }

    publish_state(mapped_value);
}

void CapacitiveWaterSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "Capacitive Water Sensor:");
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
    ESP_LOGCONFIG(TAG, "  Samples: %u", samples_);
    ESP_LOGCONFIG(TAG, "  Timeout: %u ms", timeout_ms_);
    ESP_LOGCONFIG(TAG, "  Raw range: %u - %u", min_raw_, max_raw_);
    ESP_LOGCONFIG(TAG, "  Shorted value: %u", shorted_value_);
    LOG_UPDATE_INTERVAL(this);
}

}  // namespace capacitive_water_sensor
}  // namespace esphome
