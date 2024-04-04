#include "ThermistorMonitor.h"
#include "main.h"
#include "utils.h"
#include <CAN.h>
#include <math.h>
#include <stdlib.h>

extern ADC_HandleTypeDef hadc1;

// Use NTC 10K Thermistor Parameters
// Values found here: https://www.vishay.com/docs/29218/ntcle350e4.pdf

#define CALIBRATION_TEMPERATURE 298.15
#define CALIBRATION_RESISTANCE 10000
#define THERMISTOR_BETA 3435

#define DIVIDER_RESISTANCE 10000
#define ADC_RESOLUTION 4095
#define REFERENCE_VOLTAGE 3.3

// Use Steinhart & Hart coefficients for NTC 10k Thermistor
// Values found using this site (ordering code: NTCLE350E4103FLB0): https://www.vishay.com/en/thermistors/ntc-rt-calculator/
// (VALUES ARE HIDDEN UNDER INSPECT ELEMENT)
#define TEMP_COEFF_A 3.354016434680530000E-3
#define TEMP_COEFF_B 3.00130825115663000E-4
#define TEMP_COEFF_C 5.085164943790940E-6
#define TEMP_COEFF_D 2.18765049258341E-7

// #define RES_COEFF_A -12.89228328
// #define RES_COEFF_B 4245.14800000
// #define RES_COEFF_C -87493.00000000
// #define RES_COEFF_D -9588114.00000000

ThermistorData_Struct ThermistorData = {.thermistors = {}};

/* This function is called to reconfigure the current ADC Conversion channel without
 * interrupts for the possibility to iterate over multiplexers on different ADC channels
 */
void adcChangeMUX(int channel) {
    ADC_ChannelConfTypeDef sConfig = {0};

    // Selects new ADC channel for conversion
    if (!DEV_BOARD) {
        switch (channel) {
        case 0:
            sConfig.Channel = ADC_CHANNEL_8;
            break;
        case 1:
            sConfig.Channel = ADC_CHANNEL_7;
            break;
        case 2:
            sConfig.Channel = ADC_CHANNEL_2;
            break;
        case 3:
            sConfig.Channel = ADC_CHANNEL_9;
            break;
        }
    } else {
        sConfig.Channel = ADC_CHANNEL_8;
    }

    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

/* This function gets the thermistor readings from multiple multiplexers
 *
 *
 * The order of how we scan for the data is as such:
 *
 * 1. Iterate over the Multiplexers, getting the thermistor on a specified select line on each MUX
 * 2. Perform temperature calculations for each thermistor readings and store the data
 * 3. Change to the next Select Line and repeat process
 */
void startThermistorMonitorTask() {
    uint32_t tick = osKernelGetTickCount();
    uint8_t select_line = 0;

    while (1) {
        // Resetting pins A, B, C; then setting the next binary sequence as per table 1:
        HAL_GPIO_WritePin(GPIOA, 0b1111000, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, select_line << 3, GPIO_PIN_SET);

        // This delay should be quite small, and only large enough to guarentee
        // the multiplexers have time to react to the new addresses
        osDelay(2);

        // Grab the temperature for all modules at once
        for (uint8_t cur_mux = 0; cur_mux < MUX_COUNT; cur_mux++) {
        	// Skip this multiplexer for this select line, as there is no thermistor here.
        	if (select_line == UNPOPULATED_THERMISTORS[cur_mux][0] || select_line == UNPOPULATED_THERMISTORS[cur_mux][1]) {
        		continue;
        	}

        	// This math relies on the fact that the UNPOPULATED_THERMISTORS array is arranged from least to greatest
            uint8_t current_thermistor_id = cur_mux * THERMISTORS_PER_MUX + select_line - (uint8_t)(select_line >= UNPOPULATED_THERMISTORS[cur_mux][0]) - (uint8_t)(select_line >= UNPOPULATED_THERMISTORS[cur_mux][1]);

            // changes the MUX that the ADC polls
            adcChangeMUX(cur_mux);
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            uint32_t divider_output = HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);

            // converts ADC units to volts
            float divider_voltage = ((float)divider_output / (float)ADC_RESOLUTION * REFERENCE_VOLTAGE);

            // calculates resistance of the thermistor using rearrangement of voltage divider formula:
            // https://ohmslawcalculator.com/voltage-divider-calculator
            float thermistor_resistance = (divider_voltage * DIVIDER_RESISTANCE) / (REFERENCE_VOLTAGE - divider_voltage);

            // 100 ohms of resistance with our thermistors is about 200 degrees C, the battery pack is long gone by then.
            // This is more for detecting open circuit connections.
            // It seems that the TEM modules use -41 degrees as sentinal values, so we're copying it.
            if (thermistor_resistance < 100) {
                ThermistorData.thermistors[current_thermistor_id] = -41;
                continue;
            }

            // Convert Thermistor Resistance to Temperature
            // This is an equation which is can be more accurate than generalizing the shape of the temperature-resistange curve, found here:
            // https://www.ametherm.com/thermistor/ntc-thermistors-steinhart-and-hart-equation
            float stein_temp = 1.0 / (TEMP_COEFF_A + (TEMP_COEFF_B * log(thermistor_resistance / (float)CALIBRATION_RESISTANCE)) +
                                      TEMP_COEFF_C * powf(log(thermistor_resistance / (float)CALIBRATION_RESISTANCE), 2.0) +
                                      TEMP_COEFF_D * powf(log(thermistor_resistance / (float)CALIBRATION_RESISTANCE), 3.0));

            float stein_temp_celsius = stein_temp - 273.15f;

            int8_t thermistor_temperature_celsius = (int8_t)round(stein_temp_celsius);

        	DEBUG_PRINT("Multiplexer: %d, Index: %d Thermistor ID: %d Measured Voltage: %d Temperature: %dc\n", (int)cur_mux, (int)select_line, (int)current_thermistor_id, (int)divider_output, thermistor_temperature_celsius);
            ThermistorData.thermistors[current_thermistor_id] = thermistor_temperature_celsius;
        }

        // Counter Variable for MUX Select Lines
		if (++select_line == MULTIPLEXER_SIZE) {
			select_line = 0;
		}


        osDelayUntil(tick += THERMISTOR_PERIOD);
    }
}
