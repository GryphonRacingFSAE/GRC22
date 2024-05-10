#include "TEMInterface.h"
#include "ThermistorMonitor.h"
#include "main.h"
#include "utils.h"
#include <CAN.h>


extern CAN_HandleTypeDef hcan;

// Please see CANBUS Protocol from Orion's TEM: https://www.orionbms.com/products/thermistor-expansion-module/
CANTXMsg bms_broadcast = {.header = {.IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA, .ExtId = 0x1839F380, .DLC = 8}, .to = &hcan, .data = {}};
CANTXMsg general_broadcast = {.header = {.IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA, .ExtId = 0x1838F380, .DLC = 8}, .to = &hcan, .data = {}};

void startTEMInterfaceTask() {
    uint32_t tick = osKernelGetTickCount();

    uint8_t current_thermistor_id = 0;


    while (1) {
        // Grab the current module ID.
        // Index 3 on the dip switch is LSB, index 1 on the dip switch is MSB
    	uint8_t module_bit_0 = HAL_GPIO_ReadPin(DIP0_GPIO_Port, DIP0_Pin);
		uint8_t module_bit_1 = HAL_GPIO_ReadPin(DIP1_GPIO_Port, DIP1_Pin) << 1;
		uint8_t module_bit_2 = HAL_GPIO_ReadPin(DIP2_GPIO_Port, DIP2_Pin) << 2;
		uint8_t module_number = (module_bit_2 | module_bit_1 | module_bit_0);
    	DEBUG_PRINT("Module #%d\n", (int)module_number);

    	int8_t minimum_thermistor_temperature = 127;
    	int8_t maximum_thermistor_temperature = -128;
    	int16_t thermistor_temperature_sum = 0;
    	uint8_t actual_thermistor_count = 0;
    	for (uint8_t i = 0; i < THERMISTOR_COUNT; i++) {
    		uint8_t excluded_thermistors = (module_number == 1 && i == 25);
    		if (ThermistorData.thermistors[i] > -35 && !excluded_thermistors) {
    			actual_thermistor_count++;
				if (ThermistorData.thermistors[i] < minimum_thermistor_temperature) {
					minimum_thermistor_temperature = ThermistorData.thermistors[i];
				}
				if (ThermistorData.thermistors[i] > maximum_thermistor_temperature) {
					maximum_thermistor_temperature = ThermistorData.thermistors[i];
				}
				thermistor_temperature_sum += ThermistorData.thermistors[i];
    		}
    	}
    	if (actual_thermistor_count == 0) {
    		actual_thermistor_count = 1;
    	}
    	int8_t average_thermistor_temperature = (int8_t)(thermistor_temperature_sum / actual_thermistor_count);

    	// The TEM modules don't actually seem to offset their module numbers even though the CAN specification says they should.
    	bms_broadcast.header.ExtId = 0x1839F380; // The CAN ID is offset by the module number
    	bms_broadcast.data[0] = module_number; // The TEM protocol for this number starts from 0
    	bms_broadcast.data[1] = *(uint8_t*)(&minimum_thermistor_temperature);
    	bms_broadcast.data[2] = *(uint8_t*)(&maximum_thermistor_temperature);
    	bms_broadcast.data[3] = *(uint8_t*)(&average_thermistor_temperature);
    	bms_broadcast.data[4] = THERMISTOR_COUNT;
    	bms_broadcast.data[5] = THERMISTOR_COUNT - 1;
    	bms_broadcast.data[6] = 0;

    	uint8_t bms_broadcast_checksum = 0x39 + 0x8; // Module number + 1, plus length (8) shifted to the left 4 bits
    	for (uint8_t i = 0; i < 7; i++) {
    		bms_broadcast_checksum += bms_broadcast.data[i];
    	}
    	bms_broadcast.data[7] = bms_broadcast_checksum;

    	// The TEM modules don't actually seem to offset their module numbers even though the CAN specification says they should.
    	general_broadcast.header.ExtId = 0x1838F380; // The CAN ID is offset by the module number
    	uint16_t absolute_thermistor_id = (uint16_t)module_number * 80 + current_thermistor_id;
    	general_broadcast.data[0] = (uint8_t)((absolute_thermistor_id & 0xFF00) >> 8);
    	general_broadcast.data[1] = (uint8_t)(absolute_thermistor_id & 0x00FF);
		general_broadcast.data[2] = *(uint8_t*)(&ThermistorData.thermistors[current_thermistor_id]);

    	// The TEM modules don't seem to broadcast the thermistor count, instead they just repeat the module relative thermistor ID.
    	general_broadcast.data[3] = current_thermistor_id;
    	general_broadcast.data[4] = *(uint8_t*)(&minimum_thermistor_temperature); // Min thermistor
    	general_broadcast.data[5] = *(uint8_t*)(&maximum_thermistor_temperature); // Max thermistor
    	general_broadcast.data[6] = THERMISTOR_COUNT - 1;
    	general_broadcast.data[7] = 0;

        osMessageQueuePut(CANTX_QHandle, &bms_broadcast, 0, 0);
        osMessageQueuePut(CANTX_QHandle, &general_broadcast, 0, 0);

    	if (++current_thermistor_id == THERMISTOR_COUNT) {
    		current_thermistor_id = 0;
    	}
        osDelayUntil(tick += TEM_PERIOD);
    }
}
