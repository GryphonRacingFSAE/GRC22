/*
 * control.c
 *
 *  Created on: Jan 20, 2023
 *      Author: Matt
 *
 */

#include "control.h"
#include <string.h>

Ctrl_Data_Struct Ctrl_Data;

Ctrl_Data_Struct tempCtrl_Data;

void startControlTask(){
	uint32_t tick;
	tick = osKernelGetTickCount();

	while(1){
		if (osMutexAcquire(Ctrl_Data_MtxHandle, CTRL_PERIOD) == osOK){
		memcpy(&tempCtrl_Data, &Ctrl_Data, sizeof(Ctrl_Data_Struct));
		}

		BSPC();
		RTD();
		pumpCtrl();
		fanCtrl();

		tick += CTRL_PERIOD;
		osDelayUntil(tick);
	}

}

void BSPC(){

}
void RTD(){

}
void pumpCtrl(){

}
void fanCtrl(){

}
