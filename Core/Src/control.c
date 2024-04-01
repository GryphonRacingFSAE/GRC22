#include "control.h"
#include "APPS.h"
#include "main.h"
// Array to store overflow counters for each wheel
volatile uint32_t tim_ovc[NUM_WHEELS] = { 0 };

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

// Array mapping each wheel to its corresponding timer
const TIM_HandleTypeDef* wheel_to_timer_mapping[NUM_WHEELS] = {&htim2, &htim3, &htim4, &htim3 };

Ctrl_Data_Struct Ctrl_Data = {
	.flags = CTRL_RTD_INVALID
};


// Task to manage control operations
void startControlTask() {
	uint32_t tick = osKernelGetTickCount();
	while(1){
		pressureSensorConversions();
		RTD();
		pumpCtrl();
		fanCtrl();
		LEDCtrl();
		osDelayUntil(tick += CTRL_PERIOD);
	}
}

void manageWheelSpeedTimerOverflow(const TIM_HandleTypeDef *htim) {
	// Check if the instance of the timer handler matches the current wheel's timer handler
	if (htim == &htim2) {
		// Increment overflow counter for the current wheel
		tim_ovc[0]++;
		// Reset frequency to 0 if overflow counter exceeds 2
		if (tim_ovc[0] >= 2) {
			Ctrl_Data.wheel_freq[0] = 0;
		}
	} else if (htim == &htim3) {
		tim_ovc[1]++;
		if (tim_ovc[1] >= 2) {
			Ctrl_Data.wheel_freq[1] = 0;
		}
		tim_ovc[3]++;
		if (tim_ovc[3] >= 2) {
			Ctrl_Data.wheel_freq[3] = 0;
		}
	} else if (htim == &htim4) {
		tim_ovc[2]++;
		if (tim_ovc[2] >= 2) {
			Ctrl_Data.wheel_freq[2] = 0;
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            calculateWheelSpeedFrequency(0, htim);
        }
    } else if (htim->Instance == TIM3) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            calculateWheelSpeedFrequency(1, htim);
        } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
            calculateWheelSpeedFrequency(3, htim);
        }
    } else if (htim->Instance == TIM4) {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            calculateWheelSpeedFrequency(2, htim);
        }
    }
}
void calculateWheelSpeedFrequency(uint8_t wheel_index, TIM_HandleTypeDef *htim) {
    static volatile uint32_t tim_rising[NUM_WHEELS][2] = { {0} }; // 2D array to store rising edge times for each wheel
    static volatile uint32_t tim_state[NUM_WHEELS] = {0};

	// Store timer value within rising edge variable

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    	tim_rising[wheel_index][tim_state[wheel_index]] = htim->Instance->CCR1;
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
    	tim_rising[wheel_index][tim_state[wheel_index]] = htim->Instance->CCR2;
    }
	// Calculate time interval between two rising edges at the same sensor
	uint32_t ticks_TIM = (tim_rising[wheel_index][tim_state[wheel_index]] + (tim_ovc[wheel_index] * htim->Init.Period)) - tim_rising[wheel_index][!tim_state[wheel_index]];
	// Calculate frequency and RPM if ticks is not zero and overflow counter is less than 2
	if (ticks_TIM != 0 && tim_ovc[wheel_index] < 2) {
		// Calculate frequency
		Ctrl_Data.wheel_freq[wheel_index] = (uint32_t) (96000000UL / (htim->Init.Prescaler + 1) / ticks_TIM);
		// Calculate RPM and store in Ctrl_Data.wheel_rpm array
		Ctrl_Data.wheel_rpm[wheel_index] = (Ctrl_Data.wheel_freq[wheel_index] * 60) / NUM_TEETH;
	}

	// Reset overflow counter for the current wheel
	tim_ovc[wheel_index] = 0;

	// Toggle state for the current wheel
	tim_state[wheel_index] = !tim_state[wheel_index];
}



// Ready to drive
// Rules: EV.10.4.3 & EV.10.5
void RTD() {
	static int callCounts = 0;

	if (callCounts * CTRL_PERIOD > 2000) {
		HAL_GPIO_WritePin(GPIO_RTD_BUZZER_GPIO_Port, GPIO_RTD_BUZZER_Pin, 0);
	}
	if (FLAG_ACTIVE(Ctrl_Data.flags, CTRL_RTD_INVALID)) {
		// Check if the pedal position is <3% to put APPS back into a valid state (EV.10.4.3)
		if (APPS_Data.apps_position < 30 && HAL_GPIO_ReadPin(GPIO_BRAKE_SW_GPIO_Port, GPIO_BRAKE_SW_Pin) && Ctrl_Data.tractive_voltage > RTD_TRACTIVE_VOLTAGE_ON && HAL_GPIO_ReadPin(GPIO_START_BTN_GPIO_Port, GPIO_START_BTN_Pin)) {
			CLEAR_FLAG(Ctrl_Data.flags, CTRL_RTD_INVALID); // Remove the invalid flag
			HAL_GPIO_WritePin(GPIO_RTD_BUZZER_GPIO_Port, GPIO_RTD_BUZZER_Pin, 1);
			callCounts = 0;
		} else if (Ctrl_Data.tractive_voltage < RTD_TRACTIVE_VOLTAGE_OFF) {
			// Disable RTD if TS drops below threshold
			SET_FLAG(Ctrl_Data.flags, CTRL_RTD_INVALID);
		}
	}
	callCounts++;
}

