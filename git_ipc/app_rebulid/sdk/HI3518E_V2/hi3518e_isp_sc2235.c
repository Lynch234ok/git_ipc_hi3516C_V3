#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"


#define SENSOR_NAME "sc2235"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x60, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x60, 2, 1))


#define SENSOR_SC2235_WIDTH (1920)
#define SENSOR_SC2235_HEIGHT  (1080)
#define SC2235_CHECK_DATA_LSB	(0x22)
#define SC2235_CHECK_DATA_MSB	(0x35)//CHIP_ID address is 0x3107&0x3108

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__SC2235_CMOS_H_)
#define __SC2235_CMOS_H_

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

#define SC2235_ID 2235

#define CMOS_SC2235_ISP_WRITE_SENSOR_ENABLE (1)


/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/
#ifdef INIFILE_CONFIG_MODE

extern AE_SENSOR_DEFAULT_S  g_AeDft[];
extern AWB_SENSOR_DEFAULT_S g_AwbDft[];
extern ISP_CMOS_DEFAULT_S   g_IspDft[];
extern HI_S32 Cmos_LoadINIPara(const HI_CHAR *pcName);
#else

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr=0x60;
static unsigned int sensor_addr_byte=2;
static unsigned int sensor_data_byte=1;

#define VMAX_ADDR_H              (0x320e)
#define VMAX_ADDR_L              (0x320f)

#define SENSOR_1080P_30FPS_MODE  (1)

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define VMAX_1080P30_LINEAR     (1125+INCREASE_LINES)// 1125:25fps config 2016.08.16
#define CMOS_OV2235_SLOW_FRAMERATE_MODE (0)

#define FULL_LINES_MAX  (0xFFFF)

static HI_U8 gu8SensorImageMode = SENSOR_1080P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = VMAX_1080P30_LINEAR;
static HI_U32 gu32FullLines = VMAX_1080P30_LINEAR;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};
static HI_U8 gu8Fps = 25;


/* AE default parameter and function */
static HI_S32 sc2235_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*25/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
	pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;
    pstAeSnsDft->u8AERunInterval  = 1;
    pstAeSnsDft->u32InitExposure  = 10;


    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
            
    pstAeSnsDft->u8AeCompensation = 0x40;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
    pstAeSnsDft->u32MinIntTime = 1;
//    pstAeSnsDft->u32MaxIntTimeTarget = pstAeSnsDft->u32MaxIntTime;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;//AE_ACCURACY_LINEAR;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;//0.0625; 
    pstAeSnsDft->u32MaxAgain = 15872;//248;  //62倍 
    pstAeSnsDft->u32MinAgain = 1024;//16;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;//0.0625;//invalidate
    pstAeSnsDft->u32MaxDgain = 1;  
    pstAeSnsDft->u32MinDgain = 1;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain; 
	
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift; 

	  

    return 0;
}

/* the function of sensor set fps */
static HI_VOID sc2235_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

    HI_U32 u32VblankingLines = 0xFFFF;
    if ((f32Fps <= 30) && (f32Fps >= 0.5))
    {
        if(SENSOR_1080P_30FPS_MODE == gu8SensorImageMode)
        {
            u32VblankingLines = VMAX_1080P30_LINEAR * 25 / f32Fps;
        }
    }
    else
    {
        printf("Not support Fps: %f\n", f32Fps);
        return;
    }
    gu32FullLinesStd = u32VblankingLines;
//pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;

	gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;

#if CMOS_SC2235_ISP_WRITE_SENSOR_ENABLE
	g_stSnsRegsInfo.astI2cData[5].u32Data = (u32VblankingLines >> 8) & 0xFF ;//(gu32FullLinesStd& 0xFF00) >> 8;
	g_stSnsRegsInfo.astI2cData[6].u32Data = u32VblankingLines & 0xFF;//gu32FullLinesStd & 0xFF;
#else
    SENSOR_I2C_WRITE(0x320e, (u32VblankingLines >> 8) & 0xff) ;
    SENSOR_I2C_WRITE(0x320f, u32VblankingLines & 0xff);
#endif

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32MaxIntTime = u32VblankingLines - 4;
	//	printf("%s %d u32MaxIntTime=%d \n",__FUNCTION__,__LINE__,pstAeSnsDft->u32MaxIntTime);
	//pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps/ 2;
	// pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
	//gu32FullLines = gu32FullLinesStd;
	//   pstAeSnsDft->u32MaxIntTime = u32VblankingLines - 4;
	//   pstAeSnsDft->u32MaxIntTimeTarget=u32VblankingLines - 4;
	//	printf("%s %d u32MaxIntTime=%d \n",__FUNCTION__,__LINE__,pstAeSnsDft->u32MaxIntTime);
	gu8Fps = f32Fps;	
	pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps/ 2;
	pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
	//gu32FullLines = gu32FullLinesStd;
//	pstAeSnsDft->u32FullLines = gu32FullLines;
#if 0
		SENSOR_I2C_WRITE(0x336a, ((u32VblankingLines >> 8) & 0xFF));
		SENSOR_I2C_WRITE(0x336b, u32VblankingLines & 0xFf);
	
		SENSOR_I2C_WRITE(0x3368, (((u32VblankingLines - 0x265) >> 8) & 0xFF));
		SENSOR_I2C_WRITE(0x3369, (u32VblankingLines - 0x265) & 0xFf);

		HI_U16 u16RegH = 0, u16RegL = 0,u16Temp = 0;
		SENSOR_I2C_READ(0x320c,&u16RegH);
		SENSOR_I2C_READ(0x320d,&u16RegL);

		u16Temp = (u16RegH<<8) | u16RegL;
		u16Temp -= 0x80;

		SENSOR_I2C_WRITE(0x3340, (u16Temp>>8)&0xff);
		SENSOR_I2C_WRITE(0x3341, u16Temp&0xff);
#endif
    return;
}


static HI_VOID sc2235_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

	u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
	gu32FullLines = u32FullLines;
	pstAeSnsDft->u32FullLines = gu32FullLines;//gu32FullLinesStd;

//	gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;

#if CMOS_SC2235_ISP_WRITE_SENSOR_ENABLE
	g_stSnsRegsInfo.astI2cData[5].u32Data = (u32FullLines >> 8) & 0xFF;
	g_stSnsRegsInfo.astI2cData[6].u32Data = u32FullLines & 0xFf;
#else
	SENSOR_I2C_WRITE(0x320e, ((u32FullLines >> 8) & 0xFF));
	SENSOR_I2C_WRITE(0x320f, u32FullLines & 0xFf);
