#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include <CapacitiveSensor.h>
#include <memory>

namespace esphome {
namespace capacitive_water_sensor {

class CapacitiveWaterSensor : public PollingComponent, public sensor::Sensor {
 public:
  void set_pins(int sender, int sensor) { sender_pin_ = sender; sensor_pin_ = sensor; }
  void setup() override;
  void update() override;

 protected:
  int sender_pin_;
  int sensor_pin_;
  std::unique_ptr<CapacitiveSensor> sensor_impl_;
  uint8_t packet_[43] = {0xFA, 0x29, 0x03, 0x00, 0x00, 0x00, 0x00, 0x14, 0x9A, 0x00, 0x00, 0x00, 0x03, 0x77, 0x72, 0x71, 0x03, 0x00, 0x6C, 0x4C, 0x3B, 0x03, 0x2F, 0x15, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x03, 0xE8, 0x00, 0x2C, 0x02, 0x6D, 0x37, 0xD2, 0x00};
};

}  // namespace capacitive_water_sensor
}  // namespace esphome
