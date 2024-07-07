#include "torque.h"
#include "globals.h"
#include "utils.h"
#include <Preferences.h>
#include <freertos/task.h>

int32_t kmphToRPM(int32_t kmph) {
    return kmph * 4500 / 99 / 10;
}

void startTorqueTask(void* pvParameters) {
    (void)pvParameters;

    TickType_t tick = xTaskGetTickCount();

    while (1) {
        int32_t max_torque = (int32_t)param_storage.getShort("torqueRaw", 700);
        int16_t rpm = global_motor_controller.motor_speed;

        int16_t requested_torque = max_torque;
        // Max torque as calculated from max_power
        if (rpm != 0) {
            int32_t max_power_watts = (int32_t)param_storage.getShort("powerRaw", 50) * 1000;
            int16_t max_torque_from_power = max_power_watts * 95492 / rpm / 10000;
            requested_torque = MIN(max_torque, max_torque_from_power);
        }

        // Scale based on pedal position
        requested_torque = requested_torque * global_peripherals.pedal_position / 1000;

        int16_t low_speed_cutoff = param_storage.getShort("targetSpeedLim", 6250) - SPEED_LIM_RANGE / 2;
        int16_t high_speed_cutoff = param_storage.getShort("targetSpeedLim", 6250) + SPEED_LIM_RANGE / 2;

        if (low_speed_cutoff <= rpm) {
            requested_torque = requested_torque * (high_speed_cutoff - rpm) / SPEED_LIM_RANGE;
        }

        // RULE (2024 V1): EV.3.3.3 No regen < 5km/h
        if (!REGEN_ENABLED || rpm < kmphToRPM(5)) {
            requested_torque = MAX(requested_torque, 0);
        }
        global_output_peripherals.requested_torque = requested_torque;

        vTaskDelay(pdMS_TO_TICKS(TORQUE_TASK_PERIOD));
    }
}