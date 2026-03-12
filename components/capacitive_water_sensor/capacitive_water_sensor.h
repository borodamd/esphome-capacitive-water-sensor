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

  // Сеттеры
  void set_num_samples(uint32_t samples) { samples_ = samples; }
  void set_timeout_ms(uint32_t timeout) { timeout_ms_ = timeout; }
  
  // НОВЫЕ ПАРАМЕТРЫ для софтового порога
  void set_use_soft_threshold(bool use) { use_soft_threshold_ = use; }
  void set_dry_threshold(uint32_t threshold) { dry_threshold_ = threshold; }
  void set_wet_min_raw(uint32_t min_raw) { wet_min_raw_ = min_raw; }
  void set_wet_max_raw(uint32_t max_raw) { wet_max_raw_ = max_raw; }
  void set_shorted_value(uint8_t value) { shorted_value_ = value; }

 protected:
  uint8_t send_pin_;
  uint8_t receive_pin_;
  uint32_t samples_{10};  // Уменьшаем дефолт
  uint32_t timeout_ms_{100};
  
  // Новые параметры
  bool use_soft_threshold_{true};  // Включаем по умолчанию
  uint32_t dry_threshold_{300};    // Значение, ниже которого считаем "мокро"
  uint32_t wet_min_raw_{0};         // Минимальное RAW для мокрого
  uint32_t wet_max_raw_{300};       // Максимальное RAW для мокрого
  uint8_t shorted_value_{125};
  
  long readCapacitiveSensor();
  float mapWithThreshold(long raw_value);
};

}  // namespace capacitive_water_sensor
}  // namespace esphome
