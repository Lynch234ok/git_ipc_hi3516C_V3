
#ifndef __SDK_ISP_H__
#define __SDK_ISP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>

#include "sdk_isp_def.h"

extern int SDK_ISP_init(lpSensorApi *api, lpBSPApi *bsp_api);
extern int SDK_ISP_destroy();
extern emSENSOR_MODEL SDK_ISP_sensor_check();


#ifdef __cplusplus
};
#endif
#endif //__SDK_ISP_H__

