
#include "hi3516a.h"
#include "hi3516a_isp_sensor.h"
#include "sdk/sdk_debug.h"
#include "sdk/sdk_api.h"
#include "sdk/sdk_isp_def.h"
#include "hi_isp_api.h"
#include "hi_isp.h"
#include "signal.h"
#include "hi_spi.h"

//#include "hi_ssp.h"
#include "hi_isp_cfg.h"
#include "pan_tilt.h"
#include "hi3516a_vi_info.h"
#include "sdk_common.h"

#define HI3518A_VIN_DEV (0)
#define HI3518A_VIN_CHN (0)

#define GPIO_BASE_ADDR 0x20140000


//ir-cut photoswitch read:GPIO8_4
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 0x200f0060
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 0x20140400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 4
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 8

//ir-cut led :GPIO8_5
#define IRCUT_LED_GPIO_PINMUX_ADDR 0x200f0064
#define IRCUT_LED_GPIO_DIR_ADDR 0x20140400
#define IRCUT_LED_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_LED_GPIO_PIN 5
#define IRCUT_LED_GPIO_GROUP 8


//new hardware ir-cut control :GPIO8_6
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f0068
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 6
#define NEW_IRCUT_CTRL_GPIO_GROUP 8

//old hardware ir-cut control :GPIO8_7
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f006c
#define IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_CTRL_GPIO_PIN 7
#define IRCUT_CTRL_GPIO_GROUP 8

//default factory reset:GPIO0_7
#define HW_RESET_GPIO_PINMUX_ADDR 0x200f013c
#define HW_RESET_GPIO_DIR_ADDR 0x20140400
#define HW_RESET_GPIO_DATA_ADDR 0x201403fc
#define HW_RESET_GPIO_PIN 7
#define HW_RESET_GPIO_GROUP 8


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
	stHiIspStrength aeHistOffset;
	stHiIspStrength wdr;
	stHiIspStrength denoise3d_GlobalStrength;
	uint8_t isp_auto_drc_enabled;
	stSensorAfAttr AfAttr;
	uint8_t isp_framerate_status;	
	uint8_t filter_frequency;
	LpIspCfgAttr ispCfgAttr;
	WDR_MODE_E isp_wdr_mode;
	bool vpss_flip;
	bool vpss_mirror;
	pthread_t isp_run_pid;

	int (*sensor_set_mirror)(bool mirror);
	int (*sensor_set_flip)(bool flip);
	int (*sensor_get_resolution)(uint32_t * ret_width,uint32_t * ret_height);
	int (*sensor_get_name)(char *sensor_name);
}stHiIspAttr, *lpHiIspAttr;

static stHiIspAttr _isp_attr;

