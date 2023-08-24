#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h> // EBADF
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO

// SWO IS UNAVAILABLE ON CHEAP CHINESE STLINKS:
// https://embedblog.eu/?p=673
// It's possible to mod them to enable it. (Dallas has successfully done this)

// Low-level write handler for our printf
int _write(int file, char *data, int len) {
	// Only allow stdout and stderr
	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
		errno = EBADF;
		return -1;
	}

	// Sends over SWO
	for (int idx = 0; idx < len; idx++) {
		ITM_SendChar(*data++);
	}
	return len;

	// If we ever switch back to UART - This is the code for that
	//	// Transmit in blocking mode over UART1 with an arbitrary timeout of 10ms
	//	HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t*) data, len, 10);
	// Return # of bytes written - as best we can tell
	//	return (status == HAL_OK ? len : 0)
}

// Semaphore ID for custom printf
extern osSemaphoreId_t GRCprintfSemaHandle;

// Custom printf for transmitting over UART1 (STLink connects it over USB to STMCube)
int GRCprintf(const char *format, ...) {
	// Block printf from executing concurrently
	osSemaphoreAcquire(GRCprintfSemaHandle, osWaitForever);

	// Standard printf execution
	va_list alist;
	va_start(alist, format);
	int ret = vprintf(format, alist);
	va_end(alist);

	osSemaphoreRelease(GRCprintfSemaHandle);
	return ret;
}
