#include "sdk/sdk_debug.h"
#include "hi3516a.h"
#include "hi3516a_isp_sensor.h"
#include "sdk/sdk_sys.h"
#include "hi_isp_i2c.h"

static SENSOR_DO_I2CRD sensor_i2c_read=NULL;
static SENSOR_DO_I2CWR sensor_i2c_write=NULL;

#define SENSOR_NAME "ov5658"
#define SENSOR_I2C_READ(_add, _ret_data)\
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x6c, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
		(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x6c, 2, 1))

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#define SENSOR_OV5658_WIDTH 2592
#define SENSOR_OV5658_HEIGHT 1944
#define OV5658_CHECK_DATA_LSB (0X56) //0X300B
#define OV5658_CHECK_DATA_MSB (0X56) //0X300A

#if !defined(__OV5658_CMOS_H_)
#define __OV5658_CMOS_H_

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

#define OV5658_ID 5658

/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/
#ifdef INIFILE_CONFIG_MODE

extern AE_SENSOR_DEFAULT_S  g_AeDft[];
extern AWB_SENSOR_DEFAULT_S g_AwbDft[];
extern ISP_CMOS_DEFAULT_S   g_IspDft[];
extern HI_S32 OV5658_LoadINIPara(const HI_CHAR *pcName);
#else

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned char sensor_i2c_addr = 0x6c;
/* I2C Address of OV5658 */      
static unsigned int  sensor_addr_byte  =  2;
static unsigned int  sensor_data_byte  =  1;

#define GAIN_ADDR     (0x350A)
#define VMAX_ADDR     (0x380E)
#define SHS_ADDR      (0x3500)

#define INCREASE_LINES          (1) /* make real fps less than stand fps because NVR require*/
#define SENSOR_5M_30FPS_MODE    (1) 
#define VMAX_5M30               (1984+INCREASE_LINES)

static HI_U8 gu8SensorImageMode = SENSOR_5M_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = VMAX_5M30;
static HI_U32 gu32FullLines = VMAX_5M30;
static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE; 

static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};

#define PATHLEN_MAX 256
#define CMOS_CFG_INI "ov5658_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/ov5658_cfg.ini";

/* AE default parameter and function */

#ifdef INIFILE_CONFIG_MODE

static HI_S32 ov5658_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }
    
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.1;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.1;
    
    /* linear mode */
    switch(genSensorMode)
    {
        default:
        case WDR_MODE_NONE:   /*linear mode*/
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = g_AeDft[0].u8AeCompensation;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[0].u32MaxIntTimeTarget;
            pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[0].u32MinIntTimeTarget;
            
            pstAeSnsDft->u32MaxAgain = 63488;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = g_AeDft[0].u32MaxAgainTarget;
            pstAeSnsDft->u32MinAgainTarget = g_AeDft[0].u32MinAgainTarget;
            
            pstAeSnsDft->u32MaxDgain = 1024;  
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = g_AeDft[0].u32MaxDgainTarget;
            pstAeSnsDft->u32MinDgainTarget = g_AeDft[0].u32MinDgainTarget;
          
            pstAeSnsDft->u32ISPDgainShift = g_AeDft[0].u32ISPDgainShift;
            pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[0].u32MinISPDgainTarget;
            pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[0].u32MaxISPDgainTarget; 
        break;
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE: /*linear mode for ISP frame switching WDR*/
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
           
            pstAeSnsDft->u8AeCompensation = g_AeDft[1].u8AeCompensation;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[1].u32MaxIntTimeTarget;
            pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[1].u32MinIntTimeTarget;
            
            pstAeSnsDft->u32MaxAgain = 63488;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = g_AeDft[1].u32MaxAgainTarget;
            pstAeSnsDft->u32MinAgainTarget = g_AeDft[1].u32MinAgainTarget;
            
            pstAeSnsDft->u32MaxDgain = 1024;  
            pstAeSnsDft->u32MinDgain = 1024;
            pstAeSnsDft->u32MaxDgainTarget = g_AeDft[1].u32MaxDgainTarget;
            pstAeSnsDft->u32MinDgainTarget = g_AeDft[1].u32MinDgainTarget;
          
            pstAeSnsDft->u32ISPDgainShift = g_AeDft[1].u32ISPDgainShift;
            pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[1].u32MinISPDgainTarget;
            pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[1].u32MaxISPDgainTarget;            
        break;
    }
    return 0;
}

#else

static HI_S32 ov5658_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }
    
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0625;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;
    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 8 << pstAeSnsDft->u32ISPDgainShift; 

    switch(genSensorMode)
    {
        default:
        case WDR_MODE_NONE:
            /* linear mode */
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x38; 
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535 -4;
            pstAeSnsDft->u32MinIntTimeTarget = 2;
            
            pstAeSnsDft->u32MaxAgain = 63488;//255;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
            
            pstAeSnsDft->u32MaxDgain = 1;  
            pstAeSnsDft->u32MinDgain = 1;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
        break;
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            /* FS WDR mode */
            pstAeSnsDft->au8HistThresh[0] = 0xc;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x38; 
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535 -4;
            pstAeSnsDft->u32MinIntTimeTarget = 2;
            
            pstAeSnsDft->u32MaxAgain = 63488;//255;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
            
            pstAeSnsDft->u32MaxDgain = 1;  
            pstAeSnsDft->u32MinDgain = 1;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;    
        break;
        }
    
    return 0;
}

#endif


#define AGAIN_NODE_NUM 96

static HI_U32 au32Again_table[AGAIN_NODE_NUM]=
{    
    1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,
    2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,
    4096,4352,4608,4864,5120,5376,5632,5888,6144,6400,6656,6912,7168,7424,7680,7936,
    8192,8704,9216,9728,10240,10752,11264,11776,12288,12800,13312,13824,14336,14848,
    15360,15872,16384,17408,18432,19456,20480,21504,22528,23552,24576,25600,26624,
    27648,28672,29696,30720,31744,32768,34816,36864,38912,40960,43008,45056,47104,
    49152,51200,53248,55296,57344,59392,61440,63488
};


static HI_VOID ov5658_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if((HI_NULL == pu32AgainLin) ||(HI_NULL == pu32AgainDb))
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

    if (*pu32AgainLin >= au32Again_table[AGAIN_NODE_NUM -1])
    {
         *pu32AgainLin = au32Again_table[AGAIN_NODE_NUM -1];
         *pu32AgainDb = AGAIN_NODE_NUM -1;
         return ;
    }
    
    for (i = 1; i < AGAIN_NODE_NUM; i++)
    {
        if (*pu32AgainLin < au32Again_table[i])
        {
            *pu32AgainLin = au32Again_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }
    
    return;
}



