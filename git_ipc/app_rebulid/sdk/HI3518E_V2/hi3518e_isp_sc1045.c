#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "sc1045"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x60, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x60, 2, 1))

#define SENSOR_SC1045_WIDTH 1280
#define SENSOR_SC1045_HEIGHT 720
#define SC1045_CHECK_DATA_LSB	(0x10)//0x3107
#define SC1045_CHECK_DATA_MSB	(0x45)//0x3108
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__SC1045_CMOS_H_)
#define __SC1045_CMOS_H_

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


#define SC1045_ID 1045

#define CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE (1)


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

static const unsigned int sensor_i2c_addr=0x60;
static unsigned int sensor_addr_byte=2;
static unsigned int sensor_data_byte=1;

#define VMAX_ADDR_H              (0x320e)
#define VMAX_ADDR_L              (0x320f)

#define SENSOR_720P_30FPS_MODE  (1)

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define VMAX_720P30_LINEAR     (750+INCREASE_LINES)
#define CMOS_OV1045_SLOW_FRAMERATE_MODE (0)



static HI_U8 gu8SensorImageMode = SENSOR_720P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = VMAX_720P30_LINEAR;
static HI_U32 gu32FullLines = VMAX_720P30_LINEAR;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};
static HI_U8 gu8Fps = 30;


/* AE default parameter and function */
static HI_S32 sc1045_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;

    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
            
    pstAeSnsDft->u8AeCompensation = 0x38;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;//748
    pstAeSnsDft->u32MinIntTime = 1;
    pstAeSnsDft->u32MaxIntTimeTarget = 3*gu32FullLinesStd - 6;
    pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;


    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0078125;
    pstAeSnsDft->u32MaxAgain = 16384; 
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = 16384;
    pstAeSnsDft->u32MinAgainTarget = 1024;
        


    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 6;
    pstAeSnsDft->u32MaxDgain = 1024;//4096;  
    pstAeSnsDft->u32MinDgain = 1024;
    pstAeSnsDft->u32MaxDgainTarget = 4096;
    pstAeSnsDft->u32MinDgainTarget = 1024; 
    

    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift; 
	  
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;

    return 0;
}

/* the function of sensor set fps */
static HI_VOID sc1045_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

    HI_U32 u32VblankingLines = 0xFFFF;
    if ((f32Fps <= 30) && (f32Fps >= 0.5))
    {
        if(SENSOR_720P_30FPS_MODE == gu8SensorImageMode)
        {
            u32VblankingLines = VMAX_720P30_LINEAR * 30 / f32Fps;
        }
    }
    else
    {
        printf("Not support Fps: %f\n", f32Fps);
        return;
    }


#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
    g_stSnsRegsInfo.astI2cData[4].u32Data = (u32VblankingLines >> 8)&0xff;
	g_stSnsRegsInfo.astI2cData[5].u32Data = u32VblankingLines & 0xFF;
#else
    sensor_write_register(0x320e, (u32VblankingLines >> 8)&0xff);
    sensor_write_register(0x320f, u32VblankingLines & 0xff);
#endif

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32MaxIntTime = u32VblankingLines - 4;
    gu32FullLinesStd = u32VblankingLines;
    gu8Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = u32VblankingLines * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}


static HI_VOID sc1045_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

    u32FullLines = (u32FullLines > 0xFFFF) ? 0xFFFF : u32FullLines;
    gu32FullLines = u32FullLines;
    
    
#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
	g_stSnsRegsInfo.astI2cData[4].u32Data = ((u32FullLines >> 8) & 0xFF);
	g_stSnsRegsInfo.astI2cData[5].u32Data = u32FullLines & 0xFF;
#else
	sensor_write_register(0x320e, ((u32FullLines >> 8) & 0xFF));
	sensor_write_register(0x320f, u32FullLines & 0xFF);
#endif
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;
    pstAeSnsDft->u32MaxIntTimeTarget= gu32FullLines - 4;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID sc1045_inttime_update(HI_U32 u32IntTime)
{
    
#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 4) & 0xFF; 
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime<<4) & 0xF0;
#else
	sensor_write_register(0x3e01, ((u32IntTime >> 4) & 0xFF));
	sensor_write_register(0x3e02, (u32IntTime<<4) & 0xF0);
