#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_i2c.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SC2232_CHECK_DATA_LSB	(0x22)
#define SC2232_CHECK_DATA_MSB	(0x32)//
#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define FRAME_LINES_2M_1080p  (1125+INCREASE_LINES)


#define SENSOR_NAME "sc2232"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x60, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x60, 2, 1))


#define SENSOR_SC2232_WIDTH (1920)
#define SENSOR_SC2232_HEIGHT  (1080)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__SC2232_CMOS_H_)
#define __SC2232_CMOS_H_

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

#define SC2232_ID 2232
#define SENSOR_2M_1080p25_MODE  (1)
#define SENSOR_2M_1080p30_MODE  (2)
static int g_fd = -1;

/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr=0x60;
static unsigned int sensor_addr_byte=2;
static unsigned int sensor_data_byte=1;

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/

#define FULL_LINES_MAX  (0xFFF)

HI_U8 gu8SensorImageMode = SENSOR_2M_1080p25_MODE;
WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = FRAME_LINES_2M_1080p;
static HI_U32 gu32FullLines = FRAME_LINES_2M_1080p;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};


/* AE default parameter and function */
static HI_S32 sc2232_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 2 * 25 / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 50 * 256;
    pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;
    pstAeSnsDft->u8AERunInterval  = 1;
    pstAeSnsDft->u32InitExposure  = 10;
	
    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 2;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;
    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;
	
    switch(genSensorMode)
    {
        case WDR_MODE_NONE:   /*linear mode*/
            pstAeSnsDft->au8HistThresh[0] = 0x0D;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x40;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd * 2 - 4;
            pstAeSnsDft->u32MinIntTime = 2;    
            pstAeSnsDft->u32MaxIntTimeTarget = 65534;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 15872;  //15.5
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
            
            pstAeSnsDft->u32MaxDgain = 32640;  //31.875
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
			pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;            
		break;
        default:
        break;
    }
    
    return 0;
}

/* the function of sensor set fps */
static HI_VOID sc2232_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	HI_U32 u32VMAX = FRAME_LINES_2M_1080p;

    if (SENSOR_2M_1080p25_MODE == gu8SensorImageMode)
    {
        if ((f32Fps <= 25) && (f32Fps > 8.23))
        {
            u32VMAX = FRAME_LINES_2M_1080p * 25 / f32Fps; 
        }
        else
        {
            printf("Not support Fps: %f\n", f32Fps);
            return;
        }        
    }
    else if (SENSOR_2M_1080p30_MODE == gu8SensorImageMode)
    {
        if ((f32Fps <= 30) && (f32Fps > 8.23))
        {
            u32VMAX = FRAME_LINES_2M_1080p * 30 / f32Fps;
        }
        else
        {
            printf("Not support Fps: %f\n", f32Fps);
            return;
        }
    }
	u32VMAX = (u32VMAX > FULL_LINES_MAX) ? FULL_LINES_MAX : u32VMAX;
	gu32FullLinesStd = u32VMAX;
	g_stSnsRegsInfo.astI2cData[7].u32Data = ((gu32FullLinesStd & 0xFF00) >> 8);  //DecreaseVMaxtoSlowFramerate
    g_stSnsRegsInfo.astI2cData[8].u32Data = (gu32FullLinesStd & 0xFF);

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 2 * f32Fps / 2;                                                        
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;  
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd * 2 - 4;
    gu32FullLines = gu32FullLinesStd;   
    pstAeSnsDft->u32FullLines = gu32FullLines; 

    return;
}


static HI_VOID sc2232_slow_framerate_set(HI_U32 u32FullLines,AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;

	g_stSnsRegsInfo.astI2cData[7].u32Data = ((u32FullLines & 0xFF00) >> 8);
	g_stSnsRegsInfo.astI2cData[8].u32Data = (u32FullLines & 0xFF);

    pstAeSnsDft->u32FullLines = gu32FullLines;
    pstAeSnsDft->u32MaxIntTime = gu32FullLines * 2 - 4;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_U32 u32OldIntTime = 0;
static HI_VOID sc2232_inttime_update(HI_U32 u32IntTime)
{
	 g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 12) & 0x0F;
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime >>  4) & 0xFF;
    g_stSnsRegsInfo.astI2cData[2].u32Data = (u32IntTime <<  4) & 0xF0;

	if (u32IntTime < 0x50)
	{
		g_stSnsRegsInfo.astI2cData[13].u32Data = 0x14;
	}
	else if (u32IntTime > 0xa0)
	{
		g_stSnsRegsInfo.astI2cData[13].u32Data = 0x04;
	}

    return;
}



static const HI_U16 u16AgainTab[64]={
	1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,
	2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,
	4096,4352,4608,4864,5120,5376,5632,5888,6144,6400,6656,6912,7168,7424,7680,7936,
	8192,8704,9216,9728,10240,10752,11264,11776,12288,12800,13312,13824,14336,14848,15360,15872
};

