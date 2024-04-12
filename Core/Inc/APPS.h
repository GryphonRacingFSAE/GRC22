#ifndef INC_APPS_H_
#define INC_APPS_H_

#include "main.h"
#include "cmsis_os.h"

#define APPS_DMA_CHANNELS (2)
#define APPS_DMA_BUFFER_LEN (4096 * APPS_DMA_CHANNELS)
#define ADC_CHANNEL_3_DMA_CHANNELS (3)
#define ADC_CHANNEL_3_DMA_BUFFER_LEN (4096 * ADC_CHANNEL_3_DMA_CHANNELS)
#define APPS1_MIN 1475
#define APPS1_MAX 1375
#define APPS2_MIN 2988
#define APPS2_MAX 2790
#define BRAKE_PRESSURE_MIN 410
#define BRAKE_PRESSURE_MAX 3686
#define ADC_SHORTED_GND 200
#define ADC_SHORTED_VCC 3800

// DMA variables for APPS and brake pressure
extern volatile uint16_t apps_dma_buffer[APPS_DMA_BUFFER_LEN];
extern volatile uint16_t adc_3_dma_buffer[ADC_CHANNEL_3_DMA_BUFFER_LEN];

#define APPS_BSPC_INVALID 0x1
// RULE (2023 V2): T.4.2.10 Sensor out of defined range
#define APPS_SENSOR_OUT_OF_RANGE_INVALID 0x2
// RULE (2023 V2): T.4.2.4 APPS signals are within 10% of pedal position from each other
#define APPS_SENSOR_CONFLICT_INVALID 0x4
// RULE (2023 V2): T.4.3.4 BSE sensor out of defined range
#define BRAKE_SENSOR_OUT_OF_RANGE_INVALID 0x8

typedef struct {
	uint16_t torque;
	uint32_t flags;
	uint16_t apps_position; // 10:1 value for apps position (%)
	uint16_t brake_pressure; // 10:1 value for brake pressure (PSI)
} APPS_Data_Struct;

extern APPS_Data_Struct APPS_Data;

void startAPPSTask();

#define TORQUE_MAP_ROWS 11
#define TORQUE_MAP_COLUMNS 14

typedef struct {
	int16_t map[TORQUE_MAP_ROWS][TORQUE_MAP_COLUMNS]; // Base torque map stored in 10 : 1 values
	uint8_t scaling_factor; // Additionally scales power to reduce power output
	uint8_t regen_enabled;
} Torque_Map_Struct;

extern Torque_Map_Struct Torque_Map_Data;

#endif /* INC_APPS_H_ */
