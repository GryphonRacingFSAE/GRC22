#pragma once

#define MAX(a,b) ((a)>(b) ? (a):(b))
#define MIN(a,b) ((a)<(b) ? (a):(b))
// b is the variable to be clamped, a is the minimum, and c is the maximum
#define CLAMP(a, b, c) (MIN((c), MAX((b), (a))))
#define ABS(a) ((a)<0 ? ((a)*-1):(a))


#define SET_FLAG(a, flag) ((a) |= (flag))
#define CLEAR_FLAG(a, flag) ((a) &= ~(flag))
#define FLAG_ACTIVE(a, flag) ((a) & (flag))