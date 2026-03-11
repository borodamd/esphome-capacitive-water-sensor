#include "capacitive_water_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace capacitive_water_sensor {

static const char *TAG = "capacitive_water_sensor.sensor";

CapacitiveWaterSensor::CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval)
    : PollingComponent(update_interval), send_pin_(send_pin), receive_pin_(receive_pin) {}

void CapacitiveWaterSensor::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Capacitive Water Sensor...");
    sensor_ = new CapacitiveSensor(send_pin_, receive_pin_);
    sensor_->set_CS_Timeout_Millis(timeout_ms_);
    sensor_->set_CS_AutocaL_Millis(0xFFFFFFFF);
}

void CapacitiveWaterSensor::update() {
    if (!sensor_) {
        ESP_LOGE(TAG, "Sensor not initialized!");
        return;
    }

    long reading_raw = sensor_->capacitiveSensorRaw(samples_);
    float mapped_value;

    if (reading_raw == -2) {
        mapped_value = 125.0f;
    } else if (reading_raw <= 0) {
        mapped_value = 0.0f;
    } else {
        mapped_value = clamp(
            map(reading_raw, min_raw_, max_raw_, 0.0f, 120.0f),
            0.0f, 120.0f
        );
    }

    publish_state(mapped_value);
    ESP_LOGD(TAG, "Raw: %ld, Mapped: %.1f", reading_raw, mapped_value);
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