#endif
	pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;

//	printf("%s %d u32MaxIntTime=%d \n",__FUNCTION__,__LINE__,pstAeSnsDft->u32MaxIntTime);
//    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;
//   pstAeSnsDft->u32MaxIntTimeTarget=gu32FullLines - 4;
//	printf("%s %d u32MaxIntTime=%d \n",__FUNCTION__,__LINE__,pstAeSnsDft->u32MaxIntTime);

#if 0
		SENSOR_I2C_WRITE(0x336a, ((u32FullLines >> 8) & 0xFF));
		SENSOR_I2C_WRITE(0x336b, u32FullLines & 0xFf);
	
		SENSOR_I2C_WRITE(0x3368, (((u32FullLines - 0x265) >> 8) & 0xFF));
		SENSOR_I2C_WRITE(0x3369, (u32FullLines - 0x265) & 0xFf);

		HI_U16 u16RegH = 0, u16RegL = 0,u16Temp = 0;
		SENSOR_I2C_READ(0x320c,&u16RegH);
		SENSOR_I2C_READ(0x320d,&u16RegL);

		u16Temp = (u16RegH<<8) | u16RegL;
		u16Temp -= 0x80;

		SENSOR_I2C_WRITE(0x3340, (u16Temp>>8)&0xff);
		SENSOR_I2C_WRITE(0x3341, u16Temp&0xff);
#endif
    //pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * gu8Fps/ 2;
    //pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_U32 u32OldIntTime = 0;
static HI_VOID sc2235_inttime_update(HI_U32 u32IntTime)
{
	//printf("u32IntTime:%#x...........%#x........%#x..\n",u32IntTime, (u32IntTime >> 4) & 0xFF,(u32IntTime<<4) & 0xF0);

#if 0
if (u32OldIntTime != u32IntTime)
{
	//printf("u32IntTime:%#x...........%#x........%#x..\n",u32IntTime, (u32IntTime >> 4) & 0xFF,(u32IntTime<<4) & 0xF0);
	u32OldIntTime = u32IntTime;
}
#endif

#if CMOS_SC2235_ISP_WRITE_SENSOR_ENABLE
	g_stSnsRegsInfo.astI2cData[18].u32Data = (u32IntTime >> 12) & 0x0F ;            //0x3e00
	g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 4) & 0xFF;   //0x3e01
	g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime<<4) & 0xF0;     //0x3e02
#else
	SENSOR_I2C_WRITE(0x3e01, ((u32IntTime >> 4) & 0xFF));
	SENSOR_I2C_WRITE(0x3e02, (u32IntTime<<4) & 0xF0);
#endif

    return;
}


static HI_VOID sc2235_Again_limit(HI_U32 *pu32Again )
{
		//限制最大模拟增益逻辑
		HI_U32 u32CurrVal = 0, u32AgVal = 0;
		HI_U16  u8HighRegVal = 0,u8LowRegVal = 0;
		static HI_U8 u8CurrFlag = 0;
		static HI_U8 u8StaticAgain = 0xF8;

        #define MAXTEMP 0x1980
        #define MINTEMP 0x1680
 

		if (pu32Again == NULL)
		{
			return;
		}	

		SENSOR_I2C_READ(0x3911,&u8HighRegVal);
		SENSOR_I2C_READ(0x3912,&u8LowRegVal);
		u32CurrVal = u8HighRegVal<<8 | u8LowRegVal;
		
		
		//if ((u32CurrVal >= MAXTEMP) && (u8StaticAgain > 0x20))
		if ((u32CurrVal >= MAXTEMP) && (u8StaticAgain > 0x10))
		{
			 u8StaticAgain -= 1;
			 u8CurrFlag = 1;
		}
		else if ((u32CurrVal <= MINTEMP) && (u8StaticAgain < 0xf8) && (u8CurrFlag == 1))
		{
			 u8StaticAgain += 1;
		}
		else if((u32CurrVal >= MINTEMP) && (u32CurrVal <= MAXTEMP) && (u8CurrFlag == 1))
		{
		    *pu32Again = u8StaticAgain;
		}
		else
		{
			//do nothing
		}

		if ((u32CurrVal <= MINTEMP) && (*pu32Again <= u8StaticAgain) && (u8CurrFlag == 1))//----
  	    {
  	    
  		     u8CurrFlag = 0; 
  	    }


#if 0
        if ((u32CurrVal >= 0x1980) && (u8CurrFlag == 1))
        {
             if(*pu32Again > u8StaticAgain)
             {
                 u8StaticAgain = *pu32Again;
             }
             else
             {
                 *pu32Again = u8StaticAgain;
             }
        }
        if ((u32CurrVal <= 0x1680) && (u8CurrFlag == 1))
        {
             if(*pu32Again > u8StaticAgain)
             {
                 *pu32Again = u8StaticAgain;
             }
        }
        
#else
        if ((u32CurrVal >= MAXTEMP) && (u8CurrFlag == 1))
        {
        
       		 *pu32Again=(u8StaticAgain<*pu32Again)?u8StaticAgain:(*pu32Again);
        }
        if ((u32CurrVal <= MINTEMP) && (u8CurrFlag == 1))
		{
		    
		    *pu32Again=(u8StaticAgain<*pu32Again)?u8StaticAgain:(*pu32Again);
        }
#endif
		return;
}

static const HI_U16 gau16AgainTab[64]={
	1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,
	1920,1984,2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,
	3584,3712,3840,3968,4096,4352,4608,4864,5120,5376,5632,5888,6144,6400,
	6656,6912,7168,7424,7680,7936,8192,8704,9216,9728,10240,10752,11264,
	11776,12288,12800,13312,13824,14336,14848,15360,15872
};

static HI_VOID sc2235_cmos_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
	int again;
	int i;
	static HI_U8 againmax = 63;
	again = *pu32AgainLin;

	if(again >= gau16AgainTab[againmax])
	{
		*pu32AgainDb = againmax;
	}
	else
	{
		for(i=1;i<64;i++)
		{
			if(again<gau16AgainTab[i])
			{
				*pu32AgainDb = i-1;
				break;
			}
		}
	}
	
	*pu32AgainLin = gau16AgainTab[*pu32AgainDb];

    return;
}

static HI_VOID sc2235_cmos_dgain_calc_table(HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
	*pu32DgainDb = *pu32DgainLin/1024;
	if(*pu32DgainDb == 3)
	{
		*pu32DgainDb = 2;
	}
	else if(*pu32DgainDb >= 4 && *pu32DgainDb < 8)
	{
		*pu32DgainDb = 4;
	}
	else if(*pu32DgainDb >= 8)
	{
		*pu32DgainDb = 8;
	}
	
	*pu32DgainLin = *pu32DgainDb*1024;
    return;
}

