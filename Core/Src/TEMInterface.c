#include <CAN.h>
#include "TEMInterface.h"
#include "ThermistorMonitor.h"
#include "utils.h"
#include "main.h"

extern CAN_HandleTypeDef hcan2;

void startTEMInterfaceTask() {
	uint32_t tick = osKernelGetTickCount();

	const uint8_t length = sizeof(ThermistorData.thermistors)/sizeof(ThermistorData.thermistors[0]);

	uint8_t min;
	uint8_t max;
	uint8_t avg;

	CANTxMsg bmsBroadcast;
	bmsBroadcast.header.IDE = CAN_ID_STD;
	bmsBroadcast.header.RTR = CAN_RTR_DATA;
	bmsBroadcast.header.ExtId = 0x1839F380;
	bmsBroadcast.header.DLC = 8;
	bmsBroadcast.to = &hcan2;

	CANTxMsg generalBroadcast;
	bmsBroadcast.header.IDE = CAN_ID_STD;
	bmsBroadcast.header.RTR = CAN_RTR_DATA;
	bmsBroadcast.header.ExtId = 0x1838F380;
	bmsBroadcast.header.DLC = 8;
	bmsBroadcast.to = &hcan2;

	while (1) {
		// TODO: read switches and apply bitwise offset (why?)
		// HAL_GPIO_TogglePin(ADDR1_GPIO_Port, ADDR1_Pin);

		min = ThermistorData.thermistors[0];
		max = ThermistorData.thermistors[0];
		avg = ThermistorData.thermistors[0];

		// TODO: figure out how temp is stored in the uint32 (code below is probably wrong)
		for (int i = 1; i < length; i++) {
			// update minimum temperature
			if (ThermistorData.thermistors[i] < min) {
				min = ThermistorData.thermistors[i];
			}
			// update maximum temperature
			if (ThermistorData.thermistors[i] > max) {
				max = ThermistorData.thermistors[i];
			}
			// sum together all thermistor values
			avg += ThermistorData.thermistors[i];
		}

		// calculate average
		avg /= length;

		GRCprintf("Min: %d Max: %d Avg: %d\n", &min, &max, &avg);

		// TODO: figure out how to get thermistor id and module num from uint32

		// bmsBroadcast.data[0] = THERMISTOR_MODULE_NUMBER;
		bmsBroadcast.data[1] = min;
		bmsBroadcast.data[2] = max;
		bmsBroadcast.data[3] = avg;
		// bmsBroadcast.data[4] = NUMBER_OF_THERMISTORS_ENABLED;
		// bmsBroadcast.data[5] = HIGHEST_THERMISTOR_ID;
		// bmsBroadcast.data[6] = LOWEST_THERMISTOR_ID;
		// bmsBroadcast.data[7] = CHECKSUM;

		// generalBroadcast.data[0] = RELATIVE_THERMISTOR_ID;
		// generalBroadcast.data[1] = RELATIVE_THERMISTOR_ID;
		// generalBroadcast.data[2] = THERMISTOR_VALUE;
		// generalBroadcast.data[3] = NUMBER_OF_THERMISTORS_ENABLED;
		generalBroadcast.data[4] = min;
		generalBroadcast.data[5] = max;
		// generalBroadcast.data[6] = HIGHEST_THERMISTOR_ID;
		// generalBroadcast.data[7] = LOWEST_THERMISTOR_ID;

		osMessageQueuePut(CANTX_QHandle, &bmsBroadcast, 0, 5);
		osMessageQueuePut(CANTX_QHandle, &generalBroadcast, 0, 5);

		osDelayUntil(tick += TEM_PERIOD);
	}
}
