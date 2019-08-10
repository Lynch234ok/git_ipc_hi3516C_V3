#include "sdk/sdk_debug.h"
#include "sdk/sdk_sys.h"
#include "hi3516a.h"
#include "hi3516a_isp_sensor.h"
#include "hi_i2c.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"
#include <fcntl.h>
#include <unistd.h>

static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x6e, 1, 1))
#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x6e, 1, 1))
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#define SENSOR_GC4603_WIDTH 2560
#define SENSOR_GC4603_HEIGHT 1440
#define GC4603_CHECK_DATA_MSB (0x40)//0XF0
#define GC4603_CHECK_DATA_LSB (0x03)//0XF1

#if !defined(__GC4603_CMOS_H_)
#define __GC4603_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define GC4603_ID 4603


/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/

#ifdef INIFILE_CONFIG_MODE

static AE_SENSOR_DEFAULT_S  g_AeDft[];
static AWB_SENSOR_DEFAULT_S g_AwbDft[];
static ISP_CMOS_DEFAULT_S   g_IspDft[];
static HI_S32 Cmos_LoadINIPara(const HI_CHAR *pcName);
#else

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

#define EXPOSURE_H 0x03
#define EXPOSURE_L 0x04
#define VBLINK_H   0X07
#define VBLINK_L   0X08
#define D_GAIN_H   0xb1
#define D_GAIN_L   0xb2
#define A_GAIN 	   0xb6

#define VMAX_GC4603_4MP30_LINEAR 758
#define SENSOR_4M_30FPS_MODE      (0)

static HI_U8 gu8SensorImageMode = SENSOR_4M_30FPS_MODE;
static HI_U32 gu32FullLinesStd = VMAX_GC4603_4MP30_LINEAR; 
static HI_U32 gu32FullLines = VMAX_GC4603_4MP30_LINEAR;
static HI_U32 gu32PreFullLines = VMAX_GC4603_4MP30_LINEAR;
static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE; 
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static const unsigned int sensor_i2c_addr = 0x6e;
static unsigned int sensor_addr_byte = 1;
static unsigned int sensor_data_byte = 1;

#define FULL_LINES_MAX  (0x7FFF)

#define PATHLEN_MAX 256
#define CMOS_CFG_INI "gc4603_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/gc4603_cfg.ini";

/* AE default parameter and function */
static HI_S32 gc4603_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    pstAeSnsDft->u8AeCompensation = 0x38;
    
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*25/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    //pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32MinIntTime = 2;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.1;
    pstAeSnsDft->u32MaxAgain = 12083;  
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.015625;
    pstAeSnsDft->u32MaxDgain = 1023; //16x ,if set >=1024 ,error
    pstAeSnsDft->u32MinDgain = 64;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;   

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift; 
    
    return 0;
}

/* the function of sensor set fps */
static HI_VOID gc4603_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

	HI_U32 u32Vsync=68;
	if ((f32Fps <= 30) && (f32Fps >= 5))
	{
		gu32FullLinesStd = VMAX_GC4603_4MP30_LINEAR * 25 / f32Fps;  
		gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;
		pstAeSnsDft->f32Fps = f32Fps;
		pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
		pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
		pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
		gu32FullLines = gu32FullLinesStd;

		u32Vsync = gu32FullLinesStd*2 - 1456 -16;

		SENSOR_I2C_WRITE(VBLINK_H,u32Vsync >> 8);
		SENSOR_I2C_WRITE(VBLINK_L,(u32Vsync&0xFF));

	}
	else
	{
        printf("Not support Fps: %f\n", f32Fps);
        return;
	}

    return;
}

static HI_VOID gc4603_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	HI_U32 u32Vsync=68;

    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;
    pstAeSnsDft->u32FullLinesStd = gu32FullLines;
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 2;

    u32Vsync = u32FullLines*2 - 1456 -16;
	SENSOR_I2C_WRITE(VBLINK_H,u32Vsync >> 8);
	SENSOR_I2C_WRITE(VBLINK_L,(u32Vsync&0xFF));
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID gc4603_inttime_update(HI_U32 u32IntTime)
{

	u32IntTime = u32IntTime*2;

	g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 8);
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime  & 0xFF);

	//SENSOR_I2C_WRITE(EXPOSURE_H,u32IntTime >> 8);
	//SENSOR_I2C_WRITE(EXPOSURE_L,(u32IntTime&0xFF));
	return ;
}

