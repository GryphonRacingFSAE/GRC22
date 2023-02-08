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

// Turn on Pump if motor controller > 40c
#define PUMP_MOTOR_CONTROLLER_TEMP_THRESHOLD 400
// Turn on Pump if tractive voltage > 450v
#define PUMP_TRACTIVE_VOLTAGE_THRESHOLD 4500
// Turn on Fan if coolant temp > 40c
#define RAD_FAN_COOLANT_TEMP_THRESHOLD 400
// Turn on Accumulator fan if accumulator temp > 40c
#define ACC_FAN_ACC_TEMP_THRESHOLD 400

typedef struct {
	uint32_t wheelSpeed[4];
	int32_t motorControllerTemp; // 10:1 conversion
	int32_t accumulatorMaxTemp; // 10:1 conversion?
	int32_t coolantTemp; // 10:1 conversion
	int32_t tractiveVoltage; // 10:1 conversion
} Ctrl_Data_Struct;

extern Ctrl_Data_Struct Ctrl_Data;
extern osMutexId_t Ctrl_Data_MtxHandle;

void startControlTask();

void BSPC(); // Brake system plausibility check
void RTD(); // Ready to drive
void pumpCtrl(); // Motor & Motor controller cooling pump control
void fanCtrl(); // Accumulator cooling fan control

#endif /* INC_CONTROL_H_ */
