/*
 * APPS.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt and Ian McKechnie
 */

#include "APPS.h"
#include "utils.h"
#include "CAN1.h"
#include "rpmTable.h"
#include <string.h>

#define AVG_WINDOW			3
#define APPS1_MIN 			410
#define APPS1_MAX			1230
#define APPS_DIFF_THRESH	90

float interpolate(float a, float b)
{
    // Interpolation formula: (a + b) / 2
    return (a + b) / 2;
}

APPS_Data_Struct APPS_Data;

volatile uint16_t ADC1_buff[ADC1_BUFF_LEN];

void startAPPSTask() {

	//used for averaging the apps signal
	int32_t apps1Avg = 0;
	int32_t apps2Avg = 0;

	int32_t appsPos = 0;

	CANMsg txMsg;

	//circular buffers for moving average
	uint32_t apps1PrevMesurments[AVG_WINDOW];
	uint32_t apps2PrevMesurments[AVG_WINDOW];

	//position in circular buffer used for moving average
	uint8_t circBuffPos = 0;

	uint32_t tick;

	tick = osKernelGetTickCount();

	while (1) {

		//Averages samples in DMA buffer
		apps1Avg = 0;
		for (int i = 0; i < ADC1_BUFF_LEN; i++) {
			if(i%2 ==0){
				apps1Avg += ADC1_buff[i];
			} else {
				apps2Avg += ADC1_buff[i];
			}
		}

		apps1Avg = (apps1Avg + (1<<8)) >> 10;
		apps2Avg = (apps2Avg + (1<<8)) >> 10;


		//Calculates moving average of previous measurements
		if(++circBuffPos == AVG_WINDOW){
			circBuffPos = 0;
		}
		apps1PrevMesurments[circBuffPos] = apps1Avg;
		apps2PrevMesurments[circBuffPos] = apps2Avg;

		apps1Avg = 0;
		apps2Avg = 0;

		for (int i = 0; i < AVG_WINDOW; i++) {
			apps1Avg += apps1PrevMesurments[i];
			apps2Avg += apps2PrevMesurments[i];
		}
		apps1Avg = apps1Avg/AVG_WINDOW;
		apps2Avg = apps2Avg/AVG_WINDOW;


		//TODO compare APPS signal to detect plausibility error;

		appsPos= (apps1Avg - APPS1_MIN) * 100 /(APPS1_MAX - APPS1_MIN);
		appsPos = MAX(MIN(appsPos,100),0);

		if (osMutexAcquire(APPS_Data_MtxHandle, 5) == osOK){
			APPS_Data.pedalPos = appsPos;

			osMutexRelease(APPS_Data_MtxHandle);
		}

		//Pedal Mapping
		int pedalPercent = 0; //X value
		int rpmNumber = 0; //Y value

		int pedalPercentWholeNumber = pedalPercent/10; //Chops off the ones digit
		int pedalPercentOnesColumn = pedalPercent%10; //Gets the ones digit

		int rpmId = 0;
		for (int i = 0; i < 14; i++) {
			if (rpmNumber < rpms[2]) {
				rpmId = i;
				break;
			}
		}

		int value;
		//Find the four points around the two values
		int xDiff = pedalPercentWholeNumber % 10;
		int lowerBound = pedalPercentWholeNumber - xDiff;
		int upperBound = lowerBound + 10;
		float xDiffAsPercent= Float(xDiff) * 0.001

		float yDiff = Float(rpms[rpmId][1] % 454) * 0.01;
		int lowerBoundRPM = rpms[rpmId][1];
		int upperBoundRPM = lowerBoundRPM + 454;

		// If it's not at its max x or y
		if (pedalPercentWholeNumber != 120 && rpmId != 12) {

			float x1Interpolation = interpolate(Float(rpmTable[lowerBound][lowerBoundRPM]), Float(rpmTable[lowerBound][upperBoundRPM]));
			float x2Interpolation = interpolate(Float(rpmTable[upperBound][lowerBoundRPM]), Float(rpmTable[upperBound][upperBoundRPM]));

			x1Interpolation *= xDiffAsPercent;
			x2Interpolation *= xDiffAsPercent;

			float yInterpolation = interpolate(x1Interpolation, x2Interpolation);

			value = yInterpolation * yDiff;


		// If it's at its max x but not max y
		} else if (pedalPercentWholeNumber == 100 && rpmId != 14) {

			float yInterpolation = interpolate(Float(rpmTable[pedalPercentWholeNumber][lowerBoundRPM]), Float(rpmTable[pedalPercentWholeNumber][upperBoundRPM]));
			value = yInterpolation * yDiff;

		// If it's at its max y but not max x
		} else if (pedalPercentWholeNumber != 100 && rpmId == 14) {
			float xInterpolation = interpolate(Float(rpmTable[lowerBound][rpmId]), Float(rpmTable[upperBound][rpmId]));
			value = xInterpolation * xDiffAsPercent;

		// If it's at its max x and y
		} else {
			value = rpmTable[pedalPercentWholeNumber][rpmId];
		}

		//Formatting sample can message
		//TODO format can message as motor controller torque command


		txMsg.aData[3] = apps1Avg & 0xFFU;
		txMsg.aData[2] = apps1Avg >> 8 & 0xFFU;
		txMsg.aData[1] = apps1Avg >> 16 & 0xFFU;
		txMsg.aData[0] = apps1Avg >> 24 & 0xFFU;

		txMsg.header.DLC = 4;
		txMsg.header.StdId = 0x69U;

		txMsg.header.ExtId = 0;
		txMsg.header.IDE = 0;
		txMsg.header.RTR = CAN_RTR_DATA;
		txMsg.header.TransmitGlobalTime = DISABLE;

		myprintf("APPS1:%d, APPS_POS:%d\n\r", apps1Avg, appsPos);


		osMessageQueuePut(CAN1_QHandle, &txMsg, 0, 5);


		tick+= APPS_PERIOD;
		osDelayUntil(tick);
	}
}
