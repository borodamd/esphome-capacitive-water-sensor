#include "capacitive_water_sensor.h"
#include "esphome/core/log.h"
#include <CapacitiveSensor.h>

namespace esphome {
namespace capacitive_water_sensor {

static const char *TAG = "capacitive_water_sensor.sensor";

CapacitiveWaterSensor::CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval_ms)
    : PollingComponent(update_interval_ms), send_pin_(send_pin), receive_pin_(receive_pin) {}

void CapacitiveWaterSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Capacitive Water Sensor...");
    
    sensor_ = new CapacitiveSensor(send_pin_, receive_pin_);
    CapacitiveSensor* cap = static_cast<CapacitiveSensor*>(sensor_);
    
    cap->set_CS_Timeout_Millis(timeout_ms_);
    cap->set_CS_AutocaL_Millis(0xFFFFFFFF); // Отключаем автокалибровку как в скетче
    
    ESP_LOGCONFIG(TAG, "  Send Pin: GPIO%u", send_pin_);
    ESP_LOGCONFIG(TAG, "  Receive Pin: GPIO%u", receive_pin_);
    ESP_LOGCONFIG(TAG, "  Samples: %u", samples_);
    ESP_LOGCONFIG(TAG, "  Timeout: %u ms", timeout_ms_);
}

long CapacitiveWaterSensor::readCapacitiveSensor() {
    CapacitiveSensor* cap = static_cast<CapacitiveSensor*>(sensor_);
    return cap->capacitiveSensorRaw(samples_);
}

void CapacitiveWaterSensor::update() {
    unsigned long start_time = millis();
    
    long reading_raw = readCapacitiveSensor();
    float mapped_value;

    if (reading_raw == -2) {
        // Короткое замыкание - полный бак
        mapped_value = static_cast<float>(shorted_value_);
        ESP_LOGI(TAG, "RAW: -2 → SHORTED (full tank): %.1f", mapped_value);
    } 
    else if (reading_raw <= 0) {
        // Ошибка измерения - считаем сухим
        mapped_value = 0.0f;
        ESP_LOGI(TAG, "RAW: %ld → ERROR/DRY: 0", reading_raw);
    }
    else {
        // Нормальное измерение - маппинг 0-2000? в 0-120
        // В оригинале MIN_READING=4200, MAX_READING=11000 для 1000 samples
        // Для меньших samples значения будут пропорционально меньше
        float min_val = 4200.0f * (samples_ / 1000.0f);
        float max_val = 11000.0f * (samples_ / 1000.0f);
        
        float mapped = (reading_raw - min_val) / (max_val - min_val) * 120.0f;
        mapped_value = std::max(0.0f, std::min(120.0f, mapped));
        
        ESP_LOGI(TAG, "RAW: %ld → NORMAL: %.1f (min=%.0f, max=%.0f)", 
                 reading_raw, mapped_value, min_val, max_val);
    }

    publish_state(mapped_value);
    
    unsigned long elapsed = millis() - start_time;
    if (elapsed > 100) {
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
