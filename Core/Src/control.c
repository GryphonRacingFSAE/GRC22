/*
 * control.c
 *
 *  Created on: Jan 20, 2023
 *      Author: Matt
 *
 */

#include "control.h"
#include "APPS.h"
#include "utils.h"
#include "main.h"
#include <string.h>


volatile uint16_t gu16_TIM2_OVC = 0;							//timer 2 overflow counter
volatile uint16_t gu16_TIM3_OVC = 0;							//timer 3 overflow counter
volatile uint16_t gu16_TIM4_OVC = 0;							//timer 4 overflow counter
volatile uint8_t gu8_TIM2_State = 0;
volatile uint8_t gu8_TIM3_CH1_State = 0;
volatile uint8_t gu8_TIM3_CH2_State = 0;
volatile uint8_t gu8_TIM4_State = 0;
volatile uint8_t gu8_MSG[35] = {'\0'};
volatile uint32_t gu32_T1 = 0;									//rising edge value
volatile uint32_t gu32_T2 = 0;									//falling edge value
volatile uint32_t gu32_Freq = 0;								//wheel speed frequency
volatile uint32_t wheelFreq[3]; 								//wheel speed sensor frequency array
volatile uint32_t wheelRPM[3];									//wheel rpm array

uint16_t numTeeth = 32;											//number of teeth on wheel hub

Ctrl_Data_Struct Ctrl_Data;

void startControlTask() {
	uint32_t tick = osKernelGetTickCount();
	while (1) {
		RPMconversion();
//		ERROR_PRINT("Time thing: %d\n", TIM2->CCR1);
		for (int i = 0; i < 4; i++){
    	GRCprintf("Frequency %d = %d Hz\r\n", (i+1), wheelFreq[i]);
    	GRCprintf("RPM %d = %d rpm\r\n", (i+1), wheelRPM[i]);
		}
		BSPC();
		RTD();
		pumpCtrl();
		fanCtrl();
		LEDCtrl();
		osDelayUntil(tick += CTRL_PERIOD);
	}
}

/*
 * runs checks for period overflow
 * 	if the instance is equal to TIM1, hal tick is incremented in order to keep all timing accurate and synchronized
 *
 * 	if instance is equal to TIM3, need to verify which channel since 2 are being used
 * 		Increase overflow counter by one for whatever channel has overflowed
 * 			if the overflow count is greater than 2, set frequency to 0
 *
 * 	if instance is equal to TIM2
 * 		Increase overflow counter by one
 * 			if the overflow count is greater than 2, set frequency to 0
 *
 * 	if instance is equal to TIM4
 * 		Increase overflow counter by one
 * 			if the overflow count is greater than 2, set frequency to 0
 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }

  if(htim -> Instance == TIM2){
	  gu16_TIM2_OVC++;
	  if (gu16_TIM2_OVC >= 2){
		  gu32_Freq = 0;
	  }
  }

  /* USER CODE BEGIN Callback 1 */
  else if (htim->Instance == TIM3) {
	  if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){
		  gu16_TIM3_OVC++;
		  if (gu16_TIM3_OVC >= 2) {
			  gu32_Freq = 0;
		  }
	  }
	  else if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_2){
		  gu16_TIM3_OVC++;
		  if (gu16_TIM3_OVC >= 2) {
			  gu32_Freq = 0;
		  }
	  }
  }

  else if(htim -> Instance == TIM4){
	  gu16_TIM4_OVC++;
	  if (gu16_TIM4_OVC >= 2){
		  gu32_Freq = 0;
	  }
  }


  /* USER CODE END Callback 1 */
}

