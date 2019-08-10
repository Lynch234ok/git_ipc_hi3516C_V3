
#include "ak3916.h"
#include "sdk/sdk_debug.h"
#include "sdk/sdk_api.h"
#include "sdk/sdk_isp_def.h"
#include "hi_isp_api.h"
#include "hi_isp.h"
#include "signal.h"
#include "hi_ssp.h"

#define HI3518A_VIN_DEV (0)
#define HI3518A_VIN_CHN (0)

#define GPIO_BASE_ADDR 0x20140000
//ir-cut led :GPIO0_0
#define IRCUT_LED_GPIO_PINMUX_ADDR 0x200f0120
#define IRCUT_LED_GPIO_DIR_ADDR 0x20140400
#define IRCUT_LED_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_LED_GPIO_PIN 0
#define IRCUT_LED_GPIO_GROUP 0

//new hardware ir-cut control :GPIO0_2
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f0128
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 2
#define NEW_IRCUT_CTRL_GPIO_GROUP 0

//old hardware ir-cut control :GPIO0_4
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f0130
#define IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_CTRL_GPIO_PIN 4
#define IRCUT_CTRL_GPIO_GROUP 0

//ir-cut photoswitch read:GPIO0_6
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 0x200f0138
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 0x20140400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 6
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 0

//default factory reset:GPIO0_7
#define HW_RESET_GPIO_PINMUX_ADDR 0x200f013c
#define HW_RESET_GPIO_DIR_ADDR 0x20140400
#define HW_RESET_GPIO_DATA_ADDR 0x201403fc
#define HW_RESET_GPIO_PIN 7
#define HW_RESET_GPIO_GROUP 0

#define ISP_GPIO_DAYLIGHT (0)
#define ISP_GPIO_NIGHT (1)

struct AK3916_ERR_MAP
{
	uint32_t errno;
	const char* str;
};

static struct AK3916_ERR_MAP _ak3916_err_map[] =
{
	// sys
	{ 0xA0028003, "HI_ERR_SYS_ILLEGAL_PARAM", },
	{ 0xA0028006, "HI_ERR_SYS_NULL_PTR", },
	{ 0xA0028009, "HI_ERR_SYS_NOT_PERM", },
	{ 0xA0028010, "HI_ERR_SYS_NOTREADY", },
	{ 0xA0028012, "HI_ERR_SYS_BUSY", },
	{ 0xA002800C, "HI_ERR_SYS_NOMEM", },

	// venc
	{ 0xA0078001, "HI_ERR_VENC_INVALID_DEVID", },
	{ 0xA0078002, "HI_ERR_VENC_INVALID_CHNID", },
	{ 0xA0078003, "HI_ERR_VENC_ILLEGAL_PARAM", },
	{ 0xA0078004, "HI_ERR_VENC_EXIST", },
	{ 0xA0078005, "HI_ERR_VENC_UNEXIST", },
	{ 0xA0078006, "HI_ERR_VENC_NULL_PTR", },
	{ 0xA0078007, "HI_ERR_VENC_NOT_CONFIG", },
	{ 0xA0078008, "HI_ERR_VENC_NOT_SUPPORT", },
	{ 0xA0078009, "HI_ERR_VENC_NOT_PERM", },
	{ 0xA007800C, "HI_ERR_VENC_NOMEM", },
	{ 0xA007800D, "HI_ERR_VENC_NOBUF", },
	{ 0xA007800E, "HI_ERR_VENC_BUF_EMPTY", },
	{ 0xA007800F, "HI_ERR_VENC_BUF_FULL", },
	{ 0xA0078010, "HI_ERR_VENC_SYS_NOTREADY", },

