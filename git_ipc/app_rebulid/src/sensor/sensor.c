
#include "sysconf.h"
#include "sdk/sdk_isp_def.h"
#include "sdk/sdk_isp.h"
#include "pan_tilt.h"
#include <hi_type.h>
#include <pan_tilt.h>
#include <AutoFocus.h>
#include "bsp/bsp.h"

extern int netsdk_af_callback(int focusMetries1,int focusMetries2, int focusMetries3, int* param);

typedef struct _sensor_param
{
	stBSPApi bsp_api;
	lpSensorApi api;
	stSensorAttr attr;
}stSensorParam, *lpSensorParam;

static lpSensorParam stSENSOR = NULL;

static void _sensor_set_sysconf(SYSCONF_t *sysconf)
{
	stSENSOR->attr.color_max_value = stSENSOR->api->GET_COLOR_MAX_VALUE();
	//SYSCONF_t *sysconf = SYSCONF_dup();
	sysconf->ipcam.isp.image_attr.brightness.max = stSENSOR->attr.color_max_value.BrightnessMax;
	sysconf->ipcam.isp.image_attr.contrast.max = stSENSOR->attr.color_max_value.ContrastMax;
	sysconf->ipcam.isp.image_attr.hue.max = stSENSOR->attr.color_max_value.HueMax;
	sysconf->ipcam.isp.image_attr.saturation.max = stSENSOR->attr.color_max_value.SaturationMax;
	
	sysconf->ipcam.isp.image_attr.brightness.max = stSENSOR->attr.color_max_value.BrightnessMax;
	sysconf->ipcam.isp.image_attr.contrast.max = stSENSOR->attr.color_max_value.ContrastMax;
	sysconf->ipcam.isp.image_attr.hue.max = stSENSOR->attr.color_max_value.HueMax;
	sysconf->ipcam.isp.image_attr.saturation.max = stSENSOR->attr.color_max_value.SaturationMax;
	//stSENSOR->api->SHUTTER_SET(sysconf->ipcam.vin[0].digital_shutter.val);
	/*stSENSOR->api->SATURATION_SET(sysconf->ipcam.isp.image_attr.saturation.val);
	stSENSOR->api->CONTRAST_SET(sysconf->ipcam.isp.image_attr.contrast.val);
	stSENSOR->api->HUE_SET(sysconf->ipcam.isp.image_attr.hue.val);
	stSENSOR->api->BRIGHTNESS_SET(sysconf->ipcam.isp.image_attr.brightness.val);
	//stSENSOR->api->VI_FLICKER(1, sysconf->ipcam.vin[0].digital_shutter.val == SYS_VIN_DIGITAL_SHUTTER_50HZ ? 50 : 60);
	if(sysconf->ipcam.isp.image_attr.flip){
		stSENSOR->api->MIRROR_FLIP_SET(MODE_FLIP);
	}else{
		stSENSOR->api->MIRROR_FLIP_SET(MODE_UNFLIP);
	}
	if(sysconf->ipcam.isp.image_attr.mirror){
		stSENSOR->api->MIRROR_FLIP_SET(MODE_MIRROR);
	}else{
		stSENSOR->api->MIRROR_FLIP_SET(MODE_UNMIRROR);
	}*/

	sysconf->ipcam.isp.image_attr.sharpen.val = stSENSOR->api->SHARPEN_GET();
	stSENSOR->api->SCENE_MODE_SET(sysconf->ipcam.isp.scene_mode);
	stSENSOR->api->WB_MODE_SET(sysconf->ipcam.isp.white_balance_mode);
	stSENSOR->api->IRCUT_CONTROL_MODE_SET(sysconf->ipcam.isp.day_night_mode.ircut_control_mode);
	stSENSOR->api->IRCUT_MODE_SET(sysconf->ipcam.isp.day_night_mode.ircut_mode);
	stSENSOR->api->WDR_MODE_ENABLE(sysconf->ipcam.isp.wide_dynamic_range.enable);
	stSENSOR->api->EXPOSURE_MODE_SET(sysconf->ipcam.isp.exposure.mode);
	stSENSOR->api->DENOISE_ENABLE(sysconf->ipcam.isp.denoise.denoise_enable);
	sysconf->ipcam.isp.denoise.denoise_strength.val = stSENSOR->api->DENOISE_STRENGTH_GET();
	stSENSOR->api->ANTI_FOG_ENABLE(sysconf->ipcam.isp.advance.anti_fog_enable);
	stSENSOR->api->LOWLIGHT_ENABLE(sysconf->ipcam.isp.advance.lowlight_enable);
	//stSENSOR->api->SRC_FRAMERATE_SET(sysconf->ipcam.vin[0].enc_h264[0].stream[0].fps);
	
}

