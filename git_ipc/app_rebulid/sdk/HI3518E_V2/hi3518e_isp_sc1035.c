#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "sc1035"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x60, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x60, 2, 1))
	
#define SENSOR_SC1035_WIDTH 1280
#define SENSOR_SC1035_HEIGHT 960
#define SC1035_CHECK_DATA	(0xF0)//CHIP_ID address is 0x580B
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__SC1035_CMOS_H_)
#define __SC1035_CMOS_H_

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


#define SC1035_ID 1035
#define CMOS_SC1035_ISP_WRITE_SENSOR_ENABLE (1)


/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

#define FULL_LINES_MAX  (0xFFFF)

static const unsigned int sensor_i2c_addr=0x60;
static unsigned int sensor_addr_byte=2;
static unsigned int sensor_data_byte=1;

#define SENSOR_960P_30FPS_MODE (1)

#define VMAX_SC1035_960P30_LINEAR     (1000)


static HI_U8 gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = VMAX_SC1035_960P30_LINEAR;
static HI_U32 gu32FullLines = VMAX_SC1035_960P30_LINEAR;


static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};

/* Piris attr */
static ISP_PIRIS_ATTR_S gstPirisAttr=
{
    0,      // bStepFNOTableChange
    1,      // bZeroIsMax
    93,     // u16TotalStep
    62,     // u16StepCount
    /* Step-F number mapping table. Must be from small to large. F1.0 is 1024 and F32.0 is 1 */
    {30,35,40,45,50,56,61,67,73,79,85,92,98,105,112,120,127,135,143,150,158,166,174,183,191,200,208,217,225,234,243,252,261,270,279,289,298,307,316,325,335,344,353,362,372,381,390,399,408,417,426,435,444,453,462,470,478,486,493,500,506,512},
    ISP_IRIS_F_NO_1_4, // enMaxIrisFNOTarget
    ISP_IRIS_F_NO_5_6  // enMinIrisFNOTarget
};



/* AE default parameter and function */
static HI_S32 sc1035_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    pstAeSnsDft->u32LinesPer500ms = VMAX_SC1035_960P30_LINEAR * 30 / 2;
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
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
    pstAeSnsDft->u32MinIntTime = 2;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;


    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxAgain = 26630;
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxDgain = 1928;
    pstAeSnsDft->u32MinDgain = 1024;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain; 
    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift; 

    pstAeSnsDft->enIrisType = ISP_IRIS_DC_TYPE;
    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    pstAeSnsDft->u8AERunInterval = 2;

    return 0;
}

/* the function of sensor set fps */
static HI_VOID sc1035_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if ((f32Fps <= 30) && (f32Fps >= 0.5))
    {
        if(SENSOR_960P_30FPS_MODE == gu8SensorImageMode)
        {
            gu32FullLinesStd = VMAX_SC1035_960P30_LINEAR * 30 / f32Fps;           
            gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;

            g_stSnsRegsInfo.astI2cData[3].u32Data = (gu32FullLinesStd >> 8) & 0xff;
            g_stSnsRegsInfo.astI2cData[4].u32Data = gu32FullLinesStd & 0xff;

            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
            
            pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

        }
    }
    else
    {
        printf("Not support Fps: %f\n", f32Fps);
        return;
    }
  
    return;
}

static HI_VOID sc1035_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{    
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;
    
    g_stSnsRegsInfo.astI2cData[3].u32Data = (gu32FullLines >> 8) & 0xff;
    g_stSnsRegsInfo.astI2cData[4].u32Data = gu32FullLines & 0xff;

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;
    
    return;
}

#define ANTIDENOISPOINT 680
/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID sc1035_inttime_update(HI_U32 u32IntTime)
{
    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 4) & 0xFF;
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime << 4) & 0xF0;

