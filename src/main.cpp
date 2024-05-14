#include <Arduino.h>
#include <HardwareSerial.h>
#include <driver/twai.h>
#include <freertos/task.h>
#include <can.h>
#include <globals.h>
#include <peripherals.h>
#include <utils.h>

#define CONTROL_TASK_PERIOD 5

int32_t kmphToRPM(int32_t kmph) {
	return kmph * 4500/99/10;
}

void pumpCycle(uint8_t pump_speed) {
	if(pump_speed == 0) {
        ledcWrite(0, 1023 - 1023 * 10 / 100); // Stop / Error Reset
	} else if (pump_speed == 100) {
        ledcWrite(0, 1023 - 1023 * 90 / 100); // Maximum Speed
	} else {
		// between 1-99% pump speed, 13-85% duty cycle (We only use from 15% - 85% duty cycle as 15% duty cycle at 50Hz will wake the pump)
		uint32_t duty_cycle = 1023 - (1023 * (pump_speed-1)*(85-15)/(99-1) + 1023 * 15) / 100;
        ledcWrite(0, duty_cycle); // → Controlled operation from min to max speed
	}
}

void startBMSTask(void* params) {
    (void)params;
    
    TickType_t tick = xTaskGetTickCount();
    while (1) {
        uint32_t can_error = 0;
        twai_read_alerts(&can_error, 0);
        // Serial.printf("DTC1: %04x, DTC2: %04x\n", global_bms.DTC1, global_bms.DTC2);
        if (!can_error && (tick < (global_bms.last_heartbeat + 1000))) {
            if ((global_bms.DTC1 & 0x00FF) || (global_bms.DTC2 & 0xFFF2)) {
                digitalWrite(AMS_SHUTDOWN_PIN, LOW);
            } else {
                digitalWrite(AMS_SHUTDOWN_PIN, HIGH);
            }
        } else {
            digitalWrite(AMS_SHUTDOWN_PIN, LOW);
        }
        
        xTaskDelayUntil(&tick, pdMS_TO_TICKS(10));
    }
}

void startControlTask(void *pvParameters) {
    (void)pvParameters;

	static int rtd_call_counts = 0;

    TickType_t tick = xTaskGetTickCount();
    while (1) {
        if (global_peripherals.brake_pressure > 300) {
            digitalWrite(BRAKE_LIGHT_PIN, HIGH);
        } else {
            digitalWrite(BRAKE_LIGHT_PIN, LOW);
        }

        if (rtd_call_counts * CONTROL_TASK_PERIOD > 2000) {
            digitalWrite(BUZZER_PIN, 0);
        }

        if (FLAG_ACTIVE(global_output_peripherals.flags, CTRL_RTD_INVALID)) {
            // Check if the pedal position is <3% to put APPS back into a valid state (EV.10.4.3)
            if (global_peripherals.pedal_position < 30 && global_peripherals.brake_pressure > 800 && global_motor_controller.tractive_voltage > RTD_TRACTIVE_VOLTAGE_ON && FLAG_ACTIVE(global_output_peripherals.flags, RTD_BUTTON)) {
                CLEAR_FLAG(global_output_peripherals.flags, CTRL_RTD_INVALID); // Remove the invalid flag
                digitalWrite(BUZZER_PIN, 1);   
                digitalWrite(LED_PIN, 1);
                rtd_call_counts = 0;
            }
        } else if (global_motor_controller.tractive_voltage < RTD_TRACTIVE_VOLTAGE_OFF || (global_peripherals.pedal_position < 30 && global_peripherals.brake_pressure < 800 && FLAG_ACTIVE(global_output_peripherals.flags, RTD_BUTTON))) {
            SET_FLAG(global_output_peripherals.flags, CTRL_RTD_INVALID); // Add the invalid flag
            digitalWrite(BUZZER_PIN, 0);
            digitalWrite(LED_PIN, 0);
        }
        
        if (global_motor_controller.motor_controller_temp > PUMP_MOTOR_CONTROLLER_TEMP_THRESHOLD || !FLAG_ACTIVE(global_output_peripherals.flags, CTRL_RTD_INVALID)) {
            SET_FLAG(global_output_peripherals.flags, PUMP_ACTIVE);
            pumpCycle(60);
        } else {
            CLEAR_FLAG(global_output_peripherals.flags, PUMP_ACTIVE);
            pumpCycle(0);
        }

        // Turn on fan based on coolant temperature threshold
        if (global_motor_controller.coolant_temp > RAD_FAN_COOLANT_TEMP_THRESHOLD) {
            SET_FLAG(global_output_peripherals.flags, RADIATOR_FAN_ACTIVE);
            digitalWrite(RAD_FAN_PIN, HIGH);
        } else {
            CLEAR_FLAG(global_output_peripherals.flags, RADIATOR_FAN_ACTIVE);
            digitalWrite(RAD_FAN_PIN, LOW);
        }

        // if (global_bms.max_temp > ACC_FAN_ACC_TEMP_THRESHOLD) {
        //     SET_FLAG(global_output_peripherals.flags, ACCUMULATOR_FAN_ACTIVE);
        //     digitalWrite(ACCUM_FAN_PIN, HIGH);
        // } else {
        //     CLEAR_FLAG(global_output_peripherals.flags, ACCUMULATOR_FAN_ACTIVE);
        //     digitalWrite(ACCUM_FAN_PIN, LOW);
        // }

        rtd_call_counts++;
        xTaskDelayUntil(&tick, pdMS_TO_TICKS(CONTROL_TASK_PERIOD));
    }
}

