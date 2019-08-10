#ifndef __DANA_LIB_H__
#define __DANA_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include<sys/time.h>
#include <time.h>
#include<unistd.h>


extern void DanaLib_init();
extern void DanaLib_destroy();
extern void Dana_notify_event();
extern char* Dana_get_device_id();
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif