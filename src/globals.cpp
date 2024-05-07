#include "globals.h"

MotorController global_motor_controller = {
    .motor_speed = 0,
    .tractive_voltage = 0,
    .motor_controller_temp = 0,
    .coolant_temp =0
};

BMS global_bms = {
    .max_temp = 0,
    .last_heartbeat = 0
};

Peripherals global_peripherals = {
    .pedal_position = 0,
    .brake_pressure = 0
};

TorqueMap global_torque_map = {
    .max_torque = 1200,
    .max_power = 500,
    .max_torque_scaling_factor = 1000,
    .max_power_scaling_factor = 1000,
    .target_speed_limit = 6000,
    .speed_limit_range = 500,
    .regen_enabled = 0
};

OutputPeripherals global_output_peripherals = {
    .requested_torque = 0,
    .flags = APPS_BSPC_INVALID | APPS_SENSOR_OUT_OF_RANGE_INVALID | APPS_SENSOR_CONFLICT_INVALID | BRAKE_SENSOR_OUT_OF_RANGE_INVALID | CTRL_RTD_INVALID
};