static HI_VOID sc2232_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
	int i;
	int Again = *pu32AgainLin;
	static HI_U8 AgainMaxIndex = 63;
	//printf("Again = %d\n", Again);
	if(Again >= u16AgainTab[AgainMaxIndex])
	{
		*pu32AgainDb = AgainMaxIndex;
	}
	else
	{
		for(i = 1; i < 64; i++)
		{
			if(Again < u16AgainTab[i])
			{
				*pu32AgainDb = i - 1;
				break;
			}
		}
	}
	*pu32AgainLin = u16AgainTab[*pu32AgainDb];
	
    return;
}

static HI_VOID sc2232_dgain_calc_table(HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
	// *pu32DgainDb = [31:16]Dgain + [15:8]u8Reg0x3e07 + [7:0]u8Reg0x3e06
	HI_U8 u8Reg0x3e07, u8Reg0x3e06;
	int Coarseindex, Fineindex;	
	HI_U32 Dgain = (*pu32DgainLin > 32640) ? 32640 : *pu32DgainLin;             //32640 = 16 * 2040 =16 * (2-1/128) * 1024
	//===========================================
	//X16            32640 32512---16384
	//X8      16383  16320 16256---8192
	//X4       8191  8160  8128 ---4096
	//X2       4095  4080  2064 ---2048
	//X1       2047  2040  2032 ---1024
	//===========================================
	for(Coarseindex = 1; Coarseindex <= 16; Coarseindex = Coarseindex * 2)      //1,2,4,8,16
	{
		if(Dgain <= (2048 * Coarseindex - 1))       //HisiDefault_HIGH2LOW      //2048 = 2stStage_DGainFine_Small
		{
			break;
		}
	}
	Fineindex = Dgain * 128.0 / Coarseindex / 1024;                             //[int]128~255:0x80~0xFF
	u8Reg0x3e06 = Coarseindex - 1;
	u8Reg0x3e07 = Fineindex;
	
	Dgain = Coarseindex * 1024.0 * Fineindex / 128;
	*pu32DgainLin = Dgain;
	*pu32DgainDb =  (Dgain << 16) | (u8Reg0x3e07 << 8) | u8Reg0x3e06;
	
    return;
}

static HI_VOID sc2232_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
	HI_U8 i, leftnum, u8Reg0x3e09, u8Reg0x3e08, u8Reg0x3e07, u8Reg0x3e06;
	HI_U32 GainAD = u16AgainTab[u32Again] * (u32Dgain >> 16);               //RealGain*1024*1024

	// Again
	u8Reg0x3e09 = 0x10 | (u32Again & 0x0f);
	u8Reg0x3e08 = 0x03;
	leftnum = u32Again / 16;
	for(i = 0; i <  leftnum; i++)
	{
		u8Reg0x3e08 = (u8Reg0x3e08 << 1) | 0x01;
	}
	// Dgain
	u8Reg0x3e07 = ((u32Dgain & 0xFF00) >> 8);
	u8Reg0x3e06 = u32Dgain & 0xFF;
	//ALL
	g_stSnsRegsInfo.astI2cData[3].u32Data = u8Reg0x3e06;
	g_stSnsRegsInfo.astI2cData[4].u32Data = u8Reg0x3e07;
	g_stSnsRegsInfo.astI2cData[5].u32Data = u8Reg0x3e08;
	g_stSnsRegsInfo.astI2cData[6].u32Data = u8Reg0x3e09;
	
	g_stSnsRegsInfo.astI2cData[9].u32Data = 0x00;
	//Logic
	if (GainAD < 2 * 1024 * 1024)
    {
        g_stSnsRegsInfo.astI2cData[10].u32Data = 0x06;
        g_stSnsRegsInfo.astI2cData[11].u32Data = 0x48;
        g_stSnsRegsInfo.astI2cData[12].u32Data = 0x08;
    }
    else if(GainAD < 4 * 1024 * 1024)
    {
        g_stSnsRegsInfo.astI2cData[10].u32Data = 0x14;
        g_stSnsRegsInfo.astI2cData[11].u32Data = 0x48;
        g_stSnsRegsInfo.astI2cData[12].u32Data = 0x08;
    }
	else if(GainAD < 8 * 1024 * 1024)
	{
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x18;
        g_stSnsRegsInfo.astI2cData[11].u32Data = 0x48;
        g_stSnsRegsInfo.astI2cData[12].u32Data = 0x08;
	}
	else if(GainAD < 15.5 * 1024 * 1024)
	{
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x13;
        g_stSnsRegsInfo.astI2cData[11].u32Data = 0x48;
        g_stSnsRegsInfo.astI2cData[12].u32Data = 0x08;
	}
	else if(GainAD < 31 * 1024 * 1024)
	{
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0xf9;
        g_stSnsRegsInfo.astI2cData[11].u32Data = 0x78;
        g_stSnsRegsInfo.astI2cData[12].u32Data = 0x48;
	}
	else
	{
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0xf9;
        g_stSnsRegsInfo.astI2cData[11].u32Data = 0x78;
        g_stSnsRegsInfo.astI2cData[12].u32Data = 0x78;
	}
	g_stSnsRegsInfo.astI2cData[14].u32Data = 0x30;
	
	if ((u8Reg0x3e08 < 0x1f) || (0x1f == u8Reg0x3e08 && u8Reg0x3e09 < 0x14))//<0x1f14	gain<10
	{
		SENSOR_I2C_WRITE(0x5781,0x04);
		SENSOR_I2C_WRITE(0x5785,0x18);
	}
	else//>=0x1f14	gain>=10
	{
		SENSOR_I2C_WRITE(0x5781,0x02);
		SENSOR_I2C_WRITE(0x5785,0x08);
	}

	return; 
}



