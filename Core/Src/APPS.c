/*
 * APPS.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt
 */

#include "APPS.h"
#include "utils.h"
#include "CAN1.h"
#include <string.h>

APPS_Data_Struct APPS_Data;

volatile uint16_t ADC1_buff[ADC1_BUFF_LEN];

void startAPPSTask(){
	uint32_t adc_avg = 0;

	uint32_t tick;
	tick = osKernelGetTickCount();
	CANMsg txMsg;
	int i = 0;

	while(1){

		adc_avg = 0;
		for(i = 0; i < 4; i++){
			adc_avg += ADC1_buff[i*2];
		}
		adc_avg = adc_avg>>2;

		txMsg.aData[3] = adc_avg & 0xFFU;
		txMsg.aData[2] = adc_avg>>8 & 0xFFU;
		txMsg.aData[1] = adc_avg>>16 & 0xFFU;
		txMsg.aData[0] = adc_avg>>24 & 0xFFU;

		txMsg.DLC = 4;
		txMsg.StdId = 0x69U;

		txMsg.ExtId = 0;
		txMsg.IDE = 0;

		myprintf("ADC: %d \n\r", adc_avg);

		osMessageQueuePut(CAN1_QHandle,&txMsg,0,osWaitForever);
		tick += APPS_PERIOD;

		osDelayUntil(tick);
	}
}
