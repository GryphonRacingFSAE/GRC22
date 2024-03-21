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
// Turn off flag when tractive voltage < 20
#define RTD_TRACTIVE_VOLTAGE_OFF 200
// Turn off flag when tractive voltage > 450
#define RTD_TRACTIVE_VOLTAGE_ON 4500

// FLags
#define CTRL_RTD_INVALID 0x1
#define RTD_BUTTON 0x2
#define BRAKE_SWITCH 0x4
#define PUMP_ACTIVE 0x8
#define ACCUMULATOR_FAN_ACTIVE 0x10
#define RADIATOR_FAN_ACTIVE 0x20

typedef struct {
	uint32_t wheelSpeed[4];
	int32_t motor_controller_temp; // 10:1 conversion
	int32_t accumulator_max_temp; // 10:1 conversion?
	int32_t coolant_temp; // 10:1 conversion
	int32_t tractive_voltage; // 10:1 conversion
	int32_t motor_speed; // 1:1
	uint32_t flags;
} Ctrl_Data_Struct;

extern Ctrl_Data_Struct Ctrl_Data;

void startControlTask();

void OverflowCheck(TIM_HandleTypeDef * htim);
void RPMConversion(); //frequency to RPM conversion for wheel speed sensors
void RTD(); // Ready to drive
void pumpCtrl(); // Motor & Motor controller cooling pump control
void fanCtrl(); // Accumulator cooling fan control
void LEDCtrl(); // Info

#endif /* INC_CONTROL_H_ */
