
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"
#include "sdk/sdk_debug.h"
#include "sdk/sdk_api.h"
#include "sdk/sdk_isp_def.h"
#include "hi_isp_api.h"
#include "hi_isp.h"
#include "signal.h"
#include "hi_ssp.h"
#include "hi_isp_cfg.h"
#include "sdk_common.h"

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

struct HI3518A_ERR_MAP
{
	uint32_t errno;
	const char* str;
};

static struct HI3518A_ERR_MAP _hi3518a_err_map[] =
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
	for(i = 0; i < (int)(sizeof(_hi3518a_err_map) / sizeof(_hi3518a_err_map[0])); ++i){
		if(errno == _hi3518a_err_map[i].errno){
			return _hi3518a_err_map[i].str;
		}
	}
	return "UNKNOWN ERROR!";
}

typedef struct _hi_isp_strength
{
	int strength;
	int daylight_val;
	int night_val;
}stHiIspStrength,*lpHiIspStrength;

typedef struct _hi_isp_attr
{
	stBSPApi bsp_api;
	stSensorApi api;
	emSENSOR_MODEL sensor_type;
	uint32_t gpio_status_old;// ISP_GPIO_DAYLIGHT;//daytime
	stSensorColorMaxValue color_max_value;
	uint8_t ircut_auto_switch_enable;// HI_TRUE;
	uint8_t ircut_control_mode;
	int sensor_resolution_width;
	int sensor_resolution_height;
	uint8_t src_framerate;
	uint8_t lowlight_mode;
	stHiIspStrength aeCompensition;
	stHiIspStrength aeHistSlope;
	stHiIspStrength wdr;
	stHiIspStrength denoise3d;
	stHiIspStrength demosaic;
	uint8_t isp_auto_drc_enabled;
	stSensorAfAttr AfAttr;
	uint8_t isp_framerate_status;
	uint8_t filter_frequency;
	LpIspCfgAttr ispCfgAttr;
}stHiIspAttr,*lpHiIspAttr;

static stHiIspAttr _isp_attr;

