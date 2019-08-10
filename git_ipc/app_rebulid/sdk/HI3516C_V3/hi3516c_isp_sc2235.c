#include "sdk/sdk_debug.h"
#include "hi3516c.h"
#include "hi3516c_isp_sensor.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "sc2235"
static SENSOR_SMARTSENS_SC2235_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_SMARTSENS_SC2235_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

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

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
static HI_U16 g_au16InitWBGain[ISP_MAX_DEV_NUM][3] = {{0}};
static HI_U16 g_au16SampleRgain[ISP_MAX_DEV_NUM] = {0};
static HI_U16 g_au16SampleBgain[ISP_MAX_DEV_NUM] = {0};



static const unsigned int sensor_i2c_addr = 0x60;
static unsigned int sensor_addr_byte = 2;
static unsigned int sensor_data_byte = 1;

#define FULL_LINES_MAX  (0xFFFF)

/*****SC2235 Register Address*****/
#define EXPTIME0_ADDR (0x3E01)
#define EXPTIME1_ADDR (0x3E02)

#define GAIN0_ADDR (0x3E07)
#define GAIN1_ADDR (0x3E08)
#define GAIN2_ADDR (0x3E09)

#define VMAX0_ADDR (0x320E)
#define VMAX1_ADDR (0x320F)
#define HMAX0_ADDR (0x320C)
#define HMAX1_ADDR (0x320D)

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define SC2235_VMAX_1080P30_LINEAR  (1125+INCREASE_LINES)

/* sensor fps mode */
#define SC2235_SENSOR_1080P_30FPS_LINEAR_MODE  (0)


/* global variables */
static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;

static HI_U8 gu8SensorImageMode = SC2235_SENSOR_1080P_30FPS_LINEAR_MODE;
static HI_U8 genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = SC2235_VMAX_1080P30_LINEAR;
static HI_U32 gu32FullLines = SC2235_VMAX_1080P30_LINEAR;

ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};

#define PATHLEN_MAX 256
#define CMOS_CFG_INI "sc2235_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/sc2235_cfg.ini";

#define SC2235_CHECK_DATA_LSB	(0x22)
#define SC2235_CHECK_DATA_MSB	(0x35)//CHIP_ID address is 0x3107&0x3108

static HI_S32 sc2235_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
 	if (HI_NULL == pstAeSnsDft)
	{
		printf("null pointer when get ae default value!\n");
		return -1;
	}

	if( SC2235_SENSOR_1080P_30FPS_LINEAR_MODE == gu8SensorImageMode )
    	{
       		pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 25 / 2;
    	}
	else{
	    printf("Not support this Sensor Image Mode %d\n",gu8SensorImageMode);
	    return -1;
	}

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 50 * 256;
    pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;
    pstAeSnsDft->u8AERunInterval  = 1;
    pstAeSnsDft->u32InitExposure  = 10;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift;



    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_0;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_32_0;

    pstAeSnsDft->bAERouteExValid = HI_FALSE;
    pstAeSnsDft->stAERouteAttr.u32TotalNum = 0;
    pstAeSnsDft->stAERouteAttrEx.u32TotalNum = 0;

    pstAeSnsDft->u32InitExposure = 921600;


  switch(genSensorMode)
    {
        case WDR_MODE_NONE:
            pstAeSnsDft->au8HistThresh[0] = 0x0D;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u32MaxAgain = 15872; //15.5*1024
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

	    pstAeSnsDft->u32MaxDgain = 1;  //4
            pstAeSnsDft->u32MinDgain = 1;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

            pstAeSnsDft->u8AeCompensation = 0x40;

            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
            pstAeSnsDft->u32MinIntTime = 1;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 1;
        break;
		default:
			printf("cmos_get_ae_default_Sensor Mode is error!\n");
		break;
    }

	return 0;
}


