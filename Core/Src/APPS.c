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

Torque_Map_Struct Torque_Map_Data = {
		.max_torque = 400,
		.max_power = 700,
		.max_torque_scaling_factor = 1000,
		.max_power_scaling_factor = 1000,
		.target_speed_limit = 6000,
		.speed_limit_range = 500,
		.regen_enabled = 0
};

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
		DEBUG_PRINT("APPS1:%d, APPS1 ADC: %d, APPS2:%d, APPS2 ADC: %d\r\n", apps1_pos, apps1_adc_avg, apps2_pos, apps2_adc_avg);

		// TODO: T.4.2.5
		// TODO: T.4.3.3
		// TODO: T.4.3.4

		// RULE (2024 V1): T.4.2.4 (Both APPS sensor positions must be within 10% of pedal travel of each other)
		if (ABS(apps1_pos - apps2_pos) <= 100) {
			CLEAR_FLAG(APPS_Data.flags, APPS_SENSOR_CONFLICT_INVALID);
		} else {
			SET_FLAG(APPS_Data.flags, APPS_SENSOR_CONFLICT_INVALID);
		}

		if (!FLAG_ACTIVE(APPS_Data.flags, APPS_SENSOR_OUT_OF_RANGE_INVALID) && !FLAG_ACTIVE(APPS_Data.flags, APPS_SENSOR_CONFLICT_INVALID)) {
			APPS_Data.apps_position = CLAMP(0, (apps1_pos + apps2_pos) / 2, 1000);

		} else {
			APPS_Data.apps_position = 0;
		}


		int32_t brake_pressure_adc_avg = 0;
		for (int i = 0; i < ADC_CHANNEL_3_DMA_BUFFER_LEN; i+=3) {
			brake_pressure_adc_avg += adc_3_dma_buffer[i];
		}
		brake_pressure_adc_avg /= (ADC_CHANNEL_3_DMA_BUFFER_LEN/ADC_CHANNEL_3_DMA_CHANNELS);

		if (brake_pressure_adc_avg <= ADC_SHORTED_GND || ADC_SHORTED_VCC <= brake_pressure_adc_avg) {
			SET_FLAG(APPS_Data.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
			APPS_Data.brake_pressure = 0;
		} else {
			CLEAR_FLAG(APPS_Data.flags, BRAKE_SENSOR_OUT_OF_RANGE_INVALID);
			APPS_Data.brake_pressure = CLAMP(0, (brake_pressure_adc_avg - BRAKE_PRESSURE_MIN) * 20000 /(BRAKE_PRESSURE_MAX - BRAKE_PRESSURE_MIN), 20000);
		}
//		CRITICAL_PRINT("BRAKE_PRESSURE:%d, BRAKE_PRESSURE ADC: %d\r\n", APPS_Data.brake_pressure, brake_pressure_adc_avg);

		// RULE (2024 V1): T.4.7 (BSPC)
		if (FLAG_ACTIVE(APPS_Data.flags, APPS_BSPC_INVALID)){
			// Check if the pedal position is <5% to put APPS back into a valid state (EV.4.7.2)
			if (APPS_Data.apps_position < 50) {
				CLEAR_FLAG(APPS_Data.flags, APPS_BSPC_INVALID);
			}
		} else if (APPS_Data.apps_position > 250 && APPS_Data.brake_pressure > 500) {
			// Set to invalid if over >25% travel and brakes engaged (EV.4.7.1)
			SET_FLAG(APPS_Data.flags, APPS_BSPC_INVALID);
		}

		int32_t max_torque = (int32_t)Torque_Map_Data.max_torque * Torque_Map_Data.max_torque_scaling_factor / 1000;
		int16_t rpm = Ctrl_Data.motor_speed;


		int16_t requested_torque = max_torque;
		// Max torque as calculated from max_power
		if (rpm != 0) {
			int32_t max_power_watts = (int32_t)Torque_Map_Data.max_power * 1000 * Torque_Map_Data.max_power_scaling_factor / 1000;
			int16_t max_torque_from_power = max_power_watts * 95492 / rpm / 10000;
			requested_torque = MIN(max_torque, max_torque_from_power);
		}

		// Scale based on pedal position
		requested_torque = requested_torque * APPS_Data.apps_position / 1000;

		int16_t low_speed_cutoff = Torque_Map_Data.target_speed_limit - Torque_Map_Data.speed_limit_range / 2;
		int16_t high_speed_cutoff = Torque_Map_Data.target_speed_limit + Torque_Map_Data.speed_limit_range / 2;

		if (low_speed_cutoff <= rpm) {
			requested_torque = requested_torque * (high_speed_cutoff - rpm) / Torque_Map_Data.speed_limit_range;
		}

		// RULE (2024 V1): EV.3.3.3 No regen < 5km/h
		if (!Torque_Map_Data.regen_enabled || rpm < kmphToRPM(5)) {
			requested_torque = MAX(requested_torque, 0);
		}
		DEBUG_PRINT("Requested Torque: %d\r\n", requested_torque);
		APPS_Data.torque = requested_torque;


		osDelayUntil(tick += APPS_PERIOD);
	}
}
