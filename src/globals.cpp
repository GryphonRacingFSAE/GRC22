#include "globals.h"

MotorController global_motor_controller = {.motor_speed = 0, .tractive_voltage = 0, .motor_controller_temp = 0, .coolant_temp = 0};

BMS global_bms = {.max_temp = 0, .DTC1 = 0, .DTC1_mask = 0, .DTC2 = 0, .DTC2_mask = 0, .last_heartbeat = 0};

Peripherals global_peripherals = {.pedal_position = 0, .brake_pressure = 0, .flow_rate = 0};

OutputPeripherals global_output_peripherals = {.requested_torque = 0,
                                               .flags = APPS_BSPC_INVALID | APPS_SENSOR_OUT_OF_RANGE_INVALID | APPS_SENSOR_CONFLICT_INVALID |
                                                        BRAKE_SENSOR_OUT_OF_RANGE_INVALID | CTRL_RTD_INVALID};

IMD global_imd = {.duty_cycle = 0, .frequency = 0, .resistance = 0, .state = IMD_STARTUP};
