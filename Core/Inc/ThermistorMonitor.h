#ifndef INC_THERMISTORMONITOR_H_
#define INC_THERMISTORMONITOR_H_

#include "cmsis_os2.h"

// 2ms between measurements
#define THERMISTOR_PERIOD 50

#define DEV_BOARD 0

#if DEV_BOARD == 0
#define MUX_COUNT 4
#define MULTIPLEXER_SIZE 16
#define THERMISTORS_PER_MUX 14

// This has to be done because Chris and Zohair didn't listen/forgot logic.
static const uint8_t UNPOPULATED_THERMISTORS[MUX_COUNT][2] = {
	{8, 15},
	{6, 7},
	{0, 1},
	{0, 1},
};

#else
#define MUX_COUNT 1
#define THERMISTORS_PER_MUX 2
#endif

#define THERMISTOR_COUNT (THERMISTORS_PER_MUX * MUX_COUNT)

// Thermistor data is stored in a 2D array formatted as such:
// [Multiplexer ID (from ADC channel; e.g.: 0, 1, 2, etc)] ->
// [Thermistor of Multiplexer (based on select line of MUX; e.g: 0-7 for 8 channel MUX and 0-15 for 6 channel MUX)]
typedef struct {
    int8_t thermistors[THERMISTOR_COUNT];
} ThermistorData_Struct;



extern ThermistorData_Struct ThermistorData;

void ADC_Select_MUX(int channel);

void startThermistorMonitorTask();

#endif /* INC_THERMISTORMONITOR_H_ */
