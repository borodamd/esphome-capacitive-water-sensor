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
    
    // Настраиваем пины
    pinMode(send_pin_, OUTPUT);
    pinMode(receive_pin_, INPUT_PULLDOWN);  // Важно: используем PULLDOWN
    digitalWrite(send_pin_, LOW);
    
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
}

long CapacitiveWaterSensor::readCapacitiveSensor() {
    long total = 0;
    const int samples = samples_;
    unsigned long timeout_us = timeout_ms_ * 1000UL;
    
    for (int i = 0; i < samples; i++) {
        // Полностью разряжаем receive пин
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(100);  // Увеличиваем время разряда
        
        // Переключаем receive обратно на INPUT
        pinMode(receive_pin_, INPUT_PULLDOWN);
        
        // Отправляем импульс
        digitalWrite(send_pin_, HIGH);
        
        // Измеряем время
        unsigned long start = micros();
        unsigned long end = start;
        
        // Ждем пока receive пин станет HIGH
        while (digitalRead(receive_pin_) == LOW) {
            end = micros();
            if (end - start > timeout_us) {
                break;
            }
            // Небольшая задержка для экономии CPU
            if ((end - start) % 100 == 0) delayMicroseconds(1);
        }
        
        // Выключаем send пин
        digitalWrite(send_pin_, LOW);
        
        // Добавляем измерение к общему
        total += (end - start);
        
        // Небольшая пауза между измерениями
        delayMicroseconds(500);
    }
    
    // Если total ОЧЕНЬ маленький - возможно короткое замыкание
    if (total < 10) {
        return -2;  // Короткое замыкание
    }
    
    // Если total подозрительно маленький - возможно проблема с подключением
    if (total < 500) {
        ESP_LOGW(TAG, "Very low reading: %ld", total);
    }
    
    return total / samples;
}

void CapacitiveWaterSensor::update() {
    long reading_raw = readCapacitiveSensor();
    float mapped_value;

    // Всегда логируем для отладки
    ESP_LOGI(TAG, "RAW Reading: %ld", reading_raw);

    if (reading_raw == -2) {
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGI(TAG, "  → SHORTED (full water)");
    } else if (reading_raw < 100) {
        mapped_value = 0.0f;
        ESP_LOGI(TAG, "  → DRY or ERROR");
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
        
        ESP_LOGI(TAG, "  → Raw: %ld, Mapped: %.1f", reading_raw, mapped_value);
    }

    publish_state(mapped_value);
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
