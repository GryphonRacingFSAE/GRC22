/*
 * APPS.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt and Ian McKechnie
 */

#include <CAN.h>
#include "APPS.h"
#include "utils.h"
#include "control.h"
#include <string.h>

#define APPS1_MIN 1440
#define APPS1_MAX 1375
#define APPS2_MIN (APPS1_MIN * 2)
#define APPS2_MAX (APPS1_MAX * 2)

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

int16_t interpolate(int16_t xdiff, int16_t ydiff, int16_t yoffset, int16_t xoffset_from_x1) {
	// Interpolation formula: y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
	return yoffset + xoffset_from_x1 * ydiff / xdiff;
}

APPS_Data_Struct APPS_Data;
// Columns are RPM in increments of 500 (0-6500), Rows are pedal percent in increments of 10% (0-100%)
Torque_Map_Struct Torque_Map_Data = { {
    { -6, -20, -22, -22, -22, -22, -22, -22, -22, -22, -21, -17, -19, -22 },
    { 12, -5, -15, -14, -11, -8, -7, -8, -6, -8, -7, -4, -12, -20 },
    { 27, 15, 5, 7, 5, 5, 5, 5, 5, 4, 5, 5, -6, -18 },
    { 44, 37, 26, 23, 24, 23, 22, 21, 24, 23, 20, 18, 1, -16 },
    { 67, 57, 49, 49, 48, 51, 48, 50, 42, 47, 41, 46, 16, -14 },
    { 83, 78, 72, 73, 72, 72, 69, 64, 63, 70, 62, 55, 21, -12 },
    { 96, 100, 94, 93, 97, 95, 98, 90, 91, 85, 80, 75, 33, -10 },
    { 109, 109, 108, 111, 107, 110, 109, 107, 105, 100, 88, 80, 36, -8 },
    { 117, 116, 116, 123, 121, 125, 118, 115, 111, 105, 88, 88, 41, -6 },
    { 126, 125, 124, 127, 126, 127, 123, 120, 115, 106, 104, 88, 42, -4 },
    { 129, 130, 130, 129, 129, 130, 130, 130, 120, 108, 97, 86, 42, -2 }
}, {
    {-6, -20, -22, -22, -22, -22, -22, -22, -22, -22, -21, -17, -19, -22},
    {12, -5, -15, -14, -11, -8, -7, -8, -6, -8, -7, -4, -11, -17},
    {27, 15, 5, 7, 5, 5, 5, 5, 5, 4, 5, 5, -3, -12},
    {44, 37, 26, 23, 24, 23, 22, 21, 24, 23, 20, 18, 5, -7},
    {67, 57, 49, 49, 48, 51, 48, 50, 42, 47, 41, 46, 22, -2},
    {83, 78, 72, 73, 72, 72, 69, 64, 63, 70, 62, 55, 27, 0},
    {96, 100, 94, 93, 97, 95, 98, 90, 91, 85, 80, 75, 38, 0},
    {109, 109, 108, 111, 107, 110, 109, 107, 105, 100, 88, 80, 40, 0},
    {117, 116, 116, 123, 121, 125, 118, 115, 111, 105, 88, 88, 44, 0},
    {126, 125, 124, 127, 126, 127, 123, 120, 115, 106, 104, 88, 44, 0},
    {129, 130, 130, 129, 129, 130, 130, 130, 120, 108, 97, 86, 43, 0}
}, Torque_Map_Data.map1 };

//Buffer from DMA
volatile uint16_t ADC1_buff[ADC1_BUFF_LEN] = {};

void startAPPSTask() {
	uint32_t tick = osKernelGetTickCount();

	while (1) {

		//Averages samples in DMA buffer
		int32_t apps1_adc_avg = 0;
		int32_t apps2_adc_avg = 0;
		for (int i = 0; i < ADC1_BUFF_LEN;) {
			apps1_adc_avg += ADC1_buff[i++];
			apps2_adc_avg += ADC1_buff[i++];
		}

		apps1_adc_avg = apps1_adc_avg / (ADC1_BUFF_LEN / 2);
		apps2_adc_avg = apps2_adc_avg / (ADC1_BUFF_LEN / 2);

		int32_t apps1_pos = CLAMP(0, (apps1AvgDMA - APPS1_MIN) * 1000 /(APPS1_MAX - APPS1_MIN), 1000);
		int32_t apps2_pos = CLAMP(0, (apps2AvgDMA - APPS2_MIN) * 1000 /(APPS2_MAX - APPS2_MIN), 1000);
		CRITICAL_PRINT("APPS1:%d, APPS1 ADC: %d, APPS2:%d, APPS2 ADC: %d\r\n", apps1_pos, apps1_adc_avg, apps2_pos, apps2_adc_avg);

		// RULE (2023 V2): T.4.2.4 (Both APPS sensor positions must be within 10% of pedal travel of each other)
		// TODO: T.4.2.5
		// TODO?: T.4.2.9
		// TODO?: T.4.2.10
		// TODO: T.4.3.3
		// TODO: T.4.3.4
		int32_t appsPos = apps1_pos;
		if (ABS(appsPos1 - appsPos2) <= 10) {
			int32_t averageAppsPos = (appsPos1 + appsPos2) / 2;
			appsPos = MAX(MIN(averageAppsPos, 100),0); // Clamp to between 0-100%
		} else {
			// FAULT
		}

		//Used for BSPC
		// TODO: RULE (2023 V2): EV.4.1.3 No regen < 5km/h
		APPS_Data.pedalPos = appsPos;

		int32_t pedalPercent = MIN(appsPos, 99); // NOTE: Cap values at slightly less then our max % for easier math
		int32_t rpm = 0;

		rpm = MIN(Ctrl_Data.motorSpeed, 6499); // NOTE: Cap values at slightly less then our max rpm for easier math

		// Integer division - rounds down (use this to our advantage)
		int32_t pedalOffset = pedalPercent % 10;
		int32_t pedalLowIndex = pedalPercent / 10;
		int32_t pedalHighIndex = pedalLowIndex + 1;
		int32_t rpmOffset = rpm % 500;
		int32_t rpmLowIndex = rpm / 500;
		int32_t rpmHighIndex = rpmLowIndex + 1;
		// NOTE: because we capped our values, both lower indexes will never read the maximum index
		// this always leaves one column left for the high index.
		int16_t torque_pedallow_rpmlow = Torque_Map_Data.activeMap[pedalLowIndex][rpmLowIndex];
		int16_t torque_pedallow_rpmhigh = Torque_Map_Data.activeMap[pedalLowIndex][rpmHighIndex];
		int16_t torque_pedalhigh_rpmlow = Torque_Map_Data.activeMap[pedalHighIndex][rpmLowIndex];
		int16_t torque_pedalhigh_rpmhigh = Torque_Map_Data.activeMap[pedalHighIndex][rpmHighIndex];

		// Interpolating across rpm values
		int16_t torque_pedallow = interpolate(500, torque_pedallow_rpmhigh - torque_pedallow_rpmlow, torque_pedallow_rpmlow, rpmOffset);
		int16_t torque_pedalhigh = interpolate(500, torque_pedalhigh_rpmhigh - torque_pedalhigh_rpmlow, torque_pedalhigh_rpmlow, rpmOffset);
		int16_t requestedTorque = interpolate(10, torque_pedalhigh - torque_pedallow, torque_pedallow, pedalOffset);

//		requestTorque(requestedTorque); // Transmitting scales by 10 due to Limits of motor controller

		osDelayUntil(tick += APPS_PERIOD);
	}
}