ISP_GAMMA_ATTR_S  SC2235_GammaTablesUser_Line[] =
{
	// 0
	{
		HI_TRUE,
			ISP_GAMMA_CURVE_USER_DEFINE,
		{			
			0x000,
				0x036,0x06A,0x09E,0x0D1,0x103,0x134,0x164,0x193,0x1C2,0x1EF,0x21C,0x248,0x274,0x29E,0x2C9,0x2F2,
				0x31B,0x343,0x36A,0x391,0x3B7,0x3DD,0x402,0x426,0x44A,0x46D,0x490,0x4B3,0x4D4,0x4F6,0x517,0x537,
				0x557,0x576,0x595,0x5B4,0x5D2,0x5F0,0x60D,0x62A,0x647,0x663,0x67F,0x69A,0x6B5,0x6D0,0x6EA,0x704,
				0x71E,0x738,0x751,0x769,0x782,0x79A,0x7B2,0x7C9,0x7E1,0x7F8,0x80E,0x825,0x83B,0x851,0x866,0x87C,
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
		}
	},
};

static HI_VOID sc2235_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
	HI_U8 u32Reg0x3e09,u32Reg0x3e08;
	HI_U8 i;
	int gain_all = gau16AgainTab[u32Again];
	ISP_GAMMA_ATTR_S ret_gamma_attr;
	HI_S32 ret = -1;
	ISP_DEV IspDev = 0;
	ISP_DRC_ATTR_S stDRC;

	HI_U16  u8RegVal = 0;

	// Again
	u32Reg0x3e09 = 0x10 | (u32Again&0x0f);
	u32Reg0x3e08 = 0x03;
	u32Again = u32Again/16;
	for(i=0;i <  u32Again;i++)
	{
		u32Reg0x3e08 = (u32Reg0x3e08<<1)|0x01;
	}
/**
	// Dgain
	if(u32Dgain == 2)
	{
		u32Reg0x3e08 |= 0x20; 
	}
	else if(u32Dgain == 4)
	{
		u32Reg0x3e08 |= 0x60; 
	}
	else if(u32Dgain == 8)
	{
		u32Reg0x3e08 |= 0xe0; 
	}
/**/	
	g_stSnsRegsInfo.astI2cData[3].u32Data = u32Reg0x3e08;
	g_stSnsRegsInfo.astI2cData[4].u32Data = u32Reg0x3e09;

	g_stSnsRegsInfo.astI2cData[7].u32Data =  0x84;
	g_stSnsRegsInfo.astI2cData[8].u32Data =  0x04;

	g_stSnsRegsInfo.astI2cData[9].u32Data =  0x00;
	
	if(gain_all < 2*1024){
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x05;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x84;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x2f;
		g_stSnsRegsInfo.astI2cData[14].u32Data = 0xc6;   
	}
	else if(gain_all < 8*1024){
      
		g_stSnsRegsInfo.astI2cData[14].u32Data = 0xc6;
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x13;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x88;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x2f;
	}
	else if (gain_all < 15.5*1024){
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x14;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x88;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x2f;
        g_stSnsRegsInfo.astI2cData[14].u32Data = 0xc6;  
	}
	else
	{
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0xff;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x88;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x3a;
        g_stSnsRegsInfo.astI2cData[14].u32Data = 0x06;  
	}
	
	//SENSOR_I2C_READ(0x3e01,&u8RegVal);
	//if(u8RegVal < 0x05)
	if(g_stSnsRegsInfo.astI2cData[0].u32Data < 0x05)
	{
		g_stSnsRegsInfo.astI2cData[13].u32Data = 0x12;
	}
	//else if(u8RegVal > 0x0a)
	else if(g_stSnsRegsInfo.astI2cData[0].u32Data > 0x0a)
	{
		g_stSnsRegsInfo.astI2cData[13].u32Data = 0x02;
	}
/**/	
	g_stSnsRegsInfo.astI2cData[15].u32Data =  0x30;	


if(gain_all < 10*1024)
{
g_stSnsRegsInfo.astI2cData[16].u32Data =  0x04;	
g_stSnsRegsInfo.astI2cData[17].u32Data =  0x18;	
}
else
{
g_stSnsRegsInfo.astI2cData[16].u32Data =  0x02;	
g_stSnsRegsInfo.astI2cData[17].u32Data =  0x08;	
}

/**	ret = HI_MPI_ISP_GetGammaAttr(IspDev, &ret_gamma_attr);
	if(HI_SUCCESS != ret)
	{
		printf("Get GammaAttr ERR!\n");
		return -1;
	}

	ret = HI_MPI_ISP_SetGammaAttr(IspDev, &SC2235_GammaTablesUser_Line[0]);
	if(HI_SUCCESS != ret)
	{
		printf("Set HI_MPI_ISP_SetGammaTable ERR!\n");
		return -1;
	}

	//DRC
	ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stDRC);
	if(HI_SUCCESS != ret)
	{
		printf("Get HI_MPI_ISP_GetDRCAttr ERR!\n");
		return -1;
	}
	
	if (gain_all < 4*1024)
	{
		stDRC.bEnable = HI_TRUE;
		stDRC.u16DarkGainLmtY = 0;
		stDRC.u16DarkGainLmtC = 0;
		stDRC.u16BrightGainLmt = 0;
		stDRC.enOpType = OP_TYPE_MANUAL;
		stDRC.stManual.u8Strength = 64;
		//stDRC.stManual.u8LocalMixingBright = 54;
		//stDRC.stManual.u8LocalMixingDark = 32;
	}
	else
	{
		stDRC.bEnable = HI_FALSE;
	}
	
	ret = HI_MPI_ISP_SetDRCAttr(IspDev, &stDRC);
	if(HI_SUCCESS != ret)
	{
		printf("Set HI_MPI_ISP_SetDRCAttr ERR!\n");
		return -1;
	}
/**/
    return;
}


HI_S32 sc2235_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = sc2235_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = sc2235_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= sc2235_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = sc2235_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = sc2235_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = sc2235_cmos_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = NULL;//sc2235_cmos_dgain_calc_table;

    return 0;
}