#endif

    return;
}

	
static  HI_U32   anolog_gain_table[64] = 
{ 
    1024, 1084, 1145, 1205, 1265,  1325,  1385,   1446,  1506,  1566,  1626,  1687,  1747,  1807,  1868,  1927,
    2048, 2169, 2290, 2410, 2592,  2650,  2771,   2891,  3013,  3131,  3252,  3373,  3494,  3615,  3736,  3854,
    4096, 4338, 4579, 4821, 5059,  5300,  5542,   5784,  6025,  6263,  6504,  6746,  6988,  7229,  7471,  7709,
    8192, 8675, 9159, 9642, 10117, 10600, 11083,  11567, 12050, 12526, 13009, 13492, 13976, 14456, 14942, 15417
};

static  HI_U32   digital_gain_table[5] = 
{ 
    1024,2048,4096,8192,16384
};

static HI_VOID sc1045_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if((HI_NULL == pu32AgainLin) ||(HI_NULL == pu32AgainDb))
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

   
    if (*pu32AgainLin >= anolog_gain_table[63])
    {
         *pu32AgainLin = anolog_gain_table[63];
         *pu32AgainDb = 63;
         return ;
    }
    
    for(i = 1; i < 64; i++)
    {
        if(*pu32AgainLin < anolog_gain_table[i])
        {
            *pu32AgainLin = anolog_gain_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }

    }
    return;

}

static HI_VOID sc1045_dgain_calc_table(HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
    int i;

    if((HI_NULL == pu32DgainLin) ||(HI_NULL == pu32DgainDb))
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

   
    if (*pu32DgainLin >= digital_gain_table[4])
    {
         *pu32DgainLin = digital_gain_table[4];
         *pu32DgainDb = 4;
         return ;
    }
    
    for(i = 1; i < 5; i++)
    {
        if(*pu32DgainLin < digital_gain_table[i])
        {
            *pu32DgainLin = digital_gain_table[i - 1];
            *pu32DgainDb = i - 1;
            return;
        }

    }
    return;

}


static const HI_U16 sensor_gain_map[48] = {
	0x10, 0x11, 0x12, 0x13, 0x14,0x15, 0x16, 0x17, 0x18, 0x19,
	0x1a, 0x1b, 0x1c,0x1d, 0x1e, 0x1f, 0x30, 0x31,
	0x32, 0x33, 0x34, 0x35,0x36, 0x37, 0x38, 0x39,
	0x3a, 0x3b, 0x3c, 0x3d,0x3e, 0x3f, 0x70, 0x71,
	0x72, 0x73, 0x74, 0x75,0x76, 0x77, 0x78, 0x79,
	0x7a, 0x7b, 0x7c, 0x7d,0x7e, 0x7f
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
static HI_U32 sc1045_Again_limit(HI_U32 Again)
{
#define SENSOR_BLC_TOP_VALUE (0x1c)
#define SENSOR_BLC_BOT_VALUE (0x19)
#define SENSOR_AGAIN_ADAPT_STEP (1)
#define SENSOR_MAX_AGAIN (0x7f)

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


static HI_VOID sc1045_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    HI_U8 u8AgainHigh, u8AgainLow;
	HI_U8 u8DgainReg;

    if (u32Again >= 1 && u32Again < 0x10)
		u8AgainHigh = 0x0;
	else if (u32Again >= 0x10 && u32Again < 0x20)
		u8AgainHigh = 0x1;
	else if (u32Again >= 0x20 && u32Again < 0x30)
		u8AgainHigh = 0x3;
	else if (u32Again >= 0x30 && u32Again < 0x40)
		u8AgainHigh = 0x7;
	else 
		u8AgainHigh = 0x0;

    u8AgainLow = u32Again & 0xf;

    switch(u32Dgain)               
	{
        case 0:
			u8DgainReg = 0x84;
			break;
			
		case 1:
			u8DgainReg = 0xa4;
			break;
			
	    case 2:
			u8DgainReg = 0xe4;
			break;
			
		case 3:
			u8DgainReg = 0xe5;
			break;
			
		case 4:
			u8DgainReg = 0xe7;
			break;

		default:
			u8DgainReg = 0x84;
            break;
	}
	
	
#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
    g_stSnsRegsInfo.astI2cData[2].u32Data = sc1045_Again_limit((u8AgainHigh << 4 | u8AgainLow));
	g_stSnsRegsInfo.astI2cData[3].u32Data = u8DgainReg;
#else
    sensor_write_register(0x3e09, sc1045_Again_limit((u8AgainHigh << 4 | u8AgainLow)));
    sensor_write_register(0x3e0f, u8DgainReg);
#endif

    return;
}


HI_S32 sc1045_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = sc1045_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = sc1045_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= sc1045_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = sc1045_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = sc1045_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = sc1045_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = sc1045_dgain_calc_table;

    return 0;
}


