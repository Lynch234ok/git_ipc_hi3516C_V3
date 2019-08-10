#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "imx291"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x34, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add + 0x2e00), (_data), 0x34, 2, 1))
////i2c addr + 0x2e00 for imx291	

#define SENSOR_IMX291_WIDTH 1920
#define SENSOR_IMX291_HEIGHT 1080
#define IMX291_CHECK_DATA (0x14)//0X30C2
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__IMX291_CMOS_H_)
#define __IMX291_CMOS_H_

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


#define IMX291_ID 291
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/


#define SHS1_ADDR (0x3020) 
#define SHS2_ADDR (0x3024) 
#define GAIN_ADDR (0x3014)
#define HCG_ADDR  (0x3009)
#define VMAX_ADDR (0x3018)
#define HMAX_ADDR (0x301c)
#define RHS1_ADDR (0x3030)


#define SENSOR_IMX291_1080P_30FPS_MODE  (0)
#define SENSOR_IMX291_720P_60FPS_MODE   (1)

#define VMAX_IMX291_1080P30_LINE (1133) 
#define VMAX_IMX291_720P60_LINE  (1133) 

static HI_U8 gu8SensorImageMode = SENSOR_IMX291_1080P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = SENSOR_IMX291_1080P_30FPS_MODE;
static HI_U32 gu32FullLines = SENSOR_IMX291_1080P_30FPS_MODE;

static HI_U8 gu8HCGReg = 0x02;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};

#define FULL_LINES_MAX  (0x3FFFF)

static const unsigned int sensor_i2c_addr=0x34;
static unsigned int sensor_addr_byte=2;
static unsigned int sensor_data_byte=1;

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
static HI_S32 imx291_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    pstAeSnsDft->u32LinesPer500ms = SENSOR_IMX291_1080P_30FPS_MODE * 30 / 2;
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
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32MinIntTime = 1;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 1;


    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxAgain = 8153234;
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxDgain = 1;
    pstAeSnsDft->u32MinDgain = 1;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain; 
    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 256 << pstAeSnsDft->u32ISPDgainShift; 

    pstAeSnsDft->enIrisType = ISP_IRIS_DC_TYPE;
    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    pstAeSnsDft->u8AERunInterval = 1;

    return 0;
}

