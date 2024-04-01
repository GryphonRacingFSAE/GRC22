#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include "main.h"
#include "cmsis_os.h"
#include "utils.h"
#include "main.h"
#include "APPS.h"
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
// Number of teeth on wheel hub
#define NUM_TEETH 32;
#define PRESSURE_SENSOR_MIN 410
#define PRESSURE_SENSOR_MAX 3686
#define PRESSURE_RANGE 2.5

// Flags
#define CTRL_RTD_INVALID 0x1
#define RTD_BUTTON 0x2
#define BRAKE_SWITCH 0x4
#define PUMP_ACTIVE 0x8
#define ACCUMULATOR_FAN_ACTIVE 0x10
#define RADIATOR_FAN_ACTIVE 0x20

typedef struct {
	int32_t motor_controller_temp; // 10:1 conversion
	int32_t accumulator_max_temp; // 10:1 conversion?
	int32_t coolant_temp; // 10:1 conversion
	int32_t tractive_voltage; // 10:1 conversion
	int32_t motor_speed; // 1:1
	uint32_t flags;
	volatile uint32_t wheel_rpm[NUM_WHEELS];  // Array to store final wheel RPMs
	volatile uint32_t wheel_freq[NUM_WHEELS];  // Array to store final wheel frequencies
	uint16_t pressure_readings[2];  // Array to store Pressure values (in bar)
} Ctrl_Data_Struct;

extern Ctrl_Data_Struct Ctrl_Data;

void startControlTask();

void calculateWheelSpeedFrequency(uint8_t wheel_index, TIM_HandleTypeDef *htim);
void manageWheelSpeedTimerOverflow(const TIM_HandleTypeDef * htim);
void BSPC(); // Brake system plausibility check
void RTD(); // Ready to drive
void pumpCtrl(); // Motor & Motor controller cooling pump control
void pumpCycle(uint8_t pump_speed); //cooling pump cooling cycles
void fanCtrl(); // Accumulator cooling fan control
void LEDCtrl(); // Info
void pressureSensorConversions();
#endif /* INC_CONTROL_H_ */
