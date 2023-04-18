#include "ThermistorMonitor.h"
#include "utils.h"
#include "main.h"

ThermistorData_Struct ThermistorData = {};

void startThermistorMonitorTask() {
	uint32_t tick = osKernelGetTickCount();
	while (1) {
		HAL_GPIO_TogglePin(ADDR0_GPIO_Port, ADDR0_Pin);
		osDelayUntil(tick += THERMISTOR_PERIOD);
	}
}