static HI_U32 gain_table[8]=
{
     //1024,1434,1946,2816,3994,5683,6820,12083
     1024,1454,2037,2969,4147,5928,8437,12083
};


static HI_VOID gc4603_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if (*pu32AgainLin >= gain_table[7])
    {
         *pu32AgainLin = gain_table[7];
         *pu32AgainDb = 7;
         return ;
    }
    
    for (i = 1; i < 8; i++)
    {
        if (*pu32AgainLin < gain_table[i])
        {
            *pu32AgainLin = gain_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }
    
    return;
}

static HI_VOID gc4603_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    HI_U8 Dgain_zheng,Dgain_xiao;  
	Dgain_zheng = (u32Dgain>>6)&0xF;
	Dgain_xiao = (u32Dgain & 0x3F)<<2;

	if(u32Again == 0){
		SENSOR_I2C_WRITE(0x29,0x15);	
	}
	else{
	
		SENSOR_I2C_WRITE(0x29,0x58);	
	}

	g_stSnsRegsInfo.astI2cData[2].u32Data = u32Again;
	g_stSnsRegsInfo.astI2cData[3].u32Data = Dgain_zheng;
	g_stSnsRegsInfo.astI2cData[4].u32Data = Dgain_xiao;

    //SENSOR_I2C_WRITE(A_GAIN,u32Again);			//max_11.8x
	//SENSOR_I2C_WRITE(D_GAIN_H,Dgain_zheng);	//max_16x
	//SENSOR_I2C_WRITE(D_GAIN_L,Dgain_xiao);

    return;
}

/* Only used in WDR_MODE_2To1_LINE mode */
static HI_VOID gc4603_get_inttime_max(HI_U32 u32Ratio, HI_U32 *pu32IntTimeMax)
{
    
    
    return;
}

HI_S32 gc4603_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = gc4603_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = gc4603_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= gc4603_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = gc4603_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = gc4603_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = gc4603_again_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = gc4603_get_inttime_max;

    return 0;
}


static AWB_CCM_S g_stAwbCcm =
{
   4688,
   {
	  0x1CB,0x80B3,0x8018,
	  0x804D,0x194,0x8047,
	  0x8003,0x80B9,0x1BC
   },
   
	3601,
	{	
  	  0x1AF,0x809E,0x8011,	   
  	  0x8057,0x180,0x8029,	   
  	  0x800E,0x80A9,0x1B7
	
	},
	2392,
	{	
	  0x1FD,0x805B,0x80A2,
	  0x80AB,0x225,0x807A,
	  0x80CE,0x804B,0x219  
	}
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x80,0x80,0x7e,0x72,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
};

static HI_S32 gc4603_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4688;

    pstAwbSnsDft->au16GainOffset[0] = 0x16B;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x198;

    pstAwbSnsDft->as32WbPara[0] = 202;
    pstAwbSnsDft->as32WbPara[1] = -174;
    pstAwbSnsDft->as32WbPara[2] = -229;
    pstAwbSnsDft->as32WbPara[3] = 199210;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -149654;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
	memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));

    return 0;
}


