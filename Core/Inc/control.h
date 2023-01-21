/*
 * control.h
 *
 *  Created on: Jan 20, 2023
 *      Author: Matt
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include "main.h"
#include "cmsis_os.h"

typedef struct{
	uint32_t wheelspeed[4];
	uint32_t motorControllerTemp;
	uint32_t coolantTemp;
	uint32_t tractiveV;
}Ctrl_Data_Struct;

extern Ctrl_Data_Struct Ctrl_Data;
extern osMutexId_t Ctrl_Data_MtxHandle;

void startControlTask();

void BSPC();
void RTD();
void pumpCtrl();
void fanCtrl();

#endif /* INC_CONTROL_H_ */