/* the function of sensor set fps */
static HI_VOID sc2235_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 u32VMAX = SC2235_VMAX_1080P30_LINEAR;

    switch (gu8SensorImageMode)
    {
	case SC2235_SENSOR_1080P_30FPS_LINEAR_MODE:
           if ((f32Fps <= 30) && (f32Fps > 0.5))    //8.23
           {
               u32VMAX = SC2235_VMAX_1080P30_LINEAR * 25 / f32Fps;
           }
           else
           {
               printf("FPS30_1080P_Not support Fps: %f\n", f32Fps);
               return;
           }
        break;
		default:
		break;
	}
	u32VMAX = (u32VMAX > FULL_LINES_MAX) ? FULL_LINES_MAX : u32VMAX;
	gu32FullLinesStd = u32VMAX;

    g_stSnsRegsInfo.astI2cData[4].u32Data = ((u32VMAX & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[5].u32Data = (u32VMAX & 0xFF);

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
    gu32FullLines = gu32FullLinesStd;
    pstAeSnsDft->u32FullLines = gu32FullLines;

    return;
}

static HI_VOID sc2235_slow_framerate_set(HI_U32 u32FullLines,
	AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;

    g_stSnsRegsInfo.astI2cData[4].u32Data = ((u32FullLines & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[5].u32Data = (u32FullLines & 0xFF);

    pstAeSnsDft->u32FullLines = gu32FullLines;
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID sc2235_inttime_update(HI_U32 u32IntTime)
{
	g_stSnsRegsInfo.astI2cData[16].u32Data = (u32IntTime >> 12) & 0x0F ;          //0x3e00
	g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 4) & 0xFF;             //0x3e01
	g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime << 4) & 0xF0;             //0x3e02

	if (u32IntTime < 0x23B)  //SC2235_logic_with inttime
	{
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x12;
	}
	else if (u32IntTime > 0x24B)
	{
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x02;
	}

	return;

}

static const HI_U16 u16AgainTab[64]={
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

	if(again >= u16AgainTab[againmax])
	{
		*pu32AgainDb = againmax;
	}
	else
	{
		for(i = 1; i < 64; i++)
		{
			if(again < u16AgainTab[i])
			{
				*pu32AgainDb = i - 1;
				break;
			}
		}
	}
	*pu32AgainLin = u16AgainTab[*pu32AgainDb];

    return;
}

static HI_VOID sc2235_cmos_dgain_calc_table(HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
	*pu32DgainDb = *pu32DgainLin / 1024;
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

	*pu32DgainLin = *pu32DgainDb * 1024;

    return;
}

static HI_VOID sc2235_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{

	HI_U8 u8Reg0x3e09, u8Reg0x3e08;
	HI_U8 i, leftnum;
	int gain_all = u16AgainTab[u32Again] * u32Dgain;

	// Again
	u8Reg0x3e09 = 0x10 | (u32Again & 0x0f);
	u8Reg0x3e08 = 0x03;
	leftnum = u32Again / 16;      			//Again_index >>>> Left_shift_num
	for(i = 0; i < leftnum; i++)
	{
		u8Reg0x3e08 = (u8Reg0x3e08 << 1)|0x01;
	}
	// Dgain
/**	if(u32Dgain == 2)
	{
		u8Reg0x3e08 |= 0x20;
	}
	else if(u32Dgain == 4)
	{
		u8Reg0x3e08 |= 0x60;
	}
	else if(u32Dgain == 8)
	{
		u8Reg0x3e08 |= 0xe0;
	}
/**/
	g_stSnsRegsInfo.astI2cData[2].u32Data = u8Reg0x3e08;
	g_stSnsRegsInfo.astI2cData[3].u32Data = u8Reg0x3e09;

	//Sensor_Logic
	g_stSnsRegsInfo.astI2cData[6].u32Data  = 0x84;//0x3903
	g_stSnsRegsInfo.astI2cData[7].u32Data  = 0x04;//0x3903
        g_stSnsRegsInfo.astI2cData[8].u32Data  = 0x00;//0x3812

	if(u16AgainTab[u32Again] < 2*1024){
		g_stSnsRegsInfo.astI2cData[9].u32Data  = 0x05;//0x3301
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x84;//0x3631
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x2f;//0x366f
		g_stSnsRegsInfo.astI2cData[17].u32Data = 0xc6; 
	}
	else if(u16AgainTab[u32Again] < 8*1024){
		g_stSnsRegsInfo.astI2cData[9].u32Data  = 0x13;
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x88;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x2f;
		g_stSnsRegsInfo.astI2cData[17].u32Data = 0xc6;
	}
	else if (u16AgainTab[u32Again] < 15.5*1024){
		g_stSnsRegsInfo.astI2cData[9].u32Data  = 0x14;
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x88;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x2f;
		g_stSnsRegsInfo.astI2cData[17].u32Data = 0xc6;
	}
	else
	{
		g_stSnsRegsInfo.astI2cData[9].u32Data  = 0xff;
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x88;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x3a;//(0x3a)
		 g_stSnsRegsInfo.astI2cData[17].u32Data = 0x06;
	}

	//Sensor_Internal_DPC
	if(gain_all < 10*1024)
	{
		g_stSnsRegsInfo.astI2cData[13].u32Data = 0x04;//0x5781
		g_stSnsRegsInfo.astI2cData[14].u32Data = 0x18;//0x5785
	}
	else
	{
		g_stSnsRegsInfo.astI2cData[13].u32Data = 0x02;
		g_stSnsRegsInfo.astI2cData[14].u32Data = 0x08;
	}
	g_stSnsRegsInfo.astI2cData[15].u32Data = 0x30;//0x3812

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
    pstExpFuncs->pfn_cmos_get_inttime_max   = NULL;//sc2235_get_inttime_max;
    pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = NULL;//sc2235_ae_fswdr_attr_set;

    return 0;
}

static AWB_CCM_S g_stAwbCcm =
{
	7751,
	{
		0x1B1,0x8078,0x8039,
		0x8063,0x1F2,0x808F,
		0x8006,0x80DB,0x1E1
	},
	4651,
	{
		0x231,0x8136,0x5,
		0x806C,0x17C,0x8010,
		0x5,0x8139,0x234
	},
	3048,
	{
		0x199,0x809E,0x5,
		0x808A,0x185,0x5,
		0x8032,0x824F,0x381
	}
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /*1,  2,  4,  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768*/
    /* saturation */
    {0x80,0x7a,0x78,0x74,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
};

static HI_S32 sc2235_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{ 
	ISP_DEV IspDev = 0;
	if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }
    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    pstAwbSnsDft->u16WbRefTemp = 5082;

    pstAwbSnsDft->au16GainOffset[0] = 0x15E;//R
    pstAwbSnsDft->au16GainOffset[1] = 0x100;//G
    pstAwbSnsDft->au16GainOffset[2] = 0x100;//G
    pstAwbSnsDft->au16GainOffset[3] = 0x195;//B

    pstAwbSnsDft->as32WbPara[0] = 207;
    pstAwbSnsDft->as32WbPara[1] = -141;
    pstAwbSnsDft->as32WbPara[2] = -190;
    pstAwbSnsDft->as32WbPara[3] = 186726;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -136275;

	memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
	memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));

	pstAwbSnsDft->u16SampleRgain = g_au16SampleRgain[IspDev];
    pstAwbSnsDft->u16SampleBgain = g_au16SampleBgain[IspDev];
    pstAwbSnsDft->u16InitRgain = g_au16InitWBGain[IspDev][0];
    pstAwbSnsDft->u16InitGgain = g_au16InitWBGain[IspDev][1];
    pstAwbSnsDft->u16InitBgain = g_au16InitWBGain[IspDev][2];
    pstAwbSnsDft->u8AWBRunInterval = 4;

    return 0;
}


