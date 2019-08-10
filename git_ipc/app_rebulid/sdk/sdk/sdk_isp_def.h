
#ifndef __SDK_ISP_DEF_H__
#define __SDK_ISP_DEF_H__

#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define ISP_SCENE_MODE_AUTO (0)
#define ISP_SCENE_MODE_INDOOR (1)
#define ISP_SCENE_MODE_OUTDOOR (2)

#define ISP_IRCUT_CONTROL_MODE_HARDWARE (0)
#define ISP_IRCUT_CONTROL_MODE_SOFTWARE (1)
#define ISP_IRCUT_CONTROL_MODE_IRCUTLINKAGE (2)

#define ISP_IRCUT_MODE_AUTO (0)
#define ISP_IRCUT_MODE_DAYLIGHT (1)
#define ISP_IRCUT_MODE_NIGHT (2)
//#define ISP_IRCUT_MODE_IRCUTLINKAGE (3)
#define ISP_IRCUT_MODE_LIGHTMODE	(3)
#define ISP_IRCUT_MODE_SMARTMODE	(4)

#define ISP_EXPOSURE_MODE_AUTO (0)
#define ISP_EXPOSURE_MODE_BRIGHT (1)
#define ISP_EXPOSURE_MODE_DARK (2)

#define ISP_ADVANCE_GAMMA_DEFAULT (0)
#define ISP_ADVANCE_GAMMA_NORMAL (1)
#define ISP_ADVANCE_GAMMA_HIGH (2)

#define ISP_VIN_DIGITAL_SHUTTER_50HZ (1)
#define ISP_VIN_DIGITAL_SHUTTER_60HZ (2)

#define ISP_LOWLIGHT_MODE_CLOSE (0)
#define ISP_LOWLIGHT_MODE_ALLDAY (1)
#define ISP_LOWLIGHT_MODE_NIGHT (2)
#define ISP_LOWLIGHT_MODE_AUTO (3)
#define ISP_LOWLIGHT_MODE_STARLIGHT (4)

#define SENSOR_TYPE_FILE "/tmp/sensor_type"

//mirror  or flip
enum{
	MODE_NORMAL,
	MODE_MIRROR,
	MODE_FLIP,
	MODE_MIRROR_FLIP,
	MODE_UNMIRROR,
	MODE_UNFLIP,
	MODE_END
};
//Light mode
enum{
	LIGHT_MODE_AUTO,
	LIGHT_MODE_SUNNY,
	LIGHT_MODE_CLOUDY,
	LIGHT_MODE_OFFICE,
	LIGHT_MODE_HOME,
	LIGHT_MODE_END
};
//color mode
enum{
	COLOR_MODE_NORMAL,
	COLOR_MODE_ANTIQUE,
	COLOR_MODE_BLUISH,
	COLOR_MODE_GREENISH,
	COLOR_MODE_REDDISH,
	COLOR_MODE_BW,
	COLOR_MODE_NEGATIVE,
	COLOR_MODE_END
};

typedef enum _SENSOR_MODEL
{
	SENSOR_MODEL_APTINA_AR0130 = 0,
	SENSOR_MODEL_OV_OV9712PLUS,
	SENSOR_MODEL_SOI_H22,
	SENSOR_MODEL_SONY_IMX122,
	SENSOR_MODEL_APTINA_AR0330,
	SENSOR_MODEL_OV_OV9712,
	SENSOR_MODEL_GC1004,
	SENSOR_MODEL_APTINA_AR0141,
	SENSOR_MODEL_SC1035,
	SENSOR_MODEL_OV2710,
	SENSOR_MODEL_SOI_H42,
	SENSOR_MODEL_SONY_IMX185,
	SENSOR_MODEL_OV_OV4689,
	SENSOR_MODEL_SONY_IMX178,
	SENSOR_MODEL_MN34220,	
	SENSOR_MODEL_SC1045,
	SENSOR_MODEL_OV5658,
	SENSOR_MODEL_BG0701,
	SENSOR_MODEL_APTINA_AR0230,
	SENSOR_MODEL_SMARTSENS_SC2035,
	SENSOR_MODEL_APTINA_AR0237,
	SENSOR_MODEL_IMX225,
	SENSOR_MODEL_SC1135,
	SENSOR_MODEL_SMARTSENS_SC2045,
	SENSOR_MODEL_SC1145,
	SENSOR_MODEL_SC3035,
	SENSOR_MODEL_SC2135,
	SENSOR_MODEL_IMX291,
	SENSOR_MODEL_PS5230,
	SENSOR_MODEL_GC4603,
	SENSOR_MODEL_SC1235,
	SENSOR_MODEL_SC2235,
	SENSOR_MODEL_IMX326,
	SENSOR_MODEL_PS5270,
	SENSOR_MODEL_OS05A,
	SENSOR_MODEL_SC2232,
	SENSOR_MODEL_IMX307,
	// add sensor type here!!!!!
	SENSOR_MODEL_CNT,
}emSENSOR_MODEL;


