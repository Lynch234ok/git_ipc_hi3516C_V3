
#ifndef __HI_ISP_API_H__
#define __HI_ISP_API_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "sdk/sdk_isp_def.h"
#include "sdk/sdk_api_def.h"

extern emSENSOR_MODEL hi_isp_api_get_sensor_model();
extern void hi_isp_api_mirror_flip(unsigned char mode);
extern void hi_isp_api_test_mode(unsigned char enable);
extern void hi_isp_api_AWB(unsigned char mode);
extern void hi_isp_api_light_mode(unsigned char mode);
extern void hi_isp_api_set_hue(unsigned short val);
extern void hi_isp_api_set_saturation(unsigned char val);
extern unsigned char  hi_isp_api_get_saturation();
extern void hi_isp_api_set_brightness(unsigned char val);
extern void hi_isp_api_set_contrast(unsigned char val);
extern void hi_isp_api_set_exposure(unsigned char val);
extern void hi_isp_api_set_sharpness(unsigned char val);
extern void hi_isp_api_color_mode(unsigned char mode);
extern uint16_t hi_isp_api_reg_read(uint16_t addr);
extern void hi_isp_api_reg_write(uint16_t addr,uint16_t val);
extern uint16_t hi_isp_api_spec_reg_read(unsigned char page, uint16_t addr);
extern void hi_isp_api_spec_reg_write(unsigned char page, uint16_t addr,uint16_t val);
extern stSensorColorMaxValue hi_isp_api_get_color_max_value();
extern void hi_isp_api_set_shutter(unsigned char val);
extern void hi_isp_api_ircut_auto_switch(uint8_t type, uint8_t bEnable);
extern void hi_isp_api_vi_flicker(uint8_t bEnable,uint8_t frequency, uint8_t mode);
extern uint8_t hi_isp_api_get_sharpen(void);
extern void hi_isp_api_set_sharpen(uint8_t val, uint8_t bManual);
extern void hi_isp_api_set_scene_mode(uint32_t mode);
extern void hi_isp_api_set_WB_mode(uint32_t mode);
extern void hi_isp_api_set_ircut_control_mode(uint32_t mode);
extern void hi_isp_api_set_ircut_mode(uint32_t mode);
extern void hi_isp_api_set_WDR_enable(uint8_t bEnable);
extern void hi_isp_api_set_WDR_strength(uint8_t val);
extern void hi_isp_api_set_exposure_mode(uint32_t mode);
extern void hi_isp_api_set_AEcompensation(uint8_t val);
extern void hi_isp_api_set_denoise_enable(uint8_t bEnable);
extern void hi_isp_api_set_denoise_strength(uint8_t val);
extern uint8_t hi_isp_api_get_denoise_strength(void);
extern void hi_isp_api_set_anti_fog_enable(uint8_t bEnable);
extern void hi_isp_api_set_lowlight_enable(uint8_t bEnable);
extern void hi_isp_api_set_gamma_table(uint8_t val);
extern void hi_isp_api_set_defect_pixel_enable(uint8_t bEnable);
extern void hi_isp_api_set_src_framerate(uint32_t val);
extern void hi_isp_api_set_sensor_resolution(uint32_t width, uint32_t height);
extern void hi_isp_api_get_sensor_resolution(uint32_t* ret_width, uint32_t* ret_height);
extern uint32_t hi_isp_api_get_gain(void);
extern void hi_isp_api_set_af_attr(stSensorAfAttr *pAfAttr);
extern int hi_isp_api_ini_load(const char *filename);
extern int hi_isp_api_get_wdr_mode (uint8_t *bEnable); 
extern int hi_isp_api_set_ircut_switch_array(float *array);




#endif //__HI_ISP_API_H__

