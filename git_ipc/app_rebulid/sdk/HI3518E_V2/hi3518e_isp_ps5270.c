#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "ps5270"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x90, 1, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x90, 1, 1))

#define SENSOR_PS5270_WIDTH  1536
#define SENSOR_PS5270_HEIGHT 1536
#define PS5270_CHECK_DATA_LSB	(0x52) //0x0
#define PS5270_CHECK_DATA_MSB	(0x70)//0x1 CHIP_ID address is 0x3107&0x3108
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__PS5270_CMOS_H_)
#define __PS5270_CMOS_H_

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

#define PS5270_ID 5270

#define CMOS_PS5270_ISP_WRITE_SENSOR_ENABLE (1)


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

static const unsigned int sensor_i2c_addr=0x90;
static unsigned int sensor_addr_byte=1;
static unsigned int sensor_data_byte=1;

#define SENSOR_1080P_30FPS_MODE  (1)

#define INCREASE_LINES (1) /* make real fps less than stand fps because NVR require*/
#define VMAX_1080P30_LINEAR     (0x617+INCREASE_LINES)
#define CMOS_OV3035_SLOW_FRAMERATE_MODE (0)

#define FPS_MAX		(20)


#define FULL_LINES_MAX  (0xFFFF)

#define EXPOSURE_ADDR_H		(0xc)
#define EXPOSURE_ADDR_L		(0xd)
#define AGC_SGHD_ADDR		(0x18)
#define AGC_SGHD_PATCH_ADDR	(0x97)
#define AGC_ADDR			(0x83)
#define VMAX_ADDR_H			(0xA)
#define VMAX_ADDR_L			(0xB)
#define BANK_REG			(0xEF)
#define BANK1_UDPDATE		(0x9)
#define PS5270_MIN_INT		(3)


static HI_U8 gu8SensorImageMode = SENSOR_1080P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = VMAX_1080P30_LINEAR;
static HI_U32 gu32FullLines = VMAX_1080P30_LINEAR;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};
static HI_U8 gu8Fps = 30;


/* AE default parameter and function */
static HI_S32 ps5270_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*FPS_MAX/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
	pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;

    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;

    pstAeSnsDft->u8AeCompensation = 0x38;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0.8045;

    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32MinIntTime = 3;
//    pstAeSnsDft->u32MaxIntTimeTarget = pstAeSnsDft->u32MaxIntTime;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 3;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.38;
    pstAeSnsDft->u32MaxAgain = 32382;  //62��
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1.0/256;//invalidate
    pstAeSnsDft->u32MaxDgain = 1024;
    pstAeSnsDft->u32MinDgain = 1024;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

/* the function of sensor set fps */
static HI_VOID ps5270_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

    HI_U32 u32VblankingLines = 0xFFFF;
    if ((f32Fps <= 30) && (f32Fps >= 0.5))
    {
        if(SENSOR_1080P_30FPS_MODE == gu8SensorImageMode)
        {
            gu32FullLinesStd = VMAX_1080P30_LINEAR * FPS_MAX / f32Fps;
        }
    }
    else
    {
        printf("Not support Fps: %f\n", f32Fps);
        return;
    }


    gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;

    g_stSnsRegsInfo.astI2cData[5].u32Data = (((gu32FullLinesStd-1) & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[6].u32Data = ((gu32FullLinesStd-1) & 0xFF);

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    gu32FullLines = gu32FullLinesStd;

    pstAeSnsDft->u32FullLines = gu32FullLines;

    return;
}


static HI_VOID ps5270_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;
    pstAeSnsDft->u32FullLines = gu32FullLines;

    g_stSnsRegsInfo.astI2cData[5].u32Data = (((gu32FullLines-1) & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[6].u32Data = ((gu32FullLines-1) & 0xFF);

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 2;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_U32 u32OldIntTime = 0;
static HI_VOID ps5270_inttime_update(HI_U32 u32IntTime)
{

HI_S32	u32IntTimeTmp;
	//printf("cmos_inttime_update! u32IntTime=%d\n", u32IntTime);

u32IntTimeTmp = (u32IntTime < PS5270_MIN_INT) ? PS5270_MIN_INT:u32IntTime ;
//u32IntTimeTmp = VMAX_1536x1536_HDR - u32IntTimeTmp ;
u32IntTimeTmp = gu32FullLinesStd - u32IntTimeTmp ;//20160908 modify
//printf("################# not u32IntTimeTmp=%d\n", u32IntTimeTmp);
if(u32IntTimeTmp < 4)	u32IntTimeTmp = 4;

{
	g_stSnsRegsInfo.astI2cData[1].u32Data = ((u32IntTimeTmp & 0xFF00) >> 8);
	g_stSnsRegsInfo.astI2cData[2].u32Data = (u32IntTimeTmp & 0xFF);
}


    return;
}

#define AD_MIN_GAIN_INDEX	0	//Min. Gain mapping to 1.414X * 1024 = 1448
#define AD_GAIN_TBL_NUM	(225)

static HI_U32 ad_gain_table[AD_GAIN_TBL_NUM]=
{
	1024, 1046, 1069, 1093, 1117, 1141, 1166, 1192, 1218, 1244, 1272, 1299, 1328, 1357, 1387, 1417,
	1448, 1480, 1512, 1545, 1579, 1614, 1649, 1685, 1723, 1760, 1799, 1838, 1878, 1919, 1961, 2004,
	2048, 2093, 2139, 2186, 2233, 2282, 2333, 2383, 2436, 2489, 2544, 2599, 2656, 2715, 2774, 2834,
	2897, 2960, 3024, 3091, 3158, 3226, 3297, 3372, 3444, 3519, 3597, 3676, 3755, 3837, 3924, 4010,
	4096, 4185, 4277, 4371, 4466, 4563, 4664, 4766, 4871, 4976, 5087, 5197, 5312, 5427, 5546, 5667,
	5793, 5919, 6048, 6181, 6316, 6455, 6597, 6740, 6890, 7040, 7194, 7351, 7513, 7674, 7843, 8015,
	8192, 8371, 8555, 8742, 8933, 9127, 9331, 9532, 9742, 9956, 10174, 10394, 10625, 10859, 11096, 11335,
	11586, 11839, 12096, 12363, 12633, 12905, 13189, 13486, 13774, 14074, 14388, 14703, 15019, 15349, 15694,
	16039, 16384, 16743, 17102, 17476, 17867, 18255, 18662, 19065, 19485, 19901, 20336, 20789, 21236, 21704,
	22192, 22671, 23172, 23663, 24209, 24708, 25266, 25811, 26379, 26973, 27548, 28149, 28777, 29382, 30066,
	30727, 31359, 32078, 32768, 33487, 34239, 34952, 35696, 36792, 37282, 38130, 38926, 39850, 40721, 41630,
	42473, 43464, 44384, 45343, 46345, 47393, 48349, 49490, 50533, 51622, 52758, 53946, 55188, 56299, 57456,
	58867, 60133, 61455, 62836, 64035, 65536, 66841, 68478, 69905, 71392, 72944, 74565, 76260, 78033, 79512,
	81442, 83055, 85163, 86928, 88768, 90687, 92691, 94786, 96978, 98689, 101067, 103563, 105517, 107546, 110376,
	112598, 114912, 117323, 119837, 122461, 125203, 128070, 131072,
};

static HI_VOID ps5270_cmos_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if((HI_NULL == pu32AgainLin) ||(HI_NULL == pu32AgainDb))
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

    if (*pu32AgainLin >= ad_gain_table[AD_GAIN_TBL_NUM-1])
    {
         *pu32AgainLin = ad_gain_table[AD_GAIN_TBL_NUM-1];
         *pu32AgainDb = AD_GAIN_TBL_NUM-1;
         return ;
    }
    else if (*pu32AgainLin <= ad_gain_table[AD_MIN_GAIN_INDEX])
    {
         *pu32AgainLin = ad_gain_table[AD_MIN_GAIN_INDEX];
         *pu32AgainDb = AD_MIN_GAIN_INDEX;
         return ;
    }

    for (i = 1; i < AD_GAIN_TBL_NUM; i++)
    {
        if (*pu32AgainLin < ad_gain_table[i])
        {
            *pu32AgainLin = ad_gain_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }

    return;
}

#define AG_BASE		(4)		// Ratio
#define AG_HS_NODE	(6*1024)	// 6.0x
#define AG_LS_NODE	(5*1024)	// 5.0x


static HI_VOID ps5270_Again_limit(HI_U32 *pu32Again )
{
		//�������ģ�������߼\EF\BF?
		HI_U32 u32CurrVal = 0;
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

static HI_VOID ps5270_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
	//HI_U32 tmp = 0;

	if(ad_gain_table[u32Again] > AG_HS_NODE)
	{
		g_stSnsRegsInfo.astI2cData[3].u32Data = 0;	// HS Mode
	}
	else if(ad_gain_table[u32Again] < AG_LS_NODE)
	{
		g_stSnsRegsInfo.astI2cData[3].u32Data = 1;
	}

	if(g_stSnsRegsInfo.astI2cData[3].u32Data == 0)	u32Again -= 64;
    g_stSnsRegsInfo.astI2cData[4].u32Data = (u32Again & 0xFF);

    return;
}


HI_S32 ps5270_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ps5270_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ps5270_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ps5270_slow_framerate_set;
    pstExpFuncs->pfn_cmos_inttime_update    = ps5270_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ps5270_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ps5270_cmos_again_calc_table;//cmos_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = NULL;//cmos_dgain_calc_table;

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

static HI_S32 ps5270_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
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

HI_S32 ps5270_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ps5270_get_awb_default;

    return 0;
}

#define DMNR_CALIB_CARVE_NUM_PS5270 9

float g_coef_calib_ps5270[DMNR_CALIB_CARVE_NUM_PS5270][4] =
{
    {100.000000f, 2.000000f, 0.045427f, 7.218652f, },
    {200.000000f, 2.301030f, 0.045937f, 7.593602f, },
    {401.000000f, 2.603144f, 0.048095f, 8.075705f, },
    {800.000000f, 2.903090f, 0.050832f, 9.041181f, },
    {1622.000000f, 3.210051f, 0.057881f, 10.508446f, },
    {3317.000000f, 3.520746f, 0.067995f, 13.242065f, },
    {7466.000000f, 3.873088f, 0.087098f, 18.637381f, },
    {16821.000000f, 4.225852f, 0.117693f, 28.855566f, },
    {44916.000000f, 4.652401f, 0.204691f, 45.572727f, },
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
	24,/*u16VhLimit*/
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
	/*��ֵ�˲��л���UVNR��ISO��ֵ*/
	/*UVNR�л�����ֵ�˲���ISO��ֵ*/
    /*0.0   -> disable��(0.0, 1.0]  -> weak��(1.0, 2.0]  -> normal��(2.0, 10.0) -> strong*/
	/*��˹�˲����ı�׼��*/
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
HI_U32 ps5270_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

	memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
	memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
	memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
	memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));
	//  pstDef->stNoiseTbl.u8SensorIndex = HI_ISP_NR_SENSOR_INDEX_PS5270;
	pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_PS5270;
	pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_ps5270;

	memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);
	memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
	memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));

	pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_PS5270_WIDTH;
	pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_PS5270_HEIGHT;

    return 0;
}

