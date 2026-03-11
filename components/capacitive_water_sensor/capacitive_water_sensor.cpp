#include "capacitive_water_sensor.h"
#include "esphome/core/log.h"
#include <Arduino.h>  // Добавляем для DigitalPin

// Подключаем библиотеку только в .cpp файле
#define CAPACITIVE_SENSOR_NUM_SAMPLES 10  // Уменьшаем для тестирования
#include <CapacitiveSensor.h>

namespace esphome {
namespace capacitive_water_sensor {

static const char *TAG = "capacitive_water_sensor";

CapacitiveWaterSensor::CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval_ms)
    : PollingComponent(update_interval_ms), send_pin_(send_pin), receive_pin_(receive_pin) {}

void CapacitiveWaterSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Capacitive Water Sensor...");
    
    // Создаем экземпляр библиотеки
    sensor_ = new CapacitiveSensor(send_pin_, receive_pin_);
    
    // Настройка параметров
    CapacitiveSensor *capSensor = static_cast<CapacitiveSensor*>(sensor_);
    capSensor->set_CS_Timeout_Millis(timeout_ms_);
    capSensor->set_CS_AutocaL_Millis(0xFFFFFFFF);
    
    ESP_LOGCONFIG(TAG, "Sensor initialized with send pin GPIO%u, receive pin GPIO%u", 
                  send_pin_, receive_pin_);
}

void CapacitiveWaterSensor::update() {
    if (!sensor_) {
        ESP_LOGE(TAG, "Sensor not initialized!");
        return;
    }

    CapacitiveSensor *capSensor = static_cast<CapacitiveSensor*>(sensor_);
    
    // Уменьшаем количество семплов для отладки
    long reading_raw = capSensor->capacitiveSensorRaw(samples_);
    float mapped_value;

    if (reading_raw == -2) {
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGD(TAG, "Sensor shorted (full water) - value: %.1f", mapped_value);
    } else if (reading_raw <= 0) {
        mapped_value = 0.0f;
        ESP_LOGD(TAG, "Sensor error or dry - raw: %ld", reading_raw);
    } else {
        // Исправляем проблему с clamp - приводим все к одному типу
        float raw_float = static_cast<float>(reading_raw);
        float min_float = static_cast<float>(min_raw_);
        float max_float = static_cast<float>(max_raw_);
        
        // Вычисляем маппинг вручную вместо использования clamp
        float mapped = (raw_float - min_float) / (max_float - min_float) * 120.0f;
        mapped_value = std::max(0.0f, std::min(120.0f, mapped));
        
        ESP_LOGD(TAG, "Normal reading - raw: %ld, raw_float: %.1f, min: %.1f, max: %.1f, mapped: %.1f", 
                 reading_raw, raw_float, min_float, max_float, mapped_value);
    }

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