HI_S32 gc4603_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = gc4603_get_awb_default;

    return 0;
}

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
    
    /* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */

    /* sharpen_alt_d */
    {0x3a,0x3a,0x38,0x34,0x30,0x2e,0x28,0x24,0x20,0x1b,0x18,0x14,0x12,0x10,0x10,0x10},
        
    /* sharpen_alt_ud */
    {0x60,0x60,0x58,0x50,0x48,0x40,0x38,0x28,0x20,0x20,0x18,0x12,0x10,0x10,0x10,0x10},
        
    /* snr_thresh Max=0x54 */
    {0x08,0x0a,0x0f,0x12,0x16,0x1a,0x22,0x28,0x2e,0x36,0x3a,0x40,0x40,0x40,0x40,0x40},
        
    /* demosaic_lum_thresh */
    {0x50,0x50,0x4e,0x49,0x45,0x45,0x40,0x3a,0x3a,0x30,0x30,0x2a,0x20,0x20,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x00,0x0a,0x12,0x1a,0x20,0x28,0x30,0x32,0x34,0x36,0x38,0x38,0x38,0x38,0x38,0x38},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37,0x37,0x35,0x35,0x35,0x35,0x35,0x35,0x35},

    /* rgbsharp_strength */
    {0x88,0x86,0x84,0x78,0x6a,0x60,0x58,0x50,0x40,0x30,0x20,0x16,0x12,0x12,0x12,0x12}
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,

    /* nosie_profile_weight_lut */
    {0x0, 0x0, 0x8, 0xF, 0x13, 0x16, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x23, 0x25, 
    0x25, 0x26, 0x27, 0x27, 0x27, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2C, 0x2D, 
    0x2D, 0x2D, 0x2E, 0x2E, 0x2E, 0x2F, 0x2F, 0x2F, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 
    0x32, 0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 
    0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x38,
    0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x3A, 0x3A, 
    0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 
    0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3D, 0x3D}, 


    /* demosaic_weight_lut */
    {0x8, 0xF, 0x13, 0x16, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x23, 0x25, 0x25, 0x26, 
    0x27, 0x27, 0x27, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2C, 0x2D, 0x2D, 0x2D, 
    0x2E, 0x2E, 0x2E, 0x2F, 0x2F, 0x2F, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x32, 0x32, 
    0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 0x35, 0x35, 
    0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x38, 0x38, 0x38, 
    0x38, 0x38, 0x38, 0x38, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x3A, 0x3A, 0x3A, 0x3A, 
    0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3C, 0x3C, 
    0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3D, 0x3D, 0x3D, 0x3D}        
    
};


static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xbb,

    /*aa_slope*/
    0xb8,

    /*va_slope*/
    0xb4,

    /*uu_slope*/
    0xb8,

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xa0,
    
    /*fc_slope*/
    0x8a,

    /*vh_thresh*/
    0x00,

    /*aa_thresh*/
    0x00,

    /*va_thresh*/
    0x00,

    /*uu_thresh*/
    0x08,

    /*sat_thresh*/
    0x00,

    /*ac_thresh*/
    0x1b3
};

static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{   
    /* bvalid */   
    1,   
    
    /*lut_core*/   
    192,  
    
    /*lut_strength*/  
    127, 
    
    /*lut_magnitude*/   
    6      
};

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,

    {  0, 180, 320, 426, 516, 590, 660, 730, 786, 844, 896, 946, 994, 1040, 1090, 1130, 1170, 1210, 1248,
    1296, 1336, 1372, 1416, 1452, 1486, 1516, 1546, 1580, 1616, 1652, 1678, 1714, 1742, 1776, 1798, 1830,
    1862, 1886, 1912, 1940, 1968, 1992, 2010, 2038, 2062, 2090, 2114, 2134, 2158, 2178, 2202, 2222, 2246,
    2266, 2282, 2300, 2324, 2344, 2360, 2372, 2390, 2406, 2422, 2438, 2458, 2478, 2494, 2510, 2526, 2546,
    2562, 2582, 2598, 2614, 2630, 2648, 2660, 2670, 2682, 2698, 2710, 2724, 2736, 2752, 2764, 2780, 2792,
    2808, 2820, 2836, 2848, 2864, 2876, 2888, 2896, 2908, 2920, 2928, 2940, 2948, 2960, 2972, 2984, 2992,
    3004, 3014, 3028, 3036, 3048, 3056, 3068, 3080, 3088, 3100, 3110, 3120, 3128, 3140, 3148, 3160, 3168,
    3174, 3182, 3190, 3202, 3210, 3218, 3228, 3240, 3256, 3266, 3276, 3288, 3300, 3306, 3318, 3326, 3334,
    3342, 3350, 3360, 3370, 3378, 3386, 3394, 3398, 3406, 3414, 3422, 3426, 3436, 3444, 3454, 3466, 3476,
    3486, 3498, 3502, 3510, 3518, 3526, 3530, 3538, 3546, 3554, 3558, 3564, 3570, 3574, 3582, 3590, 3598,
    3604, 3610, 3618, 3628, 3634, 3640, 3644, 3652, 3656, 3664, 3670, 3678, 3688, 3696, 3700, 3708, 3712,
    3716, 3722, 3730, 3736, 3740, 3748, 3752, 3756, 3760, 3766, 3774, 3778, 3786, 3790, 3800, 3808, 3812,
    3816, 3824, 3830, 3832, 3842, 3846, 3850, 3854, 3858, 3862, 3864, 3870, 3874, 3878, 3882, 3888, 3894,
    3900, 3908, 3912, 3918, 3924, 3928, 3934, 3940, 3946, 3952, 3958, 3966, 3974, 3978, 3982, 3986, 3990,
    3994, 4002, 4006, 4010, 4018, 4022, 4032, 4038, 4046, 4050, 4056, 4062, 4072, 4076, 4084, 4090, 4095
    }

};