const HI_U16 gs_Gamma[17][GAMMA_NODE_NUMBER] = {
	{		0x0,0x28,0x60,0xb0,0xf8,0x140,0x170,0x194,0x1db,0x207,0x24f,0x295,0x2d7,0x316,0x352,0x38c,
0x3c3,0x3f7,0x429,0x459,0x487,0x4b4,0x4de,0x507,0x52e,0x553,0x578,0x59b,0x5be,0x5df,0x600,0x620,
0x63f,0x65e,0x67c,0x69a,0x6b7,0x6d3,0x6ef,0x70b,0x726,0x741,0x75b,0x775,0x78e,0x7a7,0x7c0,0x7d9,
0x7f1,0x80a,0x822,0x83a,0x851,0x869,0x881,0x898,0x8b0,0x8c8,0x8df,0x8f7,0x90f,0x927,0x93f,0x957,
0x970,0x988,0x9a1,0x9b9,0x9d2,0x9ea,0xa02,0xa1a,0xa32,0xa49,0xa60,0xa76,0xa8c,0xaa2,0xab6,0xacb,
0xade,0xaf1,0xb03,0xb14,0xb24,0xb33,0xb42,0xb4f,0xb5b,0xb66,0xb6f,0xb78,0xb7f,0xb91,0xba2,0xbb4,
0xbc5,0xbd7,0xbe8,0xbf9,0xc09,0xc1a,0xc2a,0xc3a,0xc4a,0xc59,0xc69,0xc78,0xc87,0xc95,0xca3,0xcb1,
0xcbf,0xccc,0xcd9,0xce5,0xcf1,0xcfd,0xd08,0xd14,0xd1e,0xd28,0xd32,0xd3b,0xd44,0xd4d,0xd55,0xd5c,
0xd63,0xd6a,0xd70,0xd75,0xd7a,0xd7f,0xd88,0xd92,0xd9c,0xda5,0xdaf,0xdb8,0xdc1,0xdca,0xdd4,0xddd,
0xde6,0xdee,0xdf7,0xe00,0xe08,0xe10,0xe19,0xe21,0xe29,0xe30,0xe38,0xe3f,0xe47,0xe4e,0xe55,0xe5b,
0xe62,0xe68,0xe6e,0xe74,0xe7a,0xe80,0xe85,0xe8a,0xe8f,0xe93,0xe98,0xe9c,0xea0,0xea3,0xea6,0xea9,
0xeac,0xeaf,0xeb3,0xeb7,0xebb,0xec0,0xec4,0xec9,0xecd,0xed2,0xed7,0xedc,0xee0,0xee5,0xeea,0xeef,
0xef4,0xef9,0xefe,0xf03,0xf08,0xf0d,0xf12,0xf18,0xf1d,0xf22,0xf27,0xf2c,0xf32,0xf37,0xf3c,0xf41,
0xf46,0xf4b,0xf51,0xf56,0xf5b,0xf60,0xf65,0xf6a,0xf6f,0xf74,0xf79,0xf7e,0xf83,0xf88,0xf8c,0xf91,
0xf96,0xf9a,0xf9f,0xfa4,0xfa8,0xfac,0xfb1,0xfb5,0xfb9,0xfbd,0xfc1,0xfc5,0xfc9,0xfcc,0xfd0,0xfd4,
0xfd7,0xfda,0xfde,0xfe1,0xfe4,0xfe7,0xfea,0xfec,0xfef,0xff1,0xff3,0xff6,0xff8,0xffa,0xffb,0xffd,
0xfff,
	},
	{
		96,160,205,237,262,294,330,374,422,474,528,592,672,739,795,843,886,926,963,997,1029,1061,1090,
		1118,1147,1176,1205,1232,1259,1286,1314,1341,1368,1395,1421,1446,1472,1498,1523,1549,1574,1600,
		1626,1651,1677,1702,1728,1754,1779,1805,1830,1854,1878,1902,1926,1950,1974,1994,2013,2032,2051,
		2070,2090,2109,2127,2146,2164,2182,2201,2219,2238,2255,2273,2290,2308,2326,2343,2361,2378,2394,
		2410,2426,2442,2458,2474,2490,2506,2522,2538,2554,2570,2586,2602,2618,2634,2649,2664,2679,2694,
		2710,2723,2737,2750,2764,2778,2791,2805,2818,2832,2846,2858,2871,2884,2897,2910,2922,2935,2948,
		2961,2973,2985,2997,3009,3021,3033,3045,3056,3067,3078,3090,3101,3112,3123,3134,3146,3155,3165,
		3174,3184,3194,3203,3213,3222,3232,3242,3251,3261,3270,3280,3290,3299,3307,3315,3323,3331,3339,
		3347,3355,3363,3371,3379,3387,3395,3403,3411,3419,3427,3435,3443,3450,3456,3462,3469,3475,3482,
		3488,3494,3501,3507,3514,3520,3526,3533,3539,3546,3552,3558,3565,3571,3576,3581,3586,3590,3595,
		3600,3605,3610,3614,3619,3624,3629,3634,3638,3643,3648,3653,3658,3662,3667,3672,3677,3682,3686,
		3691,3696,3701,3706,3710,3715,3720,3725,3730,3734,3739,3744,3749,3754,3758,3763,3768,3773,3778,
		3782,3787,3792,3797,3802,3806,3811,3816,3821,3826,3830,3835,3840,3846,3854,3864,3875,3888,3901,
		3915,3930,3946,3962,3981,4000,
	},
	{				
		0x0, 0x50, 0xA3, 0xF7, 0x14B, 0x19E, 0x1EE, 0x23A, 0x27F, 0x2BE, 0x303, 0x33E, 0x372, 0x39F, 0x3C9, 
		0x3F3, 0x41D, 0x442, 0x464, 0x485, 0x4A4, 0x4C3, 0x4E2, 0x502, 0x524, 0x546, 0x567, 0x588, 0x5AA, 0x5CD, 
		0x5F3, 0x61C, 0x649, 0x662, 0x67D, 0x699, 0x6B7, 0x6D5, 0x6F5, 0x715, 0x735, 0x755, 0x775, 0x794, 0x7B2, 
		0x7CF, 0x7EB, 0x805, 0x81D, 0x833, 0x847, 0x85A, 0x86C, 0x87C, 0x88C, 0x89B, 0x8AA, 0x8B8, 0x8C6, 0x8D4, 
		0x8E3, 0x8F2, 0x902, 0x912, 0x924, 0x937, 0x94A, 0x95E, 0x972, 0x986, 0x99B, 0x9B0, 0x9C5, 0x9DA, 0x9EF, 
		0xA04, 0xA18, 0xA2C, 0xA40, 0xA53, 0xA66, 0xA78, 0xA8A, 0xA9C, 0xAAE, 0xAC0, 0xAD1, 0xAE3, 0xAF3, 0xB04, 
		0xB14, 0xB24, 0xB34, 0xB43, 0xB51, 0xB5F, 0xB6D, 0xB7A, 0xB86, 0xB91, 0xB9B, 0xBA5, 0xBAE, 0xBB7, 0xBC0, 
		0xBC8, 0xBD1, 0xBDA, 0xBE4, 0xBEE, 0xBF8, 0xC04, 0xC10, 0xC1D, 0xC2B, 0xC3A, 0xC49, 0xC59, 0xC69, 0xC7A, 
		0xC8A, 0xC9B, 0xCAB, 0xCBC, 0xCCB, 0xCDB, 0xCEA, 0xCF8, 0xD06, 0xD13, 0xD20, 0xD2D, 0xD3A, 0xD46, 0xD52, 
		0xD5E, 0xD6A, 0xD75, 0xD80, 0xD8A, 0xD94, 0xD9D, 0xDA6, 0xDAE, 0xDB6, 0xDBD, 0xDC2, 0xDC7, 0xDCB, 0xDCF, 
		0xDD2, 0xDD4, 0xDD7, 0xDD9, 0xDDB, 0xDDD, 0xDE0, 0xDE3, 0xDE7, 0xDEB, 0xDF0, 0xDF6, 0xDFC, 0xE03, 0xE0A, 
		0xE12, 0xE1A, 0xE22, 0xE2A, 0xE33, 0xE3B, 0xE43, 0xE4A, 0xE52, 0xE59, 0xE5F, 0xE65, 0xE6A, 0xE6F, 0xE74, 
		0xE78, 0xE7C, 0xE7F, 0xE83, 0xE86, 0xE89, 0xE8C, 0xE90, 0xE93, 0xE96, 0xE99, 0xE9C, 0xEA0, 0xEA4, 0xEA7, 
		0xEAB, 0xEAE, 0xEB1, 0xEB5, 0xEB8, 0xEBB, 0xEBF, 0xEC2, 0xEC6, 0xEC9, 0xECD, 0xED1, 0xED6, 0xEDA, 0xEDF, 
		0xEE4, 0xEE9, 0xEEF, 0xEF4, 0xEFA, 0xF00, 0xF06, 0xF0C, 0xF12, 0xF18, 0xF1D, 0xF23, 0xF28, 0xF2D, 0xF32, 
		0xF36, 0xF3A, 0xF3E, 0xF41, 0xF44, 0xF46, 0xF49, 0xF4C, 0xF4F, 0xF52, 0xF55, 0xF59, 0xF5D, 0xF62, 0xF67, 
		0xF6D, 0xF74, 0xF7B, 0xF83, 0xF8B, 0xF94, 0xF9D, 0xFA7, 0xFB1, 0xFBA, 0xFC4, 0xFCE, 0xFD8, 0xFE2, 0xFEC, 
		0xFF6, 0xFFF
		},
	{
		0x0,0x11,0x2b,0x4d,0x75,0xa1,0xd0,0xff,0x12f,0x15e,0x18d,0x1bc,0x1ea,0x217,0x244,0x271,
		0x29d,0x2c9,0x2f4,0x31f,0x349,0x373,0x39d,0x3c6,0x3ef,0x417,0x43f,0x466,0x48d,0x4b4,0x4da,0x500,
		0x525,0x54a,0x56f,0x593,0x5b7,0x5da,0x5fd,0x620,0x642,0x664,0x685,0x6a7,0x6c7,0x6e7,0x707,0x727,
		0x746,0x765,0x783,0x7a1,0x7bf,0x7dc,0x7f9,0x816,0x832,0x84e,0x869,0x884,0x89f,0x8b9,0x8d3,0x8ed,
		0x907,0x920,0x938,0x951,0x969,0x980,0x998,0x9af,0x9c6,0x9dc,0x9f2,0xa08,0xa1e,0xa33,0xa48,0xa5d,
		0xa71,0xa85,0xa99,0xaad,0xac0,0xad3,0xae6,0xaf9,0xb0b,0xb1d,0xb2f,0xb40,0xb52,0xb63,0xb74,0xb84,
		0xb95,0xba5,0xbb5,0xbc5,0xbd5,0xbe4,0xbf3,0xc02,0xc11,0xc20,0xc2e,0xc3c,0xc4b,0xc58,0xc66,0xc74,
		0xc81,0xc8f,0xc9c,0xca9,0xcb5,0xcc2,0xccf,0xcdb,0xce7,0xcf4,0xd00,0xd0b,0xd17,0xd23,0xd2e,0xd3a,
		0xd45,0xd50,0xd5b,0xd66,0xd71,0xd7b,0xd86,0xd90,0xd9b,0xda5,0xdaf,0xdb9,0xdc3,0xdcc,0xdd6,0xddf,
		0xde9,0xdf2,0xdfb,0xe04,0xe0d,0xe16,0xe1f,0xe27,0xe30,0xe38,0xe40,0xe48,0xe50,0xe58,0xe60,0xe68,
		0xe70,0xe77,0xe7f,0xe86,0xe8d,0xe95,0xe9c,0xea3,0xeaa,0xeb0,0xeb7,0xebe,0xec4,0xecb,0xed1,0xed7,
		0xedd,0xee4,0xeea,0xef0,0xef5,0xefb,0xf01,0xf06,0xf0c,0xf11,0xf17,0xf1c,0xf21,0xf26,0xf2b,0xf30,
		0xf35,0xf3a,0xf3f,0xf44,0xf48,0xf4d,0xf51,0xf56,0xf5a,0xf5e,0xf63,0xf67,0xf6b,0xf6f,0xf73,0xf77,
		0xf7b,0xf7f,0xf82,0xf86,0xf8a,0xf8d,0xf91,0xf94,0xf98,0xf9b,0xf9e,0xfa2,0xfa5,0xfa8,0xfab,0xfae,
		0xfb1,0xfb4,0xfb7,0xfba,0xfbd,0xfc0,0xfc2,0xfc5,0xfc8,0xfca,0xfcd,0xfd0,0xfd2,0xfd5,0xfd7,0xfda,
		0xfdc,0xfde,0xfe1,0xfe3,0xfe5,0xfe7,0xfea,0xfec,0xfee,0xff0,0xff2,0xff4,0xff6,0xff8,0xffb,0xffd,
		0xfff
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
//[6]ar0330
		{0x0,0x25,0x4A,0x6F,0x94,0xB9,0xDE,0x103,0x128,0x14D,0x172,0x198,0x1BE,0x1E4,0x20A,0x231,0x259,
		0x281,0x2AA,0x2D3,0x2FE,0x329,0x354,0x37F,0x3AB,0x3D6,0x401,0x42B,0x455,0x47E,0x4A6,0x4CE,0x4F4,
		0x519,0x53D,0x561,0x585,0x5A8,0x5CA,0x5EC,0x60E,0x62F,0x64F,0x66E,0x68D,0x6AC,0x6CA,0x6E7,0x704,
		0x71F,0x73A,0x754,0x76E,0x786,0x79E,0x7B6,0x7CD,0x7E4,0x7FA,0x810,0x826,0x83B,0x851,0x866,0x87C,
		0x891,0x8A6,0x8BB,0x8CF,0x8E3,0x8F7,0x90B,0x91F,0x932,0x945,0x958,0x96B,0x97D,0x98F,0x9A1,0x9B3,
		0x9C5,0x9D7,0x9E8,0x9F9,0xA0A,0xA1B,0xA2B,0xA3C,0xA4C,0xA5C,0xA6C,0xA7C,0xA8C,0xA9B,0xAAB,0xABA,
		0xAC9,0xAD8,0xAE6,0xAF5,0xB03,0xB12,0xB20,0xB2E,0xB3C,0xB4A,0xB57,0xB65,0xB72,0xB80,0xB8D,0xB9A,
		0xBA7,0xBB4,0xBC0,0xBCD,0xBD9,0xBE6,0xBF2,0xBFE,0xC0A,0xC16,0xC22,0xC2E,0xC39,0xC45,0xC50,0xC5C,
		0xC67,0xC72,0xC7D,0xC88,0xC93,0xC9E,0xCA8,0xCB3,0xCBE,0xCC8,0xCD2,0xCDD,0xCE7,0xCF1,0xCFB,0xD05,
		0xD0F,0xD18,0xD22,0xD2C,0xD35,0xD3F,0xD48,0xD52,0xD5B,0xD64,0xD6D,0xD76,0xD7F,0xD88,0xD91,0xD9A,
		0xDA2,0xDAB,0xDB4,0xDBC,0xDC5,0xDCD,0xDD5,0xDDE,0xDE6,0xDEE,0xDF6,0xDFE,0xE06,0xE0E,0xE16,0xE1E,
		0xE25,0xE2D,0xE35,0xE3C,0xE44,0xE4C,0xE53,0xE5A,0xE62,0xE69,0xE70,0xE77,0xE7F,0xE86,0xE8D,0xE94,
		0xE9B,0xEA2,0xEA8,0xEAF,0xEB6,0xEBD,0xEC3,0xECA,0xED1,0xED7,0xEDE,0xEE4,0xEEB,0xEF1,0xEF7,0xEFE,
		0xF04,0xF0A,0xF10,0xF17,0xF1D,0xF23,0xF29,0xF2F,0xF35,0xF3B,0xF41,0xF46,0xF4C,0xF52,0xF58,0xF5D,
		0xF63,0xF69,0xF6E,0xF74,0xF7A,0xF7F,0xF85,0xF8A,0xF8F,0xF95,0xF9A,0xF9F,0xFA5,0xFAA,0xFAF,0xFB4,
		0xFBA,0xFBF,0xFC4,0xFC9,0xFCE,0xFD3,0xFD8,0xFDD,0xFE2,0xFE7,0xFEC,0xFF1,0xFF5,0xFFA,0xFFF,0xFFF,
	},
//[7]ov4689
	{	0x0,0x5D,0xBB,0x11A,0x178,0x1D6,0x232,0x28B,0x2E2,0x31A,0x353,0x38A,0x3C1,0x3F7,0x42C,0x460,
		0x493,0x4C4,0x4F4,0x523,0x54F,0x57A,0x5A2,0x5C8,0x5ED,0x610,0x632,0x654,0x675,0x697,0x6B8,0x6DB,
		0x6FE,0x719,0x735,0x751,0x76D,0x789,0x7A6,0x7C2,0x7DE,0x7FA,0x816,0x831,0x84C,0x866,0x87F,0x898,
		0x8B0,0x8C7,0x8DD,0x8F3,0x908,0x91C,0x930,0x944,0x957,0x96A,0x97C,0x98F,0x9A1,0x9B3,0x9C5,0x9D8,
		0x9EA,0x9FC,0xA0F,0xA21,0xA33,0xA46,0xA57,0xA69,0xA7B,0xA8C,0xA9E,0xAAF,0xABF,0xAD0,0xAE0,0xAF0,
		0xAFF,0xB0E,0xB1D,0xB2B,0xB39,0xB47,0xB55,0xB62,0xB6F,0xB7C,0xB89,0xB95,0xBA2,0xBAE,0xBBA,0xBC6,
		0xBD2,0xBDE,0xBE9,0xBF4,0xBFF,0xC0A,0xC15,0xC1F,0xC2A,0xC34,0xC3E,0xC48,0xC52,0xC5C,0xC66,0xC70,
		0xC7A,0xC84,0xC8E,0xC98,0xCA2,0xCAD,0xCB7,0xCC1,0xCCA,0xCD4,0xCDE,0xCE7,0xCF1,0xCFA,0xD03,0xD0C,
		0xD14,0xD1C,0xD24,0xD2C,0xD33,0xD3A,0xD41,0xD48,0xD4F,0xD55,0xD5C,0xD63,0xD69,0xD70,0xD77,0xD7E,
		0xD85,0xD8C,0xD94,0xD9B,0xDA3,0xDAA,0xDB2,0xDBA,0xDC2,0xDC9,0xDD1,0xDD8,0xDDF,0xDE6,0xDED,0xDF4,
		0xDFA,0xE00,0xE06,0xE0B,0xE11,0xE16,0xE1B,0xE20,0xE25,0xE2A,0xE2F,0xE33,0xE38,0xE3D,0xE41,0xE46,
		0xE4B,0xE50,0xE55,0xE5A,0xE5F,0xE64,0xE69,0xE6E,0xE73,0xE78,0xE7C,0xE81,0xE86,0xE8A,0xE8F,0xE93,
		0xE97,0xE9B,0xE9F,0xEA3,0xEA6,0xEAA,0xEAE,0xEB1,0xEB4,0xEB8,0xEBB,0xEBE,0xEC1,0xEC4,0xEC7,0xECA,
		0xECD,0xED0,0xED3,0xED6,0xED8,0xEDB,0xEDE,0xEE0,0xEE3,0xEE5,0xEE8,0xEEA,0xEEC,0xEEE,0xEF0,0xEF2,
		0xEF4,0xEF6,0xEF7,0xEF9,0xEFA,0xEFB,0xEFC,0xEFD,0xEFE,0xEFF,0xF00,0xF01,0xF02,0xF02,0xF04,0xF05,
		0xF06,0xF07,0xF09,0xF0A,0xF0C,0xF0D,0xF0F,0xF10,0xF12,0xF14,0xF15,0xF17,0xF19,0xF1A,0xF1C,0xF1D,
		0xF1F,
	},
	//[8]imx178
	{	0x0,0x44,0x88,0xCC,0x110,0x154,0x199,0x1E0,0x217,0x24E,0x287,0x2BF,0x2F8,0x331,0x36A,0x3A3,0x3DD,
		0x3FD,0x41E,0x43F,0x461,0x483,0x4A5,0x4C7,0x4E8,0x50A,0x52B,0x54C,0x56D,0x58C,0x5AC,0x5CA,0x5E8,
		0x604,0x620,0x63B,0x655,0x66F,0x688,0x6A1,0x6BA,0x6D2,0x6EA,0x703,0x71B,0x733,0x74C,0x764,0x77E,
		0x797,0x7B1,0x7CB,0x7E5,0x7FF,0x819,0x833,0x84D,0x867,0x880,0x89A,0x8B3,0x8CC,0x8E4,0x8FC,0x914,
		0x92B,0x942,0x958,0x96F,0x985,0x99B,0x9B0,0x9C5,0x9DA,0x9EF,0xA04,0xA18,0xA2C,0xA3F,0xA53,0xA66,
		0xA79,0xA8B,0xA9D,0xAAF,0xAC1,0xAD3,0xAE4,0xAF4,0xB05,0xB15,0xB25,0xB34,0xB43,0xB51,0xB5F,0xB6D,
		0xB7A,0xB86,0xB91,0xB9B,0xBA5,0xBAE,0xBB7,0xBC0,0xBC8,0xBD1,0xBDA,0xBE4,0xBEE,0xBF8,0xC04,0xC10,
		0xC1D,0xC2B,0xC3A,0xC49,0xC59,0xC69,0xC7A,0xC8A,0xC9B,0xCAB,0xCBC,0xCCB,0xCDB,0xCEA,0xCF8,0xD06,
		0xD13,0xD20,0xD2D,0xD3A,0xD46,0xD52,0xD5E,0xD6A,0xD75,0xD80,0xD8A,0xD94,0xD9D,0xDA6,0xDAE,0xDB6,
		0xDBD,0xDC2,0xDC7,0xDCB,0xDCF,0xDD2,0xDD4,0xDD7,0xDD9,0xDDB,0xDDD,0xDE0,0xDE3,0xDE7,0xDEB,0xDF0,
		0xDF6,0xDFC,0xE03,0xE0A,0xE12,0xE1A,0xE22,0xE2A,0xE33,0xE3B,0xE43,0xE4A,0xE52,0xE59,0xE5F,0xE65,
		0xE6A,0xE6F,0xE74,0xE78,0xE7C,0xE7F,0xE83,0xE86,0xE89,0xE8C,0xE90,0xE93,0xE96,0xE99,0xE9C,0xEA0,
		0xEA4,0xEA7,0xEAB,0xEAE,0xEB1,0xEB5,0xEB8,0xEBB,0xEBF,0xEC2,0xEC6,0xEC9,0xECD,0xED1,0xED6,0xEDA,
		0xEDF,0xEE4,0xEE9,0xEEF,0xEF4,0xEFA,0xF00,0xF06,0xF0C,0xF12,0xF18,0xF1D,0xF23,0xF28,0xF2D,0xF32,
		0xF36,0xF3A,0xF3E,0xF41,0xF44,0xF46,0xF49,0xF4C,0xF4F,0xF52,0xF55,0xF59,0xF5D,0xF62,0xF67,0xF6D,
		0xF74,0xF7B,0xF83,0xF8B,0xF94,0xF9D,0xFA7,0xFB1,0xFBA,0xFC4,0xFCE,0xFD8,0xFE2,0xFEC,0xFF6,0xFFF,
	},
	//[9]ov4689 WDR gamma
	{
        0,1,2,4,8,12,17,23,30,38,47,57,68,79,92,105,120,133,147,161,176,192,209,226,243,260,278,296,
        317,340,365,390,416,440,466,491,517,538,561,584,607,631,656,680,705,730,756,784,812,835,
        858,882,908,934,958,982,1008,1036,1064,1092,1119,1143,1167,1192,1218,1243,1269,1294,1320,
        1346,1372,1398,1424,1450,1476,1502,1528,1554,1580,1607,1633,1658,1684,1710,1735,1761,1786,
        1811,1836,1860,1884,1908,1932,1956,1979,2002,2024,2046,2068,2090,2112,2133,2154,2175,2196,
        2217,2237,2258,2278,2298,2318,2337,2357,2376,2395,2414,2433,2451,2469,2488,2505,2523,2541,
        2558,2575,2592,2609,2626,2642,2658,2674,2690,2705,2720,2735,2750,2765,2779,2793,2807,2821,
        2835,2848,2861,2874,2887,2900,2913,2925,2937,2950,2962,2974,2986,2998,3009,3021,3033,3044,
        3056,3067,3078,3088,3099,3109,3119,3129,3139,3148,3158,3168,3177,3187,3197,3207,3217,3227,
        3238,3248,3259,3270,3281,3292,3303,3313,3324,3335,3346,3357,3368,3379,3389,3400,3410,3421,
        3431,3441,3451,3461,3471,3481,3491,3501,3511,3521,3531,3541,3552,3562,3572,3583,3593,3604,
        3615,3625,3636,3646,3657,3668,3679,3689,3700,3711,3721,3732,3743,3753,3764,3774,3784,3795,
        3805,3816,3826,3837,3847,3858,3869,3880,3891,3902,3913,3925,3937,3949,3961,3973,3985,3997,
        4009,4022,4034,4046,4058,4071,4083,4095,
	},
	{//[10]ov5658 daylight
		0x0,0x45,0x8B,0xD3,0x11B,0x164,0x1AD,0x1F5,0x23D,0x285,0x2CB,0x30F,0x352,0x393,0x3D1,0x40C,
		0x445,0x47A,0x4AD,0x4DE,0x50D,0x53A,0x565,0x58F,0x5B7,0x5DE,0x604,0x628,0x64C,0x66F,0x691,0x6B2,
		0x6D4,0x6F4,0x713,0x730,0x74C,0x767,0x781,0x79B,0x7B3,0x7CB,0x7E2,0x7F8,0x80E,0x824,0x83A,0x84F,
		0x865,0x87A,0x88E,0x8A2,0x8B6,0x8C9,0x8DC,0x8EE,0x900,0x911,0x922,0x933,0x943,0x954,0x964,0x974,
		0x984,0x993,0x9A2,0x9B1,0x9BF,0x9CD,0x9DB,0x9E8,0x9F6,0xA03,0xA0F,0xA1C,0xA29,0xA35,0xA42,0xA4E,
		0xA5B,0xA67,0xA73,0xA7F,0xA8B,0xA97,0xAA3,0xAAF,0xABA,0xAC6,0xAD1,0xADC,0xAE7,0xAF2,0xAFD,0xB07,
		0xB12,0xB1C,0xB25,0xB2F,0xB38,0xB42,0xB4B,0xB54,0xB5C,0xB65,0xB6E,0xB76,0xB7F,0xB87,0xB90,0xB99,
		0xBA2,0xBAA,0xBB3,0xBBC,0xBC5,0xBCD,0xBD6,0xBDF,0xBE7,0xBF0,0xBF9,0xC01,0xC0A,0xC12,0xC1B,0xC23,
		0xC2C,0xC34,0xC3C,0xC45,0xC4D,0xC55,0xC5E,0xC66,0xC6E,0xC76,0xC7E,0xC86,0xC8E,0xC96,0xC9D,0xCA5,
		0xCAD,0xCB4,0xCBB,0xCC2,0xCC9,0xCCF,0xCD6,0xCDD,0xCE3,0xCEA,0xCF0,0xCF7,0xCFD,0xD04,0xD0A,0xD11,
		0xD18,0xD1E,0xD25,0xD2C,0xD33,0xD3A,0xD41,0xD48,0xD4F,0xD56,0xD5D,0xD64,0xD6B,0xD72,0xD78,0xD7F,
		0xD86,0xD8C,0xD92,0xD98,0xD9F,0xDA5,0xDAB,0xDB1,0xDB7,0xDBD,0xDC2,0xDC8,0xDCE,0xDD3,0xDD9,0xDDE,
		0xDE4,0xDE9,0xDEE,0xDF3,0xDF7,0xDFC,0xE01,0xE05,0xE0A,0xE0E,0xE13,0xE17,0xE1C,0xE20,0xE25,0xE29,
		0xE2E,0xE32,0xE37,0xE3B,0xE40,0xE44,0xE49,0xE4D,0xE52,0xE56,0xE5B,0xE5F,0xE63,0xE68,0xE6C,0xE70,
		0xE75,0xE79,0xE7D,0xE81,0xE85,0xE89,0xE8D,0xE90,0xE94,0xE98,0xE9C,0xEA0,0xEA4,0xEA8,0xEAB,0xEAF,
		0xEB4,0xEB8,0xEBC,0xEC0,0xEC4,0xEC8,0xECC,0xED1,0xED5,0xED9,0xEDD,0xEE1,0xEE6,0xEEA,0xEEE,0xEF2,
		0xEF7,
	},
	{//[11]ov5658 night
		0x0,0x2F,0x5F,0x8F,0xBF,0xEF,0x120,0x151,0x181,0x1B1,0x1E0,0x20F,0x23D,0x26A,0x296,0x2C1,
		0x2EA,0x312,0x339,0x35F,0x385,0x3A9,0x3CD,0x3F0,0x412,0x434,0x455,0x476,0x496,0x4B7,0x4D7,
		0x4F7,0x517,0x537,0x556,0x576,0x594,0x5B3,0x5D1,0x5EF,0x60C,0x629,0x646,0x663,0x67F,0x69B,
		0x6B6,0x6D1,0x6EC,0x706,0x721,0x73A,0x754,0x76C,0x785,0x79D,0x7B5,0x7CD,0x7E4,0x7FC,0x813,
		0x829,0x840,0x856,0x86C,0x882,0x897,0x8AD,0x8C2,0x8D7,0x8EB,0x900,0x914,0x927,0x93B,0x94E,
		0x961,0x974,0x987,0x999,0x9AB,0x9BD,0x9CE,0x9DF,0x9F0,0xA01,0xA11,0xA21,0xA31,0xA41,0xA50,
		0xA5F,0xA6E,0xA7E,0xA8C,0xA9B,0xAAA,0xAB9,0xAC7,0xAD6,0xAE4,0xAF2,0xB00,0xB0E,0xB1B,0xB29,
		0xB36,0xB43,0xB50,0xB5D,0xB69,0xB76,0xB82,0xB8E,0xB99,0xBA5,0xBB0,0xBBA,0xBC5,0xBCF,0xBDA,
		0xBE4,0xBEE,0xBF8,0xC02,0xC0C,0xC17,0xC21,0xC2C,0xC37,0xC42,0xC4D,0xC58,0xC64,0xC6F,0xC7A,
		0xC86,0xC91,0xC9C,0xCA7,0xCB2,0xCBD,0xCC7,0xCD1,0xCDB,0xCE5,0xCEE,0xCF7,0xD00,0xD09,0xD11,
		0xD1A,0xD22,0xD2A,0xD33,0xD3B,0xD43,0xD4A,0xD52,0xD5A,0xD62,0xD6A,0xD72,0xD79,0xD81,0xD88,
		0xD90,0xD97,0xD9E,0xDA5,0xDAC,0xDB3,0xDBA,0xDC1,0xDC8,0xDCF,0xDD6,0xDDD,0xDE3,0xDEA,0xDF0,
		0xDF7,0xDFD,0xE03,0xE0A,0xE10,0xE16,0xE1C,0xE23,0xE29,0xE2F,0xE36,0xE3C,0xE43,0xE49,0xE50,
		0xE57,0xE5E,0xE65,0xE6C,0xE73,0xE7A,0xE81,0xE88,0xE8E,0xE95,0xE9B,0xEA1,0xEA6,0xEAB,0xEB0,
		0xEB4,0xEB9,0xEBD,0xEC0,0xEC4,0xEC8,0xECB,0xECF,0xED2,0xED6,0xED9,0xEDD,0xEE1,0xEE5,0xEE9,
		0xEEE,0xEF2,0xEF6,0xEFB,0xF00,0xF04,0xF09,0xF0D,0xF12,0xF16,0xF1B,0xF1F,0xF24,0xF28,0xF2C,
		0xF30,0xF34,0xF38,0xF3C,0xF3F,0xF43,0xF47,0xF4A,0xF4E,0xF51,0xF55,0xF59,0xF5C,0xF60,0xF63,
		0xF67,
	},
	{//[12]ov4689  night 
		0x0,0xCF,0x185,0x1EE,0x239,0x282,0x2C4,0x2FC,0x333,0x372,0x3B0,0x3E0,0x40C,0x43A,0x468,0x490,
		0x4B8,0x4E1,0x507,0x52D,0x551,0x574,0x597,0x5B9,0x5DB,0x5FD,0x61E,0x63F,0x65F,0x67E,0x69D,0x6BC,
		0x6DB,0x6F9,0x717,0x735,0x752,0x76F,0x78C,0x7A9,0x7C5,0x7E2,0x7FF,0x81B,0x837,0x853,0x86E,0x888,
		0x8A2,0x8BB,0x8D5,0x8ED,0x906,0x91E,0x936,0x94D,0x964,0x97B,0x991,0x9A7,0x9BD,0x9D2,0x9E6,0x9FA,
		0xA0E,0xA21,0xA33,0xA45,0xA56,0xA67,0xA77,0xA87,0xA97,0xAA6,0xAB6,0xAC5,0xAD4,0xAE3,0xAF2,0xB01,
		0xB10,0xB1F,0xB2F,0xB3E,0xB4E,0xB5D,0xB6C,0xB7B,0xB8A,0xB99,0xBA7,0xBB6,0xBC4,0xBD3,0xBE1,0xBEE,
		0xBFC,0xC09,0xC16,0xC23,0xC30,0xC3C,0xC49,0xC55,0xC61,0xC6D,0xC79,0xC85,0xC91,0xC9C,0xCA8,0xCB3,
		0xCBF,0xCCA,0xCD5,0xCE0,0xCEA,0xCF5,0xCFF,0xD0A,0xD14,0xD1E,0xD29,0xD33,0xD3D,0xD47,0xD51,0xD5B,
		0xD65,0xD6E,0xD78,0xD82,0xD8C,0xD96,0xDA0,0xDAA,0xDB4,0xDBE,0xDC7,0xDD0,0xDDA,0xDE3,0xDEC,0xDF4,
		0xDFD,0xE05,0xE0C,0xE14,0xE1B,0xE23,0xE2A,0xE31,0xE37,0xE3E,0xE45,0xE4B,0xE52,0xE58,0xE5F,0xE65,
		0xE6C,0xE72,0xE78,0xE7F,0xE85,0xE8C,0xE92,0xE98,0xE9E,0xEA4,0xEAA,0xEB0,0xEB6,0xEBC,0xEC2,0xEC8,
		0xECE,0xED3,0xED9,0xEDE,0xEE4,0xEE9,0xEEF,0xEF4,0xEFA,0xEFF,0xF04,0xF09,0xF0E,0xF13,0xF18,0xF1D,
		0xF22,0xF26,0xF2B,0xF2F,0xF33,0xF37,0xF3C,0xF40,0xF44,0xF48,0xF4B,0xF4F,0xF53,0xF57,0xF5B,0xF5F,
		0xF63,0xF66,0xF6A,0xF6E,0xF72,0xF75,0xF79,0xF7D,0xF81,0xF84,0xF88,0xF8B,0xF8F,0xF93,0xF96,0xF9A,
		0xF9E,0xFA1,0xFA5,0xFA9,0xFAC,0xFB0,0xFB4,0xFB7,0xFBB,0xFBF,0xFC2,0xFC6,0xFC9,0xFCD,0xFD0,0xFD3,
		0xFD7,0xFD9,0xFDC,0xFDF,0xFE2,0xFE4,0xFE7,0xFE9,0xFEC,0xFEE,0xFF0,0xFF3,0xFF5,0xFF7,0xFFA,0xFFC,
		0xFFF,
	},
	{//[13]P4 daylight
		0x0,0x48,0x91,0xDC,0x127,0x173,0x1C0,0x20C,0x257,0x2A1,0x2EA,0x331,0x376,0x3B8,0x3F7,0x433,
		0x46B,0x49F,0x4D1,0x4FF,0x52B,0x554,0x57B,0x5A1,0x5C4,0x5E6,0x607,0x627,0x646,0x664,0x682,0x6A0,
		0x6BE,0x6DC,0x6F8,0x713,0x72D,0x746,0x75F,0x776,0x78D,0x7A3,0x7B8,0x7CD,0x7E1,0x7F5,0x809,0x81D,
		0x830,0x843,0x855,0x867,0x877,0x888,0x897,0x8A7,0x8B6,0x8C5,0x8D3,0x8E1,0x8F0,0x8FE,0x90D,0x91B,
		0x92A,0x939,0x948,0x957,0x966,0x976,0x985,0x994,0x9A2,0x9B1,0x9BF,0x9CE,0x9DB,0x9E9,0x9F6,0xA03,
		0xA0F,0xA1B,0xA26,0xA31,0xA3C,0xA46,0xA50,0xA59,0xA63,0xA6C,0xA75,0xA7E,0xA87,0xA90,0xA98,0xAA1,
		0xAAA,0xAB3,0xABB,0xAC4,0xACD,0xAD5,0xADD,0xAE5,0xAEE,0xAF6,0xAFD,0xB05,0xB0D,0xB14,0xB1C,0xB23,
		0xB2A,0xB31,0xB38,0xB3E,0xB45,0xB4B,0xB51,0xB57,0xB5D,0xB63,0xB69,0xB6F,0xB75,0xB7B,0xB80,0xB86,
		0xB8C,0xB92,0xB98,0xB9E,0xBA3,0xBA9,0xBAF,0xBB5,0xBBA,0xBC0,0xBC5,0xBCB,0xBD0,0xBD6,0xBDB,0xBE0,
		0xBE5,0xBEA,0xBEF,0xBF3,0xBF8,0xBFC,0xC00,0xC04,0xC09,0xC0D,0xC11,0xC15,0xC19,0xC1D,0xC21,0xC26,
		0xC2A,0xC2E,0xC33,0xC38,0xC3C,0xC41,0xC46,0xC4A,0xC4F,0xC54,0xC58,0xC5D,0xC62,0xC66,0xC6A,0xC6F,
		0xC73,0xC77,0xC7B,0xC7F,0xC83,0xC87,0xC8A,0xC8E,0xC92,0xC95,0xC99,0xC9D,0xCA0,0xCA4,0xCA7,0xCAB,
		0xCAF,0xCB3,0xCB7,0xCBB,0xCBF,0xCC3,0xCC7,0xCCB,0xCCF,0xCD3,0xCD7,0xCDB,0xCDF,0xCE2,0xCE6,0xCEA,
		0xCED,0xCF0,0xCF4,0xCF7,0xCFA,0xCFD,0xD00,0xD02,0xD05,0xD08,0xD0B,0xD0E,0xD10,0xD13,0xD16,0xD19,
		0xD1C,0xD1F,0xD22,0xD25,0xD29,0xD2C,0xD2F,0xD32,0xD35,0xD39,0xD3C,0xD3F,0xD42,0xD45,0xD48,0xD4B,
		0xD4E,0xD51,0xD54,0xD56,0xD59,0xD5B,0xD5E,0xD61,0xD63,0xD66,0xD68,0xD6A,0xD6D,0xD6F,0xD72,0xD74,
		0xD77,
	},
	{//[14]imx326-day
		0x0,0x50,0xA3,0xF5,0x148,0x199,0x1E8,0x233,0x27A,0x2BD,0x2FC,0x338,0x372,0x3AA,0x3DF,0x414,
		0x447,0x479,0x4A9,0x4D7,0x504,0x52F,0x559,0x582,0x5A9,0x5CF,0x5F3,0x615,0x636,0x656,0x676,0x695,
		0x6B4,0x6D3,0x6F1,0x70F,0x72C,0x748,0x764,0x77F,0x79A,0x7B4,0x7CE,0x7E7,0x7FF,0x817,0x82E,0x845,
		0x85C,0x872,0x887,0x89C,0x8B1,0x8C5,0x8D9,0x8EC,0x900,0x913,0x927,0x93A,0x94D,0x95F,0x971,0x983,
		0x994,0x9A5,0x9B5,0x9C5,0x9D5,0x9E4,0x9F3,0xA01,0xA10,0xA1E,0xA2C,0xA3A,0xA47,0xA54,0xA61,0xA6E,
		0xA7B,0xA88,0xA94,0xAA0,0xAAC,0xAB8,0xAC5,0xAD1,0xADD,0xAEA,0xAF6,0xB03,0xB10,0xB1D,0xB29,0xB36,
		0xB42,0xB4E,0xB5A,0xB66,0xB72,0xB7E,0xB89,0xB95,0xBA0,0xBAB,0xBB7,0xBC2,0xBCD,0xBD9,0xBE4,0xBEF,
		0xBFA,0xC05,0xC10,0xC1C,0xC27,0xC32,0xC3D,0xC49,0xC54,0xC5F,0xC69,0xC74,0xC7F,0xC89,0xC93,0xC9D,
		0xCA7,0xCB0,0xCBA,0xCC3,0xCCB,0xCD4,0xCDD,0xCE5,0xCED,0xCF5,0xCFD,0xD05,0xD0D,0xD15,0xD1D,0xD25,
		0xD2D,0xD35,0xD3D,0xD45,0xD4D,0xD55,0xD5D,0xD65,0xD6D,0xD75,0xD7D,0xD84,0xD8C,0xD94,0xD9B,0xDA3,
		0xDAA,0xDB1,0xDB8,0xDBF,0xDC6,0xDCD,0xDD4,0xDDA,0xDE1,0xDE8,0xDEE,0xDF5,0xDFB,0xE02,0xE09,0xE0F,
		0xE16,0xE1D,0xE24,0xE2A,0xE31,0xE38,0xE3F,0xE46,0xE4D,0xE53,0xE5A,0xE61,0xE68,0xE6F,0xE76,0xE7D,
		0xE84,0xE8B,0xE92,0xE99,0xEA1,0xEA8,0xEAF,0xEB7,0xEBE,0xEC5,0xECC,0xED4,0xEDB,0xEE2,0xEE9,0xEEF,
		0xEF6,0xEFC,0xF03,0xF09,0xF0F,0xF15,0xF1B,0xF21,0xF27,0xF2D,0xF33,0xF39,0xF3E,0xF44,0xF4A,0xF4F,
		0xF55,0xF5B,0xF60,0xF66,0xF6B,0xF71,0xF76,0xF7C,0xF81,0xF87,0xF8C,0xF91,0xF97,0xF9C,0xFA1,0xFA7,
		0xFAC,0xFB1,0xFB7,0xFBC,0xFC1,0xFC6,0xFCB,0xFD1,0xFD6,0xFDB,0xFE0,0xFE5,0xFEA,0xFEF,0xFF5,0xFFA,
		0xFFF,
	},
	//05a-day
	{
	0x0,0x42,0x85,0xCA,0x10F,0x155,0x19C,0x1E1,0x227,0x26B,0x2AE,0x2EF,0x32E,0x36B,0x3A5,0x3DC,
	0x410,0x440,0x46E,0x498,0x4C1,0x4E7,0x50B,0x52E,0x54F,0x56E,0x58D,0x5AA,0x5C7,0x5E3,0x5FF,0x61A,
	0x636,0x651,0x66B,0x684,0x69B,0x6B2,0x6C7,0x6DC,0x6F1,0x705,0x718,0x72B,0x73E,0x750,0x763,0x775,
	0x788,0x79B,0x7AD,0x7BF,0x7D0,0x7E2,0x7F3,0x803,0x814,0x824,0x834,0x844,0x854,0x864,0x873,0x883,
	0x892,0x8A2,0x8B1,0x8C0,0x8CF,0x8DE,0x8ED,0x8FB,0x90A,0x918,0x927,0x935,0x943,0x951,0x95F,0x96D,
	0x97A,0x988,0x995,0x9A3,0x9B0,0x9BD,0x9CB,0x9D8,0x9E5,0x9F1,0x9FE,0xA0B,0xA18,0xA24,0xA31,0xA3D,
	0xA4A,0xA56,0xA62,0xA6E,0xA7B,0xA87,0xA93,0xA9E,0xAAA,0xAB6,0xAC2,0xACD,0xAD9,0xAE5,0xAF0,0xAFC,
	0xB07,0xB12,0xB1D,0xB29,0xB34,0xB3F,0xB4A,0xB55,0xB60,0xB6B,0xB76,0xB80,0xB8B,0xB96,0xBA1,0xBAB,
	0xBB6,0xBC0,0xBCB,0xBD5,0xBE0,0xBEA,0xBF4,0xBFE,0xC09,0xC13,0xC1D,0xC27,0xC31,0xC3B,0xC45,0xC4F,
	0xC59,0xC63,0xC6D,0xC76,0xC80,0xC8A,0xC93,0xC9D,0xCA7,0xCB0,0xCBA,0xCC3,0xCCD,0xCD6,0xCE0,0xCE9,
	0xCF2,0xCFC,0xD05,0xD0E,0xD17,0xD21,0xD2A,0xD33,0xD3C,0xD45,0xD4E,0xD57,0xD60,0xD69,0xD72,0xD7B,
	0xD84,0xD8C,0xD95,0xD9E,0xDA7,0xDAF,0xDB8,0xDC1,0xDCA,0xDD2,0xDDB,0xDE3,0xDEC,0xDF4,0xDFD,0xE05,
	0xE0E,0xE16,0xE1F,0xE27,0xE2F,0xE38,0xE40,0xE48,0xE50,0xE59,0xE61,0xE69,0xE71,0xE79,0xE82,0xE8A,
	0xE92,0xE9A,0xEA2,0xEAA,0xEB2,0xEBA,0xEC2,0xECA,0xED2,0xED9,0xEE1,0xEE9,0xEF1,0xEF9,0xF01,0xF08,
	0xF10,0xF18,0xF20,0xF27,0xF2F,0xF37,0xF3E,0xF46,0xF4E,0xF55,0xF5D,0xF64,0xF6C,0xF73,0xF7B,0xF82,
	0xF8A,0xF91,0xF99,0xFA0,0xFA7,0xFAF,0xFB6,0xFBE,0xFC5,0xFCC,0xFD4,0xFDB,0xFE2,0xFE9,0xFF1,0xFF8,
	0xFFF,
	},
	//05a-night
	{
	0x0,0x34,0x64,0x9D,0xEC,0x10D,0x132,0x15A,0x184,0x1B0,0x1DE,0x20D,0x23B,0x269,0x297,0x2C2,
	0x2EC,0x30A,0x327,0x345,0x363,0x380,0x39D,0x3BA,0x3D7,0x3F3,0x40F,0x42B,0x446,0x461,0x47B,
	0x495,0x4AF,0x4C7,0x4E0,0x4F7,0x50F,0x525,0x53C,0x552,0x568,0x57D,0x593,0x5A8,0x5BD,0x5D2,
	0x5E7,0x5FC,0x612,0x627,0x63C,0x651,0x666,0x67B,0x690,0x6A5,0x6B9,0x6CE,0x6E2,0x6F6,0x70A,
	0x71E,0x732,0x745,0x759,0x76C,0x77F,0x792,0x7A4,0x7B7,0x7C9,0x7DC,0x7EE,0x800,0x812,0x823,
	0x835,0x846,0x857,0x868,0x879,0x889,0x899,0x8A9,0x8B9,0x8C9,0x8D8,0x8E7,0x8F6,0x905,0x914,
	0x923,0x931,0x940,0x94E,0x95C,0x96B,0x978,0x986,0x994,0x9A1,0x9AF,0x9BC,0x9C9,0x9D6,0x9E3,
	0x9F0,0x9FC,0xA09,0xA16,0xA22,0xA2F,0xA3C,0xA49,0xA56,0xA62,0xA6F,0xA7B,0xA87,0xA94,0xAA0,
	0xAAC,0xAB8,0xAC4,0xAD1,0xADD,0xAE9,0xAF5,0xB00,0xB0C,0xB18,0xB24,0xB30,0xB3B,0xB47,0xB53,
	0xB5E,0xB6A,0xB75,0xB81,0xB8C,0xB97,0xBA3,0xBAE,0xBB9,0xBC5,0xBD0,0xBDB,0xBE6,0xBF1,0xBFC,
	0xC07,0xC12,0xC1D,0xC28,0xC33,0xC3E,0xC49,0xC54,0xC5E,0xC69,0xC74,0xC7E,0xC89,0xC94,0xC9E,
	0xCA9,0xCB3,0xCBE,0xCC8,0xCD3,0xCDD,0xCE8,0xCF2,0xCFC,0xD07,0xD11,0xD1B,0xD25,0xD30,0xD3A,
	0xD44,0xD4E,0xD58,0xD62,0xD6C,0xD76,0xD80,0xD8A,0xD94,0xD9E,0xDA8,0xDB2,0xDBC,0xDC5,0xDCF,
	0xDD9,0xDE3,0xDED,0xDF6,0xE00,0xE0A,0xE13,0xE1D,0xE26,0xE30,0xE3A,0xE43,0xE4D,0xE56,0xE60,
	0xE69,0xE73,0xE7C,0xE85,0xE8F,0xE98,0xEA1,0xEAB,0xEB4,0xEBD,0xEC6,0xED0,0xED9,0xEE2,0xEEB,
	0xEF4,0xEFE,0xF07,0xF10,0xF19,0xF22,0xF2B,0xF34,0xF3D,0xF46,0xF4F,0xF58,0xF61,0xF6A,0xF73,
	0xF7C,0xF85,0xF8D,0xF96,0xF9F,0xFA8,0xFB1,0xFB9,0xFC2,0xFCB,0xFD4,0xFDC,0xFE5,0xFEE,0xFF6,
	0xFFF,
	},
};

static combo_dev_attr_t LVDS_4lane_SENSOR_IMX178_12BIT_5M_NOWDR_ATTR =
{
    /* input mode */
    .input_mode = INPUT_MODE_LVDS,
    {
        .lvds_attr = {
            .img_size = {2592, 1944},
            HI_WDR_MODE_NONE,
            LVDS_SYNC_MODE_SAV,
            RAW_DATA_12BIT,
            LVDS_ENDIAN_BIG,
            LVDS_ENDIAN_BIG,
            .lane_id = {0, 1, 2, 3, -1, -1, -1, -1},
            .sync_code = { 
                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}},
                    
                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}},

                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}},
                    
                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}},
                    
                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}},
                        
                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}},

                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}},
                    
                    {{0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}, 
                    {0xab0, 0xb60, 0x800, 0x9d0}} 
                }
        }
    }
};


