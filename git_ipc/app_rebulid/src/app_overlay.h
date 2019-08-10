
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "overlay.h"

#ifndef APP_OVERLAY_H_
#define APP_OVERLAY_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int APP_OVERLAY_create_title(int vin);
extern void APP_OVERLAY_release_title(int vin);

extern int APP_OVERLAY_create_clock(int vin);
extern void APP_OVERLAY_release_clock(int vin);

extern int APP_OVERLAY_create_id(int vin);
extern void APP_OVERLAY_release_id(int vin);

extern void APP_OVERLAY_reload_title(int vin);
extern void APP_OVERLAY_reload_clock(int vin);
extern void APP_OVERLAY_reload_id(int vin);

extern int APP_OVERLAY_create_pic(int vin);
extern int APP_OVERLAY_release_pic(int vin);

extern int APP_OVERLAY_create_caution(int vin);
extern void APP_OVERLAY_release_caution(int vin);

extern int APP_OVERLAY_init();
extern void APP_OVERLAY_destroy();

extern int  APP_OVERLAY_table_info_add(int  line,int cow, char* font_info);
extern int  APP_OVERLAY_table_info_update();
extern int  APP_OVERLAY_table_info_clear();

#ifdef __cplusplus
};
#endif
#endif //APP_OVERLAY_H_