HI_U32 gc4603_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{   
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

	pstDef->stDrc.bEnable				= HI_FALSE;
	pstDef->stDrc.u32BlackLevel 		= 0x00;
	pstDef->stDrc.u32WhiteLevel 		= 0x4FF; 
	pstDef->stDrc.u32SlopeMax			= 0x30;
	pstDef->stDrc.u32SlopeMin			= 0x00;
	pstDef->stDrc.u32VarianceSpace		= 0x04;
	pstDef->stDrc.u32VarianceIntensity	= 0x01;
	pstDef->stDrc.u32Asymmetry			= 0x14;
	pstDef->stDrc.u32BrightEnhance		= 0xC8;

    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));
    memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_GC4603_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_GC4603_HEIGHT;
    
    return 0;
}


HI_U32 gc4603_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
          
    for (i=0; i<4; i++)
    {
        pstBlackLevel->au16BlackLevel[i] = 0x8; 
    }

    return 0;    
}

HI_VOID gc4603_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        /* 5 fps */			// VB = 1540* 30/5 - 1456-16 = 7768=0x1e58
        SENSOR_I2C_WRITE(VBLINK_H, 0x1e); 		// vb[12:8]
        SENSOR_I2C_WRITE(VBLINK_L, 0x58); 		// vb[7:0]
        
        /* min gain */
        SENSOR_I2C_WRITE(D_GAIN_H, 0x01);		//pre-gain[9:6]
        SENSOR_I2C_WRITE(D_GAIN_L, 0x00);		//pre-gain[5:0]
        
        SENSOR_I2C_WRITE(A_GAIN, 0x00);       	//analog gain
        
        /* max exposure time*/		//max exposure line = 1540*6 -2=0x2416
        SENSOR_I2C_WRITE(EXPOSURE_H, 0x24);			//exp_time[12:8]
        SENSOR_I2C_WRITE(EXPOSURE_L, 0x16);			//exp_time[7:0]
    }        
    else /* setup for ISP 'normal mode*/
    {
        SENSOR_I2C_WRITE(VBLINK_H, 0x00); //VB-min 68
        SENSOR_I2C_WRITE(VBLINK_L, 0x44); 
    }

    return;
}

static HI_S32 gc4603_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;
    bInit = HI_FALSE;
    
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }
    
    if((pstSensorImageMode->u16Width <= 2560)&&(pstSensorImageMode->u16Height <= 1440))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_4M_30FPS_MODE;
        }
        else
        {
            printf("Not support! Width:%d, Height:%d, Fps:%f\n", 
            pstSensorImageMode->u16Width, 
            pstSensorImageMode->u16Height,
            pstSensorImageMode->f32Fps);
            return -1;
        }
    }
    else
    {
        printf("Not support! Width:%d, Height:%d, Fps:%f\n", 
        pstSensorImageMode->u16Width, 
        pstSensorImageMode->u16Height,
        pstSensorImageMode->f32Fps);
        return -1;
    }

    /* Sensor first init */
    if (HI_FALSE == bSensorInit)
    {
        gu8SensorImageMode = u8SensorImageMode;
        return 0;
    }

    /* Switch SensorImageMode */
    if (u8SensorImageMode == gu8SensorImageMode)
    {
        /* Don't need to switch SensorImageMode */
        return -1;
    }

    gu8SensorImageMode = u8SensorImageMode;
    return 0;
    
}