	// vpss
	{ 0xA0088001, "HI_ERR_VPSS_INVALID_DEVID", },
	{ 0xA0088002, "HI_ERR_VPSS_INVALID_CHNID", },
	{ 0xA0088003, "HI_ERR_VPSS_ILLEGAL_PARAM", },
	{ 0xA0088004, "HI_ERR_VPSS_EXIST", },
	{ 0xA0088005, "HI_ERR_VPSS_UNEXIT", },
	{ 0xA0088006, "HI_ERR_VPSS_NULL_PTR", },
	{ 0xA0086008, "HI_ERR_VPSS_NOT_SUPPORT", },
	{ 0xA0088009, "HI_ERR_VPSS_NOT_PERM", },
	{ 0xA008800C, "HI_ERR_VPSS_NOMEM", },
	{ 0xA008800D, "HI_ERR_VPSS_NOBUF" },
	{ 0xA0088010, "HI_ERR_VPSS_NOTREADY", },
	{ 0xA0088012, "HI_ERR_VPSS_BUSY", },
};


const char* SOC_strerror(uint32_t errno)
{
	int i = 0;
	for(i = 0; i < (int)(sizeof(_ak3916_err_map) / sizeof(_ak3916_err_map[0])); ++i){
		if(errno == _ak3916_err_map[i].errno){
			return _ak3916_err_map[i].str;
		}
	}
	return "UNKNOWN ERROR!";
}

typedef struct _ak_isp_attr
{
	stSensorApi api;
	emSENSOR_MODEL sensor_type;
	uint32_t gpio_status_old;// ISP_GPIO_DAYLIGHT;//daytime
	stSensorColorMaxValue color_max_value;
	uint8_t ircut_auto_switch_enable;// HI_TRUE;
	int sensor_resolution_width;
	int sensor_resolution_height;
	uint8_t src_framerate;
}stAkIspAttr, *lpAkIspAttr;

static stAkIspAttr _isp_attr;

static uint32_t isp_gpio_get_dir_addr(int gpio_group)
{
	uint32_t ret_val = 0;
	return ret_val;
}

static uint32_t isp_gpio_get_data_addr(int gpio_group)
{
	uint32_t ret_val = 0;
	return ret_val;
}

static uint32_t isp_gpio_pin_read(int gpio_group, int gpio_pin)
{
	uint32_t reg_val = 0;
	return reg_val;
}

static void isp_gpio_pin_write(int gpio_group, int gpio_pin, uint8_t val)
{
}

static void isp_ircut_control_daylight()
{
	printf("%s\r\n", __FUNCTION__);
	isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);
	isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);	
}

static void isp_ircut_control_night()
{
	printf("%s\r\n", __FUNCTION__);
	isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);
	isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);	
}


static void isp_ircut_switch(uint8_t bEnable)//0:daytime   1:night
{

}

int exposure_calculate(time_t cur_time)
{
	 return 0;
}

static uint32_t isp_get_iso()
{

}

static uint8_t sdk_isp_calculate_exposure(uint32_t old_state)
{
	uint8_t ret_val = 0;
	return ret_val;//0:daytime 1:night
}

static void isp_ircut_gpio_init()
{
}

#define AR0130_CHECK_DATA (0x2402)
#define OV9712_CHECK_DATA_MSB (0x97)
#define OV9712_CHECK_DATA_LSB (0x11)
#define SOIH22_CHECK_DATA_MSB (0xa0)
#define SOIH22_CHECK_DATA_LSB (0x22)
#define IMX122_CHECK_DATA_LSB	(0x50)
#define IMX122_CHECK_DATA_MSB	(0x00)
#define AR0330_CHECK_DATA (0x2604)

int AK_SDK_ISP_sensor_flicker(uint8_t bEnable, uint8_t frequency, uint8_t mode)
{
	return 0;
}

int AK_SDK_ISP_sensor_check()
{
	return 0;
}


int AK_SDK_ISP_ircut_auto_switch(int vin, uint8_t type)//1:software   0: hardware 
{
	return 0;
}

int AK_SDK_ISP_set_mirror(int vin, bool mirror)
{
	return 0;
}

int AK_SDK_ISP_set_flip(int vin, bool flip)
{
	return 0;
}

int AK_SDK_ISP_set_rotate(int vin, int rotate_n)
{
	return 0;
}

