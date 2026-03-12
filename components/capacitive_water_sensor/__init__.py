import esphome.codegen as cg
import esphome.config_validation as cv

CODEOWNERS = ["@your_github_username"]
capacitive_water_sensor_ns = cg.esphome_ns.namespace("capacitive_water_sensor")

CONFIG_SCHEMA = cv.All(
    cv.Schema({}),
)