HI_S32 sc2235_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
    pstExpFuncs->pfn_cmos_get_awb_default = sc2235_get_awb_default;
    return 0;
}

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
	1,    //bEnable
    /*au16EdgeSmoothThr*/
	{8,16,16,32,32,32,1022,1022,1022,1022,1022,1022,1022,1022,1022,1022},
	/*au16EdgeSmoothSlope*/
	{8,32,32,48,48,48,0,0,0,0,0,0,0,0,0,0},
	/*au16AntiAliasThr*/
	{53,53,53,86,112,112,1022,1022,1022,1022,1022,1022,1022,1022,1022,1022},
	/*au16AntiAliasSlope*/
	{256,256,256,256,256,256,0,0,0,0,0,0,0,0,0,0},
    /*NrCoarseStr*/
    {128, 128, 128, 64, 64, 32, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16},
	/*NoiseSuppressStr*/
	{4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 36},
	/*DetailEnhanceStr*/
	{4, 4, 4, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /*SharpenLumaStr*/
    {256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256},
    /*ColorNoiseCtrlThr*/
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


static ISP_CMOS_GE_S g_stIspGe =
{
	/*For GE*/
	1,    /*bEnable*/
	9,    /*u8Slope*/
	9,    /*u8SensiSlope*/
	300, /*u16SensiThr*/
	{300,300,300,300,310,310,310,  310,  320,320,320,320,330,330,330,330}, /*au16Threshold[ISP_AUTO_ISO_STRENGTH_NUM]*/
	{ 128, 128, 128, 128, 129, 129, 129,   129,   130, 130, 130, 130, 131, 131, 131, 131}, /*au16Strength[ISP_AUTO_ISO_STRENGTH_NUM]*/
	{1024,1024,1024,2048,2048,2048,2048,  2048,  2048,2048,2048,2048,2048,2048,2048,2048}    /*au16NpOffset[ISP_AUTO_ISO_STRENGTH_NUM]*/
};

static ISP_CMOS_FCR_S g_stIspFcr =
{
	/*For FCR*/
	1,    /*bEnable*/
	{8,8,8,8,7,7,7,  6,  6,6,5,4,3,2,1,0}, /*au8Strength[ISP_AUTO_ISO_STRENGTH_NUM]*/
	{ 24, 24, 24, 24, 20, 20, 20,   16,   14, 12, 10, 8, 6, 4, 2, 0}, /*au8Threhold[ISP_AUTO_ISO_STRENGTH_NUM]*/
	{150,150,150,150,150,150,150,  150,  150,150,150,150,150,150,150,150}    /*au16Offset[ISP_AUTO_ISO_STRENGTH_NUM]*/
};

static ISP_CMOS_YUV_SHARPEN_S g_stIspYuvSharpen =
{
     /* bvalid */
     1,

     /* 100,  200,    400,     800,    1600,    3200,    6400,    12800,    25600,   51200,  102400,  204800,   409600,   819200,   1638400,  3276800 */

	 /* au8SharpenUd */
	 {23 ,	 22 ,     20,    19,     18,	   15,	    15,   	 13,	   6,	   	  6,	  	6,	  	5,	  	5,	  	5,	  	5,    5},

	/* au8SharpenD */

     {75 ,   75 ,    72,     70,    70,	       68,      65,      60,	   45,	     45,	   40,	  40,	  38,	  38,	  38,    38},

     /*au8TextureThd*/
     {0 ,    0,        0,      0 ,    0,       0,       0,       0,      0,        0,         0,      0,       0,    0,      0,     0},

     /* au8SharpenEdge */

     {80  ,    80,     80,     72,     72,     70,      70,      70,       110,      120,       120,     120,   120,    120,    120,   120},

     /* au8EdgeThd */
     {80 ,     80 ,     80,    80,     80,     80,     90,       90,      120,       150,      150,    150,    150,    150,    150,   150},

     /* au8OverShoot */
     {150 ,   150 ,    150,    150,    140,    130,     120,     100,     80,       80,    80,     80,     80,     80,     80,    80},

     /* au8UnderShoot */
     {200  ,  200  ,   200,    200,    170,    150,     120,     100,     80,       80,    80,     80,    80,     80,     80,    80},

     /*au8shootSupSt*/
	 {33,	  33,	   33,	   33,	   16,		0,     0,		0,		0,		0,		0,			0,			0,		0,		0,		0},

     /* au8DetailCtrl  */
     {128  ,  128  ,   128,    128,    128,    118,    118,     108,     80,      80,    78,     50,    50,    50,     40,      30},

	
 	 /* au8RGain */
     {16,   16,   16,   16,   24,   31,   31,   31,   31,   31,   31,   31,   31,   31,   31,   31},

	 /* au8BGain */
     {20,   20,   20,   20,   26,   31,   31,   31,   31,   31,   31,   31,   31,   31,   31,   31},

	 /* au8SkinGain */
     {127, 127,  127,  127,  127,  135,  145,  155,   165,   175,   185,   195,   205,   205,   205,   205},


};
static ISP_CMOS_DRC_S g_stIspDRC =
{
    /*bEnable*/
    1,   
    /*u8SpatialVar*/
    10,    
	/*u8RangeVar*/             
	3,
	/*u8Asymmetry8*/
    2,
	/*u8SecondPole; */
    150,
	/*u8Stretch*/
    37,
	/*u8Compress*/ 
    180,
	/*u8PDStrength*/ 
    45,
	/*u8LocalMixingBrigtht*/
    15,
    /*u8LocalMixingDark*/
    45,
    /*ColorCorrectionLut[33];*/
    {1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024},


    /*bEnable*/
//    1,
    /*u8SpatialVar*/
//    10,
    /*u8RangeVar*/
//    3,
    /*u8Asymmetry8*/
//    2,
	/*u8SecondPole; */
//    150,
	/*u8Stretch*/
//   37,
	/*u8Compress*/
//    160,
	/*u8PDStrength*/
//    64,
	/*u8LocalMixingBrigtht*/
//    112,
    /*u8LocalMixingDark*/
//    80,
    /*ColorCorrectionLut[33];*/
//   {1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024},
};

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,

#if 0
    {0, 180, 320, 426, 516, 590, 660, 730, 786, 844, 896, 946, 994, 1040, 1090, 1130, 1170, 1210, 1248,
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
#else  /*higher  contrast*/
    {0   ,120 ,220 ,310 ,390 ,470 ,540 ,610 ,670 ,730 ,786 ,842 ,894 ,944 ,994 ,1050,
    1096,1138,1178,1218,1254,1280,1314,1346,1378,1408,1438,1467,1493,1519,1543,1568,
    1592,1615,1638,1661,1683,1705,1726,1748,1769,1789,1810,1830,1849,1869,1888,1907,
    1926,1945,1963,1981,1999,2017,2034,2052,2069,2086,2102,2119,2136,2152,2168,2184,
    2200,2216,2231,2247,2262,2277,2292,2307,2322,2337,2351,2366,2380,2394,2408,2422,
    2436,2450,2464,2477,2491,2504,2518,2531,2544,2557,2570,2583,2596,2609,2621,2634,
    2646,2659,2671,2683,2696,2708,2720,2732,2744,2756,2767,2779,2791,2802,2814,2825,
    2837,2848,2859,2871,2882,2893,2904,2915,2926,2937,2948,2959,2969,2980,2991,3001,
    3012,3023,3033,3043,3054,3064,3074,3085,3095,3105,3115,3125,3135,3145,3155,3165,
    3175,3185,3194,3204,3214,3224,3233,3243,3252,3262,3271,3281,3290,3300,3309,3318,
    3327,3337,3346,3355,3364,3373,3382,3391,3400,3409,3418,3427,3436,3445,3454,3463,
    3471,3480,3489,3498,3506,3515,3523,3532,3540,3549,3557,3566,3574,3583,3591,3600,
    3608,3616,3624,3633,3641,3649,3657,3665,3674,3682,3690,3698,3706,3714,3722,3730,
    3738,3746,3754,3762,3769,3777,3785,3793,3801,3808,3816,3824,3832,3839,3847,3855,
    3862,3870,3877,3885,3892,3900,3907,3915,3922,3930,3937,3945,3952,3959,3967,3974,
    3981,3989,3996,4003,4010,4018,4025,4032,4039,4046,4054,4061,4068,4075,4082,4089,4095}
#endif
};

static ISP_CMOS_DPC_S g_stCmosDpc =
{
	//0,/*IR_channel*/
	//0,/*IR_position*/
	{0,0,0,0,200,200,220,220,220,220,152,152,152,152,152,152},/*au16Strength[16]*/
	{0,0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 50, 50, 50},/*au16BlendRatio[16]*/
};

/***BAYER NR**/
static ISP_CMOS_BAYERNR_S g_stIspBayerNr =
{
	14,     //Calibration Lut Num
	/*************Calibration LUT Table*************/
	{
		{100.000000f, 0.139807f},
		{200.000000f, 0.181612f},
		{400.000000f, 0.237434f},
		{800.000000f, 0.370313f},
		{1550.000000f, 0.573568f},
		{3100.000000f, 1.040012f},
		{6200.000000f, 1.945260f},
		{12400.000000f, 3.814323f},
		{24800.000000f, 3.834439f},
		{49600.000000f, 3.809839f},
		{99200.000000f, 3.840849f},
		{198400.000000f, 3.837305f},
		{396800.000000f, 3.747401f},
		{793600.000000f, 7.705900f}
	},
	/*********************************************/
	{ 140, 110, 110, 140},
	{ 50, 50, 45, 45, 50, 50, 52, 55, 56, 57, 58, 64, 64, 64, 64, 64},     //lutJNLMGain
	{
	  {0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},   //ChromaStrR
      {0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},   //ChromaStrGr
	  {0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},   //ChromaStrGb
	  {0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}    //ChromaStrB
	},
	{ 1200, 1100, 1000, 1000, 1000, 1000, 900, 900, 850, 800, 750, 750, 750, 750, 750, 750},     //lutCoringWeight
	{300, 350, 400, 450, 500, 550, 650, 650, 700, 700, 700, 700, 700, 700, 700, 700, \
           800, 800, 800, 850, 850, 850, 900, 900, 900, 950, 950, 950, 1000, 1000, 1000, 1000, 1000}
};

static ISP_CMOS_CA_S g_stIspCA =
{
    /*enable*/
	1,
	/*YRatioLut*/
    {36 ,81  ,111 ,136 ,158 ,182 ,207 ,228 ,259 ,290 ,317 ,345 ,369 ,396 ,420 ,444 ,
	468 ,492 ,515 ,534 ,556 ,574 ,597 ,614 ,632 ,648 ,666 ,681 ,697 ,709 ,723 ,734 ,
	748 ,758 ,771 ,780 ,788 ,800 ,808 ,815 ,822 ,829 ,837 ,841 ,848 ,854 ,858 ,864 ,
	868 ,871 ,878 ,881 ,885 ,890 ,893 ,897 ,900 ,903 ,906 ,909 ,912 ,915 ,918 ,921 ,
	924 ,926 ,929 ,931 ,934 ,936 ,938 ,941 ,943 ,945 ,947 ,949 ,951 ,952 ,954 ,956 ,
	958 ,961 ,962 ,964 ,966 ,968 ,969 ,970 ,971 ,973 ,974 ,976 ,977 ,979 ,980 ,981 ,
	983 ,984 ,985 ,986 ,988 ,989 ,990 ,991 ,992 ,993 ,995 ,996 ,997 ,998 ,999 ,1000,
	1001,1004,1005,1006,1007,1009,1010,1011,1012,1014,1016,1017,1019,1020,1022,1024},
	/*ISORatio*/
    {1300,1300,1250,1200,1150,1100,1050,1000,1000,1000,900,900,800,800,800,800}
};



HI_U32 sc2235_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
	//memcpy(&pstDef->stLsc.stLscUniParaTable, &g_stCmosLscUniTable, sizeof(ISP_LSC_CABLI_UNI_TABLE_S));
	//memcpy(&pstDef->stLsc.stLscParaTable[0], &g_stCmosLscTable[0], sizeof(ISP_LSC_CABLI_TABLE_S)*HI_ISP_LSC_LIGHT_NUM);
    switch (genSensorMode)
    {
        case WDR_MODE_NONE:
		memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
		memcpy(&pstDef->stYuvSharpen, &g_stIspYuvSharpen, sizeof(ISP_CMOS_YUV_SHARPEN_S));
		memcpy(&pstDef->stDrc, &g_stIspDRC, sizeof(ISP_CMOS_DRC_S));
		memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));
		memcpy(&pstDef->stBayerNr, &g_stIspBayerNr, sizeof(ISP_CMOS_BAYERNR_S));
		memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));
		memcpy(&pstDef->stFcr, &g_stIspFcr, sizeof(ISP_CMOS_FCR_S));
		memcpy(&pstDef->stDpc, &g_stCmosDpc, sizeof(ISP_CMOS_DPC_S));
		memcpy(&pstDef->stCa, &g_stIspCA, sizeof(ISP_CMOS_CA_S));
        break;
		default:
			printf("SC2235_Sensor Mode is error! cmos_get_isp_default Failuer!\n");
		break;
    }


	pstDef->stSensorMaxResolution.u32MaxWidth  = 1920;
	pstDef->stSensorMaxResolution.u32MaxHeight = 1080;

    return 0;
}