//AF param structure
typedef struct _sensor_net_af_param
{
	int sock;
	HI_S32 fd;
	HI_S32 Cmd;
	HI_U32 u32Fv;
	void *pStepMotor;
	ISP_AF_INFO_S *pstAfInfo;
	int failed_cnt;
}stSensorNetAfParam, *LPSensorNetAfParam;

static int autofocus_status_af_init(int sock, bool set_flag)
{
	stSensorAfAttr afAttr;

	if(set_flag){//clear af callback function
		afAttr.param = NULL;
		afAttr.af_callback = NULL;
		SENSOR_set_af_attr(&afAttr);
	}else{//af callback function init
		afAttr.param = calloc(1, sizeof(stSensorNetAfParam));
		((LPSensorNetAfParam)afAttr.param)->sock = sock;
		((LPSensorNetAfParam)afAttr.param)->failed_cnt = 0;
#if defined(STEPER_AF)
		_PstepMotor = ((LPSensorNetAfParam)afAttr.param)->pStepMotor = PanTilt();
#endif
/*
#if defined(UART_PROTOCOL)
		extern int APP_UART_protocol_init(void);
		APP_UART_protocol_init();
#endif
*/
		afAttr.af_callback = netsdk_af_callback;
		SENSOR_set_af_attr(&afAttr);
	}

	return 0;
}

void SENSOR_set_sysconf()
{
	SYSCONF_t *sysconf = SYSCONF_dup();
//	stSENSOR->api->SHUTTER_SET(sysconf->ipcam.vin[0].digital_shutter.val);
	stSENSOR->api->SATURATION_SET(sysconf->ipcam.isp.image_attr.saturation.val);
	stSENSOR->api->CONTRAST_SET(sysconf->ipcam.isp.image_attr.contrast.val);
	stSENSOR->api->HUE_SET(sysconf->ipcam.isp.image_attr.hue.val);
	stSENSOR->api->BRIGHTNESS_SET(sysconf->ipcam.isp.image_attr.brightness.val);
}

void SENSOR_mirror_flip(uint8_t mode)
{
	if(stSENSOR){
		stSENSOR->api->MIRROR_FLIP_SET(mode);
	}else{
	}
}
void SENSOR_hue_set(uint16_t val)
{
	if(stSENSOR){
		stSENSOR->api->HUE_SET(val);
	}else{
	}
}
void SENSOR_contrast_set(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->CONTRAST_SET(val);
	}else{
	}
}
void SENSOR_brightness_set(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->BRIGHTNESS_SET(val);
	}else{
	}
}
void SENSOR_saturation_set(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->SATURATION_SET(val);
	}else{
	}
}

void SENSOR_lightmode_set(uint8_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->LIGHT_MODE_SET(mode);
	}else{
	}
}
void SENSOR_test_mode(uint8_t enable)
{	
	if(stSENSOR){
		stSENSOR->api->TEST_MODE_SET(enable);
	}else{
	}
}
void SENSOR_color_mode(uint8_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->COLOR_MODE_SET(mode);
	}else{
	}
}
void SENSOR_reg_write(uint16_t addr,uint16_t val)
{	
	if(stSENSOR){
		stSENSOR->api->REG_WRITE(addr,val);
	}else{
	}
}
uint16_t SENSOR_reg_read(uint16_t addr)
{	
	if(stSENSOR){
		return stSENSOR->api->REG_READ(addr);
	}else{
		return -1;
	}
}
void SENSOR_spec_reg_write(uint8_t page, uint16_t addr, uint16_t val)
{	
	if(stSENSOR){
		stSENSOR->api->SPEC_RED_WRITE(page, addr, val);
	}else{
	}
}
uint16_t SENSOR_spec_reg_read(uint8_t page,uint16_t addr)
{	
	if(stSENSOR){
		return stSENSOR->api->SPEC_REG_READ(page, addr);
	}else{
		return -1;
	}
}

