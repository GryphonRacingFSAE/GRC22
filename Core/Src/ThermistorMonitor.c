#include "ThermistorMonitor.h"
#include "utils.h"
#include "main.h"
#include <math.h>
#include <CAN.h>

extern ADC_HandleTypeDef hadc1;

// Use NTC 10K Thermistor Parameters
#define CALIBRATION_TEMPERATURE 298.15
#define CALIBRATION_RESISTANCE 10000
#define DIVIDER_RESISTANCE 10000
#define THERMISTOR_BETA 3435
#define ADC_RESOLUTION 4095

ThermistorData_Struct ThermistorData = { .thermistors = {} };

void startThermistorMonitorTask() {
	uint32_t tick = osKernelGetTickCount();
	uint8_t counter = 0;

	while (1) {
		// Counter Variable for MUX Select Lines
        if (++counter == THERMISTORS_PER_CHANNEL) {
            counter = 0;
        }

		HAL_GPIO_WritePin(GPIOA, 0b111000, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, counter<<3, GPIO_PIN_SET);

		osDelay(100);

        // Grab the temperature for all channels at once
        for (uint8_t channel = 0; channel < CHANNEL_COUNT; channel++) {
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            uint32_t divider_voltage = HAL_ADC_GetValue(&hadc1);

            // Obtain Thermistor Resistance from Measured Voltage
            float percent_of_divider_input_voltage = (ADC_RESOLUTION - (float)divider_voltage) / ADC_RESOLUTION;
            float thermistor_resistance = DIVIDER_RESISTANCE * percent_of_divider_input_voltage;

            // TODO: pick a better sentinel value
            if (thermistor_resistance == 0) {
            	ThermistorData.thermistors[counter * CHANNEL_COUNT + channel] = -1000;
            	continue;
            }

            // Convert Thermistor Resistance to Temperature
            // This is a rearrangement of the equation found here: https://www.lasercalculator.com/ntc-thermistor-calculator/
            int32_t thermistor_temperature = 1.0/(log(thermistor_resistance/CALIBRATION_RESISTANCE) / THERMISTOR_BETA + 1.0/CALIBRATION_TEMPERATURE);

            GRCprintf("Channel: %d, Count: %d, Temp: %d, Resistance: %d, Percent of voltage: %d, Measured Voltage: %d\n", (int32_t)channel, (int32_t)counter, thermistor_temperature, (int32_t)thermistor_resistance, (int32_t)(percent_of_divider_input_voltage * 100), divider_voltage);

            ThermistorData.thermistors[counter * CHANNEL_COUNT + channel] = thermistor_temperature;
        }

        CANTXMsg bms_broadcast = {
        	.header = {
        		.IDE = CAN_ID_EXT,
        		.RTR = CAN_RTR_DATA,
        		.ExtId = 0x1839F380,
        		.DLC = 8
        	},
        	.to = &hcan
        };

        osMessageQueuePut(CANTX_QHandle, &bms_broadcast, 0, 5);
        GRCprintf("Sent TX broadcast\n");

		osDelayUntil(tick += THERMISTOR_PERIOD);
	}
}