HI_U32 sc2235_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
    pstBlackLevel->au16BlackLevel[0] = 0x47;
    pstBlackLevel->au16BlackLevel[1] = 0x47;
    pstBlackLevel->au16BlackLevel[2] = 0x47;
    pstBlackLevel->au16BlackLevel[3] = 0x47;

    return 0;

}


HI_VOID sc2235_set_pixel_detect(HI_BOOL bEnable)
{
	//min fps is 8.23,so u32FullLines_5Fps = FULL_LINES_MAX
	HI_U32 u32FullLines_5Fps, u32MaxIntTime_5Fps;

	if (SC2235_SENSOR_1080P_30FPS_LINEAR_MODE == gu8SensorImageMode)
	{
		u32FullLines_5Fps = FULL_LINES_MAX;
	}
	else
	{
		return;
	}
	//u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
	u32MaxIntTime_5Fps = u32FullLines_5Fps;
	
	if (bEnable) /* setup for ISP pixel calibration mode */
	{
		SENSOR_I2C_WRITE(VMAX0_ADDR, (u32FullLines_5Fps & 0xFF00) >> 8);  /* 5fps */
		SENSOR_I2C_WRITE(VMAX1_ADDR, u32FullLines_5Fps & 0xFF);		   /* 5fps */
		SENSOR_I2C_WRITE(EXPTIME0_ADDR, ((u32MaxIntTime_5Fps >> 4) & 0xFF));	/* max exposure lines */
		SENSOR_I2C_WRITE(EXPTIME1_ADDR, ((u32MaxIntTime_5Fps << 4) & 0xF0));	/* max exposure lines */
		SENSOR_I2C_WRITE(GAIN0_ADDR, 0x00);									/* min Gain = 1*/
		SENSOR_I2C_WRITE(GAIN1_ADDR, 0x03);
		SENSOR_I2C_WRITE(GAIN2_ADDR, 0x10);
	}
	else /* setup for ISP 'normal mode' */
	{
		gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;
		gu32FullLines = gu32FullLinesStd;		
		SENSOR_I2C_WRITE(VMAX0_ADDR, ((gu32FullLinesStd & 0xFF00) >> 8));	/* Standard FPS */
		SENSOR_I2C_WRITE(VMAX1_ADDR, (gu32FullLinesStd & 0xFF));			/* Standard FPS */
		bInit = HI_FALSE;
	}

	return;
}


