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
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(10);
        pinMode(receive_pin_, INPUT);
        
        digitalWrite(send_pin_, HIGH);
        
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
        
        if (now - start <= timeout_us) {
            total += (now - start);
        }
        
        delayMicroseconds(50);
    }
    
    if (timeout_count == samples_) {
        return -2;
    } else if (total < 100) {
        return 0;
    } else {
        return total / samples_;
    }
}

// ⚠️ ТОЛЬКО ОДНО ОПРЕДЕЛЕНИЕ этой функции!
float CapacitiveWaterSensor::mapWithThreshold(long raw_value) {
    // Для отладки всегда логируем
    ESP_LOGI(TAG, "mapWithThreshold input: raw=%ld", raw_value);
    
    // Сухой датчик - только когда действительно сухо и нет контакта с водой
    if (raw_value == -2) {
        // Проверяем, может это все-таки вода?
        // Сделаем небольшую задержку и проверим еще раз
        delay(10);
        long check = readCapacitiveSensor();
        if (check == -2) {
            ESP_LOGI(TAG, "  → Confirmed DRY (raw=%ld, check=%ld)", raw_value, check);
            return 0.0f;
        } else {
            ESP_LOGI(TAG, "  → False DRY, real raw=%ld", check);
            raw_value = check;
        }
    }
    
    // Теперь работаем с реальными значениями
    if (raw_value < dry_threshold_ && raw_value > 0) {
        // Вода есть - маппинг от dry_threshold_ до 0
        float min_val = static_cast<float>(dry_threshold_);
        float max_val = 0.0f;
        float raw = static_cast<float>(raw_value);
        
        // Инвертируем: чем меньше RAW, тем больше воды
        float progress = (min_val - raw) / min_val;
        if (progress < 0) progress = 0;
        if (progress > 1) progress = 1;
        
        // Добавляем нелинейность для более плавного начала
        // progress = pow(progress, 0.7); // Раскомментируйте для нелинейности
        
        float result = progress * shorted_value_;
        ESP_LOGI(TAG, "  → WET: raw=%ld, progress=%.2f, result=%.1f", 
                 raw_value, progress, result);
        return result;
    }
    
    // Если значение вне диапазона - сухо
    ESP_LOGI(TAG, "  → OUT OF RANGE: raw=%ld, threshold=%u", 
             raw_value, dry_threshold_);
    return 0.0f;
}

void CapacitiveWaterSensor::update() {
    unsigned long start_time = millis();
    
    long reading_raw = readCapacitiveSensor();
    
    // Сохраняем сырое значение для отладки
    ESP_LOGI(TAG, "RAW reading: %ld (samples: %u, timeout: %ums)", 
             reading_raw, samples_, timeout_ms_);
    
    float mapped_value = mapWithThreshold(reading_raw);

    ESP_LOGI(TAG, "→ Water Level: %.1f", mapped_value);

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