static combo_dev_attr_t MIPI_2lane_SENSOR_AR0330_12BIT_ATTR = 
{
    .input_mode = INPUT_MODE_MIPI,  
    {

        .mipi_attr =    
        {
            RAW_DATA_12BIT,
            {0, 1, -1, -1, -1, -1, -1, -1}
        }
    }    
};

//need TODO:
static combo_dev_attr_t MIPI_4lane_SENSOR_OV4689_12BIT_ATTR = 
{
    .input_mode = INPUT_MODE_MIPI,  
    {

        .mipi_attr =    
        {
            RAW_DATA_12BIT,
            {0, 1, 2, 3, -1, -1, -1, -1}
        }
    }    
};

static combo_dev_attr_t MIPI_4lane_SENSOR_IMX185_12BIT_ATTR = 
{
    .input_mode = INPUT_MODE_MIPI,  
    {
        .mipi_attr =    
        {
            RAW_DATA_12BIT,
            {0, 1, 2, 3, -1, -1, -1, -1}
        }
    }
    
};

static combo_dev_attr_t MIPI_4lane_SENSOR_OV5658_10BIT_ATTR = 
{
    .input_mode = INPUT_MODE_MIPI,  
    {
        .mipi_attr =    
        {
            RAW_DATA_10BIT,
            {0, 1, 2, 3, -1, -1, -1, -1}
        }
    }
    
};