HI_VOID sc2235_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;

    switch(u8Mode)
    {
		case WDR_MODE_NONE:
            genSensorMode = WDR_MODE_NONE;
            if(SC2235_SENSOR_1080P_30FPS_LINEAR_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = SC2235_VMAX_1080P30_LINEAR;
				printf("SC2235_SENSOR_1080P_30FPS_LINEAR_MODE\n");
            }
		    else
		    {
				printf("cmos_set_wdr_mode_Mode's name is wrong\n");
				return;
		    }
            printf("cmos_set_wdr_mode_linear_mode\n");
            break;
		break;
		default:
		    printf("cmos_set_wdr_mode_NOT support this mode!\n");
            return;
		break;
    }

    gu32FullLines = gu32FullLinesStd;

    return;
}

HI_U32 sc2235_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;


    if (HI_FALSE == bInit)
    {
        printf("cmos_get_sns_regs_info_HI_FALSE == bInit\n");
		g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;
        g_stSnsRegsInfo.u32RegNum = 18;
        for (i = 0; i < g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum  = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr  = EXPTIME0_ADDR;
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum  = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr  = EXPTIME1_ADDR;
	g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum  = 0;
	g_stSnsRegsInfo.astI2cData[2].u32RegAddr  = GAIN1_ADDR;
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum  = 0;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr  = GAIN2_ADDR;
	g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum  = 0;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr  = VMAX0_ADDR;
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum  = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr  = VMAX1_ADDR;
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum  = 1;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr  = 0x3903;
	g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum  = 1;
	g_stSnsRegsInfo.astI2cData[7].u32RegAddr  = 0x3903;
        g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum  = 1;
        g_stSnsRegsInfo.astI2cData[8].u32RegAddr  = 0x3812;
	g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum  = 1;
        g_stSnsRegsInfo.astI2cData[9].u32RegAddr  = 0x3301;
	g_stSnsRegsInfo.astI2cData[10].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[10].u32RegAddr = 0x3631;
        g_stSnsRegsInfo.astI2cData[11].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[11].u32RegAddr = 0x366f;
	g_stSnsRegsInfo.astI2cData[12].u8DelayFrmNum = 1;
	g_stSnsRegsInfo.astI2cData[12].u32RegAddr = 0x3314;
        g_stSnsRegsInfo.astI2cData[13].u8DelayFrmNum = 2;
        g_stSnsRegsInfo.astI2cData[13].u32RegAddr = 0x5781;
	g_stSnsRegsInfo.astI2cData[14].u8DelayFrmNum = 2;
        g_stSnsRegsInfo.astI2cData[14].u32RegAddr = 0x5785;
	g_stSnsRegsInfo.astI2cData[15].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[15].u32RegAddr = 0x3812;

	g_stSnsRegsInfo.astI2cData[16].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[16].u32RegAddr = 0x3e00;
		
	g_stSnsRegsInfo.astI2cData[17].u8DelayFrmNum = 1;
		g_stSnsRegsInfo.astI2cData[17].u32RegAddr = 0x3622;


		bInit = HI_TRUE;
    }
    else
    {
        for (i = 0; i < 6; i++)                                //AEC/AGC_Update
        {
            if (g_stSnsRegsInfo.astI2cData[i].u32Data == g_stPreSnsRegsInfo.astI2cData[i].u32Data)
            {
                g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_FALSE;
            }
            else
            {

				g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            }
        }
		if( HI_TRUE == (g_stSnsRegsInfo.astI2cData[0].bUpdate ||  g_stSnsRegsInfo.astI2cData[1].bUpdate || g_stSnsRegsInfo.astI2cData[2].bUpdate ||  g_stSnsRegsInfo.astI2cData[3].bUpdate))
		{
			g_stSnsRegsInfo.astI2cData[6].bUpdate  = HI_TRUE;    //Grouphold_AEC/AGC_Logic
			g_stSnsRegsInfo.astI2cData[7].bUpdate  = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[8].bUpdate  = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[9].bUpdate  = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[10].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[11].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[12].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[13].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[14].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[15].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[16].bUpdate = HI_TRUE;
		}
        else
        {
            g_stSnsRegsInfo.astI2cData[6].bUpdate  = HI_FALSE;    //Grouphold_AEC/AGC_Logic
        	g_stSnsRegsInfo.astI2cData[7].bUpdate  = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[8].bUpdate  = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[9].bUpdate  = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[10].bUpdate = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[11].bUpdate = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[12].bUpdate = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[13].bUpdate = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[14].bUpdate = HI_FALSE;
        	g_stSnsRegsInfo.astI2cData[15].bUpdate = HI_FALSE;
		g_stSnsRegsInfo.astI2cData[16].bUpdate = HI_FALSE;

        }
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



static HI_S32 sc2235_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode;
    bInit = HI_FALSE;

    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if ((pstSensorImageMode->u16Width <= 1920) && (pstSensorImageMode->u16Height <= 1080))
    {

        if (pstSensorImageMode->f32Fps <= 30)
        {
			u8SensorImageMode = SC2235_SENSOR_1080P_30FPS_LINEAR_MODE;
			printf("cmos_set_image_mode_SC2235_SENSOR_1080P_30FPS_LINEAR_MODE\n");
			if(WDR_MODE_NONE == genSensorMode)
            {
                gu32FullLinesStd = SC2235_VMAX_1080P30_LINEAR;
            }
            else
            {
            	printf("=======Not Support This Mode=======");
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
		printf("Sensor first init\n");
       	return 0;
    }

    /* Switch SensorImageMode */
    if (u8SensorImageMode == gu8SensorImageMode)
    {
        /* Don't need to switch SensorImageMode */
       	return -1;
    }

	gu8SensorImageMode = u8SensorImageMode;
	gu32FullLines = gu32FullLinesStd;

    return 0;
}

int  sc2235_set_inifile_path(const char *pcPath)
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

HI_VOID sc2235_global_init()
{
	gu8SensorImageMode = SC2235_SENSOR_1080P_30FPS_LINEAR_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = SC2235_VMAX_1080P30_LINEAR;
    gu32FullLines = SC2235_VMAX_1080P30_LINEAR;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE;

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));

	g_stSnsRegsInfo.astI2cData[0].u32Data  = 0x00;
	g_stSnsRegsInfo.astI2cData[1].u32Data  = 0x10;
	g_stSnsRegsInfo.astI2cData[2].u32Data  = 0x03;
	g_stSnsRegsInfo.astI2cData[3].u32Data  = 0x10;
	g_stSnsRegsInfo.astI2cData[4].u32Data  = 0x04;
	g_stSnsRegsInfo.astI2cData[5].u32Data  = 0x65;
	g_stSnsRegsInfo.astI2cData[6].u32Data  = 0x84;
	g_stSnsRegsInfo.astI2cData[7].u32Data  = 0x04;
	g_stSnsRegsInfo.astI2cData[8].u32Data  = 0x00;
	g_stSnsRegsInfo.astI2cData[9].u32Data  = 0x05;
	g_stSnsRegsInfo.astI2cData[10].u32Data = 0x84;
	g_stSnsRegsInfo.astI2cData[11].u32Data = 0x2F;
	g_stSnsRegsInfo.astI2cData[12].u32Data = 0x02;
	g_stSnsRegsInfo.astI2cData[13].u32Data = 0x04;
	g_stSnsRegsInfo.astI2cData[14].u32Data = 0x18;
	g_stSnsRegsInfo.astI2cData[15].u32Data = 0x30;
	g_stSnsRegsInfo.astI2cData[16].u32Data = 0x0;
    return;

}


int sc2235_linear_1080p30_init()
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
	SENSOR_I2C_WRITE(0x3677,0x3f); //<gain0
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

	SENSOR_I2C_WRITE(0x3306,0x44);
	SENSOR_I2C_WRITE(0x330b,0xcb);
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
	//SENSOR_I2C_WRITE(0x3e03,0x0b);
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
	SENSOR_I2C_WRITE(0x3802,0x01);//zg add for 3fps.AERunInterval1:0x01;AERunInterval3:0x00
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
	SENSOR_I2C_WRITE(0x3306,0x44);
	SENSOR_I2C_WRITE(0x330b,0xcb);
	SENSOR_I2C_WRITE(0x3639,0x09);

	//manual DPC
	SENSOR_I2C_WRITE(0x5780,0xff);
	SENSOR_I2C_WRITE(0x5781,0x04);
	SENSOR_I2C_WRITE(0x5785,0x18);

	SENSOR_I2C_WRITE(0x3313,0x05);
	SENSOR_I2C_WRITE(0x3678,0x42);

	SENSOR_I2C_WRITE(0x3237,0x05);
	SENSOR_I2C_WRITE(0x3238,0x00);
	SENSOR_I2C_WRITE(0x330b,0xc8);
	SENSOR_I2C_WRITE(0x3306,0x40);


	SENSOR_I2C_WRITE(0x3802,0x00);
	SENSOR_I2C_WRITE(0x3e01,0x00);
	SENSOR_I2C_WRITE(0x3e02,0x10);
	SENSOR_I2C_WRITE(0x3e03,0x0b);

	SENSOR_I2C_WRITE(0x3e07,0x00);
	SENSOR_I2C_WRITE(0x3e08,0x03);
	SENSOR_I2C_WRITE(0x3e09,0x10);

	SENSOR_I2C_WRITE(0x3d08,0x01);
	SENSOR_I2C_WRITE(0x3641,0x01);

	SENSOR_I2C_WRITE(0x3237,0x05);
    SENSOR_I2C_WRITE(0x3238,0x00);

	SENSOR_I2C_WRITE(0x0100,0x01);
    bSensorInit = HI_TRUE;
    printf("=========================================================\n");
    printf("===== SC2235 sensor linear 30fps init success!====\n");
    printf("=========================================================\n");
    return 0;
}