#if 1 //2016.05.25  //reduce noise
		HI_U16 _time = u32IntTime;
		int  _fulllines= gu32FullLinesStd; //vts
		HI_U16 u16DenoisValue = 0;
		static HI_U16 u16DenoisValue_backup = 0;

		int  nStartLines= gu32FullLinesStd - ANTIDENOISPOINT - 4;
	 
		if (_time <= nStartLines)
		{
			if(0xe0 != u16DenoisValue_backup)
			{
				u16DenoisValue_backup = 0xe0;
				SENSOR_I2C_WRITE(0x331e, 0xe0);
//				printf("nStartLines=0x%x,_time=0x%x,_fulllines=0x%x,u16DenoisValue=0x%x\r\n",nStartLines,_time,_fulllines,u16DenoisValue);
			}
		}
		else if ((_time > nStartLines) && (_time <= (_fulllines - 4)))
		{
			u16DenoisValue = 0xe0 - ((_time - nStartLines) * (0xe0 - 0x1a))/ANTIDENOISPOINT;
			if(u16DenoisValue != u16DenoisValue_backup)
			{
				u16DenoisValue_backup = u16DenoisValue;
				SENSOR_I2C_WRITE(0x331e, u16DenoisValue);
//				printf("nStartLines=0x%x,_time=0x%x,_fulllines=0x%x,u16DenoisValue=0x%x\r\n",nStartLines,_time,_fulllines,u16DenoisValue);
			}
		}
		else
		{
		}
		
#endif		
		
    return;
}
static  HI_U32   analog_gain_table[80] = 
{ 
	1024,	1084,	1144,	1205,	 1265,	1325,	1385,	1445,	1506,	1566,	1626,	1686,	1747,	1807,	1867,	1927, 
	1927,	1927,	2026,	2132,	 2239,	2345,	2452,	2558,	2665,	2772,	2878,	2985,	3091,	3198,	3305,	3411, 
	3625,	3838,	4051,	4264,	 4478,	4691,	4904,	5117,	5330,	5543,	5756,	5970,	6183,	6396,	6609,	6822, 
	7250,	7676,	8103,	8529,	 8955,	9381,	9808,	10234,	10660,	11087,	11513,	11939,	12365,	12792,	13218,	13644,
	14500,	15352,	16205,	17058,	17910,	18763,	19615,	20468,	21321,	22173,	23026,	23878,	24731,	25584,	26436,	27289
};


static  HI_U32   digital_gain_table[3] = 
{ 
    1024,2048,4096
};


static HI_VOID sc1035_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if (*pu32AgainLin >= analog_gain_table[79])
    {
         *pu32AgainLin = analog_gain_table[79];
         *pu32AgainDb = 79;
         return ;
    }
    
    for (i = 1; i < 80; i++)
    {
        if (*pu32AgainLin < analog_gain_table[i])
        {
            *pu32AgainLin = analog_gain_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }

    return;

}

static HI_VOID sc1035_dgain_calc_table(HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
    int i;

    if (*pu32DgainLin >= digital_gain_table[2])
    {
         *pu32DgainLin = digital_gain_table[2];
         *pu32DgainDb = 2;
         return ;
    }
    
    for (i = 1; i < 3; i++)
    {
        if (*pu32DgainLin < digital_gain_table[i])
        {
            *pu32DgainLin = digital_gain_table[i - 1];
            *pu32DgainDb = i - 1;
            break;
        }
    }

    return;
}

static const HI_U16 sensor_gain_map[62] = {
	0x82, 0x83, 0x84,0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x8b, 0x8c,0x8d, 0x8e, 0x8f, 0x90, 0x91,
	0x92, 0x93, 0x94, 0x95,0x96, 0x97, 0x98, 0x99,
	0x9a, 0x9b, 0x9c, 0x9d,0x9e, 0x9f, 0xb0, 0xb1,
	0xb2, 0xb3, 0xb4, 0xb5,0xb6, 0xb7, 0xb8, 0xb9,
	0xba, 0xbb, 0xbc, 0xbd,0xbe, 0xbf, 0xf0, 0xf1,
	0xf2, 0xf3, 0xf4, 0xf5,0xf6, 0xf7, 0xf8, 0xf9,
	0xfa, 0xfb, 0xfc, 0xfd,0xfe, 0xff
};