HI_U32 ps5270_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
   // HI_S32  i;

    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
    pstBlackLevel->au16BlackLevel[0] = 0;
    pstBlackLevel->au16BlackLevel[1] = 0;
    pstBlackLevel->au16BlackLevel[2] = 0;
    pstBlackLevel->au16BlackLevel[3] = 0;

    return 0;
}

HI_VOID ps5270_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps = 0;
    //HI_U32 u32MaxIntTime_5Fps = 0;
    if ((WDR_MODE_NONE == genSensorMode) && (SENSOR_1080P_30FPS_MODE == gu8SensorImageMode))
    {
        u32FullLines_5Fps = (VMAX_1080P30_LINEAR * FPS_MAX) / 5;
    }
    else
    {
        return;
    }

    u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
    //u32MaxIntTime_5Fps = u32FullLines_5Fps - 2;

    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR_H, ((u32FullLines_5Fps-1) & 0xFF00) >> 8);  /* 5fps */
        SENSOR_I2C_WRITE(VMAX_ADDR_L, (u32FullLines_5Fps-1) & 0xFF);           /* 5fps */
        SENSOR_I2C_WRITE(EXPOSURE_ADDR_H, (PS5270_MIN_INT & 0xFF00) >> 8);      /* max exposure lines, int_time=VMAX-config.*/
        SENSOR_I2C_WRITE(EXPOSURE_ADDR_L, PS5270_MIN_INT & 0xFF);               /* max exposure lines */
        SENSOR_I2C_WRITE(AGC_ADDR, 0x08);                                    /* min AG */
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR_H, ((gu32FullLinesStd-1) & 0xFF00) >> 8);
        SENSOR_I2C_WRITE(VMAX_ADDR_L, (gu32FullLinesStd-1) & 0xFF);

        bInit = HI_FALSE;
    }


    return;
}

HI_VOID ps5270_set_wdr_mode(HI_U8 u8Mode)
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

int g_sns_ver = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

HI_S32 ps5270_get_sns_patch(HI_VOID)
{
    static int count = 0;
    HI_U32 gain_ls, sen_lpf, sen_ny, sen_offset;
    HI_U32 temp_reg, temp_reg2;
    ISP_EXPOSURE_ATTR_S stExposureAttr;
    HI_S32 s32Ret = HI_SUCCESS;

	if(g_sns_ver != 0)	return HI_SUCCESS;

    s32Ret = HI_MPI_ISP_GetExposureAttr(0, &stExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetExposureAttr failed\n");
        return HI_FAILURE;
    }
	else if(stExposureAttr.bByPass)
	{
        //printf("AE Bypass!!\n");
		return HI_SUCCESS;
	}

    count++;
    if(60 <= count)
	{
		int FUNC_table[9] = { 0, 15, 32, 48, 68, 97, 123, 160, 214 };
		int val, FUNC_val, index, ratio;

		pthread_mutex_lock(&lock);
        SENSOR_I2C_WRITE(0xEF, 0x02);
        SENSOR_I2C_READ(0xC0, &temp_reg);
        SENSOR_I2C_WRITE(0xC1, &temp_reg2);
		//pthread_mutex_unlock(&lock);

        temp_reg = ((temp_reg & 0x7) << 8) | (temp_reg2 & 0xFF);
        sen_lpf = (g_stSnsRegsInfo.astI2cData[5].u32Data << 8) | (g_stSnsRegsInfo.astI2cData[6].u32Data & 0xFF);
        sen_ny = (g_stSnsRegsInfo.astI2cData[1].u32Data << 8) | (g_stSnsRegsInfo.astI2cData[2].u32Data & 0xFF);
		gain_ls = g_stSnsRegsInfo.astI2cData[3].u32Data&0x1;

        if (temp_reg <= 128)
		{
			FUNC_val = 0;
		}
		else
		{
			val = temp_reg - 128;
			index = val >> 7;
			if(index > 7)
			{
				FUNC_table[8] = (FUNC_table[7]-FUNC_table[6])*(index-7)+FUNC_table[7];
				index = 7;
			}
			ratio = val & 0x7F;
			FUNC_val = (((128 - ratio) * FUNC_table[index]) + (ratio * FUNC_table[index+1])) >> 4;
		}

		sen_offset = (((FUNC_val * (sen_lpf - sen_ny))+(sen_lpf/2)) / sen_lpf) >> ((gain_ls == 1) ? 5 : 3);       

		//pthread_mutex_lock(&lock);
        SENSOR_I2C_WRITE(0xEF, 0x00);
        SENSOR_I2C_WRITE(0x93, 0x10+(sen_offset>>8));
        SENSOR_I2C_WRITE(0x94, (sen_offset&0xFF));

        SENSOR_I2C_WRITE(0xEF, 0x01);	// Bank1 for AE
		pthread_mutex_unlock(&lock);

		//printf("temp_reg = %d, sen_lpf = %d, sen_ny = %d, gain_ls = %d, sen_offset = %d\n", temp_reg, sen_lpf, sen_ny, gain_ls, sen_offset);

        count=0;

    }
    return HI_SUCCESS;
}


HI_U32 ps5270_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{

#if CMOS_PS5270_ISP_WRITE_SENSOR_ENABLE

    HI_S32 i;
	HI_BOOL Updateflg = HI_FALSE;

	ps5270_get_sns_patch();

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;
        g_stSnsRegsInfo.u32RegNum = 9;

        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        /* Bank 1 */
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = BANK_REG;
        g_stSnsRegsInfo.astI2cData[0].u32Data= 1;			// Bank 1

        /* Shutter (Shutter Long) */
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = EXPOSURE_ADDR_H;
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = EXPOSURE_ADDR_L;

        /* AG */
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = AGC_SGHD_ADDR;
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = AGC_ADDR;

        /* VMAX */
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = VMAX_ADDR_H;
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = VMAX_ADDR_L;

		/* SGHD Patch */
        g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[7].u32RegAddr = AGC_SGHD_PATCH_ADDR;

		/* Bank1 Update */
        g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[8].u32RegAddr = BANK1_UDPDATE;
        g_stSnsRegsInfo.astI2cData[8].u32Data= 1;

        bInit = HI_TRUE;
    }
    else
    {
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            if (g_stSnsRegsInfo.astI2cData[i].u32Data == g_stPreSnsRegsInfo.astI2cData[i].u32Data)
            {
				if(i == 3)
				{
					g_stSnsRegsInfo.astI2cData[7].u32Data= 1;
				}

                g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_FALSE;
            }
            else
	        {
				if(i == 3)
				{
					g_stSnsRegsInfo.astI2cData[7].u32Data= (g_sns_ver != 0)?1:0;
				}

                g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
				Updateflg = HI_TRUE;
            }
        }
        g_stSnsRegsInfo.astI2cData[0].bUpdate = Updateflg;
        g_stSnsRegsInfo.astI2cData[8].bUpdate = Updateflg;

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

static HI_S32 ps5270_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;

    bInit = HI_FALSE;
