
#include "hi_isp.h"
#include "sdk/sdk_isp_def.h"

#define ISP_TRUE (1)
#define ISP_FALSE (0)

emSENSOR_MODEL hi_isp_api_get_sensor_model(char *sensor_name)
{
	emSENSOR_MODEL ret_sensor_model;
	return HI_SDK_ISP_get_sensor_model(sensor_name);
}

void hi_isp_api_mirror_flip(unsigned char mode)
{
	switch(mode){
		case MODE_MIRROR:
			HI_SDK_ISP_set_mirror(0, ISP_TRUE);
			break;
		case MODE_UNMIRROR:
			HI_SDK_ISP_set_mirror(0, ISP_FALSE);
			break;
		case MODE_FLIP:
			HI_SDK_ISP_set_flip(0, ISP_TRUE);
			break;
		case MODE_UNFLIP:
			HI_SDK_ISP_set_flip(0, ISP_FALSE);
			break;
		default:
		case MODE_NORMAL:
			HI_SDK_ISP_set_flip(0, ISP_FALSE);
			HI_SDK_ISP_set_mirror(0, ISP_FALSE);
			break;
	}	
}
void hi_isp_api_test_mode(unsigned char enable){}
void hi_isp_api_light_mode(unsigned char mode){}
void hi_isp_api_set_hue(unsigned short val)
{
	HI_SDK_ISP_set_hue(0, val);
}

void hi_isp_api_set_saturation(unsigned char val)//val should be 0,1,2,3,4,5,6,7
{
	HI_SDK_ISP_set_saturation(0, val);
}
unsigned char  hi_isp_api_get_saturation()
{
#if 0
	unsigned char val;
	SDK_ISP_get_saturation(0, &val);
	return val;
#endif
	return 0;
}
void hi_isp_api_set_brightness(unsigned char val)
{
	HI_SDK_ISP_set_brightness(0, val);
}
void hi_isp_api_set_contrast(unsigned char val)
{
	HI_SDK_ISP_set_contrast(0, val);
}

void hi_isp_api_color_mode(unsigned char mode)
{

}

void hi_isp_api_reg_write(uint16_t addr,uint16_t val)
{

}
uint16_t hi_isp_api_reg_read(uint16_t addr)
{
	return 0;
}

void hi_isp_api_spec_reg_write(uint8_t page, uint16_t addr,uint16_t val)
{

}
uint16_t hi_isp_api_spec_reg_read(uint8_t page, uint16_t addr)
{
	return 0;
}


stSensorColorMaxValue hi_isp_api_get_color_max_value()
{
	stSensorColorMaxValue ret_value;
	HI_SDK_ISP_get_color_max_value(&ret_value);
	return ret_value;
}

void hi_isp_api_set_shutter(unsigned char val)
{
	switch(val)
	{
		default:
		case 50:
			HI_SDK_ISP_set_src_framerate(25);
			//SDK_ISP_sensor_flicker(1, 50);
			break;
		case 60:
			//SDK_ISP_sensor_flicker(1, 60);
			HI_SDK_ISP_set_src_framerate(30);
			break;
	}
}

void hi_isp_api_ircut_auto_switch(uint8_t type, uint8_t bEnable)//1:software   0: hardware
{
	if(bEnable){
		HI_SDK_ISP_ircut_auto_switch(0, type);
	}
}

void hi_isp_api_vi_flicker(uint8_t bEnable,uint8_t frequency, uint8_t mode)
{
	HI_SDK_ISP_sensor_flicker(bEnable, frequency, mode);
}

uint8_t hi_isp_api_get_sharpen(void)
{
	uint8_t ret_val = 0;
	HI_SDK_ISP_get_sharpen(&ret_val);
	return ret_val;
}

void hi_isp_api_set_sharpen(uint8_t val, uint8_t bManual)
{
	HI_SDK_ISP_set_sharpen(val, bManual);
}

void hi_isp_api_set_scene_mode(uint32_t mode)
{
	HI_SDK_ISP_set_scene_mode(mode);
}

void hi_isp_api_set_WB_mode(uint32_t mode)
{
	HI_SDK_ISP_set_WB_mode(mode);
}

