/*
 * utils.h
 *
 *  Created on: Jan 19, 2023
 *      Author: Matt
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "main.h"
#include "cmsis_os.h"

#define MAX(a,b) (a>b ? a:b)
#define MIN(a,b) (a<b ? a:b)
#define ABS(a) ((a)<0 ? ((a)*-1):a)

int myprintf(const char *format, ...);

#endif /* INC_UTILS_H_ */