const HI_U16 gs_Gamma[12][GAMMA_NODE_NUMBER] = {
	{		//AR0130_2.INI
		0x0,0x4A,0x96,0xE2,0x12E,0x17A,0x1C5,0x20E,0x257,0x29D,0x2E0,0x321,0x360,0x39E,0x3DA,0x415,
		0x44E,0x486,0x4BD,0x4F2,0x527,0x552,0x57B,0x5A3,0x5C9,0x5EF,0x615,0x639,0x65E,0x682,0x6A7,0x6CC,
		0x6F1,0x70D,0x72A,0x747,0x765,0x782,0x79F,0x7BC,0x7D9,0x7F6,0x812,0x82E,0x849,0x863,0x87D,0x895,
		0x8AD,0x8C3,0x8D9,0x8ED,0x902,0x915,0x928,0x93A,0x94C,0x95D,0x96E,0x97E,0x98F,0x99F,0x9B0,0x9C0,
		0x9D1,0x9E1,0x9F1,0xA01,0xA10,0xA20,0xA2F,0xA3D,0xA4C,0xA5B,0xA69,0xA77,0xA85,0xA93,0xAA1,0xAAF,
		0xABD,0xACA,0xAD8,0xAE5,0xAF2,0xB00,0xB0D,0xB1A,0xB27,0xB33,0xB40,0xB4C,0xB58,0xB64,0xB70,0xB7B,
		0xB87,0xB91,0xB9C,0xBA6,0xBB1,0xBBA,0xBC4,0xBCE,0xBD7,0xBE0,0xBE9,0xBF3,0xBFC,0xC05,0xC0E,0xC18,
		0xC22,0xC2B,0xC35,0xC3F,0xC49,0xC54,0xC5E,0xC68,0xC72,0xC7C,0xC86,0xC90,0xC99,0xCA2,0xCAB,0xCB4,
		0xCBD,0xCC5,0xCCC,0xCD4,0xCDB,0xCE2,0xCE9,0xCF0,0xCF6,0xCFD,0xD03,0xD0A,0xD10,0xD16,0xD1C,0xD22,
		0xD29,0xD2F,0xD35,0xD3B,0xD41,0xD47,0xD4D,0xD53,0xD58,0xD5E,0xD64,0xD69,0xD6F,0xD74,0xD7A,0xD7F,
		0xD85,0xD8A,0xD8F,0xD94,0xD99,0xD9E,0xDA3,0xDA8,0xDAD,0xDB2,0xDB7,0xDBC,0xDC0,0xDC5,0xDCA,0xDCE,
		0xDD3,0xDD7,0xDDB,0xDDF,0xDE4,0xDE8,0xDEC,0xDF0,0xDF4,0xDF7,0xDFB,0xDFF,0xE03,0xE06,0xE0A,0xE0D,
		0xE11,0xE14,0xE17,0xE1A,0xE1D,0xE20,0xE23,0xE26,0xE29,0xE2B,0xE2E,0xE31,0xE33,0xE36,0xE39,0xE3C,
		0xE3F,0xE41,0xE44,0xE47,0xE4A,0xE4D,0xE50,0xE53,0xE56,0xE59,0xE5C,0xE5F,0xE62,0xE65,0xE67,0xE6A,
		0xE6D,0xE6F,0xE72,0xE74,0xE76,0xE79,0xE7B,0xE7D,0xE7F,0xE81,0xE83,0xE84,0xE86,0xE88,0xE89,0xE8B,
		0xE8C,0xE8D,0xE8E,0xE8F,0xE90,0xE91,0xE91,0xE92,0xE92,0xE93,0xE93,0xE94,0xE94,0xE94,0xE95,0xE95,
		0xE96,
	},
	{
		0x0,0x43,0x87,0xCB,0x110,0x154,0x197,0x1D8,0x218,0x255,0x29E,0x2E4,0x326,0x367,0x3A7,0x3E9,
		0x42D,0x44B,0x46A,0x488,0x4A7,0x4C6,0x4E5,0x505,0x524,0x543,0x563,0x582,0x5A2,0x5C2,0x5E1,0x601,
		0x621,0x640,0x661,0x682,0x6A3,0x6C4,0x6E6,0x707,0x729,0x74A,0x76B,0x78B,0x7AB,0x7CA,0x7E8,0x805,
		0x822,0x83D,0x858,0x872,0x88B,0x8A4,0x8BC,0x8D4,0x8EB,0x902,0x919,0x92F,0x944,0x95A,0x96F,0x984,
		0x999,0x9AE,0x9C2,0x9D6,0x9E9,0x9FC,0xA0E,0xA21,0xA33,0xA45,0xA56,0xA68,0xA79,0xA8A,0xA9B,0xAAC,
		0xABD,0xACE,0xADF,0xAEF,0xB00,0xB10,0xB21,0xB31,0xB40,0xB50,0xB5F,0xB6F,0xB7E,0xB8C,0xB9A,0xBA8,
		0xBB6,0xBC3,0xBD0,0xBDD,0xBE9,0xBF5,0xC01,0xC0D,0xC18,0xC23,0xC2E,0xC39,0xC43,0xC4D,0xC58,0xC61,
		0xC6B,0xC74,0xC7D,0xC85,0xC8D,0xC95,0xC9C,0xCA3,0xCAA,0xCB1,0xCB9,0xCC0,0xCC7,0xCCF,0xCD7,0xCDF,
		0xCE8,0xCF1,0xCFB,0xD06,0xD10,0xD1B,0xD26,0xD31,0xD3D,0xD48,0xD53,0xD5E,0xD68,0xD72,0xD7C,0xD85,
		0xD8E,0xD96,0xD9E,0xDA5,0xDAC,0xDB2,0xDB8,0xDBE,0xDC4,0xDCA,0xDCF,0xDD5,0xDDA,0xDE0,0xDE5,0xDEA,
		0xDF0,0xDF6,0xDFB,0xE01,0xE06,0xE0C,0xE11,0xE16,0xE1B,0xE20,0xE26,0xE2B,0xE30,0xE35,0xE39,0xE3E,
		0xE43,0xE48,0xE4C,0xE51,0xE56,0xE5A,0xE5E,0xE63,0xE67,0xE6B,0xE70,0xE74,0xE78,0xE7C,0xE80,0xE84,
		0xE88,0xE8C,0xE90,0xE94,0xE97,0xE9B,0xE9F,0xEA2,0xEA6,0xEA9,0xEAD,0xEB0,0xEB3,0xEB7,0xEBA,0xEBD,
		0xEC0,0xEC3,0xEC6,0xEC8,0xECB,0xECD,0xED0,0xED2,0xED5,0xED7,0xED9,0xEDC,0xEDE,0xEE1,0xEE3,0xEE6,
		0xEE9,0xEEC,0xEEF,0xEF3,0xEF6,0xEF9,0xEFD,0xF01,0xF04,0xF08,0xF0B,0xF0F,0xF13,0xF16,0xF1A,0xF1E,
		0xF21,0xF24,0xF28,0xF2B,0xF2F,0xF32,0xF36,0xF39,0xF3D,0xF40,0xF43,0xF47,0xF4A,0xF4E,0xF51,0xF55,
		0xF58,
	},
	{				
		0x0,0x3E,0x7E,0xBE,0xFF,0x141,0x182,0x1C4,0x205,0x245,0x284,0x2C2,0x2FE,0x338,0x371,0x3A7,
		0x3DA,0x40B,0x439,0x466,0x490,0x4B9,0x4E0,0x506,0x52B,0x54F,0x572,0x594,0x5B6,0x5D8,0x5F9,0x61A,
		0x63C,0x65D,0x67E,0x69E,0x6BE,0x6DC,0x6FB,0x719,0x736,0x753,0x76F,0x78B,0x7A7,0x7C3,0x7DE,0x7F9,
		0x814,0x82F,0x849,0x863,0x87D,0x896,0x8AF,0x8C7,0x8E0,0x8F8,0x90F,0x927,0x93E,0x955,0x96C,0x983,
		0x999,0x9AF,0x9C5,0x9DB,0x9F1,0xA06,0xA1B,0xA30,0xA45,0xA59,0xA6D,0xA81,0xA94,0xAA7,0xABA,0xACC,
		0xADE,0xAF0,0xB01,0xB11,0xB22,0xB31,0xB41,0xB50,0xB5F,0xB6E,0xB7D,0xB8B,0xB99,0xBA8,0xBB6,0xBC4,
		0xBD2,0xBE0,0xBEE,0xBFB,0xC09,0xC16,0xC23,0xC30,0xC3D,0xC4A,0xC57,0xC63,0xC70,0xC7C,0xC89,0xC95,
		0xCA2,0xCAF,0xCBB,0xCC8,0xCD5,0xCE2,0xCEE,0xCFB,0xD07,0xD14,0xD20,0xD2C,0xD37,0xD43,0xD4E,0xD59,
		0xD63,0xD6D,0xD76,0xD7F,0xD88,0xD91,0xD99,0xDA1,0xDA9,0xDB1,0xDB8,0xDC0,0xDC7,0xDCF,0xDD6,0xDDE,
		0xDE6,0xDEE,0xDF7,0xE00,0xE08,0xE10,0xE19,0xE21,0xE29,0xE30,0xE38,0xE3F,0xE47,0xE4E,0xE55,0xE5B,
		0xE62,0xE68,0xE6E,0xE74,0xE7A,0xE80,0xE85,0xE8A,0xE8F,0xE93,0xE98,0xE9C,0xEA0,0xEA3,0xEA6,0xEA9,
		0xEAC,0xEAF,0xEB3,0xEB7,0xEBB,0xEC0,0xEC4,0xEC9,0xECD,0xED2,0xED7,0xEDC,0xEE0,0xEE5,0xEEA,0xEEF,
		0xEF4,0xEF9,0xEFE,0xF03,0xF08,0xF0D,0xF12,0xF18,0xF1D,0xF22,0xF27,0xF2C,0xF32,0xF37,0xF3C,0xF41,
		0xF46,0xF4B,0xF51,0xF56,0xF5B,0xF60,0xF65,0xF6A,0xF6F,0xF74,0xF79,0xF7E,0xF83,0xF88,0xF8C,0xF91,
		0xF96,0xF9A,0xF9F,0xFA4,0xFA8,0xFAC,0xFB1,0xFB5,0xFB9,0xFBD,0xFC1,0xFC5,0xFC9,0xFCC,0xFD0,0xFD4,
		0xFD7,0xFDA,0xFDE,0xFE1,0xFE4,0xFE7,0xFEA,0xFEC,0xFEF,0xFF1,0xFF3,0xFF6,0xFF8,0xFFA,0xFFB,0xFFD,
		0xFFF,
	},
	{
		0x0,0x33,0x68,0x9D,0xD3,0x10A,0x140,0x176,0x1AC,0x1E1,0x216,0x249,0x27B,0x2AC,0x2DB,0x308,
		0x333,0x35B,0x382,0x3A7,0x3CA,0x3EC,0x40D,0x42C,0x44B,0x469,0x486,0x4A3,0x4C0,0x4DD,0x4FB,0x518,
		0x537,0x555,0x573,0x592,0x5B0,0x5CD,0x5EB,0x609,0x626,0x643,0x65F,0x67C,0x698,0x6B3,0x6CE,0x6E9,
		0x704,0x71E,0x738,0x751,0x769,0x782,0x79A,0x7B2,0x7C9,0x7E1,0x7F8,0x80E,0x825,0x83B,0x851,0x866,
		0x87C,0x891,0x8A6,0x8BB,0x8CF,0x8E3,0x8F7,0x90B,0x91F,0x932,0x945,0x958,0x96B,0x97D,0x98F,0x9A1,
		0x9B3,0x9C5,0x9D7,0x9E8,0x9F9,0xA0A,0xA1B,0xA2B,0xA3C,0xA4C,0xA5C,0xA6C,0xA7C,0xA8C,0xA9B,0xAAB,
		0xABA,0xAC9,0xAD8,0xAE6,0xAF5,0xB03,0xB12,0xB20,0xB2E,0xB3C,0xB4A,0xB57,0xB65,0xB72,0xB80,0xB8D,
		0xB9A,0xBA7,0xBB4,0xBC0,0xBCD,0xBD9,0xBE6,0xBF2,0xBFE,0xC0A,0xC16,0xC22,0xC2E,0xC39,0xC45,0xC50,
		0xC5C,0xC67,0xC72,0xC7D,0xC88,0xC93,0xC9E,0xCA8,0xCB3,0xCBE,0xCC8,0xCD2,0xCDD,0xCE7,0xCF1,0xCFB,
		0xD05,0xD0F,0xD18,0xD22,0xD2C,0xD35,0xD3F,0xD48,0xD52,0xD5B,0xD64,0xD6D,0xD76,0xD7F,0xD88,0xD91,
		0xD9A,0xDA2,0xDAB,0xDB4,0xDBC,0xDC5,0xDCD,0xDD5,0xDDE,0xDE6,0xDEE,0xDF6,0xDFE,0xE06,0xE0E,0xE16,
		0xE1E,0xE25,0xE2D,0xE35,0xE3C,0xE44,0xE4C,0xE53,0xE5A,0xE62,0xE69,0xE70,0xE77,0xE7F,0xE86,0xE8D,
		0xE94,0xE9B,0xEA2,0xEA8,0xEAF,0xEB6,0xEBD,0xEC3,0xECA,0xED1,0xED7,0xEDE,0xEE4,0xEEB,0xEF1,0xEF7,
		0xEFE,0xF04,0xF0A,0xF10,0xF17,0xF1D,0xF23,0xF29,0xF2F,0xF35,0xF3B,0xF41,0xF46,0xF4C,0xF52,0xF58,
		0xF5D,0xF63,0xF69,0xF6E,0xF74,0xF7A,0xF7F,0xF85,0xF8A,0xF8F,0xF95,0xF9A,0xF9F,0xFA5,0xFAA,0xFAF,
		0xFB4,0xFBA,0xFBF,0xFC4,0xFC9,0xFCE,0xFD3,0xFD8,0xFDD,0xFE2,0xFE7,0xFEC,0xFF1,0xFF5,0xFFA,0xFFF,
		0xFFF,
	},
	{
		0x0, 0x36, 0x6a, 0x9e, 0xd1, 0x103, 0x134, 0x164, 0x193, 0x1c2, 0x1ef, 0x21c, 0x248, 0x274, 0x29e, 0x2c9, 
		0x2f2, 0x31b, 0x343, 0x36a, 0x391, 0x3b7, 0x3dd, 0x402, 0x426, 0x44a, 0x46d, 0x490, 0x4b3, 0x4d4, 0x4f6, 0x517, 
		0x537, 0x557, 0x576, 0x595, 0x5b4, 0x5d2, 0x5f0, 0x60d, 0x62a, 0x647, 0x663, 0x67f, 0x69a, 0x6b5, 0x6d0, 0x6ea, 
		0x704, 0x71e, 0x738, 0x751, 0x769, 0x782, 0x79a, 0x7b2, 0x7c9, 0x7e1, 0x7f8, 0x80e, 0x825, 0x83b, 0x851, 0x866, 
		0x87c, 0x891, 0x8a6, 0x8bb, 0x8cf, 0x8e3, 0x8f7, 0x90b, 0x91f, 0x932, 0x945, 0x958, 0x96b, 0x97d, 0x98f, 0x9a1, 
		0x9b3, 0x9c5, 0x9d7, 0x9e8, 0x9f9, 0xa0a, 0xa1b, 0xa2b, 0xa3c, 0xa4c, 0xa5c, 0xa6c, 0xa7c, 0xa8c, 0xa9b, 0xaab, 
		0xaba, 0xac9, 0xad8, 0xae6, 0xaf5, 0xb03, 0xb12, 0xb20, 0xb2e, 0xb3c, 0xb4a, 0xb57, 0xb65, 0xb72, 0xb80, 0xb8d, 
		0xb9a, 0xba7, 0xbb4, 0xbc0, 0xbcd, 0xbd9, 0xbe6, 0xbf2, 0xbfe, 0xc0a, 0xc16, 0xc22, 0xc2e, 0xc39, 0xc45, 0xc50, 
		0xc5c, 0xc67, 0xc72, 0xc7d, 0xc88, 0xc93, 0xc9e, 0xca8, 0xcb3, 0xcbe, 0xcc8, 0xcd2, 0xcdd, 0xce7, 0xcf1, 0xcfb, 
		0xd05, 0xd0f, 0xd18, 0xd22, 0xd2c, 0xd35, 0xd3f, 0xd48, 0xd52, 0xd5b, 0xd64, 0xd6d, 0xd76, 0xd7f, 0xd88, 0xd91, 
		0xd9a, 0xda2, 0xdab, 0xdb4, 0xdbc, 0xdc5, 0xdcd, 0xdd5, 0xdde, 0xde6, 0xdee, 0xdf6, 0xdfe, 0xe06, 0xe0e, 0xe16, 
		0xe1e, 0xe25, 0xe2d, 0xe35, 0xe3c, 0xe44, 0xe4c, 0xe53, 0xe5a, 0xe62, 0xe69, 0xe70, 0xe77, 0xe7f, 0xe86, 0xe8d, 
		0xe94, 0xe9b, 0xea2, 0xea8, 0xeaf, 0xeb6, 0xebd, 0xec3, 0xeca, 0xed1, 0xed7, 0xede, 0xee4, 0xeeb, 0xef1, 0xef7, 
		0xefe, 0xf04, 0xf0a, 0xf10, 0xf17, 0xf1d, 0xf23, 0xf29, 0xf2f, 0xf35, 0xf3b, 0xf41, 0xf46, 0xf4c, 0xf52, 0xf58, 
		0xf5d, 0xf63, 0xf69, 0xf6e, 0xf74, 0xf7a, 0xf7f, 0xf85, 0xf8a, 0xf8f, 0xf95, 0xf9a, 0xf9f, 0xfa5, 0xfaa, 0xfaf, 
		0xfb4, 0xfba, 0xfbf, 0xfc4, 0xfc9, 0xfce, 0xfd3, 0xfd8, 0xfdd, 0xfe2, 0xfe7, 0xfec, 0xff1, 0xff5, 0xffa, 0xfff, 
		0xfff, 
	},
	{
		0x0,   0xe,    0x20, 0x35,  0x4d, 0x68, 0x85,  0xa5,  0xc7, 0xeb,  0x110,0x138,0x162,0x18d,0x1b9,0x1e6,
		0x215,0x244,0x274,0x2a5,0x2d6,0x308,0x33a,0x36b,0x39d,0x3ce,0x3ff,0x42f,0x45f,0x48d,0x4bb,0x4e8,
		0x515,0x540,0x56b,0x595,0x5bd,0x5e5,0x60c,0x633,0x658,0x67c,0x69f,0x6c1,0x6e2,0x702,0x721,0x73f,
		0x75c,0x778,0x792,0x7ac,0x7c5,0x7de,0x7f5,0x80c,0x822,0x838,0x84d,0x861,0x876,0x88a,0x89e,0x8b1,
		0x8c5,0x8d8,0x8eb,0x8ff,0x912,0x926,0x93a,0x94e,0x962,0x975,0x989,0x99d,0x9b1,0x9c5,0x9d9,0x9ec,
		0xa00,0xa13,0xa27,0xa3a,0xa4d,0xa5f,0xa72,0xa84,0xa97,0xaa8,0xaba,0xacb,0xadc,0xaed,0xafd,0xb0d,
		0xb1d,0xb2c,0xb3b,0xb49,0xb57,0xb64,0xb71,0xb7d,0xb89,0xb95,0xb9f,0xba9,0xbb3,0xbbc,0xbc4,0xbcc,
		0xbd3,0xbd9,0xbdf,0xbeb,0xbf8,0xc04,0xc10,0xc1c,0xc29,0xc35,0xc41,0xc4c,0xc58,0xc64,0xc70,0xc7b,
		0xc87,0xc92,0xc9e,0xca9,0xcb4,0xcbf,0xccb,0xcd6,0xce1,0xceb,0xcf6,0xd01,0xd0c,0xd16,0xd21,0xd2b,
		0xd36,0xd40,0xd4a,0xd54,0xd5e,0xd68,0xd72,0xd7c,0xd86,0xd90,0xd99,0xda3,0xdac,0xdb6,0xdbf,0xdc8,
		0xdd2,0xddb,0xde4,0xded,0xdf6,0xdff,0xe07,0xe10,0xe19,0xe21,0xe2a,0xe32,0xe3b,0xe43,0xe4b,0xe53,
		0xe5b,0xe63,0xe6b,0xe73,0xe7b,0xe83,0xe8a,0xe92,0xe9a,0xea1,0xea8,0xeb0,0xeb7,0xebe,0xec5,0xecc,
		0xed3,0xeda,0xee1,0xee7,0xeee,0xef5,0xefb,0xf02,0xf08,0xf0e,0xf15,0xf1b,0xf21,0xf27,0xf2d,0xf33,
		0xf39,0xf3e,0xf44,0xf4a,0xf4f,0xf55,0xf5a,0xf5f,0xf65,0xf6a,0xf6f,0xf74,0xf79,0xf7e,0xf83,0xf87,
		0xf8c,0xf91,0xf95,0xf9a,0xf9e,0xfa3,0xfa7,0xfab,0xfaf,0xfb3,0xfb7,0xfbb,0xfbf,0xfc3,0xfc7,0xfcb,
		0xfce,0xfd2,0xfd5,0xfd9,0xfdc,0xfdf,0xfe2,0xfe5,0xfe8,0xfeb,0xfee,0xff1,0xff4,0xff7,0xff9,0xffc,
		0xffe,
	},

		{
			0x0,0x146,0x1BF,0x21A,0x265,0x2A7,0x2E2,0x318,0x349,0x378,0x3A4,0x3CD,0x3F4,0x41A,0x43E,0x461,
			0x483,0x4A3,0x4C2,0x4E1,0x4FE,0x51B,0x537,0x553,0x56D,0x588,0x5A1,0x5BA,0x5D3,0x5EB,0x602,0x61A,
			0x630,0x647,0x65D,0x673,0x688,0x69D,0x6B2,0x6C6,0x6DA,0x6EE,0x702,0x715,0x728,0x73B,0x74E,0x760,
			0x773,0x785,0x797,0x7A8,0x7BA,0x7CB,0x7DC,0x7ED,0x7FE,0x80E,0x81F,0x82F,0x83F,0x84F,0x85F,0x86F,
			0x87E,0x88E,0x89D,0x8AC,0x8BB,0x8CA,0x8D9,0x8E8,0x8F7,0x905,0x913,0x922,0x930,0x93E,0x94C,0x95A,
			0x968,0x975,0x983,0x990,0x99E,0x9AB,0x9B8,0x9C6,0x9D3,0x9E0,0x9ED,0x9FA,0xA06,0xA13,0xA20,0xA2C,
			0xA39,0xA45,0xA51,0xA5E,0xA6A,0xA76,0xA82,0xA8E,0xA9A,0xAA6,0xAB2,0xABE,0xAC9,0xAD5,0xAE0,0xAEC,
			0xAF7,0xB03,0xB0E,0xB1A,0xB25,0xB30,0xB3B,0xB46,0xB51,0xB5C,0xB67,0xB72,0xB7D,0xB88,0xB93,0xB9D,
			0xBA8,0xBB3,0xBBD,0xBC8,0xBD2,0xBDD,0xBE7,0xBF1,0xBFC,0xC06,0xC10,0xC1A,0xC25,0xC2F,0xC39,0xC43,
			0xC4D,0xC57,0xC61,0xC6B,0xC75,0xC7E,0xC88,0xC92,0xC9C,0xCA5,0xCAF,0xCB9,0xCC2,0xCCC,0xCD5,0xCDF,
			0xCE8,0xCF1,0xCFB,0xD04,0xD0E,0xD17,0xD20,0xD29,0xD33,0xD3C,0xD45,0xD4E,0xD57,0xD60,0xD69,0xD72,
			0xD7B,0xD84,0xD8D,0xD96,0xD9F,0xDA7,0xDB0,0xDB9,0xDC2,0xDCB,0xDD3,0xDDC,0xDE5,0xDED,0xDF6,0xDFE,
			0xE07,0xE0F,0xE18,0xE20,0xE29,0xE31,0xE3A,0xE42,0xE4A,0xE53,0xE5B,0xE63,0xE6C,0xE74,0xE7C,0xE84,
			0xE8D,0xE95,0xE9D,0xEA5,0xEAD,0xEB5,0xEBD,0xEC5,0xECD,0xED5,0xEDD,0xEE5,0xEED,0xEF5,0xEFD,0xF05,
			0xF0D,0xF15,0xF1C,0xF24,0xF2C,0xF34,0xF3C,0xF43,0xF4B,0xF53,0xF5A,0xF62,0xF6A,0xF71,0xF79,0xF81,
			0xF88,0xF90,0xF97,0xF9F,0xFA6,0xFAE,0xFB5,0xFBD,0xFC4,0xFCB,0xFD3,0xFDA,0xFE2,0xFE9,0xFF0,0xFF8,
			0xFFF,
		},
		{/////imx 222 
			0x0,0xCF,0x185,0x1EE,0x239,0x282,0x2C4,0x2FC,0x333,0x372,0x3B0,0x3E0,0x40C,
			0x43A,0x468,0x490,
			0x4B8,0x4E1,0x507,0x52D,0x551,0x574,0x597,0x5B9,0x5DB,0x5FD,0x61E,0x63F,0x65F,
			0x67E,0x69D,0x6BC,
			0x6DB,0x6F9,0x717,0x735,0x752,0x76F,0x78C,0x7A9,0x7C5,0x7E2,0x7FF,0x81B,0x837,
			0x853,0x86E,0x888,
			0x8A2,0x8BB,0x8D5,0x8ED,0x906,0x91E,0x936,0x94D,0x964,0x97B,0x991,0x9A7,0x9BD,
			0x9D2,0x9E6,0x9FA,
			0xA0E,0xA21,0xA33,0xA45,0xA56,0xA67,0xA77,0xA87,0xA97,0xAA6,0xAB6,0xAC5,0xAD4,
			0xAE3,0xAF2,0xB01,
			0xB10,0xB1F,0xB2F,0xB3E,0xB4E,0xB5D,0xB6C,0xB7B,0xB8A,0xB99,0xBA7,0xBB6,0xBC4,
			0xBD3,0xBE1,0xBEE,
			0xBFC,0xC09,0xC16,0xC23,0xC30,0xC3C,0xC49,0xC55,0xC61,0xC6D,0xC79,0xC85,0xC91,
			0xC9C,0xCA8,0xCB3,
			0xCBF,0xCCA,0xCD5,0xCE0,0xCEA,0xCF5,0xCFF,0xD0A,0xD14,0xD1E,0xD29,0xD33,0xD3D,
			0xD47,0xD51,0xD5B,
			0xD65,0xD6E,0xD78,0xD82,0xD8C,0xD96,0xDA0,0xDAA,0xDB4,0xDBE,0xDC7,0xDD0,0xDDA,
			0xDE3,0xDEC,0xDF4,
			0xDFD,0xE05,0xE0C,0xE14,0xE1B,0xE23,0xE2A,0xE31,0xE37,0xE3E,0xE45,0xE4B,0xE52,
			0xE58,0xE5F,0xE65,
			0xE6C,0xE72,0xE78,0xE7F,0xE85,0xE8C,0xE92,0xE98,0xE9E,0xEA4,0xEAA,0xEB0,0xEB6,
			0xEBC,0xEC2,0xEC8,
			0xECE,0xED3,0xED9,0xEDE,0xEE4,0xEE9,0xEEF,0xEF4,0xEFA,0xEFF,0xF04,0xF09,0xF0E,
			0xF13,0xF18,0xF1D,
			0xF22,0xF26,0xF2B,0xF2F,0xF33,0xF37,0xF3C,0xF40,0xF44,0xF48,0xF4B,0xF4F,0xF53,
			0xF57,0xF5B,0xF5F,
			0xF63,0xF66,0xF6A,0xF6E,0xF72,0xF75,0xF79,0xF7D,0xF81,0xF84,0xF88,0xF8B,0xF8F,
			0xF93,0xF96,0xF9A,
			0xF9E,0xFA1,0xFA5,0xFA9,0xFAC,0xFB0,0xFB4,0xFB7,0xFBB,0xFBF,0xFC2,0xFC6,0xFC9,
			0xFCD,0xFD0,0xFD3,
			0xFD7,0xFD9,0xFDC,0xFDF,0xFE2,0xFE4,0xFE7,0xFE9,0xFEC,0xFEE,0xFF0,0xFF3,0xFF5,
			0xFF7,0xFFA,0xFFC,
			0xFFF,
		},

           {//BG0701 daylight
              	0x0,0x3F,0x7E,0xBF,0x100,0x141,0x183,0x1C4,0x205,0x245,0x284,0x2C3,0x300,0x33B,0x375,0x3AC,
				0x3E2,0x415,0x447,0x478,0x4A6,0x4D4,0x500,0x52B,0x555,0x57E,0x5A6,0x5CE,0x5F4,0x61A,0x640,0x665,
				0x68A,0x6AE,0x6D1,0x6F3,0x714,0x734,0x754,0x773,0x791,0x7AF,0x7CC,0x7E9,0x806,0x822,0x83F,0x85B,
				0x878,0x895,0x8B1,0x8CE,0x8EA,0x906,0x922,0x93E,0x959,0x974,0x98E,0x9A8,0x9C2,0x9DB,0x9F3,0xA0B,
				0xA22,0xA38,0xA4E,0xA64,0xA78,0xA8D,0xAA0,0xAB4,0xAC7,0xAD9,0xAEB,0xAFD,0xB0E,0xB1F,0xB30,0xB41,
				0xB51,0xB61,0xB70,0xB7F,0xB8E,0xB9C,0xBAA,0xBB7,0xBC5,0xBD2,0xBDE,0xBEB,0xBF8,0xC05,0xC11,0xC1E,
				0xC2B,0xC38,0xC44,0xC51,0xC5E,0xC6B,0xC79,0xC86,0xC93,0xCA0,0xCAC,0xCB9,0xCC5,0xCD1,0xCDC,0xCE7,
				0xCF1,0xCFB,0xD04,0xD0D,0xD16,0xD1E,0xD26,0xD2D,0xD35,0xD3C,0xD43,0xD4A,0xD51,0xD57,0xD5E,0xD64,
				0xD6B,0xD71,0xD78,0xD7E,0xD83,0xD89,0xD8E,0xD94,0xD99,0xD9E,0xDA3,0xDA9,0xDAE,0xDB3,0xDB8,0xDBE,
				0xDC3,0xDC8,0xDCE,0xDD3,0xDD9,0xDDE,0xDE4,0xDE9,0xDEE,0xDF4,0xDF9,0xDFF,0xE04,0xE0A,0xE0F,0xE15,
				0xE1B,0xE21,0xE27,0xE2E,0xE34,0xE3B,0xE41,0xE48,0xE4F,0xE55,0xE5C,0xE62,0xE68,0xE6E,0xE74,0xE79,
				0xE7E,0xE83,0xE87,0xE8B,0xE8F,0xE92,0xE95,0xE99,0xE9C,0xE9F,0xEA2,0xEA4,0xEA8,0xEAB,0xEAE,0xEB1,
				0xEB5,0xEB9,0xEBD,0xEC1,0xEC5,0xEC9,0xECD,0xED1,0xED5,0xEDA,0xEDE,0xEE2,0xEE6,0xEEA,0xEEF,0xEF3,
				0xEF7,0xEFB,0xEFF,0xF03,0xF08,0xF0C,0xF10,0xF14,0xF18,0xF1C,0xF20,0xF24,0xF29,0xF2D,0xF31,0xF35,
				0xF39,0xF3D,0xF41,0xF44,0xF48,0xF4B,0xF4F,0xF52,0xF56,0xF5A,0xF5D,0xF62,0xF66,0xF6B,0xF70,0xF75,
				0xF7B,0xF81,0xF88,0xF90,0xF97,0xF9F,0xFA8,0xFB0,0xFB9,0xFC2,0xFCB,0xFD4,0xFDC,0xFE5,0xFEE,0xFF7,
				0xFFF,
           },
		{  
			0x0,0x6F,0xE2,0x14F,0x1AD,0x1F9,0x236,0x26D,0x2A3,0x2DB,0x310,0x344,0x378,0x3AD,0x3E3,0x417,
			0x449,0x474,0x49D,0x4C5,0x4EF,0x51C,0x54A,0x57A,0x5A6,0x5CC,0x5F3,0x616,0x63D,0x662,0x689,0x6AF,
			0x6D4,0x6FB,0x716,0x733,0x750,0x76D,0x789,0x7A5,0x7C3,0x7E0,0x7FB,0x818,0x834,0x84F,0x86B,0x885,
			0x89F,0x8B8,0x8CF,0x8E6,0x8FE,0x916,0x92C,0x943,0x959,0x970,0x985,0x99A,0x9B0,0x9C5,0x9D9,0x9ED,
			0xA01,0xA15,0xA29,0xA3A,0xA4D,0xA60,0xA70,0xA83,0xA93,0xAA5,0xAB5,0xAC5,0xAD5,0xAE6,0xAF6,0xB05,
			0xB14,0xB23,0xB32,0xB41,0xB50,0xB5E,0xB6C,0xB78,0xB87,0xB94,0xBA0,0xBAE,0xBBA,0xBC8,0xBD3,0xBE1,
			0xBEE,0xBFA,0xC08,0xC13,0xC21,0xC2C,0xC3A,0xC45,0xC52,0xC5E,0xC6B,0xC77,0xC81,0xC8E,0xC9A,0xCA5,
			0xCB1,0xCBC,0xCC7,0xCD2,0xCDE,0xCE9,0xCF4,0xCFE,0xD08,0xD13,0xD1F,0xD29,0xD33,0xD3E,0xD48,0xD52,
			0xD5C,0xD66,0xD70,0xD7A,0xD84,0xD8D,0xD97,0xDA1,0xDA9,0xDB2,0xDBC,0xDC5,0xDCF,0xDD8,0xDDF,0xDE8,
			0xDF1,0xDF8,0xE01,0xE08,0xE10,0xE17,0xE1F,0xE25,0xE2D,0xE33,0xE3A,0xE41,0xE47,0xE4F,0xE56,0xE5B,
			0xE63,0xE6A,0xE70,0xE77,0xE7E,0xE84,0xE8B,0xE92,0xE98,0xE9F,0xEA5,0xEAB,0xEB1,0xEB8,0xEBE,0xEC4,
			0xEC9,0xECF,0xED4,0xEDB,0xEE0,0xEE6,0xEEB,0xEF0,0xEF5,0xEFA,0xEFF,0xF04,0xF09,0xF0E,0xF12,0xF17,
			0xF1C,0xF1F,0xF24,0xF29,0xF2D,0xF32,0xF37,0xF3C,0xF41,0xF45,0xF4A,0xF4E,0xF53,0xF56,0xF5A,0xF5F,
			0xF64,0xF68,0xF6C,0xF6F,0xF73,0xF77,0xF7B,0xF7E,0xF82,0xF86,0xF8A,0xF8D,0xF91,0xF95,0xF97,0xF9B,
			0xFA0,0xFA3,0xFA6,0xFAA,0xFAE,0xFB2,0xFB4,0xFB9,0xFBC,0xFBF,0xFC3,0xFC7,0xFC9,0xFCD,0xFD0,0xFD2,
			0xFD7,0xFD8,0xFDC,0xFDF,0xFE1,0xFE4,0xFE6,0xFE9,0xFEB,0xFEE,0xFF0,0xFF3,0xFF5,0xFF6,0xFFA,0xFFB,
			0xFFF,
		},	//1045	daylight

		{
			0x0,0x40,0x82,0xC5,0x108,0x14D,0x191,0x1D5,0x218,0x25B,0x29C,0x2DC,0x319,0x355,0x38E,0x3C3,
			0x3F6,0x425,0x452,0x47C,0x4A4,0x4C9,0x4ED,0x50F,0x530,0x54F,0x56D,0x58B,0x5A7,0x5C4,0x5E0,0x5FC,
			0x618,0x634,0x64F,0x669,0x682,0x69A,0x6B1,0x6C8,0x6DE,0x6F4,0x709,0x71E,0x733,0x747,0x75B,0x770,
			0x784,0x798,0x7AC,0x7BF,0x7D2,0x7E4,0x7F7,0x809,0x81A,0x82C,0x83D,0x84E,0x85F,0x870,0x881,0x891,
			0x8A2,0x8B3,0x8C3,0x8D3,0x8E3,0x8F3,0x903,0x913,0x922,0x932,0x941,0x950,0x95F,0x96E,0x97C,0x98B,
			0x999,0x9A7,0x9B5,0x9C3,0x9D0,0x9DD,0x9EA,0x9F7,0xA04,0xA11,0xA1E,0xA2A,0xA37,0xA43,0xA50,0xA5C,
			0xA69,0xA76,0xA82,0xA8F,0xA9B,0xAA8,0xAB4,0xAC1,0xACD,0xAD9,0xAE5,0xAF1,0xAFD,0xB09,0xB15,0xB21,
			0xB2C,0xB37,0xB42,0xB4D,0xB58,0xB63,0xB6E,0xB78,0xB83,0xB8D,0xB98,0xBA2,0xBAC,0xBB6,0xBC1,0xBCB,
			0xBD5,0xBDF,0xBE9,0xBF3,0xBFD,0xC07,0xC10,0xC1A,0xC24,0xC2D,0xC37,0xC41,0xC4A,0xC54,0xC5E,0xC67,
			0xC71,0xC7B,0xC85,0xC8E,0xC98,0xCA2,0xCAC,0xCB6,0xCC0,0xCCA,0xCD3,0xCDD,0xCE7,0xCF1,0xCFA,0xD04,
			0xD0D,0xD16,0xD20,0xD29,0xD32,0xD3B,0xD44,0xD4D,0xD57,0xD5F,0xD68,0xD71,0xD7A,0xD83,0xD8B,0xD94,
			0xD9C,0xDA4,0xDAC,0xDB4,0xDBC,0xDC4,0xDCC,0xDD3,0xDDB,0xDE2,0xDEA,0xDF1,0xDF9,0xE01,0xE08,0xE10,
			0xE18,0xE20,0xE28,0xE30,0xE38,0xE40,0xE49,0xE51,0xE59,0xE61,0xE69,0xE72,0xE7A,0xE82,0xE8A,0xE92,
			0xE9A,0xEA2,0xEAA,0xEB2,0xEB9,0xEC1,0xEC9,0xED1,0xED9,0xEE0,0xEE8,0xEF0,0xEF8,0xEFF,0xF07,0xF0F,
			0xF16,0xF1E,0xF25,0xF2D,0xF34,0xF3C,0xF43,0xF4B,0xF52,0xF5A,0xF61,0xF69,0xF70,0xF77,0xF7F,0xF86,
			0xF8D,0xF95,0xF9C,0xFA3,0xFAA,0xFB2,0xFB9,0xFC0,0xFC7,0xFCE,0xFD6,0xFDD,0xFE4,0xFEB,0xFF2,0xFF9,
			0xFFF,
	},//[10] imx225-2016.03.22

	//[11] 1135-20160419-night
	{
			
			0x0,0x39,0x73,0xAE,0xEA,0x126,0x161,0x19D,0x1D8,0x213,0x24D,0x286,0x2BD,0x2F4,0x328,0x35B,
			0x38B,0x3BA,0x3E7,0x412,0x43C,0x465,0x48D,0x4B4,0x4D9,0x4FE,0x522,0x545,0x567,0x589,0x5AA,0x5CA,
			0x5EA,0x609,0x628,0x645,0x662,0x67E,0x699,0x6B3,0x6CD,0x6E6,0x6FE,0x716,0x72D,0x744,0x75A,0x770,
			0x786,0x79B,0x7AF,0x7C2,0x7D5,0x7E7,0x7F9,0x809,0x81A,0x82A,0x83A,0x84A,0x859,0x869,0x879,0x888,
			0x898,0x8A8,0x8B7,0x8C7,0x8D6,0x8E5,0x8F4,0x903,0x912,0x921,0x92F,0x93E,0x94C,0x95A,0x968,0x976,
			0x984,0x992,0x9A0,0x9AD,0x9BB,0x9C8,0x9D6,0x9E3,0x9F0,0x9FD,0xA0A,0xA17,0xA24,0xA31,0xA3D,0xA4A,
			0xA56,0xA63,0xA6F,0xA7B,0xA88,0xA94,0xAA0,0xAAC,0xAB8,0xAC4,0xACF,0xADB,0xAE7,0xAF2,0xAFE,0xB09,
			0xB15,0xB20,0xB2B,0xB37,0xB42,0xB4D,0xB58,0xB63,0xB6E,0xB79,0xB84,0xB8F,0xB99,0xBA4,0xBAF,0xBB9,
			0xBC4,0xBCF,0xBD9,0xBE3,0xBEE,0xBF8,0xC02,0xC0D,0xC17,0xC21,0xC2B,0xC35,0xC3F,0xC49,0xC53,0xC5D,
			0xC67,0xC71,0xC7A,0xC84,0xC8E,0xC98,0xCA1,0xCAB,0xCB4,0xCBE,0xCC7,0xCD1,0xCDA,0xCE4,0xCED,0xCF6,
			0xCFF,0xD09,0xD12,0xD1B,0xD24,0xD2D,0xD36,0xD3F,0xD48,0xD51,0xD5A,0xD63,0xD6C,0xD75,0xD7E,0xD87,
			0xD8F,0xD98,0xDA1,0xDAA,0xDB2,0xDBB,0xDC3,0xDCC,0xDD4,0xDDD,0xDE5,0xDEE,0xDF6,0xDFF,0xE07,0xE10,
			0xE18,0xE20,0xE28,0xE31,0xE39,0xE41,0xE49,0xE51,0xE5A,0xE62,0xE6A,0xE72,0xE7A,0xE82,0xE8A,0xE92,
			0xE9A,0xEA2,0xEAA,0xEB2,0xEB9,0xEC1,0xEC9,0xED1,0xED9,0xEE0,0xEE8,0xEF0,0xEF8,0xEFF,0xF07,0xF0F,
			0xF16,0xF1E,0xF25,0xF2D,0xF34,0xF3C,0xF43,0xF4B,0xF52,0xF5A,0xF61,0xF69,0xF70,0xF77,0xF7F,0xF86,
			0xF8D,0xF95,0xF9C,0xFA3,0xFAA,0xFB2,0xFB9,0xFC0,0xFC7,0xFCE,0xFD6,0xFDD,0xFE4,0xFEB,0xFF2,0xFF9,
			0xFFF, 

							
	 },

};