/* AWB default parameter and function */
static AWB_CCM_S g_stAwbCcm =
{  
	4984,	 
	{		
		0x1D9,0x80B4,0x8025,
		0x805B,0x1CA,0x806F,
		0x8010,0x8150,0x260,	
	},	

	3798,	 
	{		
		0x1A0,0x8055,0x804B,
		0x806D,0x1B2,0x8045,
		0x8020,0x8140,0x260,	
	},

	2538,	 
	{			 
		0x198,0x802C,0x806C,
		0x806D,0x1AC,0x803F,
		0x8056,0x813F,0x295,	
	} 

};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,
	
    /*1,  2,  4,  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768*/
    /* saturation */   
    //{0x7a,0x7a,0x78,0x74,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
    {160,158,140,128,115,95,75,75,75,75,75,75,75,75,75,75}

};

static HI_S32 sc2235_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4984;
    pstAwbSnsDft->au16GainOffset[0] = 0x186;    
    pstAwbSnsDft->au16GainOffset[1] = 0x100;    
    pstAwbSnsDft->au16GainOffset[2] = 0x100;    
    pstAwbSnsDft->au16GainOffset[3] = 0x163;    
    pstAwbSnsDft->as32WbPara[0] = 116;    
    pstAwbSnsDft->as32WbPara[1] = 21;    
    pstAwbSnsDft->as32WbPara[2] = -118;    
    pstAwbSnsDft->as32WbPara[3] = 143294;    
    pstAwbSnsDft->as32WbPara[4] = 128;    
    pstAwbSnsDft->as32WbPara[5] = -92318;
    
    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_S32 sc2235_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = sc2235_get_awb_default;

    return 0;
}

#define DMNR_CALIB_CARVE_NUM_SC2235 7

float g_coef_calib_sc2235[DMNR_CALIB_CARVE_NUM_SC2235][4] = 
{
    {100.000000f, 2.000000f, 0.040397f, 5.835166f, }, 

    {800.000000f, 2.903090f, 0.047510f, 6.551895f, }, 

    {2300.000000f, 3.361728f, 0.058797f, 8.122231f, }, 

    {4100.000000f, 3.612784f, 0.063562f, 11.138043f, }, 

    {5500.000000f, 3.740363f, 0.072534f, 11.818048f, }, 

    {6300.000000f, 3.799341f, 0.077265f, 11.714485f, }, 
	{8000.000000f, 3.799341f, 0.077265f, 11.714485f, }, 

};


static ISP_NR_ISO_PARA_TABLE_S g_stNrIsoParaTab[HI_ISP_NR_ISO_LEVEL_MAX] = 
{
     //u16Threshold//u8varStrength//u8fixStrength//u8LowFreqSlope	
       {1500,       160,             256-256,            0 },  //100    //                      //                                                
       {1500,       120,             256-256,            0 },  //200    // ISO                  // ISO //u8LowFreqSlope
       {1500,       100,             256-256,            0 },  //400    //{400,  1200, 96,256}, //{400 , 0  }
       {1750,       80,              256-256,            8 },  //800    //{800,  1400, 80,256}, //{600 , 2  }
       {1500,       255,             256-256,            6 },  //1600   //{1600, 1200, 72,256}, //{800 , 8  }
       {1500,       255,             256-256,            0 },  //3200   //{3200, 1200, 64,256}, //{1000, 12 }
       {1375,       255,             256-256,            0 },  //6400   //{6400, 1100, 56,256}, //{1600, 6  }
       {1375,       255,             256-256,            0 },  //12800  //{12000,1100, 48,256}, //{2400, 0  }
       {1375,       255,             256-256,            0 },  //25600  //{36000,1100, 48,256}, //
       {1375,       255,             256-256,            0 },  //51200  //{64000,1100, 96,256}, //
       {1250,       255,             256-256,            0 },  //102400 //{82000,1000,240,256}, //
       {1250,       255,             256-256,            0 },  //204800 //                           //
       {1250,       255,             256-256,            0 },  //409600 //                           //
       {1250,       255,             256-256,            0 },  //819200 //                           //
       {1250,       255,             256-256,            0 },  //1638400//                           //
       {1250,       255,             256-256,            0 },  //3276800//                           //
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
	/*For Demosaic*/
	1, /*bEnable*/			
	60,/*u16VhLimit*/	
	40-24,/*u16VhOffset*/
	24,   /*u16VhSlope*/
	/*False Color*/
	1,    /*bFcrEnable*/
	{ 8, 8, 8, 8, 8, 8, 8, 8, 3, 0, 0, 0, 0, 0, 0, 0},    /*au8FcrStrength[ISP_AUTO_ISO_STENGTH_NUM]*/
	{24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24},    /*au8FcrThreshold[ISP_AUTO_ISO_STENGTH_NUM]*/
	/*For Ahd*/
	400, /*u16UuSlope*/	
	{512,512,512,512,512,512,512,  400,  0,0,0,0,0,0,0,0}    /*au16NpOffset[ISP_AUTO_ISO_STENGTH_NUM]*/
};

static ISP_CMOS_GE_S g_stIspGe =
{
	/*For GE*/
	0,    /*bEnable 1  */	
	7,    /*u8Slope*/	
	7,    /*u8Sensitivity*/
	4096, /*u16Threshold  8192 */
	4096, /*u16SensiThreshold   8192*/	
	{1024,1024,1024,2048,2048,2048,2048,  2048,  2048,2048,2048,2048,2048,2048,2048,2048}    /*au16Strength[ISP_AUTO_ISO_STENGTH_NUM]*/	
};

static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
  //{100,200,400,800,1600,3200,6400,12800,25600,51200,102400,204800,409600,819200,1638400,3276800};
    {0,	  0,   0,   0,    0,   1,   1,   1,    1,    1,    1,     1,     1,     1,     1,       1},/* enPixSel = ~bEnLowLumaShoot */
        
    {30,  24,  20,  18,  18,  18,  16,  16,    16,  16,  250,   250,   250,   250,    250,    250},/*maxSharpAmt1 = SharpenUD*16 */
    {32, 28, 24, 20,  20,  20,  18,  18,    18,  18,  18,   250,   250,   250,    250,    250},/*maxEdgeAmt = SharpenD*16 */

    //{120,  64,  64,  43,  43,  43,  18,  18,    18,  200,  250,   250,   250,   250,    250,    250},/*maxSharpAmt1 = SharpenUD*16 */  /*9712*/
    //{128, 200, 103, 86,  86,  86,  80,  80,    80,  250,  250,   250,   250,   250,    250,    250},/*maxEdgeAmt = SharpenD*16 */   /*9712*/
       
    {0,   0,   0,    0,   0,   0,   0,   40,  190,  200,  220,   250,   250,   250,     250,       250},/*sharpThd2 = TextureNoiseThd*4 */
    {0,   0,   0,    0,   0,   0,   0,   60,  140,    0,    0,     0,    0,     0,     0,       0},/*edgeThd2 = EdgeNoiseThd*4 */
    {59,  59,  59,  59,  59,  59,  59,   59,  101,  0,   0,    0,   0,    0,    0,      0},/*overshootAmt*/
    {59, 59, 59, 59, 59, 59, 59,  59, 59,  0,   0,    0,   0,    0,    0,     0},/*undershootAmt*/
};