/* AWB default parameter and function */
static AWB_CCM_S g_stAwbCcm =
{  
#if 0
    5120,    
    {       
        0x0217,  0x80df,  0x8038,
        0x8042,  0x01a6,  0x8064,       
        0x0005,  0x811e,  0x0219    
    },  
    
    3633,    
    {       
        0x01f2,  0x80a1,  0x8051,       
        0x805a,  0x01a3,  0x8049,       
        0x8011,  0x8141,  0x0252    
    },
    
    2465,    
    {            
        0x01e6,  0x80c6,  0x8020,        
        0x806c,  0x014f,  0x001d,       
        0x805a,  0x82a1,  0x03fb    
    } 
#else
	4850,	 
	{		
		0x0223,  0x80f7,  0x802c,
		0x804a,  0x01d2,  0x8088,		
		0x0017,  0x816b,  0x0254	
	},	

	3160,	 
	{		
		0x0205,  0x80d7,  0x802e,		
		0x8077,  0x01e0,  0x8069,		
		0x8005,  0x81f5,  0x02fa	
	},

	2470,	 
	{			 
		0x01b9,  0x8086,  0x8033,		 
		0x809b,  0x01c4,  0x8029,		
		0x8068,  0x832e,  0x0496	
	} 

#endif		
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,
	
    /*1,  2,  4,  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768*/
    /* saturation */   
    //{0x7a,0x7a,0x78,0x74,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
    {0x8a,0x8a,0x88,0x84,0x78,0x70,0x68,0x60,0x58,0x50,0x48,0x48,0x48,0x48,0x48,0x48}

};

static HI_S32 sc1045_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4850;
    pstAwbSnsDft->au16GainOffset[0] = 0x18b;    
    pstAwbSnsDft->au16GainOffset[1] = 0x100;    
    pstAwbSnsDft->au16GainOffset[2] = 0x100;    
    pstAwbSnsDft->au16GainOffset[3] = 0x153;    
    pstAwbSnsDft->as32WbPara[0] = 119;    
    pstAwbSnsDft->as32WbPara[1] = -5;    
    pstAwbSnsDft->as32WbPara[2] = -142;    
    pstAwbSnsDft->as32WbPara[3] = 195428;    
    pstAwbSnsDft->as32WbPara[4] = 128;    
    pstAwbSnsDft->as32WbPara[5] = -146085;
    
    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_S32 sc1045_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = sc1045_get_awb_default;

    return 0;
}

#define DMNR_CALIB_CARVE_NUM_SC1045 6

float g_coef_calib_sc1045[DMNR_CALIB_CARVE_NUM_SC1045][4] = 
{
    {100.000000f, 2.000000f, 0.038427f, 9.141136f, }, 

    {200.000000f, 2.301030f, 0.039773f, 9.402012f, }, 

    {400.000000f, 2.602060f, 0.042959f, 9.670882f, }, 

    {847.000000f, 2.927883f, 0.048590f, 10.321005f, }, 

    {1270.000000f, 3.103804f, 0.053094f, 10.733826f, }, 

    {1505.000000f, 3.177536f, 0.063434f, 9.726917f, },

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
	120,/*u16VhLimit*/	
	40-24,/*u16VhOffset*/
	130,   /*u16VhSlope*/
	/*False Color*/
	1,    /*bFcrEnable*/
	{ 8, 8, 8, 8, 8, 8, 8, 8, 3, 0, 0, 0, 0, 0, 0, 0},    /*au8FcrStrength[ISP_AUTO_ISO_STENGTH_NUM]*/
	{24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24},    /*au8FcrThreshold[ISP_AUTO_ISO_STENGTH_NUM]*/
	/*For Ahd*/
	510, /*u16UuSlope*/	
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
        
    {30,  24,  20,  18,  18,  18,  16,  16,    16,  16,  250,   250,   250,   250,    250,    250},/*maxSharpAmt1 = SharpenUD*16 */
    {32, 28, 24, 20,  20,  20,  18,  18,    18,  18,  18,   250,   250,   250,    250,    250},/*maxEdgeAmt = SharpenD*16 */

    //{120,  64,  64,  43,  43,  43,  18,  18,    18,  200,  250,   250,   250,   250,    250,    250},/*maxSharpAmt1 = SharpenUD*16 */  /*9712*/
    //{128, 200, 103, 86,  86,  86,  80,  80,    80,  250,  250,   250,   250,   250,    250,    250},/*maxEdgeAmt = SharpenD*16 */   /*9712*/
       
    {0,   0,   0,    0,   0,   0,   0,   40,  190,  200,  220,   250,   250,   250,     250,       250},/*sharpThd2 = TextureNoiseThd*4 */
    {0,   0,   0,    0,   0,   0,   0,   60,  140,    0,    0,     0,    0,     0,     0,       0},/*edgeThd2 = EdgeNoiseThd*4 */
    {59,  59,  59,  59,  59,  59,  59,   59,  101,  0,   0,    0,   0,    0,    0,      0},/*overshootAmt*/
    {59, 59, 59, 59, 59, 59, 59,  59, 59,  0,   0,    0,   0,    0,    0,     0},/*undershootAmt*/
};