int AK_SDK_ISP_set_saturation(int vin, uint16_t val)
{
	return 0;
}

int AK_SDK_ISP_get_saturation(int vin, uint16_t *val)
{
	return 0;
}


int AK_SDK_ISP_set_contrast(int vin, uint16_t val)
{
	return 0;
}

int AK_SDK_ISP_set_brightness(int vin, uint16_t val)
{
	return 0;
}

int AK_SDK_ISP_set_hue(int vin, uint16_t val)
{
	return 0;
}

int AK_SDK_ISP_set_src_framerate(unsigned int framerate)
{
   return 0;
}

int AK_SDK_ISP_get_sharpen(uint8_t *val)
{
	return 0;
}


int AK_SDK_ISP_set_sharpen(uint8_t val)
{
 	return 0;
}

int AK_SDK_ISP_set_scene_mode(uint32_t mode)
{
	return 0;
}

int AK_SDK_ISP_set_WB_mode(uint32_t mode)
{
	return 0;
}

int AK_SDK_ISP_set_ircut_control_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	return 0;
}

int AK_SDK_ISP_set_ircut_mode(uint32_t mode)
{
	return 0;
}

int AK_SDK_ISP_set_WDR_enable(uint8_t bEnable)
{
	return 0;
}

int AK_SDK_ISP_set_WDR_strength(uint8_t val)
{
	return 0;
}

int AK_SDK_ISP_set_exposure_mode(uint32_t mode)
{
	return 0;
}

int AK_SDK_ISP_set_AEcompensation(uint8_t val)
{
	return 0;
}

int AK_SDK_ISP_set_denoise_enable(uint8_t bEnable)
{
	return 0;
}

int AK_SDK_ISP_get_denoise_strength(uint8_t *val)
{
	return 0;
}


int AK_SDK_ISP_set_denoise_strength(uint8_t val)
{
	return 0;
}

int AK_SDK_ISP_set_advance_anti_fog_enable(uint8_t bEnable)
{
	return 0;
}

int AK_SDK_ISP_set_advance_lowlight_enable(uint8_t bEnable)
{
	return 0;
}

int AK_SDK_ISP_set_advance_gamma_table(uint8_t val)
{
	return 0;
}

int AK_SDK_ISP_set_advance_defect_pixel_enable(uint8_t bEnable)
{
	return 0;
}

int AK_SDK_ISP_get_color_max_value(stSensorColorMaxValue *ret_value)
{
	memcpy(ret_value, &_isp_attr.color_max_value, sizeof(stSensorColorMaxValue));
	return 0;
}


int AK_SDK_ISP_get_sensor_model(emSENSOR_MODEL *ret_value)
{
	*ret_value = _isp_attr.sensor_type;
	return 0;
}

int AK_SDK_ISP_set_sensor_resolution(uint32_t width, uint32_t height)
{
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0130:
			_isp_attr.sensor_resolution_width = 1280;
			if(height > 720){
				_isp_attr.sensor_resolution_height = 960;
				//AR0130_sensor_mode_set(1);//960P
			}else{
				_isp_attr.sensor_resolution_height = 720;
				//AR0130_sensor_mode_set(0);//720P
			}			
			break;
		case SENSOR_MODEL_OV_OV9712:
			_isp_attr.sensor_resolution_height = 720;
			_isp_attr.sensor_resolution_width = 1280;
			break;
		case SENSOR_MODEL_SOI_H22:
			_isp_attr.sensor_resolution_height = 720;
			_isp_attr.sensor_resolution_width = 1280;
			break;
		case SENSOR_MODEL_SONY_IMX122:
			_isp_attr.sensor_resolution_height = 1080;
			_isp_attr.sensor_resolution_width = 1920;
			break;
		case SENSOR_MODEL_APTINA_AR0330:
			_isp_attr.sensor_resolution_height = 1080;
			_isp_attr.sensor_resolution_width = 1920;
			break;
	}
	return 0;
}