void hi_isp_api_get_starlight_mode(bool *enable)
{
	HI_SDK_ISP_get_starlight_mode(enable);
}

uint8_t hi_isp_api_get_colortoblack_range(void)
{
	uint8_t ret_val;
	HI_SDK_ISP_get_colortoblack_range(&ret_val);
	return ret_val;
}

void hi_isp_api_set_colortoblack_range(uint8_t colortoblackrange)
{
	HI_SDK_ISP_set_colortoblack_range(colortoblackrange);
}


void hi_isp_api_set_ircut_control_mode(uint32_t mode)
{
	HI_SDK_ISP_set_ircut_control_mode(mode);
}

void hi_isp_api_set_ircut_mode(uint32_t mode)
{
	HI_SDK_ISP_set_ircut_mode(mode);
}

void hi_isp_api_set_WDR_enable(uint8_t bEnable)
{
	HI_SDK_ISP_set_WDR_enable(bEnable);
}

void hi_isp_api_set_WDR_strength(uint8_t val)
{
	HI_SDK_ISP_set_WDR_strength(val);
}

void hi_isp_api_set_exposure_mode(uint32_t mode)
{
//	HI_SDK_ISP_set_exposure_mode(mode);
}

void hi_isp_api_set_AEcompensation(uint8_t val)
{
//	HI_SDK_ISP_set_AEcompensation(val);
}

void hi_isp_api_set_denoise_enable(uint8_t bEnable)
{
//	HI_SDK_ISP_set_denoise_enable(bEnable);
}

uint8_t hi_isp_api_get_denoise_strength(void)
{
	uint8_t ret_val;
	HI_SDK_ISP_get_denoise_strength(&ret_val);
	return ret_val;
}

void hi_isp_api_set_denoise_strength(uint8_t val)
{
	HI_SDK_ISP_set_denoise_strength(val);
}

void hi_isp_api_set_anti_fog_enable(uint8_t bEnable)
{
	HI_SDK_ISP_set_advance_anti_fog_enable(bEnable);
}

void hi_isp_api_set_lowlight_enable(uint8_t bEnable)
{
	HI_SDK_ISP_set_advance_lowlight_enable(bEnable);
}

void hi_isp_api_set_gamma_table(uint8_t val)
{
	HI_SDK_ISP_set_advance_gamma_table(val);
}

void hi_isp_api_set_defect_pixel_enable(uint8_t bEnable)
{
	HI_SDK_ISP_set_advance_defect_pixel_enable(bEnable);
}

void hi_isp_api_set_src_framerate(uint32_t val)
{
	HI_SDK_ISP_set_src_framerate(val);
}

void hi_isp_api_set_sensor_resolution(uint32_t width, uint32_t height)
{
	HI_SDK_ISP_set_sensor_resolution(width,height);
}

void hi_isp_api_get_sensor_resolution(uint32_t* ret_width, uint32_t* ret_height)
{
	HI_SDK_ISP_get_sensor_resolution(ret_width, ret_height);
}

uint8_t hi_isp_api_get_gain(void)
{
	return HI_SDK_ISP_get_gain();
}

void hi_isp_api_set_af_attr(stSensorAfAttr *pAfAttr)
{
	return HI_SDK_ISP_set_AF_attr(pAfAttr);
}

int hi_isp_api_ini_load(const char *filepath)
{
	return HI_SDK_ISP_ini_load(filepath);
}

void hi_isp_api_get_wdr_mode(uint8_t *bEnable)
{
	 HI_SDK_ISP_get_wdr_mode(bEnable);
}

int hi_isp_api_get_cur_fps()
{
    return HI_SDK_ISP_get_cur_fps();

}

void hi_isp_api_get_daynight_mode(uint8_t *daynight_mode)
{
	*daynight_mode =  (uint8_t)HI_SDK_ISP_get_daynight_mode();
}


int SDK_ISP_init(lpSensorApi *api, lpBSPApi bsp_api){
	HI_SDK_ISP_init(api, bsp_api);
	return 0;
}

int SDK_ISP_destroy()
{
	HI_SDK_ISP_destroy();
	return 0;
}

emSENSOR_MODEL SDK_ISP_sensor_check()
{
	return HI_SDK_ISP_sensor_check();
}