#include "hi_i2c.h"
static int _hi_sdk_isp_init_isp_default_value(void);//Declaration
static int _hi_sdk_isp_set_slow_framerate(uint8_t bValue);//Declaration


static int ar0130_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x20;		/* I2C Address of AR0130 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

    i2c_data.dev_addr = sensor_i2c_addr ;
    i2c_data.reg_addr = addr    ;
    i2c_data.addr_byte_num   = sensor_addr_byte  ;
    i2c_data.data_byte_num   = sensor_data_byte ;
    ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
    *ret_data =  i2c_data.data ;
    printf("0x%x 0x%x\n", addr, *ret_data);

	close(fd);
	return 0;
}

static int ar0130_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x20;		/* I2C Address of AR0130 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int ov9712_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of ov9712 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	*ret_data = i2c_data.data;
	printf("0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int ov9712_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of ov9712 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int soih22_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of soih22 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	*ret_data = i2c_data.data;
	printf("0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int soih22_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of soih22 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int imx122_i2c_read(int addr, uint16_t* ret_data)
{
	unsigned int data = (unsigned int)(((addr&0xffff)<<8));
	int fd = -1;
	int ret;
	unsigned int value;

	fd = open("/dev/ssp", 0);
	if(fd < 0)
	{
		printf("Open /dev/ssp error!\n");
		return -1;
	}

	value = data;

	ret = ioctl(fd, SSP_READ_ALT, &value);

	close(fd);
	*ret_data = value&0xff;
	printf("0x%x 0x%x\n", addr, *ret_data);
	return (value&0xff);
}

static int imx122_i2c_write(int addr, int data)
{
	unsigned int value = (unsigned int)(((addr&0xffff)<<8) | (data & 0xff));

	int fd = -1;
	int ret;

	fd = open("/dev/ssp", 0);
	if(fd < 0)
	{
		printf("Open /dev/ssp error!\n");
		return -1;
	}

	ret = ioctl(fd, SSP_WRITE_ALT, &value);

	close(fd);
	return 0;
}

static int ar0330_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x20;		/* I2C Address of AR0330 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

    i2c_data.dev_addr = sensor_i2c_addr ;
    i2c_data.reg_addr = addr    ;
    i2c_data.addr_byte_num   = sensor_addr_byte  ;
    i2c_data.data_byte_num   = sensor_data_byte ;
    ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
    *ret_data =  i2c_data.data ;
    printf("0x%x 0x%x\n", addr, *ret_data);

	close(fd);
	return 0;
}


static int ar0330_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x20;		/* I2C Address of AR0330 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}


static int gc1004_i2c_read(int addr, uint16_t* ret_data) //add by yang
{
	int fd = -1;
	int ret;
	I2C_DATA_S i2c_data;

	const unsigned char sensor_i2c_addr	=	0x78;		
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;


	fd = open("/dev/hi_i2c", 0);
	if(fd<0)
	{
		printf("Open hi_i2c error!\n");
		return -1;
	}

	i2c_data.dev_addr = sensor_i2c_addr;
	i2c_data.reg_addr = addr;
	i2c_data.addr_byte_num = sensor_addr_byte;
	i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);


	*ret_data = i2c_data.data;	
//	printf("0x%x 0x%x\n", addr, *ret_data);

	close(fd);


	return 0;


}

static int gc1004_i2c_write(int addr, int data)
{   //add yang
	int fd = -1;
	int ret;
	I2C_DATA_S i2c_data;

    const unsigned char sensor_i2c_addr	=	0x78;		
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	
	fd = open("/dev/hi_i2c", 0);
	if(fd<0)
	{
		printf("Open hi_i2c error!\n");
		return -1;
	}
	
	i2c_data.dev_addr = sensor_i2c_addr;
	i2c_data.reg_addr = addr;
	i2c_data.addr_byte_num = sensor_addr_byte;
	i2c_data.data = data;
	i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

	if (ret)
	{
		printf("hi_i2c write faild!\n");
		close(fd);
		return -1;
	}

	close(fd);

	return 0; 

}

static int ar0141_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x30;		/* I2C Address of AR0141 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

    i2c_data.dev_addr = sensor_i2c_addr ;
    i2c_data.reg_addr = addr    ;
    i2c_data.addr_byte_num   = sensor_addr_byte  ;
    i2c_data.data_byte_num   = sensor_data_byte ;
    ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
    *ret_data =  i2c_data.data ;
    printf("0x%x 0x%x\n", addr, *ret_data);

	close(fd);
	return 0;
}

static int ar0141_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x30;		/* I2C Address of AR0141 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	2;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}


static int sc1035_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of AR0141 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

    i2c_data.dev_addr = sensor_i2c_addr ;
    i2c_data.reg_addr = addr    ;
    i2c_data.addr_byte_num   = sensor_addr_byte  ;
    i2c_data.data_byte_num   = sensor_data_byte ;
    ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
    *ret_data =  i2c_data.data ;
    //printf("0x%x 0x%x\n", addr, *ret_data);

	close(fd);
	return 0;
}

static int sc1035_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of AR0141 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int ov2710_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x6C;		/* I2C Address of ov2710 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	*ret_data = i2c_data.data;
	printf("0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int ov2710_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x6C;		/* I2C Address of ov2710 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int soih42_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of soih42 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	*ret_data = i2c_data.data;
	printf("0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int soih42_i2c_write(int addr, int data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of soih42 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}
static int SC1045_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of ov9712 */
	const unsigned int  sensor_addr_byte=      2;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);

	*ret_data = i2c_data.data;
	//printf("0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int SC1045_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of po4100k*/
	const unsigned int  sensor_addr_byte=	2;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}


static int bg0701_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x64;		/* I2C Address of bg0701 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

	i2c_data.dev_addr = sensor_i2c_addr|0x01;//read address :0x65
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data_byte_num = sensor_data_byte;

	ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
	//printf("bg0701 i2c ioctl read ret value is %d \n",ret);
	*ret_data = i2c_data.data;
	//printf("read :0x%x 0x%x\n", addr, *ret_data);
	close(fd);
	return 0;
}

static int bg0701_i2c_write(int addr, int data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x64;		/* I2C Address of bg0701 */
	const unsigned int  sensor_addr_byte	=	1;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}

static int imx225_i2c_read(int addr, uint16_t* ret_data)
{
	unsigned int data = (unsigned int)(((addr&0xffff)<<8));
	int fd = -1;
	int ret;
	unsigned int value;

	fd = open("/dev/ssp", 0);
	if(fd < 0)
	{
		printf("Open /dev/ssp error!\n");
		return -1;
	}

	value = data;

	ret = ioctl(fd, SSP_READ_ALT, &value);

	close(fd);
	*ret_data = value&0xff;
	printf("0x%x 0x%x\n", addr, *ret_data);
	return (value&0xff);
}

static int imx225_i2c_write(int addr, int data)
{
	unsigned int value = (unsigned int)(((addr&0xffff)<<8) | (data & 0xff));

	int fd = -1;
	int ret;

	fd = open("/dev/ssp", 0);
	if(fd < 0)
	{
		printf("Open /dev/ssp error!\n");
		return -1;
	}

	ret = ioctl(fd, SSP_WRITE_ALT, &value);

	close(fd);
	return 0;
}

static int sc1135_i2c_read(int addr, uint16_t* ret_data)
{
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of sc1135 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	1;
	I2C_DATA_S i2c_data;
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }

    i2c_data.dev_addr = sensor_i2c_addr ;
    i2c_data.reg_addr = addr    ;
    i2c_data.addr_byte_num   = sensor_addr_byte  ;
    i2c_data.data_byte_num   = sensor_data_byte ;
    ret = ioctl(fd, CMD_I2C_READ, &i2c_data);
    *ret_data =  i2c_data.data ;
    //printf("0x%x 0x%x\n", addr, *ret_data);

	close(fd);
	return 0;
}

static int sc1135_i2c_write(int addr, int data)
{
//	return 0;
    int fd = -1;
    int ret;
	const unsigned char sensor_i2c_addr	=	0x60;		/* I2C Address of AR0141 */
	const unsigned int  sensor_addr_byte	=	2;
	const unsigned int  sensor_data_byte	=	1;
    I2C_DATA_S i2c_data;
	
    fd = open("/dev/hi_i2c", 0);
    if(fd<0)
    {
        printf("Open hi_i2c error!\n");
        return -1;
    }
    
    i2c_data.dev_addr = sensor_i2c_addr;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = sensor_addr_byte;
    i2c_data.data = data;
    i2c_data.data_byte_num = sensor_data_byte;

    ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);

    if (ret)
    {
        printf("hi_i2c write faild!\n");
        return -1;
    }

    close(fd);
	return 0;
}




static uint32_t isp_gpio_get_dir_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x400;
	return ret_val;
}

static uint32_t isp_gpio_get_data_addr(int gpio_group)
{
	uint32_t ret_val;
	ret_val = GPIO_BASE_ADDR + gpio_group*0x10000 + 0x3fc;
	return ret_val;
}

static uint32_t isp_gpio_pin_read(int gpio_group, int gpio_pin)
{
	uint32_t reg_val = 0;

	//pin dir :in
	sdk_sys->read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);

	//read pin
	sdk_sys->read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= (1<<gpio_pin);
	return reg_val;
}

static void isp_gpio_pin_write(int gpio_group, int gpio_pin, uint8_t val)
{
	uint32_t reg_val = 0;
	
	//pin dir :out
	sdk_sys->read_reg(isp_gpio_get_dir_addr(gpio_group), &reg_val);
	reg_val |= (1<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_dir_addr(gpio_group), reg_val);
	
	sdk_sys->read_reg(isp_gpio_get_data_addr(gpio_group), &reg_val);
	reg_val &= ~(1<<gpio_pin);
	reg_val |= (val<<gpio_pin);
	sdk_sys->write_reg(isp_gpio_get_data_addr(gpio_group), reg_val);
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
	static uint32_t old_satuation = 0;
	
	if(!old_satuation){
			SOC_CHECK(HI_MPI_ISP_GetSaturation(&old_satuation));
		}
	if(!bEnable){
		printf("daylight mode!\r\n");	
		SOC_CHECK(HI_MPI_ISP_SetSaturation(old_satuation));
		printf("saturation:%d\r\n", old_satuation);
		//isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
		//isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
		//isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 1);
		_isp_attr.bsp_api.BSP_SET_IR_LED(true);
		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_DAYLIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_DAYLIGHT;
		//usleep(1000*150);
		
		//isp_ircut_control_daylight();
		
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY){
			_hi_sdk_isp_set_slow_framerate(true);
		}else{
			_hi_sdk_isp_set_slow_framerate(false);
		}
	}else{			
		printf("night mode!\r\n");
		SOC_CHECK(HI_MPI_ISP_SetSaturation(0));
		isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1);//IR LED on
		//isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 1);//IR-CUT on
		//isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
		_isp_attr.bsp_api.BSP_SET_IR_LED(false);
		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_NIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_NIGHT;		
		//usleep(1000*150);
		//isp_ircut_control_night();
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_CLOSE){
			_hi_sdk_isp_set_slow_framerate(false);
		}else{
			_hi_sdk_isp_set_slow_framerate(true);
		}
	}
	HI_SDK_ISP_set_isp_sensor_value();
	if(_isp_attr.ispCfgAttr){
		HI_ISP_cfg_set_all(_isp_attr.gpio_status_old, 1, _isp_attr.ispCfgAttr);
	}
}

static uint32_t gains_calculate(void)
{
	uint32_t ret_iso;
	ISP_INNER_STATE_INFO_EX_S pstInnerStateInfo;
	SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfoEx(&pstInnerStateInfo));
	//printf("u32ExposureTime = 	0x%08x\r\n", pstInnerStateInfo.u32ExposureTime);
	//printf("u32AnalogGain = 	0x%d\r\n", pstInnerStateInfo.u32AnalogGain);
	//printf("u32DigitalGain = 	0x%04d\r\n", pstInnerStateInfo.u32DigitalGain);
	//printf("u32ISPDigitalGain = 		0x%04d\r\n", pstInnerStateInfo.u32ISPDigitalGain);
	//printf("u32SystemGain = 		%d\r\n",  (pstInnerStateInfo.u32AnalogGain * pstInnerStateInfo.u32DigitalGain * pstInnerStateInfo.u32ISPDigitalGain)>>20);
	//printf("u8AveLum = 			0x%02x\r\n", pstInnerStateInfo.u8AveLum);
	//printf("bExposureIsMAX = 	0x%x\r\n", pstInnerStateInfo.bExposureIsMAX);
	ret_iso = (pstInnerStateInfo.u32AnalogGain * ((pstInnerStateInfo.u32DigitalGain * (pstInnerStateInfo.u32ISPDigitalGain >> 10))>>4))>>6;
	//printf("ret_iso :%d-%d\r\n", ret_iso, ((pstInnerStateInfo.u32DigitalGain * (pstInnerStateInfo.u32ISPDigitalGain >> 10))>>10));
	return ret_iso;
}

static uint32_t isp_get_iso()
{
	ISP_INNER_STATE_INFO_S InnerStateInfo;
	SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfo(&InnerStateInfo));
	//HI_U32 _again = InnerStateInfo.u32AnalogGain == 0 ? 1 : InnerStateInfo.u32AnalogGain;
	//HI_U32 _dgain = InnerStateInfo.u32DigitalGain == 0 ? 1 : InnerStateInfo.u32DigitalGain;
	return InnerStateInfo.u32AnalogGain * InnerStateInfo.u32DigitalGain * 100;
}

static uint8_t sdk_isp_calculate_exposure(uint32_t old_state)
{
	//ISP_EXP_STA_INFO_S pstExpStatistic;
	//SOC_CHECK(HI_MPI_ISP_GetExpStaInfo(&pstExpStatistic));
	uint8_t ret_val = 0;
	unsigned long long AE_max_gain, state_info_gain, system_gain;
	ISP_INNER_STATE_INFO_EX_S pstInnerStateInfo;
	ISP_AE_ATTR_EX_S AEAttr;

	SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfoEx(&pstInnerStateInfo));
	SOC_CHECK(HI_MPI_ISP_GetAEAttrEx(&AEAttr));

	state_info_gain = ((unsigned long long)pstInnerStateInfo.u32AnalogGain * (unsigned long long)pstInnerStateInfo.u32DigitalGain * (unsigned long long)pstInnerStateInfo.u32ISPDigitalGain)>> 20;
	AE_max_gain = ((unsigned long long)AEAttr.u32AGainMax * (unsigned long long)AEAttr.u32DGainMax * (unsigned long long)AEAttr.u32ISPDGainMax) >> 20;
	system_gain = (unsigned long long)(AE_max_gain > AEAttr.u32SystemGainMax)?AEAttr.u32SystemGainMax : AE_max_gain;
	printf("old_state:%llu/%llu/%d/%llu\r\n", state_info_gain, AE_max_gain, AEAttr.u32SystemGainMax,system_gain);

	if(state_info_gain > system_gain * 8/10){
		ret_val = 1;//night
	}else{
		ret_val = 0;//daylight
	}
	
	return ret_val;//0:daytime 1:night
}

static void isp_ircut_gpio_init()
{
	uint32_t reg_val = 0;
	//muxpin
	sdk_sys->write_reg(IRCUT_LED_GPIO_PINMUX_ADDR, 0);//GPIO0_0
	//pin dir :out
	sdk_sys->read_reg(IRCUT_LED_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_LED_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_LED_GPIO_DIR_ADDR, reg_val);

	//muxpin
	sdk_sys->write_reg(NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR, 0);//GPIO0_2
	//pin dir :out
	sdk_sys->read_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<NEW_IRCUT_CTRL_GPIO_PIN);
	sdk_sys->write_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);

	
	//muxpin
	sdk_sys->write_reg(IRCUT_CTRL_GPIO_PINMUX_ADDR, 0);//GPIO0_4
	//pin dir :out
	sdk_sys->read_reg(IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_CTRL_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);
	
	//muxpin
	sdk_sys->write_reg(IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR, 0);//GPIO0_6
	//pin dir :in
	sdk_sys->read_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1<<IRCUT_PHOTOSWITCH_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, reg_val);
	
	//muxpin
	sdk_sys->write_reg(HW_RESET_GPIO_PINMUX_ADDR, 1);//GPIO0_7
	//pin dir :in
	sdk_sys->read_reg(HW_RESET_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1<<HW_RESET_GPIO_PIN);
	sdk_sys->write_reg(HW_RESET_GPIO_DIR_ADDR, reg_val);

	isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
	isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
	
}

static int isp_get_isp_strength_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val + val->daylight_val*(val->strength - 3)/2;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val + val->night_val*(val->strength - 3)/2;
		}
		break;
	}
	if(ret_val> 255){
		ret_val = 255;
	}
	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}

static uint8_t isp_get_isp_strength_value2(uint8_t val[], uint8_t strength)
{
	uint8_t ret_val = 0;
	if(val){
		ret_val = val[_isp_attr.gpio_status_old] + val[_isp_attr.gpio_status_old]*(strength - 3)/2;
	}
	
	if(ret_val> 255){
		ret_val = 255;
	}
	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}


static int isp_get_isp_value(lpHiIspStrength val)
{
	int ret_val = 0;
	switch(_isp_attr.gpio_status_old){
		default:
		case ISP_GPIO_DAYLIGHT:
		{
			ret_val = val->daylight_val;
		}
		break;
		case ISP_GPIO_NIGHT:
		{
			ret_val = val->night_val;
		}
		break;
	}

	if(ret_val < 0){
		ret_val = 0;
	}
	return ret_val;
}


#define AR0130_CHECK_DATA (0x2402)
#define OV9712_CHECK_DATA_MSB (0x97)
#define OV9712_CHECK_DATA_LSB (0x11)
#define SOIH22_CHECK_DATA_MSB (0xa0)
#define SOIH22_CHECK_DATA_LSB (0x22)
#define IMX122_CHECK_DATA_LSB	(0x50)
#define IMX122_CHECK_DATA_MSB	(0x00)
#define AR0330_CHECK_DATA (0x2604)
#define GC1004_CHECK_DATA_MSB (0x10)
#define GC1004_CHECK_DATA_LSB (0x04)
#define AR0141_CHECK_DATA (0x51)
#define OV2710_CHECK_DATA_MSB (0x27)
#define OV2710_CHECK_DATA_LSB (0x10)
#define SOIH42_CHECK_DATA_MSB (0xa0)
#define SOIH42_CHECK_DATA_LSB (0x42)
#define BG0701_CHECK_DATA_MSB (0x07)
#define BG0701_CHECK_DATA_LSB (0x01)
#define IMX225_CHECK_DATA	(0x11)//0x21c

#define SC1035_CHECK_DATA (0xf0)	//0X580B
#define SC1045_CHECK_DATA_LSB (0x10)//0X3107
#define SC1045_CHECK_DATA_MSB (0x45)//0X3108
#define SC1135_CHECK_DATA_MSB (0x00)	//0X580B
#define SC1135_CHECK_DATA_LSB (0x00)	//0X3108



int HI_SDK_ISP_sensor_flicker(uint8_t bEnable, uint8_t frequency, uint8_t mode)
{
	ISP_ANTIFLICKER_S pstAntiflicker;
	SOC_CHECK(HI_MPI_ISP_GetAntiFlickerAttr(&pstAntiflicker));
	if(bEnable != 0xff){
		pstAntiflicker.bEnable = bEnable;
	}else{
		printf("pstAntiflicker.bEnable = %d\r\n", pstAntiflicker.bEnable);
		pstAntiflicker.bEnable = HI_TRUE;
	}
	pstAntiflicker.bEnable = HI_TRUE;
	if(frequency){
		pstAntiflicker.u8Frequency = frequency;
		_isp_attr.filter_frequency = frequency;
	}else{
		pstAntiflicker.u8Frequency = _isp_attr.filter_frequency;
	}
	pstAntiflicker.enMode = mode ? ISP_ANTIFLICKER_MODE_1:ISP_ANTIFLICKER_MODE_0;
	printf("%s---%d:%d   mode:%d\r\n", __FUNCTION__, frequency, bEnable,   pstAntiflicker.enMode);
	SOC_CHECK(HI_MPI_ISP_SetAntiFlickerAttr(&pstAntiflicker));
	//printf("%s-%d:%d/%d\r\n", __FUNCTION__, __LINE__, pstAntiflicker.bEnable, pstAntiflicker.u8Frequency);
	return 0;
}