/* the function of sensor set fps */

static HI_VOID ov5658_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (SENSOR_5M_30FPS_MODE == gu8SensorImageMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 0.5))
        {
            gu32FullLinesStd = VMAX_5M30*30/f32Fps;
        }
        else
        {
            printf("Not support Fps: %f\n", f32Fps);
            return;
        }
    }
    else
    {
        printf("Not support! gu8SensorImageMode:%d, f32Fps:%f\n", gu8SensorImageMode, f32Fps);
        return;
    }

    gu32FullLinesStd = (gu32FullLinesStd > 0xFFFF) ? 0xFFFF : gu32FullLinesStd;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[5].u32Data = ((gu32FullLinesStd & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[6].u32Data = (gu32FullLinesStd & 0xFF);
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[8].u32Data = ((gu32FullLinesStd & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[9].u32Data = (gu32FullLinesStd & 0xFF);
    }

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    gu32FullLines = gu32FullLinesStd;

    return;
}


static HI_VOID ov5658_slow_framerate_set(HI_U32 u32FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > 0xFFFF) ? 0xFFFF : u32FullLines;
    gu32FullLines= u32FullLines;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[5].u32Data = ((gu32FullLines & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[6].u32Data = (gu32FullLines & 0xFF);
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[8].u32Data = ((gu32FullLines & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[9].u32Data = (gu32FullLines & 0xFF);
    }

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;
    
    return;
}


/* while isp notify ae to update sensor regs, ae call these funcs. */

static HI_VOID ov5658_inttime_update(HI_U32 u32IntTime)
{    
    static HI_BOOL bFirst = HI_TRUE;

    if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
    {
        if(bFirst)
        {
            g_stSnsRegsInfo.astI2cData[0].u32Data = ((u32IntTime & 0xF000) >> 12);
            g_stSnsRegsInfo.astI2cData[1].u32Data = ((u32IntTime & 0x0FF0) >> 4);
            g_stSnsRegsInfo.astI2cData[2].u32Data = ((u32IntTime & 0x000F) << 4);
            bFirst = HI_FALSE;
        }
        else
        {
            g_stSnsRegsInfo.astI2cData[5].u32Data = ((u32IntTime & 0xF000) >> 12);
            g_stSnsRegsInfo.astI2cData[6].u32Data = ((u32IntTime & 0x0FF0) >> 4);
            g_stSnsRegsInfo.astI2cData[7].u32Data = ((u32IntTime & 0x000F) << 4);
            bFirst = HI_TRUE;
        }
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[0].u32Data = ((u32IntTime & 0xF000) >> 12);
        g_stSnsRegsInfo.astI2cData[1].u32Data = ((u32IntTime & 0x0FF0) >> 4);
        g_stSnsRegsInfo.astI2cData[2].u32Data = ((u32IntTime & 0x000F) << 4);            
    }
    

    return;
}



static HI_VOID ov5658_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{  
    HI_U32 u32NumOf1s = 0;
    HI_U32 u32NumOf1sReg =0;
    u32NumOf1s = u32Again>>4;        
    
    while(u32NumOf1s)
    {
        u32NumOf1sReg = (u32NumOf1sReg << 1) + 1;
        u32NumOf1s --;
    }

    g_stSnsRegsInfo.astI2cData[3].u32Data = (u32NumOf1sReg >> 4);
    g_stSnsRegsInfo.astI2cData[4].u32Data = ( (u32NumOf1sReg & 0x0F) <<4) + (u32Again & 0x0F); 

    return;
}


/* Only used in WDR_MODE_2To1_LINE and WDR_MODE_2To1_FRAME mode */
static HI_VOID ov5658_get_inttime_max(HI_U32 u32Ratio, HI_U32 *pu32IntTimeMax)
{
    if(HI_NULL == pu32IntTimeMax)
    {
        printf("null pointer when get ae sensor IntTimeMax value!\n");
        return;
    }

    if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
    {
        *pu32IntTimeMax = (gu32FullLinesStd - 4) * 0x40 / DIV_0_TO_1(u32Ratio);
    }

    return;
}



HI_S32 ov5658_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ov5658_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ov5658_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ov5658_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ov5658_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ov5658_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ov5658_again_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = ov5658_get_inttime_max;

    return 0;
}


/* AWB default parameter and function */
#ifdef INIFILE_CONFIG_MODE

static HI_S32 ov5658_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    HI_U8 i;
    
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    switch (genSensorMode)
    {
        default:
        case WDR_MODE_NONE: 
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            pstAwbSnsDft->u16WbRefTemp = g_AwbDft[0].u16WbRefTemp;
            for(i= 0; i < 4; i++)
            {
                pstAwbSnsDft->au16GainOffset[i] = g_AwbDft[0].au16GainOffset[i];
            }
            for(i= 0; i < 6; i++)
            {
                pstAwbSnsDft->as32WbPara[i] = g_AwbDft[0].as32WbPara[i];
            }
            memcpy(&pstAwbSnsDft->stCcm, &g_AwbDft[0].stCcm, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_AwbDft[0].stAgcTbl, sizeof(AWB_AGC_TABLE_S));
        break;
    }
    return 0;
}

#else

static AWB_CCM_S g_stAwbCcm =
{
   4794,
   {
	   0x1C7,0x80AC,0x801B,
	   0x8043,0x1A8,0x8065,
	   0x8016,0x80AF,0x1C5

   },
   
   3618,
   {
	   0x1C2,0x8092,0x8030,
	   0x8048,0x17B,0x8033,
	   0x801D,0x80C5,0x1E2

   },
   
   2358,
   {     
	   0x210,0x80F0,0x8020,
	   0x8011,0x114,0x8003,
	   0x8035,0x80B6,0x1EB

   }
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x82,0x82,0x7C,0x78,0x76,0x70,0x68,0x7b,0x3c,0x3c,0x38,0x38,0x38,0x38,0x38,0x38}
};

static AWB_AGC_TABLE_S g_stAwbAgcTableFSWDR =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x80,0x80,0x7e,0x72,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
};


static HI_S32 ov5658_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4800;

    pstAwbSnsDft->au16GainOffset[0] = 0x153;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x15C;

    pstAwbSnsDft->as32WbPara[0] = 85;
    pstAwbSnsDft->as32WbPara[1] = 26;
    pstAwbSnsDft->as32WbPara[2] = -143;
    pstAwbSnsDft->as32WbPara[3] = 127549;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -79660;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));

    switch (genSensorMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
        break;
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableFSWDR, sizeof(AWB_AGC_TABLE_S));
        break;
    }

    return 0;
}

