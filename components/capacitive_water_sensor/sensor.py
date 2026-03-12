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

# Константы
CONF_SEND_PIN = "send_pin"
CONF_RECEIVE_PIN = "receive_pin"
CONF_NUM_SAMPLES = "num_samples"
CONF_TIMEOUT = "timeout_ms"
CONF_USE_SOFT_THRESHOLD = "use_soft_threshold"
CONF_DRY_THRESHOLD = "dry_threshold"
CONF_WET_MIN_RAW = "wet_min_raw"
CONF_WET_MAX_RAW = "wet_max_raw"
CONF_SHORTED_VALUE = "shorted_value"

capacitive_water_sensor_ns = cg.esphome_ns.namespace("capacitive_water_sensor")
CapacitiveWaterSensor = capacitive_water_sensor_ns.class_(
    "CapacitiveWaterSensor", sensor.Sensor, cg.PollingComponent
)

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
    cv.Optional(CONF_NUM_SAMPLES, default=10): cv.int_range(min=5, max=30),
    cv.Optional(CONF_TIMEOUT, default=100): cv.int_range(min=50, max=300),
    cv.Optional(CONF_USE_SOFT_THRESHOLD, default=True): cv.boolean,
    cv.Optional(CONF_DRY_THRESHOLD, default=300): cv.int_range(min=100, max=1000),
    cv.Optional(CONF_WET_MIN_RAW, default=0): cv.int_,
    cv.Optional(CONF_WET_MAX_RAW, default=300): cv.int_,
    cv.Optional(CONF_SHORTED_VALUE, default=125): cv.int_range(min=100, max=255),
}).extend(cv.polling_component_schema("3s"))

async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_SEND_PIN],
        config[CONF_RECEIVE_PIN],
        config[CONF_UPDATE_INTERVAL].total_milliseconds,
    )
    
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    cg.add(var.set_num_samples(config[CONF_NUM_SAMPLES]))
    cg.add(var.set_timeout_ms(config[CONF_TIMEOUT]))
    cg.add(var.set_use_soft_threshold(config[CONF_USE_SOFT_THRESHOLD]))
    cg.add(var.set_dry_threshold(config[CONF_DRY_THRESHOLD]))
    cg.add(var.set_wet_min_raw(config[CONF_WET_MIN_RAW]))
    cg.add(var.set_wet_max_raw(config[CONF_WET_MAX_RAW]))
    cg.add(var.set_shorted_value(config[CONF_SHORTED_VALUE]))