/* the function of sensor set fps */
static HI_VOID imx291_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch (gu8SensorImageMode)
    { 
    case SENSOR_IMX291_1080P_30FPS_MODE:
        if ((f32Fps <= 30) && (f32Fps >= 0.5))
        {            
            gu32FullLinesStd = VMAX_IMX291_1080P30_LINE * 30 / f32Fps;
        }
        else
        {
            printf("Not support Fps: %f\n", f32Fps);
            return;
        }
        break;
    case SENSOR_IMX291_720P_60FPS_MODE:
        {
        }
        break;
    default:
        break;        
    }

    gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;

    g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLinesStd & 0xFF);
    g_stSnsRegsInfo.astI2cData[6].u32Data = ((gu32FullLinesStd & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[7].u32Data = ((gu32FullLinesStd & 0x30000) >> 16);
    
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    gu32FullLines = gu32FullLinesStd;
    
    return;
}

static HI_VOID imx291_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{    
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;
    
    g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLines & 0xFF);
    g_stSnsRegsInfo.astI2cData[6].u32Data = ((gu32FullLines & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[7].u32Data = ((gu32FullLines & 0x30000) >> 16);
    
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 2;
    
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID imx291_inttime_update(HI_U32 u32IntTime)
{
    HI_U32 u32Value;
    
    u32Value = gu32FullLines - u32IntTime - 1;

    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32Value & 0xFF);
    g_stSnsRegsInfo.astI2cData[1].u32Data = ((u32Value & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[2].u32Data = ((u32Value & 0x30000) >> 16);

    return;
}

static HI_U32 gain_table[262]=
{
    1024,1059,1097,1135,1175,1217,1259,1304,1349,1397,1446,1497,1549,1604,1660,1719,1779,1842,1906,
    1973,2043,2048,2119,2194,2271,2351,2434,2519,2608,2699,2794,2892,2994,3099,3208,3321,3438,3559,
    3684,3813,3947,4086,4229,4378,4532,4691,4856,5027,5203,5386,5576,5772,5974,6184,6402,6627,6860,
    7101,7350,7609,7876,8153,8439,8736,9043,9361,9690,10030,10383,10748,11125,11516,11921,12340,12774,
    13222,13687,14168,14666,15182,15715,16267,16839,17431,18043,18677,19334,20013,20717,21445,22198,
    22978,23786,24622,25487,26383,27310,28270,29263,30292,31356,32458,33599,34780,36002,37267,38577,
    39932,41336,42788,44292,45849,47460,49128,50854,52641,54491,56406,58388,60440,62564,64763,67039,
    69395,71833,74358,76971,79676,82476,85374,88375,91480,94695,98023,101468,105034,108725,112545,
    116501,120595,124833,129220,133761,138461,143327,148364,153578,158975,164562,170345,176331,182528,
    188942,195582,202455,209570,216935,224558,232450,240619,249074,257827,266888,276267,285976,296026,
    306429,317197,328344,339883,351827,364191,376990,390238,403952,418147,432842,448053,463799,480098,
    496969,514434,532512,551226,570597,590649,611406,632892,655133,678156,701988,726657,752194,778627,
    805990,834314,863634,893984,925400,957921,991585,1026431,1062502,1099841,1138491,1178500,1219916,
    1262786,1307163,1353100,1400651,1449872,1500824,1553566,1608162,1664676,1723177,1783733,1846417,
    1911304,1978472,2048000,2119971,2194471,2271590,2351418,2434052,2519590,2608134,2699789,2794666,
    2892876,2994538,3099773,3208706,3321467,3438190,3559016,3684087,3813554,3947571,4086297,4229898,
    4378546,4532417,4691696,4856573,5027243,5203912,5386788,5576092,5772048,5974890,6184861,6402210,
    6627198,6860092,7101170,7350721,7609041,7876439,8153234
};



static HI_VOID imx291_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if (*pu32AgainLin >= gain_table[261])
    {
         *pu32AgainLin = gain_table[261];
         *pu32AgainDb = 261;
         return ;
    }
    
    for (i = 1; i < 262; i++)
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

static HI_VOID imx291_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{  
    HI_U32 u32HCG = gu8HCGReg;
    
    if(u32Again >= 21)
    {
        u32HCG = u32HCG | 0x10;  // bit[4] HCG  .Reg0x209[7:0]
        u32Again = u32Again - 21;
    }
        
    
    g_stSnsRegsInfo.astI2cData[3].u32Data = (u32Again & 0xFF);
    g_stSnsRegsInfo.astI2cData[4].u32Data = (u32HCG & 0xFF);
    

    return;
}

HI_S32 imx291_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = imx291_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = imx291_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= imx291_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = imx291_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = imx291_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = imx291_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = NULL;

    return 0;
}


/* AWB default parameter and function */
static AWB_CCM_S g_stAwbCcm =
{  
    5291,
    {
        0x01DC, 0x80B2, 0x8029,
        0x805A, 0x0198, 0x803D,
        0x0013, 0x811B, 0x0208
    },
    3184,
    {
        0x01C3, 0x808B, 0x8038,
        0x8085, 0x01A5, 0x8020,
        0x003A, 0x814C, 0x0211
    }, 
    2457,
    {     
        0x0172, 0x801C, 0x8056,
        0x808C, 0x01A0, 0x8014,
        0x801D, 0x8157, 0x0274,
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

static HI_S32 imx291_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4900;
    pstAwbSnsDft->au16GainOffset[0] = 0x1A2;    
    pstAwbSnsDft->au16GainOffset[1] = 0x100;    
    pstAwbSnsDft->au16GainOffset[2] = 0x100;    
    pstAwbSnsDft->au16GainOffset[3] = 0x21D;    

    pstAwbSnsDft->as32WbPara[0] = 72;
    pstAwbSnsDft->as32WbPara[1] = 36;
    pstAwbSnsDft->as32WbPara[2] = -148;
    pstAwbSnsDft->as32WbPara[3] = 190509;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -141264;
    
    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_S32 imx291_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = imx291_get_awb_default;

    return 0;
}

#define DMNR_CALIB_CARVE_NUM_IMX291 7

float g_coef_calib_imx291[DMNR_CALIB_CARVE_NUM_IMX291][4] = 
{  
    {100.000000f, 2.000000f, 0.034498f, 1.543637f, }, 

    {200.000000f, 2.301030f, 0.035741f, 1.199219f, }, 

    {400.000000f, 2.602060f, 0.036418f, 1.446914f, }, 

    {800.000000f, 2.903090f, 0.038867f, 1.214789f, }, 

    {1600.000000f, 3.204120f, 0.045976f, 0.000000f, }, 

    {3200.000000f, 3.505150f, 0.049335f, 1.374389f, }, 

    {6400.000000f, 3.806180f, 0.057599f, 2.818294f, }, 
        
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
	8,/*u16VhOffset*/
	24,   /*u16VhSlope*/
	/*False Color*/
	1,    /*bFcrEnable*/
	{ 8, 8, 8, 8, 8, 8, 8, 8, 3, 0, 0, 0, 0, 0, 0, 0},    /*au8FcrStrength[ISP_AUTO_ISO_STENGTH_NUM]*/
	{24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24},    /*au8FcrThreshold[ISP_AUTO_ISO_STENGTH_NUM]*/
	/*For Ahd*/
	400, /*u16UuSlope*/	
	{512,512,512,512,512,512,512,  400,  0,0,0,0,0,0,0,0}    /*au16NpOffset[ISP_AUTO_ISO_STENGTH_NUM]*/	
};

static ISP_CMOS_DRC_S g_stIspDrcLin =
{
    0,
    10,
    0,
    2,
    180,
    54,
    0,
    0,
    0,
    {1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024}
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
    {0,   0,   0,  0,   0,   0,   0,    0,    0,    1,    1,     1,     1,     1,     1,       1},/* enPixSel */
    {40, 45,  45, 50,  50,  55,  55,   60,   60,   70,   80,    90,   110,   120,   120,     120},/*SharpenUD*/
    {20, 20,  30, 45,  30,  35,  35,   40,   50,   60,   70,    90,   110,   120,   120,     120},/*SharpenD*/
    {0,   2,   4,  6,   6,  12,  30,   60,   80,    0,    0,     0,    0,     0,     0,        0},/*NoiseThd*/
    {2,   4,   8, 16,  25,  11,  12,    0,    0,    0,    0,     0,    0,     0,     0,        0},/*EdgeThd2*/
    {220,230, 200,175, 150, 120, 110,  95,   80,   70,   40,    20,   20,    20,    20,       20},/*overshootAmt*/
    {210,220, 190,140, 135, 130, 110,  95,   75,   60,   50,    50,   50,    50,    50,       50},/*undershootAmt*/

};


static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800};
	{1,	    2,       4,      5,      7,      48,     32,     16,     16,     16,      16,     16,     16,     16,     16,        16},      /*UVNRThreshold*/
 	{0,		0,		0,		0,		0,		0,		0,		0,		0,		1,			1,		2,		2,		2,		2,		2},  /*Coring_lutLimit*/
	{0,		0,		0,		16,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,			34}  /*UVNR_blendRatio*/
};

static ISP_CMOS_DPC_S g_stCmosDpc = 
{
	{0,0,0,1,1,2,2,3,3,3,3,3,3,3,3,3},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0x30,0x60,0x80,0x80,0x80,0xE5,0xE5,0xE5,0xE5},/*au16BlendRatio[16]*/
};


HI_U32 imx291_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
    //pstDef->stDrc.u8LocalMixingBrigtht  = 0x2D;
    //pstDef->stDrc.u8LocalMixingDark     = 0x2D;
    //pstDef->stDrc.u8LocalMixingThres    = 0x02;
    pstDef->stDrc.u16BrightGainLmt      = 0x7F;
    pstDef->stDrc.u16DarkGainLmtC       = 0x7F;
    pstDef->stDrc.u16DarkGainLmtY       = 0x7F;
    pstDef->stDrc.u8RangeVar            = 0x00;
    pstDef->stDrc.u8SpatialVar          = 0x0A;
 */  
 	memcpy(&pstDef->stDrc, &g_stIspDrcLin, sizeof(ISP_CMOS_DRC_S));  
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
    memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));			
    pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_IMX291;
    pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_imx291;

    memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);

    memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
    memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_IMX291_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_IMX291_HEIGHT;

    return 0;
}

