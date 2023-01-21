/*
 * control.c
 *
 *  Created on: Jan 20, 2023
 *      Author: Matt
 *
 */

#include "control.h"
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