emSENSOR_MODEL HI_SDK_ISP_sensor_check()
{
	uint16_t ret_data1 = 0;
	uint16_t ret_data2 = 0;
	do{
		//ar0130
		{
			ar0130_i2c_read(0x3000, &ret_data1);
			if(ret_data1 == AR0130_CHECK_DATA){
				_isp_attr.sensor_type = SENSOR_MODEL_APTINA_AR0130;
				sdk_sys->write_reg(0x20030030, 0x5); //set sensor clock
				break;
			}
		}

		//ov9712
		{
			ov9712_i2c_read(0x0a, &ret_data1);
			ov9712_i2c_read(0x0b, &ret_data2);
			if(ret_data1 == OV9712_CHECK_DATA_MSB && ret_data2 == OV9712_CHECK_DATA_LSB){
				if(!strcmp(PRODUCT_MODEL, "561120")){
					_isp_attr.sensor_type = SENSOR_MODEL_OV_OV9712;
				}else{
					_isp_attr.sensor_type = SENSOR_MODEL_OV_OV9712PLUS;
				}
				sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock
				break;
			}
		}
		
		//soi_h22
		{
			soih22_i2c_read(0x0a, &ret_data1);
			soih22_i2c_read(0x0b, &ret_data2);
			if(ret_data1 == SOIH22_CHECK_DATA_MSB && ret_data2 == SOIH22_CHECK_DATA_LSB){
				_isp_attr.sensor_type = SENSOR_MODEL_SOI_H22;
				sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock
				break;
			}
		}

		//sony imx122
		{
			imx122_i2c_read(0x21c,&ret_data1);
			imx122_i2c_read(0x21d, &ret_data2);
			if(ret_data1 == IMX122_CHECK_DATA_LSB && ret_data2 == IMX122_CHECK_DATA_MSB){
				_isp_attr.sensor_type = SENSOR_MODEL_SONY_IMX122;
				sdk_sys->write_reg(0x200f000c, 0x1);
				sdk_sys->write_reg(0x200f0010, 0x1);
				sdk_sys->write_reg(0x200f0014, 0x1);
				sdk_sys->write_reg(0x20030030, 0x6);
				break;
			}
		}

		//ar0330
		{
			ar0330_i2c_read(0x3000, &ret_data1);
			if(ret_data1 == AR0330_CHECK_DATA){
				_isp_attr.sensor_type = SENSOR_MODEL_APTINA_AR0330;
				sdk_sys->write_reg(0x20030030, 0x2); //set sensor clock
				break;
			}
		}

	   //gc1004
		{  
		 	gc1004_i2c_read(0xf0,&ret_data1);
			printf("0xf0 0x%x\n", ret_data1);
			gc1004_i2c_read(0xf1,&ret_data2);
			printf("0xf1 0x%x\n", ret_data2);
			
			if(ret_data1 == GC1004_CHECK_DATA_MSB && ret_data2 == GC1004_CHECK_DATA_LSB){
				_isp_attr.sensor_type = SENSOR_MODEL_GC1004;				
				sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock
				break;
				}
		}
	   //ar0141
	   {
	   		ar0141_i2c_read(0x3000, &ret_data1);
			if(ret_data1 == AR0141_CHECK_DATA){
				_isp_attr.sensor_type = SENSOR_MODEL_APTINA_AR0141;
				sdk_sys->write_reg(0x20030030, 0x5); //set sensor clock
				break;
			}		
	   }
	   //sc1035
	   {
	   		sc1035_i2c_read(0x580B,&ret_data1);
	   		if(ret_data1 == SC1035_CHECK_DATA){
				_isp_attr.sensor_type = SENSOR_MODEL_SC1035;
				sdk_sys->write_reg(0x20030030, 0x5); //set sensor clock	
				break;
			}		
	   }
	   //ov2710 
	   {
		   ov2710_i2c_read(0x300a, &ret_data1);
		   ov2710_i2c_read(0x300b, &ret_data2);
		   if(ret_data1 == OV2710_CHECK_DATA_MSB &&  ret_data2 == OV2710_CHECK_DATA_LSB){
			   _isp_attr.sensor_type = SENSOR_MODEL_OV2710;
			   sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock
			   break;
		   }
	   }

	   //soi_h42
		{
			soih22_i2c_read(0x0a, &ret_data1);
			soih22_i2c_read(0x0b, &ret_data2);
			if(ret_data1 == SOIH42_CHECK_DATA_MSB && ret_data2 == SOIH42_CHECK_DATA_LSB){
				_isp_attr.sensor_type = SENSOR_MODEL_SOI_H42;
				sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock
				break;
			}
		}
	   
	   //sc1045
		{
			SC1045_i2c_read(0x3107,&ret_data1);
			SC1045_i2c_read(0x3108,&ret_data2);
			 if(ret_data1 == SC1045_CHECK_DATA_LSB&&ret_data2 == SC1045_CHECK_DATA_MSB){			 	
				 _isp_attr.sensor_type = SENSOR_MODEL_SC1045;
				 sdk_sys->write_reg(0x20030030, 0x5); //set sensor clock 
				 break;
			 }		 
		}
	   //bg0701 
	   {
		   bg0701_i2c_read(0x00, &ret_data1);
		   bg0701_i2c_read(0x01, &ret_data2);
		   if(ret_data1 == BG0701_CHECK_DATA_MSB &&  ret_data2 == BG0701_CHECK_DATA_LSB)
		   {
			   _isp_attr.sensor_type = SENSOR_MODEL_BG0701;
			   sdk_sys->write_reg(0x200f0008, 0x1);
			   sdk_sys->write_reg(0x200f0018, 0x1);
			   sdk_sys->write_reg(0x200f001C, 0x1);
			   sdk_sys->write_reg(0x20030030, 0x1); //set sensor clock 24MHz
			   break;
		   }
	   }
	 //sony imx225
		{	
			sdk_sys->write_reg(0x200f000c, 0x1);
			sdk_sys->write_reg(0x200f0010, 0x1);
			sdk_sys->write_reg(0x200f0014, 0x1);
			imx225_i2c_read(0x21c,&ret_data1);
			if(ret_data1 == IMX225_CHECK_DATA){
				_isp_attr.sensor_type = SENSOR_MODEL_IMX225;
				sdk_sys->write_reg(0x20030030, 0x6);
				break;
			}
		}
	//sc1135
		{
			 sc1135_i2c_read(0x580b,&ret_data1);
			 sc1135_i2c_read(0x3108,&ret_data2);
			 if(ret_data1 == SC1135_CHECK_DATA_LSB&&ret_data2==SC1135_CHECK_DATA_MSB){
				 _isp_attr.sensor_type = SENSOR_MODEL_SC1135;
				 sdk_sys->write_reg(0x20030030, 0x5); //set sensor clock 
				 break;
			 }		 
		}
	
	}while(0);

	isp_gpio_pin_write(0, 1, 1); //reset sensor 
	usleep(2000);
	isp_gpio_pin_write(0, 1, 0); //reset sensor 
	usleep(2000);
	isp_gpio_pin_write(0, 1, 1); //reset sensor 
	usleep(2000);
	return _isp_attr.sensor_type;
}


int HI_SDK_ISP_ircut_auto_switch(int vin, uint8_t type)//1:software   0: hardware 
{
	//static uint32_t old_satuation = 0;

	//uint32_t gpio_status_cur = isp_gpio_pin_read(IRCUT_GPIO_GROUP, IRCUT_GPIO_PIN);
	uint32_t gpio_status_cur;

	if(_isp_attr.ircut_auto_switch_enable){
		if(_isp_attr.ircut_control_mode == ISP_IRCUT_CONTROL_MODE_HARDWARE){//hardware detect
			//gpio_status_cur= isp_gpio_pin_read(IRCUT_PHOTOSWITCH_GPIO_GROUP, IRCUT_PHOTOSWITCH_GPIO_PIN);
			gpio_status_cur = _isp_attr.bsp_api.BSP_GET_PHOTOSWITCH();
		}else{//software detect
			gpio_status_cur = sdk_isp_calculate_exposure(_isp_attr.gpio_status_old);
		}
		gpio_status_cur = gpio_status_cur != 0 ? 1:0;
		if(_isp_attr.gpio_status_old != gpio_status_cur && ircut_edge_detect(&gpio_status_cur)){
			printf("%s-%d  ircut hareware switch:%d--%d\r\n", __FUNCTION__, __LINE__, _isp_attr.gpio_status_old, gpio_status_cur);
			_isp_attr.gpio_status_old = gpio_status_cur;
			printf("%d\r\n",_isp_attr.gpio_status_old);
			if(!gpio_status_cur){			
				isp_ircut_switch(0);
			}else{			
				isp_ircut_switch(1);
			}
		}
	}else{
		//do nothing
	}
	return 0;
}

int HI_SDK_ISP_set_mirror(int vin, bool mirror)
{
	VI_CHN_ATTR_S vi_chn_attr;
	SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
	vi_chn_attr.bMirror = mirror ? HI_TRUE : HI_FALSE;
	SOC_CHECK(HI_MPI_VI_SetChnAttr(vin, &vi_chn_attr));
	return 0;
}

int HI_SDK_ISP_set_flip(int vin, bool flip)
{
	VI_CHN_ATTR_S vi_chn_attr;
	SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
	vi_chn_attr.bFlip = flip ? HI_TRUE : HI_FALSE;
	SOC_CHECK(HI_MPI_VI_SetChnAttr(vin, &vi_chn_attr));
	return 0;
}


int HI_SDK_ISP_set_saturation(int vin, uint16_t val)
{
#if 0
	HI_U8 pu8Value;
	HI_MPI_ISP_SetSaturation(val);
	printf("saturation set:%d\r\n", val);
#else
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32SatuVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("saturation set:%d\r\n", val);
#endif
	return 0;
}

int HI_SDK_ISP_get_saturation(int vin, uint16_t *val)
{
/*
	HI_U32 pu8Value;
	HI_MPI_ISP_GetSaturation(&pu8Value);
	printf("saturation get:%d\r\n", pu8Value);
	*val = (uint16_t)pu8Value;
*/
	return 0;
}


int HI_SDK_ISP_set_contrast(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32ContrVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("contrast set:%d\r\n", val);
	return 0;
}

int HI_SDK_ISP_set_brightness(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32LumaVal= val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("brightness set:%d\r\n", val);
	return 0;
}

int HI_SDK_ISP_set_hue(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32HueVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	printf("hue set:%d\r\n", val);
	return 0;
}

int HI_SDK_ISP_set_src_framerate(unsigned int framerate)
{
	//VI_CHN_ATTR_S vi_chn_attr;
	ISP_IMAGE_ATTR_S isp_image_attr;
	SOC_CHECK(HI_MPI_ISP_GetImageAttr(&isp_image_attr));
	if(framerate <= 25){
	 isp_image_attr.u16FrameRate = 25;
	}else{
	isp_image_attr.u16FrameRate = 30;
	}

	//fix me
	if(SENSOR_MODEL_SC1045 == _isp_attr.sensor_type){
		isp_image_attr.u16FrameRate = 25;
	}
	
	_isp_attr.src_framerate = isp_image_attr.u16FrameRate;
	printf("\n\nisp_image_attr.u16FrameRate = %d-%d\n\n", isp_image_attr.u16FrameRate, framerate);
	//isp_image_attr.u16FrameRate = framerate;
	VI_CHN_ATTR_S vin_chn_attr;
	SOC_CHECK(HI_MPI_VI_GetChnAttr(0, &vin_chn_attr));
	vin_chn_attr.s32SrcFrameRate = isp_image_attr.u16FrameRate;
	vin_chn_attr.s32FrameRate = isp_image_attr.u16FrameRate;
	_isp_attr.filter_frequency = isp_image_attr.u16FrameRate*2;
	SOC_CHECK(HI_MPI_VI_SetChnAttr(0, &vin_chn_attr));
	SOC_CHECK(HI_MPI_ISP_SetImageAttr(&isp_image_attr));

	ISP_ANTIFLICKER_S Antiflicker;
	SOC_CHECK(HI_MPI_ISP_GetAntiFlickerAttr(&Antiflicker));
	Antiflicker.u8Frequency = _isp_attr.filter_frequency;
	SOC_CHECK(HI_MPI_ISP_SetAntiFlickerAttr(&Antiflicker));
	//fix me:fix 090sdk recover AE param when call HI_MPI_ISP_SetImageAttr
	sleep(2);
	_hi_sdk_isp_init_isp_default_value();	
	return 0;
}

int HI_SDK_ISP_get_sharpen(uint8_t *val)
{
	ISP_SHARPEN_ATTR_S SharpenAttr;
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(&SharpenAttr));
	int iso = isp_get_iso();
	if(iso < 200){
		*val = SharpenAttr.u8SharpenAltUd[0];
	}else if(iso < 400){
		*val = SharpenAttr.u8SharpenAltUd[1];
	}else if(iso < 800){
		*val = SharpenAttr.u8SharpenAltUd[2];
	}else if(iso < 1600){
		*val = SharpenAttr.u8SharpenAltUd[3];
	}else if(iso < 3200){
		*val = SharpenAttr.u8SharpenAltUd[4];
	}else if(iso < 6400){
		*val = SharpenAttr.u8SharpenAltUd[5];
	}else if(iso < 12800){
		*val = SharpenAttr.u8SharpenAltUd[6];
	}else{
		*val = SharpenAttr.u8SharpenAltUd[7];
	};
	return 0;
}


int HI_SDK_ISP_set_sharpen(uint8_t val, uint8_t bManual)
{
	ISP_SHARPEN_ATTR_S SharpenAttr;
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(&SharpenAttr));

	SharpenAttr.bEnable = HI_TRUE;
	if(bManual){
		SharpenAttr.bManualEnable = HI_TRUE;
	}else{
		SharpenAttr.bManualEnable = HI_FALSE;
	}
	/*if(SharpenAttr.u8StrengthMin <= val){
		SharpenAttr.u8StrengthTarget = val;
	}else{
		SharpenAttr.u8StrengthTarget = SharpenAttr.u8StrengthMin;
	}*/
	SharpenAttr.u8StrengthMin = 0;
	SharpenAttr.u8StrengthUdTarget = val;
	printf("%s: %d %d\r\n", __FUNCTION__, SharpenAttr.bManualEnable, SharpenAttr.u8StrengthTarget);
	
	SOC_CHECK(HI_MPI_ISP_SetSharpenAttr(&SharpenAttr));
 	return 0;
}

int HI_SDK_ISP_set_scene_mode(uint32_t mode)
{
	ISP_DRC_ATTR_S DRCAttr;
	ISP_GAMMA_TABLE_S GammaAttr;
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&GammaAttr)); 
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));
	printf("%s:%d\r\n", __FUNCTION__, mode);
	switch(mode){
		default:
		case ISP_SCENE_MODE_AUTO:
			HI_SDK_ISP_sensor_flicker(0xff,0,ISP_ANTIFLICKER_MODE_1);
			DRCAttr.bDRCEnable = HI_TRUE;
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0130:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	
					//setgamma1();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));
					break;
				case SENSOR_MODEL_OV_OV9712PLUS:
					//SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));//for 9712+
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));		
					break;
				case SENSOR_MODEL_OV_OV9712:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));		
					break;
				case SENSOR_MODEL_SOI_H22:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					//setgamma7();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));	
					memcpy(GammaAttr.u16Gamma, gs_Gamma[2], sizeof(gs_Gamma[2]));	
					break;
				case SENSOR_MODEL_SONY_IMX122:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_APTINA_AR0330:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_GC1004:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_APTINA_AR0141:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_SC1035:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	
					break;
				case SENSOR_MODEL_OV2710:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;

				case SENSOR_MODEL_SOI_H42:
					
					break;
				case SENSOR_MODEL_SC1045:
					
					
					break;
				case SENSOR_MODEL_BG0701:
	
					break;
				case SENSOR_MODEL_IMX225:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_SC1135:
	
					break;
			} 
			break;
		case ISP_SCENE_MODE_INDOOR:
			HI_SDK_ISP_sensor_flicker(0xff,0,ISP_ANTIFLICKER_MODE_0);
			DRCAttr.bDRCEnable = HI_TRUE;
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0130:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	
					//setgamma1();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));
					break;
				case SENSOR_MODEL_OV_OV9712PLUS:
					//SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));//for 9712+
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));		
					break;
				case SENSOR_MODEL_OV_OV9712:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));		
					break;
				case SENSOR_MODEL_SOI_H22:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					//setgamma7();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));	
					memcpy(GammaAttr.u16Gamma, gs_Gamma[2], sizeof(gs_Gamma[2]));
					break;
				case SENSOR_MODEL_SONY_IMX122:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_APTINA_AR0330:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_GC1004:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_APTINA_AR0141:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	
					break;				
				case SENSOR_MODEL_SC1035:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_OV2710:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_SOI_H42:
					
					break;
				case SENSOR_MODEL_SC1045:
					
					
					break;
				case SENSOR_MODEL_BG0701:
	
					break;
				case SENSOR_MODEL_IMX225:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;				
				case SENSOR_MODEL_SC1135:
	
					break;
			}
			break;
		case ISP_SCENE_MODE_OUTDOOR:
			HI_SDK_ISP_sensor_flicker(0xff,0,ISP_ANTIFLICKER_MODE_1);
			DRCAttr.bDRCEnable = HI_FALSE;
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0130:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	
					//setgamma1();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));
					break;
				case SENSOR_MODEL_OV_OV9712PLUS:
					//SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));//for 9712+
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));		
					break;
				case SENSOR_MODEL_OV_OV9712:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[3], sizeof(gs_Gamma[3]));		
					break;
				case SENSOR_MODEL_SOI_H22:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					//setgamma7();
					//memcpy(GammaAttr.u16Gamma, u16Gamma, sizeof(u16Gamma));	
					memcpy(GammaAttr.u16Gamma, gs_Gamma[2], sizeof(gs_Gamma[2]));
					break;
				case SENSOR_MODEL_SONY_IMX122:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_APTINA_AR0330:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_GC1004:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_APTINA_AR0141:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	
					break;		
				case SENSOR_MODEL_SC1035:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_OV2710:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;
				case SENSOR_MODEL_SOI_H42:
					
					break;
				case SENSOR_MODEL_SC1045:
					
					
					break;
				case SENSOR_MODEL_BG0701:
	
					break;
				case SENSOR_MODEL_IMX225:
					GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
					break;					
				case SENSOR_MODEL_SC1135:
	
					break;				
			}
			break;
	}
	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(&DRCAttr));
	//SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr)); 

	return 0;
}

int HI_SDK_ISP_set_WB_mode(uint32_t mode)
{
	ISP_ADV_AWB_ATTR_S AWBAttr;
	SOC_CHECK(HI_MPI_ISP_GetAdvAWBAttr(&AWBAttr));
	printf("%s:%d\r\n", __FUNCTION__, mode);
	switch(mode){
		default:
		case ISP_SCENE_MODE_AUTO:
			AWBAttr.bAccuPrior = HI_FALSE;
			break;
		case ISP_SCENE_MODE_INDOOR:
			AWBAttr.bAccuPrior = HI_TRUE;
			break;
		case ISP_SCENE_MODE_OUTDOOR:
			AWBAttr.bAccuPrior = HI_FALSE;
			break;
	}
	SOC_CHECK(HI_MPI_ISP_SetAdvAWBAttr(&AWBAttr));
	return 0;
}

int HI_SDK_ISP_set_ircut_control_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	_isp_attr.ircut_control_mode = mode;
	return 0;
}

int HI_SDK_ISP_set_ircut_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	switch(mode){
		default:
		case ISP_IRCUT_MODE_AUTO:
			_isp_attr.ircut_auto_switch_enable = HI_TRUE;
			break;
		case ISP_IRCUT_MODE_DAYLIGHT:
			_isp_attr.ircut_auto_switch_enable = HI_FALSE;
			isp_ircut_switch(0);
			break;
		case ISP_IRCUT_MODE_NIGHT:
			_isp_attr.ircut_auto_switch_enable = HI_FALSE;
			isp_ircut_switch(1);
			break;
	}
	return 0;
}

int HI_SDK_ISP_set_WDR_enable(uint8_t bEnable)
{
	//printf("%s:%d\r\n", __FUNCTION__, _isp_attr.isp_auto_drc_enabled);
	ISP_DRC_ATTR_S DRCAttr;
	HI_MPI_ISP_GetDRCAttr(&DRCAttr);
	DRCAttr.bDRCEnable = bEnable;
	_isp_attr.isp_auto_drc_enabled = bEnable;
	DRCAttr.bDRCManualEnable = _isp_attr.isp_auto_drc_enabled;
	if(_isp_attr.ispCfgAttr){
		DRCAttr.u32StrengthTarget = isp_get_isp_strength_value2(_isp_attr.ispCfgAttr->impCfgAttr.DrcSlope, _isp_attr.ispCfgAttr->userAttr.DrcStrength);
		printf("---------DRCAttr.u32StrengthTarget = %d-\n", DRCAttr.u32StrengthTarget);
	}
	//DRCAttr.u32StrengthTarget = isp_get_isp_strength_value(&_isp_attr.wdr);
	if(DRCAttr.u32StrengthTarget > DRCAttr.u32SlopeMax){
		DRCAttr.u32StrengthTarget = DRCAttr.u32SlopeMax;
	}
	if(DRCAttr.u32StrengthTarget < DRCAttr.u32SlopeMin){
		DRCAttr.u32StrengthTarget = DRCAttr.u32SlopeMin;
	}
	HI_MPI_ISP_SetDRCAttr(&DRCAttr);

	return 0;
}

int HI_SDK_ISP_set_WDR_strength(uint8_t val)
{
	printf("%s:%d\r\n", __FUNCTION__, val);
	ISP_DRC_ATTR_S DRCAttr;
	HI_MPI_ISP_GetDRCAttr(&DRCAttr);
	if(val >5){
		val = 5;
	}
	if(val < 1){
		val = 1;
	}
	
	_isp_attr.wdr.strength = val;
	if(_isp_attr.ispCfgAttr){
		_isp_attr.ispCfgAttr->userAttr.DrcStrength = val;
		DRCAttr.u32StrengthTarget = isp_get_isp_strength_value2(_isp_attr.ispCfgAttr->impCfgAttr.DrcSlope, _isp_attr.ispCfgAttr->userAttr.DrcStrength);
		printf("---------DRCAttr.u32StrengthTarget = %d-\n", DRCAttr.u32StrengthTarget);
		HI_ISP_cfg_set_imp(_isp_attr.gpio_status_old, _isp_attr.ispCfgAttr);
	}
	//DRCAttr.u32StrengthTarget = isp_get_isp_strength_value(&_isp_attr.wdr);
	if(DRCAttr.u32StrengthTarget > DRCAttr.u32SlopeMax){
		DRCAttr.u32StrengthTarget = DRCAttr.u32SlopeMax;
	}
	if(DRCAttr.u32StrengthTarget < DRCAttr.u32SlopeMin){
		DRCAttr.u32StrengthTarget ; DRCAttr.u32SlopeMin;
	}
	HI_MPI_ISP_SetDRCAttr(&DRCAttr);
	return 0;
}

