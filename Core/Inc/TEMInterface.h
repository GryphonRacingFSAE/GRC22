#ifndef INC_TEMINTERFACE_H_
#define INC_TEMINTERFACE_H_

#include <stdint.h>

#define TEM_PERIOD 100

#define MAX_MODULE_COUNT 8

// This is to avoid including cold solders in our temperatures
#define NEGATIVE_TEMPERATURE_CUTOFF (-10)

// Simply change the bit# of the thermistor ID to 1 to ignore that thermistor.
static const uint64_t IGNORED_THERMISTORS[MAX_MODULE_COUNT] = {
	0x0000000000000000,
	0x0000000001000000,
	0x0000000000000000,
	0x0000000000000000,
	0x0000000000000000,
	0x0000000000000000,
	0x0000000000000000,
	0x0000000000000000,
};

void startTEMInterfaceTask();

uint8_t isThermistorIgnored(uint8_t module_number, uint8_t thermistor_id);

#endif /* INC_TEMINTERFACE_H_ */