// printf("%s.............................%d....\n",__FUNCTION__,__LINE__);
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if ((pstSensorImageMode->u16Width <= 1536) && (pstSensorImageMode->u16Height <= 1536))
    {
        if (WDR_MODE_NONE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= FPS_MAX)
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

HI_VOID ps5270_global_init()
{
    gu8SensorImageMode = SENSOR_1080P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_1080P30_LINEAR;
    gu32FullLines = VMAX_1080P30_LINEAR;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE;

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
}


static void ps5270_reg_init(void)
{
	//1536*1536 -12fps
	SENSOR_DELAY_MS(10);

	// PNS308_1536x1536_ImgSyn_156M_20fps_FDCoff_1002_RAW14_600Mbps_A02a.asc
	SENSOR_I2C_WRITE (0xEF, 0x05);
	SENSOR_I2C_WRITE (0x0F, 0x00);
	SENSOR_I2C_WRITE (0x42, 0x00);
	SENSOR_I2C_WRITE (0x43, 0x06);
	SENSOR_I2C_WRITE (0xED, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x01);
	SENSOR_I2C_WRITE (0xF5, 0x01);
	SENSOR_I2C_WRITE (0x09, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x00);
	SENSOR_I2C_WRITE (0x06, 0x02);
	SENSOR_I2C_WRITE (0x0B, 0x00);
	SENSOR_I2C_WRITE (0x0C, 0xA0);
	SENSOR_I2C_WRITE (0x10, 0x20);//Cmd_BYTECLK_InvSel=0 for PS308
	SENSOR_I2C_WRITE (0x11, 0x80);//GatedAllClk enable
	SENSOR_I2C_WRITE (0x12, 0x80);
	SENSOR_I2C_WRITE (0x13, 0x00);
	SENSOR_I2C_WRITE (0x14, 0xBF);
	SENSOR_I2C_WRITE (0x15, 0x07);
	SENSOR_I2C_WRITE (0x16, 0xBF);
	SENSOR_I2C_WRITE (0x17, 0xBF);
	SENSOR_I2C_WRITE (0x18, 0xBF);
	SENSOR_I2C_WRITE (0x19, 0x64);
	SENSOR_I2C_WRITE (0x1B, 0x64);
	SENSOR_I2C_WRITE (0x1C, 0x64);
	SENSOR_I2C_WRITE (0x1D, 0x64);
	SENSOR_I2C_WRITE (0x1E, 0x64);
	SENSOR_I2C_WRITE (0x1F, 0x64);
	SENSOR_I2C_WRITE (0x20, 0x64);
	SENSOR_I2C_WRITE (0x21, 0x00);
	SENSOR_I2C_WRITE (0x23, 0x00);
	SENSOR_I2C_WRITE (0x24, 0x00);
	SENSOR_I2C_WRITE (0x25, 0x00);
	SENSOR_I2C_WRITE (0x26, 0x00);
	SENSOR_I2C_WRITE (0x27, 0x00);
	SENSOR_I2C_WRITE (0x28, 0x00);
	SENSOR_I2C_WRITE (0x29, 0x64);
	SENSOR_I2C_WRITE (0x2B, 0x64);
	SENSOR_I2C_WRITE (0x2C, 0x64);
	SENSOR_I2C_WRITE (0x2D, 0x64);
	SENSOR_I2C_WRITE (0x2E, 0x64);
	SENSOR_I2C_WRITE (0x2F, 0x64);
	SENSOR_I2C_WRITE (0x30, 0x64);
	SENSOR_I2C_WRITE (0x31, 0x0F);
	SENSOR_I2C_WRITE (0x32, 0x00);
	SENSOR_I2C_WRITE (0x33, 0x64);
	SENSOR_I2C_WRITE (0x34, 0x64);
	SENSOR_I2C_WRITE (0x36, 0x00);
	SENSOR_I2C_WRITE (0x37, 0x0E);
	SENSOR_I2C_WRITE (0x38, 0xC8);
	SENSOR_I2C_WRITE (0x39, 0x0A);
	SENSOR_I2C_WRITE (0x3A, 0x02);
	SENSOR_I2C_WRITE (0x3B, 0x00);
	SENSOR_I2C_WRITE (0x3C, 0x07);

	SENSOR_I2C_WRITE (0x45, 0x00);
	SENSOR_I2C_WRITE (0x46, 0x00);
	SENSOR_I2C_WRITE (0x47, 0x00);
	SENSOR_I2C_WRITE (0x48, 0x00);
	SENSOR_I2C_WRITE (0x49, 0xD2);
	SENSOR_I2C_WRITE (0x4A, 0x0A);
	SENSOR_I2C_WRITE (0x4B, 0x6E);
	SENSOR_I2C_WRITE (0x4C, 0x01);
	SENSOR_I2C_WRITE (0x4D, 0xD2);
	SENSOR_I2C_WRITE (0x4E, 0xC4);
	SENSOR_I2C_WRITE (0x4F, 0x28);
	SENSOR_I2C_WRITE (0x53, 0x00);
	SENSOR_I2C_WRITE (0x54, 0x65);
	SENSOR_I2C_WRITE (0x55, 0x43);
	SENSOR_I2C_WRITE (0x56, 0x21);
	SENSOR_I2C_WRITE (0x57, 0x00);
	SENSOR_I2C_WRITE (0x58, 0x10);
	SENSOR_I2C_WRITE (0x59, 0x08);
	SENSOR_I2C_WRITE (0x5A, 0x04);
	SENSOR_I2C_WRITE (0x5B, 0x02);
	SENSOR_I2C_WRITE (0x5C, 0x01);
	SENSOR_I2C_WRITE (0x5D, 0x00);
	SENSOR_I2C_WRITE (0x5E, 0x01);
	SENSOR_I2C_WRITE (0x5F, 0x2C);
	SENSOR_I2C_WRITE (0x60, 0xC2);
	SENSOR_I2C_WRITE (0x61, 0xFF);
	SENSOR_I2C_WRITE (0x62, 0x07);
	SENSOR_I2C_WRITE (0x63, 0x46);
	SENSOR_I2C_WRITE (0x64, 0x05);
	SENSOR_I2C_WRITE (0x65, 0x82);
	SENSOR_I2C_WRITE (0x66, 0x05);
	SENSOR_I2C_WRITE (0x67, 0xBE);
	SENSOR_I2C_WRITE (0x68, 0x05);
	SENSOR_I2C_WRITE (0x69, 0x40);
	SENSOR_I2C_WRITE (0x6A, 0x80);
	SENSOR_I2C_WRITE (0x6B, 0x00);
	SENSOR_I2C_WRITE (0x6C, 0x00);
	SENSOR_I2C_WRITE (0x6D, 0x00);
	SENSOR_I2C_WRITE (0x6E, 0x80);
	SENSOR_I2C_WRITE (0x6F, 0x5A);
	SENSOR_I2C_WRITE (0x70, 0x00);
	SENSOR_I2C_WRITE (0x71, 0x2D);
	SENSOR_I2C_WRITE (0x72, 0x00);
	SENSOR_I2C_WRITE (0x74, 0x00);
	SENSOR_I2C_WRITE (0x75, 0x78);
	SENSOR_I2C_WRITE (0x76, 0x00);
	SENSOR_I2C_WRITE (0x77, 0x20);
	SENSOR_I2C_WRITE (0x7A, 0x00);
	SENSOR_I2C_WRITE (0x7B, 0x14);
	SENSOR_I2C_WRITE (0x7C, 0x96);
	SENSOR_I2C_WRITE (0x7D, 0xA0);
	SENSOR_I2C_WRITE (0x7E, 0x00);
	SENSOR_I2C_WRITE (0x7F, 0xA0);
	SENSOR_I2C_WRITE (0x80, 0x0F);
	SENSOR_I2C_WRITE (0x81, 0x6E);
	SENSOR_I2C_WRITE (0x82, 0x01);
	SENSOR_I2C_WRITE (0x83, 0x00);
	SENSOR_I2C_WRITE (0x84, 0x00);
	SENSOR_I2C_WRITE (0x85, 0x96);
	SENSOR_I2C_WRITE (0x86, 0x00);
	SENSOR_I2C_WRITE (0x87, 0x9F);
	SENSOR_I2C_WRITE (0x88, 0x09);
	SENSOR_I2C_WRITE (0x89, 0x01);
	SENSOR_I2C_WRITE (0x8B, 0x00);
	SENSOR_I2C_WRITE (0x8C, 0x00);
	SENSOR_I2C_WRITE (0x8D, 0x00);
	SENSOR_I2C_WRITE (0x8E, 0x00);
	SENSOR_I2C_WRITE (0x8F, 0x00);
	SENSOR_I2C_WRITE (0x90, 0x02);
	SENSOR_I2C_WRITE (0x91, 0x00);
	SENSOR_I2C_WRITE (0x92, 0x11);
	SENSOR_I2C_WRITE (0x93, 0x10);
	SENSOR_I2C_WRITE (0x94, 0x00);
	SENSOR_I2C_WRITE (0x95, 0x00);
	SENSOR_I2C_WRITE (0x96, 0x00);
	SENSOR_I2C_WRITE (0x97, 0x00);
	SENSOR_I2C_WRITE (0x99, 0x00);
	SENSOR_I2C_WRITE (0x9A, 0x00);
	SENSOR_I2C_WRITE (0x9B, 0x08);
	SENSOR_I2C_WRITE (0x9C, 0x00);
	SENSOR_I2C_WRITE (0x9D, 0x00);
	SENSOR_I2C_WRITE (0x9E, 0x82);
	SENSOR_I2C_WRITE (0x9F, 0x00);
	SENSOR_I2C_WRITE (0xA0, 0x05);
	SENSOR_I2C_WRITE (0xA1, 0x00);
	SENSOR_I2C_WRITE (0xA2, 0x0A);
	SENSOR_I2C_WRITE (0xA3, 0x07);
	SENSOR_I2C_WRITE (0xA4, 0xFF);
	SENSOR_I2C_WRITE (0xA5, 0x03);
	SENSOR_I2C_WRITE (0xA6, 0xFF);
	SENSOR_I2C_WRITE (0xA7, 0x00);
	SENSOR_I2C_WRITE (0xA8, 0x00);
	SENSOR_I2C_WRITE (0xA9, 0x11);
	SENSOR_I2C_WRITE (0xAA, 0x65);
	SENSOR_I2C_WRITE (0xAB, 0x65);
	SENSOR_I2C_WRITE (0xAD, 0x00);
	SENSOR_I2C_WRITE (0xAE, 0x00);
	SENSOR_I2C_WRITE (0xAF, 0x00);
	SENSOR_I2C_WRITE (0xB0, 0x02);
	SENSOR_I2C_WRITE (0xB1, 0x00);
	SENSOR_I2C_WRITE (0xBE, 0x05);
	SENSOR_I2C_WRITE (0xBF, 0x80);
	SENSOR_I2C_WRITE (0xC0, 0x10);
	SENSOR_I2C_WRITE (0xC7, 0x10);
	SENSOR_I2C_WRITE (0xC8, 0x01);
	SENSOR_I2C_WRITE (0xC9, 0x00);
	SENSOR_I2C_WRITE (0xCA, 0x55);
	SENSOR_I2C_WRITE (0xCB, 0x06);
	SENSOR_I2C_WRITE (0xCC, 0x09);
	SENSOR_I2C_WRITE (0xCD, 0x00);
	SENSOR_I2C_WRITE (0xCE, 0xA2);
	SENSOR_I2C_WRITE (0xCF, 0x00);
	SENSOR_I2C_WRITE (0xD0, 0x02);
	SENSOR_I2C_WRITE (0xD1, 0x10);
	SENSOR_I2C_WRITE (0xD2, 0x1E);
	SENSOR_I2C_WRITE (0xD3, 0x19);
	SENSOR_I2C_WRITE (0xD4, 0x04);
	SENSOR_I2C_WRITE (0xD5, 0x18);
	SENSOR_I2C_WRITE (0xD6, 0xC8);
	SENSOR_I2C_WRITE (0xD8, 0x40);
	SENSOR_I2C_WRITE (0xD9, 0x60);
	SENSOR_I2C_WRITE (0xDA, 0x80);
	SENSOR_I2C_WRITE (0xDB, 0x02);
	SENSOR_I2C_WRITE (0xDC, 0xF8);
	SENSOR_I2C_WRITE (0xDD, 0x64);
	SENSOR_I2C_WRITE (0xDE, 0x00);
	SENSOR_I2C_WRITE (0xDF, 0x2A);
	SENSOR_I2C_WRITE (0xE0, 0x0D);
	SENSOR_I2C_WRITE (0xE1, 0x03);
	SENSOR_I2C_WRITE (0xE2, 0x00);
	SENSOR_I2C_WRITE (0xE3, 0x2A);
	SENSOR_I2C_WRITE (0xE4, 0x0D);
	SENSOR_I2C_WRITE (0xE5, 0x03);
	SENSOR_I2C_WRITE (0xE6, 0x00);
	SENSOR_I2C_WRITE (0xF0, 0x00);
	SENSOR_I2C_WRITE (0xF1, 0x00);
	SENSOR_I2C_WRITE (0xF2, 0x00);
	SENSOR_I2C_WRITE (0xF3, 0x00);
	SENSOR_I2C_WRITE (0xF4, 0x00);
	SENSOR_I2C_WRITE (0xF5, 0x40);
	SENSOR_I2C_WRITE (0xF6, 0x00);
	SENSOR_I2C_WRITE (0xF7, 0x00);
	SENSOR_I2C_WRITE (0xF8, 0x00);
	SENSOR_I2C_WRITE (0xED, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x01);
	SENSOR_I2C_WRITE (0x02, 0xFF);
	SENSOR_I2C_WRITE (0x03, 0x03);
	SENSOR_I2C_WRITE (0x04, 0x00);
	SENSOR_I2C_WRITE (0x05, 0x0B);
	SENSOR_I2C_WRITE (0x06, 0xFF);
	SENSOR_I2C_WRITE (0x07, 0x04);
	SENSOR_I2C_WRITE (0x08, 0x00);
	SENSOR_I2C_WRITE (0x09, 0x00);
	SENSOR_I2C_WRITE (0x0A, 0x06);
	SENSOR_I2C_WRITE (0x0B, 0x17);
	SENSOR_I2C_WRITE (0x0C, 0x00);
	SENSOR_I2C_WRITE (0x0D, 0x04);
	SENSOR_I2C_WRITE (0x0E, 0x01);
	SENSOR_I2C_WRITE (0x0F, 0x2C);
	SENSOR_I2C_WRITE (0x10, 0xA0);
	SENSOR_I2C_WRITE (0x11, 0x1A);
	SENSOR_I2C_WRITE (0x12, 0x0C);
	SENSOR_I2C_WRITE (0x13, 0xB2);
	SENSOR_I2C_WRITE (0x14, 0x01);
	SENSOR_I2C_WRITE (0x15, 0x00);
	SENSOR_I2C_WRITE (0x16, 0x00);
	SENSOR_I2C_WRITE (0x17, 0x00);
	SENSOR_I2C_WRITE (0x18, 0x00);
	SENSOR_I2C_WRITE (0x19, 0x3F);
	SENSOR_I2C_WRITE (0x1A, 0x00);
	SENSOR_I2C_WRITE (0x1B, 0x06);
	SENSOR_I2C_WRITE (0x1C, 0x08);
	SENSOR_I2C_WRITE (0x1D, 0x06);
	SENSOR_I2C_WRITE (0x1E, 0x0A);
	SENSOR_I2C_WRITE (0x1F, 0x00);
	SENSOR_I2C_WRITE (0x20, 0x02);
	SENSOR_I2C_WRITE (0x21, 0x00);
	SENSOR_I2C_WRITE (0x22, 0x00);
	SENSOR_I2C_WRITE (0x23, 0x00);
	SENSOR_I2C_WRITE (0x24, 0x20);
	SENSOR_I2C_WRITE (0x25, 0x00);
	SENSOR_I2C_WRITE (0x26, 0x08);
	SENSOR_I2C_WRITE (0x27, 0x13);
	SENSOR_I2C_WRITE (0x28, 0x88);
	SENSOR_I2C_WRITE (0x29, 0x0A);
	SENSOR_I2C_WRITE (0x2A, 0x0A);
	SENSOR_I2C_WRITE (0x2C, 0x00);
	SENSOR_I2C_WRITE (0x2D, 0x14);
	SENSOR_I2C_WRITE (0x2E, 0x64);
	SENSOR_I2C_WRITE (0x2F, 0x02);
	SENSOR_I2C_WRITE (0x31, 0x03);
	SENSOR_I2C_WRITE (0x32, 0xFF);
	SENSOR_I2C_WRITE (0x33, 0x70);
	SENSOR_I2C_WRITE (0x34, 0x01);
	SENSOR_I2C_WRITE (0x35, 0x01);
	SENSOR_I2C_WRITE (0x36, 0x00);
	SENSOR_I2C_WRITE (0x37, 0x90);
	SENSOR_I2C_WRITE (0x38, 0x04);
	SENSOR_I2C_WRITE (0x39, 0x9A);
	SENSOR_I2C_WRITE (0x3A, 0xFF);
	SENSOR_I2C_WRITE (0x3B, 0x38);
	SENSOR_I2C_WRITE (0x3D, 0x04);
	SENSOR_I2C_WRITE (0x3E, 0x20);
	SENSOR_I2C_WRITE (0x3F, 0x22);
	SENSOR_I2C_WRITE (0x40, 0xFF);
	SENSOR_I2C_WRITE (0x41, 0x13);
	SENSOR_I2C_WRITE (0x42, 0x2C);
	SENSOR_I2C_WRITE (0x43, 0x70);
	SENSOR_I2C_WRITE (0x44, 0x04);
	SENSOR_I2C_WRITE (0x47, 0x00);
	SENSOR_I2C_WRITE (0x48, 0x08);
	SENSOR_I2C_WRITE (0x49, 0x00);
	SENSOR_I2C_WRITE (0x4A, 0x08);
	SENSOR_I2C_WRITE (0x4B, 0x00);
	SENSOR_I2C_WRITE (0x4C, 0x08);
	SENSOR_I2C_WRITE (0x4D, 0x00);
	SENSOR_I2C_WRITE (0x4E, 0x08);
	SENSOR_I2C_WRITE (0x4F, 0x02);
	SENSOR_I2C_WRITE (0x50, 0x08);
	SENSOR_I2C_WRITE (0x52, 0x02);
	SENSOR_I2C_WRITE (0x53, 0x08);
	SENSOR_I2C_WRITE (0x54, 0x00);
	SENSOR_I2C_WRITE (0x55, 0x00);
	SENSOR_I2C_WRITE (0x56, 0x02);
	SENSOR_I2C_WRITE (0x57, 0x00);
	SENSOR_I2C_WRITE (0x58, 0x02);
	SENSOR_I2C_WRITE (0x59, 0x08);
	SENSOR_I2C_WRITE (0x5A, 0x02);
	SENSOR_I2C_WRITE (0x5B, 0x08);
	SENSOR_I2C_WRITE (0x5C, 0x05);
	SENSOR_I2C_WRITE (0x5D, 0x78);
	SENSOR_I2C_WRITE (0x5F, 0x00);
	SENSOR_I2C_WRITE (0x60, 0x64);
	SENSOR_I2C_WRITE (0x64, 0x03);
	SENSOR_I2C_WRITE (0x65, 0xFF);
	SENSOR_I2C_WRITE (0x66, 0x70);
	SENSOR_I2C_WRITE (0x67, 0x22);
	SENSOR_I2C_WRITE (0x68, 0xBC);
	SENSOR_I2C_WRITE (0x69, 0xBC);
	SENSOR_I2C_WRITE (0x6B, 0x08);
	SENSOR_I2C_WRITE (0x6C, 0x00);
	SENSOR_I2C_WRITE (0x6D, 0xC8);
	SENSOR_I2C_WRITE (0x6F, 0x08);
	SENSOR_I2C_WRITE (0x70, 0x00);
	SENSOR_I2C_WRITE (0x71, 0xC8);
	SENSOR_I2C_WRITE (0x72, 0x00);
	SENSOR_I2C_WRITE (0x73, 0x96);
	SENSOR_I2C_WRITE (0x74, 0x11);
	SENSOR_I2C_WRITE (0x75, 0x12);
	SENSOR_I2C_WRITE (0x76, 0x00);
	SENSOR_I2C_WRITE (0x77, 0x00);
	SENSOR_I2C_WRITE (0x78, 0x00);
	SENSOR_I2C_WRITE (0x79, 0x0F);
	SENSOR_I2C_WRITE (0x7A, 0x05);
	SENSOR_I2C_WRITE (0x7B, 0x7D);
	SENSOR_I2C_WRITE (0x7C, 0x06);
	SENSOR_I2C_WRITE (0x7D, 0x31);
	SENSOR_I2C_WRITE (0x7E, 0x04);
	SENSOR_I2C_WRITE (0x7F, 0x00);
	SENSOR_I2C_WRITE (0x80, 0x00);
	SENSOR_I2C_WRITE (0x81, 0x00);
	SENSOR_I2C_WRITE (0x82, 0x00);
	SENSOR_I2C_WRITE (0x83, 0x00);
	SENSOR_I2C_WRITE (0x87, 0x00);
	SENSOR_I2C_WRITE (0x88, 0x0A);
	SENSOR_I2C_WRITE (0x89, 0x00);
	SENSOR_I2C_WRITE (0x8A, 0x06);
	SENSOR_I2C_WRITE (0x8B, 0x0E);
	SENSOR_I2C_WRITE (0x8C, 0x00);
	SENSOR_I2C_WRITE (0x8D, 0x02);
	SENSOR_I2C_WRITE (0x8E, 0xBC);
	SENSOR_I2C_WRITE (0x8F, 0x07);
	SENSOR_I2C_WRITE (0x90, 0x01);
	SENSOR_I2C_WRITE (0x91, 0xBF);
	SENSOR_I2C_WRITE (0x92, 0x00);
	SENSOR_I2C_WRITE (0x93, 0x00);
	SENSOR_I2C_WRITE (0x94, 0xFF);
	SENSOR_I2C_WRITE (0x95, 0x00);
	SENSOR_I2C_WRITE (0x96, 0x80);
	SENSOR_I2C_WRITE (0x97, 0x01);
	SENSOR_I2C_WRITE (0x98, 0x02);
	SENSOR_I2C_WRITE (0x99, 0x0D);
	SENSOR_I2C_WRITE (0x9A, 0x14);
	SENSOR_I2C_WRITE (0x9B, 0x0A);
	SENSOR_I2C_WRITE (0x9C, 0x14);
	SENSOR_I2C_WRITE (0x9D, 0x03);
	SENSOR_I2C_WRITE (0x9E, 0xE8);
	SENSOR_I2C_WRITE (0x9F, 0x01);
	SENSOR_I2C_WRITE (0xA0, 0x00);
	SENSOR_I2C_WRITE (0xA1, 0x3E);
	SENSOR_I2C_WRITE (0xA2, 0x3E);
	SENSOR_I2C_WRITE (0xA3, 0x00);
	SENSOR_I2C_WRITE (0xA4, 0x10);
	SENSOR_I2C_WRITE (0xA5, 0x06);
	SENSOR_I2C_WRITE (0xA6, 0x00);
	SENSOR_I2C_WRITE (0xA7, 0x00);
	SENSOR_I2C_WRITE (0xA8, 0x06);
	SENSOR_I2C_WRITE (0xA9, 0x06);
	SENSOR_I2C_WRITE (0xAA, 0x00);
	SENSOR_I2C_WRITE (0xAB, 0x01);
	SENSOR_I2C_WRITE (0xAD, 0x00);
	SENSOR_I2C_WRITE (0xAE, 0x00);
	SENSOR_I2C_WRITE (0xAF, 0x00);
	SENSOR_I2C_WRITE (0xB0, 0x00);
	SENSOR_I2C_WRITE (0xB1, 0x00);
	SENSOR_I2C_WRITE (0xB2, 0x00);
	SENSOR_I2C_WRITE (0xB3, 0x00);
	SENSOR_I2C_WRITE (0xB4, 0x00);
	SENSOR_I2C_WRITE (0xB5, 0x07);
	SENSOR_I2C_WRITE (0xB6, 0x80);
	SENSOR_I2C_WRITE (0xB7, 0x82);
	SENSOR_I2C_WRITE (0xB8, 0x0A);
	SENSOR_I2C_WRITE (0xBC, 0x01);
	SENSOR_I2C_WRITE (0xBD, 0x01);
	SENSOR_I2C_WRITE (0xC9, 0x14);
	SENSOR_I2C_WRITE (0xCB, 0x0A);
	SENSOR_I2C_WRITE (0xCE, 0xF0);
	SENSOR_I2C_WRITE (0xCF, 0x80);
	SENSOR_I2C_WRITE (0xD0, 0x82);
	SENSOR_I2C_WRITE (0xD1, 0x44);
	SENSOR_I2C_WRITE (0xD3, 0x02);
	SENSOR_I2C_WRITE (0xD4, 0xA0);
	SENSOR_I2C_WRITE (0xD5, 0x69);
	SENSOR_I2C_WRITE (0xD6, 0x00);
	SENSOR_I2C_WRITE (0xD7, 0x06);
	SENSOR_I2C_WRITE (0xD8, 0x68);
	SENSOR_I2C_WRITE (0xDA, 0x70);
	SENSOR_I2C_WRITE (0xDB, 0x00);
	SENSOR_I2C_WRITE (0xDC, 0x00);
	SENSOR_I2C_WRITE (0xDD, 0x52);
	SENSOR_I2C_WRITE (0xDE, 0x42);
	SENSOR_I2C_WRITE (0xE0, 0x42);
	SENSOR_I2C_WRITE (0xE1, 0x11);
	SENSOR_I2C_WRITE (0xE2, 0x4F);
	SENSOR_I2C_WRITE (0xE3, 0x02);
	SENSOR_I2C_WRITE (0xE4, 0x00);
	SENSOR_I2C_WRITE (0xE6, 0x01);
	SENSOR_I2C_WRITE (0xE7, 0x00);
	SENSOR_I2C_WRITE (0xEA, 0xBB);
	SENSOR_I2C_WRITE (0xF5, 0x00);//SPLL disable
	SENSOR_I2C_WRITE (0xF0, 0x03);
	SENSOR_I2C_WRITE (0xF1, 0x19);
	SENSOR_I2C_WRITE (0xF2, 0x25);
	SENSOR_I2C_WRITE (0xF3, 0x00);
	SENSOR_I2C_WRITE (0xF4, 0x06);
	SENSOR_I2C_WRITE (0xF6, 0xC8);
	SENSOR_I2C_WRITE (0xF7, 0x02);
	SENSOR_I2C_WRITE (0xF5, 0x10);//SPLL enable
	SENSOR_I2C_WRITE (0xF8, 0x08);
	SENSOR_I2C_WRITE (0xF9, 0x14);
	SENSOR_I2C_WRITE (0xFA, 0x31);
	SENSOR_I2C_WRITE (0xFB, 0x02);
	SENSOR_I2C_WRITE (0xFC, 0x04);
	SENSOR_I2C_WRITE (0xFD, 0x20);
	SENSOR_I2C_WRITE (0x09, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x02);
	SENSOR_I2C_WRITE (0x10, 0x00);
	SENSOR_I2C_WRITE (0x20, 0x03);
	SENSOR_I2C_WRITE (0x21, 0x18);
	SENSOR_I2C_WRITE (0x22, 0x0C);
	SENSOR_I2C_WRITE (0x23, 0x08);
	SENSOR_I2C_WRITE (0x24, 0x05);
	SENSOR_I2C_WRITE (0x25, 0x03);
	SENSOR_I2C_WRITE (0x26, 0x02);
	SENSOR_I2C_WRITE (0x27, 0x18);
	SENSOR_I2C_WRITE (0x28, 0x0C);
	SENSOR_I2C_WRITE (0x29, 0x08);
	SENSOR_I2C_WRITE (0x2A, 0x05);
	SENSOR_I2C_WRITE (0x2B, 0x03);
	SENSOR_I2C_WRITE (0x2C, 0x02);
	SENSOR_I2C_WRITE (0x2E, 0x00);
	SENSOR_I2C_WRITE (0x30, 0xBF);
	SENSOR_I2C_WRITE (0x31, 0x06);
	SENSOR_I2C_WRITE (0x32, 0x07);
	SENSOR_I2C_WRITE (0x33, 0x85);
	SENSOR_I2C_WRITE (0x34, 0x00);
	SENSOR_I2C_WRITE (0x35, 0x00);
	SENSOR_I2C_WRITE (0x36, 0x01);
	SENSOR_I2C_WRITE (0x37, 0x00);
	SENSOR_I2C_WRITE (0x38, 0x00);
	SENSOR_I2C_WRITE (0x39, 0x00);
	SENSOR_I2C_WRITE (0x3A, 0xCE);
	SENSOR_I2C_WRITE (0x3B, 0x15);
	SENSOR_I2C_WRITE (0x3C, 0x64);
	SENSOR_I2C_WRITE (0x3D, 0x04);
	SENSOR_I2C_WRITE (0x3E, 0x00);
	SENSOR_I2C_WRITE (0x3F, 0x0A);
	SENSOR_I2C_WRITE (0x40, 0x08);
	SENSOR_I2C_WRITE (0x41, 0x09);
	SENSOR_I2C_WRITE (0x42, 0x08);
	SENSOR_I2C_WRITE (0x43, 0x09);
	SENSOR_I2C_WRITE (0x45, 0x00);
	SENSOR_I2C_WRITE (0x46, 0x00);
	SENSOR_I2C_WRITE (0x47, 0x00);
	SENSOR_I2C_WRITE (0x48, 0x00);
	SENSOR_I2C_WRITE (0x49, 0x00);
	SENSOR_I2C_WRITE (0x4A, 0x00);
	SENSOR_I2C_WRITE (0x4B, 0x00);
	SENSOR_I2C_WRITE (0x4C, 0x00);
	SENSOR_I2C_WRITE (0x4D, 0x00);
	SENSOR_I2C_WRITE (0x4E, 0x02);
	SENSOR_I2C_WRITE (0x4F, 0x05);
	SENSOR_I2C_WRITE (0x50, 0x00);
	SENSOR_I2C_WRITE (0xA0, 0x00);
	SENSOR_I2C_WRITE (0xA1, 0x00);
	SENSOR_I2C_WRITE (0xA2, 0x00);
	SENSOR_I2C_WRITE (0xA3, 0x00);
	SENSOR_I2C_WRITE (0xA4, 0x00);
	SENSOR_I2C_WRITE (0xA5, 0x00);
	SENSOR_I2C_WRITE (0xA6, 0x00);
	SENSOR_I2C_WRITE (0xA7, 0x00);
	SENSOR_I2C_WRITE (0xA9, 0x00);
	SENSOR_I2C_WRITE (0xAA, 0x00);
	SENSOR_I2C_WRITE (0xAB, 0x00);
	SENSOR_I2C_WRITE (0xB2, 0x01);
	SENSOR_I2C_WRITE (0xB3, 0x00);
	SENSOR_I2C_WRITE (0xB4, 0x03);
	SENSOR_I2C_WRITE (0xB5, 0x01);
	SENSOR_I2C_WRITE (0xB6, 0x03);
	SENSOR_I2C_WRITE (0xB7, 0x01);
	SENSOR_I2C_WRITE (0xB8, 0x03);
	SENSOR_I2C_WRITE (0xB9, 0x00);
	SENSOR_I2C_WRITE (0xBA, 0x00);
	SENSOR_I2C_WRITE (0xBB, 0x00);
	SENSOR_I2C_WRITE (0xBC, 0x00);
	SENSOR_I2C_WRITE (0xBD, 0x00);
	SENSOR_I2C_WRITE (0xBE, 0x00);
	SENSOR_I2C_WRITE (0xCB, 0xBD);
	SENSOR_I2C_WRITE (0xCC, 0x00);
	SENSOR_I2C_WRITE (0xCD, 0x00);
	SENSOR_I2C_WRITE (0xCE, 0x00);
	SENSOR_I2C_WRITE (0xCF, 0x00);
	SENSOR_I2C_WRITE (0xD0, 0x00);
	SENSOR_I2C_WRITE (0xD1, 0x00);
	SENSOR_I2C_WRITE (0xD2, 0x00);
	SENSOR_I2C_WRITE (0xD3, 0x00);
	SENSOR_I2C_WRITE (0xD4, 0x01);
	SENSOR_I2C_WRITE (0xD5, 0x00);
	SENSOR_I2C_WRITE (0xD6, 0x00);
	SENSOR_I2C_WRITE (0xD7, 0x00);
	SENSOR_I2C_WRITE (0xD8, 0x00);
	SENSOR_I2C_WRITE (0xD9, 0x00);
	SENSOR_I2C_WRITE (0xDA, 0x00);
	SENSOR_I2C_WRITE (0xDB, 0x00);
	SENSOR_I2C_WRITE (0xDC, 0x00);
	SENSOR_I2C_WRITE (0xDD, 0xF0);
	SENSOR_I2C_WRITE (0xDE, 0x04);
	SENSOR_I2C_WRITE (0xDF, 0x97);
	SENSOR_I2C_WRITE (0xE0, 0x50);
	SENSOR_I2C_WRITE (0xE1, 0x50);
	SENSOR_I2C_WRITE (0xE2, 0x14);
	SENSOR_I2C_WRITE (0xE3, 0x1B);
	SENSOR_I2C_WRITE (0xE4, 0x3F);
	SENSOR_I2C_WRITE (0xE5, 0xFF);
	SENSOR_I2C_WRITE (0xE6, 0x00);
	SENSOR_I2C_WRITE (0xF0, 0x00);
	SENSOR_I2C_WRITE (0xF1, 0x00);
	SENSOR_I2C_WRITE (0xF2, 0x00);
	SENSOR_I2C_WRITE (0xF3, 0x00);
	SENSOR_I2C_WRITE (0xF6, 0x00);
	SENSOR_I2C_WRITE (0xF7, 0x00);
	SENSOR_I2C_WRITE (0xFD, 0x18);
	SENSOR_I2C_WRITE (0xFE, 0x9E);
	SENSOR_I2C_WRITE (0xED, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x05);
	SENSOR_I2C_WRITE (0x03, 0x10);
	SENSOR_I2C_WRITE (0x04, 0x3A);
	SENSOR_I2C_WRITE (0x05, 0x04);
	SENSOR_I2C_WRITE (0x06, 0x06);
	SENSOR_I2C_WRITE (0x07, 0x80);
	SENSOR_I2C_WRITE (0x08, 0x07);
	SENSOR_I2C_WRITE (0x09, 0x09);
	SENSOR_I2C_WRITE (0x0A, 0x05);
	SENSOR_I2C_WRITE (0x0B, 0x06);
	SENSOR_I2C_WRITE (0x0C, 0x04);
	SENSOR_I2C_WRITE (0x0D, 0x5E);
	SENSOR_I2C_WRITE (0x0E, 0x01);
	SENSOR_I2C_WRITE (0x0F, 0x00);
	SENSOR_I2C_WRITE (0x10, 0x02);
	SENSOR_I2C_WRITE (0x11, 0x01);
	SENSOR_I2C_WRITE (0x12, 0x00);
	SENSOR_I2C_WRITE (0x13, 0x00);
	SENSOR_I2C_WRITE (0x14, 0xB8);
	SENSOR_I2C_WRITE (0x15, 0x07);
	SENSOR_I2C_WRITE (0x16, 0x06);
	SENSOR_I2C_WRITE (0x17, 0x05);
	SENSOR_I2C_WRITE (0x18, 0x02);
	SENSOR_I2C_WRITE (0x19, 0x04);
	SENSOR_I2C_WRITE (0x1A, 0x06);
	SENSOR_I2C_WRITE (0x1B, 0x03);
	SENSOR_I2C_WRITE (0x1C, 0x04);
	SENSOR_I2C_WRITE (0x1D, 0x08);
	SENSOR_I2C_WRITE (0x1E, 0x1A);
	SENSOR_I2C_WRITE (0x1F, 0x00);
	SENSOR_I2C_WRITE (0x20, 0x00);
	SENSOR_I2C_WRITE (0x21, 0x1E);
	SENSOR_I2C_WRITE (0x22, 0x1E);
	SENSOR_I2C_WRITE (0x23, 0x01);
	SENSOR_I2C_WRITE (0x24, 0x04);
	SENSOR_I2C_WRITE (0x25, 0x01);//Cmd_CSI2_Stall=1
	SENSOR_I2C_WRITE (0x27, 0x00);
	SENSOR_I2C_WRITE (0x28, 0x00);
	SENSOR_I2C_WRITE (0x2A, 0x4C);
	SENSOR_I2C_WRITE (0x2B, 0x04);
	SENSOR_I2C_WRITE (0x2C, 0xD0);
	SENSOR_I2C_WRITE (0x2D, 0x07);
	SENSOR_I2C_WRITE (0x2E, 0x00);
	SENSOR_I2C_WRITE (0x2F, 0x06);
	SENSOR_I2C_WRITE (0x30, 0x3A);
	SENSOR_I2C_WRITE (0x31, 0x04);
	SENSOR_I2C_WRITE (0x32, 0x00);
	SENSOR_I2C_WRITE (0x33, 0x00);
	SENSOR_I2C_WRITE (0x34, 0x00);
	SENSOR_I2C_WRITE (0x35, 0x00);
	SENSOR_I2C_WRITE (0x36, 0x00);
	SENSOR_I2C_WRITE (0x37, 0x00);
	SENSOR_I2C_WRITE (0x38, 0x0E);
	SENSOR_I2C_WRITE (0x3A, 0x02);
	SENSOR_I2C_WRITE (0x3B, 0x01);//R_Cmd_Gated_MIPI_Clk=1
	SENSOR_I2C_WRITE (0x3C, 0x00);
	SENSOR_I2C_WRITE (0x3D, 0x00);
	SENSOR_I2C_WRITE (0x3E, 0x00);
	SENSOR_I2C_WRITE (0x42, 0x00);//MIPI PLL disable
	SENSOR_I2C_WRITE (0x3F, 0x00);
	SENSOR_I2C_WRITE (0x40, 0x19);
	SENSOR_I2C_WRITE (0x41, 0x1C);
	SENSOR_I2C_WRITE (0x43, 0x06);//T_MIPI_sel[0], Bank5_67[1]=1
	SENSOR_I2C_WRITE (0x47, 0x05);
	SENSOR_I2C_WRITE (0x48, 0x00);
	SENSOR_I2C_WRITE (0x49, 0x01);
	SENSOR_I2C_WRITE (0x4A, 0x01);
	SENSOR_I2C_WRITE (0x4D, 0x02);
	SENSOR_I2C_WRITE (0x4F, 0x00);
	SENSOR_I2C_WRITE (0x54, 0x0A);
	SENSOR_I2C_WRITE (0x55, 0x01);
	SENSOR_I2C_WRITE (0x56, 0x0A);
	SENSOR_I2C_WRITE (0x57, 0x01);
	SENSOR_I2C_WRITE (0x58, 0x01);
	SENSOR_I2C_WRITE (0x59, 0x01);
	SENSOR_I2C_WRITE (0x42, 0x01);//MIPI PLL enable
	SENSOR_I2C_WRITE (0x5B, 0x10);//R_MIPI_FrameReset_by_Vsync_En=1
	SENSOR_I2C_WRITE (0x5C, 0x00);
	SENSOR_I2C_WRITE (0x5D, 0x00);
	SENSOR_I2C_WRITE (0x5E, 0x07);
	SENSOR_I2C_WRITE (0x5F, 0x08);
	SENSOR_I2C_WRITE (0x60, 0x00);
	SENSOR_I2C_WRITE (0x61, 0x00);
	SENSOR_I2C_WRITE (0x62, 0x00);
	SENSOR_I2C_WRITE (0x63, 0x28);
	SENSOR_I2C_WRITE (0x64, 0x30);
	SENSOR_I2C_WRITE (0x65, 0x9E);
	SENSOR_I2C_WRITE (0x66, 0xB9);
	SENSOR_I2C_WRITE (0x67, 0x52);
	SENSOR_I2C_WRITE (0x68, 0x70);
	SENSOR_I2C_WRITE (0x69, 0x4E);
	SENSOR_I2C_WRITE (0x70, 0x00);
	SENSOR_I2C_WRITE (0x71, 0x00);
	SENSOR_I2C_WRITE (0x72, 0x00);
	SENSOR_I2C_WRITE (0x90, 0x04);
	SENSOR_I2C_WRITE (0x91, 0x01);
	SENSOR_I2C_WRITE (0x92, 0x00);
	SENSOR_I2C_WRITE (0x93, 0x00);
	SENSOR_I2C_WRITE (0x94, 0x04);
	SENSOR_I2C_WRITE (0x96, 0x00);
	SENSOR_I2C_WRITE (0x97, 0x01);
	SENSOR_I2C_WRITE (0x98, 0x01);
	SENSOR_I2C_WRITE (0x9B, 0x00);
	SENSOR_I2C_WRITE (0x9C, 0x80);
	SENSOR_I2C_WRITE (0xA0, 0x00);
	SENSOR_I2C_WRITE (0xA1, 0x01);
	SENSOR_I2C_WRITE (0xA2, 0x00);
	SENSOR_I2C_WRITE (0xA3, 0x01);
	SENSOR_I2C_WRITE (0xA4, 0x00);
	SENSOR_I2C_WRITE (0xA5, 0x01);
	SENSOR_I2C_WRITE (0xA6, 0x00);
	SENSOR_I2C_WRITE (0xA7, 0x00);
	SENSOR_I2C_WRITE (0xAA, 0x00);
	SENSOR_I2C_WRITE (0xAB, 0x0F);
	SENSOR_I2C_WRITE (0xAC, 0x08);
	SENSOR_I2C_WRITE (0xAD, 0x09);
	SENSOR_I2C_WRITE (0xAE, 0x0A);
	SENSOR_I2C_WRITE (0xAF, 0x0B);
	SENSOR_I2C_WRITE (0xB0, 0x00);
	SENSOR_I2C_WRITE (0xB1, 0x00);
	SENSOR_I2C_WRITE (0xB2, 0x01);
	SENSOR_I2C_WRITE (0xB3, 0x00);
	SENSOR_I2C_WRITE (0xB4, 0x00);
	SENSOR_I2C_WRITE (0xB5, 0x0A);
	SENSOR_I2C_WRITE (0xB6, 0x0A);
	SENSOR_I2C_WRITE (0xB7, 0x0A);
	SENSOR_I2C_WRITE (0xB8, 0x0A);
	SENSOR_I2C_WRITE (0xB9, 0x00);
	SENSOR_I2C_WRITE (0xBA, 0x00);
	SENSOR_I2C_WRITE (0xBB, 0x00);
	SENSOR_I2C_WRITE (0xBC, 0x00);
	SENSOR_I2C_WRITE (0xBD, 0x00);
	SENSOR_I2C_WRITE (0xBE, 0x00);
	SENSOR_I2C_WRITE (0xBF, 0x00);
	SENSOR_I2C_WRITE (0xC0, 0x00);
	SENSOR_I2C_WRITE (0xC1, 0x00);
	SENSOR_I2C_WRITE (0xC2, 0x00);
	SENSOR_I2C_WRITE (0xC3, 0x00);
	SENSOR_I2C_WRITE (0xC4, 0x00);
	SENSOR_I2C_WRITE (0xC5, 0x00);
	SENSOR_I2C_WRITE (0xC6, 0x00);
	SENSOR_I2C_WRITE (0xC7, 0x00);
	SENSOR_I2C_WRITE (0xC8, 0x00);
	SENSOR_I2C_WRITE (0xCE, 0x00);
	SENSOR_I2C_WRITE (0xCF, 0x63);
	SENSOR_I2C_WRITE (0xD3, 0x80);
	SENSOR_I2C_WRITE (0xD4, 0x00);
	SENSOR_I2C_WRITE (0xD5, 0x00);
	SENSOR_I2C_WRITE (0xD6, 0x03);
	SENSOR_I2C_WRITE (0xD7, 0x77);
	SENSOR_I2C_WRITE (0xD8, 0x00);
	SENSOR_I2C_WRITE (0xED, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x06);
	SENSOR_I2C_WRITE (0x00, 0x0C);
	SENSOR_I2C_WRITE (0x01, 0x00);
	SENSOR_I2C_WRITE (0x02, 0x13);
	SENSOR_I2C_WRITE (0x03, 0x7D);
	SENSOR_I2C_WRITE (0x04, 0x05);
	SENSOR_I2C_WRITE (0x05, 0x01);
	SENSOR_I2C_WRITE (0x06, 0x00);
	SENSOR_I2C_WRITE (0x07, 0x01);
	SENSOR_I2C_WRITE (0x08, 0x01);
	SENSOR_I2C_WRITE (0x09, 0x01);
	SENSOR_I2C_WRITE (0x0A, 0x01);
	SENSOR_I2C_WRITE (0x0B, 0x82);
	SENSOR_I2C_WRITE (0x0C, 0xFA);
	SENSOR_I2C_WRITE (0x0D, 0xC2);
	SENSOR_I2C_WRITE (0x0E, 0x00);
	SENSOR_I2C_WRITE (0x0F, 0x01);
	SENSOR_I2C_WRITE (0x10, 0xC2);
	SENSOR_I2C_WRITE (0x11, 0x01);
	SENSOR_I2C_WRITE (0x12, 0xC2);
	SENSOR_I2C_WRITE (0x17, 0x00);
	SENSOR_I2C_WRITE (0x18, 0x00);
	SENSOR_I2C_WRITE (0x19, 0x00);
	SENSOR_I2C_WRITE (0x1A, 0x00);
	SENSOR_I2C_WRITE (0x1F, 0x04);
	SENSOR_I2C_WRITE (0x20, 0x00);
	SENSOR_I2C_WRITE (0x21, 0x00);
	SENSOR_I2C_WRITE (0x22, 0x00);
	SENSOR_I2C_WRITE (0x28, 0x01);
	SENSOR_I2C_WRITE (0x29, 0x00);
	SENSOR_I2C_WRITE (0x2A, 0x78);
	SENSOR_I2C_WRITE (0x2B, 0x40);
	SENSOR_I2C_WRITE (0x2C, 0x08);
	SENSOR_I2C_WRITE (0x30, 0xDA);
	SENSOR_I2C_WRITE (0x31, 0xDA);
	SENSOR_I2C_WRITE (0x32, 0xDA);
	SENSOR_I2C_WRITE (0x33, 0x13);
	SENSOR_I2C_WRITE (0x34, 0x3D);
	SENSOR_I2C_WRITE (0x35, 0x9D);
	SENSOR_I2C_WRITE (0x36, 0x7F);
	SENSOR_I2C_WRITE (0x37, 0x7F);
	SENSOR_I2C_WRITE (0x38, 0x7F);
	SENSOR_I2C_WRITE (0x39, 0x7F);
	SENSOR_I2C_WRITE (0x3A, 0x73);
	SENSOR_I2C_WRITE (0x3B, 0x73);
	SENSOR_I2C_WRITE (0x3C, 0x73);
	SENSOR_I2C_WRITE (0x3D, 0xE6);
	SENSOR_I2C_WRITE (0x3E, 0x13);
	SENSOR_I2C_WRITE (0x3F, 0xE6);
	SENSOR_I2C_WRITE (0x40, 0x00);
	SENSOR_I2C_WRITE (0x41, 0x00);
	SENSOR_I2C_WRITE (0x42, 0x00);
	SENSOR_I2C_WRITE (0x43, 0x00);
	SENSOR_I2C_WRITE (0x44, 0x00);
	SENSOR_I2C_WRITE (0x45, 0x80);
	SENSOR_I2C_WRITE (0x46, 0x00);
	SENSOR_I2C_WRITE (0x49, 0x0C);
	SENSOR_I2C_WRITE (0x5A, 0x04);
	SENSOR_I2C_WRITE (0x5B, 0x00);
	SENSOR_I2C_WRITE (0x5C, 0x04);
	SENSOR_I2C_WRITE (0x5D, 0x00);
	SENSOR_I2C_WRITE (0x5E, 0x90);
	SENSOR_I2C_WRITE (0x5F, 0x90);
	SENSOR_I2C_WRITE (0x60, 0x10);
	SENSOR_I2C_WRITE (0x61, 0x10);
	SENSOR_I2C_WRITE (0x64, 0x40);
	SENSOR_I2C_WRITE (0x65, 0x01);
	SENSOR_I2C_WRITE (0x66, 0x60);
	SENSOR_I2C_WRITE (0x67, 0x11);
	SENSOR_I2C_WRITE (0x68, 0x60);
	SENSOR_I2C_WRITE (0x69, 0x51);
	SENSOR_I2C_WRITE (0x6A, 0x70);
	SENSOR_I2C_WRITE (0x6B, 0x91);
	SENSOR_I2C_WRITE (0x6C, 0x81);
	SENSOR_I2C_WRITE (0x6D, 0x11);
	SENSOR_I2C_WRITE (0x6E, 0x82);
	SENSOR_I2C_WRITE (0x6F, 0x11);
	SENSOR_I2C_WRITE (0x70, 0x83);
	SENSOR_I2C_WRITE (0x71, 0x11);
	SENSOR_I2C_WRITE (0x72, 0x94);
	SENSOR_I2C_WRITE (0x73, 0x11);
	SENSOR_I2C_WRITE (0x74, 0x86);
	SENSOR_I2C_WRITE (0x75, 0x10);
	SENSOR_I2C_WRITE (0x76, 0x87);
	SENSOR_I2C_WRITE (0x77, 0x10);
	SENSOR_I2C_WRITE (0x78, 0x88);
	SENSOR_I2C_WRITE (0x79, 0x10);
	SENSOR_I2C_WRITE (0x7A, 0x79);
	SENSOR_I2C_WRITE (0x7B, 0x10);
	SENSOR_I2C_WRITE (0x7C, 0x69);
	SENSOR_I2C_WRITE (0x7D, 0x90);
	SENSOR_I2C_WRITE (0x7E, 0x69);
	SENSOR_I2C_WRITE (0x7F, 0xD0);
	SENSOR_I2C_WRITE (0x80, 0x4A);
	SENSOR_I2C_WRITE (0x81, 0x0C);
	SENSOR_I2C_WRITE (0x82, 0x0A);
	SENSOR_I2C_WRITE (0x83, 0x20);
	SENSOR_I2C_WRITE (0x84, 0x60);
	SENSOR_I2C_WRITE (0x85, 0x01);
	SENSOR_I2C_WRITE (0x86, 0x70);
	SENSOR_I2C_WRITE (0x87, 0x41);
	SENSOR_I2C_WRITE (0x88, 0x80);
	SENSOR_I2C_WRITE (0x89, 0xC1);
	SENSOR_I2C_WRITE (0x8A, 0x81);
	SENSOR_I2C_WRITE (0x8B, 0xC1);
	SENSOR_I2C_WRITE (0x8C, 0x92);
	SENSOR_I2C_WRITE (0x8D, 0xC1);
	SENSOR_I2C_WRITE (0x8E, 0x84);
	SENSOR_I2C_WRITE (0x8F, 0xD8);
	SENSOR_I2C_WRITE (0x90, 0x85);
	SENSOR_I2C_WRITE (0x91, 0xD8);
	SENSOR_I2C_WRITE (0x92, 0x76);
	SENSOR_I2C_WRITE (0x93, 0xD8);
	SENSOR_I2C_WRITE (0x94, 0x67);
	SENSOR_I2C_WRITE (0x95, 0x58);
	SENSOR_I2C_WRITE (0x96, 0x07);
	SENSOR_I2C_WRITE (0x97, 0x98);
	SENSOR_I2C_WRITE (0xA7, 0x00);
	SENSOR_I2C_WRITE (0xA8, 0x00);
	SENSOR_I2C_WRITE (0xA9, 0x00);
	SENSOR_I2C_WRITE (0xAA, 0x00);
	SENSOR_I2C_WRITE (0xAC, 0x00);
	SENSOR_I2C_WRITE (0xAD, 0x03);
	SENSOR_I2C_WRITE (0xAE, 0x00);
	SENSOR_I2C_WRITE (0xAF, 0xE8);
	SENSOR_I2C_WRITE (0xB0, 0x00);
	SENSOR_I2C_WRITE (0xB1, 0x80);
	SENSOR_I2C_WRITE (0xB2, 0x80);
	SENSOR_I2C_WRITE (0xB3, 0x80);
	SENSOR_I2C_WRITE (0xB4, 0x80);
	SENSOR_I2C_WRITE (0xBD, 0x00);
	SENSOR_I2C_WRITE (0xBE, 0x03);
	SENSOR_I2C_WRITE (0xBF, 0xC8);
	SENSOR_I2C_WRITE (0xC0, 0xE8);
	SENSOR_I2C_WRITE (0xC1, 0x00);
	SENSOR_I2C_WRITE (0xC2, 0x80);
	SENSOR_I2C_WRITE (0xC3, 0x80);
	SENSOR_I2C_WRITE (0xC4, 0x80);
	SENSOR_I2C_WRITE (0xC5, 0x80);
	SENSOR_I2C_WRITE (0xE0, 0x00);
	SENSOR_I2C_WRITE (0xE1, 0x00);

	SENSOR_I2C_WRITE (0xE2, 0x00);
	SENSOR_I2C_WRITE (0xE3, 0x00);
	SENSOR_I2C_WRITE (0xF0, 0x00);
	SENSOR_I2C_WRITE (0xF3, 0x00);
	SENSOR_I2C_WRITE (0xF4, 0x00);
	SENSOR_I2C_WRITE (0xF5, 0x00);
	SENSOR_I2C_WRITE (0xF6, 0x00);
	SENSOR_I2C_WRITE (0xF7, 0x00);
	SENSOR_I2C_WRITE (0xF8, 0x00);
	SENSOR_I2C_WRITE (0xF9, 0x00);
	SENSOR_I2C_WRITE (0xFA, 0x00);
	SENSOR_I2C_WRITE (0xFB, 0x00);
	SENSOR_I2C_WRITE (0xFC, 0x00);
	SENSOR_I2C_WRITE (0xFD, 0x00);
	SENSOR_I2C_WRITE (0xFE, 0x00);
	SENSOR_I2C_WRITE (0xED, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x03);
	SENSOR_I2C_WRITE (0x00, 0xEF);
	SENSOR_I2C_WRITE (0x01, 0x01);
	SENSOR_I2C_WRITE (0x02, 0x31);
	SENSOR_I2C_WRITE (0x03, 0x00);
	SENSOR_I2C_WRITE (0x04, 0x32);
	SENSOR_I2C_WRITE (0x05, 0x00);
	SENSOR_I2C_WRITE (0x06, 0x33);
	SENSOR_I2C_WRITE (0x07, 0x00);
	SENSOR_I2C_WRITE (0x08, 0x3B);
	SENSOR_I2C_WRITE (0x09, 0x4C);
	SENSOR_I2C_WRITE (0x0A, 0x3E);
	SENSOR_I2C_WRITE (0x0B, 0x30);
	SENSOR_I2C_WRITE (0x0C, 0x3F);
	SENSOR_I2C_WRITE (0x0D, 0xFF);
	SENSOR_I2C_WRITE (0x0E, 0x42);
	SENSOR_I2C_WRITE (0x0F, 0xFF);
	SENSOR_I2C_WRITE (0x10, 0x43);
	SENSOR_I2C_WRITE (0x11, 0xFF);
	SENSOR_I2C_WRITE (0x12, 0x44);
	SENSOR_I2C_WRITE (0x13, 0xFF);
	SENSOR_I2C_WRITE (0x14, 0x47);
	SENSOR_I2C_WRITE (0x15, 0x08);
	SENSOR_I2C_WRITE (0x16, 0x4B);
	SENSOR_I2C_WRITE (0x17, 0x08);
	SENSOR_I2C_WRITE (0x18, 0x4E);
	SENSOR_I2C_WRITE (0x19, 0x00);
	SENSOR_I2C_WRITE (0x1a, 0x52);
	SENSOR_I2C_WRITE (0x1b, 0x00);
	SENSOR_I2C_WRITE (0x1c, 0x53);
	SENSOR_I2C_WRITE (0x1d, 0x00);
	SENSOR_I2C_WRITE (0x1e, 0x58);
	SENSOR_I2C_WRITE (0x1f, 0x00);
	SENSOR_I2C_WRITE (0x20, 0x59);
	SENSOR_I2C_WRITE (0x21, 0x00);
	SENSOR_I2C_WRITE (0x22, 0x5A);
	SENSOR_I2C_WRITE (0x23, 0x14);
	SENSOR_I2C_WRITE (0x24, 0x64);
	SENSOR_I2C_WRITE (0x25, 0x00);
	SENSOR_I2C_WRITE (0x26, 0x65);
	SENSOR_I2C_WRITE (0x27, 0x00);
	SENSOR_I2C_WRITE (0x28, 0x66);
	SENSOR_I2C_WRITE (0x29, 0x00);
	SENSOR_I2C_WRITE (0x2a, 0x67);
	SENSOR_I2C_WRITE (0x2b, 0x00);
	SENSOR_I2C_WRITE (0x2c, 0x68);
	SENSOR_I2C_WRITE (0x2d, 0x00);
	SENSOR_I2C_WRITE (0x2e, 0x69);
	SENSOR_I2C_WRITE (0x2f, 0x00);
	SENSOR_I2C_WRITE (0x30, 0x74);
	SENSOR_I2C_WRITE (0x31, 0x0B);
	SENSOR_I2C_WRITE (0x32, 0x75);
	SENSOR_I2C_WRITE (0x33, 0xB8);
	SENSOR_I2C_WRITE (0x34, 0x7A);
	SENSOR_I2C_WRITE (0x35, 0x00);
	SENSOR_I2C_WRITE (0x36, 0x7B);
	SENSOR_I2C_WRITE (0x37, 0x96);
	SENSOR_I2C_WRITE (0x38, 0x7C);
	SENSOR_I2C_WRITE (0x39, 0x0B);
	SENSOR_I2C_WRITE (0x3a, 0x7D);
	SENSOR_I2C_WRITE (0x3b, 0xB8);
	SENSOR_I2C_WRITE (0x3c, 0xBC);
	SENSOR_I2C_WRITE (0x3d, 0x81);
	SENSOR_I2C_WRITE (0x3e, 0x00);
	SENSOR_I2C_WRITE (0x3f, 0x00);
	SENSOR_I2C_WRITE (0xef, 0x01);
	SENSOR_I2C_WRITE (0x9f, 0x03);
	SENSOR_I2C_WRITE (0xef, 0x03);
	SENSOR_I2C_WRITE (0x00, 0xef);
	SENSOR_I2C_WRITE (0x01, 0x01);
	SENSOR_I2C_WRITE (0x02, 0x31);
	SENSOR_I2C_WRITE (0x03, 0x03);
	SENSOR_I2C_WRITE (0x04, 0x32);
	SENSOR_I2C_WRITE (0x05, 0xFF);
	SENSOR_I2C_WRITE (0x06, 0x33);
	SENSOR_I2C_WRITE (0x07, 0x70);
	SENSOR_I2C_WRITE (0x08, 0x3B);
	SENSOR_I2C_WRITE (0x09, 0x38);
	SENSOR_I2C_WRITE (0x0A, 0x3E);
	SENSOR_I2C_WRITE (0x0B, 0x20);
	SENSOR_I2C_WRITE (0x0C, 0x3F);
	SENSOR_I2C_WRITE (0x0D, 0x22);
	SENSOR_I2C_WRITE (0x0E, 0x42);
	SENSOR_I2C_WRITE (0x0F, 0x2C);
	SENSOR_I2C_WRITE (0x10, 0x43);
	SENSOR_I2C_WRITE (0x11, 0x70);
	SENSOR_I2C_WRITE (0x12, 0x44);
	SENSOR_I2C_WRITE (0x13, 0x04);
	SENSOR_I2C_WRITE (0x14, 0x47);
	SENSOR_I2C_WRITE (0x15, 0x00);
	SENSOR_I2C_WRITE (0x16, 0x4B);
	SENSOR_I2C_WRITE (0x17, 0x00);
	SENSOR_I2C_WRITE (0x18, 0x4E);
	SENSOR_I2C_WRITE (0x19, 0x08);
	SENSOR_I2C_WRITE (0x1a, 0x52);
	SENSOR_I2C_WRITE (0x1b, 0x02);
	SENSOR_I2C_WRITE (0x1c, 0x53);
	SENSOR_I2C_WRITE (0x1d, 0x08);
	SENSOR_I2C_WRITE (0x1e, 0x58);
	SENSOR_I2C_WRITE (0x1f, 0x02);
	SENSOR_I2C_WRITE (0x20, 0x59);
	SENSOR_I2C_WRITE (0x21, 0x08);
	SENSOR_I2C_WRITE (0x22, 0x5A);
	SENSOR_I2C_WRITE (0x23, 0x02);
	SENSOR_I2C_WRITE (0x24, 0x64);
	SENSOR_I2C_WRITE (0x25, 0x03);
	SENSOR_I2C_WRITE (0x26, 0x65);
	SENSOR_I2C_WRITE (0x27, 0xFF);
	SENSOR_I2C_WRITE (0x28, 0x66);
	SENSOR_I2C_WRITE (0x29, 0x70);
	SENSOR_I2C_WRITE (0x2a, 0x67);
	SENSOR_I2C_WRITE (0x2b, 0x22);
	SENSOR_I2C_WRITE (0x2c, 0x68);
	SENSOR_I2C_WRITE (0x2d, 0xBC);
	SENSOR_I2C_WRITE (0x2e, 0x69);
	SENSOR_I2C_WRITE (0x2f, 0xBC);
	SENSOR_I2C_WRITE (0x30, 0x74);
	SENSOR_I2C_WRITE (0x31, 0x11);
	SENSOR_I2C_WRITE (0x32, 0x75);
	SENSOR_I2C_WRITE (0x33, 0x12);
	SENSOR_I2C_WRITE (0x34, 0x7A);
	SENSOR_I2C_WRITE (0x35, 0x05);
	SENSOR_I2C_WRITE (0x36, 0x7B);
	SENSOR_I2C_WRITE (0x37, 0x7D);
	SENSOR_I2C_WRITE (0x38, 0x7C);
	SENSOR_I2C_WRITE (0x39, 0x06);
	SENSOR_I2C_WRITE (0x3a, 0x7D);
	SENSOR_I2C_WRITE (0x3b, 0x31);
	SENSOR_I2C_WRITE (0x3c, 0xBC);
	SENSOR_I2C_WRITE (0x3d, 0x01);
	SENSOR_I2C_WRITE (0x3e, 0x00);
	SENSOR_I2C_WRITE (0x3f, 0x00);
	SENSOR_I2C_WRITE (0xEF, 0x01);
	SENSOR_I2C_WRITE (0x9f, 0x00);
	SENSOR_I2C_WRITE (0xEF, 0x00);
	SENSOR_I2C_WRITE (0x11, 0x00);//GatedAllClk disable
	SENSOR_I2C_WRITE (0xEF, 0x05);
	SENSOR_I2C_WRITE (0x3B, 0x00);//R_Cmd_Gated_MIPI_Clk=0
	SENSOR_I2C_WRITE (0xED, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x01);
	SENSOR_I2C_WRITE (0x02, 0x73);//Reset MIPI & ISP & TG
	SENSOR_I2C_WRITE (0x09, 0x01);
	SENSOR_I2C_WRITE (0xEF, 0x05);
	SENSOR_I2C_WRITE (0x0F, 0x01);//MIPI CSI enable
	SENSOR_I2C_WRITE (0xED, 0x01);
	SENSOR_DELAY_MS(500);
	SENSOR_I2C_WRITE (0xEF, 0x05);
	SENSOR_I2C_WRITE (0x25, 0x00);//Cmd_CSI2_Stall=0
	SENSOR_I2C_WRITE (0xED, 0x01);

	SENSOR_I2C_WRITE(0xEF,0x01);	// return to bank1 for AE
	SENSOR_I2C_READ(0x01, &g_sns_ver);	// Check Ver.
	g_sns_ver &= 0x0F;

	printf("=========================================================\n");
	printf("====PS5270 sensor 12.5fps 1536x1536 reg init success!====\n");
	printf("=========================================================\n");
	bSensorInit = HI_TRUE;
}


HI_S32 ps5270_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ps5270_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ps5270_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = ps5270_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ps5270_set_wdr_mode;

    pstSensorExpFunc->pfn_cmos_get_isp_default = ps5270_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ps5270_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ps5270_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = ps5270_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int ps5270_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ps5270_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, PS5270_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    ps5270_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, PS5270_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    ps5270_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, PS5270_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    return 0;
}

