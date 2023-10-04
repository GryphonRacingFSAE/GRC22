#ifndef INC_THERMISTORMONITOR_H_
#define INC_THERMISTORMONITOR_H_

#include "cmsis_os2.h"

// 2ms between measurements
#define THERMISTOR_PERIOD 500

#define DEV_BOARD 1

#if DEV_BOARD == 0
#define MODULE_COUNT 5
#define THERMISTORS_PER_MODULE 8
#else
#define MODULE_COUNT 1
#define THERMISTORS_PER_MODULE 2
#endif

#define THERMISTOR_COUNT (THERMISTORS_PER_MODULE * MODULE_COUNT)

typedef struct {
	int32_t thermistors[MODULE_COUNT][THERMISTORS_PER_MODULE];
} ThermistorData_Struct;

extern ThermistorData_Struct ThermistorData;

void startThermistorMonitorTask();

#endif /* INC_THERMISTORMONITOR_H_ */