int HI_SDK_ISP_set_exposure_mode(uint32_t mode)
{
	printf("%s:%d\r\n", __FUNCTION__, mode);
	ISP_AE_ATTR_S AEAttr;
	HI_MPI_ISP_GetAEAttr(&AEAttr);
	switch(mode){
		default:
		case ISP_EXPOSURE_MODE_AUTO:
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0130:
					AEAttr.u8ExpCompensation = 0x61;
					break;
				case SENSOR_MODEL_OV_OV9712PLUS:
					AEAttr.u8ExpCompensation = 0x50;
					break;
				case SENSOR_MODEL_OV_OV9712:
					AEAttr.u8ExpCompensation = 0x50;
					break;
				case SENSOR_MODEL_SOI_H22:
					AEAttr.u8ExpCompensation = 0x70;
					break;
				case SENSOR_MODEL_SONY_IMX122:
					AEAttr.u8ExpCompensation = 0x58;
					break;
				case SENSOR_MODEL_APTINA_AR0330:
					AEAttr.u8ExpCompensation = 0x80;
					break;
				case SENSOR_MODEL_GC1004:
					
					AEAttr.u8ExpCompensation = 0x48;
					
					break;
					
				case SENSOR_MODEL_APTINA_AR0141:					
					AEAttr.u8ExpCompensation = 0x48;
					break;
				case SENSOR_MODEL_SC1035:
					
					AEAttr.u8ExpCompensation = 0x48;
					
					break;
				case SENSOR_MODEL_SOI_H42:
					
					AEAttr.u8ExpCompensation = 0x48;
					break;				
				case SENSOR_MODEL_SC1045:
					
					
					break;
				case SENSOR_MODEL_BG0701:
	
					break;	
				case SENSOR_MODEL_IMX225:

					break;					
				case SENSOR_MODEL_SC1135:
	
					break;
									
				
			}			
			break;
			
		case ISP_EXPOSURE_MODE_BRIGHT:
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0130:
					AEAttr.u8ExpCompensation = 0xa8;
					break;
				case SENSOR_MODEL_OV_OV9712PLUS:
					AEAttr.u8ExpCompensation = 0xa0;
					break;
				case SENSOR_MODEL_OV_OV9712:
					AEAttr.u8ExpCompensation = 0xa0;
					break;
				case SENSOR_MODEL_SOI_H22:
					AEAttr.u8ExpCompensation = 0xa8;
					break;
				case SENSOR_MODEL_SONY_IMX122:
					AEAttr.u8ExpCompensation = 0x70;
					break;
				case SENSOR_MODEL_APTINA_AR0330:
					AEAttr.u8ExpCompensation = 0xa0;
					break;
				case SENSOR_MODEL_GC1004:					
					AEAttr.u8ExpCompensation = 0x60;					
					break;	
				case SENSOR_MODEL_APTINA_AR0141:					
					AEAttr.u8ExpCompensation = 0x58;
					break;			
				case SENSOR_MODEL_SC1035:
					
					AEAttr.u8ExpCompensation = 0x48;
					
					break;
				case SENSOR_MODEL_SOI_H42:
					
					AEAttr.u8ExpCompensation = 0x48;
					break;				
				case SENSOR_MODEL_SC1045:
					
					
					break;
				case SENSOR_MODEL_BG0701:
	
					break;	
				case SENSOR_MODEL_IMX225:

					break;					
				case SENSOR_MODEL_SC1135:
	
					break;
									
			}			
			break;

		case ISP_EXPOSURE_MODE_DARK:
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0130:
					AEAttr.u8ExpCompensation = 0x40;
					break;
				case SENSOR_MODEL_OV_OV9712PLUS:
					AEAttr.u8ExpCompensation = 0x30;
					break;
				case SENSOR_MODEL_OV_OV9712:
					AEAttr.u8ExpCompensation = 0x30;
					break;
				case SENSOR_MODEL_SOI_H22:
					AEAttr.u8ExpCompensation = 0x38;
					break;
				case SENSOR_MODEL_SONY_IMX122:
					AEAttr.u8ExpCompensation = 0x30;
					break;
				case SENSOR_MODEL_APTINA_AR0330:
					AEAttr.u8ExpCompensation = 0x40;
					break;
				case SENSOR_MODEL_GC1004:					
					AEAttr.u8ExpCompensation = 0x20;					
					break;	
				case SENSOR_MODEL_APTINA_AR0141:					
					AEAttr.u8ExpCompensation = 0x40;
					break;	
				case SENSOR_MODEL_SC1035:
					
					AEAttr.u8ExpCompensation = 0x48;
					
					break;
				case SENSOR_MODEL_SOI_H42:
					
					AEAttr.u8ExpCompensation = 0x48;
					break;			
				case SENSOR_MODEL_SC1045:
					
					
					break;
				case SENSOR_MODEL_BG0701:
	
					break;		
				case SENSOR_MODEL_IMX225:

					break;					
				case SENSOR_MODEL_SC1135:
	
					break;
								
			}			
			break;
	}
	//HI_MPI_ISP_SetAEAttr(&AEAttr);
	return 0;
}

int HI_SDK_ISP_set_AEcompensation(uint8_t val)
{
	ISP_AE_ATTR_S AEAttr;
	SOC_CHECK(HI_MPI_ISP_GetAEAttr(&AEAttr));
	if(val >5){
		val = 5;
	}else if(val < 1){
		val = 1;
	}
	_isp_attr.aeCompensition.strength = val;
	AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
	SOC_CHECK(HI_MPI_ISP_SetAEAttr(&AEAttr));
	return 0;
}

int HI_SDK_ISP_set_denoise_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);
	VPSS_GRP_PARAM_S VpssParam;
	HI_MPI_VPSS_GetGrpParam(0, &VpssParam);
	return 0;
}

int HI_SDK_ISP_get_denoise_strength(uint8_t *val)
{

	return 0;
}


int HI_SDK_ISP_set_denoise_strength(uint8_t val)
{
	VPSS_GRP_PARAM_S vpss_grp_param;
	SOC_CHECK(HI_MPI_VPSS_GetGrpParam(0, &vpss_grp_param));
	if(val >5){
		val = 5;
	}
	if(val < 1){
		val = 1;
	}
	_isp_attr.denoise3d.strength = val;
	vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
	vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
	vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
	SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
	return 0;
}

int HI_SDK_ISP_set_advance_anti_fog_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);
	ISP_ANTIFOG_S AntiFog;
	AntiFog.bEnable = bEnable;
	HI_MPI_ISP_SetAntiFogAttr(&AntiFog);
	return 0;
}

int HI_SDK_ISP_set_advance_lowlight_enable(uint8_t bEnable)
{
	printf("%s:%d\r\n", __FUNCTION__, bEnable);
	_isp_attr.lowlight_mode = bEnable;
	/*ISP_AE_ATTR_EX_S AEAttr;
	SOC_CHECK(HI_MPI_ISP_GetAEAttrEx(&AEAttr));	
	if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO){
		AEAttr.enAEMode = AE_MODE_LOW_NOISE;
	}else{
		AEAttr.enAEMode = AE_MODE_FRAME_RATE;
	}
	SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));	*/
	if(_isp_attr.ispCfgAttr){
		_isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate = _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO ? true : false;
		printf("AutoSlowFrameRate:%d\r\n", _isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate);
	}
	if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY){
		_hi_sdk_isp_set_slow_framerate(true);
	}else if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_NIGHT && _isp_attr.gpio_status_old == ISP_GPIO_NIGHT){
		_hi_sdk_isp_set_slow_framerate(true);
	}else{
		_hi_sdk_isp_set_slow_framerate(false);
	}
	return 0;
}

int HI_SDK_ISP_set_advance_gamma_table(uint8_t val)
{
	ISP_GAMMA_TABLE_S GammaAttr;
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&GammaAttr)); 
	memcpy(GammaAttr.u16Gamma, gs_Gamma[val], sizeof(gs_Gamma[val]));
	SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr)); 
	return 0;
}

int HI_SDK_ISP_set_advance_defect_pixel_enable(uint8_t bEnable)
{
	ISP_DP_ATTR_S DPAttr;
	HI_MPI_ISP_GetDefectPixelAttr(&DPAttr);
	printf("DPAttr.bEnableStatic:%d\r\n", DPAttr.bEnableStatic);
	printf("DPAttr.bEnableDynamic:%d\r\n", DPAttr.bEnableDynamic);
	printf("DPAttr.bEnableDetect:%d\r\n", DPAttr.bEnableDetect);
	HI_MPI_ISP_SetDefectPixelAttr(&DPAttr);
	return 0;
}

int HI_SDK_ISP_get_color_max_value(stSensorColorMaxValue *ret_value)
{
	memcpy(ret_value, &_isp_attr.color_max_value, sizeof(stSensorColorMaxValue));
	return 0;
}


int HI_SDK_ISP_get_sensor_model(emSENSOR_MODEL *ret_value)
{
	*ret_value = _isp_attr.sensor_type;
	return 0;
}

int HI_SDK_ISP_set_sensor_resolution(uint32_t width, uint32_t height)
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
		case SENSOR_MODEL_OV_OV9712PLUS:
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
		case SENSOR_MODEL_GC1004:
			_isp_attr.sensor_resolution_height = 720;
			_isp_attr.sensor_resolution_width = 1280;	
			break;			
		case SENSOR_MODEL_APTINA_AR0141:
			_isp_attr.sensor_resolution_height = 720;
			_isp_attr.sensor_resolution_width = 1280;						
			break;
		case SENSOR_MODEL_OV2710:
		    _isp_attr.sensor_resolution_height = 1080;
			_isp_attr.sensor_resolution_width = 1920;
			break;
		case SENSOR_MODEL_SOI_H42:
			_isp_attr.sensor_resolution_height = 720;
			_isp_attr.sensor_resolution_width = 1280;
			break;
		case SENSOR_MODEL_SC1045:
			_isp_attr.sensor_resolution_height = 720;
			_isp_attr.sensor_resolution_width = 1280;
			break;

		case SENSOR_MODEL_BG0701:
		    _isp_attr.sensor_resolution_height = 720;
			_isp_attr.sensor_resolution_width = 1280;
			break;
		case SENSOR_MODEL_IMX225:
		    _isp_attr.sensor_resolution_height = 960;
			_isp_attr.sensor_resolution_width = 1280;
			break;				
		case SENSOR_MODEL_SC1135:
			_isp_attr.sensor_resolution_height = 960;
			_isp_attr.sensor_resolution_width = 1280;
			break;
	}
	return 0;
}

int HI_SDK_ISP_get_sensor_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
	*ret_width = _isp_attr.sensor_resolution_width;
	*ret_height = _isp_attr.sensor_resolution_height;
	return 0;
}

extern int HI_SDK_ISP_get_sensor_defect_pixel_table(void)
{
	return 0;
}
extern int HI_SDK_ISP_set_sensor_defect_pixel_table(void)
{
	return 0;
}

static uint8_t cal_isp_function_opened(int iso_border, int iso, uint8_t isBrightOpened)
{
	uint8_t cur_val;
	if(isBrightOpened){//if iso_border >= iso --> open function
		if(iso_border >= iso){
			cur_val = true;//open function
		}else{
			cur_val = false;//close function
		}
	}else{//if iso_border <= iso --> open function
		if(iso_border <= iso){
			cur_val = true;
		}else{
			cur_val = false;
		}
	}
	return cur_val;
}

static int get_histslope(int iso)
{
	int histslope = isp_get_isp_value(&_isp_attr.aeHistSlope), ret_histslope, iso_top, iso_bot, slope_top, slope_bot;
	int slope_default = 0x80;//for the default histslope with big iso
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0130:
			iso_top = 11;
			iso_bot = 4;
			slope_top = 0x200;
			slope_bot = slope_default;
			break;
		case SENSOR_MODEL_OV_OV9712PLUS:
		case SENSOR_MODEL_OV_OV9712:
		case SENSOR_MODEL_APTINA_AR0330:
			iso_top = 4;
			iso_bot = 4;
			slope_top = histslope;
			slope_bot = slope_default;
			break;
		case SENSOR_MODEL_SOI_H22:
			iso_top = 31;
			iso_bot = 31;
			slope_top = histslope;
			slope_default = histslope;
			slope_bot = slope_default;
			break;
		case SENSOR_MODEL_SONY_IMX122:		
			iso_top = 50;
			iso_bot = 16;
			slope_top = 0x200;
			//slope_default = histslope;
			slope_bot = 0x100;
			break;			
		case SENSOR_MODEL_GC1004:
			iso_top = 3;
			iso_bot = 3;
			slope_top = histslope;
			slope_bot = slope_default;
			break;
		case SENSOR_MODEL_APTINA_AR0141:
			iso_top = 40;
			iso_bot = 3;
			slope_top = histslope;
			slope_bot = slope_default;
			break;
		case SENSOR_MODEL_SC1035:
			iso_top = 3;
			iso_bot = 3;
			slope_top = histslope;
			slope_bot = slope_default;
			
			break;
		
	}
	if(iso <= iso_bot){
		ret_histslope = histslope;
	}else if(iso > iso_top){
		ret_histslope = slope_default;
	}else{// slope_bot<iso<=slope_top
		ret_histslope = slope_bot + ((slope_top - slope_bot) *(iso_top - iso + 1))/(iso_top - iso_bot + 1);
	}

	if(ret_histslope < 0x80){
		ret_histslope = 0x80;
	}
	return ret_histslope;
}

static HI_U8 get_denoise3d(int iso)
{
	HI_U8 sfstrength_top, sfstrength_bot, sfstrength;
	switch(_isp_attr.sensor_type){
		case SENSOR_MODEL_APTINA_AR0141:
			sfstrength_top = isp_get_isp_strength_value(&_isp_attr.denoise3d);
			sfstrength_bot = (HI_U8)isp_get_isp_strength_value(&_isp_attr.denoise3d)/4;
			break;
		default:
			sfstrength_top = sfstrength_bot = isp_get_isp_strength_value(&_isp_attr.denoise3d);
			break;
	}
	sfstrength = (HI_U8)(sfstrength_bot + (sfstrength_top - sfstrength_bot)*(log(iso)/log(2))/5);
	//printf("sfstrength = %d\n", sfstrength);
	return sfstrength;
}


static HI_U8 get_demosaic(int iso)
{
	HI_U8 uuslope_top, uuslope_bot, uuslope;
	switch(_isp_attr.sensor_type){
		case SENSOR_MODEL_APTINA_AR0141:
			uuslope_top = isp_get_isp_value(&_isp_attr.demosaic);
			uuslope_bot = uuslope_top - 30;
			break;
		default:
			uuslope_top = uuslope_bot = isp_get_isp_value(&_isp_attr.demosaic);
			break;
	}
	uuslope = (HI_U8)(uuslope_top-(uuslope_top - uuslope_bot)*(log(iso)/log(2))/5);
	//printf("uuslope = %d/%d\n", iso, uuslope);
	return uuslope;
}


static void reset_ar0130_BLC()
{
	static HI_U32 i = 0;	
	if(++i%60 == 0){//60 seconds
		ar0130_i2c_write(0x3180, 0xc000);
		usleep(90000);//after 1 frame
		ar0130_i2c_write(0x3180, 0x8000);
	}
}

static void do_isp_common(uint32_t iso)
{
    int m_iso = iso / 1024;
	static uint8_t framerate_status = false;
	uint8_t isp_auto_drc_status;
	uint8_t AntiFlicker_status;
	ISP_AE_ATTR_EX_S AEAttr;
	ISP_DP_ATTR_S  DPAttr;	
	VPSS_GRP_PARAM_S vpss_grp_param;
	
	int histslope = 0;
	SOC_CHECK(HI_MPI_ISP_GetAEAttrEx(&AEAttr));	
	SOC_CHECK(HI_MPI_ISP_GetDefectPixelAttr(&DPAttr));
	
	SOC_CHECK(HI_MPI_VPSS_GetGrpParam(0, &vpss_grp_param));	
	switch(_isp_attr.sensor_type){
		case SENSOR_MODEL_SONY_IMX122:
		case SENSOR_MODEL_APTINA_AR0130:
			{
				if(m_iso < 1){
					DPAttr.u16DynamicBadPixelSlope = 512;
					DPAttr.u16DynamicBadPixelThresh = 64;
					isp_auto_drc_status = true;
					framerate_status = false;//false for normol frame rate;true for slow frame rate
					AntiFlicker_status = true;
					DPAttr.u16DynamicBadPixelSlope = 512;
					DPAttr.u16DynamicBadPixelThresh = 64;
				}else if(m_iso < 2){
					DPAttr.u16DynamicBadPixelSlope = 512;
					DPAttr.u16DynamicBadPixelThresh = 64;
					isp_auto_drc_status = true;
					framerate_status = false;
					AntiFlicker_status = true;
					DPAttr.u16DynamicBadPixelSlope = 512;
					DPAttr.u16DynamicBadPixelThresh = 64;
				}else if(m_iso < 4){
					DPAttr.u16DynamicBadPixelSlope = 512;
					DPAttr.u16DynamicBadPixelThresh = 64;
					isp_auto_drc_status = true;
					framerate_status = false;
					AntiFlicker_status = true;	
					DPAttr.u16DynamicBadPixelSlope = 512;
					DPAttr.u16DynamicBadPixelThresh = 64;
				}else if(m_iso < 8){
					DPAttr.u16DynamicBadPixelSlope = 768;
					DPAttr.u16DynamicBadPixelThresh = 50;
					isp_auto_drc_status = false;
					framerate_status = false;
					AntiFlicker_status = false;	
					DPAttr.u16DynamicBadPixelSlope = 768;
					DPAttr.u16DynamicBadPixelThresh = 48;
				}else if(m_iso < 16){
					DPAttr.u16DynamicBadPixelSlope = 1024;
					DPAttr.u16DynamicBadPixelThresh = 40;
					isp_auto_drc_status = false;
					framerate_status = false;
					AntiFlicker_status = false;	
					DPAttr.u16DynamicBadPixelSlope = 1024;
					DPAttr.u16DynamicBadPixelThresh = 30;
				}else if(m_iso < 32){
					DPAttr.u16DynamicBadPixelSlope = 1024;
					DPAttr.u16DynamicBadPixelThresh = 30;
					isp_auto_drc_status = false;
					AntiFlicker_status = false;
					DPAttr.u16DynamicBadPixelSlope = 1024;
					DPAttr.u16DynamicBadPixelThresh = 30;
				}else if(m_iso < 64){
					DPAttr.u16DynamicBadPixelSlope = 1024;
					DPAttr.u16DynamicBadPixelThresh = 30;
					isp_auto_drc_status = false;
					AntiFlicker_status = false;
					if(m_iso > 32){
						framerate_status = true;
					}
					DPAttr.u16DynamicBadPixelSlope = 1024;
					DPAttr.u16DynamicBadPixelThresh = 30;
				}else{//ISO > 64 
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
				}
				if(SENSOR_MODEL_APTINA_AR0130 == _isp_attr.sensor_type){
					reset_ar0130_BLC();
				}
			}	
		break;
		case SENSOR_MODEL_GC1004:
			{
				if(m_iso < 1){
					isp_auto_drc_status = true;
					framerate_status = false;//false for normol frame rate;true for slow frame rate
					AntiFlicker_status = true;
			//		DPAttr.u16DynamicBadPixelSlope = 512;
			//		DPAttr.u16DynamicBadPixelThresh = 64;
					DPAttr.u16DynamicBadPixelSlope = 0x200;
					DPAttr.u16DynamicBadPixelThresh = 0x40;
					_isp_attr.denoise3d.daylight_val = 0x8;
				    _isp_attr.denoise3d.night_val = 0x10;
				}else if(m_iso < 2){
					isp_auto_drc_status = true;
					framerate_status = false;
					AntiFlicker_status = true;
					DPAttr.u16DynamicBadPixelSlope = 0x300;//550;
					DPAttr.u16DynamicBadPixelThresh = 0x30;//55;				
					_isp_attr.denoise3d.daylight_val = 0xc;
				    _isp_attr.denoise3d.night_val = 0x14;
				}else if(m_iso < 4){
					isp_auto_drc_status = false;
					AntiFlicker_status = true;
					//framerate_status = false;
					DPAttr.u16DynamicBadPixelSlope = 0x380;//600;
					DPAttr.u16DynamicBadPixelThresh = 0x28;//45;
				    _isp_attr.denoise3d.daylight_val = 0x10;
				    _isp_attr.denoise3d.night_val = 0x18;
				}else if(m_iso < 8){
					isp_auto_drc_status = false;
					AntiFlicker_status = false;
					if(m_iso > 4){
						framerate_status = true;
					}
					DPAttr.u16DynamicBadPixelSlope = 0x400;//800
					DPAttr.u16DynamicBadPixelThresh = 0x20;//35;
					_isp_attr.denoise3d.daylight_val = 0x18;
				    _isp_attr.denoise3d.night_val = 0x20;
				}else if(m_iso < 16){
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
					DPAttr.u16DynamicBadPixelSlope = 0x400;//1400;
					DPAttr.u16DynamicBadPixelThresh = 0x20;//20;				  
					_isp_attr.denoise3d.daylight_val = 0x1b;
				    _isp_attr.denoise3d.night_val = 0x26;
				}else if(m_iso < 32){
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;					
					DPAttr.u16DynamicBadPixelSlope = 0x600;//2000;
					DPAttr.u16DynamicBadPixelThresh = 0x10;//10;
					_isp_attr.denoise3d.daylight_val = 0x20;
				    _isp_attr.denoise3d.night_val = 0x2b;
				}else if(m_iso < 64){
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
					DPAttr.u16DynamicBadPixelSlope = 0x700;//1600;
					DPAttr.u16DynamicBadPixelThresh = 0x10;//16;
					_isp_attr.denoise3d.daylight_val = 0x24;
				    _isp_attr.denoise3d.night_val = 0x30;
				}else{//ISO > 64 
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
					DPAttr.u16DynamicBadPixelSlope = 0x500;//1600;
					DPAttr.u16DynamicBadPixelThresh = 0x20;//16;
					_isp_attr.denoise3d.daylight_val = 0x20;
				    _isp_attr.denoise3d.night_val = 0x30;
				}
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
			}
		break;
		case SENSOR_MODEL_APTINA_AR0141:
			{
				if(m_iso < 1){
					isp_auto_drc_status = true;
					framerate_status = false;//false for normol frame rate;true for slow frame rate
					AntiFlicker_status = true;
				}else if(m_iso < 2){
					isp_auto_drc_status = true;
					framerate_status = false;
					AntiFlicker_status = true;
				}else if(m_iso < 4){
					isp_auto_drc_status = true;
					AntiFlicker_status = true;
					framerate_status = false;
				}else if(m_iso < 8){
					isp_auto_drc_status = true;
					AntiFlicker_status = true;
					framerate_status = false;
				}else if(m_iso < 16){
					isp_auto_drc_status = false;
					
					AntiFlicker_status = false;
					if(m_iso < 10){
						framerate_status = false;
					}
				}else if(m_iso < 32){
					isp_auto_drc_status = false;
					//framerate_status = false;
					AntiFlicker_status = false;
				}else if(m_iso < 64){
					isp_auto_drc_status = false;			
					AntiFlicker_status = false;
					if(m_iso>40){
						framerate_status = true;
					}
				}else{//ISO > 64 
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
				}
			}
		break;
		case SENSOR_MODEL_SC1035:
		{
			if(m_iso < 1){
				isp_auto_drc_status = true;
				framerate_status = false;//false for normol frame rate;true for slow frame rate
				AntiFlicker_status = true;
				DPAttr.u16DynamicBadPixelSlope = 512;
				DPAttr.u16DynamicBadPixelThresh = 64;
			}else if(m_iso < 2){
				isp_auto_drc_status = true;
				framerate_status = false;
				AntiFlicker_status = true;
				DPAttr.u16DynamicBadPixelSlope = 512;
				DPAttr.u16DynamicBadPixelThresh = 64;
			}else if(m_iso < 4){
				isp_auto_drc_status = true;
				AntiFlicker_status = true;
				framerate_status = false;
				DPAttr.u16DynamicBadPixelSlope = 512;
				DPAttr.u16DynamicBadPixelThresh = 64;
			}else if(m_iso < 8){
				isp_auto_drc_status = false;
				AntiFlicker_status = true;
				framerate_status = false;
				DPAttr.u16DynamicBadPixelSlope = 768;
				DPAttr.u16DynamicBadPixelThresh = 64;
			}else if(m_iso < 16){
				isp_auto_drc_status = false;
				framerate_status = false;
				AntiFlicker_status = false;
				DPAttr.u16DynamicBadPixelSlope = 768;
				DPAttr.u16DynamicBadPixelThresh = 40;
			}else if(m_iso < 32){
				isp_auto_drc_status = false;
				framerate_status = false;
				AntiFlicker_status = false;
				DPAttr.u16DynamicBadPixelSlope = 1024;
				DPAttr.u16DynamicBadPixelThresh = 30;
			}else if(m_iso < 64){
				isp_auto_drc_status = false;			
				AntiFlicker_status = false;
				DPAttr.u16DynamicBadPixelSlope = 1024;
				DPAttr.u16DynamicBadPixelThresh = 30;
			}else{//ISO > 64 
				isp_auto_drc_status = false;
				AntiFlicker_status = false;
				if(m_iso>102){
					framerate_status = true;
				}
				DPAttr.u16DynamicBadPixelSlope = 1024;
				DPAttr.u16DynamicBadPixelThresh = 30;
			}
		}
		break;
		case SENSOR_MODEL_SOI_H42:
			{
				if(m_iso < 1){
					isp_auto_drc_status = true;
					framerate_status = false;//false for normol frame rate;true for slow frame rate
					AntiFlicker_status = true;
				}else if(m_iso < 2){
					isp_auto_drc_status = true;
					framerate_status = false;
					AntiFlicker_status = true;
				}else if(m_iso < 4){
					isp_auto_drc_status = false;
					AntiFlicker_status = true;
					framerate_status = false;
				}else if(m_iso < 8){
					isp_auto_drc_status = false;
					AntiFlicker_status = false;
					//framerate_status = false;
				}else if(m_iso < 16){
					isp_auto_drc_status = false;					
					AntiFlicker_status = false;
					if(m_iso > 12){
						
						framerate_status = false;
						}
				}else if(m_iso < 32){
					isp_auto_drc_status = false;
					framerate_status = false;
					AntiFlicker_status = false;
				}else if(m_iso < 64){
					isp_auto_drc_status = false;
					framerate_status = false;
					AntiFlicker_status = false;
				}else{//ISO > 64 
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
				}
			}
		break;
		case SENSOR_MODEL_OV_OV9712:
		case SENSOR_MODEL_OV_OV9712PLUS:
		case SENSOR_MODEL_SOI_H22:
		case SENSOR_MODEL_APTINA_AR0330:
		default:
			{
				if(m_iso < 1){
					isp_auto_drc_status = true;
					framerate_status = false;//false for normol frame rate;true for slow frame rate
					AntiFlicker_status = true;
				}else if(m_iso < 2){
					isp_auto_drc_status = true;
					framerate_status = false;
					AntiFlicker_status = true;
				}else if(m_iso < 4){
					isp_auto_drc_status = false;
					AntiFlicker_status = true;
					//framerate_status = false;
				}else if(m_iso < 8){
					isp_auto_drc_status = false;
					AntiFlicker_status = false;
					if(m_iso > 6){
						framerate_status = true;
					}
				}else if(m_iso < 16){
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
				}else if(m_iso < 32){
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
				}else if(m_iso < 64){
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
				}else{//ISO > 64 
					isp_auto_drc_status = false;
					framerate_status = true;
					AntiFlicker_status = false;
				}
			}
		break;
	}

	//SOC_CHECK(HI_MPI_ISP_SetDefectPixelAttr(&DPAttr));	
	//SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
	

	//fix me
	/*histslope = get_histslope(m_iso);
	if(histslope != AEAttr.u16HistRatioSlope){
		AEAttr.u16HistRatioSlope = histslope;
		SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
	}*/
	
	//printf("m_iso:%d/%d-%d/%d-%d\r\n", iso, m_iso, framerate_status, _isp_attr.isp_framerate_status, _isp_attr.lowlight_mode);
	//slow frame rate setup while iso changed between 4 and 8
	if(_isp_attr.isp_framerate_status!= framerate_status && ISP_LOWLIGHT_MODE_AUTO == _isp_attr.lowlight_mode){
		//_hi_sdk_isp_set_slow_framerate(framerate_status);
	}

	//auto drc
	/*if(_isp_attr.isp_auto_drc_enabled != isp_auto_drc_status){
		HI_SDK_ISP_set_WDR_enable(isp_auto_drc_status);
		_isp_attr.isp_auto_drc_enabled = isp_auto_drc_status;
	}*/
	/*ISP_DRC_ATTR_S DRCAttr;
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));
	if(DRCAttr.bDRCEnable != isp_auto_drc_status){
		DRCAttr.bDRCEnable = isp_auto_drc_status;
		SOC_CHECK(HI_MPI_ISP_SetDRCAttr(&DRCAttr));
	}*/
	
	//AntiFlicker
	
	ISP_ANTIFLICKER_S pstAntiflicker;
	SOC_CHECK(HI_MPI_ISP_GetAntiFlickerAttr(&pstAntiflicker));
	if(pstAntiflicker.bEnable != AntiFlicker_status){
		pstAntiflicker.bEnable = AntiFlicker_status;
		SOC_CHECK(HI_MPI_ISP_SetAntiFlickerAttr(&pstAntiflicker));
	}

	//Demosaic
	/*ISP_DEMOSAIC_ATTR_S stDemosaicAttr;
	HI_U8 uuslope = get_demosaic(m_iso);
	SOC_CHECK(HI_MPI_ISP_GetDemosaicAttr(&stDemosaicAttr));
	if(stDemosaicAttr.u8UuSlope != uuslope){
		stDemosaicAttr.u8UuSlope = uuslope;
		SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
	}*/

	//3D denoise
	/*HI_U32 SfStrength = get_denoise3d(m_iso);
	if(vpss_grp_param.u32SfStrength != SfStrength){
		vpss_grp_param.u32SfStrength = SfStrength;
		vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
		vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
		SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
	}*/

	HI_ISP_cfg_set_imp_single(_isp_attr.gpio_status_old, _isp_attr.ispCfgAttr);
}

