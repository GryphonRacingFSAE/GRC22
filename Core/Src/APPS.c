/*
 * APPS.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt and Ian McKechnie
 */

#include "APPS.h"
#include "utils.h"
#include "CAN1.h"
#include "torqueTable.h"
#include <string.h>

#define AVG_WINDOW			3
#define APPS1_MIN 			410
#define APPS1_MAX			1230
#define APPS_DIFF_THRESH	90


//Rows (pedal percent): 0   10	 20 	30 	    40 	    50 	    60 	    70 	 	80 		90 		100
//Columns RPM		    0	454	 909	1363	1817	2271	2726	3180	3634	4089	4543	4997	5452	5906
int torqueTable[11][14] = {
	{-6,  -21,     29,  -28   -27,  -25,  -22,  -26,  -27,  -23,  -21,  -17,  -19,  -19},
	{12,   -5,    -15,  -14,  -11,   -8,   -7,   -8,   -6,   -8,   -7,   -5,   -4,   -4},
	{27,   15,      5,   7,     5,    5,    5,    5,    5,    4,    5,    5,    5,    5},
	{45,   38,     27,  24,    24,   23,   22,   21,   24,   23,   20,   18,   13,   13},
	{68,   57,     50,  50,    48,   51,   49,   50,   42,   47,   42,   46,   42,   42},
	{84,   78,     73,  74,    73,   73,   70,   65,   63,   71,   62,   55,   57,   57},
	{97,   101,    95,  94,    98,   96,   99,   91,   92,   86,   81,   76,   71,   71},
	{110,  110,   109,  112,  108,  111,  110,  108,  106,  101,   89,   81,   72,   72},
	{118,  118,   117,  125,  122,  126,  119,  116,  113,  106,   89,   89,   76,   76},
	{127,  126,   126,  129,  127,  129,  124,  122,  116,  107,  106,   89,   80,   80},
	{130,  131,   131,  130,  131,  131,  131,  131,  121,  110,   98,   87,   78,   78}
};

// Loopup table for the torqueTable array
int rpms [14][2] = {
	{0, 454},
	{1, 909},
	{2, 1363},
	{3, 1817},
	{4, 2271},
	{5, 2726},
	{6, 3180},
	{7, 3634},
	{8, 4088},
	{9, 4543},
	{10, 4997},
	{11, 5452},
	{12, 5906},
	{13, 6360}
};


float interpolate(float a1, float b1, float a2, float b2, float x)
{
	// Interpolation formula: y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
	return b1 + (x - a1) * (b2 - b1) / (a2 - a1);
}

APPS_Data_Struct APPS_Data;
Torque_Map_Struct Torque_Map_Data = { { {} }, { {} }, &Torque_Map_Data.map1 };

//Buffer from DMA
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
		//Circular for moving average
		apps1PrevMesurments[circBuffPos] = apps1Avg;
		apps2PrevMesurments[circBuffPos] = apps2Avg;

		apps1Avg = 0;
		apps2Avg = 0;

		for (int i = 0; i < AVG_WINDOW; i++) {
			apps1Avg += apps1PrevMesurments[i];
			apps2Avg += apps2PrevMesurments[i];
		}

		//Moving average of raw analog value
		apps1Avg = apps1Avg/AVG_WINDOW;
		apps2Avg = apps2Avg/AVG_WINDOW;


		//TODO compare APPS signal to detect plausibility error;

		appsPos= (apps1Avg - APPS1_MIN) * 100 /(APPS1_MAX - APPS1_MIN);

		//Used for BSPC
		appsPos = MAX(MIN(appsPos,100),0);

		if (osMutexAcquire(APPS_Data_MtxHandle, 5) == osOK){
			APPS_Data.pedalPos = appsPos;

			osMutexRelease(APPS_Data_MtxHandle);
		}

		//Pedal Mapping
		int pedalPercent = 0; //X value
		int rpmNumber = 0; //Y value

		// Using the lookup table to find the column number
		int rpmId = 0;
		for (int i = 0; i < 14; i++) {
			if (rpmNumber < torqueTable[i][2]) {
				rpmId = i;
				break;
			}
		}

		int torque;
		//Find the four points around the two values
		int pedalPercentOnesColumn = pedalPercent % 10;	//Get the ones column from the pedal percent *** This assumes the percent is a whole number ***
		int lowerBoundPedal = pedalPercent - pedalPercentOnesColumn; // This finds the table index value lower than
		int upperBoundPedal = lowerBoundPedal + 10;	// This gets the upper bound of the pedal percent
		// float pedalPercentOnesColumnAsPercent = (float)pedalPercentOnesColumn * 0.01;

		// float yDiff = (float)(torqueTable[rpmId][1] % 454) * 0.01;
		int lowerBoundRPM = torqueTable[rpmId][1];	//Find the rpm value lower than it
		int upperBoundRPM = lowerBoundRPM + 454;	//Find the rpm value higher than it

		// If it's not at its max x or y
		if (pedalPercent != 120 && rpmId != 12) {
			float x1Interpolation = interpolate((float)lowerBoundPedal, (float)lowerBoundRPM, (float)lowerBoundPedal, (float)upperBoundPedal, (float)pedalPercent);
			float x2Interpolation = interpolate((float)upperBoundPedal, (float)lowerBoundRPM, (float)upperBoundPedal, (float)upperBoundPedal, (float)pedalPercent);

			// x1Interpolation *= pedalPercentOnesColumnAsPercent;
			// x2Interpolation *= pedalPercentOnesColumnAsPercent;

			torque = interpolate(x1Interpolation, lowerBoundRPM, x2Interpolation, upperBoundRPM, (float)rpmNumber);

		// If it's at its max x but not max y
		} else if (pedalPercentWholeNumber == 100 && rpmId != 14) {
			torque = interpolate((float)lowerBoundPedal, (float)lowerBoundRPM, (float)lowerBoundPedal, (float)upperBoundPedal, (float)pedalPercent);

		// If it's at its max y but not max x
		} else if (pedalPercentWholeNumber != 100 && rpmId == 14) {
			torque = interpolate((float)lowerBoundRPM, (float)lowerBoundPedal, (float)upperBoundRPM, (float)lowerBoundPedal, (float)rpmNumber);
			// float xInterpolation = interpolate((float)(torqueTable[lowerBound][rpmId]), (float)(torqueTable[upperBound][rpmId]));
			// torque = xInterpolation * pedalPercentOnesColumnAsPercent;

		// If it's at its max x and y
		} else {
			torque = torqueTable[pedalPercentWholeNumber][rpmId];
		}

		//Formatting sample can message
		//TODO format can message as motor controller torque command


		txMsg.aData[3] = apps1Avg & 0xFFU;
		txMsg.aData[2] = apps1Avg >> 8 & 0xFFU;
		txMsg.aData[1] = apps1Avg >> 16 & 0xFFU;
		txMsg.aData[0] = apps1Avg >> 24 & 0xFFU;

		txMsg.header.DLC = 4;
		txMsg.header.StdId = 0x69U;
		txMsg.header.IDE = CAN_ID_STD;
		txMsg.header.RTR = CAN_RTR_DATA;

		DEBUG_PRINT("APPS1:%d, APPS_POS:%d\r\n", apps1Avg, appsPos);


		osMessageQueuePut(CAN2_QHandle, &txMsg, 0, 5);


		tick+= APPS_PERIOD;
		osDelayUntil(tick);
	}
}