/*
 * Called when an input capture event occurs
 * Check to see which timer the instance occurs at (TIM2, TIM3, TIM4)
 * using if statements to determine which channel is causing the interrupt (subject to change for more efficient method)
 * If CH1, CH2, CH3 or CH4 (same algorithm applies to all):
 * 		If gu8_state is in state 0
 * 			Captures the timer counter value at rising edge of input signal
 * 			Calculates time interval between rising and falling edge
 * 			if ticks is not zero and overflow counter is less than 2, frequency is calculated
 * 			frequency is stored within corresponding index of the array
 * 			overflow is reset, state is changed to 1
 * 		If gu8_state is in state 1
 * 			Captures the timer counter value at falling edge of input signal
 * 			Calculates time interval between falling and rising edge
 * 			if ticks is not zero and overflow counter is less than 2, frequency is calculated
 * 			frequency is stored within corresponding index of the array
 * 			overflow is reset, state is changed to 0
 *
 * 	Channel to array correspondence:
 * 		TIM2_CH1 = 0
 * 		TIM3_CH1 = 1
 * 		TIM3_CH2 = 2
 * 		TIM4_CH1 = 3
 */


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
	uint32_t gu32_T1 = 0;
	uint32_t gu32_Ticks = 0;
	uint32_t gu32_T2 = 0;
	if(htim -> Instance == TIM2){
		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			gu32_Freq = 0;
			if(gu8_TIM2_State == 0){
				gu32_T1 = TIM2->CCR1;
				gu32_Ticks = (gu32_T1 + (gu16_TIM2_OVC * htim->Init.Period )) - gu32_T2;
				if (gu32_Ticks != 0 && gu16_TIM2_OVC < 2) {
					gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
					wheelFreq[0] = gu32_Freq;
				}
				gu16_TIM2_OVC = 0;
				gu8_TIM2_State = 1;
			}
			else if(gu8_TIM2_State == 1)
			{
				gu32_T2 = TIM2->CCR1;
				gu32_Ticks = (gu32_T2 + (gu16_TIM2_OVC * htim->Init.Period )) - gu32_T1;
				if (gu32_Ticks != 0 && gu16_TIM2_OVC < 2) {
					gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
					wheelFreq[0] = gu32_Freq;
				}
				gu16_TIM2_OVC = 0;
				gu8_TIM2_State = 0;
			}
		}
	}

	if(htim -> Instance == TIM3){
		gu32_Freq = 0;

		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			if(gu8_TIM3_CH1_State == 0)
			{
				gu32_T1 = TIM3->CCR1;
				gu32_Ticks = (gu32_T1 + (gu16_TIM3_OVC * htim->Init.Period )) - gu32_T2;
				if (gu32_Ticks != 0 && gu16_TIM3_OVC < 2) {
					gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
					wheelFreq[1] = gu32_Freq;
				}
				gu16_TIM3_OVC = 0;
				gu8_TIM3_CH1_State = 1;
			}
			else if(gu8_TIM3_CH1_State == 1)
			{
				gu32_T2 = TIM3->CCR1;
				gu32_Ticks = (gu32_T2 + (gu16_TIM3_OVC * htim->Init.Period )) - gu32_T1;
				if (gu32_Ticks != 0 && gu16_TIM3_OVC < 2) {
					gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
					wheelFreq[1] = gu32_Freq;
				}
				gu16_TIM3_OVC = 0;
				gu8_TIM3_CH1_State = 0;
			}
		}

		else if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_2){
			if(gu8_TIM3_CH2_State == 0)
			{
				gu32_T1 = TIM3->CCR1;
				gu32_Ticks = (gu32_T1 + (gu16_TIM3_OVC * htim->Init.Period )) - gu32_T2;
					if (gu32_Ticks != 0 && gu16_TIM3_OVC < 2) {
						gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
						wheelFreq[2] = gu32_Freq;
					}
				gu16_TIM3_OVC = 0;
				gu8_TIM3_CH2_State = 1;
			}
			else if(gu8_TIM3_CH2_State == 1)
			{
				gu32_T2 = TIM3->CCR1;
				gu32_Ticks = (gu32_T2 + (gu16_TIM3_OVC * htim->Init.Period )) - gu32_T1;
					if (gu32_Ticks != 0 && gu16_TIM3_OVC < 2) {
						gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
						wheelFreq[2] = gu32_Freq;
					}
				gu16_TIM3_OVC = 0;
				gu8_TIM3_CH2_State = 0;
			}
		}

	}

	if(htim -> Instance == TIM4){
		if(htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1){

			gu32_Freq = 0;
			if(gu8_TIM4_State == 0){
				gu32_T1 = TIM4->CCR1;
				gu32_Ticks = (gu32_T1 + (gu16_TIM4_OVC * htim->Init.Period )) - gu32_T2;
				if (gu32_Ticks != 0 && gu16_TIM4_OVC < 2) {
					gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
					wheelFreq[3] = gu32_Freq;
				}
				gu16_TIM4_OVC = 0;
				gu8_TIM4_State = 1;
			}
			else if(gu8_TIM4_State == 1)
			{
				gu32_T2 = TIM4->CCR1;
				gu32_Ticks = (gu32_T2 + (gu16_TIM4_OVC * htim->Init.Period )) - gu32_T1;
				if (gu32_Ticks != 0 && gu16_TIM4_OVC < 2) {
					gu32_Freq = (uint32_t)(96000000UL/gu32_Ticks);
					wheelFreq[3] = gu32_Freq;
				}
				gu16_TIM4_OVC = 0;
				gu8_TIM4_State = 0;
			}
		}
	}
}

/*
 * Convert frequencies to rpm
 * 	run for loop up to 4 cycles to parse through each array index
 * 	multiply frequency by 60, divide by number of teeth (currently 32)
 */
void RPMconversion(){
	uint32_t tempFreq = 0;
	for(int i = 0; i < 4; i++){
		tempFreq = wheelFreq[i];

		wheelRPM[i] = (tempFreq * 60)/numTeeth;
	}
}