static HI_U16 gainmap_gain2index(HI_U16 Gain)
{
	HI_U16 i = 0;
	for(i = 0; i < sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0]); i++){
		if(sensor_gain_map[i] == Gain){
			break;
		}
	}
	return i;
}

//return limited Again
static HI_U32 sc1035_Again_limit(HI_U32 Again)
{
#define SENSOR_BLC_TOP_VALUE (0x58)
#define SENSOR_BLC_BOT_VALUE (0x45)
#define SENSOR_AGAIN_ADAPT_STEP (1)
#define SENSOR_MAX_AGAIN (0xff)

	HI_U32 ret_Again;
	HI_U16 BLC_top = SENSOR_BLC_TOP_VALUE, BLC_bot = SENSOR_BLC_BOT_VALUE, BLC_reg, gain_index;
	static HI_U16 MaxAgain = SENSOR_MAX_AGAIN;
	SENSOR_I2C_READ(0X3911,&BLC_reg);

	gain_index = gainmap_gain2index(MaxAgain);

	if(BLC_reg > BLC_top){//>0x58
		if(gain_index>0){
			//limit max Again by step
			gain_index -= SENSOR_AGAIN_ADAPT_STEP;
		}
		MaxAgain = sensor_gain_map[gain_index];
	}else if(BLC_reg < BLC_bot){//<0x45
		//release Again limit by step
		gain_index += SENSOR_AGAIN_ADAPT_STEP;
		if(gain_index > sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0])-1){
			gain_index = sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0]) - 1;
		}
		MaxAgain = sensor_gain_map[gain_index];
	}else{//0x45 < BLC_reg < 0x58
		//do nothing
	}
	ret_Again = Again > MaxAgain ? MaxAgain : Again;
	//printf("limit gain:ret_Again=%d;Again=%d;MaxAgain=%d\n", ret_Again, Again, MaxAgain);
	return ret_Again;
}

static HI_VOID sc1035_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{  
    HI_U32 u32DGReg = 0;
	HI_U8 u8AgainHigh, u8AgainLow;

	if (u32Again >= 1 && u32Again < 0x10)
		u8AgainHigh = 0x0;
	else if (u32Again >= 0x10 && u32Again < 0x20)
		u8AgainHigh = 0x8;
	else if (u32Again >= 0x20 && u32Again < 0x30)
		u8AgainHigh = 0x9;
	else if (u32Again >= 0x30 && u32Again < 0x40)
		u8AgainHigh = 0xb;
	else if (u32Again >= 0x40 && u32Again < 0x50)
		u8AgainHigh = 0xf;
	else 
		u8AgainHigh = 0x0;

	u8AgainLow = u32Again & 0xf;

	if (u32Dgain == 0)
	{
		u32DGReg = 0x14;
	}
	else if (u32Dgain == 1) 
	{
		u32DGReg = 0x15;
	}
	else if (u32Dgain == 2)
	{
		u32DGReg = 0x17;
	}
	else
	{
		u32DGReg = 0x14;
	}
	
#if CMOS_SC1035_ISP_WRITE_SENSOR_ENABLE
	g_stSnsRegsInfo.astI2cData[2].u32Data =  sc1035_Again_limit((u8AgainHigh<<4) | u8AgainLow);
	g_stSnsRegsInfo.astI2cData[5].u32Data = u32DGReg;
#else
    SENSOR_I2C_WRITE(0x3e09, sc1035_Again_limit((u8AgainHigh<<4) | u8AgainLow));
    SENSOR_I2C_WRITE(0x3e0f, u32DGReg);
#endif 
	
    return;
}

HI_S32 sc1035_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = sc1035_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = sc1035_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= sc1035_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = sc1035_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = sc1035_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = sc1035_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = sc1035_dgain_calc_table;

    return 0;
}


