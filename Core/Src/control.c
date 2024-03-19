/*
 * control.c
 *
 * Created: Jan 20, 2023
 * Authors: Matt, Dallas, Riyan, Ronak
 *
 */

#include "control.h"
#include "APPS.h"
#include "main.h"
// Array to store overflow counters for each wheel
volatile uint32_t tim_ovc[NUM_WHEELS] = { 0 };



extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

// Array mapping each wheel to its corresponding timer
const TIM_HandleTypeDef* wheel_to_timer_mapping[NUM_WHEELS] = {&htim2, &htim3, &htim4, &htim3 };

Ctrl_Data_Struct Ctrl_Data;

// Task to manage control operations
void startControlTask() {
	uint32_t tick = osKernelGetTickCount();
	while (1) {
		for (int i = 0; i < 4; i++) {
			GRCprintf("Frequency %d= %d\n\r", (i + 1), Ctrl_Data.wheel_freq[i]);
		}

		BSPC();
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




// Brake system plausibility check
void BSPC() {
	if (osMutexAcquire(APPS_Data_MtxHandle, 5) == osOK) {
		// If the BSPC is in the invalid state,
		if (APPS_Data.flags & APPS_BSPC_INVALID) {
			// Check if the pedal position is <5% to put APPS back into a valid state (EV.5.7.2)
			if (APPS_Data.pedalPos < 5) {
				APPS_Data.flags &= ~APPS_BSPC_INVALID; // Remove the invalid flag
			}
		}
		// Only try to put the APPS into an invalid state if it's valid regarding the BSPC
		// Set to invalid if over >25% travel and brakes engaged (EV.5.7.1)
		else if (APPS_Data.pedalPos > 25
				&& HAL_GPIO_ReadPin(GPIO_BRAKE_SW_GPIO_Port,
				GPIO_BRAKE_SW_Pin)) {
			APPS_Data.flags |= APPS_BSPC_INVALID; // Consider APPS as invalid due to BSPC
		}
		osMutexRelease(APPS_Data_MtxHandle);
	} else {
		ERROR_PRINT(
				"Missed osMutexAcquire(APPS_Data_MtxHandle): control.c:BSPC\n");
	}
}

// Ready to drive
// Rules: EV.10.4.3 & EV.10.5
void RTD() {
	static int callCounts = 0;

	if (callCounts * CTRL_PERIOD > 2000) {
		HAL_GPIO_WritePin(GPIO_RTD_BUZZER_GPIO_Port, GPIO_RTD_BUZZER_Pin, 0);
	}
	if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
		if (osMutexAcquire(APPS_Data_MtxHandle, 5) == osOK) {
			// If the RTD is in the invalid state,
			if (APPS_Data.flags & APPS_RTD_INVALID) {
				// Check if the pedal position is <3% to put APPS back into a valid state (EV.10.4.3)
				if (APPS_Data.pedalPos < 3
						&& HAL_GPIO_ReadPin(GPIO_BRAKE_SW_GPIO_Port,
						GPIO_BRAKE_SW_Pin)
						&& Ctrl_Data.tractiveVoltage > RTD_TRACTIVE_VOLTAGE_ON
						&& HAL_GPIO_ReadPin(GPIO_START_BTN_GPIO_Port,
						GPIO_START_BTN_Pin)) {
					APPS_Data.flags &= ~APPS_RTD_INVALID; // Remove the invalid flag
					HAL_GPIO_WritePin(GPIO_RTD_BUZZER_GPIO_Port,
					GPIO_RTD_BUZZER_Pin, 1);
					callCounts = 0;
				}
			} else if (Ctrl_Data.tractiveVoltage < RTD_TRACTIVE_VOLTAGE_OFF) {
				APPS_Data.flags |= APPS_RTD_INVALID; // Consider APPS as invalid due to RTD
			}
			osMutexRelease(APPS_Data_MtxHandle);
		} else {
			ERROR_PRINT(
					"Missed osMutexAcquire(APPS_Data_MtxHandle): control.c:RTD\n");
		}
		osMutexRelease(Ctrl_Data_MtxHandle);
	} else {
		ERROR_PRINT(
				"Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:RTD\n");
	}
	callCounts++;
}

// Motor & Motor controller cooling pump control
void pumpCtrl() {
	if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
		// Turn on pump based on motor controller temperature threshold and tractive voltage threshold
		HAL_GPIO_WritePin(GPIO_PUMP_GPIO_Port, GPIO_PUMP_Pin,
				Ctrl_Data.motorControllerTemp
						> PUMP_MOTOR_CONTROLLER_TEMP_THRESHOLD
						|| Ctrl_Data.tractiveVoltage
								> PUMP_TRACTIVE_VOLTAGE_THRESHOLD);
		osMutexRelease(Ctrl_Data_MtxHandle);
	} else {
		HAL_GPIO_WritePin(GPIO_PUMP_GPIO_Port, GPIO_PUMP_Pin, GPIO_PIN_SET); // Turn on pump if cannot acquire mutex
		ERROR_PRINT(
				"Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:pumpCtrl\n");
	}
}

// Motor controller cooling fan control
void fanCtrl() {
	if (osMutexAcquire(Ctrl_Data_MtxHandle, 5) == osOK) {
		// Turn on fan based on coolant temperature threshold
		HAL_GPIO_WritePin(GPIO_RAD_FAN_GPIO_Port, GPIO_RAD_FAN_Pin,
				Ctrl_Data.coolantTemp > RAD_FAN_COOLANT_TEMP_THRESHOLD);
		HAL_GPIO_WritePin(GPIO_ACC_FAN_GPIO_Port, GPIO_ACC_FAN_Pin,
				Ctrl_Data.accumulatorMaxTemp > ACC_FAN_ACC_TEMP_THRESHOLD);
		osMutexRelease(Ctrl_Data_MtxHandle);
	} else {
		// Turn on fans if cannot acquire mutex
		HAL_GPIO_WritePin(GPIO_RAD_FAN_GPIO_Port, GPIO_RAD_FAN_Pin,
				GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIO_ACC_FAN_GPIO_Port, GPIO_ACC_FAN_Pin,
				GPIO_PIN_SET);
		ERROR_PRINT(
				"Missed osMutexAcquire(Ctrl_Data_MtxHandle): control.c:fanCtrl\n");
	}
}

void LEDCtrl() {

}
