#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <stdint.h>

#define CONTROL_TASK_PERIOD 5

#define APPS1_MIN 1340
#define APPS1_MAX 1160
#define APPS2_MIN 2915
#define APPS2_MAX 2520
#define BRAKE_PRESSURE_MIN 290
#define BRAKE_PRESSURE_MAX (4095 * 9 / 10)
#define ADC_SHORTED_GND 150
#define ADC_SHORTED_VCC 3950

void pumpCycle(uint8_t pump_speed);
void startPeripheralTask(void* pvParameters);
void startControlTask(void *pvParameters);

#endif // PERIPHERALS_H