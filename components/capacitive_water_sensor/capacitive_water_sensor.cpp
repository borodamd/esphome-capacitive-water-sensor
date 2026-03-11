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
    
    // Инициализация пинов
    pinMode(send_pin_, OUTPUT);
    pinMode(receive_pin_, INPUT);
    digitalWrite(send_pin_, LOW);
    
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
    ESP_LOGCONFIG(TAG, "  Samples: %u", samples_);
    ESP_LOGCONFIG(TAG, "  Timeout: %u ms", timeout_ms_);
}

long CapacitiveWaterSensor::readCapacitiveSensor() {
    long total = 0;
    unsigned long timeout_us = timeout_ms_ * 1000UL;
    
    for (uint32_t i = 0; i < samples_; i++) {
        // Отправляем импульс
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(10);
        pinMode(receive_pin_, INPUT);
        
        digitalWrite(send_pin_, HIGH);
        
        // Измеряем время
        unsigned long start = micros();
        unsigned long end = start;
        
        while (digitalRead(receive_pin_) == LOW && (end - start) < timeout_us) {
            end = micros();
        }
        
        digitalWrite(send_pin_, LOW);
        
        // Сбрасываем пин
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(10);
        pinMode(receive_pin_, INPUT);
        
        total += (end - start);
        
        // Небольшая задержка между измерениями
        delayMicroseconds(100);
    }
    
    // Если total очень маленький - возможно короткое замыкание
    if (total < 100) {
        return -2;  // Сигнал короткого замыкания как в оригинальной библиотеке
    }
    
    return total / samples_;
}

void CapacitiveWaterSensor::update() {
    long reading_raw = readCapacitiveSensor();
    float mapped_value;

    if (reading_raw == -2) {
        // Короткое замыкание (полное погружение)
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGD(TAG, "Sensor shorted - value: %.1f", mapped_value);
    } else if (reading_raw <= 10) {
        // Очень маленькое значение - сухой датчик
        mapped_value = 0.0f;
        ESP_LOGD(TAG, "Sensor dry - raw: %ld", reading_raw);
    } else {
        // Нормальное измерение - маппинг в диапазон 0-120
        float raw_float = static_cast<float>(reading_raw);
        float min_float = static_cast<float>(min_raw_);
        float max_float = static_cast<float>(max_raw_);
        
        // Защита от деления на ноль
        if (max_float <= min_float) {
            max_float = min_float + 1.0f;
        }
        
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