#endif

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

HI_U32 sc1045_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
    pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_SC1045;
    pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_sc1045;

    memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);

    memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
    memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_SC1045_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_SC1045_HEIGHT;

    return 0;
}

HI_U32 sc1045_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
   // HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
          
    pstBlackLevel->au16BlackLevel[0] = 258;
    pstBlackLevel->au16BlackLevel[1] = 258;
    pstBlackLevel->au16BlackLevel[2] = 258;
    pstBlackLevel->au16BlackLevel[3] = 258;
    

    return 0;  
    
}

HI_VOID sc1045_set_pixel_detect(HI_BOOL bEnable)
{
	HI_U32 u32Lines = VMAX_720P30_LINEAR * 30 /5;
	
return;

#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        /* 5 fps */
		sensor_write_register(0x320e, (u32Lines >> 4) && 0xFF);
		sensor_write_register(0x320f, ((u32Lines<<4)&&0xF0));
       
        /* max exposure time*/
		

    }
    else /* setup for ISP 'normal mode' */
    { 
        sensor_write_register(0x320e, (gu32FullLinesStd >> 8) && 0XFF);
        sensor_write_register(0x320f, gu32FullLinesStd && 0xFF);
        
        bInit = HI_FALSE;
    }
#else
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        
		sensor_write_register(0x3e01, (u32Lines >> 8) && 0xFF);
		sensor_write_register(0x3e02, (u32Lines - 4) && 0xFF);
        
        /* min gain */
        sensor_write_register(0x3e0e, 0x00);
		sensor_write_register(0x3e0f, 0x00);

		/* 5 fps */
        sensor_write_register(0x320e, (u32Lines >> 8) && 0xFF);
        sensor_write_register(0x320f, u32Lines && 0xFF);
    }
    else /* setup for ISP 'normal mode' */
    { 
        sensor_write_register(0x320e, (gu32FullLinesStd >> 8) && 0XFF);
        sensor_write_register(0x320f, gu32FullLinesStd && 0xFF);
        
        bInit = HI_FALSE;
    }
#endif

    return;
}

HI_VOID sc1045_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            if (SENSOR_720P_30FPS_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = VMAX_720P30_LINEAR;
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

HI_U32 sc1045_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{

#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE

    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;		
        g_stSnsRegsInfo.u32RegNum = 6;
	
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
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3e09;     //analog agin
		g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 2;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3e0f;     //digita agin

		g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x320e;     //TIMING_VTS  high bit[3:0] 
		g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x320f;     //TIMING_VTS  low bit[7:0] 

	
        bInit = HI_TRUE;
    }
    else    
    {        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)        
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
	
#endif
    return 0;
}

