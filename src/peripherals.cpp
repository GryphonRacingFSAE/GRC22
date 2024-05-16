#include "peripherals.h"
#include "globals.h"
#include "utils.h"
#include <freertos/task.h>
static uint32_t imd_rising0 = 0;
static uint32_t imd_rising1 = 0;
static uint32_t imd_falling = 0;


void IRAM_ATTR imdRisingEdgeTime(void){
    imd_rising0 = imd_rising1; 
    imd_rising1 = micros();
    if(imd_rising1 != imd_rising0){
        global_imd.frequency = 10000000 / (imd_rising1 - imd_rising0);
    }
}

void IRAM_ATTR imdFallingEdgeTime(void){
    imd_falling = micros();
    
    if(imd_rising1 != imd_rising0){
        global_imd.duty_cycle = 1000 - (imd_falling - imd_rising1) * 1000 / (imd_rising1 - imd_rising0);
    }
}

void imdReadings(uint32_t duty_cycle, uint32_t frequency){
    Serial.printf("Difference %d \r\n", (imd_rising1 - imd_rising0));
    Serial.printf("Frequency %d \r\n", global_imd.frequency);
    Serial.printf("Duty Cycle %d \r\n", global_imd.duty_cycle);

    if (frequency < 30)
    {
        global_imd.state = IMD_SHORT_CIRCUIT;
    } 
    if (frequency > 70 && frequency < 130)
    {
        global_imd.resistance = ((90 - (1200 * 1000))/(duty_cycle - 5)) - (1200 * 1000);
        global_imd.state = IMD_NORMAL_CONDITION;  
    }
    if(frequency > 170 && frequency < 230){
        global_imd.resistance = ((90 - (1200 * 1000))/(duty_cycle - 5)) - (1200 * 1000);
        global_imd.state = IMD_UNDERVOLTAGE;
    }
    if (frequency > 270 && frequency < 330)
    {
        global_imd.state = IMD_STARTUP;
    }
    if (frequency > 370 && frequency < 430)
    {
        global_imd.state = IMD_DEVICE_ERROR;
    } 
    if (frequency > 470 && frequency < 530)
    {
        global_imd.state = IMD_EARTH_FAULT;
    }  
}

void pumpCycle(uint8_t pump_speed) {
    if (pump_speed == 0) {
        ledcWrite(0, 1023 - 1023 * 10 / 100); // Stop / Error Reset
    } else if (pump_speed == 100) {
        ledcWrite(0, 1023 - 1023 * 90 / 100); // Maximum Speed
    } else {
        // between 1-99% pump speed, 13-85% duty cycle (We only use from 15% - 85% duty cycle as 15% duty cycle at 50Hz will wake the pump)
        uint32_t duty_cycle = 1023 - (1023 * (pump_speed - 1) * (85 - 15) / (99 - 1) + 1023 * 15) / 100;
        ledcWrite(0, duty_cycle); // Controlled operation from min to max speed
    }
}

uint16_t analogReadRepeated(uint8_t pin) {
    uint32_t adc_avg = 0;
    const uint16_t counts = 150;
    for (uint16_t i = 0; i < counts; i++) {
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

        int32_t apps1_pos = CLAMP(0, (apps1_adc - APPS1_MIN) * 1000 / (APPS1_MAX - APPS1_MIN), 1000);
        int32_t apps2_pos = CLAMP(0, (apps2_adc - APPS2_MIN) * 1000 / (APPS2_MAX - APPS2_MIN), 1000);

        // Serial.print("apps1:");
        // Serial.print(apps1_adc);
        // Serial.print(",apps2:");
        // Serial.print(apps2_adc);
        // Serial.print(",apps1 pos:");
        // Serial.print(apps1_pos);
        // Serial.print(",apps2 pos:");
        // Serial.println(apps2_pos);

        // RULE (2024 V1): T.4.2.4 (Both APPS sensor positions must be within 10% of pedal travel of each other)
        if (ABS(apps1_pos - apps2_pos) <= 100) {
            global_peripherals.pedal_position = (apps1_pos + apps2_pos) / 2;
            CLEAR_FLAG(global_output_peripherals.flags, APPS_SENSOR_CONFLICT_INVALID);
        } else {
            global_peripherals.pedal_position = 0;
            SET_FLAG(global_output_peripherals.flags, APPS_SENSOR_CONFLICT_INVALID);
        }

        int32_t brake_pressure_adc = analogReadRepeated(BRAKE_PRESSURE_PIN);
        // Serial.print(",brake:");
        // Serial.print(brake_pressure_adc);
        if (brake_pressure_adc <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= brake_pressure_adc) {
            SET_FLAG(global_output_peripherals.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
            global_peripherals.brake_pressure = 0;
        } else {
            CLEAR_FLAG(global_output_peripherals.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
            global_peripherals.brake_pressure =
                CLAMP(0, (brake_pressure_adc - BRAKE_PRESSURE_MIN) * 20000 / (BRAKE_PRESSURE_MAX - BRAKE_PRESSURE_MIN), 20000);
        }
        // Serial.print(",brake pressure:");
        // Serial.println(global_peripherals.brake_pressure);

        // RULE (2024 V1): T.4.7 (BSPC)
        if (FLAG_ACTIVE(global_output_peripherals.flags, APPS_BSPC_INVALID)) {
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
        if (push_button) {
            SET_FLAG(global_output_peripherals.flags, RTD_BUTTON);
        } else {
            CLEAR_FLAG(global_output_peripherals.flags, RTD_BUTTON);
        }
        // if (push_button != push_button_status) {
        //     // reset the debouncing timer
        //     last_push_button_change = tick;
        // }

        // if ((tick - last_push_button_change) > pdMS_TO_TICKS(10)) {
        //     push_button_status = push_button;
        //
        // }
        // xTaskDelayUntil(&tick, pdMS_TO_TICKS(2));
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    // TODO: T.4.2.5
    // TODO: T.4.3.3
    // TODO: T.4.3.4
}

void startControlTask(void* pvParameters) {
    (void)pvParameters;


    static int rtd_call_counts = 0;

    TickType_t tick = xTaskGetTickCount();
    while (1) {
        
        imdReadings(global_imd.duty_cycle, global_imd.frequency);


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
            if (global_peripherals.pedal_position < 30 && global_peripherals.brake_pressure > 800 &&
                global_motor_controller.tractive_voltage > RTD_TRACTIVE_VOLTAGE_ON && FLAG_ACTIVE(global_output_peripherals.flags, RTD_BUTTON)) {
                CLEAR_FLAG(global_output_peripherals.flags, CTRL_RTD_INVALID); // Remove the invalid flag
                digitalWrite(BUZZER_PIN, 1);
                // digitalWrite(LED_PIN, 1);
                rtd_call_counts = 0;
            }
        } else if (global_motor_controller.tractive_voltage < RTD_TRACTIVE_VOLTAGE_OFF ||
                   (global_peripherals.pedal_position < 30 && global_peripherals.brake_pressure < 800 &&
                    FLAG_ACTIVE(global_output_peripherals.flags, RTD_BUTTON))) {
            SET_FLAG(global_output_peripherals.flags, CTRL_RTD_INVALID); // Add the invalid flag
            digitalWrite(BUZZER_PIN, 0);
            // digitalWrite(LED_PIN, 0);
        }

        if (global_motor_controller.motor_controller_temp > PUMP_MOTOR_CONTROLLER_TEMP_THRESHOLD ||
            !FLAG_ACTIVE(global_output_peripherals.flags, CTRL_RTD_INVALID)) {
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