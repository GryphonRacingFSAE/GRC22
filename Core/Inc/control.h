/*
 * control.h
 *
 *  Created on: Jan 20, 2023
 *      Author: Matt
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include "main.h"
#include "cmsis_os.h"

typedef struct {
	uint32_t wheelSpeed[4];
	int32_t motorControllerTemp;
	int32_t coolantTemp;
	uint32_t tractiveVoltage;
} Ctrl_Data_Struct;

extern Ctrl_Data_Struct Ctrl_Data;
extern osMutexId_t Ctrl_Data_MtxHandle;

void startControlTask();

void BSPC(); // Brake system plausibility check
void RTD(); // Ready to drive
void pumpCtrl(); // Motor & Motor controller cooling pump control
void fanCtrl(); // Accumulator cooling fan control

#endif /* INC_CONTROL_H_ */
