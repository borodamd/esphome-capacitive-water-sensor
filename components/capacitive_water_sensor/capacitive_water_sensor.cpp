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
    ESP_LOGCONFIG(TAG, "  Samples: %u", samples_);
    ESP_LOGCONFIG(TAG, "  Timeout: %u ms", timeout_ms_);
}

long CapacitiveWaterSensor::readCapacitiveSensor() {
    long total = 0;
    unsigned long timeout_us = timeout_ms_ * 1000UL;
    bool timeout_occurred = false;
    
    for (uint32_t i = 0; i < samples_; i++) {
        // Полностью разряжаем receive пин
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(10);
        pinMode(receive_pin_, INPUT);
        
        // Отправляем импульс
        digitalWrite(send_pin_, HIGH);
        
        // Измеряем время до срабатывания или таймаута
        unsigned long start = micros();
        unsigned long now = start;
        
        while (digitalRead(receive_pin_) == LOW) {
            now = micros();
            if (now - start >= timeout_us) {
                timeout_occurred = true;
                break;
            }
        }
        
        digitalWrite(send_pin_, LOW);
        
        // Если был таймаут, не добавляем к total (как в библиотеке CapacitiveSensor)
        if (!timeout_occurred) {
            total += (now - start);
        }
        
        // Быстрая разрядка
        pinMode(receive_pin_, OUTPUT);
        digitalWrite(receive_pin_, LOW);
        delayMicroseconds(10);
        pinMode(receive_pin_, INPUT);
        
        // Небольшая пауза между измерениями
        delayMicroseconds(50);
    }
    
    // Если были таймауты во всех измерениях - значит короткое замыкание
    if (timeout_occurred && total == 0) {
        return -2;  // Специальное значение - короткое замыкание (бак полон)
    }
    
    // Если измерений не было или они очень маленькие - сухо
    if (total < 100) {
        return 0;  // Сухой датчик
    }
    
    return total / samples_;
}

void CapacitiveWaterSensor::update() {
    unsigned long start_time = millis();
    
    long reading_raw = readCapacitiveSensor();
    float mapped_value;

    // Подробное логирование для калибровки
    ESP_LOGI(TAG, "RAW reading: %ld", reading_raw);

    if (reading_raw == -2) {
        // Короткое замыкание - вода касается обоих щупов (бак полон)
        mapped_value = static_cast<float>(shorted_value_);  // 125
        ESP_LOGI(TAG, "  → SHORTED (full tank) - value: %.1f", mapped_value);
    } else if (reading_raw == 0) {
        // Сухой датчик - нет воды
        mapped_value = 0.0f;
        ESP_LOGI(TAG, "  → DRY - value: 0");
    } else {
        // Нормальное измерение - есть вода, но нет замыкания
        float raw_float = static_cast<float>(reading_raw);
        float min_float = static_cast<float>(min_raw_);
        float max_float = static_cast<float>(max_raw_);
        
        if (max_float <= min_float) {
            max_float = min_float + 1.0f;
        }
        
        // Маппинг в диапазон 0-120 как в оригинале
        float mapped = (raw_float - min_float) / (max_float - min_float) * 120.0f;
        mapped_value = std::max(0.0f, std::min(120.0f, mapped));
        
        ESP_LOGI(TAG, "  → NORMAL - raw: %ld, min: %.0f, max: %.0f, mapped: %.1f", 
                 reading_raw, min_float, max_float, mapped_value);
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
    ESP_LOGCONFIG(TAG, "  Raw range: %u - %u", min_raw_, max_raw_);
    LOG_UPDATE_INTERVAL(this);
}

}  // namespace capacitive_water_sensor
}  // namespace esphome