#endif


HI_S32 ov5658_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ov5658_get_awb_default;

    return 0;
}


/* ISP default parameter and function */
#ifdef INIFILE_CONFIG_MODE

HI_U32 ov5658_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{   
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

    switch (genSensorMode)
    {
        default:
        case WDR_MODE_NONE:           
            memcpy(&pstDef->stDrc, &g_IspDft[0].stDrc, sizeof(ISP_CMOS_DRC_S));
            memcpy(&pstDef->stAgcTbl, &g_IspDft[0].stAgcTbl, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stNoiseTbl, &g_IspDft[0].stNoiseTbl, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_IspDft[0].stDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_IspDft[0].stRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
            memcpy(&pstDef->stGamma, &g_IspDft[0].stGamma, sizeof(ISP_CMOS_GAMMA_S));
        break;
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:            
            memcpy(&pstDef->stDrc, &g_IspDft[1].stDrc, sizeof(ISP_CMOS_DRC_S));
            memcpy(&pstDef->stAgcTbl, &g_IspDft[1].stAgcTbl, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stNoiseTbl, &g_IspDft[1].stNoiseTbl, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_IspDft[1].stDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_IspDft[1].stRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));            
            memcpy(&pstDef->stGamma, &g_IspDft[1].stGamma, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stGammafe, &g_IspDft[1].stGammafe, sizeof(ISP_CMOS_GAMMAFE_S));
        break;

    }

    pstDef->stSensorMaxResolution.u32MaxWidth  = 2592;
    pstDef->stSensorMaxResolution.u32MaxHeight = 1944;

    return 0;
}

#else

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
    
    /* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */

    /* sharpen_alt_d */
 //   {0x26,0x24,0x20,0x1d,0x18,0x16,0x14,0x12,0x10,0xc,0xa,0x9,0x7,0x5,0x5,0x5},
    {110,107,106,105,103,100,88,70,60,50,40,20,18,16,16,16},
        
    /* sharpen_alt_ud */
//    {0x44,0x42,0x40,0x3d,0x3a,0x36,0x30,0x26,0x22,0x20,0x18,0x12,0x10,0x10,0x10,0x10},
    {95,94,91,90,88,82,75,55,45,40,24,18,16,16,16,16},
        
    /* snr_thresh Max=0x54 */
//    {0x08,0x0a,0x0f,0x12,0x16,0x1a,0x22,0x28,0x2e,0x36,0x3a,0x40,0x40,0x40,0x40,0x40},
    {15,25,35,50,53,60,70,75,75,75,75,75,75,75,75,75},
        
    /* demosaic_lum_thresh */
    {0x50,0x50,0x4e,0x49,0x45,0x45,0x40,0x3a,0x3a,0x30,0x30,0x2a,0x20,0x20,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x00,0x0a,0x12,0x1a,0x20,0x28,0x30,0x32,0x34,0x36,0x38,0x38,0x38,0x38,0x38,0x38},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37,0x37,0x35,0x35,0x35,0x35,0x35,0x35,0x35},

    /* rgbsharp_strength */
 //   {0x58,0x50,0x48,0x44,0x40,0x60,0x58,0x50,0x40,0x30,0x20,0x16,0x12,0x12,0x12,0x12}
	{ 50,50,48,46,46,35,30,30,30,28,25,22,18,18,18,18}
};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTableFSWDR =
{
    /* bvalid */
    1,
    
    /* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */
    /* sharpen_alt_d */
    {0x26,0x24,0x20,0x1d,0x18,0x16,0x14,0x12,0x10,0xc,0xa,0x9,0x7,0x5,0x5,0x5},
        
    /* sharpen_alt_ud */
    {0x44,0x42,0x40,0x3d,0x3a,0x36,0x30,0x26,0x22,0x20,0x18,0x12,0x10,0x10,0x10,0x10},
        
    /* snr_thresh Max=0x54 */
    {0x08,0x0a,0x0f,0x12,0x16,0x1a,0x22,0x28,0x2e,0x36,0x3a,0x40,0x40,0x40,0x40,0x40},
        
    /* demosaic_lum_thresh */
    {0x50,0x50,0x4e,0x49,0x45,0x45,0x40,0x3a,0x3a,0x30,0x30,0x2a,0x20,0x20,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x00,0x0a,0x12,0x1a,0x20,0x28,0x30,0x32,0x34,0x36,0x38,0x38,0x38,0x38,0x38,0x38},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37,0x37,0x35,0x35,0x35,0x35,0x35,0x35,0x35},

    /* rgbsharp_strength */
    {0x58,0x50,0x48,0x44,0x40,0x60,0x58,0x50,0x40,0x30,0x20,0x16,0x12,0x12,0x12,0x12}
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,

    /* nosie_profile_weight_lut */
    {
        0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x8,  0xc,  0xf, 0x12, 0x14, 0x16, 0x17, 0x18, 0x19,
        0x1b, 0x1c, 0x1c, 0x1d, 0x1e, 0x1f, 0x1f, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24,
        0x25, 0x25, 0x26, 0x26, 0x26, 0x27, 0x27, 0x27, 0x28, 0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a,
        0x2a, 0x2a, 0x2b, 0x2b, 0x2b, 0x2b, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2d, 0x2d, 0x2d, 0x2d, 0x2e,
        0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
        0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x33,
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34,
        0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
    },   

    /* demosaic_weight_lut */
    {
        0x1,  0x8,  0xc,  0xf, 0x12, 0x14, 0x16, 0x17, 0x18, 0x19, 0x1b, 0x1c, 0x1c, 0x1d, 0x1e, 0x1f,
        0x1f, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x26, 0x27,
        0x27, 0x27, 0x28, 0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a, 0x2a, 0x2a, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2d, 0x2d, 0x2d, 0x2d, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2f,
        0x2f, 0x2f, 0x2f, 0x2f, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31,
        0x31, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
        0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35,
        0x35, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
    }        
    
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTableFSWDR =
{
    /* bvalid */
    1,

    /* nosie_profile_weight_lut */
    {
        0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,  0x8,  0xc,  0xf, 0x12, 0x14, 0x16, 0x17, 0x18, 0x19,
        0x1b, 0x1c, 0x1c, 0x1d, 0x1e, 0x1f, 0x1f, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24,
        0x25, 0x25, 0x26, 0x26, 0x26, 0x27, 0x27, 0x27, 0x28, 0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a,
        0x2a, 0x2a, 0x2b, 0x2b, 0x2b, 0x2b, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2d, 0x2d, 0x2d, 0x2d, 0x2e,
        0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
        0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x33,
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34,
        0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
    },   

    /* demosaic_weight_lut */
    {
        0x1,  0x8,  0xc,  0xf, 0x12, 0x14, 0x16, 0x17, 0x18, 0x19, 0x1b, 0x1c, 0x1c, 0x1d, 0x1e, 0x1f,
        0x1f, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x26, 0x27,
        0x27, 0x27, 0x28, 0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a, 0x2a, 0x2a, 0x2b, 0x2b, 0x2b, 0x2b,
        0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2d, 0x2d, 0x2d, 0x2d, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2f,
        0x2f, 0x2f, 0x2f, 0x2f, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31,
        0x31, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
        0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35,
        0x35, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
    }        
};


