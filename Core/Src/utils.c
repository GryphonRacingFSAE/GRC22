/*
 * utils.c
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt
 */

#include "utils.h"
#include  <stdio.h>
#include  <stdarg.h>
#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO

extern UART_HandleTypeDef huart3;
extern osSemaphoreId_t printSemHandle;

int _write(int file, char *data, int len)
{
   if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
   {
      errno = EBADF;
      return -1;
   }

   // arbitrary timeout 1000
   HAL_StatusTypeDef status =
      HAL_UART_Transmit(&huart3, (uint8_t*)data, len, 1000);

   // return # of bytes written - as best we can tell
   return (status == HAL_OK ? len : 0);
}

int myprintf(const char *format, ...)
{

	osSemaphoreAcquire(printSemHandle, osWaitForever);
	va_list alist;
	va_start(alist, format);
	vprintf(format, alist);
    va_end(alist);
	osSemaphoreRelease(printSemHandle);
	return 0;
}