HI_U32 gc4603_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
   // return 0;

    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;		
        g_stSnsRegsInfo.u32RegNum = 5;
	
        for (i = 0; i < g_stSnsRegsInfo.u32RegNum; i++)
        {	
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }

        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;     
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x03;    //integration time h
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;        
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x04;    //integration time l
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0xb6;    //analog gain
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0xb1;    //[3:0]->digital gain[9:6] h
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0xb2;    //[7:2]digital gain[5:0] l
/*
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x07;    //VB h
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x08;    //VB l
 */       
        bInit = HI_TRUE;
    }   
    
    if (HI_NULL == pstSnsRegsInfo)
    {
        printf("null pointer when get sns reg info!\n");
        return -1;
    }

    memcpy(pstSnsRegsInfo, &g_stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S)); 
    memcpy(&g_stPreSnsRegsInfo, &g_stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S)); 

    return 0;
}

int  gc4603_set_inifile_path(const char *pcPath)
{
    memset(pcName, 0, sizeof(pcName));
        
    if (HI_NULL == pcPath)
    {        
        strncat(pcName, "configs/", strlen("configs/"));
        strncat(pcName, CMOS_CFG_INI, sizeof(CMOS_CFG_INI));
    }
    else
    {
        if(strlen(pcPath) > (PATHLEN_MAX - 30))
        {
            printf("Set inifile path is larger PATHLEN_MAX!\n");
            return -1;        
        }
        
        strncat(pcName, pcPath, strlen(pcPath));
        strncat(pcName, CMOS_CFG_INI, sizeof(CMOS_CFG_INI));
    }
    
    return 0;
}

HI_VOID gc4603_global_init()
{   
    gu8SensorImageMode = SENSOR_4M_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_GC4603_4MP30_LINEAR; 
    gu32FullLines = VMAX_GC4603_4MP30_LINEAR;
    gu32PreFullLines = VMAX_GC4603_4MP30_LINEAR;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    
  
}