static HI_S32  soih22_reset_OB_reg(HI_U32 AGainValue, HI_U32 DGainValue)
{
	float A = 46.7;
	float B = 17.0;
	float C = 46.7;
	float a = 16;
	float b = -16;
	
	int ob;
	float Const;
	float Delta = 32;//32
	static HI_U32 AGainValue_old = 1;
	printf("%s***%d** = %d-%d\n",__FUNCTION__,__LINE__,AGainValue, DGainValue);
	AGainValue =  (int)(0.5 + (float)AGainValue/1024) * (int)(0.5 + ((float)DGainValue/1024));
	printf("%s***%d**after = %d\n",__FUNCTION__,__LINE__,AGainValue);
	if(AGainValue_old != AGainValue && _isp_attr.gpio_status_old == ISP_GPIO_NIGHT){//change OB only at night
		//Const = (B - A)/(b -a);
		//Delta = (C -B) / Const;				
	    ob = a - (int)(AGainValue * Delta/ 32);
		printf("%s***%d**The new OB value = %d-%d\n",__FUNCTION__,__LINE__,AGainValue, ob);
		if(ob < 0){
			ob = abs(ob)+0x80;
		}
		soih22_i2c_write(0x49, ob);

		//for uuSlope setting
		ISP_DEMOSAIC_ATTR_S stDemosaicAttr;
		SOC_CHECK(HI_MPI_ISP_GetDemosaicAttr(&stDemosaicAttr));
		stDemosaicAttr.u8UuSlope = (HI_U8)(201-(201 - 100)*(log(AGainValue)/log(2))/5);
		printf("u8UuSlope = %d\n",stDemosaicAttr.u8UuSlope);
		SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));

		AGainValue_old = AGainValue;
	}
	return 0;	
}


static void do_isp_sensor(uint32_t iso)
{
	ISP_INNER_STATE_INFO_EX_S pstInnerStateInfo;
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0130:
			break;
		case SENSOR_MODEL_OV_OV9712PLUS:
			break;
		case SENSOR_MODEL_OV_OV9712:
			break;
		case SENSOR_MODEL_SOI_H22:	
			SOC_CHECK(HI_MPI_ISP_QueryInnerStateInfoEx(&pstInnerStateInfo));
			soih22_reset_OB_reg(pstInnerStateInfo.u32AnalogGain, pstInnerStateInfo.u32DigitalGain);
			break;
		case SENSOR_MODEL_SONY_IMX122:
			break;
		case SENSOR_MODEL_APTINA_AR0330:
			break;
		case SENSOR_MODEL_GC1004:
			break;
		case SENSOR_MODEL_APTINA_AR0141:
			break;
		case SENSOR_MODEL_SC1035:
			break;
		case SENSOR_MODEL_SOI_H42:
			break;
	}
}

uint32_t HI_SDK_ISP_get_gain(void)
{
	uint32_t ret_gain;
	ret_gain = gains_calculate();

	//all the sensor
	do_isp_common(ret_gain);
	
	//the own isp setting for each sensor
	//do_isp_sensor(ret_gain);
	return ret_gain;
}

void HI_SDK_ISP_set_AF_attr(stSensorAfAttr *pAfAttr)
{
	if(_isp_attr.AfAttr.param){
		free(_isp_attr.AfAttr.param);
		_isp_attr.AfAttr.param = NULL;
	}
	_isp_attr.AfAttr.param = pAfAttr->param;
	_isp_attr.AfAttr.af_callback = pAfAttr->af_callback;
	
}

static int _hi_sdk_isp_set_slow_framerate(uint8_t bValue)
{
	uint8_t Value = bValue ? 0x1:0x0;
	int actual_fps;
	VENC_CHN_ATTR_S vencChannelAttr;
	VI_CHN_ATTR_S vi_chn_attr;

	
	_isp_attr.isp_framerate_status = Value;
	
	SOC_CHECK(HI_MPI_VI_GetChnAttr(0, &vi_chn_attr));
	actual_fps = vi_chn_attr.s32FrameRate/(1 << Value);

	printf("slow frame rate:%d\n", actual_fps);
	HI_MPI_VENC_GetChnAttr(0,&vencChannelAttr); 
	switch(vencChannelAttr.stRcAttr.enRcMode){
		default:
		case VENC_RC_MODE_H264VBRv2:
			if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate){//slow framerate
				
				vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate;
			}else{//actual framerate
				vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = vi_chn_attr.s32FrameRate;
			}
			//vencChannelAttr.stRcAttr.stAttrH264Vbr.u32Gop = vencChannelAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate*2;
			break;
		case VENC_RC_MODE_H264CBRv2:
			if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32TargetFrmRate){//slow framerate
				vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32TargetFrmRate;
			}else{//actual framerate
				vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate = vi_chn_attr.s32FrameRate;
			}
			//vencChannelAttr.stRcAttr.stAttrH264Cbr.u32Gop = vencChannelAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate*2;
			break;		
	}
	if(Value){
		SOC_CHECK(HI_MPI_ISP_SetSlowFrameRate(1 << (4 + Value)));
		usleep(100*1000);
		HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr); 
	}else{
		HI_MPI_VENC_SetChnAttr(0,&vencChannelAttr); 
		usleep(100*1000);
		SOC_CHECK(HI_MPI_ISP_SetSlowFrameRate(1 << (4 + Value)));	
	}
}

static int _hi_sdk_isp_init_isp_default_value(void)
{
	ISP_GAMMA_TABLE_S GammaAttr;
	ISP_AWB_ATTR_S AWBAttr;
	ISP_AE_ATTR_EX_S AEAttr;	
	ISP_SHADING_ATTR_S SHADINGAttr;
	ISP_DP_ATTR_S DPAttr;
	ISP_DEMOSAIC_ATTR_S stDemosaicAttr;
	ISP_DRC_ATTR_S DRCAttr;
	
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&GammaAttr)); 
	SOC_CHECK(HI_MPI_ISP_GetAEAttrEx(&AEAttr));	
	SOC_CHECK(HI_MPI_ISP_GetAWBAttr(&AWBAttr));
	SOC_CHECK(HI_MPI_ISP_GetShadingAttr(&SHADINGAttr));
	SOC_CHECK(HI_MPI_ISP_GetDemosaicAttr(&stDemosaicAttr));
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));
	//AEAttr.enAEMode = AE_MODE_LOW_NOISE;
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0130:
			{
				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 960;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x200;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x40;
				_isp_attr.aeCompensition.night_val = 0x38;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x14;
				_isp_attr.denoise3d.night_val = 0x30;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xBC;
				_isp_attr.demosaic.night_val = 0xC0;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x35;
				_isp_attr.wdr.night_val = 0x35;

				AEAttr.u32SystemGainMax = 0x8000;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);

				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	
				//to reduce the white pixel
				HI_MPI_ISP_GetDefectPixelAttr(&DPAttr);
				DPAttr.u16DynamicBadPixelSlope = 1024;
				DPAttr.u16DynamicBadPixelThresh = 20;
				//HI_MPI_ISP_SetDefectPixelAttr(&DPAttr);
			}
		break;
		case SENSOR_MODEL_OV_OV9712PLUS:
			{				
				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 720;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x100;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x48;
				_isp_attr.aeCompensition.night_val = 0x30;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x20;
				_isp_attr.denoise3d.night_val = 0x50;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xc8;
				_isp_attr.demosaic.night_val = 0xc8;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x40;
				_isp_attr.wdr.night_val = 0x40;

				AEAttr.u32AGainMax = 0x4000;
				AEAttr.u32SystemGainMax = 0x4800;
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);

				
				AWBAttr.u8RGStrength = 0x74;
				AWBAttr.u8BGStrength = 0x78;
				//SOC_CHECK(HI_MPI_ISP_SetAWBAttr(&AWBAttr));
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
			}
		break;
		case SENSOR_MODEL_OV_OV9712:
			{				
				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 720;			
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x300;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 10;
				_isp_attr.aeCompensition.night_val = 0x2a;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x10;
				_isp_attr.denoise3d.night_val = 0x50;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xe9;
				_isp_attr.demosaic.night_val = 0xe9;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x50;
				_isp_attr.wdr.night_val = 0x50;

				AEAttr.u32AGainMax = 0x1400;
				AEAttr.u32SystemGainMax = 0x3000;
				//AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.enFrameEndUpdateMode = 0x2;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);

				AWBAttr.u8RGStrength = 0x42;
				AWBAttr.u8BGStrength = 0x7b;
				SOC_CHECK(HI_MPI_ISP_SetAWBAttr(&AWBAttr));
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
			}
		break;
		case SENSOR_MODEL_SOI_H22:
			{				
				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 720;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x300;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x50;
				_isp_attr.aeCompensition.night_val = 0x50;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x14;
				_isp_attr.denoise3d.night_val = 0x60;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xC0;
				_isp_attr.demosaic.night_val = 0x8C;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x20;
				_isp_attr.wdr.night_val = 0x20;

				ISP_SHADING_ATTR_S ShadingAttr;
				SOC_CHECK(HI_MPI_ISP_GetShadingAttr(&ShadingAttr));
				ShadingAttr.Enable = HI_FALSE;
				SOC_CHECK(HI_MPI_ISP_SetShadingAttr(&ShadingAttr));

				AEAttr.u32DGainMax = 0x400;
				//AEAttr.u32AGainMax = 0x1400;
				AEAttr.u32ISPDGainMax = 0xB00;
				//AEAttr.u32SystemGainMax = 0x2c00;
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
				
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
			}
		break;
		case SENSOR_MODEL_SONY_IMX122:
			{					
				_isp_attr.sensor_resolution_width = 1920;
				_isp_attr.sensor_resolution_height = 1080;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x200;
				_isp_attr.aeHistSlope.night_val = 0x900;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x48;
				_isp_attr.aeCompensition.night_val = 0x40;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x20;
				_isp_attr.denoise3d.night_val = 0x50;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xaa;
				_isp_attr.demosaic.night_val = 0xaa;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x15;
				_isp_attr.wdr.night_val = 0x15;

				AEAttr.s16ExpTolerance = 0x8;
				AEAttr.u16EVBias = 1024;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);

				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
				VPSS_CHN_SP_PARAM_S ChnSpParam;
				HI_MPI_VPSS_GetChnSpParam(0, 1, &ChnSpParam);
				ChnSpParam.u32LumaGain = 50;
				HI_MPI_VPSS_SetChnSpParam(0, 1, &ChnSpParam);
			}
		break;
		case SENSOR_MODEL_APTINA_AR0330:
			{					
				_isp_attr.sensor_resolution_width = 1920;
				_isp_attr.sensor_resolution_height = 1080;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x400;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x50;
				_isp_attr.aeCompensition.night_val = 0x50;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x30;
				_isp_attr.denoise3d.night_val = 0x3c;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xc2;
				_isp_attr.demosaic.night_val = 0xc2;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x20;
				_isp_attr.wdr.night_val = 0x20;

				AEAttr.u32SystemGainMax = 0x7500;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
				
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
			}
		break;
		case SENSOR_MODEL_GC1004:
			{				
                _isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 720;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x80;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x14;
				_isp_attr.denoise3d.night_val = 0x40;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x68;
				_isp_attr.aeCompensition.night_val = 0x5A;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0x14;
				_isp_attr.demosaic.night_val = 0x30;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x20;
				_isp_attr.wdr.night_val = 0x20;

				SHADINGAttr.Enable = HI_FALSE;
				SOC_CHECK(HI_MPI_ISP_SetShadingAttr (&SHADINGAttr));

				AEAttr.u8ExpStep = 0x30;
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
				
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
		}
			break;
			
		case SENSOR_MODEL_APTINA_AR0141:
			{			
				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 720;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x80;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x58;					;
				_isp_attr.aeCompensition.night_val = 0x40;				
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val =  0x40;//0x20;
				_isp_attr.denoise3d.night_val =  0x40;//0x40;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xA2;
				_isp_attr.demosaic.night_val = 0xA2;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x35;
				_isp_attr.wdr.night_val = 0x35;

				AEAttr.u8ExpStep = 0x30;
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
				
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
				//to reduce the white pixel
				HI_MPI_ISP_GetDefectPixelAttr(&DPAttr);
				DPAttr.u16DynamicBadPixelSlope = 768;
				DPAttr.u16DynamicBadPixelThresh = 32;
				//HI_MPI_ISP_SetDefectPixelAttr(&DPAttr);
		}
			break;
     	 case SENSOR_MODEL_SC1035:
	  	{
				AEAttr.u32SystemGainMax = 0x1C401;//113
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpStep = 0x30;
				SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
				HI_MPI_ISP_GetDefectPixelAttr(&DPAttr);
				DPAttr.u16DynamicBadPixelSlope = 1400;
				DPAttr.u16DynamicBadPixelThresh = 20;
				//HI_MPI_ISP_SetDefectPixelAttr(&DPAttr);
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));
				VPSS_CHN_SP_PARAM_S ChnSpParam;
				HI_MPI_VPSS_GetChnSpParam(0, 1, &ChnSpParam);
				ChnSpParam.u32LumaGain = 40;
				HI_MPI_VPSS_SetChnSpParam(0, 1, &ChnSpParam);

				/*VI_LDC_ATTR_S pstLDCAttr;
				SOC_CHECK( HI_MPI_VI_GetLDCAttr(HI3518A_VIN_CHN,&pstLDCAttr));
		    	pstLDCAttr.bEnable = true;
				pstLDCAttr.stAttr.enViewType = 0;
				pstLDCAttr.stAttr.s32CenterXOffset = 0;
				pstLDCAttr.stAttr.s32CenterYOffset = 13;
				pstLDCAttr.stAttr.s32Ratio = 160;
				SOC_CHECK(HI_MPI_VI_SetLDCAttr(HI3518A_VIN_CHN,&pstLDCAttr));*/

				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 960;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x80;

				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x40;
				_isp_attr.aeCompensition.night_val = 0x3c;
				
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x14;
				_isp_attr.denoise3d.night_val = 0x40;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x20;
				_isp_attr.wdr.night_val = 0x20;
	  }
	  break;
	  case SENSOR_MODEL_OV2710:
			{
				_isp_attr.sensor_resolution_width = 1920;
				_isp_attr.sensor_resolution_height = 1080;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x80;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 50;					;
				_isp_attr.aeCompensition.night_val = 50;				
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x50;
				_isp_attr.denoise3d.night_val =  0x40;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 170;//0xaa
				_isp_attr.demosaic.night_val = 170;//0xaa;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x20;
				_isp_attr.wdr.night_val = 0x20;

				AEAttr.u32ISPDGainMax = 0x400;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
				
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
				
				AEAttr.u8MaxHistOffset = 0;

				ISP_ADV_AWB_ATTR_S AdvAWBAttr;
				HI_MPI_ISP_GetAdvAWBAttr(&AdvAWBAttr);
				AdvAWBAttr.u16CurveRLimit = 1500;
				HI_MPI_ISP_SetAdvAWBAttr(&AdvAWBAttr);
				
				/*VI_LDC_ATTR_S pstLDCAttr;
				SOC_CHECK( HI_MPI_VI_GetLDCAttr(HI3518A_VIN_CHN,&pstLDCAttr));
		    	pstLDCAttr.bEnable = true;
				pstLDCAttr.stAttr.enViewType = 0;
				pstLDCAttr.stAttr.s32CenterXOffset = 0;
				pstLDCAttr.stAttr.s32CenterYOffset = 13;
				pstLDCAttr.stAttr.s32Ratio = 160;
			    SOC_CHECK(HI_MPI_VI_SetLDCAttr(HI3518A_VIN_CHN,&pstLDCAttr));*/
				}
		break;
		case SENSOR_MODEL_SOI_H42:
			{				
				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 720;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x80;
				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 55;
				_isp_attr.aeCompensition.night_val = 40;
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x14;
				_isp_attr.denoise3d.night_val = 0x40;
				_isp_attr.demosaic.strength = 3;
				_isp_attr.demosaic.daylight_val = 0xC0;
				_isp_attr.demosaic.night_val = 0x8C;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x20;
				_isp_attr.wdr.night_val = 0x20;

				//AEAttr.u32DGainMax = 0x400;
				//AEAttr.u32AGainMax = 0x1400;
				//AEAttr.u32ISPDGainMax = 0xB00;
				//AEAttr.u32SystemGainMax = 0x2c00;

				AEAttr.u8MaxHistOffset = 0;

				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpStep = 0x30;
				AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
				
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	

				VPSS_CHN_SP_PARAM_S ChnSpParam;
				HI_MPI_VPSS_GetChnSpParam(0, 1, &ChnSpParam);
				ChnSpParam.u32LumaGain = 40;
				HI_MPI_VPSS_SetChnSpParam(0, 1, &ChnSpParam);
			}
		break;
 		case SENSOR_MODEL_SC1045:
		{    
			GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
			memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));	

			
			ISP_SHADING_ATTR_S ShadingAttr;
		    SOC_CHECK(HI_MPI_ISP_GetShadingAttr(&ShadingAttr));
			ShadingAttr.Enable = HI_FALSE;
			SOC_CHECK(HI_MPI_ISP_SetShadingAttr(&ShadingAttr));
			}
		break;
	    case SENSOR_MODEL_BG0701:
		  	{				
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x3c;
				_isp_attr.denoise3d.night_val =  0x3c;
								
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
				AEAttr.u32SystemGainMax = 0x44C01;//113	
			}
		break;
		case SENSOR_MODEL_IMX225:
		  	{				
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x3c;
				_isp_attr.denoise3d.night_val =  0x3c;
								
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));	
				//AEAttr.u32SystemGainMax = 0x44C01;//113	
			}
		case SENSOR_MODEL_SC1135:
			{
				AEAttr.u32SystemGainMax = 0x1C401;//113
				AEAttr.s16ExpTolerance = 0xa;
				AEAttr.u8ExpStep = 0x30;
				SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
				HI_MPI_ISP_GetDefectPixelAttr(&DPAttr);
				DPAttr.u16DynamicBadPixelSlope = 1400;
				DPAttr.u16DynamicBadPixelThresh = 20;
				//HI_MPI_ISP_SetDefectPixelAttr(&DPAttr);
				GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Gamma, gs_Gamma[10], sizeof(gs_Gamma[10]));
				VPSS_CHN_SP_PARAM_S ChnSpParam;
				HI_MPI_VPSS_GetChnSpParam(0, 1, &ChnSpParam);
				ChnSpParam.u32LumaGain = 40;
				HI_MPI_VPSS_SetChnSpParam(0, 1, &ChnSpParam);

				
				_isp_attr.sensor_resolution_width = 1280;
				_isp_attr.sensor_resolution_height = 960;
				_isp_attr.aeHistSlope.strength = 3;
				_isp_attr.aeHistSlope.daylight_val = 0x80;
				_isp_attr.aeHistSlope.night_val = 0x80;

				_isp_attr.aeCompensition.strength = 3;
				_isp_attr.aeCompensition.daylight_val = 0x40;
				_isp_attr.aeCompensition.night_val = 0x3c;
				
				_isp_attr.denoise3d.strength = 3;
				_isp_attr.denoise3d.daylight_val = 0x14;
				_isp_attr.denoise3d.night_val = 0x40;
				_isp_attr.wdr.strength = 3;
				_isp_attr.wdr.daylight_val = 0x20;
				_isp_attr.wdr.night_val = 0x20;
			}
			break;

			
	}
	SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
	SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
	SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));

	DRCAttr.u32StrengthTarget = isp_get_isp_strength_value(&_isp_attr.wdr);
	if(DRCAttr.u32StrengthTarget > DRCAttr.u32SlopeMax){
		DRCAttr.u32StrengthTarget = DRCAttr.u32SlopeMax;
	}
	if(DRCAttr.u32StrengthTarget < DRCAttr.u32SlopeMin){
		DRCAttr.u32StrengthTarget = DRCAttr.u32SlopeMin;
	}
	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(&DRCAttr));
}

