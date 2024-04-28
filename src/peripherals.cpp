#include "peripherals.h"
#include "globals.h"
#include "utils.h"

#include <freertos/task.h>

#define APPS1_MIN 1525
#define APPS1_MAX 1280
#define APPS2_MIN 3250
#define APPS2_MAX 2740
#define BRAKE_PRESSURE_MIN 290
#define BRAKE_PRESSURE_MAX (4095 * 9 / 10)
#define ADC_SHORTED_GND 150
#define ADC_SHORTED_VCC 3950


uint16_t analogReadRepeated(uint8_t pin) {
    uint32_t adc_avg = 0;
    const uint8_t counts = 128;
    for (uint8_t i = 0; i < counts; i++) {
        adc_avg += analogRead(pin);
    }
    adc_avg /= counts;
    return adc_avg;
}

void startPeripheralTask(void* pvParameters) {
    (void)pvParameters;

    TickType_t tick = xTaskGetTickCount();

    bool push_button_status = LOW;
    TickType_t last_push_button_change = tick;
    while (1) {
        uint16_t apps1_adc = analogReadRepeated(APPS1_PIN);
        uint16_t apps2_adc = analogReadRepeated(APPS2_PIN);
        
		// RULE (2024 V1): T.4.2.10 (Detect open circuit and short circuit conditions)
		if (apps1_adc <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= apps1_adc || apps2_adc <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= apps2_adc) {
			SET_FLAG(global_output_peripherals.flags, APPS_SENSOR_OUT_OF_RANGE_INVALID);
		} else {
			CLEAR_FLAG(global_output_peripherals.flags, APPS_SENSOR_OUT_OF_RANGE_INVALID);
		}

		int32_t apps1_pos = CLAMP(0, (apps1_adc - APPS1_MIN) * 1000 /(APPS1_MAX - APPS1_MIN), 1000);
		int32_t apps2_pos = CLAMP(0, (apps2_adc - APPS2_MIN) * 1000 /(APPS2_MAX - APPS2_MIN), 1000);

        Serial.print("apps1:");
        Serial.print(apps1_adc);
        Serial.print(",apps2:");
        Serial.print(apps2_adc);
        Serial.print(",apps1 pos:");
        Serial.print(apps1_pos);
        Serial.print(",apps2 pos:");
        Serial.print(apps2_pos);

		// RULE (2024 V1): T.4.2.4 (Both APPS sensor positions must be within 10% of pedal travel of each other)
		if (ABS(apps1_pos - apps2_pos) <= 100) {
			global_peripherals.pedal_position = (apps1_pos + apps2_pos) / 2;
			CLEAR_FLAG(global_output_peripherals.flags, APPS_SENSOR_CONFLICT_INVALID);
		} else {
            global_peripherals.pedal_position = 0;
			SET_FLAG(global_output_peripherals.flags, APPS_SENSOR_CONFLICT_INVALID);
		}

        int32_t brake_pressure_adc = analogReadRepeated(BRAKE_PRESSURE_PIN);
        Serial.print(",brake:");
        Serial.print(brake_pressure_adc);
		if (brake_pressure_adc <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= brake_pressure_adc) {
			SET_FLAG(global_output_peripherals.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
            global_peripherals.brake_pressure = 0;
		} else {
			CLEAR_FLAG(global_output_peripherals.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
            global_peripherals.brake_pressure = CLAMP(0, (brake_pressure_adc - BRAKE_PRESSURE_MIN) * 20000 /(BRAKE_PRESSURE_MAX - BRAKE_PRESSURE_MIN), 20000);
		}
        Serial.print(",brake pressure:");
        Serial.println(global_peripherals.brake_pressure);


		// RULE (2024 V1): T.4.7 (BSPC)
		if (FLAG_ACTIVE(global_output_peripherals.flags, APPS_BSPC_INVALID)){
			// Check if the pedal position is <5% to put APPS back into a valid state (EV.4.7.2)
			if (global_peripherals.pedal_position < 50) {
				CLEAR_FLAG(global_output_peripherals.flags, APPS_BSPC_INVALID);
			}
		} else if (global_peripherals.pedal_position > 250 && global_peripherals.brake_pressure > 500) {
			// Set to invalid if over >25% travel and brakes engaged (EV.4.7.1)
			SET_FLAG(global_output_peripherals.flags, APPS_BSPC_INVALID);
		}

        // read the state of the switch into a local variable:
        int push_button = digitalRead(PUSH_BUTTON_PIN);
        if (push_button != push_button_status) {
            // reset the debouncing timer
            last_push_button_change = tick;
        }

        if ((tick - last_push_button_change) > pdMS_TO_TICKS(10)) {
            push_button_status = push_button;
            if (push_button_status) {
                CLEAR_FLAG(global_output_peripherals.flags, RTD_BUTTON);
            } else {
                SET_FLAG(global_output_peripherals.flags, RTD_BUTTON);
            }
        }
        xTaskDelayUntil(&tick, pdMS_TO_TICKS(2));
    }

    // TODO: T.4.2.5
    // TODO: T.4.3.3
    // TODO: T.4.3.4
}