int ps5270_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, PS5270_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, PS5270_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, PS5270_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    return 0;
}

int SmartSens_PS5270_init(SENSOR_DO_I2CRD do_i2c_read,
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	ps5270_register_callback();
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
	printf("SmartSens PS5270 sensor 1080P30fps init success!\n");
	return s32Ret;
}

int PS5270_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_PS5270_WIDTH;
        *ret_height = SENSOR_PS5270_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_PS5270_probe()
{
    uint16_t ret_data1 = 0;
    uint16_t ret_data2 = 0;

//    SENSOR_I2C_WRITE(0xef,0x0);

    SENSOR_I2C_READ(0x0, &ret_data1);
    SENSOR_I2C_READ(0x1, &ret_data2);

	printf(" PS5270 sensor probe ret_data1=%d;ret_data2=%d\n",ret_data1,ret_data1);

	if(ret_data1 == PS5270_CHECK_DATA_LSB && ret_data2 == PS5270_CHECK_DATA_MSB){
		printf(" ===================zg test  ps5270  probe================================\n");
	//set i2c pinmux
		sdk_sys->write_reg(0x200f0040, 0x2);
	    sdk_sys->write_reg(0x200f0044, 0x2);
		sdk_sys->write_reg(0x2003002c, 0xb4001);	 // sensor unreset, clk 27MHz, VI 99MHz
       return true;
    }
    return false;
}

int PS5270_get_sensor_name(char *sensor_name)
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

#endif /* __PS5270_CMOS_H_ */