// Motor & Motor controller cooling pump control
void pumpCtrl() {

	//pump off
	pumpCycle(0);
	// Turn on pump based on motor controller temperature threshold and tractive voltage threshold
	if (Ctrl_Data.motor_controller_temp > PUMP_MOTOR_CONTROLLER_TEMP_THRESHOLD || Ctrl_Data.tractive_voltage > PUMP_TRACTIVE_VOLTAGE_THRESHOLD) {
		SET_FLAG(Ctrl_Data.flags, PUMP_ACTIVE);
		HAL_GPIO_WritePin(GPIO_PUMP_GPIO_Port, GPIO_PUMP_Pin, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIO_PUMP_GPIO_Port, GPIO_PUMP_Pin, GPIO_PIN_SET); // Turn on pump if cannot acquire mutex
		ERROR_PRINT(
				"Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:pumpCtrl\n");
	}
}

// Cooling pump duty cycles based on input of desired pump speed in percentage
void pumpCycle(uint8_t pump_speed){

	if(pump_speed == 0){			//pump off
		TIM1 -> CCR1 = 0;			//duty cycle between 0-12%
	} else if (pump_speed == 100){	//max speed
		TIM1 -> CCR1 = 3600;		//duty cycle between 86-97%
	} else{							//between 1-99% pump speed, 13-85% duty cycle
		uint32_t duty_cycle = (((pump_speed-1)/(99-1))*(85-13))+13;
		TIM1 -> CCR1 = (duty_cycle*4000)/100;	//divide by 100 since duty cycle is in percentage not decimal value
	}
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

// Motor controller cooling fan control
void fanCtrl() {
	// Turn on fan based on coolant temperature threshold
	if (Ctrl_Data.coolant_temp > RAD_FAN_COOLANT_TEMP_THRESHOLD) {
		SET_FLAG(Ctrl_Data.flags, RADIATOR_FAN_ACTIVE);
		HAL_GPIO_WritePin(GPIO_RAD_FAN_GPIO_Port, GPIO_RAD_FAN_Pin, GPIO_PIN_SET);
	} else {
		CLEAR_FLAG(Ctrl_Data.flags, RADIATOR_FAN_ACTIVE);
		HAL_GPIO_WritePin(GPIO_RAD_FAN_GPIO_Port, GPIO_RAD_FAN_Pin, GPIO_PIN_RESET);
	}


	if (Ctrl_Data.accumulator_max_temp > ACC_FAN_ACC_TEMP_THRESHOLD) {
		SET_FLAG(Ctrl_Data.flags, ACCUMULATOR_FAN_ACTIVE);
		HAL_GPIO_WritePin(GPIO_ACC_FAN_GPIO_Port, GPIO_ACC_FAN_Pin, GPIO_PIN_SET);
	} else {
		CLEAR_FLAG(Ctrl_Data.flags, ACCUMULATOR_FAN_ACTIVE);
		HAL_GPIO_WritePin(GPIO_ACC_FAN_GPIO_Port, GPIO_ACC_FAN_Pin, GPIO_PIN_RESET);
	}
}

void pressureSensorConversions(){
	uint32_t pressure1_adc_avg = 0;
	uint32_t pressure2_adc_avg = 0;
	int32_t pressure1 = 0;
	int32_t pressure2 = 0;
//
//	//sum of all pressure 1 values
//	for(uint16_t i = 1; i < ADC_CHANNEL_3_DMA_BUFFER_LEN; i += 3){
//		pressure1_adc_avg += adc_3_dma_buffer[i];
//	}
//	//sum of all pressure 2 values
//	for(uint16_t i = 2; i < ADC_CHANNEL_3_DMA_BUFFER_LEN; i += 3){
//		pressure2_adc_avg += adc_3_dma_buffer[i];
//	}
//	pressure1_adc_avg /= (ADC_CHANNEL_3_DMA_BUFFER_LEN/ADC_CHANNEL_3_DMA_CHANNELS);
//	pressure2_adc_avg /= (ADC_CHANNEL_3_DMA_BUFFER_LEN/ADC_CHANNEL_3_DMA_CHANNELS);

	// RULE (2024 V1): T.4.2.10 (Detect open circuit and short circuit conditions)
	// TODO: add flags for pressure sensor shorts
	if(pressure1_adc_avg <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= pressure1_adc_avg){
		GRCprintf("possible short detected at pressure sensor 1");
	} else if(pressure2_adc_avg <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= pressure2_adc_avg){
		GRCprintf("Possible short detected at pressure sensor 2");
	}

	pressure1 = CLAMP(PRESSURE_SENSOR_MIN, pressure1_adc_avg, PRESSURE_SENSOR_MIN);
	pressure2 = CLAMP(PRESSURE_SENSOR_MIN, pressure2_adc_avg, PRESSURE_SENSOR_MAX);

	Ctrl_Data.pressure_readings[0] = (((pressure1 - PRESSURE_SENSOR_MIN)/(PRESSURE_SENSOR_MAX - PRESSURE_SENSOR_MIN))*PRESSURE_RANGE);
	Ctrl_Data.pressure_readings[1] = (((pressure2 - PRESSURE_SENSOR_MIN)/(PRESSURE_SENSOR_MAX - PRESSURE_SENSOR_MIN))*PRESSURE_RANGE);

	GRCprintf("Pressure 1 = ", Ctrl_Data.pressure_readings[0]);
	GRCprintf("Pressure 2 = ", Ctrl_Data.pressure_readings[1]);
}


void LEDCtrl() {

}
