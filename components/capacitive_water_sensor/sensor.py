import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import CONF_ID, STATE_CLASS_MEASUREMENT, UNIT_EMPTY

DEPENDENCIES = ["uart"]

capacitive_water_sensor_ns = cg.esphome_ns.namespace("capacitive_water_sensor")
CapacitiveWaterSensor = capacitive_water_sensor_ns.class_(
    "CapacitiveWaterSensor", cg.PollingComponent, sensor.Sensor, uart.UARTDevice
)

CONFIG_SCHEMA = sensor.sensor_schema(
    CapacitiveWaterSensor,
    unit_of_measurement=UNIT_EMPTY,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
).extend({
    cv.GenerateID(): cv.declare_id(CapacitiveWaterSensor),
    cv.Required("sender_pin"): cv.int_,
    cv.Required("sensor_pin"): cv.int_,
}).extend(cv.polling_component_schema("100ms")).extend(uart.UART_DEVICE_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    yield uart.register_uart_device(var, config)

    cg.add(var.set_pins(config["sender_pin"], config["sensor_pin"]))
