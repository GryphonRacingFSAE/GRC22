#include "TEMInterface.h"
#include "utils.h"
#include "main.h"

void startTEMInterfaceTask() {
	uint32_t tick = osKernelGetTickCount();
	while (1) {
		HAL_GPIO_TogglePin(ADDR1_GPIO_Port, ADDR1_Pin);
		osDelayUntil(tick += TEM_PERIOD);
	}
}
