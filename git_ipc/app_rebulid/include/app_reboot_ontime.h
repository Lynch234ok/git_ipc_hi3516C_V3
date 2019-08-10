#ifdef REBOOT_ONTIME
#ifndef _REBOOT_ONTIME_H
#define _REBOOT_ONTIME_H
#include "reboot_ontime.h"
#ifdef __cplusplus
extern "C" {
#endif

extern NK_Int REBOOT_ONTIME_init(NK_Int hourNum);
extern NK_Void REBOOT_ONTIME_destroy();
extern NK_Boolean REBOOT_ONTIME_is_flag_exist();

#ifdef __cplusplus
};
#endif
#endif/*_REBOOT_ONTIME_H*/
#endif