int HI_SDK_ISP_set_isp_sensor_value(void)////mode  0:daytime   1:night
{
	ISP_GAMMA_TABLE_S GammaAttr;
	ISP_AE_ATTR_EX_S AEAttr;
	VPSS_GRP_PARAM_S vpss_grp_param;
	ISP_DP_ATTR_S  DPAttr;
	ISP_AE_DELAY_S AEDelayAttr;
	ISP_SHARPEN_ATTR_S SHARPENAttr;
	ISP_DEMOSAIC_ATTR_S stDemosaicAttr;
	ISP_COLORTONE_S ColorTone;
	ISP_ADV_AWB_ATTR_S	AdvAWBAttr;	
	ISP_SATURATION_ATTR_S SaturationAttr;

	
	SOC_CHECK(HI_MPI_ISP_GetAEAttrEx(&AEAttr));
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&GammaAttr)); 
	SOC_CHECK(HI_MPI_VPSS_GetGrpParam(0, &vpss_grp_param));	
	SOC_CHECK(HI_MPI_ISP_GetDefectPixelAttr(&DPAttr));
	SOC_CHECK(HI_MPI_ISP_GetAEAttrEx(&AEDelayAttr));	
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(&SHARPENAttr));
	SOC_CHECK(HI_MPI_ISP_GetDemosaicAttr(&stDemosaicAttr));
	SOC_CHECK(HI_MPI_ISP_GetColorTone(&ColorTone));
	SOC_CHECK(HI_MPI_ISP_GetAdvAWBAttr (&AdvAWBAttr));
	SOC_CHECK(HI_MPI_ISP_GetSaturationAttr(&SaturationAttr));


		
	GammaAttr.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0130:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime	
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_OV_OV9712PLUS:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
					memcpy(GammaAttr.u16Gamma, gs_Gamma[5], sizeof(gs_Gamma[5]));
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_OV_OV9712:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
					memcpy(GammaAttr.u16Gamma, gs_Gamma[5], sizeof(gs_Gamma[5]));
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_SOI_H22:
			{			
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){
					//AEAttr.u32AGainMax = 0x1400;
					//AEAttr.u32ISPDGainMax = 0x800;
					//AEAttr.u32SystemGainMax = 0x2c00;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					soih22_i2c_write(0x49, 0xe);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
					//AEAttr.u32AGainMax = 0x1800;
					//AEAttr.u32ISPDGainMax = 0x800;
					//AEAttr.u32SystemGainMax = 0x3000;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					soih22_i2c_write(0x49, 0xa);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[5], sizeof(gs_Gamma[5]));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_SONY_IMX122:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					AEAttr.u16EVBias = 1024;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					AEAttr.u16EVBias = 1024;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_APTINA_AR0330:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_GC1004:
				{
					
						DPAttr.bEnableStatic = HI_FALSE; 
						DPAttr.bEnableDynamic= HI_TRUE; 
						DPAttr.bEnableDetect = HI_FALSE; 						
						SOC_CHECK(HI_MPI_ISP_SetDefectPixelAttr(&DPAttr));
							
						if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
						    AEAttr.u32SystemGainMax = 0x6000;

							SHARPENAttr.u8SharpenAltD[0] = 0xb0;
							SHARPENAttr.u8SharpenAltD[1] = 0xa0;
							SHARPENAttr.u8SharpenAltD[2] = 0x90;
							SHARPENAttr.u8SharpenAltD[3] = 0x80;
							SHARPENAttr.u8SharpenAltD[4] = 0x70;
							SHARPENAttr.u8SharpenAltD[5] = 0x60;
							SHARPENAttr.u8SharpenAltD[6] = 0x48;
							SHARPENAttr.u8SharpenAltD[7] = 0x30;

							SHARPENAttr.u8SharpenAltUd[0] = 0x80;
							SHARPENAttr.u8SharpenAltUd[1] = 0x70;
							SHARPENAttr.u8SharpenAltUd[2] = 0x60;
							SHARPENAttr.u8SharpenAltUd[3] = 0x50;
							SHARPENAttr.u8SharpenAltUd[4] = 0x48;
							SHARPENAttr.u8SharpenAltUd[5] = 0x48;
							SHARPENAttr.u8SharpenAltUd[6] = 0x48;
							SHARPENAttr.u8SharpenAltUd[7] = 0x48;
					
							AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
						    AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
							memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
							vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
							vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
							vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
							
							SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
							SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
							SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
							SOC_CHECK(HI_MPI_ISP_SetSharpenAttr(&SHARPENAttr));
							stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
							SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
						    
		
						}else{//night
						
						    AEAttr.u32SystemGainMax = 0x7000;			//28±¶	
	
							SHARPENAttr.u8SharpenAltD[0] = 0xa0;
							SHARPENAttr.u8SharpenAltD[1] = 0x90;
							SHARPENAttr.u8SharpenAltD[2] = 0x80;
							SHARPENAttr.u8SharpenAltD[3] = 0x70;
							SHARPENAttr.u8SharpenAltD[4] = 0x15;
							SHARPENAttr.u8SharpenAltD[5] = 0x13;
							SHARPENAttr.u8SharpenAltD[6] = 0x10;
							SHARPENAttr.u8SharpenAltD[7] = 0x10;

							SHARPENAttr.u8SharpenAltUd[0] = 0x70;
							SHARPENAttr.u8SharpenAltUd[1] = 0x73;
							SHARPENAttr.u8SharpenAltUd[2] = 0x76;
							SHARPENAttr.u8SharpenAltUd[3] = 0x79;
							SHARPENAttr.u8SharpenAltUd[4] = 0x7b;
							SHARPENAttr.u8SharpenAltUd[5] = 0x80;
							SHARPENAttr.u8SharpenAltUd[6] = 0x83;
							SHARPENAttr.u8SharpenAltUd[7] = 0x86;
		
							AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
							AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);	
					
							vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
							vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
							vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;

							memcpy(GammaAttr.u16Gamma, gs_Gamma[6], sizeof(gs_Gamma[6]));
							
							SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
							SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
							SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));	
							SOC_CHECK(HI_MPI_ISP_SetSharpenAttr(&SHARPENAttr));
							stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
							SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
						}
			}
				break;
		case SENSOR_MODEL_APTINA_AR0141:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
				
   					//AEAttr.u32SystemGainMax = 40000;		
   					//AEAttr.u32DGainMax = 0x800;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
				
   					//AEAttr.u32SystemGainMax = 0xE00A;		
					//AEAttr.u32AGainMax = 0x1800;
					AEAttr.u32DGainMax = 0x500;
					//AEAttr.u32ISPDGainMax = 0x400;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_SC1035:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
				
   					//AEAttr.u32SystemGainMax = 40000;							
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[0], sizeof(gs_Gamma[0]));
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
				}else{//night
				
   					//AEAttr.u32SystemGainMax = 0x1C415;		
					//AEAttr.u32AGainMax = 0x1800;
				//	AEAttr.u32ISPDGainMax = 0x400;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
				}
			}
		break;
		case SENSOR_MODEL_SOI_H42:
			{			
				
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){

					soih42_i2c_write(0x49, 0x0b);

					ColorTone.u16BlueCastGain = 256;
					ColorTone.u16GreenCastGain = 256;
					ColorTone.u16RedCastGain = 256;
					SOC_CHECK(HI_MPI_ISP_SetColorTone(&ColorTone));

					AEAttr.u32ISPDGainMax = 0x400;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}else{//night
				
					soih42_i2c_write(0x49, 0x0b);
				    AEAttr.u32ISPDGainMax = 0x400;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/4;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[5], sizeof(gs_Gamma[5]));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					stDemosaicAttr.u8UuSlope = isp_get_isp_value(&_isp_attr.demosaic);
					SOC_CHECK(HI_MPI_ISP_SetDemosaicAttr(&stDemosaicAttr));
				}
			}
		break;
		case SENSOR_MODEL_SC1045:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime


					AdvAWBAttr.u16CurveLLimit =  224;
					AdvAWBAttr.u16CurveRLimit =  304 ; 
					AdvAWBAttr.bGainNormEn = HI_TRUE;
					AdvAWBAttr.stCTLimit.bEnable   =  HI_TRUE;
					AdvAWBAttr.stInOrOut.bEnable = HI_TRUE ;
					AdvAWBAttr.stInOrOut.bGreenEnhanceEn =HI_TRUE ;
					SOC_CHECK(HI_MPI_ISP_SetAdvAWBAttr (&AdvAWBAttr));

				}else{//night
				
   
				}
			}
		break;
		case SENSOR_MODEL_BG0701:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT)//daytime
				{
				    vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));

				}else//night
				{
				    vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));

				}
			}
		break;
		case SENSOR_MODEL_IMX225:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT)//daytime
				{
				    vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));

				}else//night
				{
				    vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
				}
			}
			break;
		case SENSOR_MODEL_SC1135:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
					SaturationAttr.u8SatTarget=145;
					//ColorTone.u16GreenCastGain=260;
					//AEAttr.u32SystemGainMax = 40000;							
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[11], sizeof(gs_Gamma[11]));
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					SOC_CHECK(HI_MPI_ISP_SetSaturationAttr(&SaturationAttr));
					SOC_CHECK(HI_MPI_ISP_SetColorTone(&ColorTone));
				}else{//night
					//ColorTone.u16GreenCastGain=256;
					//AEAttr.u32SystemGainMax = 0x1C415;		
					//AEAttr.u32AGainMax = 0x1800;
				//	AEAttr.u32ISPDGainMax = 0x400;
					AEAttr.u16HistRatioSlope = isp_get_isp_value(&_isp_attr.aeHistSlope);
					AEAttr.u8ExpCompensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
					vpss_grp_param.u32SfStrength = isp_get_isp_strength_value(&_isp_attr.denoise3d);
					vpss_grp_param.u32TfStrength = vpss_grp_param.u32SfStrength/2;
					vpss_grp_param.u32ChromaRange = vpss_grp_param.u32SfStrength;
					memcpy(GammaAttr.u16Gamma, gs_Gamma[4], sizeof(gs_Gamma[4]));
					SOC_CHECK(HI_MPI_ISP_SetAEAttrEx(&AEAttr));
					SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
					SOC_CHECK(HI_MPI_ISP_SetGammaTable(&GammaAttr));
					SOC_CHECK(HI_MPI_ISP_SetColorTone(&ColorTone));
				}
			}
			break;	
	}
	return 0;
}

int HI_SDK_ISP_ini_load(const char *filepath)
{
	if(NULL == _isp_attr.ispCfgAttr){
		_isp_attr.ispCfgAttr = (LpIspCfgAttr)calloc(sizeof(StIspCfgAttr), 1);
	}
	if(-1 == HI_ISP_cfg_load(filepath, _isp_attr.ispCfgAttr)){
		if(_isp_attr.ispCfgAttr){
			free(_isp_attr.ispCfgAttr);
		}
		_isp_attr.ispCfgAttr = NULL;
		return -1;
	}else{
		_isp_attr.ispCfgAttr->userAttr.DrcStrength = 3;//setting as default
		_isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate = _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO ? true : false;
		return HI_ISP_cfg_set_all(_isp_attr.gpio_status_old, 1, _isp_attr.ispCfgAttr);
	}
}

int HI_SDK_ISP_get_wdr_mode(uint8_t *bEnable)
{
	ISP_DRC_ATTR_S DRCAttr;
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(&DRCAttr));
	
	*bEnable = DRCAttr.bDRCEnable;

	return 0;
}

int hi_isp_api_set_ircut_switch_to_day_array(float * night_to_day_array)
{
	return 0;
}


#define SET_VI_DEV_ATTR_AR0130(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xfff00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 960;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}

#define SET_VI_DEV_ATTR_OV9712(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFC00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 408;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}

#define SET_VI_DEV_ATTR_SOIH22(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFC00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 408;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}

#define SET_VI_DEV_ATTR_IMX122(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}

#define SET_VI_DEV_ATTR_AR0330(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xfff00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}

#define SET_VI_DEV_ATTR_GC1004(info)\
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xffc00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}
#define SET_VI_DEV_ATTR_AR0141(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xfff00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}
#define SET_VI_DEV_ATTR_SC1035(info)\
{\
	info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
	info.enWorkMode = VI_WORK_MODE_1Multiplex;\
	info.au32CompMask[0] = 0xfff00000;\
	info.au32CompMask[1] = 0x00000000;\
	info.enScanMode = VI_SCAN_PROGRESSIVE;\
	info.s32AdChnId[0] = -1;\
	info.s32AdChnId[1] = -1;\
	info.s32AdChnId[2] = -1;\
	info.s32AdChnId[3] = -1;\
	info.enDataSeq = VI_INPUT_DATA_YUYV;\
	info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
	info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
	info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
	info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
	info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
	info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
	info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
	info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
	info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVact = 960;\
	info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
	info.enDataPath = VI_PATH_ISP;\
	info.enInputDataType = VI_DATA_TYPE_RGB;\
	info.bDataRev = HI_FALSE;\
}

#define SET_VI_DEV_ATTR_OV2710(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
		    info.au32CompMask[0] = 0xffc00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1920;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 1080;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}


#define SET_VI_DEV_ATTR_SOIH42(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFC00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 408;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}
#if defined(P1)
#define SET_VI_DEV_ATTR_SC1045(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0x3FF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 408;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}
#else
#define SET_VI_DEV_ATTR_SC1045(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFC00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_NORM_PULSE;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 408;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 6;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}
#endif
#define SET_VI_DEV_ATTR_BG0701(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xFFF00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}

#define SET_VI_DEV_ATTR_IMX225(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xfff00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 960;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\			
}

#define SET_VI_DEV_ATTR_SC1135(info)\
{\
	info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
	info.enWorkMode = VI_WORK_MODE_1Multiplex;\
	info.au32CompMask[0] = 0xfff00000;\
	info.au32CompMask[1] = 0x00000000;\
	info.enScanMode = VI_SCAN_PROGRESSIVE;\
	info.s32AdChnId[0] = -1;\
	info.s32AdChnId[1] = -1;\
	info.s32AdChnId[2] = -1;\
	info.s32AdChnId[3] = -1;\
	info.enDataSeq = VI_INPUT_DATA_YUYV;\
	info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
	info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
	info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
	info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
	info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
	info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
	info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
	info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
	info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVact = 960;\
	info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
	info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
	info.enDataPath = VI_PATH_ISP;\
	info.enInputDataType = VI_DATA_TYPE_RGB;\
	info.bDataRev = HI_FALSE;\
}

