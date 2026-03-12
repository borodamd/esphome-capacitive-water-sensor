#include "capacitive_water_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace capacitive_water_sensor {

static const char *const TAG = "capacitive_water_sensor";

void CapacitiveWaterSensor::setup() {
  sensor_impl_ = std::make_unique<CapacitiveSensor>(sender_pin_, sensor_pin_);
  sensor_impl_->set_CS_AutocaL_Millis(0xFFFFFFFF);
  sensor_impl_->set_CS_Timeout_Millis(500);
  
  // Используем глобальный селектор :: для Serial
  ::Serial.begin(9600);
}

void CapacitiveWaterSensor::update() {
  // 1000UL гарантирует тип unsigned long
  long reading_raw = sensor_impl_->capacitiveSensorRaw(1000UL);
  uint8_t mapped_value;

  if (reading_raw == -2) {
    mapped_value = 125;
  } else {
    long val = reading_raw;
    if (val < 4200) val = 4200;
    if (val > 11000) val = 11000;
    mapped_value = (uint8_t)((val - 4200) * 120 / (11000 - 4200));
  }

  packet_[11] = mapped_value;
  uint8_t checksum = 0;
  for (int i = 0; i < 42; i++) checksum ^= packet_[i];
  checksum ^= 0xA0;
  packet_[42] = checksum;

  ::Serial.write(packet_, 43);
  ::Serial.flush();
  
  this->publish_state(reading_raw);
}

}  // namespace capacitive_water_sensor
}  // namespace esphome
