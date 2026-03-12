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
    int timeout_count = 0;
    unsigned long timeout_us = timeout_ms_ * 1000UL;
    
    for (uint32_t i = 0; i < samples_; i++) {
        // Разряжаем receive пин
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(10);
        pinMode(receive_pin_, INPUT);
        
        // Отправляем импульс
        digitalWrite(send_pin_, HIGH);
        
        // Измеряем время
        unsigned long start = micros();
        unsigned long now = start;
        
        while (digitalRead(receive_pin_) == LOW) {
            now = micros();
            if (now - start > timeout_us) {
                timeout_count++;
                break;
            }
        }
        
        digitalWrite(send_pin_, LOW);
        
        // Если не было таймаута, добавляем к total
        if (now - start <= timeout_us) {
            total += (now - start);
        }
        
        // Короткая пауза
        delayMicroseconds(50);
    }
    
    if (timeout_count == samples_) {
        return -2;  // Все измерения с таймаутом
    } else if (total < 100) {
        return 0;   // Очень маленькие значения
    } else {
        return total / samples_;
    }
}

float CapacitiveWaterSensor::mapWithThreshold(long raw_value) {
    if (!use_soft_threshold_) {
        // Старая логика (без порога)
        if (raw_value == -2) return 0.0f;
        if (raw_value == 0) return static_cast<float>(shorted_value_);
        return 120.0f - ((raw_value - wet_min_raw_) / (float)(wet_max_raw_ - wet_min_raw_)) * 120.0f;
    }
    
    // Новая логика с софтовым порогом
    if (raw_value == -2) {
        // Сухой датчик
        return 0.0f;
    }
    else if (raw_value < dry_threshold_) {
        // Мокрый датчик (значение меньше порога)
        if (raw_value <= 0) {
            return static_cast<float>(shorted_value_);  // Полный
        }
        // Пропорциональное значение от dry_threshold_ до 0
        float progress = 1.0f - ((float)raw_value / dry_threshold_);
        return progress * shorted_value_;
    }
    else {
        // Очень сухо (выше порога) - считаем сухим
        return 0.0f;
    }
}

void CapacitiveWaterSensor::update() {
    unsigned long start_time = millis();
    
    long reading_raw = readCapacitiveSensor();
    float mapped_value = mapWithThreshold(reading_raw);

    ESP_LOGI(TAG, "RAW: %ld → Value: %.1f (samples: %u, timeout: %ums)", 
             reading_raw, mapped_value, samples_, timeout_ms_);

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
    ESP_LOGCONFIG(TAG, "  Soft Threshold: %s", use_soft_threshold_ ? "YES" : "NO");
    ESP_LOGCONFIG(TAG, "  Dry Threshold: %u", dry_threshold_);
    LOG_UPDATE_INTERVAL(this);
}

}  // namespace capacitive_water_sensor
}  // namespace esphome