static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
	/*中值滤波切换到UVNR的ISO阈值*/
	/*UVNR切换到中值滤波的ISO阈值*/
    /*0.0   -> disable，(0.0, 1.0]  -> weak，(1.0, 2.0]  -> normal，(2.0, 10.0) -> strong*/
	/*高斯滤波器的标准差*/
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800};
	{1,	    2,       4,      5,      7,      48,     32,     16,     16,     16,      16,     16,     16,     16,     16,        16},      /*UVNRThreshold*/
 	{0,		0,		0,		0,		0,		0,		0,		0,		0,		1,			1,		2,		2,		2,		2,		2},  /*Coring_lutLimit*/
	{0,		0,		0,		16,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,			34}  /*UVNR_blendRatio*/
};

static ISP_CMOS_DPC_S g_stCmosDpc = 
{
	//1,/*IR_channel*/
	//1,/*IR_position*/
	//{0,0,0,1,1,1,2,2,2,3,3,3,3,3,3,3},/*au16Strength[16]*/
	{70,100,180,244,248,250,252,252,252,252,252,252,252,252,252,252},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0,0x24,0x80,0x80,0x80,0xE5,0xE5,0xE5,0xE5},/*au16BlendRatio[16]*/
};


static ISP_CMOS_DRC_S g_stIspDrc =
{
    0,
    10,
    0,
    2,
    192,
    60,
    0,
    0,
    0,
    {1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024}
};

/*static ISP_CMOS_RGBIR_S g_stCmosRgbir =
{
    {
        1,
        1,
        ISP_IRPOS_TYPE_GR,
        0x41,
        4095,
    },
    {
        0,
        1,
        1,
        OP_TYPE_AUTO,
        0x100,
        {269,7,27,-27,293,13,-49,42,255,-295,-271,-253,-37,-125,-60}
    }
};
*/
HI_U32 sc2235_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
/*    pstDef->stDrc.bEnable               = HI_FALSE;
    pstDef->stDrc.u8Asymmetry           = 0x02;
    pstDef->stDrc.u8SecondPole          = 0xC0;
    pstDef->stDrc.u8Stretch             = 0x3C;
    pstDef->stDrc.u16BrightGainLmt      = 0x7F;
    pstDef->stDrc.u16DarkGainLmtC       = 0x7F;
    pstDef->stDrc.u16DarkGainLmtY       = 0x7F;
    pstDef->stDrc.u8RangeVar            = 0x00;
    pstDef->stDrc.u8SpatialVar          = 0x0A;
*/    


 //   memcpy(&pstDef->stLsc.stLscParaTable[0], &g_stCmosLscTable[0], sizeof(ISP_LSC_CABLI_TABLE_S)*HI_ISP_LSC_LIGHT_NUM);  
 	memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
    memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));			
  //  pstDef->stNoiseTbl.u8SensorIndex = HI_ISP_NR_SENSOR_INDEX_SC2235;
    pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_SC2235;
    pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_sc2235;

    memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);

    memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
    memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));
   // memcpy(&pstDef->stRgbir,       &g_stCmosRgbir,       sizeof(ISP_CMOS_RGBIR_S));
 //   memcpy(&pstDef->stLsc,       &g_stCmosLscTable,       sizeof(ISP_LSC_CABLI_TABLE_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_SC2235_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_SC2235_HEIGHT;

    return 0;
}

HI_U32 sc2235_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
   // HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
          
    pstBlackLevel->au16BlackLevel[0] = 74;
    pstBlackLevel->au16BlackLevel[1] = 74;
    pstBlackLevel->au16BlackLevel[2] = 74;
    pstBlackLevel->au16BlackLevel[3] = 74;

    return 0;  
    
}

HI_VOID sc2235_set_pixel_detect(HI_BOOL bEnable)
{
	HI_U32 u32Lines = VMAX_1080P30_LINEAR * 30 /5;
	
#if CMOS_SC2235_ISP_WRITE_SENSOR_ENABLE
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        /* 5 fps */
		SENSOR_I2C_WRITE(0x320e, (u32Lines >> 4) && 0xFF);
		SENSOR_I2C_WRITE(0x320f, ((u32Lines<<4)&&0xF0));
       
        /* max exposure time*/
		

    }
    else /* setup for ISP 'normal mode' */
    { 
        SENSOR_I2C_WRITE(0x320e, (gu32FullLinesStd >> 8) && 0XFF);
        SENSOR_I2C_WRITE(0x320f, gu32FullLinesStd && 0xFF);
        
        bInit = HI_FALSE;
    }
#else
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        
		SENSOR_I2C_WRITE(0x3e01, (u32Lines >> 8) && 0xFF);
		SENSOR_I2C_WRITE(0x3e02, (u32Lines - 4) && 0xFF);
        
        /* min gain */
        SENSOR_I2C_WRITE(0x3e0e, 0x00);
		SENSOR_I2C_WRITE(0x3e0f, 0x00);

		/* 5 fps */
        SENSOR_I2C_WRITE(0x320e, (u32Lines >> 8) && 0xFF);
        SENSOR_I2C_WRITE(0x320f, u32Lines && 0xFF);
    }
    else /* setup for ISP 'normal mode' */
    { 
        SENSOR_I2C_WRITE(0x320e, (gu32FullLinesStd >> 8) && 0XFF);
        SENSOR_I2C_WRITE(0x320f, gu32FullLinesStd && 0xFF);
        
        bInit = HI_FALSE;
    }
#endif

    return;
}

HI_VOID sc2235_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            if (SENSOR_1080P_30FPS_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = VMAX_1080P30_LINEAR;
            }
            genSensorMode = WDR_MODE_NONE;
            printf("linear mode\n");
        break;
        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    return;
}