/* AWB default parameter and function */
static AWB_CCM_S g_stAwbCcm =
{  
    5000,
    {
        0x01a2, 0x8080, 0x8022,
        0x8068, 0x0178, 0x8010,
        0x8008, 0x811A, 0x0222
    },
    3200,
    {
        0x01e0,0x807b,0x8065,
        0x804f,0x0165,0x8016,
        0x001f,0x8126,0x0206
    },
    2600,
    {
        0x0200, 0x80c3, 0x803D,
        0x8057, 0x015C, 0x8005,
        0x8020, 0x8290, 0x03b0
    }

};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,
	
    /*1,  2,  4,  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768*/
    /* saturation */   
    {0x80,0x80,0x7a,0x78,0x70,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38}

};

static HI_S32 sc1035_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;
    pstAwbSnsDft->au16GainOffset[0] = 0x18b;    
    pstAwbSnsDft->au16GainOffset[1] = 0x100;    
    pstAwbSnsDft->au16GainOffset[2] = 0x100;    
    pstAwbSnsDft->au16GainOffset[3] = 0x1a7;    
    pstAwbSnsDft->as32WbPara[0] = 86;
    pstAwbSnsDft->as32WbPara[1] = -16;
    pstAwbSnsDft->as32WbPara[2] = -182;
    pstAwbSnsDft->as32WbPara[3] = 243836;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -195566;
    
    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_S32 sc1035_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = sc1035_get_awb_default;

    return 0;
}

#define DMNR_CALIB_CARVE_NUM_SC1035 (6)

float g_coef_calib_sc1035[DMNR_CALIB_CARVE_NUM_SC1035][4] = 
{  
    {103.000000f, 2.012837f, 0.031290f, 10.517865f, }, 

    {218.000000f, 2.338456f, 0.033560f, 10.211367f, }, 

    {437.000000f, 2.640481f, 0.035517f, 10.179346f, }, 

    {916.000000f, 2.961895f, 0.037293f, 10.693511f, }, 

    {1207.000000f, 3.081707f, 0.037759f, 11.220128f, }, 

    {2581.000000f, 3.411788f, 0.072266f, 4.389672f, }, 

        
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
	1,    /*bEnable*/			
	7,    /*u8Slope*/	
	7,    /*u8Sensitivity*/
	8192, /*u16Threshold*/
	8192, /*u16SensiThreshold*/	
	{1024,1024,1024,2048,2048,2048,2048,  2048,  2048,2048,2048,2048,2048,2048,2048,2048}    /*au16Strength[ISP_AUTO_ISO_STENGTH_NUM]*/	
};
#if 0
static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
  //{100,200,400,800,1600,3200,6400,12800,25600,51200,102400,204800,409600,819200,1638400,3276800};
    {0,	  0,   0,  0,  0,   1,   1,    1,    1,    1,    1,     1,     1,     1,     1,       1},/* enPixSel = ~bEnLowLumaShoot */
    {30, 30, 32, 35,  35,  60,  100,  120,    160,  200,  250,   250,   250,   250,    250,    250},/*maxSharpAmt1 = SharpenUD*16 */
    {80, 80, 100, 140,  180,  200,  250,  250,    250,  250,  250,   250,   250,   250,    250,    250},/*maxEdgeAmt = SharpenD*16 */
    {0,  0,   0,  0,   0,  0,  20,   40,    90,    120,    180,     250,    250,     250,     250,       250},/*sharpThd2 = TextureNoiseThd*4 */
    {0,  0,   0,  0,   0,  0,  0,   0,    0,    0,    0,     0,    0,     0,     0,       0},/*edgeThd2 = EdgeNoiseThd*4 */
    {255, 220, 200, 160, 120,  80, 30,   20,  0,  0,   0,    0,   0,    0,    0,      0},/*overshootAmt*/
    {255, 255, 220, 200, 160,  130,100,  70,  30,  0,   0,    0,   0,    0,    0,     0},/*undershootAmt*/
};
#else
static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
  //{100,200,400,800,1600,3200,6400,12800,25600,51200,102400,204800,409600,819200,1638400,3276800};
    {0,	  0,   0,   0,    0,   1,   1,   1,    1,    1,    1,     1,     1,     1,     1,       1},/* enPixSel = ~bEnLowLumaShoot */
        
    {120,  64,  64,  43,  43,  43,  18,  18,    18,  200,  250,   250,   250,   250,    250,    250},/*maxSharpAmt1 = SharpenUD*16 */
    {128, 200, 103, 86,  86,  86,  80,  80,    80,  250,  250,   250,   250,   250,    250,    250},/*maxEdgeAmt = SharpenD*16 */
        
    {0,   0,   0,    0,   0,   0,   0,   40,  190,  200,  220,   250,   250,   250,     250,       250},/*sharpThd2 = TextureNoiseThd*4 */
    {0,   0,   0,    0,   0,   0,   0,   60,  140,    0,    0,     0,    0,     0,     0,       0},/*edgeThd2 = EdgeNoiseThd*4 */
    {59,  59,  59,  59,  59,  59,  59,   59,  101,  0,   0,    0,   0,    0,    0,      0},/*overshootAmt*/
    {117, 117, 117, 108, 108, 108, 122,  122, 139,  0,   0,    0,   0,    0,    0,     0},/*undershootAmt*/
};