HI_S32 sc2232_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));
    pstExpFuncs->pfn_cmos_get_ae_default    = sc2232_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = sc2232_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= sc2232_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = sc2232_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = sc2232_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = sc2232_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = sc2232_dgain_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = NULL; //cmos_get_inttime_max;
    pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = NULL; //cmos_ae_fswdr_attr_set;
    return 0;
}


/* AWB default parameter and function */
static AWB_CCM_S g_stAwbCcm =
{
	6500,
	{
		0x1C1,0x80AA,0x8017,
		0x8066,0x1A6,0x8040,
		0x4,0x8198,0x294,
	},
	4100,
	{
		0x193,0x8087,0x800C,
		0x8080,0x191,0x8011,
		0x8019,0x81A6,0x2BF,
	},
	2856,
	{
		0x18C,0x8068,0x8024,
		0x8078,0x16A,0xE,
		0x8045,0x8257,0x39C
	}
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
	/* bvalid */
	1,

	/*1,  2,  4,  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768*/
	/* saturation */   
	{0x74,0x74,0x74,0x74,0x6c,0x6c,0x3C,0x3C,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00}
};


static HI_S32 sc2232_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
   if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

	pstAwbSnsDft->u16WbRefTemp = 5082;
	pstAwbSnsDft->au16GainOffset[0] = 0x172;	
	pstAwbSnsDft->au16GainOffset[1] = 0x100;	
	pstAwbSnsDft->au16GainOffset[2] = 0x100;	
	pstAwbSnsDft->au16GainOffset[3] = 0x158;	
	pstAwbSnsDft->as32WbPara[0] = 129;	  
	pstAwbSnsDft->as32WbPara[1] = -31;    
	pstAwbSnsDft->as32WbPara[2] = -158;    
	pstAwbSnsDft->as32WbPara[3] = 200259;	 
	pstAwbSnsDft->as32WbPara[4] = 128;	  
	pstAwbSnsDft->as32WbPara[5] = -154475;
    
    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_S32 sc2232_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
    pstExpFuncs->pfn_cmos_get_awb_default = sc2232_get_awb_default;
    return 0;
}

#define DMNR_CALIB_CARVE_NUM_SC2232 8



static HI_FLOAT g_coef_calib_SC2232[DMNR_CALIB_CARVE_NUM_SC2232][4] = 
{
	{100.000000f, 2.000000f, 0.039561f, 6.095408f, }, 
	{200.000000f, 2.301030f, 0.040519f, 6.380517f, }, 
	{400.000000f, 2.602060f, 0.042396f, 6.888251f, }, 
	{800.000000f, 2.903090f, 0.046129f, 7.650892f, }, 
	{1600.000000f, 3.204120f, 0.051765f, 9.290079f, }, 
	{3200.000000f, 3.505150f, 0.058894f, 12.351269f, }, 
	{6200.000000f, 3.792392f, 0.074431f, 18.024876f, }, 
	{12400.000000f, 3.792392f, 0.095554f, 27.922104f, }, 
};


