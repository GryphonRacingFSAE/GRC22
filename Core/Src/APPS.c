#include <CAN.h>
#include "APPS.h"
#include "utils.h"
#include "control.h"
#include <string.h>


extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

int16_t interpolate(int16_t xdiff, int16_t ydiff, int16_t yoffset, int16_t xoffset_from_x1) {
	// Interpolation formula: y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
	return yoffset + xoffset_from_x1 * ydiff / xdiff;
}

APPS_Data_Struct APPS_Data = {
	.torque = 0,
	.apps_position = 0,
	.brake_pressure = 0,
	.flags = APPS_BSPC_INVALID | APPS_SENSOR_OUT_OF_RANGE_INVALID | APPS_SENSOR_CONFLICT_INVALID | BRAKE_SENSOR_OUT_OF_RANGE_INVALID
};
// Columns are RPM in increments of 500 (0-6500), Rows are pedal percent in increments of 10% (0-100%)
Torque_Map_Struct Torque_Map_Data = { {
    {  -60, -200, -220, -220, -220, -220, -220, -220, -220, -220, -210, -170, -190, -220 },
    {  120,  -50, -150, -140, -110,  -80,  -70,  -80,  -60,  -80,  -70,  -40, -120, -200 },
    {  270,  150,   50,   70,   50,   50,   50,   50,   50,   40,   50,   50,  -60, -180 },
    {  440,  370,  260,  230,  240,  230,  220,  210,  240,  230,  200,  180,   50,  -70 },
    {  670,  570,  490,  490,  480,  510,  480,  500,  420,  470,  410,  460,  220,  -20 },
    {  830,  780,  720,  730,  720,  720,  690,  640,  630,  700,  620,  550,  270,    0 },
    {  960, 1000,  940,  930,  970,  950,  980,  900,  910,  850,  800,  750,  380,    0 },
    { 1090, 1090, 1080, 1110, 1070, 1100, 1090, 1070, 1050, 1000,  880,  800,  400,    0 },
    { 1170, 1160, 1160, 1230, 1210, 1250, 1180, 1150, 1110, 1050,  880,  880,  440,    0 },
    { 1260, 1250, 1240, 1270, 1260, 1270, 1230, 1200, 1150, 1060, 1040,  880,  440,    0 },
    { 1290, 1300, 1300, 1290, 1290, 1300, 1300, 1300, 1200, 1080,  970,  860,  430,    0 }
}, .scaling_factor = 5, .regen_enabled = 0 };

//Buffer from DMA
volatile uint16_t apps_dma_buffer[APPS_DMA_BUFFER_LEN] = {};
volatile uint16_t adc_3_dma_buffer[ADC_CHANNEL_3_DMA_BUFFER_LEN] = {};