HI_U32 sc2235_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{

#if CMOS_SC2235_ISP_WRITE_SENSOR_ENABLE

    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;		
        g_stSnsRegsInfo.u32RegNum = 19;
	
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {	
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }		
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3e01;     //exp high  bit[7:0] 
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3e02;     //exp low  bit[7:4] 

	g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
	g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3e07; 

        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3e08; //digita agin[6:5];    coarse analog again[4:2]
	g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
	g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x3e09; //fine analog again[4:0]

	g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
	g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x320e;     //TIMING_VTS  high bit[7:0] 
	g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
	g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x320f;     //TIMING_VTS  low bit[7:0] 

	g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 0;		 //aec/agc timing 
	g_stSnsRegsInfo.astI2cData[7].u32RegAddr = 0x3903;
	g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[8].u32RegAddr = 0x3903;
	g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[9].u32RegAddr = 0x3812;
	g_stSnsRegsInfo.astI2cData[10].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[10].u32RegAddr = 0x3301;
	g_stSnsRegsInfo.astI2cData[11].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[11].u32RegAddr = 0x3631;
	g_stSnsRegsInfo.astI2cData[12].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[12].u32RegAddr = 0x366f;
	g_stSnsRegsInfo.astI2cData[13].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[13].u32RegAddr = 0x3314;

	g_stSnsRegsInfo.astI2cData[14].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[14].u32RegAddr = 0x3622;

	g_stSnsRegsInfo.astI2cData[15].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[15].u32RegAddr = 0x3812;
	
	g_stSnsRegsInfo.astI2cData[16].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[16].u32RegAddr = 0x5781;

	g_stSnsRegsInfo.astI2cData[17].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[17].u32RegAddr = 0x5785;

	g_stSnsRegsInfo.astI2cData[18].u8DelayFrmNum = 0;		 
	g_stSnsRegsInfo.astI2cData[18].u32RegAddr = 0x3e00;

        bInit = HI_TRUE;
    }
    else    
    {        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)        
        {            
            if (g_stSnsRegsInfo.astI2cData[i].u32Data == g_stPreSnsRegsInfo.astI2cData[i].u32Data)            
            {                
               // printf("HI_FALSE....\n");
                g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_FALSE;
            }            
            else            
	        {
	            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            }
            if( HI_TRUE == (g_stSnsRegsInfo.astI2cData[0].bUpdate ||  g_stSnsRegsInfo.astI2cData[1].bUpdate))
			{
				g_stSnsRegsInfo.astI2cData[7].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[8].bUpdate = HI_TRUE;
			}
			if( HI_TRUE == (g_stSnsRegsInfo.astI2cData[2].bUpdate ||  g_stSnsRegsInfo.astI2cData[3].bUpdate ||  g_stSnsRegsInfo.astI2cData[4].bUpdate))
			{
				g_stSnsRegsInfo.astI2cData[7].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[8].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[9].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[10].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[11].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[12].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[13].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[14].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[15].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[16].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[17].bUpdate = HI_TRUE;
				g_stSnsRegsInfo.astI2cData[18].bUpdate = HI_TRUE;
			}
        }    
    }

    if (HI_NULL == pstSnsRegsInfo)
    {
        printf("null pointer when get sns reg info!\n");
        return -1;
    }

//printf(".%#X..%#X.\n",g_stSnsRegsInfo.astI2cData[0].u32Data,g_stPreSnsRegsInfo.astI2cData[0].u32Data);
    memcpy(pstSnsRegsInfo, &g_stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S)); 
    memcpy(&g_stPreSnsRegsInfo, &g_stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S)); 
#endif
    return 0;
}

static HI_S32 sc2235_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;

    bInit = HI_FALSE;
// printf("%s.............................%d....\n",__FUNCTION__,__LINE__);   
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if ((pstSensorImageMode->u16Width <= 1920) && (pstSensorImageMode->u16Height <= 1080))
    {
        if (WDR_MODE_NONE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = SENSOR_1080P_30FPS_MODE;
            }
            else
            {
                printf("Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n", 
                    pstSensorImageMode->u16Width, 
                    pstSensorImageMode->u16Height,
                    pstSensorImageMode->f32Fps,
                    genSensorMode);

                return -1;
            }
        }
        else
        {
            printf("Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n", 
                pstSensorImageMode->u16Width, 
                pstSensorImageMode->u16Height,
                pstSensorImageMode->f32Fps,
                genSensorMode);

            return -1;
        }
    }
    else
    {
        printf("Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n", 
            pstSensorImageMode->u16Width, 
            pstSensorImageMode->u16Height,
            pstSensorImageMode->f32Fps,
            genSensorMode);

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
//	printf("%s.............................%d....\n",__FUNCTION__,__LINE__);   

    return 0;
}

HI_VOID sc2235_global_init()
{   
    gu8SensorImageMode = SENSOR_1080P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_1080P30_LINEAR; 
    gu32FullLines = VMAX_1080P30_LINEAR;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));

    g_stSnsRegsInfo.astI2cData[0].u32Data = 0x00;	//intt
	g_stSnsRegsInfo.astI2cData[1].u32Data = 0x10;
	g_stSnsRegsInfo.astI2cData[2].u32Data = 0x00;	//gain
	g_stSnsRegsInfo.astI2cData[3].u32Data = 0x00;
	g_stSnsRegsInfo.astI2cData[4].u32Data = 0x10;
	g_stSnsRegsInfo.astI2cData[5].u32Data = 0x04;	//frame
	g_stSnsRegsInfo.astI2cData[6].u32Data = 0x65;	

	g_stSnsRegsInfo.astI2cData[7].u32Data = 0x84;
	g_stSnsRegsInfo.astI2cData[8].u32Data = 0x04;
	g_stSnsRegsInfo.astI2cData[9].u32Data = 0x00;
	g_stSnsRegsInfo.astI2cData[10].u32Data = 0x84;
	g_stSnsRegsInfo.astI2cData[11].u32Data = 0x05;
	g_stSnsRegsInfo.astI2cData[12].u32Data = 0x05;
	g_stSnsRegsInfo.astI2cData[13].u32Data = 0x02;
	g_stSnsRegsInfo.astI2cData[14].u32Data = 0x42;
	g_stSnsRegsInfo.astI2cData[15].u32Data = 0x30;
	g_stSnsRegsInfo.astI2cData[16].u32Data = 0x04;
	g_stSnsRegsInfo.astI2cData[17].u32Data = 0x18;
	g_stSnsRegsInfo.astI2cData[18].u32Data = 0x0;
}


