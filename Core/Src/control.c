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

Ctrl_Data_Struct Ctrl_Data;

Ctrl_Data_Struct localCtrlData;

void startControlTask() {
	while (1) {
		// Use mutex to grab latest data - Determine if timeout should be changed to 0 due to later osDelay
		if (osMutexAcquire(Ctrl_Data_MtxHandle, CTRL_PERIOD) == osOK) {
			memcpy(&localCtrlData, &Ctrl_Data, sizeof(Ctrl_Data_Struct)); // Copy over latest data
			osMutexRelease(Ctrl_Data_MtxHandle); // Release the acquired mutex
		}

		BSPC();
		RTD();
		pumpCtrl();
		fanCtrl();

		osDelay(CTRL_PERIOD);
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
		myprintf("Missed osMutexAcquire(APPS_Data_MtxHandle): control.c:BSPC\n");
	}
}

// Ready to drive
void RTD() {

}

// Motor & Motor controller cooling pump control
void pumpCtrl() {

}

// Accumulator cooling fan control
void fanCtrl() {

}