#endif

static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
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

HI_U32 sc1035_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
/*
    pstDef->stDrc.bEnable               = HI_FALSE;
    pstDef->stDrc.u8Asymmetry           = 0x02;
    pstDef->stDrc.u8SecondPole          = 0xC0;
    pstDef->stDrc.u8Stretch             = 0x3C;
    pstDef->stDrc.u8LocalMixingBrigtht  = 0x2D;
    pstDef->stDrc.u8LocalMixingDark     = 0x2D;
    pstDef->stDrc.u8LocalMixingThres    = 0x02;
    pstDef->stDrc.u16BrightGainLmt      = 0x7F;
    pstDef->stDrc.u16DarkGainLmtC       = 0x7F;
    pstDef->stDrc.u16DarkGainLmtY       = 0x7F;
    pstDef->stDrc.u8RangeVar            = 0x00;
    pstDef->stDrc.u8SpatialVar          = 0x0A;
  */ 
 	memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
    memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));			
    pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_SC1035;
    pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_sc1035;

    memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);

    memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
    memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_SC1035_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_SC1035_HEIGHT;

    return 0;
}

HI_U32 sc1035_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
          
    pstBlackLevel->au16BlackLevel[0] = 0xc0;
    pstBlackLevel->au16BlackLevel[1] = 0xc0;
    pstBlackLevel->au16BlackLevel[2] = 0xc0;
    pstBlackLevel->au16BlackLevel[3] = 0xc0;
    

    return 0;  
    
}

HI_VOID sc1035_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(0x320e, 0x17);
        SENSOR_I2C_WRITE(0x320f, 0x70);
        SENSOR_I2C_WRITE(0x3e01, 0x76);
        SENSOR_I2C_WRITE(0x3e02, 0xc0);
        SENSOR_I2C_WRITE(0x3e09, 0x00);
        
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(0x320e, (gu32FullLinesStd >> 8) & 0xff);
        SENSOR_I2C_WRITE(0x320f, gu32FullLinesStd & 0xff);
        SENSOR_I2C_WRITE(0x3e01, 0x3E);
        SENSOR_I2C_WRITE(0x3e02, 0x40);
        bInit = HI_FALSE;
    }

    return;
}

