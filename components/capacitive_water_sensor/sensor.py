import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_UPDATE_INTERVAL,
    DEVICE_CLASS_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY
)
from esphome.core import CORE

# Определение конфигурационных констант
CONF_SEND_PIN = "send_pin"
CONF_RECEIVE_PIN = "receive_pin"
CONF_NUM_SAMPLES = "num_samples"
CONF_MIN_RAW = "min_raw_value"
CONF_MAX_RAW = "max_raw_value"
CONF_TIMEOUT = "timeout_ms"
CONF_SHORTED_VALUE = "shorted_value"

# Создание пространства имен для C++ кода
capacitive_water_sensor_ns = cg.esphome_ns.namespace("capacitive_water_sensor")
CapacitiveWaterSensor = capacitive_water_sensor_ns.class_(
    "CapacitiveWaterSensor", sensor.Sensor, cg.PollingComponent
)

# Схема конфигурации
CONFIG_SCHEMA = sensor.sensor_schema(
    CapacitiveWaterSensor,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_EMPTY,
    state_class=STATE_CLASS_MEASUREMENT,
    unit_of_measurement=UNIT_EMPTY
).extend({
    cv.GenerateID(): cv.declare_id(CapacitiveWaterSensor),
    cv.Required(CONF_SEND_PIN): cv.int_,
    cv.Required(CONF_RECEIVE_PIN): cv.int_,
    cv.Optional(CONF_NUM_SAMPLES, default=100): cv.int_range(min=1, max=1000),
    cv.Optional(CONF_MIN_RAW, default=4200): cv.int_,
    cv.Optional(CONF_MAX_RAW, default=11000): cv.int_,
    cv.Optional(CONF_SHORTED_VALUE, default=125): cv.int_range(min=100, max=255),
    cv.Optional(CONF_TIMEOUT, default=500): cv.int_range(min=1, max=5000),
}).extend(cv.polling_component_schema("500ms"))

async def to_code(config):
    """Генерация C++ кода из YAML конфигурации"""
    
    # Создаем переменную компонента
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_SEND_PIN],
        config[CONF_RECEIVE_PIN],
        config[CONF_UPDATE_INTERVAL].total_milliseconds,
    )
    
    # Регистрируем компонент
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    # Устанавливаем параметры
    cg.add(var.set_num_samples(config[CONF_NUM_SAMPLES]))
    cg.add(var.set_min_raw_value(config[CONF_MIN_RAW]))
    cg.add(var.set_max_raw_value(config[CONF_MAX_RAW]))
    cg.add(var.set_shorted_value(config[CONF_SHORTED_VALUE]))
    cg.add(var.set_timeout_ms(config[CONF_TIMEOUT]))
    
    # Добавляем библиотеку CapacitiveSensor
    cg.add_library("PaulStoffregen/CapacitiveSensor", "0.5.1")
    
    # Добавляем необходимые define для ESP32
    if CORE.is_esp32:
        cg.add_define("ESP32")