static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xac,

    /*aa_slope*/
    0xaa,

    /*va_slope*/
    0xa8,

    /*uu_slope*/
    0xa0,

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


static ISP_CMOS_DEMOSAIC_S g_stIspDemosaicFSWDR =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xac,

    /*aa_slope*/
    0xaa,

    /*va_slope*/
    0xa8,

    /*uu_slope*/
    0xa0,

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

static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpenFSWDR =
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
    {  
        0, 180, 320, 426, 516, 590, 660, 730, 786, 844, 896, 946, 994, 1040, 1090, 1130, 1170, 1210, 1248,
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

static ISP_CMOS_GAMMA_S g_stIspGammaFSWDR =
{
    /* bvalid */
    1,
    
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
        4009,4022,4034,4046,4058,4071,4083,4095
    }
};

static ISP_CMOS_GAMMAFE_S g_stGammafeFSWDR = 
{
    /* bvalid */
    1,

    /* gamma_fe0 */
    {
        0, 17537, 35074, 36089, 37105, 38120, 39136, 40151, 41166, 42182, 43197, 44213, 45228, 46243, 47259, 48274, 49290, 50305, 51320, 52336, 53351, 54367, 55382, 56397, 57413, 58428, 59444, 60459, 61474, 62490, 63505, 64521, 65535
    },

    /* gamma_fe1 */
    {
        0, 73, 147, 221, 296, 371, 447, 524, 601, 678, 757, 837, 916, 997, 1078, 1160, 1243, 1326, 1410, 1496, 1582, 1669, 1757, 1846, 1936, 2026, 2119, 2212, 2305, 2401, 2498, 2595, 2694, 2795, 2897, 3000, 3105, 3212, 3320, 3430, 3542, 3656, 3772, 3890, 4011, 4133, 4259, 4388, 4519, 4654, 4792, 4935, 5081, 5231, 5387, 5548, 5715, 5889, 6071, 6262, 6462, 6676, 6903, 7150, 7420, 7722, 8071, 8500, 9122, 10827, 11453, 11883, 12233, 12535, 12805, 13051, 13279, 13492, 13693, 13884, 14066, 14240, 14407, 14568, 14724, 14875, 15021, 15163, 15301, 15436, 15568, 15696, 15822, 15945, 16066, 16184, 16300, 16414, 16526, 16636, 16744, 16851, 16956, 17059, 17161, 17261, 17361, 17459, 17555, 17650, 17745, 17837, 17929, 18021, 18110, 18199, 18287, 18374, 18460, 18545, 18630, 18713, 18796, 18878, 18959, 19040, 19120, 19199, 19277, 19355, 19432, 19509, 19585, 19660, 19735, 19809, 19883, 19956, 21136, 22209, 23196, 24114, 24975, 25789, 26564, 27304, 28014, 28697, 29357, 29994, 30611, 31210, 31793, 32361, 32915, 33456, 33985, 34503, 35009, 35506, 35993, 36471, 36941, 37402, 37856, 38302, 38742, 39175, 39602, 40023, 40437, 40846, 41250, 41649, 42043, 42432, 42817, 43197, 43573, 43945, 44312, 44676, 45035, 45392, 45744, 46093, 46439, 46782, 47121, 47458, 47791, 48122, 48449, 48774, 49096, 49416, 49733, 50047, 50360, 50669, 50977, 51282, 51584, 51885, 52183, 52480, 52774, 53066, 53357, 53645, 53931, 54216, 54499, 54780, 55059, 55337, 55612, 55887, 56160, 56431, 56700, 56968, 57234, 57499, 57762, 58024, 58285, 58544, 58802, 59058, 59313, 59567, 59819, 60070, 60320, 60569, 60817, 61063, 61308, 61552, 61795, 62037, 62277, 62517, 62755, 62992, 63229, 63464, 63698, 63931, 64163, 64394, 64624, 64854, 65082, 65309, 65535
    }
};



HI_U32 ov5658_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            pstDef->stDrc.bEnable               = HI_FALSE;
            pstDef->stDrc.u32BlackLevel         = 0x00;
            pstDef->stDrc.u32WhiteLevel         = 0x4FF; 
            pstDef->stDrc.u32SlopeMax           = 0x30;
            pstDef->stDrc.u32SlopeMin           = 0x00;
            pstDef->stDrc.u32VarianceSpace      = 0x04;
            pstDef->stDrc.u32VarianceIntensity  = 0x01;
            
            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
            memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));
        break;
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            
            pstDef->stDrc.bEnable               = HI_TRUE;
            pstDef->stDrc.u32BlackLevel         = 0x00;
            pstDef->stDrc.u32WhiteLevel         = 0xFFF; 
            pstDef->stDrc.u32SlopeMax           = 0x38;
            pstDef->stDrc.u32SlopeMin           = 0xC0;
            pstDef->stDrc.u32VarianceSpace      = 0x0A;
            pstDef->stDrc.u32VarianceIntensity  = 0x04;

            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTableFSWDR, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTableFSWDR, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaicFSWDR, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpenFSWDR, sizeof(ISP_CMOS_RGBSHARPEN_S));            
            memcpy(&pstDef->stGamma, &g_stIspGammaFSWDR, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stGammafe, &g_stGammafeFSWDR, sizeof(ISP_CMOS_GAMMAFE_S));
        break;
        default:
        break;
    }

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_OV5658_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_OV5658_HEIGHT;

    return 0;
}

#endif


