#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/sensor/sensor.h"
#include <CapacitiveSensor.h>

namespace esphome {
namespace capacitive_water_sensor {

class CapacitiveWaterSensor : public PollingComponent, public sensor::Sensor {
 public:
  CapacitiveWaterSensor(uint8_t send_pin, uint8_t receive_pin, uint32_t update_interval);

  void setup() override;
  void update() override;
  void dump_config() override;

  void set_num_samples(uint32_t samples) { samples_ = samples; }
  void set_min_raw_value(uint32_t min_raw) { min_raw_ = min_raw; }
  void set_max_raw_value(uint32_t max_raw) { max_raw_ = max_raw; }
  void set_timeout_ms(uint32_t timeout) { timeout_ms_ = timeout; }

 protected:
  uint8_t send_pin_;
  uint8_t receive_pin_;
  uint32_t samples_{1000};
  uint32_t min_raw_{4200};
  uint32_t max_raw_{11000};
  uint32_t timeout_ms_{500};

  CapacitiveSensor *sensor_{nullptr};
};

}  // namespace capacitive_water_sensor
}  // namespace esphome