void startAPPSTask(void *pvParameters) {
    (void)pvParameters;

    TickType_t tick = xTaskGetTickCount();

    while (1) {
		int32_t max_torque = (int32_t)global_torque_map.max_torque * global_torque_map.max_torque_scaling_factor / 1000;
		int16_t rpm = global_motor_controller.motor_speed;

		int16_t requested_torque = max_torque;
		// Max torque as calculated from max_power
		if (rpm != 0) {
			int32_t max_power_watts = (int32_t)global_torque_map.max_power * 1000 * global_torque_map.max_power_scaling_factor / 1000;
			int16_t max_torque_from_power = max_power_watts * 95492 / rpm / 10000;
			requested_torque = MIN(max_torque, max_torque_from_power);
		}

		// Scale based on pedal position
		requested_torque = requested_torque * global_peripherals.pedal_position / 1000;

		int16_t low_speed_cutoff = global_torque_map.target_speed_limit - global_torque_map.speed_limit_range / 2;
		int16_t high_speed_cutoff = global_torque_map.target_speed_limit + global_torque_map.speed_limit_range / 2;

		if (low_speed_cutoff <= rpm) {
			requested_torque = requested_torque * (high_speed_cutoff - rpm) / global_torque_map.speed_limit_range;
		}

		// RULE (2024 V1): EV.3.3.3 No regen < 5km/h
		if (!global_torque_map.regen_enabled || rpm < kmphToRPM(5)) {
			requested_torque = MAX(requested_torque, 0);
		}
		global_output_peripherals.requested_torque = requested_torque;
        // xTaskDelayUntil(&tick, pdMS_TO_TICKS(3));
        
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}

void setup() {
    Serial.begin(921600);

    initCAN();

    ledcSetup(0, 50, 10);   // 50Hz PWM, 10-bit resolution
    pinMode(PUMP_PWM_PIN, OUTPUT);
    ledcAttachPin(PUMP_PWM_PIN, 0); // assign RGB led pins to channels
    pumpCycle(0);

    pinMode(APPS1_PIN, INPUT);
    pinMode(APPS2_PIN, INPUT);
    pinMode(BRAKE_PRESSURE_PIN, INPUT);
    pinMode(PUSH_BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BRAKE_LIGHT_PIN, OUTPUT);
    pinMode(AMS_SHUTDOWN_PIN, OUTPUT);

    xTaskCreate(startBMSTask, "CONTROL_TASK", 2048, NULL, 8, NULL);
    Serial.println("Finished creating task 0");
    xTaskCreate(startControlTask, "CONTROL_TASK", 8192, NULL, 2, NULL);
    Serial.println("Finished creating task 1");
    xTaskCreate(startPeripheralTask, "PERIPHERAL_TASK", 8192, NULL, 2, NULL);
    Serial.println("Finished creating task 2");
    xTaskCreate(startAPPSTask, "APPS_TASK", 8192, NULL, 2, NULL);
    Serial.println("Finished creating task 3");
    xTaskCreate(startReceiveCANTask, "CAN_RECEIVE_TASK", 8192, NULL, 3, NULL);
    Serial.println("Finished creating task 4");
    xTaskCreate(startTransmitCANTask, "CAN_TRANSMIT_TASK", 8192, NULL, 5, NULL);
    Serial.println("Finished creating task 5");
}

void loop() {}