static void sc2235_reg_init(void)
{	
	
	SENSOR_I2C_WRITE(0x0103,0x01);
	SENSOR_I2C_WRITE(0x0100,0x00);

	SENSOR_I2C_WRITE(0x3621,0x28);

	SENSOR_I2C_WRITE(0x3309,0x60);
	SENSOR_I2C_WRITE(0x331f,0x4d);
	SENSOR_I2C_WRITE(0x3321,0x4f);
	SENSOR_I2C_WRITE(0x33b5,0x10);

	SENSOR_I2C_WRITE(0x3303,0x20);
	SENSOR_I2C_WRITE(0x331e,0xd);
	SENSOR_I2C_WRITE(0x3320,0xf);

	SENSOR_I2C_WRITE(0x3622,0x02);
	SENSOR_I2C_WRITE(0x3633,0x42);
	SENSOR_I2C_WRITE(0x3634,0x42);

	SENSOR_I2C_WRITE(0x3306,0x66);
	SENSOR_I2C_WRITE(0x330b,0xd1);

	SENSOR_I2C_WRITE(0x3301,0x0e);

	SENSOR_I2C_WRITE(0x320c,0x0a);
	SENSOR_I2C_WRITE(0x320d,0x50);

	SENSOR_I2C_WRITE(0x3364,0x05);// [2] 1: write at sampling ending

	SENSOR_I2C_WRITE(0x363c,0x28); //bypass nvdd
	SENSOR_I2C_WRITE(0x363b,0x0a); //HVDD
	SENSOR_I2C_WRITE(0x3635,0xa0); //TXVDD

	SENSOR_I2C_WRITE(0x4500,0x59);
	SENSOR_I2C_WRITE(0x3d08,0x00);
	SENSOR_I2C_WRITE(0x3908,0x11);

	SENSOR_I2C_WRITE(0x363c,0x08);    
                        
	SENSOR_I2C_WRITE(0x3e03,0x03);
	SENSOR_I2C_WRITE(0x3e01,0x46);
	//SENSOR_I2C_WRITE(0x3e08,0x7f);
	//SENSOR_I2C_WRITE(0x3e09,0x1f);
	//SENSOR_I2C_WRITE(0x5000,0x00);
	//SENSOR_I2C_WRITE(0x3908,0x31);
	//0703
	SENSOR_I2C_WRITE(0x3381,0x0a);
	SENSOR_I2C_WRITE(0x3348,0x09);
	SENSOR_I2C_WRITE(0x3349,0x50);
	SENSOR_I2C_WRITE(0x334a,0x02);
	SENSOR_I2C_WRITE(0x334b,0x60);  
                          
	SENSOR_I2C_WRITE(0x3380,0x04);
	SENSOR_I2C_WRITE(0x3340,0x06);
	SENSOR_I2C_WRITE(0x3341,0x50);
	SENSOR_I2C_WRITE(0x3342,0x02);
	SENSOR_I2C_WRITE(0x3343,0x60);
	//0707
	SENSOR_I2C_WRITE(0x3632,0x88); //anti sm
	SENSOR_I2C_WRITE(0x3309,0xa0);
	SENSOR_I2C_WRITE(0x331f,0x8d);
	SENSOR_I2C_WRITE(0x3321,0x8f);

	SENSOR_I2C_WRITE(0x335e,0x01);  //ana dithering
	SENSOR_I2C_WRITE(0x335f,0x03);
	SENSOR_I2C_WRITE(0x337c,0x04);
	SENSOR_I2C_WRITE(0x337d,0x06);
	SENSOR_I2C_WRITE(0x33a0,0x05);
	SENSOR_I2C_WRITE(0x3301,0x05);
	//atuo logic
	SENSOR_I2C_WRITE(0x3670,0x08); //[3]:3633 logic ctrl  real value in 3682    0x00

	SENSOR_I2C_WRITE(0x367e,0x07); //gain0
	SENSOR_I2C_WRITE(0x367f,0x0f); //gain1
	SENSOR_I2C_WRITE(0x3677,0x3f); //<gain0 20171123
	SENSOR_I2C_WRITE(0x3678,0x22); //gain0 - gain1
	SENSOR_I2C_WRITE(0x3679,0x43); //>gain1

	SENSOR_I2C_WRITE(0x337f,0x03);//new auto precharge  330e in 3372   [7:6] 11: close div_rst 00:open div_rst
	SENSOR_I2C_WRITE(0x3368,0x02);
	SENSOR_I2C_WRITE(0x3369,0x00);
	SENSOR_I2C_WRITE(0x336a,0x00);
	SENSOR_I2C_WRITE(0x336b,0x00);
	SENSOR_I2C_WRITE(0x3367,0x08);
	SENSOR_I2C_WRITE(0x330e,0x30);
	
	SENSOR_I2C_WRITE(0x3366,0x7c);// div_rst gap

	SENSOR_I2C_WRITE(0x3635,0xc1);
	SENSOR_I2C_WRITE(0x363b,0x09);
	SENSOR_I2C_WRITE(0x363c,0x07);

	SENSOR_I2C_WRITE(0x391e,0x00);  
                         
	SENSOR_I2C_WRITE(0x3637,0x14); //fullwell 7K       
	
	SENSOR_I2C_WRITE(0x3306,0x54);
	SENSOR_I2C_WRITE(0x330b,0xd8);
	SENSOR_I2C_WRITE(0x366e,0x08);  // ofs auto en [3]
	SENSOR_I2C_WRITE(0x366f,0x2f);  // ofs+finegain  real ofs in 0x3687[4:0]

	SENSOR_I2C_WRITE(0x3631,0x84);
	SENSOR_I2C_WRITE(0x3630,0x48);
	SENSOR_I2C_WRITE(0x3622,0x06);
	//ramp by sc
	SENSOR_I2C_WRITE(0x3638,0x1f);
	SENSOR_I2C_WRITE(0x3625,0x02);
	SENSOR_I2C_WRITE(0x3636,0x24);
	//0714 
	SENSOR_I2C_WRITE(0x3348,0x08);
	SENSOR_I2C_WRITE(0x3e03,0x0b);
	//7.17 fpn 
	SENSOR_I2C_WRITE(0x3342,0x03);  //0x04
	SENSOR_I2C_WRITE(0x3343,0xa0);
	SENSOR_I2C_WRITE(0x334a,0x03);  //0x04
	SENSOR_I2C_WRITE(0x334b,0xa0);
	//0718   
	SENSOR_I2C_WRITE(0x3343,0xb0);
	SENSOR_I2C_WRITE(0x334b,0xb0);
	//0720
	//digital ctrl
	SENSOR_I2C_WRITE(0x3802,0x01);//0x01
	SENSOR_I2C_WRITE(0x3235,0x04);
	SENSOR_I2C_WRITE(0x3236,0x63); // vts-2
	//fpn  
	SENSOR_I2C_WRITE(0x3343,0xd0); //0x20
	SENSOR_I2C_WRITE(0x334b,0xd0); //0x20
	SENSOR_I2C_WRITE(0x3348,0x07);
	SENSOR_I2C_WRITE(0x3349,0x80);
	//0724   
	SENSOR_I2C_WRITE(0x391b,0x4d);
	//0804  
	SENSOR_I2C_WRITE(0x3222,0x29);
	SENSOR_I2C_WRITE(0x3901,0x02);  //0x02
	//0808
	SENSOR_I2C_WRITE(0x3f00,0x07);  // bit[2] = 1
	SENSOR_I2C_WRITE(0x3f04,0x0a);
	SENSOR_I2C_WRITE(0x3f05,0x2c);  // hts - 0x24
	//0809
	SENSOR_I2C_WRITE(0x330b,0xc8);

	SENSOR_I2C_WRITE(0x3342,0x04);
	SENSOR_I2C_WRITE(0x3343,0x20);
	SENSOR_I2C_WRITE(0x334a,0x04);
	SENSOR_I2C_WRITE(0x334b,0x20);
	//0817
	SENSOR_I2C_WRITE(0x3306,0x4a);
	SENSOR_I2C_WRITE(0x330b,0xca);
	SENSOR_I2C_WRITE(0x3639,0x09);

	//manual DPC
	SENSOR_I2C_WRITE(0x5780,0xff);
	SENSOR_I2C_WRITE(0x5781,0x04);
	SENSOR_I2C_WRITE(0x5785,0x18);

	SENSOR_I2C_WRITE(0x3313,0x05);
	SENSOR_I2C_WRITE(0x3678,0x42);
	SENSOR_I2C_WRITE(0x3802,0x00);
//0922
	SENSOR_I2C_WRITE(0x330b,0xcb);  // 20171123
	SENSOR_I2C_WRITE(0x3306,0x44);  // 20171123

	SENSOR_I2C_WRITE(0x3e01,0x00);
	SENSOR_I2C_WRITE(0x3e02,0x10);
	SENSOR_I2C_WRITE(0x3e03,0x0b);
	SENSOR_I2C_WRITE(0x3e07,0x00);
	SENSOR_I2C_WRITE(0x3e08,0x03);
	SENSOR_I2C_WRITE(0x3e09,0x10);

    SENSOR_I2C_WRITE(0x3237,0x05);
    SENSOR_I2C_WRITE(0x3238,0x00);

	SENSOR_I2C_WRITE(0x0100,0x01);
	
		bSensorInit = HI_TRUE;
		printf("==================================================\n");
		printf("===SC2235 sensor 1080P30fps reg init success!=====\n");
		printf("==================================================\n");

}


