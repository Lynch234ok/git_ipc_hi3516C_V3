#ifndef __ONVIF_SPOOK_H__
#define __ONVIF_SPOOK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "spook.h"

extern SPOOK_SESSION_PROBE_t ONVIF_nvt_probe(const void* msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t ONVIF_nvt_loop(bool* trigger, int sock, time_t* read_pts);

#ifdef __cplusplus
}
#endif
#endif