stSensorColorMaxValue SENSOR_get_color_max_value()
{	
	if(stSENSOR){
		return stSENSOR->api->GET_COLOR_MAX_VALUE();
	}else{
		return ;
	}
}

void SENSOR_shutter_set(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->SHUTTER_SET(val);
	}else{
	}
}

void SENSOR_ircut_auto_switch(uint8_t type, uint8_t bEnable)//0:software   1: hardware
{	
	if(stSENSOR){
		stSENSOR->api->IRCUT_AUTO_SWITCH(type, bEnable);
	}else{
	}
}

void SENSOR_vi_flicker(uint8_t bEnable, uint8_t frequency, uint8_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->VI_FLICKER(bEnable, frequency, mode);
	}else{
	}
}

void SENSOR_sharpen_set(uint8_t val, uint8_t bManual)
{	
	if(stSENSOR){
		stSENSOR->api->SHARPEN_SET(val, bManual);
	}else{
	}
}

void SENSOR_scene_mode_set(uint32_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->SCENE_MODE_SET(mode);
	}else{
	}
}

void SENSOR_WB_mode_set(uint32_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->WB_MODE_SET(mode);
	}else{
	}
}

void SENSOR_ircut_control_mode_set(uint32_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->IRCUT_CONTROL_MODE_SET(mode);
	}else{
	}
}

void SENSOR_ircut_mode_set(uint32_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->IRCUT_MODE_SET(mode);
	}else{
	}
}

void SENSOR_WDR_mode_enable(uint8_t bEnable)
{	
	if(stSENSOR){
		stSENSOR->api->WDR_MODE_ENABLE(bEnable);
	}else{
	}
}

void SENSOR_WDR_strength_set(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->WDR_STRENGTH_SET(val);
	}else{
	}
}

void SENSOR_exposure_mode_set(uint32_t mode)
{	
	if(stSENSOR){
		stSENSOR->api->EXPOSURE_MODE_SET(mode);
	}else{
	}
}

void SENSOR_AEcompensation_set(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->AE_COMPENSATION_SET(val);
	}else{
	}
}

void SENSOR_denoise_3d_enable(uint8_t bEnable)
{	
	if(stSENSOR){
		stSENSOR->api->DENOISE_ENABLE(bEnable);
	}else{
	}
}

void SENSOR_denoise_3d_strength(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->DENOISE_STRENGTH_SET(val);
	}else{
	}
	
}

void SENSOR_anti_fog_enable(uint8_t bEnable)
{	
	if(stSENSOR){
		stSENSOR->api->ANTI_FOG_ENABLE(bEnable);
	}else{
	}
}

void SENSOR_lowlight_enable(uint8_t bEnable)
{	
	if(stSENSOR){
		stSENSOR->api->LOWLIGHT_ENABLE(bEnable);
	}else{
	}
}

void SENSOR_gamma_table_set(uint8_t val)
{	
	if(stSENSOR){
		stSENSOR->api->GAMMA_TABLE_SET(val);
	}else{
	}
}

void SENSOR_defect_pixel_enable(uint8_t bEnable)
{	
	if(stSENSOR){
		stSENSOR->api->DEFECT_PIXEL_ENABLE(bEnable);
	}else{
	}
}

void SENSOR_set_src_framerate(uint32_t val)
{	
	if(stSENSOR){
		stSENSOR->api->SRC_FRAMERATE_SET(val);
	}else{
	}
}

