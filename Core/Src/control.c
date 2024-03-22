#include "control.h"
#include "APPS.h"
#include "utils.h"
#include "main.h"
#include <string.h>

extern TIM_HandleTypeDef htim1;

//overflow counters for wheel speed sensors
volatile uint16_t TIM2_OVC = 0;
volatile uint16_t TIM3_CH1_OVC = 0;
volatile uint16_t TIM3_CH2_OVC = 0;
volatile uint16_t TIM4_OVC = 0;

//state variables for wheel speed (refer to rising or falling edge of each read iteration)
volatile uint8_t TIM2_State = 0;
volatile uint8_t TIM3_CH1_State = 0;
volatile uint8_t TIM3_CH2_State = 0;
volatile uint8_t TIM4_State = 0;

//rising edge and falling edge variables for each sensor
volatile uint32_t TIM2_rising = 0;
volatile uint32_t TIM2_rising2 = 0;

volatile uint32_t TIM3_CH1_rising = 0;
volatile uint32_t TIM3_CH1_rising2 = 0;

volatile uint32_t TIM3_CH2_rising = 0;
volatile uint32_t TIM3_CH2_rising2 = 0;

volatile uint32_t TIM4_rising = 0;
volatile uint32_t TIM4_rising2 = 0;

//frequency values for each wheel speed sensor
volatile uint32_t TIM2_freq = 0;
volatile uint32_t TIM3_CH1_freq = 0;
volatile uint32_t TIM3_CH2_freq = 0;
volatile uint32_t TIM4_freq = 0;

//wheel frequency array (stores each wheel's frequency)
volatile uint32_t wheelFreq[4];
volatile uint32_t wheelRPM[4];

//number of teeth on wheel hub
volatile uint16_t numTeeth = 32;

Ctrl_Data_Struct Ctrl_Data = {
	.flags = CTRL_RTD_INVALID
};

void startControlTask(){
	uint32_t tick = osKernelGetTickCount();
	while(1){
		RPMConversion();
		RTD();
		pumpCtrl();
		fanCtrl();
		LEDCtrl();
		osDelayUntil(tick += CTRL_PERIOD);
	}
}

/*
 * runs checks for period overflow, after 2 overflows the wheel is not moving
 * 	if the instance is equal to TIM1, hal tick is incremented in order to keep all timing accurate and synchronized
 *
 * 	if instance is equal to TIM2
 * 		increment tim2 overflow count
 * 			if overflow is greater than or equal to 2, set tim2 frequency to 0
 *
 * 	if instance is equal to TIM3
 * 		check if CH1 or CH2 is the active channel (same pattern of work for each channel just using channel specific variables)
 * 			increment overflow count
 * 				if overflow count is greater than or equal to 2, set wheel frequency to 0
 *
 * 	if instance is equal to TIM2
 * 		increment tim2 overflow count
 * 			if overflow is greater than or equal to 2, set tim2 frequency to 0

 */

void OverflowCheck(TIM_HandleTypeDef * htim){

	//instance for tim2
	if(htim -> Instance == TIM2){
		TIM2_OVC++;
		if(TIM2_OVC >= 2){
			TIM2_freq = 0;
		}
	}
	//instance for tim3 (channel checks)
	if (htim -> Instance == TIM3){
		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			TIM3_CH1_OVC ++;
			if(TIM3_CH1_OVC >= 2){
				TIM3_CH1_freq = 0;
			}
		}
		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_2){
			TIM3_CH2_OVC ++;
			if(TIM3_CH2_OVC >= 2){
				TIM3_CH2_freq = 0;
			}
		}
	}
	//instance for tim4
	if(htim -> Instance == TIM4){
		TIM4_OVC ++;
		if(TIM4_OVC >= 2){
			TIM4_freq = 0;
		}
	}
}