static ISP_NR_ISO_PARA_TABLE_S g_stNrIsoParaTab[HI_ISP_NR_ISO_LEVEL_MAX] = 
{
	//u16Threshold//u8varStrength//u8fixStrength//u8LowFreqSlope	
	{0x5dc,       0xa8,         0x6b,         0x2 },  //100    //                      //                                                
	{0x5dc,       0xa8,         0x6b,         0x2 },  //200    // ISO                  // ISO //u8LowFreqSlope
	{0x5dc,       0x8c,         0x64,         0x2 },  //400    //{400,  1200, 96,256}, //{400 , 0  }
	{0x5dc,       0x78,         0x50,         0x2 },  //800    //{800,  1400, 80,256}, //{600 , 2  }
	{0x5dc,       0x78,         0x50,         0x3 },  //1600   //{1600, 1200, 72,256}, //{800 , 8  }
	{0x5dc,       0x64,         0x40,         0x3 },  //3200   //{3200, 1200, 64,256}, //{1000, 12 }
	{0x5dc,       0x4b,         0x40,         0x3 },  //6400   //{6400, 1100, 56,256}, //{1600, 6  }
	{0x5dc,       0x1c,         0x40,         0x3 },  //12800  //{12000,1100, 48,256}, //{2400, 0  }
	{0x5dc,       0x1c,         0x40,         0x3 },  //25600  //{36000,1100, 48,256}, //
	{0x5dc,       0x1c,         0x40,         0x3 },  //51200  //{64000,1100, 96,256}, //
	{0,       	 0,            0,            0   },  //102400 //{82000,1000,240,256}, //
	{0,       	 0,            0,            0 },  //204800 //                           //
	{0,       	 0,            0,            0 },  //409600 //                           //
	{0,       	 0,            0,            0 },  //819200 //                           //
	{0,       	 0,            0,            0 },  //1638400//                           //
	{0,       	 0,            0,            0 },  //3276800//							 //
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
	{512,512,512,512,512,512,512,400,0,0,0,0,0,0,0,0}    /*au16NpOffset[ISP_AUTO_ISO_STENGTH_NUM]*/
};


static ISP_CMOS_GE_S g_stIspGe =
{
	/*For GE*/
	1,    /*bEnable*/			
	7,    /*u8Slope*/	
	7,    /*u8Sensitivity*/
	8192, /*u16Threshold*/
	8192, /*u16SensiThreshold*/	
	{1024,1024,1024,2048,2048,2048,2048,  2048,  2048,2048,2048,2048,2048,2048,2048,2048}    /*au16Strength[ISP_AUTO_ISO_STENGTH_NUM]*/	
};

static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
	//{100,200,400,800,1600,3200,6400,12800,25600,51200,102400,204800,409600,819200,1638400,3276800};
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},/* enPixSel = ~bEnLowLumaShoot */

	{0x78,0x78,0x72,0x66,0x66,0x4c,0x3c,0x20,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*maxSharpAmt1 = SharpenUD*16 */
	{0x92,0x89,0x7c,0x7c,0x7c,0x4E,0x3a,0x34,0x1a,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*maxEdgeAmt = SharpenD*16 */

	{0x2,0x8,0xc,0x10,0x1c,0x20,0x38,0x36,0x56,0x0,0x00,0x00,0x00,0x00,0x00,0x00},/*sharpThd2 = TextureNoiseThd*4 */
	{0x2,0x8,0xc,0x10,0x1c,0x20,0x20,0x30,0x4c,0x0,0x00,0x00,0x00,0x00,0x00,0x00},/*edgeThd2 = EdgeNoiseThd*4 */

	{0x5c,0x57,0x57,0x57,0x57,0x4a,0x30,0x1c,0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /*overshootAmt*/
	{0x97,0x78,0x72,0x72,0x72,0x46,0x32,0x1e,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*undershootAmt*/
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
	{0,0,0,1,1,1,2,2,2,3,3,3,3,3,3,3},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0,0x24,0x80,0x80,0x80,0xE5,0xE5,0xE5,0xE5},/*au16BlendRatio[16]*/
};

static ISP_CMOS_GAMMAFE_S g_stGammafe = 
{
	/* bvalid */
	1,
	
	/* gamma_fe0 */
	{	   
		//10,8//
		
		/*0	 ,	43008,	46130,	49152,	51200,	52224,	52717,	53210,	53703,	54196,	54689,	55182,	55675,	56168,	56661,	57154,	57647,	58140,	58633,	59127,	59620,	60113,	60606,	61099,	61592,	62085,	62578,	63071,	63564,	64057,	64550,	65043,	65535*/

		//14,12
		0,43008,46130,49152,51200,52224,52717,53210,53703,54196,54689,55182,55675,56168,56661,57154,57647,58140,58633,59127,59620,60113,60606,61099,61592,62085,62578,63071,63564,64057,64550,65043,65535
	},

	/* gamma_fe1 */
	{
		//16,14	
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

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
	1,
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
};


HI_U32 sc2232_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
   if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
	
    switch (genSensorMode)
    {
        case WDR_MODE_NONE:
            memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));
			pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_SC2232;//DMNR_CALIB_CARVE_NUM_AR0230;
			pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_SC2232;
			memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);
			memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));
			memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
			memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
			memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));
        break;
		default:
		break;
    }

    pstDef->stSensorMaxResolution.u32MaxWidth  = 1920;
    pstDef->stSensorMaxResolution.u32MaxHeight = 1080;

    return 0;
}

