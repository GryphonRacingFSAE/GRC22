#include "ThermistorMonitor.h"
#include "utils.h"
#include "main.h"
#include <math.h>

extern ADC_HandleTypeDef hadc1;

ThermistorData_Struct ThermistorData = {};

void startThermistorMonitorTask() {
	uint32_t tick = osKernelGetTickCount();
	uint8_t counter = 0;
	float R0 = 10000;
	float R1= 10000;


	while (1) {

		counter = (counter+1) & 7;
		HAL_GPIO_WritePin(GPIOA, 0b111000, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(GPIOA, counter<<3, GPIO_PIN_SET);

	    // Startup the ADC
	    HAL_ADC_Start(&hadc1);

	    //Obtain ADC Measurement
	    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		uint32_t Vt = HAL_ADC_GetValue(&hadc1);

	    //Obtain Thermistor Resistance from Measured Voltage
	    float R2 = (R1 * (4095.0/ (4095.0-(float)Vt)-1.0));

		//Convert Thermistor Resistance to Temperature
	    int32_t temp = ((1.0/((1.0/298.15)+((1.0/3435)*log(R2/R0))))-273.15);

		GRCprintf("VT = %d, Channel %d Resistance = %d Temperature = %ld\n",Vt,counter,((uint32_t)R2),temp);

		osDelayUntil(tick += THERMISTOR_PERIOD);
	}
}
