/*

#ifndef _DEBUG_H
#define _DEBUG_H 1

#ifdef _DEBUG

#define dbg(format, ...)    \
    do {fprintf(stdout, format, ##__VA_ARGS__); } while(0)//没有;
#else
#define dbg(format, ...)
#endif
#ifdef _ANDROID
#include "android_log.h"
#define dbg		LOG
#endif

#endif
*/


#ifndef _DANA_DEBUG_H_
#define _DENA_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

void dbg_on();
void dbg_off();

void dbg(const char *msg, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _DANA_DEBUG_H_ */
