
#include "sdk/sdk_isp_def.h"
#include "sdk/sdk_isp.h"


typedef struct _sensor_param
{
	lpSensorApi api;
	stSensorAttr attr;
}stSensorParam, *lpSensorParam;

static lpSensorParam stSENSOR = NULL;

char *sensor_model_str[] = {
	"ar0130",
	"ov9712plus", 
	"soih22",
	"imx122",
	"ar0330",
	"ov9712",
	"gc1004",
	"ar0141",
	"sc1035",
	"ov2710",
	"soih42",
	"imx185",		
	"ov4689",	
	"imx178",
	"mn34220",
	"sc1045",
	"ov5658",
	"bg0701",
};

void _sensor_setup_tools()
{
	char cmd[128] = {0};
	system("kill -9 `pidof ittb_control`");
	usleep(200000);
	//sprintf(cmd, "/root/nfs/gm_ipc/HiPCTools_Board/release_hi3518/ittb_control -n -s %s &", sensor_model_str[sensor_type]);
	sprintf(cmd, "/root/tools/ittb_control -n -s %s &", sensor_model_str[stSENSOR->attr.sensor_type]);
	system(cmd);
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

emSENSOR_MODEL SENSOR_get_sensor_model(char *sensor_name)
{	
	if(stSENSOR){
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
		int ret_gain;
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


int SENSOR_init(const char *soc)
{
	if(stSENSOR == NULL){
		stSENSOR = (lpSensorParam)malloc(sizeof(stSensorParam));
		SDK_ISP_init(&stSENSOR->api);
		char sensor_name[16];
		stSENSOR->attr.sensor_type = SENSOR_get_sensor_model(sensor_name);
		char sensor_model[8];
		sprintf(sensor_model, "%02d", stSENSOR->attr.sensor_type);
		printf("sensor type:%s-%d\r\n", sensor_model_str[stSENSOR->attr.sensor_type], stSENSOR->attr.sensor_type);
	//_sensor_setup_tools((uint32_t)g_sensor_type);
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


