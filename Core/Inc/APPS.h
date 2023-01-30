/*
 * APPS.h
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt
 */

#ifndef INC_APPS_H_
#define INC_APPS_H_

#include "main.h"
#include "cmsis_os.h"

#define ADC1_BUFF_LEN 1024

extern volatile uint16_t ADC1_buff[ADC1_BUFF_LEN];

typedef struct {
	uint16_t torque;
	uint16_t pedalPos;
	uint32_t flags;
} APPS_Data_Struct;

extern APPS_Data_Struct APPS_Data;

void startAPPSTask();

#endif /* INC_APPS_H_ */