HI_U32 ov5658_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{

    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

	pstBlackLevel->au16BlackLevel[0] = 67;
	pstBlackLevel->au16BlackLevel[1] = 67;
	pstBlackLevel->au16BlackLevel[2] = 67;
	pstBlackLevel->au16BlackLevel[3] = 67;
	
    return 0;    
}


HI_VOID ov5658_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps = VMAX_5M30;
    HI_U32 u32MaxExpTime_5Fps = VMAX_5M30 - 2;
    
    if (SENSOR_5M_30FPS_MODE == gu8SensorImageMode)
    {
        u32FullLines_5Fps = VMAX_5M30 * 30 / 5;
    }
    else
    {
        return;
    }

    u32FullLines_5Fps = (u32FullLines_5Fps > 0xFFFF) ? 0xFFFF : u32FullLines_5Fps;
    u32MaxExpTime_5Fps = u32FullLines_5Fps - 4;
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR+1, u32FullLines_5Fps & 0xFF); 
        SENSOR_I2C_WRITE(VMAX_ADDR, (u32FullLines_5Fps & 0xFF00) >> 8 );
        
        SENSOR_I2C_WRITE(SHS_ADDR, (u32MaxExpTime_5Fps & 0xF000) >> 12);    /* shutter */
        SENSOR_I2C_WRITE(SHS_ADDR+1, (u32MaxExpTime_5Fps & 0x0FF0) >> 4);
        SENSOR_I2C_WRITE(SHS_ADDR+2, (u32MaxExpTime_5Fps & 0x000F) << 4);
        
        SENSOR_I2C_WRITE(GAIN_ADDR, 0x00); //gain
        SENSOR_I2C_WRITE(GAIN_ADDR+1, 0x10);
    }
    else /* setup for ISP 'normal mode' */
    {
        gu32FullLinesStd = (gu32FullLinesStd > 0xFFFF) ? 0xFFFF : gu32FullLinesStd;
        SENSOR_I2C_WRITE (VMAX_ADDR+1, gu32FullLinesStd & 0xFF); 
        SENSOR_I2C_WRITE (VMAX_ADDR, (gu32FullLinesStd & 0xFF00) >> 8);
    }

    return;
}

HI_VOID ov5658_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            genSensorMode = WDR_MODE_NONE;
            printf("linear mode\n");
        break;

        case WDR_MODE_2To1_FRAME:
            genSensorMode = WDR_MODE_2To1_FRAME;        
            printf("2to1 half-rate frame WDR mode\n");
        break;

        case WDR_MODE_2To1_FRAME_FULL_RATE:
            genSensorMode = WDR_MODE_2To1_FRAME_FULL_RATE;
            printf("2to1 full-rate frame WDR mode\n");
        break;

        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    return;
}


static HI_S32 ov5658_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;
    
    bInit = HI_FALSE;    
    
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if((pstSensorImageMode->u16Width <= 2592)&&(pstSensorImageMode->u16Height <= 1944))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_5M_30FPS_MODE;
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