HI_U32 sc2232_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
   if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
	pstBlackLevel->bUpdate = HI_FALSE;
	pstBlackLevel->au16BlackLevel[0] = 68;
	pstBlackLevel->au16BlackLevel[1] = 68;
	pstBlackLevel->au16BlackLevel[2] = 68;
	pstBlackLevel->au16BlackLevel[3] = 68;

    return 0;
    
}

HI_VOID sc2232_set_pixel_detect(HI_BOOL bEnable)
{
	  printf("-----------------cmos_set_pixel_detect\n");
    HI_U32 u32FullLines_5Fps = FRAME_LINES_2M_1080p * 25 / 5;
    HI_U32 u32MaxExpTime_5Fps = u32FullLines_5Fps*2 - 4;

    u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
    u32MaxExpTime_5Fps = u32FullLines_5Fps*2 - 4; 
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
		SENSOR_I2C_WRITE(0x320E, (u32FullLines_5Fps & 0xFF00) >> 8);    /* 5fps */
        SENSOR_I2C_WRITE(0x320F, (u32FullLines_5Fps & 0xFF));             /* 5fps */
        SENSOR_I2C_WRITE(0x3E00, ((u32MaxExpTime_5Fps >> 12) & 0x0F));  /* max exposure lines */
		SENSOR_I2C_WRITE(0x3E01, ((u32MaxExpTime_5Fps >> 4) & 0xFF));   /* max exposure lines */
        SENSOR_I2C_WRITE(0x3E02, ((u32MaxExpTime_5Fps << 4) & 0xF0));   /* max exposure lines */
		SENSOR_I2C_WRITE(0x3E06, 0x00);                                 /* min Gain = 1*/
		SENSOR_I2C_WRITE(0x3E07, 0x80);
		SENSOR_I2C_WRITE(0x3E08, 0x03);
        SENSOR_I2C_WRITE(0x3E09, 0x10);
    }
    else /* setup for ISP 'normal mode' */
    {
        gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;
        gu32FullLines = gu32FullLinesStd;		
		SENSOR_I2C_WRITE(0x320E, ((gu32FullLinesStd & 0xFF00) >> 8));  /* Standard FPS */
		SENSOR_I2C_WRITE(0x320F, (gu32FullLinesStd & 0xFF));           /* Standard FPS */
		bInit = HI_FALSE;
    }

    return;
}

HI_VOID sc2232_set_wdr_mode(HI_U8 u8Mode)
{
     bInit = HI_FALSE;

    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            genSensorMode = WDR_MODE_NONE;
			gu32FullLinesStd = FRAME_LINES_2M_1080p;
        break;
        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
	
	gu32FullLines = gu32FullLinesStd;
	
    return;
}

