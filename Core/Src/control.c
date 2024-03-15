/*
 * control.c
 *
 * Created: Jan 20, 2023
 * Authors: Matt, Dallas, Riyan, Ronak
 *
 */

#include "control.h"
#include "APPS.h"




static volatile uint32_t tim_ovc[NUM_WHEELS] = {0};        // Array to store overflow counters for each wheel

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

Ctrl_Data_Struct Ctrl_Data;


void startControlTask(){
    uint32_t tick = osKernelGetTickCount();
    while(1){
        for(int i = 0; i < 4; i++){
            GRCprintf("Frequency %d= %d\n\r", (i+1), Ctrl_Data.wheel_freq[i]);
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




const TIM_HandleTypeDef* wheel_to_timer_mapping[NUM_WHEELS] = {&htim2, &htim3, &htim3, &htim4}; // Note: TIM3 is repeated for both channels CH1 and CH2

// Function to manage overflow conditions for each wheel's timer
void ManageTimerOverflow(TIM_HandleTypeDef * htim) {
    // Loop through each wheel
    for (int i = 0; i < NUM_WHEELS; i++) {
        // Check if the instance of the timer handler matches the current wheel's timer handler
    	if (htim == wheel_to_timer_mapping[i]) {
            // Increment overflow counter for the current wheel
            tim_ovc[i]++;
            // Reset frequency to 0 if overflow counter exceeds 2
            if (tim_ovc[i] >= 2) {
            	Ctrl_Data.wheel_freq[i] = 0;
            }
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


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef* htim) {

	static volatile uint32_t tim_rising[NUM_WHEELS][NUM_WHEELS] = {{0}};  // 2D array to store rising edge times for each wheel
	static volatile uint8_t tim_state[NUM_WHEELS] = {0};       // Array to store state (0 or 1) for each wheel

	// Loop through each wheel
    for (int i = 0; i < NUM_WHEELS; i++) {
        // Check if the instance of the timer handler matches the current wheel's timer handler
        if (htim == wheel_to_timer_mapping[i]) {
            // Store timer value within rising edge variable
            tim_rising[i][tim_state[i]] = htim->Instance->CCR1; // Changed tim_rising2 to tim_rising

            // Calculate time interval between two rising edges at the same sensor
            uint32_t ticks_TIM = (tim_rising[i][1] + (tim_ovc[i] * htim->Init.Period)) - tim_rising[i][0];
            // Calculate frequency and RPM if ticks is not zero and overflow counter is less than 2
            if (ticks_TIM != 0 && tim_ovc[i] < 2) {
                // Calculate frequency
            	Ctrl_Data.wheel_freq[i] = (uint32_t)(96000000UL / ticks_TIM);
                // Calculate RPM and store in Ctrl_Data.wheel_rpm array
                Ctrl_Data.wheel_rpm[i] = (Ctrl_Data.wheel_freq[i] * 60) / numTeeth; // assuming numTeeth is defined elsewhere
            }
            // Reset overflow counter
            tim_ovc[i] = 0;
            // Toggle state (0 to 1, or 1 to 0)
            tim_state[i] = !tim_state[i];
        }
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
