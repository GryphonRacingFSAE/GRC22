#ifndef INC_THERMISTORMONITOR_H_
#define INC_THERMISTORMONITOR_H_

#include "cmsis_os2.h"

// 2ms between measurements
#define THERMISTOR_PERIOD 500

#define DEV_BOARD 0

#if DEV_BOARD == 0
#define MUX_COUNT 4
#define THERMISTORS_PER_MUX 8
#else
#define MUX_COUNT 1
#define THERMISTORS_PER_MUX 2
#endif

#define THERMISTOR_COUNT (THERMISTORS_PER_MUX * MUX_COUNT)

typedef struct {
	int32_t thermistors[MUX_COUNT][THERMISTORS_PER_MUX];
} ThermistorData_Struct;

extern ThermistorData_Struct ThermistorData;

void ADC_Select_MUX(int channel);

void startThermistorMonitorTask();

#endif /* INC_THERMISTORMONITOR_H_ */