HI_U32 ov5658_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;        
        g_stSnsRegsInfo.u32RegNum = 7;

        if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
        {
            g_stSnsRegsInfo.u32RegNum += 3;
        }
        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }        
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = SHS_ADDR;      
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = SHS_ADDR+1;      
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = SHS_ADDR+2;      
        
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = GAIN_ADDR;        
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = GAIN_ADDR+1;    

        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = VMAX_ADDR;       
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = VMAX_ADDR+1;

        if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
        {
            g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[5].u32RegAddr = SHS_ADDR;      
            g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[6].u32RegAddr = SHS_ADDR+1;      
            g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[7].u32RegAddr = SHS_ADDR+2; 
            g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[8].u32RegAddr = VMAX_ADDR;       
            g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[9].u32RegAddr = VMAX_ADDR+1;    
        }

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

        if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
        {
            g_stSnsRegsInfo.astI2cData[0].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[1].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[2].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[5].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[6].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[7].bUpdate = HI_TRUE;
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


int  ov5658_set_inifile_path(const char *pcPath)
{
    if(strlen(pcPath) > (PATHLEN_MAX - 30))
    {
        printf("Set inifile path is larger PATHLEN_MAX!\n");
        return -1;        
    }

    memset(pcName, 0, sizeof(pcName));
        
    if (HI_NULL == pcPath)
    {        
        strncat(pcName, "configs/", strlen("configs/"));
        strncat(pcName, CMOS_CFG_INI, sizeof(CMOS_CFG_INI));
    }
    else
    {
        strncat(pcName, pcPath, strlen(pcPath));
        strncat(pcName, CMOS_CFG_INI, sizeof(CMOS_CFG_INI));
    }
    
    return 0;
}


HI_VOID ov5658_global_init()
{   
    gu8SensorImageMode = SENSOR_5M_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;   
    gu32FullLinesStd = VMAX_5M30;
    gu32FullLines = VMAX_5M30;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 
    
    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    
#ifdef INIFILE_CONFIG_MODE 
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = OV5658_LoadINIPara(pcName);
    if (HI_SUCCESS != s32Ret)
    {
        printf("OV5658_LoadINIPara failed!!!!!!\n");
    }
#else

#endif    
}


void ov5658_reg_init()
{
    SENSOR_I2C_WRITE(0x0103, 0x01); 
    SENSOR_I2C_WRITE(0x3210, 0x43);
    SENSOR_I2C_WRITE(0x3001, 0x0e);
    SENSOR_I2C_WRITE(0x3002, 0xc0);
    SENSOR_I2C_WRITE(0x3011, 0x41);
    SENSOR_I2C_WRITE(0x3012, 0x0a);
    SENSOR_I2C_WRITE(0x3013, 0x50);
    SENSOR_I2C_WRITE(0x3015, 0x09);
    SENSOR_I2C_WRITE(0x3018, 0xf0);
    SENSOR_I2C_WRITE(0x3021, 0x40);
    SENSOR_I2C_WRITE(0x3500, 0x00);
    SENSOR_I2C_WRITE(0x3501, 0x7b);
    SENSOR_I2C_WRITE(0x3502, 0x00);
    SENSOR_I2C_WRITE(0x3503, 0x07);
    SENSOR_I2C_WRITE(0x3505, 0x00);
    SENSOR_I2C_WRITE(0x3506, 0x00);
    SENSOR_I2C_WRITE(0x3507, 0x02);
    SENSOR_I2C_WRITE(0x3508, 0x00);
    SENSOR_I2C_WRITE(0x3509, 0x08);
    SENSOR_I2C_WRITE(0x350a, 0x00);
    SENSOR_I2C_WRITE(0x350b, 0x80);
    SENSOR_I2C_WRITE(0x3600, 0x4b);    
    SENSOR_I2C_WRITE(0x3602, 0x3c);
    SENSOR_I2C_WRITE(0x3605, 0x14);
    SENSOR_I2C_WRITE(0x3606, 0x09);
    SENSOR_I2C_WRITE(0x3612, 0x04);
    SENSOR_I2C_WRITE(0x3613, 0x66);
    SENSOR_I2C_WRITE(0x3614, 0x39);
    SENSOR_I2C_WRITE(0x3615, 0x33);
    SENSOR_I2C_WRITE(0x3616, 0x46);
    SENSOR_I2C_WRITE(0x361a, 0x0a);
    SENSOR_I2C_WRITE(0x361c, 0x76);
    SENSOR_I2C_WRITE(0x3620, 0x40);
    SENSOR_I2C_WRITE(0x3640, 0x03);
    SENSOR_I2C_WRITE(0x3641, 0x60);
    SENSOR_I2C_WRITE(0x3642, 0x00);
    SENSOR_I2C_WRITE(0x3643, 0x90);
    SENSOR_I2C_WRITE(0x3660, 0x04);
    SENSOR_I2C_WRITE(0x3665, 0x00);
    SENSOR_I2C_WRITE(0x3666, 0x20);
    SENSOR_I2C_WRITE(0x3667, 0x00);
    SENSOR_I2C_WRITE(0x366a, 0x80);
    SENSOR_I2C_WRITE(0x3680, 0xe0);
    SENSOR_I2C_WRITE(0x3692, 0x80);
    SENSOR_I2C_WRITE(0x3700, 0x42);
    SENSOR_I2C_WRITE(0x3701, 0x14);
    SENSOR_I2C_WRITE(0x3702, 0xe8);
    SENSOR_I2C_WRITE(0x3703, 0x20);
    SENSOR_I2C_WRITE(0x3704, 0x5e);
    SENSOR_I2C_WRITE(0x3705, 0x02);
    SENSOR_I2C_WRITE(0x3708, 0xe3);
    SENSOR_I2C_WRITE(0x3709, 0xc3);
    SENSOR_I2C_WRITE(0x370a, 0x00);
    SENSOR_I2C_WRITE(0x370b, 0x20);
    SENSOR_I2C_WRITE(0x370c, 0x0c);
    SENSOR_I2C_WRITE(0x370d, 0x11);
    SENSOR_I2C_WRITE(0x370e, 0x00);
    SENSOR_I2C_WRITE(0x370f, 0x40);
    SENSOR_I2C_WRITE(0x3710, 0x00);
    SENSOR_I2C_WRITE(0x3715, 0x09);
    SENSOR_I2C_WRITE(0x371a, 0x04);
    SENSOR_I2C_WRITE(0x371b, 0x05);
    SENSOR_I2C_WRITE(0x371c, 0x01);
    SENSOR_I2C_WRITE(0x371e, 0xa1);
    SENSOR_I2C_WRITE(0x371f, 0x18);
    SENSOR_I2C_WRITE(0x3721, 0x00);
    SENSOR_I2C_WRITE(0x3726, 0x00);
    SENSOR_I2C_WRITE(0x372a, 0x01);
    SENSOR_I2C_WRITE(0x3730, 0x10);
    SENSOR_I2C_WRITE(0x3738, 0x22);
    SENSOR_I2C_WRITE(0x3739, 0xe5);
    SENSOR_I2C_WRITE(0x373a, 0x50);
    SENSOR_I2C_WRITE(0x373b, 0x02);
    SENSOR_I2C_WRITE(0x373c, 0x2c);
    SENSOR_I2C_WRITE(0x373f, 0x02);
    SENSOR_I2C_WRITE(0x3740, 0x42);
    SENSOR_I2C_WRITE(0x3741, 0x02);
    SENSOR_I2C_WRITE(0x3743, 0x01);
    SENSOR_I2C_WRITE(0x3744, 0x02);
    SENSOR_I2C_WRITE(0x3747, 0x00);
    SENSOR_I2C_WRITE(0x3754, 0xc0);
    SENSOR_I2C_WRITE(0x3755, 0x07);
    SENSOR_I2C_WRITE(0x3756, 0x1a);
    SENSOR_I2C_WRITE(0x3759, 0x0f);
    SENSOR_I2C_WRITE(0x375c, 0x04);
    SENSOR_I2C_WRITE(0x3767, 0x00);
    SENSOR_I2C_WRITE(0x376b, 0x44);
    SENSOR_I2C_WRITE(0x377b, 0x44);
    SENSOR_I2C_WRITE(0x377c, 0x30);
    SENSOR_I2C_WRITE(0x377e, 0x30);
    SENSOR_I2C_WRITE(0x377f, 0x08);
    SENSOR_I2C_WRITE(0x3781, 0x0c);
    SENSOR_I2C_WRITE(0x3785, 0x1e);
    SENSOR_I2C_WRITE(0x378f, 0xf5);
    SENSOR_I2C_WRITE(0x3791, 0xb0);
    SENSOR_I2C_WRITE(0x379c, 0x0c);
    SENSOR_I2C_WRITE(0x379d, 0x20);
    SENSOR_I2C_WRITE(0x379e, 0x00);
    SENSOR_I2C_WRITE(0x3796, 0x72);
    SENSOR_I2C_WRITE(0x379a, 0x07);
    SENSOR_I2C_WRITE(0x379b, 0xb0);
    SENSOR_I2C_WRITE(0x37c5, 0x00);
    SENSOR_I2C_WRITE(0x37c6, 0x00);
    SENSOR_I2C_WRITE(0x37c7, 0x00);
    SENSOR_I2C_WRITE(0x37c9, 0x00);
    SENSOR_I2C_WRITE(0x37ca, 0x00);
    SENSOR_I2C_WRITE(0x37cb, 0x00);
    SENSOR_I2C_WRITE(0x37cc, 0x00);
    SENSOR_I2C_WRITE(0x37cd, 0x00);
    SENSOR_I2C_WRITE(0x37ce, 0x01);
    SENSOR_I2C_WRITE(0x37cf, 0x00);
    SENSOR_I2C_WRITE(0x37d1, 0x00);
    SENSOR_I2C_WRITE(0x37de, 0x00);
    SENSOR_I2C_WRITE(0x37df, 0x00);
    SENSOR_I2C_WRITE(0x3800, 0x00);
    SENSOR_I2C_WRITE(0x3801, 0x00);
    SENSOR_I2C_WRITE(0x3802, 0x00);
    SENSOR_I2C_WRITE(0x3803, 0x00);
    SENSOR_I2C_WRITE(0x3804, 0x0a);
    SENSOR_I2C_WRITE(0x3805, 0x3f);
    SENSOR_I2C_WRITE(0x3806, 0x07);
    SENSOR_I2C_WRITE(0x3807, 0xa3);
    SENSOR_I2C_WRITE(0x3808, 0x0a);
    SENSOR_I2C_WRITE(0x3809, 0x20);        //output windows size

    SENSOR_I2C_WRITE(0x380a, 0x07);
    SENSOR_I2C_WRITE(0x380b, 0x98);
    SENSOR_I2C_WRITE(0x380c, 0x0c);
    SENSOR_I2C_WRITE(0x380d, 0x98);
    SENSOR_I2C_WRITE(0x380e, 0x07);
    SENSOR_I2C_WRITE(0x380f, 0xc0);
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x10);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x06);
    SENSOR_I2C_WRITE(0x3814, 0x11);
    SENSOR_I2C_WRITE(0x3815, 0x11);

    SENSOR_I2C_WRITE(0x3820, 0x10);  //org
    //SENSOR_I2C_WRITE(0x3820, 0x42);    //mirror
    SENSOR_I2C_WRITE(0x3821, 0x1e);  //org
    //SENSOR_I2C_WRITE(0x3821, 0x00);
    
    SENSOR_I2C_WRITE(0x3823, 0x00);
    SENSOR_I2C_WRITE(0x3824, 0x00);
    SENSOR_I2C_WRITE(0x3825, 0x00);
    SENSOR_I2C_WRITE(0x3826, 0x00);
    SENSOR_I2C_WRITE(0x3827, 0x00);
    SENSOR_I2C_WRITE(0x3829, 0x0b);
    SENSOR_I2C_WRITE(0x382a, 0x04);
    SENSOR_I2C_WRITE(0x382c, 0x34);
    SENSOR_I2C_WRITE(0x382d, 0x00);
    SENSOR_I2C_WRITE(0x3a04, 0x06);
    SENSOR_I2C_WRITE(0x3a05, 0x14);
    SENSOR_I2C_WRITE(0x3a06, 0x00);
    SENSOR_I2C_WRITE(0x3a07, 0xfe);
    SENSOR_I2C_WRITE(0x3b00, 0x00);
    SENSOR_I2C_WRITE(0x3b02, 0x00);
    SENSOR_I2C_WRITE(0x3b03, 0x00);
    SENSOR_I2C_WRITE(0x3b04, 0x00);
    SENSOR_I2C_WRITE(0x3b05, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x18);
    SENSOR_I2C_WRITE(0x4001, 0x04);
    SENSOR_I2C_WRITE(0x4002, 0x45);
    SENSOR_I2C_WRITE(0x4004, 0x04);
    SENSOR_I2C_WRITE(0x4005, 0x18);
    SENSOR_I2C_WRITE(0x4006, 0x20);
    SENSOR_I2C_WRITE(0x4007, 0x98);
    SENSOR_I2C_WRITE(0x4008, 0x24);
    SENSOR_I2C_WRITE(0x4009, 0x10);
    SENSOR_I2C_WRITE(0x400c, 0x00);
    SENSOR_I2C_WRITE(0x400d, 0x00);
    SENSOR_I2C_WRITE(0x404e, 0x37);
    SENSOR_I2C_WRITE(0x404f, 0x8f);
    SENSOR_I2C_WRITE(0x4058, 0x00);
    SENSOR_I2C_WRITE(0x4100, 0x50);
    SENSOR_I2C_WRITE(0x4101, 0x34);
    SENSOR_I2C_WRITE(0x4102, 0x34);
    SENSOR_I2C_WRITE(0x4104, 0xde);
    SENSOR_I2C_WRITE(0x4300, 0xff);
    SENSOR_I2C_WRITE(0x4307, 0x30);
    SENSOR_I2C_WRITE(0x4311, 0x04);
    SENSOR_I2C_WRITE(0x4315, 0x01);
    SENSOR_I2C_WRITE(0x4501, 0x08);
    SENSOR_I2C_WRITE(0x4502, 0x08);
    SENSOR_I2C_WRITE(0x4816, 0x52);
    SENSOR_I2C_WRITE(0x481f, 0x30);
    SENSOR_I2C_WRITE(0x4826, 0x28);
    SENSOR_I2C_WRITE(0x4837, 0x0d);
    SENSOR_I2C_WRITE(0x4a00, 0xaa);
    SENSOR_I2C_WRITE(0x4a02, 0x00);
    SENSOR_I2C_WRITE(0x4a03, 0x01);
    SENSOR_I2C_WRITE(0x4a05, 0x40);
    SENSOR_I2C_WRITE(0x4a0a, 0x88);
    SENSOR_I2C_WRITE(0x5000, 0x06);
    SENSOR_I2C_WRITE(0x5001, 0x01);
    SENSOR_I2C_WRITE(0x5002, 0x00);
    SENSOR_I2C_WRITE(0x5003, 0x20);
    SENSOR_I2C_WRITE(0x5013, 0x00);
    SENSOR_I2C_WRITE(0x501f, 0x00);
    SENSOR_I2C_WRITE(0x5043, 0x48);
    SENSOR_I2C_WRITE(0x5780, 0x1c);
    SENSOR_I2C_WRITE(0x5786, 0x20);
    SENSOR_I2C_WRITE(0x5788, 0x18);
    SENSOR_I2C_WRITE(0x578a, 0x04);
    SENSOR_I2C_WRITE(0x578b, 0x02);
    SENSOR_I2C_WRITE(0x578c, 0x02);
    SENSOR_I2C_WRITE(0x578e, 0x06);
    SENSOR_I2C_WRITE(0x578f, 0x02);
    SENSOR_I2C_WRITE(0x5790, 0x02);
    SENSOR_I2C_WRITE(0x5791, 0xff);
    SENSOR_I2C_WRITE(0x5e00, 0x00);
    SENSOR_I2C_WRITE(0x5e10, 0x0c);
    SENSOR_I2C_WRITE(0x0100, 0x01);

    SENSOR_DELAY_MS(10000);

    SENSOR_I2C_WRITE(0x5800, 0x22);
    SENSOR_I2C_WRITE(0x5801, 0x11);
    SENSOR_I2C_WRITE(0x5802, 0x0d);
    SENSOR_I2C_WRITE(0x5803, 0x0d);
    SENSOR_I2C_WRITE(0x5804, 0x12);
    SENSOR_I2C_WRITE(0x5805, 0x26);
    SENSOR_I2C_WRITE(0x5806, 0x09);
    SENSOR_I2C_WRITE(0x5807, 0x07);
    SENSOR_I2C_WRITE(0x5808, 0x05);
    SENSOR_I2C_WRITE(0x5809, 0x05);
    SENSOR_I2C_WRITE(0x580a, 0x07);
    SENSOR_I2C_WRITE(0x580b, 0x0a);
    SENSOR_I2C_WRITE(0x580c, 0x07);
    SENSOR_I2C_WRITE(0x580d, 0x02);
    SENSOR_I2C_WRITE(0x580e, 0x00);
    SENSOR_I2C_WRITE(0x580f, 0x00);
    SENSOR_I2C_WRITE(0x5810, 0x03);
    SENSOR_I2C_WRITE(0x5811, 0x07);
    SENSOR_I2C_WRITE(0x5812, 0x06);
    SENSOR_I2C_WRITE(0x5813, 0x02);
    SENSOR_I2C_WRITE(0x5814, 0x00);
    SENSOR_I2C_WRITE(0x5815, 0x00);
    SENSOR_I2C_WRITE(0x5816, 0x03);
    SENSOR_I2C_WRITE(0x5817, 0x07);
    SENSOR_I2C_WRITE(0x5818, 0x09);
    SENSOR_I2C_WRITE(0x5819, 0x05);
    SENSOR_I2C_WRITE(0x581a, 0x04);
    SENSOR_I2C_WRITE(0x581b, 0x04);
    SENSOR_I2C_WRITE(0x581c, 0x07);
    SENSOR_I2C_WRITE(0x581d, 0x09);
    SENSOR_I2C_WRITE(0x581e, 0x1d);
    SENSOR_I2C_WRITE(0x581f, 0x0e);
    SENSOR_I2C_WRITE(0x5820, 0x0b);
    SENSOR_I2C_WRITE(0x5821, 0x0b);
    SENSOR_I2C_WRITE(0x5822, 0x0f);
    SENSOR_I2C_WRITE(0x5823, 0x1e);
    SENSOR_I2C_WRITE(0x5824, 0x59);
    SENSOR_I2C_WRITE(0x5825, 0x46);
    SENSOR_I2C_WRITE(0x5826, 0x37);
    SENSOR_I2C_WRITE(0x5827, 0x36);
    SENSOR_I2C_WRITE(0x5828, 0x45);
    SENSOR_I2C_WRITE(0x5829, 0x39);
    SENSOR_I2C_WRITE(0x582a, 0x46);
    SENSOR_I2C_WRITE(0x582b, 0x44);
    SENSOR_I2C_WRITE(0x582c, 0x45);
    SENSOR_I2C_WRITE(0x582d, 0x28);
    SENSOR_I2C_WRITE(0x582e, 0x38);
    SENSOR_I2C_WRITE(0x582f, 0x52);
    SENSOR_I2C_WRITE(0x5830, 0x60);
    SENSOR_I2C_WRITE(0x5831, 0x51);
    SENSOR_I2C_WRITE(0x5832, 0x26);
    SENSOR_I2C_WRITE(0x5833, 0x38);
    SENSOR_I2C_WRITE(0x5834, 0x43);
    SENSOR_I2C_WRITE(0x5835, 0x42);
    SENSOR_I2C_WRITE(0x5836, 0x34);
    SENSOR_I2C_WRITE(0x5837, 0x18);
    SENSOR_I2C_WRITE(0x5838, 0x05);
    SENSOR_I2C_WRITE(0x5839, 0x27);
    SENSOR_I2C_WRITE(0x583a, 0x27);
    SENSOR_I2C_WRITE(0x583b, 0x27);
    SENSOR_I2C_WRITE(0x583c, 0x0a);
    SENSOR_I2C_WRITE(0x583d, 0xbf);
    SENSOR_I2C_WRITE(0x5842, 0x01);
    SENSOR_I2C_WRITE(0x5843, 0x2b);
    SENSOR_I2C_WRITE(0x5844, 0x01);
    SENSOR_I2C_WRITE(0x5845, 0x92);
    SENSOR_I2C_WRITE(0x5846, 0x01);
    SENSOR_I2C_WRITE(0x5847, 0x8f);
    SENSOR_I2C_WRITE(0x5848, 0x01);
    SENSOR_I2C_WRITE(0x5849, 0x0c);
	
    SENSOR_I2C_WRITE(0x4810, 0x00);
    SENSOR_I2C_WRITE(0x4811, 0x01);  
    printf("-------Omnivision ov5658 Sensor 5M30fps Initial OK!-------\n");

	bSensorInit = HI_TRUE;
}