void gc4603_reg_init()
{
	//////////////////////////
	/////////////system///////
	//////////////////////////
	SENSOR_I2C_WRITE(0xfe,0x00);				
	SENSOR_I2C_WRITE(0xfe,0x00);						  
	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0xf7,0x01);
	SENSOR_I2C_WRITE(0xf8,0x0d);
	SENSOR_I2C_WRITE(0xf9,0xae);
	SENSOR_I2C_WRITE(0xfa,0x84);						  
	SENSOR_I2C_WRITE(0xfc,0x8e);	
	SENSOR_I2C_WRITE(0xfe,0x00);				
	SENSOR_I2C_WRITE(0xfe,0x00);						  
	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0x88,0x03);	  
	SENSOR_I2C_WRITE(0x8a,0xbf);
	SENSOR_I2C_WRITE(0xe7,0xc0);
	//////////////////////////
	/////////////analog////////
	//////////////////////////
	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0x03,0x05);
	SENSOR_I2C_WRITE(0x04,0x6a);
	SENSOR_I2C_WRITE(0x05,0x02);
	SENSOR_I2C_WRITE(0x06,0x2a); 
	SENSOR_I2C_WRITE(0x07,0x00);
	SENSOR_I2C_WRITE(0x08,0x2c);
	SENSOR_I2C_WRITE(0x09,0x00);
	SENSOR_I2C_WRITE(0x0a,0x1c); 
	SENSOR_I2C_WRITE(0x0b,0x00);
	SENSOR_I2C_WRITE(0x0c,0x08); 
	SENSOR_I2C_WRITE(0x0d,0x05);
	SENSOR_I2C_WRITE(0x0e,0xb0); 
	SENSOR_I2C_WRITE(0x0f,0x0a);
	SENSOR_I2C_WRITE(0x10,0x18);
	SENSOR_I2C_WRITE(0x12,0x26); 
	SENSOR_I2C_WRITE(0x17,0xd5);
	SENSOR_I2C_WRITE(0x18,0x02);
	SENSOR_I2C_WRITE(0x19,0x0e);
	SENSOR_I2C_WRITE(0x1a,0x18); 
	SENSOR_I2C_WRITE(0xe2,0x00); 
	SENSOR_I2C_WRITE(0x1b,0x21); 
	SENSOR_I2C_WRITE(0x1c,0x2d); 
	SENSOR_I2C_WRITE(0x1d,0x13);
	SENSOR_I2C_WRITE(0x21,0x05);
	SENSOR_I2C_WRITE(0x29,0x58); 
	SENSOR_I2C_WRITE(0x24,0x80); 
	SENSOR_I2C_WRITE(0x25,0xc1); 
	SENSOR_I2C_WRITE(0x27,0x64); 
	SENSOR_I2C_WRITE(0x2f,0x14); 
	SENSOR_I2C_WRITE(0xdc,0xb3); 
	SENSOR_I2C_WRITE(0x32,0x48); 
	SENSOR_I2C_WRITE(0xcd,0xaa); 
	SENSOR_I2C_WRITE(0xcf,0x40); 
	SENSOR_I2C_WRITE(0xd0,0xb2); 
	SENSOR_I2C_WRITE(0xd1,0xb4); 
	SENSOR_I2C_WRITE(0xd2,0xcb); 
	SENSOR_I2C_WRITE(0xd3,0x63); 
	SENSOR_I2C_WRITE(0xd8,0x16); 
	SENSOR_I2C_WRITE(0xe4,0x78); 
	SENSOR_I2C_WRITE(0xe6,0x08); 
	SENSOR_I2C_WRITE(0xe8,0x02);
	SENSOR_I2C_WRITE(0xe9,0x02);
	SENSOR_I2C_WRITE(0xea,0x03);
	SENSOR_I2C_WRITE(0xeb,0x03);
	SENSOR_I2C_WRITE(0xec,0x02);
	SENSOR_I2C_WRITE(0xed,0x02);
	SENSOR_I2C_WRITE(0xee,0x03);
	SENSOR_I2C_WRITE(0xef,0x03);
	//////////////////////////
	////////window 2560x1440//
	//////////////////////////
	SENSOR_I2C_WRITE(0x90,0x01);
	SENSOR_I2C_WRITE(0x92,0x02); 
	SENSOR_I2C_WRITE(0x94,0x01); 
	SENSOR_I2C_WRITE(0x95,0x05);
	SENSOR_I2C_WRITE(0x96,0xa0);
	SENSOR_I2C_WRITE(0x97,0x0a);
	SENSOR_I2C_WRITE(0x98,0x00);
	//////////////////////////
	////////////gain //////////
	//////////////////////////
	SENSOR_I2C_WRITE(0xb0,0x4b);
	SENSOR_I2C_WRITE(0xb1,0x01);
	SENSOR_I2C_WRITE(0xb2,0x00);
	SENSOR_I2C_WRITE(0xb6,0x00);
	SENSOR_I2C_WRITE(0x99,0x40);
	SENSOR_I2C_WRITE(0x9a,0x41);
	SENSOR_I2C_WRITE(0x9b,0x42);
	SENSOR_I2C_WRITE(0x9c,0x43);
	SENSOR_I2C_WRITE(0x9d,0x44);
	SENSOR_I2C_WRITE(0x9e,0x4c);
	SENSOR_I2C_WRITE(0x9f,0x54);
	SENSOR_I2C_WRITE(0xa0,0x5c);
	//////////////////////////
	////// BLK ////////////////
	//////////////////////////
	SENSOR_I2C_WRITE(0x40,0x22);
	SENSOR_I2C_WRITE(0x41,0x20);
	SENSOR_I2C_WRITE(0x43,0x01);
	SENSOR_I2C_WRITE(0x4c,0x00);
	SENSOR_I2C_WRITE(0x4d,0x03);
	SENSOR_I2C_WRITE(0x4e,0x3c);
	SENSOR_I2C_WRITE(0x4f,0x3c);
	SENSOR_I2C_WRITE(0x60,0x00);
	SENSOR_I2C_WRITE(0x61,0x80);
	SENSOR_I2C_WRITE(0xc0,0x00);
	SENSOR_I2C_WRITE(0xc1,0xff);
	SENSOR_I2C_WRITE(0xc2,0xff);
	SENSOR_I2C_WRITE(0xc3,0xff);
	SENSOR_I2C_WRITE(0xc4,0xff);
	SENSOR_I2C_WRITE(0xc5,0xfe);
	SENSOR_I2C_WRITE(0xc6,0xfd);
	SENSOR_I2C_WRITE(0xc7,0xfc);
	SENSOR_I2C_WRITE(0xc8,0x00);
	SENSOR_I2C_WRITE(0xc9,0xff);
	SENSOR_I2C_WRITE(0xca,0xff);
	SENSOR_I2C_WRITE(0xcb,0xff);
	SENSOR_I2C_WRITE(0xcc,0xff);
	SENSOR_I2C_WRITE(0x84,0xfe);
	SENSOR_I2C_WRITE(0x85,0xfd);
	SENSOR_I2C_WRITE(0x87,0xfc);
	///////////dark offset//////
	SENSOR_I2C_WRITE(0xac,0x30);
	SENSOR_I2C_WRITE(0xab,0x00);
	SENSOR_I2C_WRITE(0xae,0x08);
	//////////////////////////
	///// dark sun /////////////
	//////////////////////////
	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0x68,0xf7);
	SENSOR_I2C_WRITE(0x6c,0x06);
	SENSOR_I2C_WRITE(0x6e,0xff); 
	/////////////////////////
	//////////	MIPI   /////////
	/////////////////////////
	SENSOR_I2C_WRITE(0xfe,0x03);
	SENSOR_I2C_WRITE(0x01,0x07);
	SENSOR_I2C_WRITE(0x02,0x33);
	SENSOR_I2C_WRITE(0x03,0x93);
	SENSOR_I2C_WRITE(0x04,0x04);
	SENSOR_I2C_WRITE(0x05,0x00);
	SENSOR_I2C_WRITE(0x06,0x80);
	SENSOR_I2C_WRITE(0x10,0x91);
	SENSOR_I2C_WRITE(0x11,0x2b);
	SENSOR_I2C_WRITE(0x12,0x80);
	SENSOR_I2C_WRITE(0x13,0x0c);
	SENSOR_I2C_WRITE(0x15,0x02);
	SENSOR_I2C_WRITE(0x16,0x09);
	SENSOR_I2C_WRITE(0x18,0x01); 
	SENSOR_I2C_WRITE(0x1b,0x14);
	SENSOR_I2C_WRITE(0x1c,0x14);
	SENSOR_I2C_WRITE(0x21,0x05);
	SENSOR_I2C_WRITE(0x22,0x04);
	SENSOR_I2C_WRITE(0x23,0x19);
	SENSOR_I2C_WRITE(0x24,0x01);
	SENSOR_I2C_WRITE(0x25,0x14);
	SENSOR_I2C_WRITE(0x26,0x0c);
	SENSOR_I2C_WRITE(0x29,0x04);
	SENSOR_I2C_WRITE(0x2a,0x09);
	SENSOR_I2C_WRITE(0x2b,0x08);
	SENSOR_I2C_WRITE(0xfe,0x00);
	
    bSensorInit = HI_TRUE;
    printf("===============================================\n");
    printf("=======GC4603 Sensor 4M30fps Initial OK!=======\n");
    printf("===============================================\n");

}