void sc2235_reg_init()
{
    switch (gu8SensorImageMode)
    {
		case 0:
			sc2235_linear_1080p30_init();
		break;
		default:
            printf("SC2235_SENSOR_CTL_Not support this mode\n");
            bSensorInit = HI_FALSE;
		break;
    }
   return ;
}

HI_S32 sc2235_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = sc2235_reg_init;//sc2235_wdr_1080p30_init;//sc2235_wdr_1080p30_init //sc2235_linear_1080p30_init
   // pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
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
        printf("sensor register callback function to awb lib failed!\n");
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
        printf("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }

    return 0;
}

static bool sensor_mirror = false;
static bool sensor_flip = false;

int SC2235_set_mirror(bool mirror)
{
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
	return 0;
}

void SmartSens_SC2235_init(SENSOR_SMARTSENS_SC2235_DO_I2CRD do_i2c_read,
	SENSOR_SMARTSENS_SC2235_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	sc2235_reg_init();

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
	printf("SC2235 sensor 1080P30fps init success!\n");

}

bool SENSOR_SC2235_probe()
{
	uint16_t ret_data1 = 0;
	uint16_t ret_data2 = 0;

	sc2235_i2c_read(0x3107, &ret_data1);
	sc2235_i2c_read(0x3108, &ret_data2);
	if(ret_data1 == SC2235_CHECK_DATA_LSB && ret_data2 == SC2235_CHECK_DATA_MSB){

		sdk_sys->write_reg(0x1204002C, 0x3);/*I2C0_SDA*/
		sdk_sys->write_reg(0x12040030, 0x3);/*I2C0_SCL*/

		sdk_sys->write_reg(0x12040040,0x1); /*VI_DATA13*/
		sdk_sys->write_reg(0x12040044,0x1); /*VI_DATA10*/
		sdk_sys->write_reg(0x12040048,0x1); /*VI_DATA12*/
		sdk_sys->write_reg(0x1204004C,0x1); /*VI_DATA11*/
		sdk_sys->write_reg(0x12040050,0x1); /*VI_DATA9*/
		sdk_sys->write_reg(0x12040054,0x1); /*VI_DATA14*/
		sdk_sys->write_reg(0x12040058,0x1); /*VI_DATA15*/
		sdk_sys->write_reg(0x1204005C,0x1); /*VI_VS*/
		sdk_sys->write_reg(0x12040060,0x1); /*VI_HS*/
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

#endif // __AR_CMOS_H_


