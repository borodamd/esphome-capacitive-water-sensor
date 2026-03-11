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
    // Исправляем: приводим оба аргумента к одному типу
    uint32_t actual_samples = samples_;
    if (actual_samples > 30) actual_samples = 30; // Ограничиваем для теста
    
    unsigned long timeout_us = timeout_ms_ * 1000UL;
    
    for (uint32_t i = 0; i < actual_samples; i++) {
        // Разряжаем receive пин
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(10);
        
        // Переключаем receive обратно на INPUT
        pinMode(receive_pin_, INPUT);
        
        // Отправляем импульс
        digitalWrite(send_pin_, HIGH);
        
        // Измеряем время
        unsigned long start = micros();
        unsigned long now = start;
        
        // Ждем пока receive пин станет HIGH
        while (digitalRead(receive_pin_) == LOW) {
            now = micros();
            if (now - start > timeout_us) {
                break;
            }
            // Небольшая задержка для экономии CPU
            if ((now - start) > 100) {
                delayMicroseconds(5);
            }
        }
        
        // Выключаем send пин
        digitalWrite(send_pin_, LOW);
        
        // Добавляем измерение
        total += (now - start);
        
        // Короткая пауза между измерениями
        delayMicroseconds(50);
    }
    
    // Если total очень маленький - короткое замыкание
    if (total < 10) {
        return -2;
    }
    
    return total / actual_samples;
}

void CapacitiveWaterSensor::update() {
    unsigned long start_time = millis();
    
    long reading_raw = readCapacitiveSensor();
    float mapped_value;

    if (reading_raw == -2) {
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGD(TAG, "RAW: -2 → SHORTED (%.1f)", mapped_value);
    } else if (reading_raw < 10) {
        mapped_value = 0.0f;
        ESP_LOGD(TAG, "RAW: %ld → DRY", reading_raw);
    } else {
        // Нормальное измерение
        float raw_float = static_cast<float>(reading_raw);
        float min_float = static_cast<float>(min_raw_);
        float max_float = static_cast<float>(max_raw_);
        
        if (max_float <= min_float) {
            max_float = min_float + 1.0f;
        }
        
        float mapped = (raw_float - min_float) / (max_float - min_float) * 120.0f;
        mapped_value = std::max(0.0f, std::min(120.0f, mapped));
        
        ESP_LOGD(TAG, "RAW: %ld → %.1f", reading_raw, mapped_value);
    }

    publish_state(mapped_value);
    
    unsigned long elapsed = millis() - start_time;
    if (elapsed > 50) {
        ESP_LOGW(TAG, "Update took %lu ms", elapsed);
    }
}

void CapacitiveWaterSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "Capacitive Water Sensor:");
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
    ESP_LOGCONFIG(TAG, "  Samples: %u", samples_);
    ESP_LOGCONFIG(TAG, "  Timeout: %u ms", timeout_ms_);
    LOG_UPDATE_INTERVAL(this);
}

}  // namespace capacitive_water_sensor
}  // namespace esphome
