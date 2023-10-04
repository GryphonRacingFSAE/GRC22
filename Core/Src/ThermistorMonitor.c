#include "ThermistorMonitor.h"
#include "utils.h"
#include "main.h"
#include <math.h>
#include <CAN.h>

extern ADC_HandleTypeDef hadc1;

// Use NTC 10K Thermistor Parameters
// Values found here: https://www.vishay.com/docs/29218/ntcle350e4.pdf
#define CALIBRATION_TEMPERATURE 298.15
#define CALIBRATION_RESISTANCE 10000
#define DIVIDER_RESISTANCE 10000
#define THERMISTOR_BETA 3435
#define ADC_RESOLUTION 4095


ThermistorData_Struct ThermistorData = { .thermistors = {} };

void startThermistorMonitorTask() {
	uint32_t tick = osKernelGetTickCount();
	uint8_t current_thermistor = 0;


	while (1) {

		// Resetting pins A, B, C; then setting the next binary sequence as per table 1:
		HAL_GPIO_WritePin(GPIOA, 0b111000, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, current_thermistor<<3, GPIO_PIN_SET);

		osDelay(100);

        // Grab the temperature for all modules at once
        for (uint8_t module = 0; module < MODULE_COUNT; module++) {
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            uint32_t divider_0_to_4095 = HAL_ADC_GetValue(&hadc1);

            DEBUG_PRINT("----------------------------\n");
            DEBUG_PRINT("Divider Analog Range: %d\n", divider_0_to_4095);
            DEBUG_PRINT("Reference Voltage: %d\n", 3.3);



            uint32_t divider_voltage = divider_0_to_4095/ADC_RESOLUTION * 3;

            DEBUG_PRINT("Divider Voltage: %d\n", divider_voltage);

            uint32_t thermistor_resistance = (divider_voltage * DIVIDER_RESISTANCE) / (3 - divider_voltage);

            DEBUG_PRINT("Thermistor Resistance: %d\n", thermistor_resistance);


            // TODO: pick a better sentinel value
            if (thermistor_resistance == 0) {
            	ThermistorData.thermistors[module][current_thermistor] = -1000;
            	continue;
            }

            // Convert Thermistor Resistance to Temperature
            // This is a rearrangement of the equation found here: https://www.lasercalculator.com/ntc-thermistor-calculator/
            int32_t therm_temp = (CALIBRATION_TEMPERATURE * THERMISTOR_BETA) / ((CALIBRATION_TEMPERATURE*log(thermistor_resistance/CALIBRATION_RESISTANCE)) + THERMISTOR_BETA);

            DEBUG_PRINT("Thermistor Temperature: %d\n", therm_temp);

            //GRCprintf("Channel: %d, Count: %d, Temp: %d, Resistance: %d, Percent of voltage: %d, Measured Voltage: %d\n", (int32_t)module, (int32_t)current_thermistor, thermistor_temperature, (int32_t)thermistor_resistance, (int32_t)(percent_of_divider_input_voltage * 100), divider_voltage);

            ThermistorData.thermistors[module][current_thermistor] = therm_temp;
        }

        // Counter Variable for MUX Select Lines
		if (++current_thermistor == THERMISTORS_PER_MODULE) {
			current_thermistor = 0;
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
        //GRCprintf("Sent TX broadcast\n");

		osDelayUntil(tick += THERMISTOR_PERIOD);
	}
}