// Brake system plausibility check
void BSPC() {
	if (osMutexAcquire(APPS_Data_MtxHandle, 5) == osOK){
		// If the BSPC is in the invalid state,
		if (APPS_Data.flags & APPS_BSPC_INVALID){
			// Check if the pedal position is <5% to put APPS back into a valid state (EV.5.7.2)
			if (APPS_Data.pedalPos < 5) {
				APPS_Data.flags &= ~APPS_BSPC_INVALID; // Remove the invalid flag
			}
		}
		// Only try to put the APPS into an invalid state if it's valid regarding the BSPC
		// Set to invalid if over >25% travel and brakes engaged (EV.5.7.1)
		else if (APPS_Data.pedalPos > 25 && HAL_GPIO_ReadPin(GPIO_BRAKE_SW_GPIO_Port, GPIO_BRAKE_SW_Pin)) {
			APPS_Data.flags |= APPS_BSPC_INVALID; // Consider APPS as invalid due to BSPC
		}
		osMutexRelease(APPS_Data_MtxHandle);
	} else {
		ERROR_PRINT("Missed osMutexAcquire(APPS_Data_MtxHandle): control.c:BSPC\n");
	}
}

// Ready to drive
// Rules: EV.10.4.3 & EV.10.5
void RTD() {
	static int callCounts = 0;

	if (callCounts * CTRL_PERIOD > 2000) {
		HAL_GPIO_WritePin(GPIO_RTD_BUZZER_GPIO_Port, GPIO_RTD_BUZZER_Pin, 0);
	}
	if(osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
		if (osMutexAcquire(APPS_Data_MtxHandle, 5) == osOK){
			// If the RTD is in the invalid state,
			if (APPS_Data.flags & APPS_RTD_INVALID){
				// Check if the pedal position is <3% to put APPS back into a valid state (EV.10.4.3)				
				if (APPS_Data.pedalPos < 3 && HAL_GPIO_ReadPin(GPIO_BRAKE_SW_GPIO_Port, GPIO_BRAKE_SW_Pin) && Ctrl_Data.tractiveVoltage > RTD_TRACTIVE_VOLTAGE_ON && HAL_GPIO_ReadPin(GPIO_START_BTN_GPIO_Port, GPIO_START_BTN_Pin)) {
					APPS_Data.flags &= ~APPS_RTD_INVALID; // Remove the invalid flag
					HAL_GPIO_WritePin(GPIO_RTD_BUZZER_GPIO_Port, GPIO_RTD_BUZZER_Pin, 1);
					callCounts = 0;
				}				
			} 
			else if (Ctrl_Data.tractiveVoltage < RTD_TRACTIVE_VOLTAGE_OFF) {
				APPS_Data.flags |= APPS_RTD_INVALID; // Consider APPS as invalid due to RTD
			}
			osMutexRelease(APPS_Data_MtxHandle);
		} else {
			ERROR_PRINT("Missed osMutexAcquire(APPS_Data_MtxHandle): control.c:RTD\n");
		}
		osMutexRelease(Ctrl_Data_MtxHandle);
	} else {
		ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:RTD\n");
	}
	callCounts++;
}

// Motor & Motor controller cooling pump control
void pumpCtrl() {
	if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK){
		// Turn on pump based on motor controller temperature threshold and tractive voltage threshold
		HAL_GPIO_WritePin(GPIO_PUMP_GPIO_Port, GPIO_PUMP_Pin,
				Ctrl_Data.motorControllerTemp > PUMP_MOTOR_CONTROLLER_TEMP_THRESHOLD || Ctrl_Data.tractiveVoltage > PUMP_TRACTIVE_VOLTAGE_THRESHOLD);
		osMutexRelease(Ctrl_Data_MtxHandle);
	} else {
		HAL_GPIO_WritePin(GPIO_PUMP_GPIO_Port, GPIO_PUMP_Pin, GPIO_PIN_SET); // Turn on pump if cannot acquire mutex
		ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:pumpCtrl\n");
	}
}

// Motor controller cooling fan control
void fanCtrl() {
	if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK){
		// Turn on fan based on coolant temperature threshold
		HAL_GPIO_WritePin(GPIO_RAD_FAN_GPIO_Port, GPIO_RAD_FAN_Pin,
				Ctrl_Data.coolantTemp > RAD_FAN_COOLANT_TEMP_THRESHOLD);
		HAL_GPIO_WritePin(GPIO_ACC_FAN_GPIO_Port, GPIO_ACC_FAN_Pin,
				Ctrl_Data.accumulatorMaxTemp > ACC_FAN_ACC_TEMP_THRESHOLD);
		osMutexRelease(Ctrl_Data_MtxHandle);
	} else {
		// Turn on fans if cannot acquire mutex
		HAL_GPIO_WritePin(GPIO_RAD_FAN_GPIO_Port, GPIO_RAD_FAN_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_ACC_FAN_GPIO_Port, GPIO_ACC_FAN_Pin, GPIO_PIN_SET);
		ERROR_PRINT("Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:fanCtrl\n");
	}
}

void LEDCtrl() {

}
