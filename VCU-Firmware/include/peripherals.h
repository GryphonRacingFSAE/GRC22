#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <stdint.h>

#define CONTROL_TASK_PERIOD 5

#define APPS1_MIN 1430
#define APPS1_MAX 1180
#define APPS2_MIN 3088
#define APPS2_MAX 2579
#define BRAKE_PRESSURE_MIN 290
#define BRAKE_PRESSURE_MAX (4095 * 9 / 10)
#define ADC_SHORTED_GND 150
#define ADC_SHORTED_VCC 3950

void imdFallingEdgeTime(void);
void imdRisingEdgeTime(void);
void flowSensFrequency(void);
void pumpCycle(uint8_t pump_speed);
void startPeripheralTask(void* pvParameters);
void startControlTask(void* pvParameters);

#endif // PERIPHERALS_H