#ifndef INC_THERMISTORMONITOR_H_
#define INC_THERMISTORMONITOR_H_

#include "cmsis_os2.h"

// 2ms between measurements
#define THERMISTOR_PERIOD 500

typedef struct {
	int32_t thermistors[4 * 8];
} ThermistorData_Struct;

extern ThermistorData_Struct ThermistorData;

void startThermistorMonitorTask();

#endif /* INC_THERMISTORMONITOR_H_ */
