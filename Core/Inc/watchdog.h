#ifndef INC_WATCHDOG_H_
#define INC_WATCHDOG_H_

#include <stdint.h>
#include "cmsis_os.h"


typedef struct {
	uint32_t flags;
	uint32_t tick_stamp[32];
	const uint32_t tick_limit[32];
} Watchdog_Data_Struct;

extern Watchdog_Data_Struct Watchdog_Data;
// This task watches the following rules, and shuts down motor controller if invalid
// RTD, BSPC, T.4.2.5, T.4.3.3
void startWatchdogTask();

#endif /* INC_WATCHDOG_H_ */
