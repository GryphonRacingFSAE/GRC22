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
#include "utils.h"
#include "main.h"
#include <string.h>
#define NUM_WHEELS 4
// Turn on Pump if motor controller > 40c
#define PUMP_MOTOR_CONTROLLER_TEMP_THRESHOLD 400
// Turn on Pump if tractive voltage > 450v
#define PUMP_TRACTIVE_VOLTAGE_THRESHOLD 4500
// Turn on Fan if coolant temp > 40c
#define RAD_FAN_COOLANT_TEMP_THRESHOLD 400
// Turn on Accumulator fan if accumulator temp > 40c
#define ACC_FAN_ACC_TEMP_THRESHOLD 400
// Turn off flag when tractive voltage < 20
#define RTD_TRACTIVE_VOLTAGE_OFF 200
// Turn off flag when tractive voltage > 450
#define RTD_TRACTIVE_VOLTAGE_ON 4500
//number of teeth on wheel hub
#define NUM_TEETH 32;

typedef struct {
	uint32_t wheelSpeed[4];
	int32_t motorControllerTemp; // 10:1 conversion
	int32_t accumulatorMaxTemp; // 10:1 conversion?
	int32_t coolantTemp; // 10:1 conversion
	int32_t tractiveVoltage; // 10:1 conversion
	int32_t motorSpeed; // 1:1
	volatile uint32_t wheel_rpm[NUM_WHEELS];  // Array to store final wheel RPMs
	volatile uint32_t wheel_freq[NUM_WHEELS];  // Array to store final wheel frequencies
} Ctrl_Data_Struct;

extern Ctrl_Data_Struct Ctrl_Data;
extern osMutexId_t Ctrl_Data_MtxHandle;


void startControlTask();

void calculateWheelSpeedFrequency(uint8_t wheel_index);
void OverflowCheck(TIM_HandleTypeDef * htim);
void RPMConversion(); //frequency to RPM conversion for wheel speed sensors
void BSPC(); // Brake system plausibility check
void RTD(); // Ready to drive
void pumpCtrl(); // Motor & Motor controller cooling pump control
void fanCtrl(); // Accumulator cooling fan control
void LEDCtrl(); // Info

#endif /* INC_CONTROL_H_ */