HI_U32 sc2232_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
 	HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;
        g_stSnsRegsInfo.u32RegNum = 15;//hisi���ļ�Ĭ�ϴ�������󳤶�Ϊ16����Ҫ�޸�hisi��궨��
        for (i=0; i < g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;     //Init
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3e00;
		g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3e01;
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3e02;
		
		g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 0;     //Gain
		g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3e06;
		g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x3e07;
		g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x3e08;
		g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x3e09;
		
		g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 0;     //Vmax
        g_stSnsRegsInfo.astI2cData[7].u32RegAddr = 0x320e;
		g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[8].u32RegAddr = 0x320f;
		
		g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 1;
		g_stSnsRegsInfo.astI2cData[9].u32RegAddr = 0x3812;    //Grouphold_Logic
		g_stSnsRegsInfo.astI2cData[10].u8DelayFrmNum = 1;
		g_stSnsRegsInfo.astI2cData[10].u32RegAddr = 0x3301;
		g_stSnsRegsInfo.astI2cData[11].u8DelayFrmNum = 1;
		g_stSnsRegsInfo.astI2cData[11].u32RegAddr = 0x3306;
		g_stSnsRegsInfo.astI2cData[12].u8DelayFrmNum = 1;
		g_stSnsRegsInfo.astI2cData[12].u32RegAddr = 0x3632;
		g_stSnsRegsInfo.astI2cData[13].u8DelayFrmNum = 1;
		g_stSnsRegsInfo.astI2cData[13].u32RegAddr = 0x3314;
		g_stSnsRegsInfo.astI2cData[14].u8DelayFrmNum = 1;
		g_stSnsRegsInfo.astI2cData[14].u32RegAddr = 0x3812;
        bInit = HI_TRUE;
    }
    else
    {
        for (i = 0; i < g_stSnsRegsInfo.u32RegNum; i++)
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
		
		if( HI_TRUE == (g_stSnsRegsInfo.astI2cData[13].bUpdate))
		{
			g_stSnsRegsInfo.astI2cData[9].bUpdate  = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[13].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[14].bUpdate = HI_TRUE;
		}
		else if( HI_TRUE == (g_stSnsRegsInfo.astI2cData[10].bUpdate ||  g_stSnsRegsInfo.astI2cData[11].bUpdate || g_stSnsRegsInfo.astI2cData[12].bUpdate))
		{
			g_stSnsRegsInfo.astI2cData[9].bUpdate  = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[10].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[11].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[12].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[14].bUpdate = HI_TRUE;
		}
		else
		{
			g_stSnsRegsInfo.astI2cData[9].bUpdate  = HI_FALSE;
			g_stSnsRegsInfo.astI2cData[10].bUpdate = HI_FALSE;
			g_stSnsRegsInfo.astI2cData[11].bUpdate = HI_FALSE;
			g_stSnsRegsInfo.astI2cData[12].bUpdate = HI_FALSE;
			g_stSnsRegsInfo.astI2cData[14].bUpdate = HI_FALSE;
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



static HI_S32 sc2232_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode;
    
    bInit = HI_FALSE;    
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if((pstSensorImageMode->u16Width <= 1920)&&(pstSensorImageMode->u16Height <= 1080))
    {
        if (pstSensorImageMode->f32Fps <= 25)
        {
            u8SensorImageMode = SENSOR_2M_1080p25_MODE;
			if(WDR_MODE_NONE == genSensorMode)
            {
                gu32FullLinesStd = FRAME_LINES_2M_1080p;
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


static void sc2232_reg_init(){
	printf("===Sc2232_1080p_27Minput_74.25M_25fps.ini modify 20180131! ===\n");
	SENSOR_I2C_WRITE(0x0103,0x01);
	SENSOR_I2C_WRITE(0x0100,0x00);
	SENSOR_I2C_WRITE(0x3018,0x1f);
	SENSOR_I2C_WRITE(0x3019,0xff);
	SENSOR_I2C_WRITE(0x301c,0xb4);
	usleep(1000);
	SENSOR_I2C_WRITE(0x320c,0x0a);
	SENSOR_I2C_WRITE(0x320d,0x50);
	SENSOR_I2C_WRITE(0x3e01,0x23);
	SENSOR_I2C_WRITE(0x363c,0x05);
	SENSOR_I2C_WRITE(0x3635,0xa8);
	SENSOR_I2C_WRITE(0x363b,0x0d);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3620,0x08);
	SENSOR_I2C_WRITE(0x3622,0x02);
	SENSOR_I2C_WRITE(0x3635,0xc0);
	SENSOR_I2C_WRITE(0x3908,0x10);
	SENSOR_I2C_WRITE(0x3624,0x08);
	SENSOR_I2C_WRITE(0x5000,0x06);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3e06,0x00);
	SENSOR_I2C_WRITE(0x3e08,0x03);
	SENSOR_I2C_WRITE(0x3e09,0x10);
	SENSOR_I2C_WRITE(0x3333,0x10);
	SENSOR_I2C_WRITE(0x3306,0x7e);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3902,0x05);
	SENSOR_I2C_WRITE(0x3213,0x08);
	SENSOR_I2C_WRITE(0x337f,0x03);
	SENSOR_I2C_WRITE(0x3368,0x04);
	SENSOR_I2C_WRITE(0x3369,0x00);
	usleep(1000);
	SENSOR_I2C_WRITE(0x336a,0x00);
	SENSOR_I2C_WRITE(0x336b,0x00);
	SENSOR_I2C_WRITE(0x3367,0x08);
	SENSOR_I2C_WRITE(0x330e,0x30);
	SENSOR_I2C_WRITE(0x3366,0x7c);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3633,0x42);
	SENSOR_I2C_WRITE(0x330b,0xe0);
	SENSOR_I2C_WRITE(0x3637,0x57);
	SENSOR_I2C_WRITE(0x3302,0x1f);
	SENSOR_I2C_WRITE(0x3309,0xde);
	usleep(1000);
	SENSOR_I2C_WRITE(0x303f,0x81);
	SENSOR_I2C_WRITE(0x3907,0x00);
	SENSOR_I2C_WRITE(0x3908,0x61);
	SENSOR_I2C_WRITE(0x3902,0x45);
	SENSOR_I2C_WRITE(0x3905,0xb8);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3e01,0x8c);
	SENSOR_I2C_WRITE(0x3e02,0x10);
	SENSOR_I2C_WRITE(0x3e06,0x00);
	SENSOR_I2C_WRITE(0x3038,0x48);
	SENSOR_I2C_WRITE(0x3637,0x5d);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3e06,0x00);
	SENSOR_I2C_WRITE(0x3908,0x11);
	SENSOR_I2C_WRITE(0x335e,0x01);
	SENSOR_I2C_WRITE(0x335f,0x03);
	SENSOR_I2C_WRITE(0x337c,0x04);
	usleep(1000);
	SENSOR_I2C_WRITE(0x337d,0x06);
	SENSOR_I2C_WRITE(0x33a0,0x05);
	SENSOR_I2C_WRITE(0x3301,0x04);
	SENSOR_I2C_WRITE(0x3633,0x4f);
	SENSOR_I2C_WRITE(0x3622,0x06);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3630,0x08);
	SENSOR_I2C_WRITE(0x3631,0x84);
	SENSOR_I2C_WRITE(0x3306,0x30);
	SENSOR_I2C_WRITE(0x366e,0x08);
	SENSOR_I2C_WRITE(0x366f,0x22);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3637,0x59);
	SENSOR_I2C_WRITE(0x3320,0x06);
	SENSOR_I2C_WRITE(0x3326,0x00);
	SENSOR_I2C_WRITE(0x331e,0x11);
	SENSOR_I2C_WRITE(0x331f,0xc1);
	SENSOR_I2C_WRITE(0x3303,0x20);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3309,0xd0);
	SENSOR_I2C_WRITE(0x330b,0xbe);
	SENSOR_I2C_WRITE(0x3306,0x36);
	SENSOR_I2C_WRITE(0x3635,0xc2);
	SENSOR_I2C_WRITE(0x363b,0x0a);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3038,0x88);
	SENSOR_I2C_WRITE(0x3638,0x1f);
	SENSOR_I2C_WRITE(0x3636,0x25);
	SENSOR_I2C_WRITE(0x3625,0x02);
	SENSOR_I2C_WRITE(0x331b,0x83);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3333,0x30);
	SENSOR_I2C_WRITE(0x3635,0xa0);
	SENSOR_I2C_WRITE(0x363b,0x0a);
	SENSOR_I2C_WRITE(0x363c,0x05);
	SENSOR_I2C_WRITE(0x3314,0x13);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3038,0xc8);
	SENSOR_I2C_WRITE(0x363b,0x0b);
	SENSOR_I2C_WRITE(0x3632,0x18);
	SENSOR_I2C_WRITE(0x3038,0xff);
	SENSOR_I2C_WRITE(0x3639,0x09);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3621,0x28);
	SENSOR_I2C_WRITE(0x3211,0x0c);
	SENSOR_I2C_WRITE(0x366f,0x26);
	SENSOR_I2C_WRITE(0x366f,0x2f);
	SENSOR_I2C_WRITE(0x3320,0x01);
	SENSOR_I2C_WRITE(0x3306,0x48);
	usleep(1000);
	SENSOR_I2C_WRITE(0x331e,0x19);
	SENSOR_I2C_WRITE(0x331f,0xc9);
	SENSOR_I2C_WRITE(0x330b,0xd3);
	SENSOR_I2C_WRITE(0x3620,0x28);
	SENSOR_I2C_WRITE(0x3309,0x60);
	usleep(1000);
	SENSOR_I2C_WRITE(0x331f,0x59);
	SENSOR_I2C_WRITE(0x3308,0x10);
	SENSOR_I2C_WRITE(0x3630,0x0c);
	SENSOR_I2C_WRITE(0x3f00,0x07);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3f04,0x05);
	SENSOR_I2C_WRITE(0x3f05,0x04);
	SENSOR_I2C_WRITE(0x3802,0x01);
	SENSOR_I2C_WRITE(0x3235,0x08);
	SENSOR_I2C_WRITE(0x3236,0xc8);
	SENSOR_I2C_WRITE(0x3630,0x1c);
	usleep(1000);
	SENSOR_I2C_WRITE(0x33aa,0x10);
	SENSOR_I2C_WRITE(0x3670,0x04);
	SENSOR_I2C_WRITE(0x3677,0x84);
	SENSOR_I2C_WRITE(0x3678,0x88);
	SENSOR_I2C_WRITE(0x3679,0x88);
	usleep(1000);
	SENSOR_I2C_WRITE(0x367e,0x08);
	SENSOR_I2C_WRITE(0x367f,0x28);
	SENSOR_I2C_WRITE(0x3670,0x0c);
	SENSOR_I2C_WRITE(0x3690,0x34);
	SENSOR_I2C_WRITE(0x3691,0x11);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3692,0x42);
	SENSOR_I2C_WRITE(0x369c,0x08);
	SENSOR_I2C_WRITE(0x369d,0x28);
	SENSOR_I2C_WRITE(0x360f,0x01);
	SENSOR_I2C_WRITE(0x3671,0xc6);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3672,0x06);
	SENSOR_I2C_WRITE(0x3673,0x16);
	SENSOR_I2C_WRITE(0x367a,0x28);
	SENSOR_I2C_WRITE(0x367b,0x3f);
	SENSOR_I2C_WRITE(0x3222,0x29);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3901,0x02);
	SENSOR_I2C_WRITE(0x3905,0x98);
	SENSOR_I2C_WRITE(0x3e1e,0x34);
	SENSOR_I2C_WRITE(0x3314,0x08);
	SENSOR_I2C_WRITE(0x3301,0x06);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3306,0x48);
	SENSOR_I2C_WRITE(0x3632,0x08);
	SENSOR_I2C_WRITE(0x3e00,0x00);
	SENSOR_I2C_WRITE(0x3e01,0x46);
	SENSOR_I2C_WRITE(0x3e02,0x10);
	SENSOR_I2C_WRITE(0x3e03,0x0b);
	usleep(1000);
	SENSOR_I2C_WRITE(0x3e06,0x00);
	SENSOR_I2C_WRITE(0x3e07,0x80);
	SENSOR_I2C_WRITE(0x3e08,0x03);
	SENSOR_I2C_WRITE(0x3e09,0x10);
	SENSOR_I2C_WRITE(0x3314,0x04);
	usleep(1000);
	SENSOR_I2C_WRITE(0x5780,0xff);
	SENSOR_I2C_WRITE(0x3802,0x00);
	SENSOR_I2C_WRITE(0x0100,0x01);

	bSensorInit = HI_TRUE;
	return;
}