static combo_dev_attr_t MIPI_4lane_SENSOR_OS05A_10BIT_ATTR = 
{
    .input_mode = INPUT_MODE_MIPI,  
    {
        .mipi_attr =    
        {
            RAW_DATA_10BIT,
            {0, 1, 2, 3, -1, -1, -1, -1}
        }
    }
    
};


static combo_dev_attr_t SUBLVDS_4lane_SENSOR_MN34220_12BIT_1080_NOWDR_ATTR =
{
  //   input mode 
    .input_mode = INPUT_MODE_SUBLVDS,
        
    {
        .lvds_attr = {
            .img_size = {1920, 1080},
            HI_WDR_MODE_NONE,            
            LVDS_SYNC_MODE_SOL,
            RAW_DATA_12BIT,                     
            LVDS_ENDIAN_BIG,
            LVDS_ENDIAN_BIG, 
            .lane_id = {0, 2, -1, -1, 1, 3, -1, -1},
            .sync_code =  {
                  {{0x002, 0x003, 0x000, 0x001}, //PHY0_lane0
                  {0x202, 0x203, 0x200, 0x201},
                  {0x102, 0x103, 0x100, 0x101},
                  {0x302, 0x303, 0x300, 0x301}},

                  {{0x006, 0x007, 0x004, 0x005}, //PHY0_lane1
                  {0x206, 0x207, 0x204, 0x205},
                  {0x106, 0x107, 0x104, 0x105},
                  {0x306, 0x307, 0x304, 0x305}},

                  {{0x00a, 0x00b, 0x008, 0x009}, //PHY0_lane2
                  {0x20a, 0x20b, 0x208, 0x209},
                  {0x10a, 0x10b, 0x108, 0x109},
                  {0x30a, 0x30b, 0x308, 0x309}},

                  {{0x00a, 0x00b, 0x008, 0x009}, //PHY0_lane3  INPUT_MODE_LVDS
                  {0x20a, 0x20b, 0x208, 0x209},
                  {0x10a, 0x10b, 0x108, 0x109},
                  {0x30a, 0x30b, 0x308, 0x309}},

                  {{0x012, 0x013, 0x010, 0x011},//PHY1_lane0
                  {0x212, 0x213, 0x210, 0x211},
                  {0x112, 0x113, 0x110, 0x111},
                  {0x312, 0x313, 0x310, 0x311}},

                  {{0x016, 0x017, 0x014, 0x015}, //PHY1_lane1
                  {0x216, 0x217, 0x214, 0x215},
                  {0x116, 0x117, 0x114, 0x115},
                  {0x316, 0x317, 0x314, 0x315}},

                  {{0x01a, 0x01b, 0x018, 0x019}, //PHY1_lane2
                  {0x21a, 0x21b, 0x218, 0x219},
                  {0x11a, 0x11b, 0x118, 0x119},
                  {0x31a, 0x31b, 0x318, 0x319}},

                  {{0x01a, 0x01b, 0x018, 0x019}, //PHY1_lane3
                  {0x21a, 0x21b, 0x218, 0x219},
                  {0x11a, 0x11b, 0x118, 0x119},
                  {0x31a, 0x31b, 0x318, 0x319}}
               }
        }
    }
};

static combo_dev_attr_t SUBLVDS_4lane_SENSOR_MN34220_12BIT_1080_2WDR1_ATTR =
{
    /* input mode */
    .input_mode = INPUT_MODE_SUBLVDS,
        
    {
        .lvds_attr = {
            .img_size = {1920, 1108},
            HI_WDR_MODE_2F,            
            LVDS_SYNC_MODE_SOL,
            RAW_DATA_12BIT,                     
            LVDS_ENDIAN_BIG,
            LVDS_ENDIAN_BIG, 
            .lane_id = {0, 2, -1, -1, 1, 3, -1, -1},
            .sync_code =  {
                  {{0x002, 0x003, 0x000, 0x001}, //PHY0_lane0
                  {0x202, 0x203, 0x200, 0x201},
                  {0x102, 0x103, 0x100, 0x101},
                  {0x302, 0x303, 0x300, 0x301}},

                  {{0x006, 0x007, 0x004, 0x005}, //PHY0_lane1
                  {0x206, 0x207, 0x204, 0x205},
                  {0x106, 0x107, 0x104, 0x105},
                  {0x306, 0x307, 0x304, 0x305}},

                  {{0x00a, 0x00b, 0x008, 0x009}, //PHY0_lane2
                  {0x20a, 0x20b, 0x208, 0x209},
                  {0x10a, 0x10b, 0x108, 0x109},
                  {0x30a, 0x30b, 0x308, 0x309}},

                  {{0x00a, 0x00b, 0x008, 0x009}, //PHY0_lane3  INPUT_MODE_LVDS
                  {0x20a, 0x20b, 0x208, 0x209},
                  {0x10a, 0x10b, 0x108, 0x109},
                  {0x30a, 0x30b, 0x308, 0x309}},

                  {{0x012, 0x013, 0x010, 0x011},//PHY1_lane0
                  {0x212, 0x213, 0x210, 0x211},
                  {0x112, 0x113, 0x110, 0x111},
                  {0x312, 0x313, 0x310, 0x311}},

                  {{0x016, 0x017, 0x014, 0x015}, //PHY1_lane1
                  {0x216, 0x217, 0x214, 0x215},
                  {0x116, 0x117, 0x114, 0x115},
                  {0x316, 0x317, 0x314, 0x315}},

                  {{0x01a, 0x01b, 0x018, 0x019}, //PHY1_lane2
                  {0x21a, 0x21b, 0x218, 0x219},
                  {0x11a, 0x11b, 0x118, 0x119},
                  {0x31a, 0x31b, 0x318, 0x319}},

                  {{0x01a, 0x01b, 0x018, 0x019}, //PHY1_lane3
                  {0x21a, 0x21b, 0x218, 0x219},
                  {0x11a, 0x11b, 0x118, 0x119},
                  {0x31a, 0x31b, 0x318, 0x319}}
               } 
        }
    }
};

static combo_dev_attr_t MIPI_4lane_SENSOR_IMX326_12BIT_ATTR = 
{
    .input_mode = INPUT_MODE_MIPI,  
    {

        .mipi_attr =    
        {
            RAW_DATA_12BIT,
            {0, 1, 2, 3, -1, -1, -1, -1}
        }
    }    
};


#include "hi_i2c.h"
static int HI_SDK_ISP_set_isp_sensor_value(void);

static int _hi_sdk_isp_set_slow_framerate(uint8_t bValue);//Declaration


int HI_SDK_ISP_set_wdr_mode_ini(uint8_t mode);


static HI_S32 hi_isp_SetMipiAttr()
{
    HI_S32 fd;
    combo_dev_attr_t *pstcomboDevAttr;

    /* mipi reset unrest */
    fd = open("/dev/hi_mipi", O_RDWR);
    if (fd < 0)
    {
        printf("warning: open hi_mipi dev failed\n");
        return -1;
    }
     if (_isp_attr.sensor_type == SENSOR_MODEL_SONY_IMX185)
    {
       pstcomboDevAttr = &MIPI_4lane_SENSOR_IMX185_12BIT_ATTR;
    }
	    
    if (_isp_attr.sensor_type == SENSOR_MODEL_SONY_IMX178)
    {
        pstcomboDevAttr = &LVDS_4lane_SENSOR_IMX178_12BIT_5M_NOWDR_ATTR;
    }
	
    if (_isp_attr.sensor_type == SENSOR_MODEL_APTINA_AR0330)
    {
        pstcomboDevAttr = &MIPI_2lane_SENSOR_AR0330_12BIT_ATTR;
    }

   if (_isp_attr.sensor_type == SENSOR_MODEL_OV_OV4689)
    {
        pstcomboDevAttr = &MIPI_4lane_SENSOR_OV4689_12BIT_ATTR;
    }

	if (_isp_attr.sensor_type == SENSOR_MODEL_MN34220)
	 {
		if(_isp_attr.isp_wdr_mode == WDR_MODE_2To1_LINE){
			pstcomboDevAttr = &SUBLVDS_4lane_SENSOR_MN34220_12BIT_1080_2WDR1_ATTR;
		}else if (_isp_attr.isp_wdr_mode == WDR_MODE_NONE){
			pstcomboDevAttr = &SUBLVDS_4lane_SENSOR_MN34220_12BIT_1080_NOWDR_ATTR;
		}
	 }

	if (_isp_attr.sensor_type == SENSOR_MODEL_OV5658)
	 {
			 pstcomboDevAttr = &MIPI_4lane_SENSOR_OV5658_10BIT_ATTR;
	 }
	if (_isp_attr.sensor_type == SENSOR_MODEL_OS05A)
	 {
			 pstcomboDevAttr = &MIPI_4lane_SENSOR_OS05A_10BIT_ATTR;
	 }

    if (ioctl(fd, HI_MIPI_SET_DEV_ATTR, pstcomboDevAttr))
    {
        printf("set mipi attr failed\n");
        close(fd);
        return -1;
    }
    close(fd);

    return HI_SUCCESS;
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

//remove warning
/*static uint32_t isp_gpio_pin_read(int gpio_group, int gpio_pin)
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
}*/

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

static void isp_ircut_switch(uint8_t bEnable)//0:daytime   1:night
{
	static uint32_t old_saturation = 0;
	ISP_WB_INFO_S pstWBInfo;
	ISP_SATURATION_ATTR_S pstSatAttr;
	

	ISP_PUB_ATTR_S pstPubAttr;

	HI_MPI_ISP_GetPubAttr(0,&pstPubAttr);
	printf("old_FRate = %f\n",pstPubAttr.f32FrameRate);
	
	if(!old_saturation){
			SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));//	pstWBInfo.u16Saturation
			printf("old saturation =%d \n",pstWBInfo.u16Saturation);
//			old_saturation = pstWBInfo.u16Saturation;
			
		
		}
	if(!bEnable){
		printf("daylight mode!\r\n");	

		SOC_CHECK(HI_MPI_ISP_GetSaturationAttr(0,&pstSatAttr));
		pstSatAttr.enOpType = OP_TYPE_AUTO;		
		SOC_CHECK(HI_MPI_ISP_SetSaturationAttr(0,&pstSatAttr));
		
		//isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
		//isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
		//isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 1);	
		_isp_attr.bsp_api.BSP_SET_IR_LED(true);
		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_DAYLIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_DAYLIGHT;
	
		
		SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));				
	    printf("new saturation =%d \r\n",pstWBInfo.u16Saturation);

		//isp_ircut_control_daylight();
		
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY){			
			//pstPubAttr.f32FrameRate =  frame_rate/2;
			_hi_sdk_isp_set_slow_framerate(true);
		}else{			
			//pstPubAttr.f32FrameRate =  frame_rate;			
			_hi_sdk_isp_set_slow_framerate(false);
		}

		//SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&pstPubAttr));
		
	}else{			
		printf("night mode!\r\n");
		SOC_CHECK(HI_MPI_ISP_GetSaturationAttr(0,&pstSatAttr));
		pstSatAttr.enOpType = OP_TYPE_MANUAL;
     	pstSatAttr.stManual.u8Saturation = 0;

		SOC_CHECK(HI_MPI_ISP_SetSaturationAttr(0,&pstSatAttr));


		//isp_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1);//IR LED on
		//isp_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 1);//IR-CUT on
		//isp_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
		_isp_attr.bsp_api.BSP_SET_IR_LED(false);
		_isp_attr.bsp_api.BSP_IRCUT_SWITCH(ISP_GPIO_NIGHT);
		_isp_attr.gpio_status_old = ISP_GPIO_NIGHT;

		SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));				
	    printf("new saturation =%d \r\n",pstWBInfo.u16Saturation);
		
		//isp_ircut_control_night();
		if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_CLOSE){
		//	pstPubAttr.f32FrameRate =  frame_rate;
			
			_hi_sdk_isp_set_slow_framerate(false);
		}else{
		//	pstPubAttr.f32FrameRate =  frame_rate/2;
			_hi_sdk_isp_set_slow_framerate(true);
		}
		//SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&pstPubAttr));

	}
	HI_SDK_ISP_set_isp_sensor_value();
	
	if(_isp_attr.ispCfgAttr){
		HI_ISP_cfg_set_all(_isp_attr.gpio_status_old, 1, _isp_attr.ispCfgAttr);
	}
}



static uint32_t gains_calculate(void)
{
	uint32_t ret_iso = 1;

	ISP_DEV IspDev = 0;
    ISP_EXP_INFO_S stExpInfo ;	
 	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(IspDev,&stExpInfo));
//	ret_iso = (stExpInfo.u32AGain* ((stExpInfo.u32DGain* (stExpInfo.u32ISPDGain>> 10))>>4))>>6;
	ret_iso = (uint32_t)((unsigned long long)stExpInfo.u32AGain* (unsigned long long)stExpInfo.u32DGain* (unsigned long long)stExpInfo.u32ISPDGain >> 20);

	return ret_iso;
}

static uint32_t isp_get_iso()
{
	ISP_DEV IspDev = 0;
    ISP_EXP_INFO_S stExpInfo ;
 	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(IspDev,&stExpInfo));
	return stExpInfo.u32AGain*stExpInfo.u32DGain*100;
}

static uint8_t sdk_isp_calculate_exposure(uint32_t old_state)
{

	uint8_t ret_val = 0;
	//HI_U32 switch_area[2] = {0x20fa, 0x2544}; 
	HI_U32 switch_area[2] = {0x2844, 0x67};

	ISP_DEV IspDev = 0;
    ISP_EXP_INFO_S stExpInfo ;		
 	SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(IspDev,&stExpInfo));
	//u32Exposure[0x400,0xFFFFFFF];
	
	printf("aveLum = 0x%04x/%02x\r\n",stExpInfo.u32Exposure, stExpInfo.u8AveLum);
	if(!old_state){
		if(stExpInfo.u32Exposure > switch_area[0]){
			ret_val = 1;
		}else{
			ret_val = 0;
		}
		if(stExpInfo.u32Exposure < switch_area[1]){
			ret_val = 0;
		}else{
			ret_val = 1;
		}
	}

	printf("old_state:%d/%d\r\n", old_state, ret_val);
	

	return ret_val;//0:daytime 1:night
}