static HI_S32 sc1045_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;

    bInit = HI_FALSE;
    
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if ((pstSensorImageMode->u16Width <= 1280) && (pstSensorImageMode->u16Height <= 720))
    {
        if (WDR_MODE_NONE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = SENSOR_720P_30FPS_MODE;
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

HI_VOID sc1045_global_init()
{   
    gu8SensorImageMode = SENSOR_720P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_720P30_LINEAR; 
    gu32FullLines = VMAX_720P30_LINEAR;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
}

void sc1045_reg_init(void)
{

    SENSOR_I2C_WRITE(0x3003,0x01);//soft reset
    SENSOR_I2C_WRITE(0x3000,0x00);//pause for reg writing
    SENSOR_I2C_WRITE(0x3010,0x01);//output format 43.2M 1920X750 30fps
    SENSOR_I2C_WRITE(0x3011,0xc6);
    SENSOR_I2C_WRITE(0x3004,0x45);
    SENSOR_I2C_WRITE(0x3e03,0x03);//exp and gain
    SENSOR_I2C_WRITE(0x3600,0x94);//0607 reduce power 
    SENSOR_I2C_WRITE(0x3610,0x03);
    SENSOR_I2C_WRITE(0x3634,0x00);// reduce power
    SENSOR_I2C_WRITE(0x3620,0x84);
    SENSOR_I2C_WRITE(0x3631,0x85);// txvdd 0910
    SENSOR_I2C_WRITE(0x3907,0x03);
    SENSOR_I2C_WRITE(0x3622,0x0e);
    SENSOR_I2C_WRITE(0x3633,0x2c);//0825
    SENSOR_I2C_WRITE(0x3630,0x88);
    SENSOR_I2C_WRITE(0x3635,0x80);
    SENSOR_I2C_WRITE(0x3310,0X83);//prechg tx auto ctrl [5] 0825
    SENSOR_I2C_WRITE(0x3336,0x00);
    SENSOR_I2C_WRITE(0x3337,0x00);
    SENSOR_I2C_WRITE(0x3338,0x02);
    SENSOR_I2C_WRITE(0x3339,0xee);
    SENSOR_I2C_WRITE(0x331E,0xa0);//    
    SENSOR_I2C_WRITE(0x3335,0x10);   
    SENSOR_I2C_WRITE(0x3315,0X44);
    SENSOR_I2C_WRITE(0x3308,0X40);
    SENSOR_I2C_WRITE(0x3330,0x0d);// sal_en timing,cancel the fpn in low light
    SENSOR_I2C_WRITE(0x3320,0x05);//0825
    SENSOR_I2C_WRITE(0x3321,0x60);
    SENSOR_I2C_WRITE(0x3322,0x02);
    SENSOR_I2C_WRITE(0x3323,0xc0);
    SENSOR_I2C_WRITE(0x3303,0xa0);
    SENSOR_I2C_WRITE(0x3304,0x60);
    SENSOR_I2C_WRITE(0x3d04,0x04);//0806    vsync gen mode
    SENSOR_I2C_WRITE(0x3d08,0x02);
	SENSOR_I2C_WRITE(0x320e,0x02);//barlow
	SENSOR_I2C_WRITE(0x320f,0xee);//barlow
    SENSOR_I2C_WRITE(0x3000,0x01);//recover 

    bSensorInit = HI_TRUE;
    printf("===========================================================\n");
    printf("===sc1045 sensor 720P30fps(Parallel port) init success!====\n");
    printf("===========================================================\n");


    return;
}



HI_S32 sc1045_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = sc1045_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sc1045_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = sc1045_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = sc1045_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = sc1045_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = sc1045_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = sc1045_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = sc1045_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int sc1045_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    sc1045_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, SC1045_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    sc1045_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, SC1045_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    sc1045_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, SC1045_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int sc1045_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, SC1045_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, SC1045_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, SC1045_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int SmartSens_SC1045_init(SENSOR_DO_I2CRD do_i2c_read, 
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	sc1045_register_callback();
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
	printf("SmartSens SC1045 sensor 720P25fps init success!\n");
	return s32Ret;
}

int SC1045_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_SC1045_WIDTH;
        *ret_height = SENSOR_SC1045_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_SC1045_probe()
{
    uint16_t ret_data1 = 0;
    uint16_t ret_data2 = 0;
	
    SENSOR_I2C_READ(0x3107, &ret_data1);
    SENSOR_I2C_READ(0x3108, &ret_data2);
    if(ret_data1 == SC1045_CHECK_DATA_LSB && ret_data2 == SC1045_CHECK_DATA_MSB){
		//set i2c pinmux
		sdk_sys->write_reg(0x200f0040, 0x2);
	    sdk_sys->write_reg(0x200f0044, 0x2);
		
        sdk_sys->write_reg(0x2003002c, 0xb4001);	 // sensor unreset, clk 27MHz, VI 99MHz
        return true;
    }
    return false;
}
int SC1045_get_sensor_name(char *sensor_name)
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

#endif /* __SC1045_CMOS_H_ */

