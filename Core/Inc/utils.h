#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "main.h"
#include "cmsis_os.h"

#define MAX(a,b) ((a)>(b) ? (a):(b))
#define MIN(a,b) ((a)<(b) ? (a):(b))
// b is the variable to be clamped, a is the minimum, and c is the maximum
#define CLAMP(a, b, c) (MIN((c), MAX((b), (a))))
#define ABS(a) ((a)<0 ? ((a)*-1):(a))


#define SET_FLAG(a, flag) ((a) |= (flag))
#define CLEAR_FLAG(a, flag) ((a) &= ~(flag))
#define FLAG_ACTIVE(a, flag) ((a) & (flag))

// 1:1 RPM and 10:1 kmph
int32_t kmphToRPM(int32_t);

int GRCprintf(const char *format, ...);

#define DEBUG_PRINT_LEVEL 5

#if DEBUG_PRINT_LEVEL == 1
	#define TRACE_PRINT(...)
	#define DEBUG_PRINT(...)
	#define WARNING_PRINT(...)
	#define ERROR_PRINT(...)
	#define CRITICAL_PRINT(...) GRCprintf(__VA_ARGS__)
#elif DEBUG_PRINT_LEVEL == 2
	#define TRACE_PRINT(...)
	#define DEBUG_PRINT(...)
	#define WARNING_PRINT(...)
	#define ERROR_PRINT(...) GRCprintf(__VA_ARGS__)
	#define CRITICAL_PRINT(...) GRCprintf(__VA_ARGS__)
#elif DEBUG_PRINT_LEVEL == 3
	#define TRACE_PRINT(...)
	#define DEBUG_PRINT(...)
	#define WARNING_PRINT(...) GRCprintf(__VA_ARGS__)
	#define ERROR_PRINT(...) GRCprintf(__VA_ARGS__)
	#define CRITICAL_PRINT(...) GRCprintf(__VA_ARGS__)
#elif DEBUG_PRINT_LEVEL == 4
	#define TRACE_PRINT(...)
	#define DEBUG_PRINT(...) GRCprintf(__VA_ARGS__)
	#define WARNING_PRINT(...) GRCprintf(__VA_ARGS__)
	#define ERROR_PRINT(...) GRCprintf(__VA_ARGS__)
	#define CRITICAL_PRINT(...) GRCprintf(__VA_ARGS__)
#elif DEBUG_PRINT_LEVEL == 5
	#define TRACE_PRINT(...) GRCprintf(__VA_ARGS__)
	#define DEBUG_PRINT(...) GRCprintf(__VA_ARGS__)
	#define WARNING_PRINT(...) GRCprintf(__VA_ARGS__)
	#define ERROR_PRINT(...) GRCprintf(__VA_ARGS__)
	#define CRITICAL_PRINT(...) GRCprintf(__VA_ARGS__)
#endif

#endif /* INC_UTILS_H_ */