static void isp_ircut_gpio_init()
{
	uint32_t reg_val = 0;
	//muxpin
	sdk_sys->write_reg(IRCUT_LED_GPIO_PINMUX_ADDR, 0);//GPIO8_5
	//pin dir :out
	sdk_sys->read_reg(IRCUT_LED_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_LED_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_LED_GPIO_DIR_ADDR, reg_val);

	//muxpin
	sdk_sys->write_reg(NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR, 0);//GPIO8_6
	//pin dir :out
	sdk_sys->read_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<NEW_IRCUT_CTRL_GPIO_PIN);
	sdk_sys->write_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);

	
	//muxpin
	sdk_sys->write_reg(IRCUT_CTRL_GPIO_PINMUX_ADDR, 0);//GPIO8_7
	//pin dir :out
	sdk_sys->read_reg(IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_CTRL_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);
	
	//muxpin
	sdk_sys->write_reg(IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR, 0);//GPIO0_4
	//pin dir :in
	sdk_sys->read_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1<<IRCUT_PHOTOSWITCH_GPIO_PIN);
	sdk_sys->write_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, reg_val);
	

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


//remove warning
/*static int isp_get_denoise3d_global_strength_value(lpHiIspStrength val)
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
	if(ret_val> 1408){
		ret_val = 1408;
	}
	if(ret_val < 0){
		ret_val = 0;
	}

	return ret_val;

}*/

int HI_SDK_ISP_sensor_flicker(uint8_t bEnable, uint8_t frequency, uint8_t mode)
{	
	ISP_EXPOSURE_ATTR_S pstExpAttr;	
	ISP_DEV IspDev = 0;
	SOC_CHECK(HI_MPI_ISP_GetExposureAttr(IspDev,&pstExpAttr));
	if(bEnable != 0xff){
		pstExpAttr.stAuto.stAntiflicker.bEnable = bEnable;
	}else{
		//printf("pstAntiflicker.bEnable = %d\r\n", pstExpAttr.stAuto.stAntiflicker.bEnable);
		pstExpAttr.stAuto.stAntiflicker.bEnable = HI_TRUE;
	}
	pstExpAttr.stAuto.stAntiflicker.bEnable = HI_TRUE;
	
	if(frequency){
		pstExpAttr.stAuto.stAntiflicker.u8Frequency = frequency;	
		_isp_attr.filter_frequency = frequency;
		if(_isp_attr.ispCfgAttr){			
			_isp_attr.ispCfgAttr->impCfgAttr.flick_frequency = _isp_attr.filter_frequency;
		}
	}else{
		pstExpAttr.stAuto.stAntiflicker.u8Frequency = _isp_attr.filter_frequency;
	}
	//printf("%s---%d:%d\r\n", __FUNCTION__, frequency, bEnable);
	pstExpAttr.stAuto.stAntiflicker.enMode = mode ? ISP_ANTIFLICKER_AUTO_MODE:ISP_ANTIFLICKER_NORMAL_MODE;
	SOC_CHECK(HI_MPI_ISP_SetExposureAttr(IspDev,&pstExpAttr));
	//printf("%s-%d:%d/%d\r\n", __FUNCTION__, __LINE__, pstExpAttr.stAuto.stAntiflicker.bEnable,pstExpAttr.stAuto.stAntiflicker.u8Frequency);
	return 0;

}

emSENSOR_MODEL HI_SDK_ISP_sensor_check()
{
//save file for sensor check in case of saving time for checking sensor
	FILE* fd = NULL;
	char sensor_type[4] = {0};
	char cmd[32];
	fd = fopen(SENSOR_TYPE_FILE, "rb");

	if(fd){
		fread(sensor_type, sizeof(sensor_type), 1, fd);
		_isp_attr.sensor_type = atoi(sensor_type);
		fclose(fd);
		if(_isp_attr.sensor_type < SENSOR_MODEL_CNT){
			printf("get sensor type from file:%d\n", _isp_attr.sensor_type);
			goto reset_sensor;
		}
	}else{
		//need to check sensor by I2C
	}

	
	do{	

		//imx326
		if(SENSOR_IMX326_probe())
		{
			_isp_attr.sensor_type = SENSOR_MODEL_IMX326;
			break;
		}

		//ov5658
		if(SENSOR_OV5658_probe())
		{
			_isp_attr.sensor_type = SENSOR_MODEL_OV5658;
			break;
		}
	 	//ar0330
		if(SENSOR_AR0330_probe())
		{
		  _isp_attr.sensor_type = SENSOR_MODEL_APTINA_AR0330;
		  break;
		}
	 	//ov4689
		if(SENSOR_OV4689_probe())
		{
		 _isp_attr.sensor_type = SENSOR_MODEL_OV_OV4689;
		 break;
		}
		// mn34220
		if(SENSOR_MN34220_probe())
		{
		  _isp_attr.sensor_type = SENSOR_MODEL_MN34220;
		  break;
		}
	    //sony imx178
		if(SENSOR_IMX178_probe())
		{
			_isp_attr.sensor_type = SENSOR_MODEL_SONY_IMX178;
			break;
		}
	    if(SENSOR_IMX185_probe())
        {
            _isp_attr.sensor_type = SENSOR_MODEL_SONY_IMX185;
        }
		if(SENSOR_OS05A_probe())
		{
			_isp_attr.sensor_type = SENSOR_MODEL_OS05A;
		}
	}while(0);

	//set a file for for sensor check in case of saving time for checking sensor
	snprintf(cmd, sizeof(cmd), "echo %d > %s", _isp_attr.sensor_type, SENSOR_TYPE_FILE);
	system(cmd);

	
reset_sensor:
	isp_gpio_pin_write(0, 0, 1); //reset sensor 
	usleep(2000);
	isp_gpio_pin_write(0, 0, 0); //reset sensor 
	usleep(2000);
	isp_gpio_pin_write(0, 0, 1); //reset sensor 
	usleep(2000);


	//init mipi interface
	hi_isp_SetMipiAttr();

	return _isp_attr.sensor_type;
}


int HI_SDK_ISP_ircut_auto_switch(int vin, uint8_t type)//1:software   0: hardware 
{
	uint32_t gpio_status_cur;

	if(_isp_attr.ircut_auto_switch_enable){
		if(_isp_attr.ircut_control_mode == ISP_IRCUT_CONTROL_MODE_HARDWARE){//hardware detect
			//gpio_status_cur= isp_gpio_pin_read(IRCUT_PHOTOSWITCH_GPIO_GROUP, IRCUT_PHOTOSWITCH_GPIO_PIN);
			gpio_status_cur = _isp_attr.bsp_api.BSP_GET_PHOTOSWITCH();
		}else{//software detect
			gpio_status_cur = sdk_isp_calculate_exposure(_isp_attr.gpio_status_old);
		}
		gpio_status_cur = gpio_status_cur != 0 ? 1:0;
		if(_isp_attr.gpio_status_old != gpio_status_cur && ircut_edge_detect((int *)&gpio_status_cur)){
			printf("%s-%d  ircut hareware switch:%u--%u\r\n", __FUNCTION__, __LINE__, _isp_attr.gpio_status_old, gpio_status_cur);
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
/*	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 1;//main stream
	VPSS_CHN_ATTR_S stVpssChnAttr;

	_isp_attr.vpss_mirror = mirror;

	for(VpssChn = 1;VpssChn <= 3;VpssChn++){
		SOC_CHECK(HI_MPI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
		stVpssChnAttr.bMirror = mirror ? HI_TRUE : HI_FALSE;
		SOC_CHECK(HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
	}
*/
	return 0;
}

int HI_SDK_ISP_set_flip(int vin, bool flip)
{   
/*	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 1;//main stream
	VPSS_CHN_ATTR_S stVpssChnAttr;
	
	_isp_attr.vpss_flip = flip;
	
	for(VpssChn = 1;VpssChn <= 3;VpssChn++){
		SOC_CHECK(HI_MPI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
		stVpssChnAttr.bFlip= flip ? HI_TRUE : HI_FALSE;
		SOC_CHECK(HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
	}

*/	return 0;
}


int HI_SDK_ISP_set_saturation(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32SatuVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	//printf("saturation set:%d\r\n", val);

	return 0;
}

int HI_SDK_ISP_get_saturation(int vin, uint16_t *val)
{

	ISP_WB_INFO_S pstWBInfo;
	SOC_CHECK(HI_MPI_ISP_QueryWBInfo(0,&pstWBInfo));//	pstWBInfo.u16Saturation
	printf("saturation get:%d\r\n", pstWBInfo.u16Saturation);
	*val = pstWBInfo.u16Saturation;
	return 0;
}


int HI_SDK_ISP_set_contrast(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32ContrVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	//printf("contrast set:%d\r\n", val);
	return 0;
}

int HI_SDK_ISP_set_brightness(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32LumaVal= val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	//printf("brightness set:%d\r\n", val);
	return 0;
}

int HI_SDK_ISP_set_hue(int vin, uint16_t val)
{
	VI_CSC_ATTR_S pstCSCAttr;
	SOC_CHECK(HI_MPI_VI_GetCSCAttr(vin, &pstCSCAttr));
	pstCSCAttr.u32HueVal = val;
	SOC_CHECK(HI_MPI_VI_SetCSCAttr(vin, &pstCSCAttr));
	//printf("hue set:%d\r\n", val);
	return 0;
}


int HI_SDK_ISP_set_advance_lowlight_enable(uint8_t bEnable)
{
	//printf("%s:%d\r\n", __FUNCTION__, bEnable);     // 0   1   2  3 

	ISP_PUB_ATTR_S pstPubAttr;
	
	_isp_attr.lowlight_mode = bEnable;

	if(_isp_attr.ispCfgAttr){
		_isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate = _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO ? true : false;
		_isp_attr.ispCfgAttr->impCfgAttr.src_framerate = _isp_attr.src_framerate ;
		
		//printf("AutoSlowFrameRate:%d\r\n", _isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate);
	}

//	HI_MPI_ISP_GetPubAttr(0,&pstPubAttr);
//	printf("old_FRate = %f\n",pstPubAttr.f32FrameRate);

	if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_ALLDAY ){
		_isp_attr.isp_framerate_status = true;
	//	pstPubAttr.f32FrameRate =  frame_rate/2;	
		_hi_sdk_isp_set_slow_framerate(true);
		
	}else if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_NIGHT && _isp_attr.gpio_status_old == ISP_GPIO_NIGHT ){
		_isp_attr.isp_framerate_status = true;
	//	pstPubAttr.f32FrameRate = frame_rate/2;
		_hi_sdk_isp_set_slow_framerate(true);
	}else if(_isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO &&  _isp_attr.isp_framerate_status == 1){
		_isp_attr.isp_framerate_status = true;
	//	pstPubAttr.f32FrameRate = frame_rate/2;
		_hi_sdk_isp_set_slow_framerate(true);
	}else{
		_isp_attr.isp_framerate_status = false;
	//	pstPubAttr.f32FrameRate = frame_rate;
		_hi_sdk_isp_set_slow_framerate(false);
	}
	
	printf("new_FRate =%f\n",pstPubAttr.f32FrameRate);
	//HI_MPI_ISP_SetPubAttr(0,&pstPubAttr);
	return 0;
}


int HI_SDK_ISP_set_src_framerate(unsigned int framerate)
{
	ISP_DEV IspDev = 0;
	ISP_PUB_ATTR_S stPubAttr;	
	ISP_EXPOSURE_ATTR_S stExpAttr;
	
	SOC_CHECK(HI_MPI_ISP_GetPubAttr(IspDev,&stPubAttr));
	
	if(framerate <= 25){
		stPubAttr.f32FrameRate = 25; 
		_isp_attr.filter_frequency = 50;
	}else{
		stPubAttr.f32FrameRate= 30;
		_isp_attr.filter_frequency = 60;
	}
	switch(_isp_attr.sensor_type){
		case SENSOR_MODEL_OV_OV4689:
			stPubAttr.f32FrameRate = 20;
			break;
		case SENSOR_MODEL_OV5658:
			stPubAttr.f32FrameRate = 14;
			break;
		default:
			break;
	}
	printf("\n\nstPubAttr.f32FrameRate = %f-%d\n\n", stPubAttr.f32FrameRate, framerate);
	
//	_isp_attr.src_framerate = stPubAttr.f32FrameRate;
	
/*	VI_CHN_ATTR_S stChnAttr;
	
	SOC_CHECK(HI_MPI_VI_GetChnAttr(0, &stChnAttr));
	stChnAttr.s32SrcFrameRate = stPubAttr.f32FrameRate;
	stChnAttr.s32DstFrameRate = stPubAttr.f32FrameRate;
	SOC_CHECK(HI_MPI_VI_SetChnAttr(0, &stChnAttr));
*/

//	SOC_CHECK(HI_MPI_ISP_SetPubAttr(IspDev,&stPubAttr));


	SOC_CHECK(HI_MPI_ISP_GetExposureAttr(IspDev,&stExpAttr));
	stExpAttr.stAuto.stAntiflicker.bEnable = HI_TRUE; 
	stExpAttr.stAuto.stAntiflicker.u8Frequency = _isp_attr.filter_frequency;	
	SOC_CHECK(HI_MPI_ISP_SetExposureAttr(IspDev,&stExpAttr));
	
	SOC_CHECK(HI_SDK_ISP_set_advance_lowlight_enable(_isp_attr.lowlight_mode));
	sleep(2);

	return 0;
}

int HI_SDK_ISP_get_sharpen(uint8_t *val)
{	
	
    ISP_DEV IspDev = 0;
    ISP_SHARPEN_ATTR_S SharpenAttr;
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(IspDev,&SharpenAttr));
	
	int iso = isp_get_iso();
	if(iso < 200){
		*val = SharpenAttr.stAuto.au8SharpenUd[0];
	}else if(iso < 400){
		*val = SharpenAttr.stAuto.au8SharpenUd[1];
	}else if(iso < 800){
		*val = SharpenAttr.stAuto.au8SharpenUd[2];
	}else if(iso < 1600){
		*val = SharpenAttr.stAuto.au8SharpenUd[3];
	}else if(iso < 3200){
		*val = SharpenAttr.stAuto.au8SharpenUd[4];
	}else if(iso < 6400){
		*val = SharpenAttr.stAuto.au8SharpenUd[5];
	}else if(iso < 12800){
		*val = SharpenAttr.stAuto.au8SharpenUd[6];
	}else if(iso < 25600){
		*val = SharpenAttr.stAuto.au8SharpenUd[7];
	}else if(iso < 51200){
		*val = SharpenAttr.stAuto.au8SharpenUd[8];
	}else if(iso < 102400){
		*val = SharpenAttr.stAuto.au8SharpenUd[9];
	}else if(iso < 204800){
		*val = SharpenAttr.stAuto.au8SharpenUd[10];
	}else if(iso < 409600){	
	    *val = SharpenAttr.stAuto.au8SharpenUd[11];
	}else if(iso < 819200){
		*val = SharpenAttr.stAuto.au8SharpenUd[12];
	}else if(iso < 1638400){
		*val = SharpenAttr.stAuto.au8SharpenUd[13];
	}else if(iso < 3276800){
		*val = SharpenAttr.stAuto.au8SharpenUd[14];
	}else {
		*val = SharpenAttr.stAuto.au8SharpenUd[15];
	};

	return 0;
}


int HI_SDK_ISP_set_sharpen(uint8_t val, uint8_t bManual)
{
	//printf("%s:%d   val = %d\r\n", __FUNCTION__, bManual,val);
	ISP_SHARPEN_ATTR_S SharpenAttr;	
	SOC_CHECK(HI_MPI_ISP_GetSharpenAttr(0,&SharpenAttr));
	SharpenAttr.bEnable = HI_TRUE;
		if(bManual){
		SharpenAttr.enOpType = OP_TYPE_MANUAL;	
		SharpenAttr.stManual.u8SharpenRGB = val;
	}else{
		
		SharpenAttr.enOpType = OP_TYPE_AUTO;
	}
 	SOC_CHECK(HI_MPI_ISP_SetSharpenAttr(0,&SharpenAttr)); 
	
	return 0;
}

int HI_SDK_ISP_set_scene_mode(uint32_t mode)
{
	//printf("%s:%d\r\n", __FUNCTION__, mode);
    ISP_DRC_ATTR_S pstDRC;
	
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));
	switch(mode){
		default:
		case ISP_SCENE_MODE_AUTO:
			HI_SDK_ISP_sensor_flicker(0xff,0,1);
			pstDRC.bEnable = HI_TRUE;
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0330:
					break;
				case SENSOR_MODEL_SONY_IMX185:
					break;
				case SENSOR_MODEL_SONY_IMX178:
					break;
				case SENSOR_MODEL_OV_OV4689:
					break;
				case SENSOR_MODEL_MN34220:
					break;
				case SENSOR_MODEL_OV5658:
					break;
				case SENSOR_MODEL_IMX326:
					break;
				case SENSOR_MODEL_OS05A:
					break;
			} 
			break;
		case ISP_SCENE_MODE_INDOOR:
			HI_SDK_ISP_sensor_flicker(0xff,0,0);
			pstDRC.bEnable = HI_TRUE;
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0330:
					break;	
				case SENSOR_MODEL_OV_OV4689:
					break;
				case SENSOR_MODEL_SONY_IMX185:
					break;
				case SENSOR_MODEL_SONY_IMX178:
					break;
				case SENSOR_MODEL_MN34220:
					break;
				case SENSOR_MODEL_OV5658:
					break;
				case SENSOR_MODEL_IMX326:
					break;	
				case SENSOR_MODEL_OS05A:
					break;
			}
			break;
		case ISP_SCENE_MODE_OUTDOOR:
			HI_SDK_ISP_sensor_flicker(0xff,0,1);
			pstDRC.bEnable = HI_FALSE;
			switch(_isp_attr.sensor_type){
				default:
				case SENSOR_MODEL_APTINA_AR0330:
					break;
				case SENSOR_MODEL_OV_OV4689:
					break;
				case SENSOR_MODEL_SONY_IMX185:
					break;
				case SENSOR_MODEL_SONY_IMX178:
					break;	
				case SENSOR_MODEL_MN34220:
					break;
				case SENSOR_MODEL_OV5658:
					break;
				case SENSOR_MODEL_IMX326:
					break;
				case SENSOR_MODEL_OS05A:
					break;
			}
			break;
	}
	//_isp_attr.isp_auto_drc_enabled = pstDRC.bEnable;
	//SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));
	return 0;

}