void startAPPSTask() {
	uint32_t tick = osKernelGetTickCount();

	while (1) {
		// Average samples in DMA buffer
		int32_t apps1_adc_avg = 0;
		int32_t apps2_adc_avg = 0;
		for (int i = 0; i < APPS_DMA_BUFFER_LEN;) {
			apps1_adc_avg += apps_dma_buffer[i++];
			apps2_adc_avg += apps_dma_buffer[i++];
		}

		apps1_adc_avg /= (APPS_DMA_BUFFER_LEN / APPS_DMA_CHANNELS);
		apps2_adc_avg /= (APPS_DMA_BUFFER_LEN / APPS_DMA_CHANNELS);

		// RULE (2024 V1): T.4.2.10 (Detect open circuit and short circuit conditions)
		if (apps1_adc_avg <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= apps1_adc_avg || apps2_adc_avg <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= apps2_adc_avg) {
			SET_FLAG(APPS_Data.flags, APPS_SENSOR_OUT_OF_RANGE_INVALID);
		} else {
			CLEAR_FLAG(APPS_Data.flags, APPS_SENSOR_OUT_OF_RANGE_INVALID);
		}

		int32_t apps1_pos = CLAMP(0, (apps1_adc_avg - APPS1_MIN) * 1000 /(APPS1_MAX - APPS1_MIN), 1000);
		int32_t apps2_pos = CLAMP(0, (apps2_adc_avg - APPS2_MIN) * 1000 /(APPS2_MAX - APPS2_MIN), 1000);
		CRITICAL_PRINT("APPS1:%d, APPS1 ADC: %d, APPS2:%d, APPS2 ADC: %d\r\n", apps1_pos, apps1_adc_avg, apps2_pos, apps2_adc_avg);

		// TODO: T.4.2.5
		// TODO: T.4.3.3
		// TODO: T.4.3.4

		// RULE (2024 V1): T.4.2.4 (Both APPS sensor positions must be within 10% of pedal travel of each other)
		if (ABS(apps1_pos - apps2_pos) <= 100) {
			APPS_Data.apps_position = CLAMP(0, (apps1_pos + apps2_pos) / 2, 1000);
			CLEAR_FLAG(APPS_Data.flags, APPS_SENSOR_CONFLICT_INVALID);
		} else {
			SET_FLAG(APPS_Data.flags, APPS_SENSOR_CONFLICT_INVALID);
		}


		int32_t brake_pressure_adc_avg = 0;
		for (int i = 0; i < ADC_CHANNEL_3_DMA_BUFFER_LEN; i+=3) {
			brake_pressure_adc_avg += adc_3_dma_buffer[i];
		}
		brake_pressure_adc_avg /= (ADC_CHANNEL_3_DMA_BUFFER_LEN/ADC_CHANNEL_3_DMA_CHANNELS);

		if (brake_pressure_adc_avg <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= brake_pressure_adc_avg) {
			SET_FLAG(APPS_Data.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
		} else {
			CLEAR_FLAG(APPS_Data.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
		}
		APPS_Data.brake_pressure = CLAMP(0, (brake_pressure_adc_avg - BRAKE_PRESSURE_MIN) * 20000 /(BRAKE_PRESSURE_MAX - BRAKE_PRESSURE_MIN), 20000);
		CRITICAL_PRINT("BRAKE_PRESSURE:%d, BRAKE_PRESSURE ADC: %d\r\n", APPS_Data.brake_pressure, brake_pressure_adc_avg);

		// RULE (2024 V1): T.4.7 (BSPC)
		if (FLAG_ACTIVE(APPS_Data.flags, APPS_BSPC_INVALID)){
			// Check if the pedal position is <5% to put APPS back into a valid state (EV.4.7.2)
			if (APPS_Data.apps_position < 50) {
				CLEAR_FLAG(APPS_Data.flags, APPS_BSPC_INVALID);
			}
		} else if (APPS_Data.apps_position > 250 && HAL_GPIO_ReadPin(GPIO_BRAKE_SW_GPIO_Port, GPIO_BRAKE_SW_Pin)) {
			// Set to invalid if over >25% travel and brakes engaged (EV.4.7.1)
			SET_FLAG(APPS_Data.flags, APPS_BSPC_INVALID);
		}


		// NOTE: Cap values at slightly less then our max % for easier math
		int32_t pedalPercent = MIN(APPS_Data.apps_position, 99);
		int32_t rpm = MIN(Ctrl_Data.motor_speed, 6499); // NOTE: Cap values at slightly less then our max rpm for easier math

		// Integer division - rounds down (use this to our advantage)
		int32_t pedalOffset = pedalPercent % 1000;
		int32_t pedalLowIndex = pedalPercent / 1000;
		int32_t pedalHighIndex = pedalLowIndex + 1;
		int32_t rpmOffset = rpm % 500;
		int32_t rpmLowIndex = rpm / 500;
		int32_t rpmHighIndex = rpmLowIndex + 1;
		// NOTE: because we capped our values, both lower indexes will never read the maximum index
		// this always leaves one column left for the high index.
		int16_t torque_pedallow_rpmlow = Torque_Map_Data.map[pedalLowIndex][rpmLowIndex];
		int16_t torque_pedallow_rpmhigh = Torque_Map_Data.map[pedalLowIndex][rpmHighIndex];
		int16_t torque_pedalhigh_rpmlow = Torque_Map_Data.map[pedalHighIndex][rpmLowIndex];
		int16_t torque_pedalhigh_rpmhigh = Torque_Map_Data.map[pedalHighIndex][rpmHighIndex];

		// Interpolating across rpm values
		int16_t torque_pedallow = interpolate(500, torque_pedallow_rpmhigh - torque_pedallow_rpmlow, torque_pedallow_rpmlow, rpmOffset);
		int16_t torque_pedalhigh = interpolate(500, torque_pedalhigh_rpmhigh - torque_pedalhigh_rpmlow, torque_pedalhigh_rpmlow, rpmOffset);
		int16_t requested_torque = interpolate(1000, torque_pedalhigh - torque_pedallow, torque_pedallow, pedalOffset) / Torque_Map_Data.scaling_factor;


		// RULE (2024 V1): EV.3.3.3 No regen < 5km/h
		if (!Torque_Map_Data.regen_enabled || rpm < kmphToRPM(5)) {
			requested_torque = MAX(requested_torque, 0);
		}
		APPS_Data.torque = requested_torque;

		osDelayUntil(tick += APPS_PERIOD);
	}
}
