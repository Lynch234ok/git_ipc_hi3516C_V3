
#ifndef __SDK_ISP_H__
#define __SDK_ISP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
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

#include "sdk/sdk_isp_def.h"


extern int HI_SDK_ISP_set_mirror(int vin, bool mirror);
extern int HI_SDK_ISP_set_flip(int vin, bool flip);
extern int HI_SDK_ISP_ircut_auto_switch(int vin, uint8_t type);
extern int HI_SDK_ISP_set_saturation(int vin, uint16_t val);
extern int HI_SDK_ISP_get_saturation(int vin, uint16_t *val);
extern int HI_SDK_ISP_set_contrast(int vin, uint16_t val);
extern int HI_SDK_ISP_set_hue(int vin, uint16_t val);
extern int HI_SDK_ISP_set_brightness(int vin, uint16_t val);
extern int HI_SDK_ISP_sensor_flicker(uint8_t bEnable, uint8_t frequency, uint8_t mode);
extern int HI_SDK_ISP_set_src_framerate(unsigned int framerate);
extern int HI_SDK_ISP_set_sharpen(uint8_t val, uint8_t bManual);
extern int HI_SDK_ISP_get_sharpen(uint8_t *val);
extern int HI_SDK_ISP_set_scene_mode(uint32_t mode);
extern int HI_SDK_ISP_set_WB_mode(uint32_t mode);
extern int HI_SDK_ISP_set_ircut_control_mode(uint32_t mode);
extern int HI_SDK_ISP_set_ircut_mode(uint32_t mode);
extern int HI_SDK_ISP_set_WDR_enable(uint8_t bEnable);
extern int HI_SDK_ISP_set_WDR_strength(uint8_t val);
extern int HI_SDK_ISP_set_exposure_mode(uint32_t mode);
extern int HI_SDK_ISP_set_AEcompensation(uint8_t val);
extern int HI_SDK_ISP_set_denoise_enable(uint8_t bEnable);
extern int HI_SDK_ISP_set_denoise_strength(uint8_t val);
extern int HI_SDK_ISP_get_denoise_strength(uint8_t *val);
extern int HI_SDK_ISP_set_advance_anti_fog_enable(uint8_t bEnable);
extern int HI_SDK_ISP_set_advance_lowlight_enable(uint8_t bEnable);
extern int HI_SDK_ISP_set_advance_gamma_table(uint8_t val);
extern int HI_SDK_ISP_set_advance_defect_pixel_enable(uint8_t bEnable);
extern int HI_SDK_ISP_get_color_max_value(stSensorColorMaxValue *ret_value);
extern emSENSOR_MODEL HI_SDK_ISP_get_sensor_model(char *sensor_name);
extern int HI_SDK_ISP_set_isp_sensor_value(void);
extern int HI_SDK_ISP_set_sensor_resolution(uint32_t width,uint32_t height);
extern int HI_SDK_ISP_get_sensor_resolution(uint32_t *ret_width, uint32_t *ret_height);
extern int HI_SDK_ISP_init(lpSensorApi*api, lpBSPApi *bsp_api);
extern int HI_SDK_ISP_destroy();
extern int HI_SDK_ISP_ini_load(const char *filepath);
extern int HI_SDK_ISP_get_wdr_mode(uint8_t *bEnable);
extern int HI_SDK_ISP_get_cur_fps();

#ifdef __cplusplus
};
#endif
#endif //__SDK_ISP_H__