static stHiIspAttr _isp_attr = {
	.api = {
		.SENSOR_MODEL_GET=hi_isp_api_get_sensor_model,
		.MIRROR_FLIP_SET=hi_isp_api_mirror_flip,
		.HUE_SET=hi_isp_api_set_hue,
		.CONTRAST_SET=hi_isp_api_set_contrast,
		.BRIGHTNESS_SET=hi_isp_api_set_brightness,
		.SATURATION_SET=hi_isp_api_set_saturation,
		.LIGHT_MODE_SET=hi_isp_api_light_mode,
		.TEST_MODE_SET=hi_isp_api_test_mode,
		.COLOR_MODE_SET=hi_isp_api_color_mode,
		.REG_READ=hi_isp_api_reg_read,
	 	.REG_WRITE=hi_isp_api_reg_write,
		.SPEC_RED_WRITE=hi_isp_api_spec_reg_write,
		.SPEC_REG_READ=hi_isp_api_spec_reg_read,
		.GET_COLOR_MAX_VALUE=hi_isp_api_get_color_max_value,
		.SHUTTER_SET=hi_isp_api_set_shutter,
		.IRCUT_AUTO_SWITCH=hi_isp_api_ircut_auto_switch,
		.VI_FLICKER=hi_isp_api_vi_flicker,
		.SHARPEN_SET=hi_isp_api_set_sharpen,
		.SHARPEN_GET=hi_isp_api_get_sharpen,
		.SCENE_MODE_SET=hi_isp_api_set_scene_mode,
		.WB_MODE_SET=hi_isp_api_set_WB_mode,
		.IRCUT_CONTROL_MODE_SET=hi_isp_api_set_ircut_control_mode,
		.IRCUT_MODE_SET=hi_isp_api_set_ircut_mode,
		.WDR_MODE_ENABLE=hi_isp_api_set_WDR_enable,
		.WDR_STRENGTH_SET=hi_isp_api_set_WDR_strength,
		.EXPOSURE_MODE_SET=hi_isp_api_set_exposure_mode,
		.AE_COMPENSATION_SET=hi_isp_api_set_AEcompensation,
		.DENOISE_ENABLE=hi_isp_api_set_denoise_enable,
		.DENOISE_STRENGTH_SET=hi_isp_api_set_denoise_strength,
		.DENOISE_STRENGTH_GET=hi_isp_api_get_denoise_strength,
		.ANTI_FOG_ENABLE=hi_isp_api_set_anti_fog_enable,
		.LOWLIGHT_ENABLE=hi_isp_api_set_lowlight_enable,
		.GAMMA_TABLE_SET=hi_isp_api_set_gamma_table,
		.DEFECT_PIXEL_ENABLE=hi_isp_api_set_defect_pixel_enable,
		.SRC_FRAMERATE_SET=hi_isp_api_set_src_framerate,
		.SENSOR_RESOLUTION_GET=hi_isp_api_get_sensor_resolution,
		.SENSOR_RESOLUTION_SET=hi_isp_api_set_sensor_resolution,
		.GAIN_GET=hi_isp_api_get_gain,
		.AF_CALLBACK_SET = hi_isp_api_set_af_attr,
		.INI_LOAD=hi_isp_api_ini_load,
		.WDR_MODE_GET= hi_isp_api_get_wdr_mode,	
		.IRCUT_SWITCH_TO_DAY_ARRAY_SET = hi_isp_api_set_ircut_switch_array,
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
	.lowlight_mode = 0,
	.isp_auto_drc_enabled = HI_TRUE,
	.AfAttr.param = NULL,
	.AfAttr.af_callback = NULL,
	.isp_framerate_status = HI_FALSE,
	.filter_frequency = 50,
	.ispCfgAttr = NULL,
};

static HI_S32 pfunction_af_init(HI_S32 s32Handle, const ISP_AF_PARAM_S *pstAfParam)
{
	if (HI_NULL == pstAfParam)
    {
        printf("null pointer when af init default value!\n");
        return -1;
    }

	return 0;
}

static HI_S32 pfunction_af_run(HI_S32 s32Handle, const ISP_AF_INFO_S *pstAfInfo, ISP_AF_RESULT_S *pstAfResult, HI_S32 s32Rsv)
{
	/*printf("u32FrameCnt = %d\r\n"
		"u16FocusMetrics = %d\r\n"
		"u16ThresholdRead = %d\r\n"
		"u16ThresholdWrite = %d\r\n"
		"u16FocusIntensity = %d\r\n"
		"u8MetricsShift = %d\r\n"
		"bChange = %d\r\n"
		"u16ThresholdWrite = %d\r\n"
		"u8MetricsShift = %d\r\n"
		"u8NpOffset = %d\r\n",
		pstAfInfo->u32FrameCnt,
		pstAfInfo->pstStatistics->u16FocusMetrics,
		pstAfInfo->pstStatistics->u16ThresholdRead, 
		pstAfInfo->pstStatistics->u16ThresholdWrite,
		pstAfInfo->pstStatistics->u16FocusIntensity,
		pstAfInfo->pstStatistics->u8MetricsShift,
		pstAfResult->stStatAttr.bChange,
		pstAfResult->stStatAttr.u16ThresholdWrite,
		pstAfResult->stStatAttr.u8MetricsShift,
		pstAfResult->stStatAttr.u8NpOffset);*/
	int ret = 0;
	if(NULL != _isp_attr.AfAttr.af_callback){
		//printf("sock=%d\r\n", *_isp_attr.AfAttr.param);
		ret = _isp_attr.AfAttr.af_callback((int)pstAfInfo->pstStatistics->u16FocusMetrics, (int)pstAfInfo->pstStatistics->u16FocusMetrics, (int)pstAfInfo->pstStatistics->u16FocusMetrics, _isp_attr.AfAttr.param);
		if(ret< 0){
			if(_isp_attr.AfAttr.param){
				free(_isp_attr.AfAttr.param);
				_isp_attr.AfAttr.param = NULL;
				_isp_attr.AfAttr.af_callback = NULL;
			}
		}
		
	}

		//printf("u16FocusMetrics =%d-%d\r\n", pstAfInfo->pstStatistics->u16FocusMetrics, pstAfResult->stStatAttr.bChange);
	return 0;
}

static HI_S32 pfunctin_af_ctrl(HI_S32 s32Handle, HI_U32 u32Cmd, HI_VOID *pValue)
{
	return 0;
}

static HI_S32 pfunction_af_exit(HI_S32 s32Handle)
{
	return 0;
}


static int isp_af_init_function(	ISP_AF_REGISTER_S *pAfRegister)
{
	memset(&pAfRegister->stAfExpFunc, 0, sizeof(ISP_AF_EXP_FUNC_S));
	pAfRegister->stAfExpFunc.pfn_af_init = pfunction_af_init;
	pAfRegister->stAfExpFunc.pfn_af_ctrl= pfunctin_af_ctrl;
	pAfRegister->stAfExpFunc.pfn_af_run = pfunction_af_run;
	pAfRegister->stAfExpFunc.pfn_af_exit = pfunction_af_exit;
}

int HI_SDK_ISP_init(lpSensorApi*api, lpBSPApi *bsp_api)
{
	int ret = 0;
	pthread_t isp_tid = 0;
	VI_DEV_ATTR_S vi_dev_attr_720p_30fps;
	VI_CHN_ATTR_S vin_chn_attr;
	VPSS_GRP_ATTR_S vpss_grp_attr;
	ISP_IMAGE_ATTR_S isp_image_attr;
    ISP_INPUT_TIMING_S isp_input_timing;
	ISP_GAMMA_TABLE_S pstGammaAttr;
	ISP_AF_REGISTER_S stAfRegister;
	memcpy(&_isp_attr.bsp_api, bsp_api, sizeof(stBSPApi));
	//_isp_attr.bsp_api.BSP_GET_PHOTOSWITCH = bsp_api->BSP_GET_PHOTOSWITCH;
	//_isp_attr.bsp_api.BSP_IRCUT_SWITCH = bsp_api->BSP_IRCUT_SWITCH;
	//_isp_attr.bsp_api.BSP_SENSOR_RESET = bsp_api->BSP_SENSOR_RESET;
	//_isp_attr.bsp_api.BSP_SET_IR_LED = bsp_api->BSP_SET_IR_LED;
	HI_SDK_ISP_sensor_check();
	isp_af_init_function(&stAfRegister);

	memset(&vpss_grp_attr, 0, sizeof(vpss_grp_attr));
	//init sensor
	printf("sensor type:%d\r\n", _isp_attr.sensor_type);
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0130:
			APTINA_AR0130_init((SENSOR_APTINA_AR0130_DO_I2CRD)ar0130_i2c_read, 
				(SENSOR_APTINA_AR0130_DO_I2CWR)ar0130_i2c_write,
				&stAfRegister); 
			SET_VI_DEV_ATTR_AR0130(vi_dev_attr_720p_30fps);
			break;
		case SENSOR_MODEL_OV_OV9712PLUS:
			OV9712PLUS_init((SENSOR_OV9712_DO_I2CRD)ov9712_i2c_read, 
				(SENSOR_OV9712_DO_I2CWR)ov9712_i2c_write);
			SET_VI_DEV_ATTR_OV9712(vi_dev_attr_720p_30fps);			
			break;
		case SENSOR_MODEL_OV_OV9712:
			OV9712_init((SENSOR_OV9712_DO_I2CRD)ov9712_i2c_read, 
				(SENSOR_OV9712_DO_I2CWR)ov9712_i2c_write);
			SET_VI_DEV_ATTR_OV9712(vi_dev_attr_720p_30fps);			
			break;
		case SENSOR_MODEL_SOI_H22:
			SOIH22_init((SENSOR_SOIH22_DO_I2CRD)soih22_i2c_read, 
				(SENSOR_SOIH22_DO_I2CWR)soih22_i2c_write);
			SET_VI_DEV_ATTR_SOIH22(vi_dev_attr_720p_30fps);			
			break;	
		case SENSOR_MODEL_SONY_IMX122:
			SONY_IMX122_init((SENSOR_SONY_IMX122_DO_I2CRD)imx122_i2c_read,
				(SENSOR_SONY_IMX122_DO_I2CWR)imx122_i2c_write);
			SET_VI_DEV_ATTR_IMX122(vi_dev_attr_720p_30fps);
			break;
		case SENSOR_MODEL_APTINA_AR0330:
			APTINA_AR0330_init((SENSOR_APTINA_AR0330_DO_I2CRD)ar0330_i2c_read, 
				(SENSOR_APTINA_AR0330_DO_I2CWR)ar0330_i2c_write); 
			SET_VI_DEV_ATTR_AR0330(vi_dev_attr_720p_30fps);
			break;
		case SENSOR_MODEL_GC1004:
			GC1004_init((SENSOR_GC1004_DO_I2CRD) gc1004_i2c_read,
				(SENSOR_GC1004_DO_I2CWR) gc1004_i2c_write);			
			SET_VI_DEV_ATTR_GC1004(vi_dev_attr_720p_30fps);
			break;

		case SENSOR_MODEL_OV2710:
			OV2710_init((SENSOR_OV2710_DO_I2CRD) ov2710_i2c_read, 
				(SENSOR_OV2710_DO_I2CWR) ov2710_i2c_write);
			SET_VI_DEV_ATTR_OV2710(vi_dev_attr_720p_30fps);
			
			break;
		case SENSOR_MODEL_APTINA_AR0141:
			APTINA_AR0141_init((SENSOR_APTINA_AR0141_DO_I2CRD) ar0141_i2c_read,
				(SENSOR_APTINA_AR0141_DO_I2CWR) ar0141_i2c_write);
			SET_VI_DEV_ATTR_AR0141(vi_dev_attr_720p_30fps);
			break;
		case SENSOR_MODEL_SC1035:
			SC1035_init((SENSOR_SC1035_DO_I2CRD) sc1035_i2c_read,
				(SENSOR_SC1035_DO_I2CWR) sc1035_i2c_write);
			SET_VI_DEV_ATTR_AR0141(vi_dev_attr_720p_30fps);
			break;
		case SENSOR_MODEL_SOI_H42:
			SOIH42_init((SENSOR_SOIH42_DO_I2CRD)soih42_i2c_read, 
				(SENSOR_SOIH42_DO_I2CWR)soih42_i2c_write);
			SET_VI_DEV_ATTR_SOIH42(vi_dev_attr_720p_30fps);			
			break;	
		case SENSOR_MODEL_SC1045:
			SC1045_init((SENSOR_SC1045_DO_I2CRD)SC1045_i2c_read, 
				(SENSOR_SC1045_DO_I2CWR)SC1045_i2c_write);
			SET_VI_DEV_ATTR_SC1045(vi_dev_attr_720p_30fps);			
			break;
		case SENSOR_MODEL_BG0701:
				BG0701_init((SENSOR_BG0701_DO_I2CRD) bg0701_i2c_read,
					(SENSOR_BG0701_DO_I2CWR) bg0701_i2c_write);
				SET_VI_DEV_ATTR_BG0701(vi_dev_attr_720p_30fps);
				break;
		case SENSOR_MODEL_IMX225:
			SONY_IMX225_init((SENSOR_IMX225_DO_I2CRD)imx225_i2c_read,
				(SENSOR_IMX225_DO_I2CWR)imx225_i2c_write);
			SET_VI_DEV_ATTR_IMX225(vi_dev_attr_720p_30fps);
			break;
		case SENSOR_MODEL_SC1135:
			SC1135_init((SENSOR_SC1135_DO_I2CRD) sc1135_i2c_read,
				(SENSOR_SC1135_DO_I2CWR) sc1135_i2c_write);
			SET_VI_DEV_ATTR_SC1135(vi_dev_attr_720p_30fps);
			break;

	}
	//AR0130_sensor_mode_set(1);

	{
		SOC_CHECK(HI_MPI_VI_SetDevAttr(HI3518A_VIN_DEV, &vi_dev_attr_720p_30fps));
		SOC_CHECK(HI_MPI_VI_EnableDev(HI3518A_VIN_DEV));
	}
	{
	    // ISP isp init
	    
	SOC_CHECK(HI_MPI_ISP_Init());
	SOC_CHECK(HI_MPI_ISP_GetGammaTable(&pstGammaAttr)); 
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0130:
			isp_image_attr.enBayer = BAYER_GRBG;
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 960;
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 960;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 960;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 960;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SENSOR_MODEL_OV_OV9712PLUS:
		case SENSOR_MODEL_OV_OV9712:
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
			isp_image_attr.enBayer = BAYER_BGGR;		
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 720;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SENSOR_MODEL_SOI_H22:
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 720;
			isp_image_attr.enBayer = BAYER_BGGR;	
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SENSOR_MODEL_SONY_IMX122:
			vin_chn_attr.stCapRect.u32Width = 1920;
			vin_chn_attr.stCapRect.u32Height = 1080;
			vin_chn_attr.stDestSize.u32Width = 1920;
			vin_chn_attr.stDestSize.u32Height = 1080;
			isp_image_attr.enBayer = BAYER_RGGB;
       		isp_image_attr.u16Width = 1920;
        	isp_image_attr.u16Height = 1080;
			vpss_grp_attr.u32MaxW = 1920;
			vpss_grp_attr.u32MaxH = 1080;			
			isp_input_timing.enWndMode = ISP_WIND_ALL;
			isp_input_timing.u16HorWndStart = 200;
			isp_input_timing.u16HorWndLength = 1920;
			isp_input_timing.u16VerWndStart = 24;
			isp_input_timing.u16VerWndLength = 1080;
			break;
		case SENSOR_MODEL_APTINA_AR0330:
			vin_chn_attr.stCapRect.u32Width = 1920;
			vin_chn_attr.stCapRect.u32Height = 1080;
			vin_chn_attr.stDestSize.u32Width = 1920;
			vin_chn_attr.stDestSize.u32Height = 1080;
			isp_image_attr.enBayer = BAYER_GRBG;
       		isp_image_attr.u16Width = 1920;
        	isp_image_attr.u16Height = 1080;
			vpss_grp_attr.u32MaxW = 1920;
			vpss_grp_attr.u32MaxH = 1080;			
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			/*isp_input_timing.u16HorWndStart = 0;
			isp_input_timing.u16HorWndLength = 1920;
			isp_input_timing.u16VerWndStart = 24;
			isp_input_timing.u16VerWndLength = 1080;*/
			break;
		case SENSOR_MODEL_GC1004:
			isp_image_attr.enBayer = BAYER_RGGB;
       		isp_image_attr.u16Width = 1280;
			isp_image_attr.u16Height = 720;
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			/*isp_input_timing.enWndMode = ISP_WIND_ALL;	
			isp_input_timing.u16HorWndStart = 80;
			isp_input_timing.u16HorWndLength = 1120;
			isp_input_timing.u16VerWndStart = 48;
			isp_input_timing.u16VerWndLength = 630;*/
			break;
		case SENSOR_MODEL_APTINA_AR0141:
			isp_image_attr.enBayer = BAYER_GRBG;
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 720;
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SENSOR_MODEL_SC1035:
			isp_image_attr.enBayer = BAYER_BGGR;
			isp_image_attr.u16Width = 1184;
			isp_image_attr.u16Height = 896;
			vin_chn_attr.stCapRect.u32Width = 1184;
			vin_chn_attr.stCapRect.u32Height = 896;
			vin_chn_attr.stDestSize.u32Width = 1184;
			vin_chn_attr.stDestSize.u32Height = 896;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 960;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			isp_input_timing.enWndMode = ISP_WIND_ALL;	
			isp_input_timing.u16HorWndStart = 48;
			isp_input_timing.u16HorWndLength = 1184;
			isp_input_timing.u16VerWndStart = 32;
			isp_input_timing.u16VerWndLength = 896;
			break;
		case SENSOR_MODEL_OV2710:
			vin_chn_attr.stCapRect.u32Width = 1824;
			vin_chn_attr.stCapRect.u32Height = 1016;
			vin_chn_attr.stDestSize.u32Width = 1824;
			vin_chn_attr.stDestSize.u32Height = 1016;
			isp_image_attr.enBayer = BAYER_BGGR;		
       		isp_image_attr.u16Width = 1824;
        	isp_image_attr.u16Height = 1016;
			vpss_grp_attr.u32MaxW = 1920;
			vpss_grp_attr.u32MaxH = 1080;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			isp_input_timing.enWndMode = ISP_WIND_ALL;	
			isp_input_timing.u16HorWndStart = 48;
			isp_input_timing.u16HorWndLength = 1824;
			isp_input_timing.u16VerWndStart = 32;
			isp_input_timing.u16VerWndLength = 1016;
			/*vin_chn_attr.stCapRect.u32Width = 1920;
			vin_chn_attr.stCapRect.u32Height = 1080;
			vin_chn_attr.stDestSize.u32Width = 1920;
			vin_chn_attr.stDestSize.u32Height = 1080;
			isp_image_attr.enBayer = BAYER_BGGR;		
       		isp_image_attr.u16Width = 1920;
        	isp_image_attr.u16Height = 1080;
			vpss_grp_attr.u32MaxW = 1920;
			vpss_grp_attr.u32MaxH = 1080;
			isp_input_timing.enWndMode = ISP_WIND_NONE;*/
			break;
		case SENSOR_MODEL_SOI_H42:
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
       		isp_image_attr.u16Width = 1280;
        	isp_image_attr.u16Height = 720;
			isp_image_attr.enBayer = BAYER_BGGR;	
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
			
		case SENSOR_MODEL_SC1045:
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
			isp_image_attr.u16Width = 1280;
			isp_image_attr.u16Height = 720;
			isp_image_attr.enBayer = BAYER_BGGR;	
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			break;
		case SENSOR_MODEL_BG0701:

			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 720;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 720;
			vin_chn_attr.enCapSel = VI_CAPSEL_BOTH;

			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 720;
			isp_image_attr.enBayer = BAYER_GBRG;
			isp_image_attr.u16Width = 1280;
			isp_image_attr.u16Height = 720;
			isp_input_timing.enWndMode = ISP_WIND_ALL;
			isp_input_timing.u16HorWndStart = 0;
			isp_input_timing.u16HorWndLength = 1280;
			isp_input_timing.u16VerWndStart = 0;
			isp_input_timing.u16VerWndLength = 720;
			break;
		case SENSOR_MODEL_IMX225:
			isp_image_attr.enBayer = BAYER_GBRG;
			isp_image_attr.u16Width = 1280;
			isp_image_attr.u16Height = 960;
			vin_chn_attr.stCapRect.u32Width = 1280;
			vin_chn_attr.stCapRect.u32Height = 960;
			vin_chn_attr.stDestSize.u32Width = 1280;
			vin_chn_attr.stDestSize.u32Height = 960;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 960;
			isp_input_timing.enWndMode = ISP_WIND_ALL;
			isp_input_timing.u16HorWndStart = 8;
			isp_input_timing.u16HorWndLength = 1280;
			isp_input_timing.u16VerWndStart = 24;
			isp_input_timing.u16VerWndLength = 960;

			break;
		case SENSOR_MODEL_SC1135:
			isp_image_attr.enBayer = BAYER_BGGR;
			isp_image_attr.u16Width = 1184;
			isp_image_attr.u16Height = 896;
			vin_chn_attr.stCapRect.u32Width = 1184;
			vin_chn_attr.stCapRect.u32Height = 896;
			vin_chn_attr.stDestSize.u32Width = 1184;
			vin_chn_attr.stDestSize.u32Height = 896;
			vpss_grp_attr.u32MaxW = 1280;
			vpss_grp_attr.u32MaxH = 960;
			isp_input_timing.enWndMode = ISP_WIND_NONE;
			isp_input_timing.enWndMode = ISP_WIND_ALL;	
			isp_input_timing.u16HorWndStart = 48;
			isp_input_timing.u16HorWndLength = 1184;
			isp_input_timing.u16VerWndStart = 32;
			isp_input_timing.u16VerWndLength = 896;
			break;

	}
	    // ISP set image attributes		
	    isp_image_attr.u16FrameRate = 30;
		_isp_attr.src_framerate = isp_image_attr.u16FrameRate;
	    SOC_CHECK(HI_MPI_ISP_SetImageAttr(&isp_image_attr));
	    // ISP set timing
	    SOC_CHECK(HI_MPI_ISP_SetInputTiming(&isp_input_timing));
		
		ret = pthread_create(&isp_tid, NULL, (void*(*)(void*))HI_MPI_ISP_Run, NULL);
		
		//assert(0 == ret);
	}
	{
		
		/* step  5: config & start vicap dev */
		vin_chn_attr.stCapRect.s32X = 0;
		vin_chn_attr.stCapRect.s32Y = 0;
		vin_chn_attr.enCapSel = VI_CAPSEL_BOTH;
		vin_chn_attr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		vin_chn_attr.bMirror = HI_FALSE;
		vin_chn_attr.bFlip = HI_FALSE;
		vin_chn_attr.bChromaResample = HI_FALSE;
		vin_chn_attr.s32SrcFrameRate = 30;
		vin_chn_attr.s32FrameRate = 30;
	
		SOC_CHECK(HI_MPI_VI_SetChnAttr(HI3518A_VIN_CHN, &vin_chn_attr));

		HI_SDK_ISP_set_mirror(HI3518A_VIN_CHN, false);
		HI_SDK_ISP_set_flip(HI3518A_VIN_CHN, false);
		
		SOC_CHECK(HI_MPI_VI_EnableChn(HI3518A_VIN_CHN));

	}
	{
		int vpss_grp = 0; // only use one vpss
		VPSS_GRP_PARAM_S vpss_grp_param;

		memset(&vpss_grp_param, 0, sizeof(vpss_grp_param));

		
		vpss_grp_attr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		vpss_grp_attr.bDrEn = HI_FALSE;
		vpss_grp_attr.bDbEn = HI_FALSE;
		vpss_grp_attr.bIeEn = HI_TRUE;
		vpss_grp_attr.bNrEn = HI_TRUE;	
		vpss_grp_attr.bHistEn = HI_TRUE;
		vpss_grp_attr.enDieMode = VPSS_DIE_MODE_AUTO;

		SOC_CHECK(HI_MPI_VPSS_CreateGrp(vpss_grp, &vpss_grp_attr));
		SOC_CHECK(HI_MPI_VPSS_GetGrpParam(vpss_grp, &vpss_grp_param));
		vpss_grp_param.u32SfStrength = 0x20;
		vpss_grp_param.u32TfStrength = 0x8;
		vpss_grp_param.u32ChromaRange = 0x8;
		vpss_grp_param.u32MotionThresh = 0;
		SOC_CHECK(HI_MPI_VPSS_SetGrpParam(vpss_grp, &vpss_grp_param));
		SOC_CHECK(HI_MPI_VPSS_StartGrp(vpss_grp));

		MPP_CHN_S viu_mpp_chn;
		MPP_CHN_S vpss_mpp_chn;

		viu_mpp_chn.enModId = HI_ID_VIU;
		viu_mpp_chn.s32DevId = HI3518A_VIN_DEV;
		viu_mpp_chn.s32ChnId = HI3518A_VIN_CHN;
		vpss_mpp_chn.enModId = HI_ID_VPSS;
		vpss_mpp_chn.s32DevId = 0;
		vpss_mpp_chn.s32ChnId = 0;
		SOC_CHECK(HI_MPI_SYS_Bind(&viu_mpp_chn, &vpss_mpp_chn));
	}

	_hi_sdk_isp_init_isp_default_value();
	isp_ircut_gpio_init();
	isp_ircut_switch(0);//default for daylight

	*api = &_isp_attr.api;
	//AR0130_sensor_mode_set(1);
	//memcpy(api, &_isp_attr.api, sizeof(stSensorApi));
	return 0;
}

int HI_SDK_ISP_destroy()
{
	if(_isp_attr.ispCfgAttr){
		free(_isp_attr.ispCfgAttr);
	}
	SOC_CHECK(HI_MPI_ISP_Exit());
	return 0;
}