HI_S32 gc4603_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = gc4603_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = gc4603_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = gc4603_set_image_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = gc4603_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = gc4603_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = gc4603_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = gc4603_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int gc4603_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    gc4603_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, GC4603_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    gc4603_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, GC4603_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    gc4603_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, GC4603_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int gc4603_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, GC4603_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, GC4603_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, GC4603_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


HI_S32 GC_GC4603_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)
{
		// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;
	gc4603_register_callback();
	ALG_LIB_S stLib;
	HI_S32 s32Ret;
    /* 1. register ae lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_Register(0,&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AE_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 2. register awb lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_Register(0,&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AWB_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 3. register af lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_AF_Register(0,&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AF_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }
	printf(" GC4603 sensor 2560-25fps init success!\n");
	
	return s32Ret;
}
int GC4603_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_GC4603_WIDTH;
        *ret_height = SENSOR_GC4603_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_GC4603_probe()
{
    uint16_t ret_data1 = 0;
    uint16_t ret_data2 = 0;
    SENSOR_I2C_READ(0xF0, &ret_data1);
    SENSOR_I2C_READ(0xF1, &ret_data2);
    if(ret_data1 == GC4603_CHECK_DATA_MSB	&& ret_data2 == GC4603_CHECK_DATA_LSB)
    {
        sdk_sys->write_reg(0x200f0050, 0x2); //set sensor clock
        sdk_sys->write_reg(0x200f0054, 0x2); //set sensor clock
        sdk_sys->write_reg(0x2003002c, 0xE0007); //set sensor clock  24MHz
        sdk_sys->write_reg(0x20030104, 0x0);
        return true;
    }
    return false;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif 

