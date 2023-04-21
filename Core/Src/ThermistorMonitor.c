#include "ThermistorMonitor.h"
#include "utils.h"
#include "main.h"
#include <math.h>
#include <stdio.h>

ThermistorData_Struct ThermistorData = {};

void startThermistorMonitorTask() {

	uint32_t Vt;
	int32_t temp;
	uint8_t counter = 0;
	float R0 = 10000;
	float R1= 10000;
	float R2;


	while (1) {

		counter = (counter+1) & 7;
		HAL_GPIO_WritePin(GPIOA, 0b111000, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(GPIOA, counter<<3, GPIO_PIN_SET);

	    HAL_ADC_Start(&hadc1);
	    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

	    //Obtain ADC Measurement
		Vt = HAL_ADC_GetValue(&hadc1);

	    //Obtain Thermistor Resistance from Measured Voltage
	    R2 = (R1 * (4095.0/ (4095.0-(float)Vt)-1.0));

		//Convert Thermistor Resistance to Temperature
	    temp = ((1.0/((1.0/298.15)+((1.0/3435)*log(R2/R0))))-273.15);

		printf("VT = %d, Channel %d Resistance = %d Temperature = %ld\n",Vt,counter,((uint32_t)R2),temp);

		HAL_Delay(500);

	}
}