int HI_SDK_ISP_set_WB_mode(uint32_t mode)
{
	ISP_AWB_ATTR_EX_S  pstAWBAttrEx;
	SOC_CHECK(HI_MPI_ISP_GetAWBAttrEx(0,&pstAWBAttrEx));
	switch(mode){
	default:
	case ISP_SCENE_MODE_AUTO:
		pstAWBAttrEx.u8ZoneRadius = 16;
		break;
	case ISP_SCENE_MODE_INDOOR:		
		pstAWBAttrEx.u8ZoneRadius = 8;
		break;
	case ISP_SCENE_MODE_OUTDOOR:
		
		pstAWBAttrEx.u8ZoneRadius = 16;
		break;
	}
    pstAWBAttrEx.stInOrOut.enOpType = OP_TYPE_MANUAL;
    pstAWBAttrEx.stInOrOut.bOutdoorStatus = HI_FALSE;
	SOC_CHECK(HI_MPI_ISP_SetAWBAttrEx(0,&pstAWBAttrEx));

	return 0;
}

int HI_SDK_ISP_set_ircut_control_mode(uint32_t mode)
{
	//printf("%s:%d\r\n", __FUNCTION__, mode);
	_isp_attr.ircut_control_mode = mode;
	return 0;
}

int HI_SDK_ISP_set_ircut_mode(uint32_t mode)
{
	//printf("%s:%d\r\n", __FUNCTION__, mode);
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
	printf("\n\n\n%s enable=%d\n",__FUNCTION__,bEnable);
 	return 0;
	ISP_DRC_ATTR_S pstDRC;
	
	if(_isp_attr.sensor_type ==  SENSOR_MODEL_OV_OV4689 || _isp_attr.sensor_type ==  SENSOR_MODEL_MN34220)
	{			
		HI_SDK_ISP_set_wdr_mode_ini(bEnable);	 //   0   1
	}else {
		SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));
		pstDRC.bEnable = bEnable;
		_isp_attr.isp_auto_drc_enabled = bEnable;			
		SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));
	}
	  return 0;
}

int HI_SDK_ISP_set_WDR_strength(uint8_t val)
{
	//printf("%s:%d\r\n", __FUNCTION__, val);
	return 0;
	HI_SDK_ISP_set_mirror(0, _isp_attr.vpss_mirror);
	HI_SDK_ISP_set_flip(0,_isp_attr.vpss_flip);
	
	ISP_DRC_ATTR_S pstDRC;
	
	uint32_t drc_value;
	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));

	if(val >5){
		val = 5;
	}
	if(val < 1){
		val = 1;
	}
	_isp_attr.wdr.strength = val;
	
	drc_value = isp_get_isp_strength_value(&_isp_attr.wdr);
	
	//pstDRC.bEnable = _isp_attr.isp_auto_drc_enabled;
	pstDRC.enOpType = OP_TYPE_AUTO;
	pstDRC.stAuto.u32Strength = drc_value;

	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));

	return 0;
}



int HI_SDK_ISP_get_denoise_strength(uint8_t *val)
{		
//	VPSS_GRP_PARAM_S vpss_grp_param;
//	SOC_CHECK(HI_MPI_VPSS_GetGrpParam(0, &vpss_grp_param));
//	*val = vpss_grp_param.s32GlobalStrength;
	return 0;
}



int HI_SDK_ISP_set_denoise_strength(uint8_t val)
{
/*	
	    printf("%s\r\n", __FUNCTION__);
	
		uint32_t ret_val = 0;
	
		VPSS_GRP_PARAM_S vpss_grp_param;
		SOC_CHECK(HI_MPI_VPSS_GetGrpParam(0, &vpss_grp_param));
	
		if(val >5){
			val = 5;
		}
		if(val < 1){
			val = 1;
		}
		_isp_attr.denoise3d_GlobalStrength.strength = val;
		
		switch(_isp_attr.gpio_status_old){
			default:
			case ISP_GPIO_DAYLIGHT:
			{
				ret_val = _isp_attr.denoise3d_GlobalStrength.daylight_val + _isp_attr.denoise3d_GlobalStrength.daylight_val*(_isp_attr.denoise3d_GlobalStrength.strength - 3 )/2;
			}
			break;
			case ISP_GPIO_NIGHT:
			{				
				ret_val = _isp_attr.denoise3d_GlobalStrength.night_val + _isp_attr.denoise3d_GlobalStrength.night_val*(_isp_attr.denoise3d_GlobalStrength.strength - 3 )/2;
			}
			break;
		}
		if(ret_val> 1408){
			ret_val = 1408;
		}
		if(ret_val < 0){
			ret_val = 0;
		}
		
		vpss_grp_param.s32GlobalStrength = ret_val; //s32GlobalStrength 
		SOC_CHECK(HI_MPI_VPSS_SetGrpParam(0, &vpss_grp_param));
*/
	return 0;
}

int HI_SDK_ISP_set_advance_anti_fog_enable(uint8_t bEnable)
{
	ISP_DEFOG_ATTR_S pstDefogAttr;
	SOC_CHECK(HI_MPI_ISP_GetDeFogAttr(0,&pstDefogAttr));
	pstDefogAttr.bEnable = bEnable;
	SOC_CHECK(HI_MPI_ISP_SetDeFogAttr(0,&pstDefogAttr));
	return 0;
}


int HI_SDK_ISP_set_advance_gamma_table(uint8_t val)
{

	return 0;
}

int HI_SDK_ISP_set_advance_defect_pixel_enable(uint8_t bEnable)
{

	return 0;
}

int HI_SDK_ISP_get_color_max_value(stSensorColorMaxValue *ret_value)
{
	memcpy(ret_value, &_isp_attr.color_max_value, sizeof(stSensorColorMaxValue));
	return 0;
}


emSENSOR_MODEL HI_SDK_ISP_get_sensor_model(char *sensor_name)
{
	_isp_attr.sensor_get_name(sensor_name);
	return _isp_attr.sensor_type;
}

int HI_SDK_ISP_set_sensor_resolution(uint32_t width, uint32_t height)
{
	if(0 == width || 0 == height){
		//set default resolution
		if(_isp_attr.sensor_get_resolution){
			_isp_attr.sensor_get_resolution(&_isp_attr.sensor_resolution_width, &_isp_attr.sensor_resolution_height);
			printf("%s-%d:%dx%d\n", __FUNCTION__, __LINE__, _isp_attr.sensor_resolution_width, _isp_attr.sensor_resolution_height);
		}
	}
	else{
		_isp_attr.sensor_resolution_width = width;
		_isp_attr.sensor_resolution_height = height;
	}
	return 0;
}

int HI_SDK_ISP_get_sensor_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
	*ret_width = _isp_attr.sensor_resolution_width;
	*ret_height = _isp_attr.sensor_resolution_height;
	return 0;
}

static bool IS_FILE_EXIST(const char *filePath)
{
	return (-1 != access(filePath, F_OK));
}


int HI_SDK_ISP_get_sensor_defect_pixel_table(void )
{	
	char dstFilePath[128] = "/media/conf/defect_pixel_table";
	ISP_DP_STATIC_CALIBRATE_S stDPCalibrate;

	FILE *dstFID = NULL;
	int ret;

	if(!IS_FILE_EXIST(dstFilePath)){
		SOC_CHECK(HI_MPI_ISP_GetDPCalibrate(0,&stDPCalibrate));
		printf("stDPCalibrate.bEnable = %d \n",stDPCalibrate.bEnable);
		/*
		printf("bEnableDetect = %d\n",stDPCalibrate.bEnableDetect);	
		printf("enStaticDPType = %d \n",stDPCalibrate.enStaticDPType);
		printf("u16Count =%d \n",stDPCalibrate.u16Count);
		printf("u16CountMax = %d \n",stDPCalibrate.u16CountMax);
		printf("u16CountMin = %d \n",stDPCalibrate.u16CountMin);
		printf("u16TimeLimit = %d \n",stDPCalibrate.u16TimeLimit);
		printf("u8FinishThresh = %d \n",stDPCalibrate.u8FinishThresh);
		printf("u8StartThresh = %d \n",stDPCalibrate.u8StartThresh);
		*/
		
		printf("enStatus = %d\n",stDPCalibrate.enStatus);
		stDPCalibrate.bEnable = HI_TRUE;
		stDPCalibrate.bEnableDetect = HI_TRUE;
		stDPCalibrate.enStaticDPType = ISP_STATIC_DP_DARK;
		if(stDPCalibrate.enStatus == ISP_STATE_INIT){	
			SOC_CHECK(HI_MPI_ISP_SetDPCalibrate(0,&stDPCalibrate));
			while(stDPCalibrate.enStatus == ISP_STATE_INIT){				
				SOC_CHECK(HI_MPI_ISP_GetDPCalibrate(0,&stDPCalibrate));
			}
			printf("enStatus = %d\n",stDPCalibrate.enStatus);					
			printf("u16Count =%d \n",stDPCalibrate.u16Count);
			
		}
		
		dstFID = fopen(dstFilePath, "w+b");
		if(dstFID != NULL){		
			ret = fwrite(stDPCalibrate.u32Table, 1, stDPCalibrate.u16Count*sizeof(HI_U32), dstFID);
			printf("DP table total size =%d \n ",ret);
			if(ret <0 ){
				printf("Write ERRO!");
				return -1;
			}
		
			fclose(dstFID);
			printf("write OK\n");
			return 0;
		}
	}
	return -1;
	
}

int HI_SDK_ISP_set_sensor_defect_pixel_table(void )
{ 
	char srcFilePath[128] = "/media/conf/defect_pixel_table";	
	ISP_DP_ATTR_S stDPAttr;
	FILE *srcFID = NULL;
	int count = 1;
	
	SOC_CHECK(HI_MPI_ISP_GetDPAttr(0,&stDPAttr));
	srcFID = fopen(srcFilePath ,"rb");
	if(srcFID != NULL){
		fread(stDPAttr.stStaticAttr.au32DarkTable,1, sizeof(stDPAttr.stStaticAttr.au32DarkTable), srcFID);
		fclose(srcFID);
		while(stDPAttr.stStaticAttr.au32DarkTable[count - 1] != 0){
			count = count +1;
			}
		count = count -1;

	/*	if(ret/4 > 4096){			
			stDPAttr.stStaticAttr.u16DarkCount = 4096;
		}else{			
			stDPAttr.stStaticAttr.u16DarkCount = ret/4;
		}		
	*/

		printf("DarkTable count = %d \n",count);
			
		stDPAttr.stStaticAttr.bEnable = HI_TRUE; 
		stDPAttr.stStaticAttr.u16DarkCount = count;

		SOC_CHECK(HI_MPI_ISP_SetDPAttr(0,&stDPAttr));

		return 0;
	}

	return -1;

	
}

	 

static void do_isp_common(uint32_t iso)
{
	HI_ISP_cfg_set_imp_single(_isp_attr.gpio_status_old, _isp_attr.ispCfgAttr);
}


static void do_isp_sensor(uint32_t iso)
{
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0330:
		case SENSOR_MODEL_OV_OV4689:
		case SENSOR_MODEL_SONY_IMX178:
		case SENSOR_MODEL_IMX326:
		case SENSOR_MODEL_SONY_IMX185:	
		case SENSOR_MODEL_MN34220:
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
	do_isp_sensor(ret_gain);
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

/////////////H265   &&  H264
static int _hi_sdk_isp_set_slow_framerate(uint8_t bValue)
{
	uint8_t Value = bValue ? 0x1:0x0;
	VENC_CHN_ATTR_S vencChannelAttr;
	ISP_PUB_ATTR_S stPubAttr;	
	int actual_fps;
	int ret = 0;
	int i = 0;
	
	_isp_attr.isp_framerate_status = Value;
	SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));
	printf("old_FRate = %f\n",stPubAttr.f32FrameRate);

	for(i = 0; i <= 1; i++){
		ret = HI_MPI_VENC_GetChnAttr(i,&vencChannelAttr);	

		if(ret != 0){
			return -1;
		}

		actual_fps = _isp_attr.src_framerate/(1 << Value);
		if(vencChannelAttr.stVeAttr.enType == PT_H264){

			switch(vencChannelAttr.stRcAttr.enRcMode){
				default:
				case VENC_RC_MODE_H264VBR:
					if(bValue == true){//slow framerate			
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =  actual_fps;
						}	

					}else{//actual framerate		
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate =  actual_fps;
						}
					}
					break;
				
				case VENC_RC_MODE_H264CBR:
					if(bValue == true){//slow framerate
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =  actual_fps;
						}		
						
					}else{//actual framerate	
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate =  actual_fps;
						}
					}
					break;		
				}
			



		}else if(vencChannelAttr.stVeAttr.enType ==  PT_H265){
			switch(vencChannelAttr.stRcAttr.enRcMode){
				default:
				case VENC_RC_MODE_H265VBR:
					if(bValue == true){//slow framerate 		
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate =	actual_fps;
						}	
			
					}else{//actual framerate		
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Vbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRate =	actual_fps;
						}
					}
					break;
				
				case VENC_RC_MODE_H265CBR:
					if(bValue == true){//slow framerate
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
						}		
						
					}else{//actual framerate	
						stPubAttr.f32FrameRate = actual_fps;
						if(actual_fps < vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate){
							stPubAttr.f32FrameRate = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate;
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate  = vencChannelAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate; 
						}else{
							vencChannelAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRate =	actual_fps;
						}
					}
					break;		
				}
			


		}else{
		}

		HI_MPI_VENC_SetChnAttr(i,&vencChannelAttr);
		SOC_CHECK(HI_MPI_ISP_SetPubAttr(0,&stPubAttr));

	}
	
	printf("new_FRate = %f\n",stPubAttr.f32FrameRate);
	
	return 0;

}