int AK_SDK_ISP_get_sensor_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
	*ret_width = _isp_attr.sensor_resolution_width;
	*ret_height = _isp_attr.sensor_resolution_height;
	return 0;
}


int AK_SDK_ISP_set_isp_default_value(int mode)////mode  0:daytime   1:night
{
	return 0;
}

static stAkIspAttr _isp_attr = {
	.api = {
		.SENSOR_MODEL_GET=ak_isp_api_get_sensor_model,
		.MIRROR_FLIP_SET=ak_isp_api_mirror_flip,
		.HUE_SET=ak_isp_api_set_hue,
		.CONTRAST_SET=ak_isp_api_set_contrast,
		.BRIGHTNESS_SET=ak_isp_api_set_brightness,
		.SATURATION_SET=ak_isp_api_set_saturation,
		.LIGHT_MODE_SET=ak_isp_api_light_mode,
		.TEST_MODE_SET=ak_isp_api_test_mode,
		.COLOR_MODE_SET=ak_isp_api_color_mode,
		.REG_READ=ak_isp_api_reg_read,
	 	.REG_WRITE=ak_isp_api_reg_write,
		.SPEC_RED_WRITE=ak_isp_api_spec_reg_write,
		.SPEC_REG_READ=ak_isp_api_spec_reg_read,
		.GET_COLOR_MAX_VALUE=ak_isp_api_get_color_max_value,
		.SHUTTER_SET=ak_isp_api_set_shutter,
		.IRCUT_AUTO_SWITCH=ak_isp_api_ircut_auto_switch,
		.VI_FLICKER=ak_isp_api_vi_flicker,
		.SHARPEN_SET=ak_isp_api_set_sharpen,
		.SHARPEN_GET=ak_isp_api_get_sharpen,
		.SCENE_MODE_SET=ak_isp_api_set_scene_mode,
		.WB_MODE_SET=ak_isp_api_set_WB_mode,
		.IRCUT_CONTROL_MODE_SET=ak_isp_api_set_ircut_control_mode,
		.IRCUT_MODE_SET=ak_isp_api_set_ircut_mode,
		.WDR_MODE_ENABLE=ak_isp_api_set_WDR_enable,
		.WDR_STRENGTH_SET=ak_isp_api_set_WDR_strength,
		.EXPOSURE_MODE_SET=ak_isp_api_set_exposure_mode,
		.AE_COMPENSATION_SET=ak_isp_api_set_AEcompensation,
		.DENOISE_ENABLE=ak_isp_api_set_denoise_enable,
		.DENOISE_STRENGTH_SET=ak_isp_api_set_denoise_strength,
		.DENOISE_STRENGTH_GET=ak_isp_api_get_denoise_strength,
		.ANTI_FOG_ENABLE=ak_isp_api_set_anti_fog_enable,
		.LOWLIGHT_ENABLE=ak_isp_api_set_lowlight_enable,
		.GAMMA_TABLE_SET=ak_isp_api_set_gamma_table,
		.DEFECT_PIXEL_ENABLE=ak_isp_api_set_defect_pixel_enable,
		.SRC_FRAMERATE_SET=ak_isp_api_set_src_framerate,
		.SENSOR_RESOLUTION_GET=ak_isp_api_get_sensor_resolution,
		.SENSOR_RESOLUTION_SET=ak_isp_api_set_sensor_resolution,
	},
	.sensor_type = SENSOR_MODEL_APTINA_AR0130,
	.gpio_status_old = ISP_GPIO_DAYLIGHT,// ISP_GPIO_DAYLIGHT;//daytime
	.color_max_value = {
		.HueMax = 100,
		.SaturationMax = 100,
		.ContrastMax = 100,
		.BrightnessMax = 100,
	},
	.ircut_auto_switch_enable = HI_TRUE,// HI_TRUE;
};


int AK_SDK_ISP_init(lpSensorApi*api)
{
	*api = &_isp_attr.api;

	return 0;
}

int AK_SDK_ISP_destroy()
{
	return 0;
}