typedef struct _sensor_color_max_value
{
	unsigned short HueMax;
	unsigned short SaturationMax;
	unsigned short ContrastMax;
	unsigned short BrightnessMax;
}stSensorColorMaxValue;

typedef struct _sensor_af_attr
{
	int *param;
	int (*af_callback)(int,int, int, int*);
}stSensorAfAttr;

typedef struct _bsp_api
{
	int (*BSP_GET_PHOTOSWITCH)(void);
	void (*BSP_SET_IR_LED)(bool);
	void (*BSP_SET_WHITE_LIGHT_LED)(bool);
	void (*BSP_IRCUT_SWITCH)(bool);
	int (*BSP_SENSOR_RESET)(bool);
	void (*BSP_SET_PWM_DUTY_CYCLE)(uint8_t, uint16_t);
}stBSPApi, *lpBSPApi;

typedef struct _sensor_api
{
	emSENSOR_MODEL (*SENSOR_MODEL_GET)(char *);
	void (*MIRROR_FLIP_SET)(uint8_t);
	void (*HUE_SET)(uint16_t);
	void (*CONTRAST_SET)(uint8_t);
	void (*BRIGHTNESS_SET)(uint8_t);
	void (*SATURATION_SET)(uint8_t);
	void (*LIGHT_MODE_SET)(uint8_t);
	void (*TEST_MODE_SET)(uint8_t);
	void (*COLOR_MODE_SET)(uint8_t);
	void (*REG_WRITE)(uint16_t,uint16_t);
	uint16_t (*REG_READ)(uint16_t);
	void (*SPEC_RED_WRITE)(uint8_t, uint16_t, uint16_t);
	uint16_t (*SPEC_REG_READ)(uint8_t, uint16_t);
	stSensorColorMaxValue (*GET_COLOR_MAX_VALUE)(void);
	void (*SHUTTER_SET)(uint8_t);
	void (*IRCUT_AUTO_SWITCH)(uint8_t, uint8_t);
	void (*VI_FLICKER)(uint8_t, uint8_t, uint8_t);
	void (*SHARPEN_SET)(uint8_t, uint8_t);
	uint8_t (*SHARPEN_GET)(void);
	void (*SCENE_MODE_SET)(uint32_t);
	void (*WB_MODE_SET)(uint32_t);
	void (*STARLIGHT_MODE_GET)(bool*);
	void	(*COLORTOBLACK_RANGE_SET)(uint8_t);
	uint8_t (*COLORTOBLACK_RANGE_GET)(void);
	void (*IRCUT_CONTROL_MODE_SET)(uint32_t);
	void (*IRCUT_MODE_SET)(uint32_t);
	void (*WDR_MODE_ENABLE)(uint8_t);
	void (*WDR_STRENGTH_SET)(uint8_t);
	void (*EXPOSURE_MODE_SET)(uint32_t);
	void (*AE_COMPENSATION_SET)(uint8_t);
	void (*DENOISE_ENABLE)(uint8_t);
	void (*DENOISE_STRENGTH_SET)(uint8_t);
	uint8_t (*DENOISE_STRENGTH_GET)(void);
	void (*ANTI_FOG_ENABLE)(uint8_t);
	void (*LOWLIGHT_ENABLE)(uint8_t);
	void (*GAMMA_TABLE_SET)(uint8_t);
	void (*DEFECT_PIXEL_ENABLE)(uint8_t);
	void (*SRC_FRAMERATE_SET)(uint32_t);
	void (*SENSOR_RESOLUTION_SET)(uint32_t, uint32_t);
	void (*SENSOR_RESOLUTION_GET)(uint32_t*, uint32_t*);
	uint32_t (*GAIN_GET)(void);
	void (*AF_CALLBACK_SET)(stSensorAfAttr *);
	int (*INI_LOAD)(const char *);
	
	int (*WDR_MODE_GET)(uint8_t*);
    int (*GET_CUR_FPS)();
	void (*DAYNIGHT_MODE_GET)(uint8_t *);
}stSensorApi, *lpSensorApi;

typedef struct _sensor_attr
{
	emSENSOR_MODEL sensor_type;
	stSensorColorMaxValue color_max_value;
}stSensorAttr,*lpSensorAttr;

#define ISP_CFG_TMP_INI "/tmp/hi_isp_cfg.ini"


#ifdef __cplusplus
};
#endif

#endif //__SDK_ISP_DEF_H__