HI_VOID sc2232_global_init()
{   
    gu8SensorImageMode = SENSOR_2M_1080p25_MODE;
    genSensorMode = WDR_MODE_NONE;       
    gu32FullLinesStd = FRAME_LINES_2M_1080p;
    gu32FullLines = FRAME_LINES_2M_1080p; 
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 
    
    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    
	g_stSnsRegsInfo.astI2cData[0].u32Data = 0x00;	//intt
	g_stSnsRegsInfo.astI2cData[1].u32Data = 0x00;
	g_stSnsRegsInfo.astI2cData[2].u32Data = 0x10;
	
	g_stSnsRegsInfo.astI2cData[3].u32Data = 0x00;	//gain
	g_stSnsRegsInfo.astI2cData[4].u32Data = 0x80;
	g_stSnsRegsInfo.astI2cData[5].u32Data = 0x03;
	g_stSnsRegsInfo.astI2cData[6].u32Data = 0x10;
	
	g_stSnsRegsInfo.astI2cData[7].u32Data = 0x04;   //VMax
	g_stSnsRegsInfo.astI2cData[8].u32Data = 0x65;
	
	g_stSnsRegsInfo.astI2cData[9].u32Data  = 0x00;  //Grouphold
	g_stSnsRegsInfo.astI2cData[10].u32Data = 0x06;
	g_stSnsRegsInfo.astI2cData[11].u32Data = 0x48;
	g_stSnsRegsInfo.astI2cData[12].u32Data = 0x08;
	g_stSnsRegsInfo.astI2cData[13].u32Data = 0x04;
	g_stSnsRegsInfo.astI2cData[14].u32Data = 0x30;
}


