#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace capacitive_water_sensor {

class CapacitiveWaterSensor : public PollingComponent, public sensor::Sensor {
 public:
  CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval_ms);

  void setup() override;
  void update() override;
  void dump_config() override;

  void set_num_samples(uint16_t samples) { samples_ = samples; }
  void set_timeout_ms(uint16_t timeout) { timeout_ms_ = timeout; }
  void set_shorted_value(uint8_t value) { shorted_value_ = value; }

 protected:
  uint8_t send_pin_;
  uint8_t receive_pin_;
  uint16_t samples_{1000};
  uint16_t timeout_ms_{500};
  uint8_t shorted_value_{125};
  
  void *sensor_{nullptr};  // Указатель на CapacitiveSensor
  long readCapacitiveSensor();
};

}  // namespace capacitive_water_sensor
}  // namespace esphome