HI_U32 imx291_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
          
    pstBlackLevel->au16BlackLevel[0] = 0xF0;
    pstBlackLevel->au16BlackLevel[1] = 0xF0;
    pstBlackLevel->au16BlackLevel[2] = 0xF0;
    pstBlackLevel->au16BlackLevel[3] = 0xF0;
    

    return 0;  
    
}

HI_VOID imx291_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps, u32MaxIntTime_5Fps;

    if(SENSOR_IMX291_1080P_30FPS_MODE == gu8SensorImageMode)
    {
        u32FullLines_5Fps = (VMAX_IMX291_1080P30_LINE * 30) / 5;
    }
    else  if(SENSOR_IMX291_720P_60FPS_MODE == gu8SensorImageMode)
    {
        u32FullLines_5Fps = (VMAX_IMX291_720P60_LINE* 30) / 5;
    }
    else
    {
        return;
    }
    
    u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
    u32MaxIntTime_5Fps =  u32FullLines_5Fps - 2;
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE (GAIN_ADDR - 0x2e00,0x00);
        
        SENSOR_I2C_WRITE (VMAX_ADDR - 0x2e00, u32FullLines_5Fps & 0xFF); 
        SENSOR_I2C_WRITE (VMAX_ADDR - 0x2e00 + 1, (u32FullLines_5Fps & 0xFF00) >> 8); 
        SENSOR_I2C_WRITE (VMAX_ADDR - 0x2e00 + 2, (u32FullLines_5Fps & 0x30000) >> 16);

        SENSOR_I2C_WRITE (SHS1_ADDR - 0x2e00, u32MaxIntTime_5Fps & 0xFF);
        SENSOR_I2C_WRITE (SHS1_ADDR - 0x2e00 + 1,  (u32MaxIntTime_5Fps & 0xFF00) >> 8); 
        SENSOR_I2C_WRITE (SHS1_ADDR - 0x2e00 + 2, (u32MaxIntTime_5Fps & 0x30000) >> 16); 
        
    }
    else /* setup for ISP 'normal mode' */
    {
        gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;
        
        SENSOR_I2C_WRITE (VMAX_ADDR - 0x2e00, gu32FullLines & 0xFF); 
        SENSOR_I2C_WRITE (VMAX_ADDR - 0x2e00 + 1, (gu32FullLines & 0xFF00) >> 8); 
        SENSOR_I2C_WRITE (VMAX_ADDR - 0x2e00 + 2, (gu32FullLines & 0x30000) >> 16);
        bInit = HI_FALSE;
    }

    return;
}