HI_VOID sc1035_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            if (SENSOR_960P_30FPS_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = VMAX_SC1035_960P30_LINEAR;
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

HI_U32 sc1035_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;		
        g_stSnsRegsInfo.u32RegNum = 6;
	
        for (i = 0; i < g_stSnsRegsInfo.u32RegNum; i++)
        {	
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }

        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;         //exposure time
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3e01;
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3e02;
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3e09;        //gain
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x3e0f;        //gain

        
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 1;       
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x320e;
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x320f;

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

static HI_S32 sc1035_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;

    bInit = HI_FALSE;
    
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if ((pstSensorImageMode->u16Width <= 1280) && (pstSensorImageMode->u16Height <= 960))
    {
        if (WDR_MODE_NONE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = SENSOR_960P_30FPS_MODE;
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

    return 0;
}

HI_VOID sc1035_global_init()
{   
    gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_SC1035_960P30_LINEAR; 
    gu32FullLines = VMAX_SC1035_960P30_LINEAR;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
}


static void sc1035_reg_init()
{
    SENSOR_I2C_WRITE(0x3000,0x0001);//manualstreamenbale
    SENSOR_I2C_WRITE(0x3003,0x0001);//softreset
    SENSOR_I2C_WRITE(0x3400,0x0053);
    SENSOR_I2C_WRITE(0x3416,0x00C0);
    SENSOR_I2C_WRITE(0x3d08,0x0000);
    SENSOR_I2C_WRITE(0x5000,0x0009);
    SENSOR_I2C_WRITE(0x3e03,0x0003);
    SENSOR_I2C_WRITE(0x3928,0x0000);
    SENSOR_I2C_WRITE(0x3630,0x0058);
    SENSOR_I2C_WRITE(0x3612,0x0000);
    SENSOR_I2C_WRITE(0x3632,0x0041);
    SENSOR_I2C_WRITE(0x3635,0x0004);
    SENSOR_I2C_WRITE(0x3500,0x0010);
    SENSOR_I2C_WRITE(0x3631,0x0080);
    SENSOR_I2C_WRITE(0x3620,0x0044);
    SENSOR_I2C_WRITE(0x3633,0x007c);
    SENSOR_I2C_WRITE(0x3780,0x000b);
    SENSOR_I2C_WRITE(0x3300,0x0033);
    SENSOR_I2C_WRITE(0x3301,0x0038);
    SENSOR_I2C_WRITE(0x3302,0x0030);
    SENSOR_I2C_WRITE(0x3303,0x0066);
    SENSOR_I2C_WRITE(0x3304,0x00a0);
    SENSOR_I2C_WRITE(0x3305,0x0072);
    SENSOR_I2C_WRITE(0x331e,0x0056);
    SENSOR_I2C_WRITE(0x321e,0x0000);
    SENSOR_I2C_WRITE(0x321f,0x000a);
    SENSOR_I2C_WRITE(0x3216,0x000a);
    SENSOR_I2C_WRITE(0x3115,0x000a);
    SENSOR_I2C_WRITE(0x3332,0x0038);
    SENSOR_I2C_WRITE(0x5054,0x0082);
    SENSOR_I2C_WRITE(0x3622,0x0026);
    SENSOR_I2C_WRITE(0x3907,0x0002);
    SENSOR_I2C_WRITE(0x3908,0x0000);
    SENSOR_I2C_WRITE(0x3601,0x0018);
    SENSOR_I2C_WRITE(0x3315,0x0044);
    SENSOR_I2C_WRITE(0x3308,0x0040);
    SENSOR_I2C_WRITE(0x3223,0x0022);//vysncmode[5]
    SENSOR_I2C_WRITE(0x3e0e,0x0050);
    /*DPC*/
    SENSOR_I2C_WRITE(0x3101,0x009b);
    SENSOR_I2C_WRITE(0x3114,0x0003);
    SENSOR_I2C_WRITE(0x3115,0x00d1);
    SENSOR_I2C_WRITE(0x3211,0x0064);
    SENSOR_I2C_WRITE(0x5780,0x00ff);
    SENSOR_I2C_WRITE(0x5781,0x007f);
    SENSOR_I2C_WRITE(0x5785,0x00ff);
    SENSOR_I2C_WRITE(0x5000,0x0000);
    
    //SENSOR_I2C_WRITE(0x3e0f,0x0010);
    SENSOR_I2C_WRITE(0x3631,0x0082);       //0x3e0a dgain N+2
    
    /*27Minput54Moutputpixelclockfrequency*/
    SENSOR_I2C_WRITE(0x3010,0x0007);
    SENSOR_I2C_WRITE(0x3011,0x0046);
    SENSOR_I2C_WRITE(0x3004,0x0004);
    SENSOR_I2C_WRITE(0x3610,0x002b);

    /*configFramelengthandwidth*/
    SENSOR_I2C_WRITE(0x320c,0x0007); //1800
    SENSOR_I2C_WRITE(0x320d,0x0008); //1000
    SENSOR_I2C_WRITE(0x320e,0x0003);
    SENSOR_I2C_WRITE(0x320f,0x00e8);
    
    /*configOutputwindowposition*/
    SENSOR_I2C_WRITE(0x3210,0x0000);
    SENSOR_I2C_WRITE(0x3211,0x0064);
    SENSOR_I2C_WRITE(0x3212,0x0000);
    SENSOR_I2C_WRITE(0x3213,0x0008);//if 0x3213=0x09,the picture is red 
    
    /*configOutputimagesize*/
    SENSOR_I2C_WRITE(0x3208,0x0005);
    SENSOR_I2C_WRITE(0x3209,0x0000);
    SENSOR_I2C_WRITE(0x320a,0x0003);
    SENSOR_I2C_WRITE(0x320b,0x00c0);
    
    /*configFramestartphysicalposition*/
    SENSOR_I2C_WRITE(0x3202,0x0000);
    SENSOR_I2C_WRITE(0x3203,0x0008);
    SENSOR_I2C_WRITE(0x3206,0x0003);
    SENSOR_I2C_WRITE(0x3207,0x00cf);
    
    /*powerconsumptionreduction*/
    SENSOR_I2C_WRITE(0x3330,0x000d);
    SENSOR_I2C_WRITE(0x3320,0x0006);
    SENSOR_I2C_WRITE(0x3321,0x00e8);
    SENSOR_I2C_WRITE(0x3322,0x0001);
    SENSOR_I2C_WRITE(0x3323,0x00c0);
    SENSOR_I2C_WRITE(0x3600,0x0054);

    bSensorInit = HI_TRUE;
    printf("======================================================================\n");
    printf("===SmartSens sc1035 sensor 960P30fps(Parallel port) init success!=====\n");
    printf("======================================================================\n");

    return;
}


HI_S32 sc1035_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = sc1035_reg_init;
   // pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sc1035_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = sc1035_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = sc1035_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = sc1035_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = sc1035_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = sc1035_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = sc1035_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int sc1035_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    sc1035_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, SC1035_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    sc1035_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, SC1035_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    sc1035_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, SC1035_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int sc1035_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, SC1035_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, SC1035_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, SC1035_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int SmartSens_SC1035_init(SENSOR_DO_I2CRD do_i2c_read, 
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	sc1035_register_callback();
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
	printf("SmartSens SC1035 sensor 1080P30fps init success!\n");
	return s32Ret;
}

int SC1035_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_SC1035_WIDTH;
        *ret_height = SENSOR_SC1035_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_SC1035_probe()
{
    uint16_t ret_data1 = 0;
	
    SENSOR_I2C_READ(0x580B, &ret_data1);
	
    if(ret_data1 == SC1035_CHECK_DATA ){
		//set i2c pinmux
		sdk_sys->write_reg(0x200f0040, 0x2);
	    sdk_sys->write_reg(0x200f0044, 0x2);
		
        sdk_sys->write_reg(0x2003002c, 0xb4001);	 // sensor unreset, clk 27MHz, VI 99MHz
        return true;
    }
    return false;
}
int SC1035_get_sensor_name(char *sensor_name)
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

#endif /* __SOISC1035_CMOS_H_ */