/*
 * Called when an input capture event occurs
 * using if statements (subject to change if more efficient method is found):
 * 		Check to see which timer the event occurs at (TIM2, TIM3 (need to also check which channel (CH1, CH2), TIM4)
 * for each instance the same algorithm is applied:
 *
 * 		if state 0:
 * 			store timer value within rising edge variable
 * 			calculate time interval between 2 rising edges at the same sensor
 * 			if ticks is not zero and overflow counter is less than 2, calculate frequency
 * 			store frequency within corresponding wheel frequency index in array
 * 			reset overflow counter, set state to 1
 * 		if state 1:
 * 			store timer value within falling edge variable
 * 			calculate time interval between 2 rising edges at the same sensor
 * 			if ticks is not zero and overflow counter is less than 2, calculate frequency
 * 			store frequency within corresponding wheel frequency index in array
 * 			reset overflow counter, set state to 0
 *
 * 	TIMER to array index correspondence:
 * 		TIM2 = 0
 * 		TIM3_CH1 = 1
 * 		TIM3_CH2 = 2
 * 		TIM4 = 3
 */

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim){

	//timer 2 input
	if(htim -> Instance == TIM2){
		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			//state 0
			if (TIM2_State == 0){
				TIM2_rising = TIM2 -> CCR1;
				uint32_t ticks_TIM2 = (TIM2_rising + (TIM2_OVC * htim->Init.Period)) - TIM2_rising2;
				if(ticks_TIM2 != 0 && TIM2_OVC < 2){
					TIM2_freq = (uint32_t)(96000000UL/ticks_TIM2);
					wheelFreq[0] = TIM2_freq;
				}
				TIM2_OVC = 0;
				TIM2_State = 1;
			}
			//state 1
			if (TIM2_State == 1){
				TIM2_rising2 = TIM2 -> CCR1;
				uint32_t ticks_TIM2 = (TIM2_rising2 + (TIM2_OVC * htim->Init.Period)) - TIM2_rising;
				if(ticks_TIM2 != 0 && TIM2_OVC < 2){
					TIM2_freq = (uint32_t)(96000000UL/ticks_TIM2);
					wheelFreq[0] = TIM2_freq;
				}
				TIM2_OVC = 0;
				TIM2_State = 0;
			}
		}
	}

	//timer 3 input
	if(htim -> Instance == TIM3){
		//channel 1
		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			//state 0
			if(TIM3_CH1_State == 0){
				TIM3_CH1_rising = TIM3 -> CCR1;
				uint32_t ticks_TIM3_CH1 = (TIM3_CH1_rising + (TIM3_CH1_OVC * htim -> Init.Period)) - TIM3_CH1_rising2;
				if(ticks_TIM3_CH1 != 0 && TIM3_CH1_OVC < 2){
					TIM3_CH1_freq = (uint32_t)(96000000UL/ticks_TIM3_CH1);
					wheelFreq[1] = TIM3_CH1_freq;
				}
				TIM3_CH1_OVC = 0;
				TIM3_CH1_State = 0;
			}
			//state 1
			if(TIM3_CH1_State == 1){
				TIM3_CH1_rising2 = TIM3 -> CCR1;
				uint32_t ticks_TIM3_CH1 = (TIM3_CH1_rising2 + (TIM3_CH1_OVC * htim -> Init.Period)) - TIM3_CH1_rising;
				if(ticks_TIM3_CH1 != 0 && TIM3_CH1_OVC < 2){
					TIM3_CH1_freq = (uint32_t)(96000000UL/ticks_TIM3_CH1);
					wheelFreq[1] = TIM3_CH1_freq;
				}
				TIM3_CH1_OVC = 0;
				TIM3_CH1_State = 0;
			}
		}
	//channel 2
		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_2){
			//state 0
			if(TIM3_CH2_State == 0){
				TIM3_CH2_rising = TIM3 -> CCR1;
				uint32_t ticks_TIM3_CH2 = (TIM3_CH2_rising + (TIM3_CH2_OVC * htim -> Init.Period)) - TIM3_CH2_rising2;
				if(ticks_TIM3_CH2 != 0 && TIM3_CH2_OVC < 2){
					TIM3_CH2_freq = (uint32_t)(96000000UL/ticks_TIM3_CH2);
					wheelFreq[2] = TIM3_CH2_freq;
				}
				TIM3_CH2_OVC = 0;
				TIM3_CH2_State = 0;
			}
			//state 1
			if(TIM3_CH2_State == 1){
				TIM3_CH2_rising2 = TIM3 -> CCR1;
				uint32_t ticks_TIM3_CH2 = (TIM3_CH2_rising2 + (TIM3_CH2_OVC * htim -> Init.Period)) - TIM3_CH2_rising;
				if(ticks_TIM3_CH2 != 0 && TIM3_CH2_OVC < 2){
					TIM3_CH2_freq = (uint32_t)(96000000UL/ticks_TIM3_CH2);
					wheelFreq[2] = TIM3_CH2_freq;
				}
				TIM3_CH2_OVC = 0;
				TIM3_CH2_State = 1;
			}
		}
	}
	//timer 4 input
	if(htim -> Instance == TIM4){
		if (htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			//state 0
			if(TIM4_State == 0){
				TIM4_rising = TIM4 -> CCR1;
				uint32_t ticks_TIM4 = (TIM4_rising + (TIM4_OVC * htim -> Init.Period) - TIM4_rising2);
				if(ticks_TIM4 != 0 && TIM4_OVC <2){
					TIM4_freq = (uint32_t)(96000000UL/ticks_TIM4);
					wheelFreq[3] = TIM4_freq;
				}
				TIM4_OVC = 0;
				TIM4_State = 1;
			}
			//state 1
			if(TIM4_State == 1){
				TIM4_rising2 = TIM4 -> CCR1;
				uint32_t ticks_TIM4 = (TIM4_rising2 + (TIM4_OVC * htim -> Init.Period) - TIM4_rising);
				if(ticks_TIM4 != 0 && TIM4_OVC <2){
					TIM4_freq = (uint32_t)(96000000UL/ticks_TIM4);
					wheelFreq[3] = TIM4_freq;
				}
				TIM4_OVC = 0;
				TIM4_State = 0;
			}
		}
	}

}

/*
 * Convert frequencies to rpm
 * 		run for loop for 4 iterations to parse through each wheel
 * 		multiply frequency by 60, divide by number of teeth on hub
 */

void RPMConversion(){
	for(int i = 0; i < 4; i++){
		wheelFreq[i];

		wheelRPM[i] = (wheelFreq[i] * 60)/ numTeeth;
	}
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
		ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:pumpCtrl\n");
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

void LEDCtrl() {

}