emSENSOR_MODEL SENSOR_get_sensor_model(char * sensor_name)
{	

	if(stSENSOR && stSENSOR->api && stSENSOR->api->SENSOR_MODEL_GET){
		return stSENSOR->api->SENSOR_MODEL_GET(sensor_name);
	}else{
		return -1;
	}
}

void SENSOR_get_resolution(uint32_t* ret_width, uint32_t* ret_height)
{	
	if(stSENSOR){
		stSENSOR->api->SENSOR_RESOLUTION_GET(ret_width, ret_height);
	}else{
	}
}

void SENSOR_set_resolution(uint32_t width, uint32_t height)
{	
	if(stSENSOR){
		stSENSOR->api->SENSOR_RESOLUTION_SET(width, height);
	}else{
	}
}


int SENSOR_set_conf()
{	
	if(stSENSOR){
		//_sensor_check_default_value();
		return 0;
	}else{
		return -1;
	}

}

uint8_t SENSOR_get_gain()
{	
	if(stSENSOR){
		uint8_t ret_gain;
		ret_gain = stSENSOR->api->GAIN_GET();
		return ret_gain;
	}else{
		return -1;
	}

}

void SENSOR_set_af_attr(stSensorAfAttr *afAttr)
{	
	if(stSENSOR){
		stSENSOR->api->AF_CALLBACK_SET(afAttr);
	}else{
		return -1;
	}
}

int SENSOR_cfg_load(const char *filepath)
{	
	if(stSENSOR){	
		return stSENSOR->api->INI_LOAD(filepath);
	}else{
		return -1;		
	}
}


void SENSOR_WDR_mode_get(uint8_t *bEnable)  //add test wdr
{	
	if(stSENSOR){
		stSENSOR->api->WDR_MODE_GET(bEnable);
	}else{
	}
	
}

//获取IPC的日夜模式：  0-白天            1-夜视
uint8_t  SENSOR_DAYNIGHT_mode_get()  
{	
	if(stSENSOR && stSENSOR->api && stSENSOR->api->DAYNIGHT_MODE_GET){
		uint8_t daylight_mode;
		stSENSOR->api->DAYNIGHT_MODE_GET(&daylight_mode);
		return daylight_mode;
	}else{
	}
	
}





int SENSOR_GET_CUR_FPS()
{
    if(stSENSOR) {
        if(stSENSOR->api->GET_CUR_FPS) {
            return stSENSOR->api->GET_CUR_FPS();
        }
    }
    else {
        return 0;
    }

}

int SENSOR_init(const char *soc, lpBSPApi *api)
{
	if(stSENSOR == NULL){
		stSENSOR = (lpSensorParam)malloc(sizeof(stSensorParam));
		memcpy(&stSENSOR->bsp_api, api, sizeof(stBSPApi));
		/*stSENSOR->bsp_api.BSP_GET_PHOTOSWITCH = api->BSP_GET_PHOTOSWITCH;
		stSENSOR->bsp_api.BSP_IRCUT_SWITCH = api->BSP_IRCUT_SWITCH;
		stSENSOR->bsp_api.BSP_SENSOR_RESET = api->BSP_SENSOR_RESET;
		stSENSOR->bsp_api.BSP_SET_IR_LED = api->BSP_SET_IR_LED;*/
		SDK_ISP_init(&stSENSOR->api, &stSENSOR->bsp_api);
		char sensor_name[16];
		stSENSOR->attr.sensor_type = SENSOR_get_sensor_model(sensor_name);
		char sensor_model[8];
		sprintf(sensor_model, "%02d", stSENSOR->attr.sensor_type);
		SYSCONF_set_software_version_ext(sensor_model);
		printf("sensor type:%s-%d\r\n", sensor_name, stSENSOR->attr.sensor_type);
	//_sensor_setup_tools((uint32_t)g_sensor_type);
		autofocus_status_af_init(0, HI_FALSE);
	}
	return 0;
}

void SENSOR_destroy()
{	
	SDK_ISP_destroy();
	if(stSENSOR){
		free(stSENSOR);
		stSENSOR = NULL;
	}
}