HI_S32 sc2235_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = sc2235_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sc2235_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = sc2235_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = sc2235_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = sc2235_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = sc2235_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = sc2235_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = sc2235_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int sc2235_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    sc2235_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, SC2235_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    sc2235_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, SC2235_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    sc2235_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, SC2235_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int sc2235_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, SC2235_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, SC2235_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, SC2235_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

 static bool sensor_mirror = false;
 static bool sensor_flip = false;
 
int SC2235_set_mirror(bool mirror){

	sensor_mirror = mirror;
	if(sensor_mirror == false && sensor_flip == false){//normal
		SENSOR_I2C_WRITE(0x3221,0x00);
	}else if(sensor_mirror == true && sensor_flip == false){//mirror

		SENSOR_I2C_WRITE(0x3221,0x06);

	}else if(sensor_mirror == false && sensor_flip == true){//flip

		SENSOR_I2C_WRITE(0x3221,0x60);

	}else if(sensor_mirror == true && sensor_flip == true){//rotate

		SENSOR_I2C_WRITE(0x3221,0x66);

	}else{		

		SENSOR_I2C_WRITE(0x3221,0x00);
	}
/**/
	return 0;

}

int SC2235_set_flip(bool flip)
{

	sensor_flip = flip;
	if(sensor_mirror == false && sensor_flip == false){//normal

		SENSOR_I2C_WRITE(0x3221,0x00);

	}else if(sensor_mirror == true && sensor_flip == false){//mirror

		SENSOR_I2C_WRITE(0x3221,0x06);

	}else if(sensor_mirror == false && sensor_flip == true){//flip

		SENSOR_I2C_WRITE(0x3221,0x60);

	}else if(sensor_mirror == true && sensor_flip == true){//rotate

		SENSOR_I2C_WRITE(0x3221,0x66);

	}else{

		SENSOR_I2C_WRITE(0x3221,0x00);

	}

/**/
	return 0;

}


int SmartSens_SC2235_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister

{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	sc2235_register_callback();
//	af_register_callback(pAfRegister);

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
	printf("SmartSens SC2235 sensor 1080P30fps init success!\n");
		
}


	
int SC2235_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
	if(ret_width && ret_height){
		*ret_width = SENSOR_SC2235_WIDTH;
		*ret_height = SENSOR_SC2235_HEIGHT;
		return 0;
	}
	return -1;
}

bool SENSOR_SC2235_probe()
{
	uint16_t ret_data1 = 0;
	uint16_t ret_data2 = 0;

	SENSOR_I2C_READ(0x3107, &ret_data1);
	SENSOR_I2C_READ(0x3108, &ret_data2);
	
	if(ret_data1 == SC2235_CHECK_DATA_LSB && ret_data2 == SC2235_CHECK_DATA_MSB){							
		//sensor power down GPIO7_4  --- high
		sdk_sys->write_reg(0x200f00f0, 0x1); 
		sdk_sys->write_reg(0x201b0400, 0x90); 
		sdk_sys->write_reg(0x201b0040, 0x00); 
		usleep(100000) ;
		sdk_sys->write_reg(0x201b0040, 0x10); 
		
		
		//set sensor CLK		
		sdk_sys->write_reg(0x200f0040, 0x2); 
		sdk_sys->write_reg(0x200f0044, 0x2);
		
		sdk_sys->write_reg(0x2003002c, 0xb4001);	 // sensor unreset, clk 27MHz, VI 99MHz

		return true;
	}
	return false;
}


int SC2235_get_sensor_name(char *sensor_name)
{
	if(sensor_name != NULL) {
		memcpy(sensor_name,SENSOR_NAME,strlen(SENSOR_NAME));
		return 0;
	}
	return -1;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __SC2235_CMOS_H_ */