HI_VOID imx291_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            if (SENSOR_IMX291_1080P_30FPS_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = VMAX_IMX291_1080P30_LINE;
            }
            else if(SENSOR_IMX291_720P_60FPS_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = VMAX_IMX291_720P60_LINE;
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

HI_U32 imx291_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;        
        g_stSnsRegsInfo.u32RegNum = 8;
        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }        
        
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = SHS1_ADDR;       
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = SHS1_ADDR + 1;        
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = SHS1_ADDR + 2;   
        
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 0;       //make shutter and gain effective at the same time
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = GAIN_ADDR;  //gain     
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 1;       //make shutter and gain effective at the same time
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = HCG_ADDR;   //gain   
        
        
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = VMAX_ADDR;
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = VMAX_ADDR + 1;
        g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[7].u32RegAddr = VMAX_ADDR + 2;

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

static HI_S32 imx291_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
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
            if (pstSensorImageMode->f32Fps <= 60)
            {
                // TODO: not support
                printf("Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n", 
                    pstSensorImageMode->u16Width, 
                    pstSensorImageMode->u16Height,
                    pstSensorImageMode->f32Fps,
                    genSensorMode);
                return -1;
                u8SensorImageMode = SENSOR_IMX291_720P_60FPS_MODE;
                gu8HCGReg = 0x2;
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
    if ((pstSensorImageMode->u16Width <= 1920) && (pstSensorImageMode->u16Height <= 1080))
    {
        if (WDR_MODE_NONE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = SENSOR_IMX291_1080P_30FPS_MODE;
                gu8HCGReg = 0x2;
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

HI_VOID imx291_global_init()
{   
    gu8SensorImageMode = SENSOR_IMX291_1080P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_IMX291_1080P30_LINE; 
    gu32FullLines = VMAX_IMX291_1080P30_LINE;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
}


void imx291_reg_init()
{

	/* imx291 1080p30 */	
	SENSOR_I2C_WRITE (0x200, 0x01); /* standby */

	SENSOR_DELAY_MS(200); 

	SENSOR_I2C_WRITE (0x207, 0x00);  
	SENSOR_I2C_WRITE (0x209, 0x02);
	SENSOR_I2C_WRITE (0x20A, 0xF0);
	SENSOR_I2C_WRITE (0x20F, 0x00);
	SENSOR_I2C_WRITE (0x210, 0x21);
	SENSOR_I2C_WRITE (0x212, 0x64);
	SENSOR_I2C_WRITE (0x214, 0x20);//Gain
	SENSOR_I2C_WRITE (0x216, 0x09);
	SENSOR_I2C_WRITE (0x216, 0x09);

	SENSOR_I2C_WRITE (0x218, 0x6D); /* VMAX[7:0] */
	SENSOR_I2C_WRITE (0x219, 0x04); /* VMAX[15:8] */
	SENSOR_I2C_WRITE (0x21a, 0x00); /* VMAX[16] */
	SENSOR_I2C_WRITE (0x21b, 0x00); 
	SENSOR_I2C_WRITE (0x21c, 0x30); /* HMAX[7:0] */
	SENSOR_I2C_WRITE (0x21d, 0x11); /* HMAX[15:8] */


	SENSOR_I2C_WRITE (0x220, 0x01);//SHS1
	SENSOR_I2C_WRITE (0x221, 0x00);

	SENSOR_I2C_WRITE (0x246, 0xE1);//LANE CHN
	SENSOR_I2C_WRITE (0x25C, 0x18);
	SENSOR_I2C_WRITE (0x25E, 0x20);

	SENSOR_I2C_WRITE (0x270, 0x02);
	SENSOR_I2C_WRITE (0x271, 0x11);
	SENSOR_I2C_WRITE (0x29B, 0x10);
	SENSOR_I2C_WRITE (0x29C, 0x22);
	SENSOR_I2C_WRITE (0x2A2, 0x02);
	SENSOR_I2C_WRITE (0x2A6, 0x20);
	SENSOR_I2C_WRITE (0x2A8, 0x20);
	SENSOR_I2C_WRITE (0x2AA, 0x20);
	SENSOR_I2C_WRITE (0x2AC, 0x20);
	SENSOR_I2C_WRITE (0x2B0, 0x43);

	SENSOR_I2C_WRITE (0x319, 0x9E);
	SENSOR_I2C_WRITE (0x31C, 0x1E);
	SENSOR_I2C_WRITE (0x31E, 0x08);
	SENSOR_I2C_WRITE (0x328, 0x05);
	SENSOR_I2C_WRITE (0x33D, 0x83);
	SENSOR_I2C_WRITE (0x350, 0x03);

	SENSOR_I2C_WRITE (0x35E, 0x1A);// 1A:37.125MHz 1B:74.25MHz
	SENSOR_I2C_WRITE (0x364, 0x1A);// 1A:37.125MHz 1B:74.25MHz

	SENSOR_I2C_WRITE (0x37C, 0x00);
	SENSOR_I2C_WRITE (0x37E, 0x00);

	SENSOR_I2C_WRITE (0x37E, 0x00);

	SENSOR_I2C_WRITE (0x4B8, 0x50);
	SENSOR_I2C_WRITE (0x4B9, 0x10);
	SENSOR_I2C_WRITE (0x4BA, 0x00);
	SENSOR_I2C_WRITE (0x4BB, 0x04);
	SENSOR_I2C_WRITE (0x4C8, 0x50);
	SENSOR_I2C_WRITE (0x4C9, 0x10);
	SENSOR_I2C_WRITE (0x4CA, 0x00);
	SENSOR_I2C_WRITE (0x4CB, 0x04);

	SENSOR_I2C_WRITE (0x52C, 0xD3);
	SENSOR_I2C_WRITE (0x52D, 0x10);
	SENSOR_I2C_WRITE (0x52E, 0x0D);
	SENSOR_I2C_WRITE (0x558, 0x06);
	SENSOR_I2C_WRITE (0x559, 0xE1);
	SENSOR_I2C_WRITE (0x55A, 0x11);
	SENSOR_I2C_WRITE (0x560, 0x1E);
	SENSOR_I2C_WRITE (0x561, 0x61);
	SENSOR_I2C_WRITE (0x562, 0x10);
	SENSOR_I2C_WRITE (0x5B0, 0x50);
	SENSOR_I2C_WRITE (0x5B2, 0x1A);
	SENSOR_I2C_WRITE (0x5B3, 0x04);

	SENSOR_DELAY_MS(200);
	SENSOR_I2C_WRITE (0x200, 0x00); /* standby */
	SENSOR_I2C_WRITE (0x202, 0x00); /* master mode start */
	SENSOR_I2C_WRITE (0x249, 0x0A); /* XVSOUTSEL XHSOUTSEL */

	bSensorInit = HI_TRUE;
	printf("==============================================================\n");
	printf("===Sony imx291 sensor 1080P30fps(LVDS port) init success!=====\n");
	printf("==============================================================\n");

	return;

}
HI_S32 imx291_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = imx291_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = imx291_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = imx291_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = imx291_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = imx291_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = imx291_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = imx291_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = imx291_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int imx291_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    imx291_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, IMX291_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    imx291_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, IMX291_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    imx291_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, IMX291_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int imx291_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, IMX291_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, IMX291_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, IMX291_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int SONY_IMX291_init(SENSOR_DO_I2CRD do_i2c_read, 
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	imx291_register_callback();

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
	return s32Ret;
}

int IMX291_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_IMX291_WIDTH;
        *ret_height = SENSOR_IMX291_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_IMX291_probe()
{
    uint16_t ret_data1 = 0;
	
    SENSOR_I2C_READ(0x30c2, &ret_data1);
	
    if(ret_data1 == IMX291_CHECK_DATA ){
		//set i2c pinmux
		sdk_sys->write_reg(0x200f0040, 0x2);
	    sdk_sys->write_reg(0x200f0044, 0x2);
		
        sdk_sys->write_reg(0x2003002c, 0x94005);	 // sensor unreset, clk 37.125MHz, VI 148.5MHz
        sdk_sys->write_reg(0x20030104, 0x1);
        return true;
    }
    return false;
}

int IMX291_get_sensor_name(char *sensor_name)
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

#endif /* __SOIIMX291_CMOS_H_ */