int HI_SDK_ISP_set_isp_sensor_value(void)////mode  0:daytime   1:night
{
	ISP_GAMMA_ATTR_S GammaAttr;
	ISP_EXPOSURE_ATTR_S stExpAttr;	
	ISP_COLOR_TONE_ATTR_S stColorTone;	
	ISP_DP_ATTR_S  stDPAttr;

	SOC_CHECK(HI_MPI_ISP_GetGammaAttr(0, &GammaAttr));
	SOC_CHECK(HI_MPI_ISP_GetExposureAttr(0,&stExpAttr));
	SOC_CHECK(HI_MPI_ISP_GetColorToneAttr(0, &stColorTone));	
	SOC_CHECK(HI_MPI_ISP_GetDPAttr(0, &stDPAttr));
	
	GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0330:		
			{
			if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
				stExpAttr.stAuto.stAGainRange.u32Max= 8192; 						
				stExpAttr.stAuto.stDGainRange.u32Max = 16384 ;
				stExpAttr.stAuto.stSysGainRange.u32Max = 131072;
				
				stExpAttr.stAuto.u8MaxHistOffset = 0;
				

				stColorTone.u16RedCastGain= 245;
				stColorTone.u16GreenCastGain= 256;
				stColorTone.u16BlueCastGain= 275;
				
				stDPAttr.stDynamicAttr.u16Slope= 200;
								
				
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
		//		stExpAttr.stAuto.u8MaxHistOffset = isp_get_isp_strength_value(&_isp_attr.aeHistOffset);				
				

			}else{//night
				
				stExpAttr.stAuto.stAGainRange.u32Max= 3994; 						
				stExpAttr.stAuto.stDGainRange.u32Max = 8192 ;
				stExpAttr.stAuto.stSysGainRange.u32Max = 65536;
				
				stDPAttr.stDynamicAttr.u16Slope= 200;
				
				stExpAttr.stAuto.u8MaxHistOffset = 0;
								
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
			//	stExpAttr.stAuto.u8MaxHistOffset = isp_get_isp_strength_value(&_isp_attr.aeHistOffset);
				
				}
			}
	    break;

	    case SENSOR_MODEL_SONY_IMX178:
			{
				if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
					stExpAttr.stAuto.stAGainRange.u32Max = 16832;
					stExpAttr.stAuto.stDGainRange.u32Max = 8192;
					stExpAttr.stAuto.stISPDGainRange.u32Max = 4096;
					stExpAttr.stAuto.stSysGainRange.u32Max = 262144; 
							
					stColorTone.u16RedCastGain= 210;
					stColorTone.u16GreenCastGain= 216;
					stColorTone.u16BlueCastGain= 232;
					
					stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);

				}else{//night		
					stExpAttr.stAuto.stAGainRange.u32Max = 16832;
					stExpAttr.stAuto.stDGainRange.u32Max = 8192;
					stExpAttr.stAuto.stISPDGainRange.u32Max = 4096;
					stExpAttr.stAuto.stSysGainRange.u32Max = 262144; 
					
					stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
			
				}
			}
			break;
		case SENSOR_MODEL_SONY_IMX185:		
			{
			if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
			


			}else{//night


			

				}
			}
	    break;

		case SENSOR_MODEL_OV_OV4689:		
			{
				stExpAttr.stAuto.u8Speed = 48;
				stExpAttr.stAuto.u8Tolerance = 4;	
			if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
				if(_isp_attr.isp_wdr_mode ==  WDR_MODE_NONE){
					GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Table, gs_Gamma[7], sizeof(gs_Gamma[7]));
				}else{
					GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
					memcpy(GammaAttr.u16Table, gs_Gamma[9], sizeof(gs_Gamma[9]));
				}
			
				stExpAttr.stAuto.stSysGainRange.u32Max = 172032;//131072;
				stExpAttr.stAuto.stAGainRange.u32Max = 16832;
				stExpAttr.stAuto.stISPDGainRange.u32Max = 15360;
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
			//	stExpAttr.stAuto.u8MaxHistOffset = isp_get_isp_strength_value(&_isp_attr.aeHistOffset);
				stExpAttr.stAuto.u8MaxHistOffset = 32;
				stExpAttr.stAuto.u16HistRatioSlope = 128;
				 		
				
				stColorTone.u16RedCastGain= 243;
				stColorTone.u16GreenCastGain= 248;
				stColorTone.u16BlueCastGain= 255;
			
				stDPAttr.stDynamicAttr.u16Slope= 1200;
				stDPAttr.stDynamicAttr.u16Thresh = 32;				

			}else{//night


				GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Table, gs_Gamma[12], sizeof(gs_Gamma[12]));
				
				stExpAttr.stAuto.stSysGainRange.u32Max = 172032;//131072;
				stExpAttr.stAuto.stAGainRange.u32Max = 16832 ;				
				stExpAttr.stAuto.stISPDGainRange.u32Max = 8192;
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
			//	stExpAttr.stAuto.u8MaxHistOffset = isp_get_isp_strength_value(&_isp_attr.aeHistOffset);
				
				stExpAttr.stAuto.u8MaxHistOffset = 32;			
				stExpAttr.stAuto.u16HistRatioSlope = 256;
				

				stDPAttr.stDynamicAttr.u16Slope= 1200;	
				stDPAttr.stDynamicAttr.u16Thresh = 32;
				
				}
			}
	    break;
		case SENSOR_MODEL_MN34220:		
			{
			if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
			
//				stVpssParam.s32GlobalStrength = isp_get_denoise3d_global_strength_value(&_isp_attr.denoise3d_GlobalStrength);
				stDPAttr.stDynamicAttr.u16Slope= 200;
			
			}else{//night

//				stVpssParam.s32GlobalStrength = isp_get_denoise3d_global_strength_value(&_isp_attr.denoise3d_GlobalStrength);
				stDPAttr.stDynamicAttr.u16Slope= 200;

				}
			}
	    break;
		case SENSOR_MODEL_OV5658:
			{
			ISP_GAMMA_ATTR_S GammaAttr;
	 		SOC_CHECK(HI_MPI_ISP_GetGammaAttr(0, &GammaAttr));
			
			if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
			
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDPAttr.stDynamicAttr.u16Slope= 1200;
				stDPAttr.stDynamicAttr.u16Thresh = 32;		
				stExpAttr.stAuto.u8MaxHistOffset = 0;
				stExpAttr.stAuto.stDGainRange.u32Max = 4096;
				stExpAttr.stAuto.stSysGainRange.u32Max = 204800; 

				GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Table, gs_Gamma[10], sizeof(gs_Gamma[10]));
			
			}else{//night
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDPAttr.stDynamicAttr.u16Slope= 1200;
				stDPAttr.stDynamicAttr.u16Thresh = 32;		
				stExpAttr.stAuto.u8MaxHistOffset = 0;
				stExpAttr.stAuto.stDGainRange.u32Max = 2048;
				stExpAttr.stAuto.stSysGainRange.u32Max = 131072; 
				GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Table, gs_Gamma[11], sizeof(gs_Gamma[11]));
			}

				
		}
		break;
		case SENSOR_MODEL_IMX326:
		{
			ISP_GAMMA_ATTR_S GammaAttr;
	 		SOC_CHECK(HI_MPI_ISP_GetGammaAttr(0, &GammaAttr));
			
			if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
			
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDPAttr.stDynamicAttr.u16Slope= 1200;
				stDPAttr.stDynamicAttr.u16Thresh = 32;		
				stExpAttr.stAuto.u8MaxHistOffset = 0;
				stExpAttr.stAuto.stDGainRange.u32Max = 4096;
				stExpAttr.stAuto.stSysGainRange.u32Max = 204800; 

				GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Table, gs_Gamma[14], sizeof(gs_Gamma[14]));
			
			}else{//night
				stExpAttr.stAuto.u8Compensation = isp_get_isp_strength_value(&_isp_attr.aeCompensition);
				stDPAttr.stDynamicAttr.u16Slope= 1200;
				stDPAttr.stDynamicAttr.u16Thresh = 32;		
				stExpAttr.stAuto.u8MaxHistOffset = 0;
				stExpAttr.stAuto.stDGainRange.u32Max = 2048;
				stExpAttr.stAuto.stSysGainRange.u32Max = 131072; 
				GammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
				memcpy(GammaAttr.u16Table, gs_Gamma[15], sizeof(gs_Gamma[15]));
			}
		}
		break;
    	case SENSOR_MODEL_OS05A:
		{
    		if(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT){//daytime
                stColorTone.u16RedCastGain= 249;
    			stColorTone.u16GreenCastGain= 257;
    			stColorTone.u16BlueCastGain= 260;

    		}else{//night

    			}
		}
	    break;
	}

	
	SOC_CHECK(HI_MPI_ISP_SetExposureAttr(0,&stExpAttr));	
	SOC_CHECK(HI_MPI_ISP_SetGammaAttr(0, &GammaAttr));
	SOC_CHECK(HI_MPI_ISP_SetColorToneAttr(0, &stColorTone));		
	SOC_CHECK(HI_MPI_ISP_SetDPAttr(0, &stDPAttr));
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
		_isp_attr.ispCfgAttr->impCfgAttr.AutoSlowFrameRate = _isp_attr.lowlight_mode == ISP_LOWLIGHT_MODE_AUTO ? true : false;	
		_isp_attr.ispCfgAttr->impCfgAttr.flick_frequency =  _isp_attr.filter_frequency;		
		return HI_ISP_cfg_set_all(_isp_attr.gpio_status_old == ISP_GPIO_DAYLIGHT ? 0 : 1, 1, _isp_attr.ispCfgAttr);
	}
}


int HI_SDK_ISP_get_wdr_mode(uint8_t *bEnable)
{
	ISP_WDR_MODE_S stWdrMode;
	ISP_DRC_ATTR_S pstDRC;
			    
	if(_isp_attr.sensor_type == SENSOR_MODEL_OV_OV4689 
		|| _isp_attr.sensor_type == SENSOR_MODEL_MN34220)
	{		
		SOC_CHECK(HI_MPI_ISP_GetWDRMode(HI3518A_VIN_DEV, &stWdrMode));
		if (stWdrMode.enWDRMode)  //wdr mode
		{	
			*bEnable = 1;
		}else{
			*bEnable = 0;
		}
	}else{	
		SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));		
		*bEnable = pstDRC.bEnable;
	}
	
	return 0;

}

int hi_isp_api_set_ircut_switch_to_day_array(float * night_to_day_array)
{
	return 0;
}

static int float2int(float fVal)
{
    float retVal = 0.0f;

    retVal = fVal + ((fVal >= 0.0f) ? (0.50f) : (-0.50f));

    return (int)retVal;

}

int HI_SDK_ISP_get_cur_fps()
{
    float vi_fps;
    ISP_EXP_INFO_S stExpInfo;
    ISP_PUB_ATTR_S stPubAttr;
    VENC_CHN_ATTR_S venc_ch_attr;
    int i_cur_fps;
    float f_cur_fps;
    float val;
    float dstFrmRate;
    float srcFrmRate;
    SOC_CHECK(HI_MPI_ISP_QueryExposureInfo(0, &stExpInfo));
    SOC_CHECK(HI_MPI_VENC_GetChnAttr(0, &venc_ch_attr));
    switch(venc_ch_attr.stRcAttr.enRcMode) {
        default:
        case VENC_RC_MODE_H264VBR:
        {
            dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate;
            srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate;
            break;
        }
        case VENC_RC_MODE_H264CBR:
        {
            dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
            srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate;
            break;
        }
        case VENC_RC_MODE_H264ABR:
        {
           dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264Abr.fr32DstFrmRate;
           srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264Abr.u32SrcFrmRate;
            break;
        }
        case VENC_RC_MODE_H264FIXQP:
        {
            dstFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264FixQp.fr32DstFrmRate;
            srcFrmRate = (float)venc_ch_attr.stRcAttr.stAttrH264FixQp.u32SrcFrmRate;
            break;
        }
    }

    /* stExpInfo.u32Fps / 100 * (dstFrmRate / srcFrmRate) */
    val = dstFrmRate / srcFrmRate;
    vi_fps = stExpInfo.u32Fps / 100;
    if(srcFrmRate <= vi_fps) {
        f_cur_fps = srcFrmRate * val;
    }
    else {
        f_cur_fps = vi_fps * val;
    }
    i_cur_fps = float2int(f_cur_fps);
    return i_cur_fps;

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
		.GET_CUR_FPS = hi_isp_api_get_cur_fps,
	},
	.sensor_type = SENSOR_MODEL_APTINA_AR0330,
	.gpio_status_old = ISP_GPIO_DAYLIGHT,// ISP_GPIO_DAYLIGHT;//daytime
	.color_max_value = {
		.HueMax = 100,
		.SaturationMax = 100,
		.ContrastMax = 100,
		.BrightnessMax = 100,
	},
	.ircut_auto_switch_enable = HI_TRUE,// HI_TRUE;
	.lowlight_mode = 0,
	.isp_auto_drc_enabled = HI_FALSE,
	.AfAttr.param = NULL,
	.AfAttr.af_callback = NULL,
	.isp_framerate_status = HI_FALSE,
	.filter_frequency = 50,
};

/*static HI_S32 pfunction_af_init(HI_S32 s32Handle, const ISP_AF_PARAM_S *pstAfParam)
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
	printf("u32FrameCnt = %d\r\n"
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
		pstAfResult->stStatAttr.u8NpOffset);
	int ret = 0;
	if(NULL != _isp_attr.AfAttr.af_callback){
		//printf("sock=%d\r\n", *_isp_attr.AfAttr.param);
		ret = _isp_attr.AfAttr.af_callback((int)pstAfInfo->pstStatistics->u16FocusMetrics, _isp_attr.AfAttr.param);
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
*/
/*
static HI_S32 pfunctin_af_ctrl(HI_S32 s32Handle, HI_U32 u32Cmd, HI_VOID *pValue)
{
	return 0;
}

static HI_S32 pfunction_af_exit(HI_S32 s32Handle)
{
	return 0;
}
*/

/*
static int isp_af_init_function(	ISP_AF_REGISTER_S *pAfRegister)
{
	memset(&pAfRegister->stAfExpFunc, 0, sizeof(ISP_AF_EXP_FUNC_S));
	pAfRegister->stAfExpFunc.pfn_af_init = pfunction_af_init;
	pAfRegister->stAfExpFunc.pfn_af_ctrl= pfunctin_af_ctrl;
	pAfRegister->stAfExpFunc.pfn_af_run = pfunction_af_run;
	pAfRegister->stAfExpFunc.pfn_af_exit = pfunction_af_exit;
}*/

static HI_VOID* ISP_Thread_Run(HI_VOID *param)
{
 //   ISP_DEV IspDev = 0;
    SOC_CHECK(HI_MPI_ISP_Run(HI3518A_VIN_DEV));

    return HI_NULL;
}
HI_S32  HI_SDK_ISP_vi_start_dev(VI_DEV ViDev)
{	
	ISP_WDR_MODE_S stWdrMode;
	VI_DEV_ATTR_S vi_dev_attr_720p_30fps;
	
	stWdrMode.enWDRMode = _isp_attr.isp_wdr_mode;
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_OV_OV4689:
			SET_VI_DEV_ATTR_OV4689(vi_dev_attr_720p_30fps);         //TO DO
			break;
		case SENSOR_MODEL_MN34220:			
			SET_VI_DEV_ATTR_MN34220(vi_dev_attr_720p_30fps);// TO DO
			break;	
	}	
	SOC_CHECK(HI_MPI_ISP_SetWDRMode(0, &stWdrMode));
	SOC_CHECK(HI_MPI_VI_SetDevAttr(ViDev, &vi_dev_attr_720p_30fps));	
	SOC_CHECK(HI_MPI_ISP_GetWDRMode(0, &stWdrMode));

	if (stWdrMode.enWDRMode)  //wdr mode
	{
		VI_WDR_ATTR_S stWdrAttr;
	
		stWdrAttr.enWDRMode = stWdrMode.enWDRMode;
		stWdrAttr.bCompress = HI_FALSE;
	
		SOC_CHECK(HI_MPI_VI_SetWDRAttr(ViDev, &stWdrAttr));
	}
	SOC_CHECK(HI_MPI_VI_EnableDev(ViDev));
	return 0;
}

 int HI_SDK_ISP_set_wdr_mode_ini(uint8_t mode)
{	
	ISP_DEV IspDev = 0;	
	VI_CHN ViChn = 0;
	VI_DEV ViDev = HI3518A_VIN_DEV;  //0
	ISP_DRC_ATTR_S pstDRC;
	VI_DEV_ATTR_S stViDevAttr;
	ISP_WDR_MODE_S stWDRMode;
    ISP_INNER_STATE_INFO_S stInnerStateInfo = {0};

	SOC_CHECK(HI_MPI_ISP_GetDRCAttr(0,&pstDRC));
	pstDRC.bEnable = mode;
	_isp_attr.isp_auto_drc_enabled = mode;			
	SOC_CHECK(HI_MPI_ISP_SetDRCAttr(0,&pstDRC));
	
	/* switch to linear mode */
	SOC_CHECK(HI_MPI_VI_GetDevAttr(ViDev, &stViDevAttr));
	if(mode == 0){
		printf("******switch to linear mode******\n");
		SOC_CHECK(HI_MPI_ISP_SetFMWState(IspDev, ISP_FMW_STATE_FREEZE));
		SOC_CHECK(HI_MPI_VI_DisableChn(ViChn));
		SOC_CHECK(HI_MPI_VI_DisableDev(ViDev));


		_isp_attr.isp_wdr_mode = WDR_MODE_NONE;
		stWDRMode.enWDRMode = WDR_MODE_NONE;

		SOC_CHECK(hi_isp_SetMipiAttr());
		SOC_CHECK(HI_MPI_ISP_SetFMWState(IspDev, ISP_FMW_STATE_RUN));
		SOC_CHECK(HI_MPI_ISP_SetWDRMode(IspDev, &stWDRMode));

	    while (1)
	    {
	        HI_MPI_ISP_QueryInnerStateInfo(IspDev, &stInnerStateInfo);			
	        if (HI_TRUE == stInnerStateInfo.bWDRSwitchFinish)
	        {
	            printf("wdr switch finish!\n");
	            break;
	        }
	        usleep(10000);
	    }
		
		SOC_CHECK(HI_SDK_ISP_vi_start_dev(ViDev));
		SOC_CHECK(HI_MPI_VI_EnableChn(ViChn));

	}
	else if (mode== 1){/* switch to 2to1 line WDR mode */
		
		SOC_CHECK(HI_MPI_ISP_SetFMWState(IspDev, ISP_FMW_STATE_FREEZE));
		SOC_CHECK(HI_MPI_VI_DisableChn(ViChn));
		SOC_CHECK(HI_MPI_VI_DisableDev(ViDev));

		_isp_attr.isp_wdr_mode = WDR_MODE_2To1_LINE;		
		stWDRMode.enWDRMode = WDR_MODE_2To1_LINE;

		SOC_CHECK(hi_isp_SetMipiAttr());
		SOC_CHECK(HI_MPI_ISP_SetFMWState(IspDev, ISP_FMW_STATE_RUN));
		SOC_CHECK(HI_MPI_ISP_SetWDRMode(IspDev, &stWDRMode));

	    while (1)
	    {
	        HI_MPI_ISP_QueryInnerStateInfo(IspDev, &stInnerStateInfo);
	        if (HI_TRUE == stInnerStateInfo.bWDRSwitchFinish)
	        {
	            printf("wdr switch finish!\n");
	            break;
	        }
	        usleep(10000);
	    }		
		SOC_CHECK(HI_SDK_ISP_vi_start_dev(ViDev));
		SOC_CHECK(HI_MPI_VI_EnableChn(ViChn));
	}  

	usleep(100000);
	HI_SDK_ISP_set_isp_sensor_value();
	return 0;

}

