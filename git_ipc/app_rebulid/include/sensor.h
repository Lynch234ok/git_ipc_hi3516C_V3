
#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>

#include "sdk/sdk_isp_def.h"


extern int SENSOR_init(const char *soc, lpBSPApi *api);
extern void SENSOR_destroy();
extern int SENSOR_set_conf();

extern void SENSOR_mirror_flip(uint8_t mode);
extern void SENSOR_hue_set(uint16_t val);
extern void SENSOR_contrast_set(uint8_t val);
extern void SENSOR_brightness_set(uint8_t val);
extern void SENSOR_saturation_set(uint8_t val);
extern void SENSOR_lightmode_set(uint8_t mode);
extern void SENSOR_test_mode(uint8_t enable);
extern void SENSOR_color_mode(uint8_t mode);
extern void SENSOR_reg_write(uint16_t addr,uint16_t val);
extern uint16_t SENSOR_reg_read(uint8_t addr);
extern void SENSOR_spec_reg_write(uint8_t page, uint16_t addr, uint16_t val);
extern uint16_t SENSOR_spec_reg_read(uint8_t page,uint16_t addr);
extern stSensorColorMaxValue SENSOR_get_color_max_value();
extern void SENSOR_shutter_set(uint8_t val);
extern void SENSOR_set_sysconf();
extern void SENSOR_ircut_auto_switch(uint8_t type, uint8_t bEnable);
extern void SENSOR_vi_flicker(uint8_t bEnable, uint8_t frequency, uint8_t mode);
extern void SENSOR_sharpen_set(uint8_t val, uint8_t bManual);
extern void SENSOR_scene_mode_set(uint32_t mode);
extern void SENSOR_WB_mode_set(uint32_t mode);
extern void SENSOR_ircut_control_mode_set(uint32_t mode);
extern void SENSOR_ircut_mode_set(uint32_t mode);
extern void SENSOR_WDR_mode_enable(uint8_t bEnable);
extern void SENSOR_WDR_strength_set(uint8_t val);
extern void SENSOR_exposure_mode_set(uint32_t mode);
extern void SENSOR_AEcompensation_set(uint8_t val);
extern void SENSOR_denoise_3d_enable(uint8_t bEnable);
extern void SENSOR_denoise_3d_strength(uint8_t val);
extern void SENSOR_anti_fog_enable(uint8_t bEnable);
extern void SENSOR_lowlight_enable(uint8_t bEnable);
extern void SENSOR_gamma_table_set(uint8_t val);
extern void SENSOR_defect_pixel_enable(uint8_t bEnable);
extern void SENSOR_set_src_framerate(uint32_t val);
extern uint32_t SENSOR_get_gain();
extern int SENSOR_cfg_load(const char *filepath);

extern void SENSOR_get_resolution(uint32_t* ret_width, uint32_t* ret_height);
extern void SENSOR_set_af_attr(stSensorAfAttr *afAttr);
extern emSENSOR_MODEL SENSOR_get_sensor_model(char * sensor_name);
extern int SENSOR_GET_CUR_FPS();

extern uint8_t  SENSOR_DAYNIGHT_mode_get();




#endif //__SENSOR_H__