HI_S32 ov5658_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ov5658_reg_init;
//   pstSensorExpFunc->pfn_cmos_sensor_exit = ov5658_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ov5658_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = ov5658_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ov5658_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = ov5658_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ov5658_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ov5658_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = ov5658_get_sns_regs_info;

    return 0;
}


/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int ov5658_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ov5658_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, OV5658_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    ov5658_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, OV5658_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    ov5658_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, OV5658_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int ov5658_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, OV5658_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, OV5658_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, OV5658_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


HI_S32 OV5658_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)
{

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ov5658_reg_init();

	ov5658_register_callback();

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
	printf("omnivision ov5658 sensor 5M 30fps init success!\n");
	
	return 0;

}

int OV5658_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_OV5658_WIDTH;
        *ret_height = SENSOR_OV5658_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_OV5658_probe()
{
    uint16_t ret_data1 = 0;
    uint16_t ret_data2 = 0;
    SENSOR_I2C_READ(0x300A, &ret_data1);
    SENSOR_I2C_READ(0x300B, &ret_data2);
    if(ret_data1 == OV5658_CHECK_DATA_MSB	&& ret_data2 == OV5658_CHECK_DATA_LSB)
    {
        sdk_sys->write_reg(0x200f0050, 0x2); //set sensor clock
        sdk_sys->write_reg(0x200f0054, 0x2); //set sensor clock
        sdk_sys->write_reg(0x2003002c, 0xE0007); //set sensor clock  24MHz
        sdk_sys->write_reg(0x20030104, 0x0);
        return true;
    }
    return false;
}

int OV5658_get_sensor_name(char *sensor_name)
{
	if(sensor_name != NULL)
	{
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

#endif 