static HI_S32 pfunction_af_init(HI_S32 s32Handle, const ISP_AF_PARAM_S *pstAfParam)
{
	if (HI_NULL == pstAfParam)
    {
        printf("null pointer when af init default value!\n");
        return -1;
    }

	return 0;
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

//图像权重
static int HI16EV1_AFWeight[8][8] = {
	{1,1,1,1,1,1,1,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,2,2,2,2,2,2,1,},
	{1,1,1,1,1,1,1,1,},
};

#define AF_BLEND_SHIFT 		6
#define AF_ALPHA 			29 // 0.45
#define AF_BELTA				54 // 0.85


static HI_S32 pfunction_af_run(HI_S32 s32Handle, const ISP_AF_INFO_S *pstAfInfo, ISP_AF_RESULT_S *pstAfResult, HI_S32 s32Rsv)
{
#if 1
	int ret = 0, i=0, j=0;
	if(NULL != _isp_attr.AfAttr.af_callback){
		//printf("sock = %d\r\n", *_isp_attr.AfAttr.param);

		ISP_STATISTICS_CFG_S stIspStaticsCfg;
		HI_U32 u32Fv1_n, u32Fv1, u32Fv2, u32Fv3, u32Fv2_n;
		static ISP_FOCUS_STATISTICS_CFG_S stFocusCfg;

		HI_MPI_ISP_GetStatisticsConfig(0, &stIspStaticsCfg);
		memcpy(&stFocusCfg, &stIspStaticsCfg.stFocusCfg, sizeof(ISP_FOCUS_STATISTICS_CFG_S));

		HI_U32 u32SumFv1 = 0;
		HI_U32 u32SumFv2 = 0;
		HI_U32 u32WgtSum = 0;

		//get data
		for(i=0; i<stFocusCfg.stConfig.u16Vwnd; i++)
		{
			for(j=0; j<stFocusCfg.stConfig.u16Hwnd; j++)
			{
				HI_U32 u32H1 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16h1;
				HI_U32 u32V1 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16v1;

				HI_U32 u32H2 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16h2;
				HI_U32 u32V2 = pstAfInfo->stAfStat->stZoneMetrics[i][j].u16v2;

				u32Fv1_n = (u32H1 * AF_ALPHA + u32V1 * ((1<<AF_BLEND_SHIFT) - AF_ALPHA)) >> AF_BLEND_SHIFT;
				u32Fv2_n = (u32H2 * AF_BELTA + u32V2 * ((1<<AF_BLEND_SHIFT) - AF_BELTA)) >> AF_BLEND_SHIFT;

				u32SumFv1 += HI16EV1_AFWeight[i][j] * u32Fv1_n;
				u32SumFv2 += HI16EV1_AFWeight[i][j] * u32Fv2_n;
				u32WgtSum += HI16EV1_AFWeight[i][j];
				//printf("weight[%d][%d]=%d--%d\n", i,j, HI16EV1_AFWeight[i][j], u32Fv1_n);
			}
		}

		u32Fv1 = u32SumFv1;
		u32Fv2 = u32SumFv2;
		u32Fv3 = u32SumFv1 / u32WgtSum;

		ret = _isp_attr.AfAttr.af_callback((int)u32Fv1,(int)u32Fv2, (int)u32Fv3, _isp_attr.AfAttr.param);

		if(ret< 0){
			if(_isp_attr.AfAttr.param){
				free(_isp_attr.AfAttr.param);
				_isp_attr.AfAttr.param = NULL;
				_isp_attr.AfAttr.af_callback = NULL;
			}
		}
	}
	//printf("u16FocusMetrics = %d - %d\r\n", pstAfInfo->pstAfStat->u16FocusMetrics, pstAfResult->stStatAttr.bChange);
#endif
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

static int isp_af_init_function(ISP_AF_REGISTER_S *pAfRegister)
{
	memset(&pAfRegister->stAfExpFunc, 0, sizeof(ISP_AF_EXP_FUNC_S));
	pAfRegister->stAfExpFunc.pfn_af_init = pfunction_af_init;
	pAfRegister->stAfExpFunc.pfn_af_ctrl= pfunctin_af_ctrl;
	pAfRegister->stAfExpFunc.pfn_af_run = pfunction_af_run;
	pAfRegister->stAfExpFunc.pfn_af_exit = pfunction_af_exit;
	return 0;
}

static int af_register_callback(ISP_AF_REGISTER_S *pAfRegister)
{
	HI_S32 s32Ret = 0;
	ALG_LIB_S stLib;
	
	stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_ISP_AFLibRegCallBack(0, &stLib, pAfRegister);
    if (HI_SUCCESS != s32Ret)
    {
        printf("sensor register callback function to AF lib failed!\n");
        return s32Ret;
    }
	return s32Ret;
}

static void mpp_vb_conf_clear(VB_CONF_S* p_vb_conf)
{
	p_vb_conf->u32MaxPoolCnt = 0;
}

static int mpp_vb_conf_add_block(VB_CONF_S* p_vb_conf, int block_size, int block_count)
{
	if(p_vb_conf->u32MaxPoolCnt < VB_MAX_COMM_POOLS){
		p_vb_conf->astCommPool[p_vb_conf->u32MaxPoolCnt].u32BlkSize = block_size;
		p_vb_conf->astCommPool[p_vb_conf->u32MaxPoolCnt].u32BlkCnt = block_count;
		++p_vb_conf->u32MaxPoolCnt;
		return 0;
	}
	return -1;
}

static void hi_mpp_destroy()
{
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();
}

static void hi_mpp_init()
{
	MPP_SYS_CONF_S sys_conf;
	VB_CONF_S vb_conf;

	hi_mpp_destroy();

	memset(&vb_conf, 0, sizeof(vb_conf));
	mpp_vb_conf_clear(&vb_conf);
	
	mpp_vb_conf_add_block(&vb_conf, 2592 * 2000 * 3/2, 7); //for imx178 16D
	mpp_vb_conf_add_block(&vb_conf, 720 * 640 * 3/2, 4);
	mpp_vb_conf_add_block(&vb_conf, 320 * 250 * 2, 5);

	SOC_CHECK(HI_MPI_VB_SetConf(&vb_conf));
	SOC_CHECK(HI_MPI_VB_Init());

	memset(&sys_conf, 0, sizeof(sys_conf));
    sys_conf.u32AlignWidth = 64;
    SOC_CHECK(HI_MPI_SYS_SetConf(&sys_conf));
    SOC_CHECK(HI_MPI_SYS_Init());
}


int HI_SDK_ISP_init(lpSensorApi*api, lpBSPApi *bsp_api)
{
//	pthread_t isp_tid = 0;
	VI_DEV_ATTR_S vi_dev_attr_720p_30fps;

	ISP_PUB_ATTR_S stPubAttr;
	VI_CHN_ATTR_S stChnAttr;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_MODE_S stVpssChnMode;
	ISP_WDR_MODE_S stWdrMode;
	ISP_AF_REGISTER_S stAfRegister;
	memcpy(&_isp_attr.bsp_api, bsp_api, sizeof(stBSPApi));

	//set mipi stable
	//system("himm 0x20681048 0xff0");
	//system("himm 0x20681214 0x1100");
	sdk_sys->write_reg(0x20681048, 0xff0);
	sdk_sys->write_reg(0x20681214, 0x1100);
	
	HI_SDK_ISP_sensor_check();
	isp_af_init_function(&stAfRegister);

	memset(&stVpssGrpAttr, 0, sizeof(stVpssGrpAttr));
	//init sensor
	//printf("sensor type:%d\r\n", _isp_attr.sensor_type);
	af_register_callback(&stAfRegister);
	
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0330:
			APTINA_AR0330_init(NULL, NULL);			
			SET_VI_DEV_ATTR_AR0330(vi_dev_attr_720p_30fps);
			_isp_attr.sensor_get_resolution = AR0330_get_resolution;
			_isp_attr.sensor_get_name = AR0330_get_sensor_name;

			break;
		case SENSOR_MODEL_OV_OV4689:
			OV_OV4689_init(NULL, NULL);			
			SET_VI_DEV_ATTR_OV4689(vi_dev_attr_720p_30fps);         //TO DO
			_isp_attr.sensor_get_resolution = OV4689_get_resolution;
			_isp_attr.sensor_get_name = OV4689_get_sensor_name;
			break;

		case SENSOR_MODEL_SONY_IMX178:
			SONY_IMX178_init(NULL, NULL);			
			SET_VI_DEV_ATTR_IMX178(vi_dev_attr_720p_30fps);
			_isp_attr.sensor_get_resolution = IMX178_get_resolution;
			_isp_attr.sensor_get_name = IMX178_get_sensor_name;
			break;
		case SENSOR_MODEL_SONY_IMX185:
			SONY_IMX185_init(NULL, NULL);			
			SET_VI_DEV_ATTR_IMX185(vi_dev_attr_720p_30fps);// TO DO
			_isp_attr.sensor_get_resolution = IMX185_get_resolution;
			_isp_attr.sensor_get_name = IMX185_get_sensor_name;
			break;
		case SENSOR_MODEL_MN34220:			
			MN34220_init(NULL, NULL);
			SET_VI_DEV_ATTR_MN34220(vi_dev_attr_720p_30fps);// TO DO
			_isp_attr.sensor_get_resolution = MN34220_get_resolution;
			_isp_attr.sensor_get_name = MN34220_get_sensor_name;
			break;
		case SENSOR_MODEL_OV5658:
			OV5658_init(NULL, NULL);
			SET_VI_DEV_ATTR_OV5658(vi_dev_attr_720p_30fps);
			_isp_attr.sensor_get_resolution = OV5658_get_resolution;
			_isp_attr.sensor_get_name = OV5658_get_sensor_name;
			break;
		case SENSOR_MODEL_IMX326:
			SONY_IMX326_init(NULL, NULL);
			SET_VI_DEV_ATTR_IMX326(vi_dev_attr_720p_30fps);	
			_isp_attr.sensor_get_resolution = IMX326_get_resolution;	
			break;	
		case SENSOR_MODEL_OS05A:
			OS05A_init(NULL, NULL);
			SET_VI_DEV_ATTR_OS05A(vi_dev_attr_720p_30fps);
			_isp_attr.sensor_get_resolution = OS05A_get_resolution;
			_isp_attr.sensor_get_name = OS05A_get_sensor_name;
			break;
	}

	hi_mpp_init();
	HI_SDK_ISP_set_sensor_resolution(0, 0);
	//return 0;
	switch(_isp_attr.sensor_type){
		default:
		case SENSOR_MODEL_APTINA_AR0330:
		case SENSOR_MODEL_MN34220:
			stPubAttr.enBayer	   = BAYER_GRBG;
			stPubAttr.f32FrameRate			= 25;
	    break;
		case SENSOR_MODEL_OV5658:	
		case SENSOR_MODEL_OV_OV4689:
		case SENSOR_MODEL_OS05A:
			stPubAttr.enBayer               = BAYER_BGGR;
			stPubAttr.f32FrameRate          = 25;	   
			break;
		case SENSOR_MODEL_SONY_IMX178:
		    stPubAttr.enBayer               = BAYER_GBRG;
		    stPubAttr.f32FrameRate          = 25;  
			break;
		case SENSOR_MODEL_IMX326:
		case SENSOR_MODEL_SONY_IMX185:
			stPubAttr.enBayer               = BAYER_RGGB;
		    stPubAttr.f32FrameRate          = 25;   
			break;
		}
    stPubAttr.stWndRect.s32X		= 0;
	stPubAttr.stWndRect.s32Y		= 0;
	stPubAttr.stWndRect.u32Width	= _isp_attr.sensor_resolution_width;
	stPubAttr.stWndRect.u32Height	= _isp_attr.sensor_resolution_height;
	
	stChnAttr.stCapRect.s32X = 0;
	stChnAttr.stCapRect.s32Y = 0;
	stChnAttr.stCapRect.u32Width  = _isp_attr.sensor_resolution_width;
	stChnAttr.stCapRect.u32Height = _isp_attr.sensor_resolution_height;
	stChnAttr.stDestSize.u32Width = _isp_attr.sensor_resolution_width;
	stChnAttr.stDestSize.u32Height = _isp_attr.sensor_resolution_height;
	
	stVpssGrpAttr.u32MaxW = _isp_attr.sensor_resolution_width;
	stVpssGrpAttr.u32MaxH = _isp_attr.sensor_resolution_height;
	
	stVpssChnMode.u32Width		 = _isp_attr.sensor_resolution_width;		
	stVpssChnMode.u32Height 	 = _isp_attr.sensor_resolution_height;
	
	stWdrMode.enWDRMode  = WDR_MODE_NONE;	
	_isp_attr.src_framerate = stPubAttr.f32FrameRate ;
	_isp_attr.isp_wdr_mode = stWdrMode.enWDRMode;

	{
		//isp init
		SOC_CHECK(HI_MPI_ISP_MemInit(HI3518A_VIN_DEV));

	/* 6. isp set WDR mode */
	
	SOC_CHECK(HI_MPI_ISP_SetWDRMode(0, &stWdrMode));

    /* 7. isp set pub attributes */
    /* note : different sensor, different ISP_PUB_ATTR_S define.
              if the sensor you used is different, you can change
              ISP_PUB_ATTR_S definition */
		SOC_CHECK(HI_MPI_ISP_SetPubAttr(HI3518A_VIN_DEV, &stPubAttr));
    	SOC_CHECK(HI_MPI_ISP_Init(HI3518A_VIN_DEV));

		pthread_t pid;
	    if (0 != pthread_create(&pid, 0, (void* (*)(void*))ISP_Thread_Run, NULL))
	    {
	        printf("%s: create isp running thread failed!\n", __FUNCTION__);
	        return HI_FAILURE;
	    }

	}

	{
		SOC_CHECK(HI_MPI_VI_SetDevAttr(HI3518A_VIN_DEV, &vi_dev_attr_720p_30fps));
#if 1
		    SOC_CHECK(HI_MPI_ISP_GetWDRMode(HI3518A_VIN_DEV, &stWdrMode));
		    
		    if (stWdrMode.enWDRMode)  //wdr mode
		    {
		        VI_WDR_ATTR_S stWdrAttr;

		        stWdrAttr.enWDRMode = stWdrMode.enWDRMode;
		        stWdrAttr.bCompress = HI_FALSE;

		        SOC_CHECK(HI_MPI_VI_SetWDRAttr(HI3518A_VIN_DEV, &stWdrAttr));
		    }
#endif

		SOC_CHECK(HI_MPI_VI_EnableDev(HI3518A_VIN_DEV));
	}
	{
	//	VI_CHN_ATTR_S stChnAttr;
	    /* step  5: config & start vicap dev */

	    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
	    stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;   /* sp420 or sp422 */

	    stChnAttr.bMirror = HI_FALSE;
	    stChnAttr.bFlip = HI_FALSE;

	    stChnAttr.s32SrcFrameRate = stPubAttr.f32FrameRate;
	    stChnAttr.s32DstFrameRate = stPubAttr.f32FrameRate;
	    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;

		
		SOC_CHECK(HI_MPI_VI_SetChnAttr(HI3518A_VIN_CHN, &stChnAttr));
		SOC_CHECK(HI_MPI_VI_EnableChn(HI3518A_VIN_CHN));
	}
	{
		VPSS_GRP VpssGrp = 0;		
	    VPSS_CHN VpssChn = 1;
	    VPSS_CHN_ATTR_S stVpssChnAttr;

		stVpssGrpAttr.bIeEn = HI_FALSE;
		stVpssGrpAttr.bNrEn = HI_TRUE;
		stVpssGrpAttr.bHistEn = HI_FALSE;
		stVpssGrpAttr.bDciEn = HI_FALSE;
		stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
		stVpssGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	//	VPSS_GRP_PARAM_S stVpssParam;

		SOC_CHECK(HI_MPI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr));
			
	
		//return 0;
	    /*** set vpss param ***/
	//	SOC_CHECK(HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam));
	//	SOC_CHECK(HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam));	
		SOC_CHECK(HI_MPI_VPSS_StartGrp(VpssGrp));

		MPP_CHN_S stSrcChn;
    	MPP_CHN_S stDestChn;
		stSrcChn.enModId  = HI_ID_VIU;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = 0;
    
        stDestChn.enModId  = HI_ID_VPSS;
        stDestChn.s32DevId = VpssGrp;
        stDestChn.s32ChnId = 0;
		
        SOC_CHECK(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn));

		stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
		stVpssChnMode.bDouble		 = HI_FALSE;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
		memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
		stVpssChnAttr.s32SrcFrameRate = -1;
		stVpssChnAttr.s32DstFrameRate = -1;

//		for(;VpssChn <=3;VpssChn++){	
			SOC_CHECK(HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr));
			SOC_CHECK(HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stVpssChnMode));
			SOC_CHECK(HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn));
			if(0 <= VpssChn && VpssChn < VPSS_MAX_PHY_CHN_NUM-1)
			SOC_CHECK(HI_MPI_VPSS_SetChnCover(VpssGrp, VpssChn, 255));			
//		}

	}

	isp_ircut_gpio_init();
	isp_ircut_switch(0);//default for daylight
	HI_SDK_ISP_set_sensor_defect_pixel_table();
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
	SOC_CHECK(HI_MPI_ISP_Exit(0));

	hi_mpp_destroy();
	return 0;
}

