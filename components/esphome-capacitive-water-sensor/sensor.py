import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL

CONF_SEND_PIN = "send_pin"
CONF_RECEIVE_PIN = "receive_pin"
CONF_NUM_SAMPLES = "num_samples"
CONF_MIN_RAW = "min_raw_value"
CONF_MAX_RAW = "max_raw_value"
CONF_TIMEOUT = "timeout_ms"

capacitive_water_sensor_ns = cg.esphome_ns.namespace("capacitive_water_sensor")
CapacitiveWaterSensor = capacitive_water_sensor_ns.class_(
    "CapacitiveWaterSensor", sensor.Sensor, cg.PollingComponent
)

CONFIG_SCHEMA = sensor.sensor_schema(
    CapacitiveWaterSensor,
    accuracy_decimals=0,
    device_class=""
).extend({
    cv.GenerateID(): cv.declare_id(CapacitiveWaterSensor),
    cv.Required(CONF_SEND_PIN): cv.int_,
    cv.Required(CONF_RECEIVE_PIN): cv.int_,
    cv.Optional(CONF_NUM_SAMPLES, default=1000): cv.int_range(min=1, max=10000),
    cv.Optional(CONF_MIN_RAW, default=4200): cv.int_,
    cv.Optional(CONF_MAX_RAW, default=11000): cv.int_,
    cv.Optional(CONF_TIMEOUT, default=500): cv.int_range(min=1, max=5000),
}).extend(cv.polling_component_schema("500ms"))

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
    cg.add(var.set_min_raw_value(config[CONF_MIN_RAW]))
    cg.add(var.set_max_raw_value(config[CONF_MAX_RAW]))
    cg.add(var.set_timeout_ms(config[CONF_TIMEOUT]))
    cg.add_library("PaulStoffregen/CapacitiveSensor", "0.5.1")