HI_S32 sc2232_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
     memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = sc2232_reg_init;
//    pstSensorExpFunc->pfn_cmos_sensor_exit = SC2232_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sc2232_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = sc2232_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = sc2232_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = sc2232_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = sc2232_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = sc2232_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = sc2232_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int sc2232_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    sc2232_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, SC2232_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    sc2232_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, SC2232_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    sc2232_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, SC2232_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int sc2232_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, SC2232_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, SC2232_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, SC2232_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}



int SmartSens_SC2232_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister

{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;
//	sc2232_reg_init();

	sc2232_register_callback();
//	af_register_callback(pAfRegister);

	ALG_LIB_S stLib;
	HI_S32 s32Ret = 0;
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
	printf("SC2232 sensor 1080P30fps init success!\n");

	return s32Ret;
}



bool SENSOR_SC2232_probe()
{
	uint16_t ret_data1 = 0;
	uint16_t ret_data2 = 0;

	SENSOR_I2C_READ(0x3107, &ret_data1);
	SENSOR_I2C_READ(0x3108, &ret_data2);
	
	if(ret_data1 == SC2232_CHECK_DATA_LSB && ret_data2 == SC2232_CHECK_DATA_MSB){

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



int SC2232_get_sensor_name(char *sensor_name)
{
	if(sensor_name != NULL) {
		memcpy(sensor_name,SENSOR_NAME,strlen(SENSOR_NAME));
		return 0;
	}
	return -1;
}

int SC2232_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
	if(ret_width && ret_height){
		*ret_width = SENSOR_SC2232_WIDTH;
		*ret_height = SENSOR_SC2232_HEIGHT;
		return 0;
	}
	return -1;
}
static bool sensor_mirror = false;
static bool sensor_flip = false;

int SC2232_set_mirror(bool mirror)
{
	return 0;
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

int SC2232_set_flip(bool flip)
{
	return 0;
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

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */




