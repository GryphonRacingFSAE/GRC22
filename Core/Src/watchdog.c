/*
 * APPS.c
 *
 *  Created on: Apr 10, 2023
 *      Author: Dallas
 */

#include "watchdog.h"
#include "utils.h"
#include "CAN.h"
#include "cmsis_os2.h"
#include "main.h"

Watchdog_Data_Struct Watchdog_Data = {
	.flags = BSPC_INVALID | RTD_INVALID, // This must contain all errors that can be reset
	.tick_stamp = {},
	.tick_limit = {}
};

void startWatchdogTask() {
	// NOTE: tick will overflow every 49 days, much more then is important to us
	uint32_t tick = osKernelGetTickCount();

	while (1) {
		// Watch RTD, BSPC, T.4.2.5, T.4.3.3 and disable motor controller after the "delay time", and send notification message over CAN

		uint32_t flags = Watchdog_Data.flags;
//		uint32_t flags2 = 0;
		while (flags) {
			// Safe as flags is never 0 here
			uint32_t flag_index = 31 - __builtin_clz(flags);

			if ((tick - Watchdog_Data.tick_stamp[flag_index]) > Watchdog_Data.tick_limit[flag_index]) {

			}

			// Remove the just checked flag
			flags &= ~(1 << flag_index);
		}

		osDelayUntil(tick += WATCHDOG_PERIOD);
	}
}
