/*
 * utils.c
 *	Shared utilities
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt
 */

#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h> // EBADF
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO

// UART Handle for _write
extern UART_HandleTypeDef huart3;

// Low-level write handler for our printf
int _write(int file, char *data, int len) {
	// Only allow stdout and stderr
	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
		errno = EBADF;
		return -1;
	}

	// Transmit in blocking mode over UART3 with an arbitrary timeout of 10ms
	HAL_StatusTypeDef status = HAL_UART_Transmit(&huart3, (uint8_t*) data, len, 10);

	// Return # of bytes written - as best we can tell
	return (status == HAL_OK ? len : 0);
}

// Semaphore ID for custom printf
extern osSemaphoreId_t printSemHandle;

// Custom printf for transmitting over UART3 (STLink connects it over USB to STMCube)
int GRCprintf(const char *format, ...) {
	// Block printf from executing concurrently
	osSemaphoreAcquire(printSemHandle, osWaitForever);

	// Standard printf execution
	va_list alist;
	va_start(alist, format);
	int ret = vprintf(format, alist);
	va_end(alist);

	osSemaphoreRelease(printSemHandle);
	return ret;
}


int32_t kmphToRPM(int32_t kmph) {
	return kmph * 4500/99/10;
}
