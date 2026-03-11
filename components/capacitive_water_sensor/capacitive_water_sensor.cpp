#include "capacitive_water_sensor.h"
#include "esphome/core/log.h"
#include <Arduino.h>

namespace esphome {
namespace capacitive_water_sensor {

static const char *TAG = "capacitive_water_sensor.sensor";

CapacitiveWaterSensor::CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval_ms)
    : PollingComponent(update_interval_ms), send_pin_(send_pin), receive_pin_(receive_pin) {}

void CapacitiveWaterSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Capacitive Water Sensor...");
    
    pinMode(send_pin_, OUTPUT);
    pinMode(receive_pin_, INPUT);
    digitalWrite(send_pin_, LOW);
    
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
}

long CapacitiveWaterSensor::readCapacitiveSensor() {
    long total = 0;
    unsigned long timeout_us = timeout_ms_ * 1000UL;
    
    for (uint32_t i = 0; i < samples_; i++) {
        // Более чувствительный метод измерения
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(20);
        pinMode(receive_pin_, INPUT);
        
        digitalWrite(send_pin_, HIGH);
        
        unsigned long start = micros();
        unsigned long end = start;
        
        // Ждем пока пин не станет HIGH или не выйдет время
        while (digitalRead(receive_pin_) == LOW && (end - start) < timeout_us) {
            end = micros();
            // Небольшая задержка для экономии CPU
            if (end - start > 100) delayMicroseconds(10);
        }
        
        digitalWrite(send_pin_, LOW);
        
        // Быстрый разряд
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(20);
        pinMode(receive_pin_, INPUT);
        
        total += (end - start);
    }
    
    if (total < 100 * samples_) {  // Очень маленькое значение - короткое замыкание
        return -2;
    }
    
    return total / samples_;
}

void CapacitiveWaterSensor::update() {
    long reading_raw = readCapacitiveSensor();
    float mapped_value;

    // Подробный вывод для калибровки
    ESP_LOGI(TAG, "RAW Reading: %ld", reading_raw);

    if (reading_raw == -2) {
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGI(TAG, "→ Sensor SHORTED (full water) - value: %.1f", mapped_value);
    } else if (reading_raw < 500) {
        mapped_value = 0.0f;
        ESP_LOGI(TAG, "→ Sensor DRY - value: 0");
    } else {
        // Автоматическая калибровка, если значения не заданы
        static uint32_t auto_min = reading_raw;
        static uint32_t auto_max = reading_raw;
        
        if (reading_raw < auto_min) auto_min = reading_raw;
        if (reading_raw > auto_max) auto_max = reading_raw;
        
        // Используем заданные значения или автокалибровку
        uint32_t min_val = (min_raw_ != 4200) ? min_raw_ : auto_min;
        uint32_t max_val = (max_raw_ != 11000) ? max_raw_ : auto_max;
        
        if (max_val <= min_val) max_val = min_val + 1;
        
        float raw_float = static_cast<float>(reading_raw);
        float min_float = static_cast<float>(min_val);
        float max_float = static_cast<float>(max_val);
        
        float mapped = (raw_float - min_float) / (max_float - min_float) * 120.0f;
        mapped_value = std::max(0.0f, std::min(120.0f, mapped));
        
        ESP_LOGI(TAG, "→ Raw: %ld, Min: %lu, Max: %lu, Mapped: %.1f", 
                 reading_raw, min_val, max_val, mapped_value);
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
    LOG_UPDATE_INTERVAL(this);
}

}  // namespace capacitive_water_sensor
}  // namespace esphome
