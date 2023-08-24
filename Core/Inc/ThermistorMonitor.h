#ifndef INC_THERMISTORMONITOR_H_
#define INC_THERMISTORMONITOR_H_

#include "cmsis_os2.h"

// 2ms between measurements
#define THERMISTOR_PERIOD 500
#define CHANNEL_COUNT 4
#define THERMISTORS_PER_CHANNEL 7
#define THERMISTOR_COUNT (THERMISTORS_PER_CHANNEL * CHANNEL_COUNT)

typedef struct {
	int32_t thermistors[THERMISTOR_COUNT];
} ThermistorData_Struct;

extern ThermistorData_Struct ThermistorData;

void startThermistorMonitorTask();

#endif /* INC_THERMISTORMONITOR_H_ */
