#include "ThermistorMonitor.h"
#include "utils.h"
#include "main.h"
#include <math.h>

extern ADC_HandleTypeDef hadc1;

// Use NTC 10K Thermistor Parameters
#define CALIBRATION_TEMPERATURE 298.15
#define CALIBRATION_RESISTANCE 10000
#define DIVIDER_RESISTANCE 10000
#define THERMISTOR_BETA 3435
#define CHANNEL_COUNT 4
#define ADC_RESOLUTION ADC_RESOLUTION12b

ThermistorData_Struct ThermistorData = {};

void startThermistorMonitorTask() {
	uint32_t tick = osKernelGetTickCount();
	uint8_t counter = 0;

	while (1) {
		// Counter Variable for MUX Select Lines
        if (++counter > 7) {
            counter = 0;
        }

        // Grab the temperature for all channels at once
        for (uint8_t channel = 0; channel < CHANNEL_COUNT; channel++) {
            HAL_GPIO_WritePin(GPIOA, 0b111000, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, counter<<3, GPIO_PIN_SET);

            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            float divider_voltage = HAL_ADC_GetValue(&hadc1);

            // Obtain Thermistor Resistance from Measured Voltage
            float percent_of_divider_input_voltage = (ADC_RESOLUTION - divider_voltage) / ADC_RESOLUTION;
            float thermistor_resistance = DIVIDER_RESISTANCE * percent_of_divider_input_voltage;

            // Convert Thermistor Resistance to Temperature
            // This is a rearrangement of the equation found here: https://www.lasercalculator.com/ntc-thermistor-calculator/
            int32_t thermistor_temperature = 1.0/(ln(thermistor_resistance/CALIBRATION_RESISTANCE) / THERMISTOR_BETA + 1.0/CALIBRATION_TEMPERATURE);

            GRCprintf("VT = %d, Channel %d Resistance = %d Temperature = %ld\n", divider_voltage, counter, (uint32_t)thermistor_resistance, thermistor_temperature);

            ThermistorData.thermistors[counter * CHANNEL_COUNT + channel] = thermistor_temperature;
        }

		osDelayUntil(tick += THERMISTOR_PERIOD);
	}
}
