#include "sdk/sdk_sys.h"
#include "sdk/sdk_debug.h"
#include "hi3516a.h"
#include "hi_isp_i2c.h"
#include "hi3516a_isp_sensor.h"

static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x20, 2, 2))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) :_i2c_write((_add), (_data), 0x20, 2, 2))

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#define SENSOR_AR0330_WIDTH 2048
#define SENSOR_AR0330_HEIGHT 1520
#define AR0330_CHECK_DATA (0x2604)
#define SENSOR_NAME "ar0330"

#if !defined(__AR0330_CMOS_H_)
#define __AR0330_CMOS_H_

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

#define AR0330_ID 330

/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/
#define QUICK_CONFIG_MODE
#ifdef QUICK_CONFIG_MODE
#else

static AE_SENSOR_DEFAULT_S  g_AeDft[];
static AWB_SENSOR_DEFAULT_S g_AwbDft[];
static ISP_CMOS_DEFAULT_S   g_IspDft[];
extern HI_S32 AR0330_LoadINIPara(const HI_CHAR *pcName);

#endif
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr = 0x20;
static unsigned int sensor_addr_byte = 2;
static unsigned int sensor_data_byte = 2;

#define ANALOG_GAIN    (0x3060)
#define DIGITAL_GAIN   (0x305E)
#define FRAME_LINES    (0x300A)
#define EXPOSURE_TIME  (0x3012)
#define LINE_LEN_PCK   (0x300C)

#define SENSOR_2M_1080p_MODE  (1) 
#define SENSOR_3M_1536p_MODE  (2)
#define SENSOR_3M_1296p_MODE  (3)


#define INCREASE_LINES (1) /* make real fps less than stand fps because NVR require*/
#define FRAME_LINES_2M_1080p  (1308+INCREASE_LINES)
#define FRAME_LINES_3M_1296p  (1308+INCREASE_LINES)
#define FRAME_LINES_3M_1536p  (1709+INCREASE_LINES)
#define LINE_LENGTH_PCK  	  (1248) 


static HI_U8 gu8SensorImageMode = SENSOR_3M_1536p_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = FRAME_LINES_3M_1536p;
static HI_U32 gu32FullLines = FRAME_LINES_3M_1536p; 

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE; 


static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};


#define PATHLEN_MAX 256
#define CMOS_CFG_INI "ar0330_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/ar0330_cfg.ini";

/* AE default parameter and function */

#ifdef INIFILE_CONFIG_MODE
static HI_S32 ar0330_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
	   pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0078125;
   
	   pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
	   pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0078125; 
	   
	   switch(genSensorMode)
	   {
		   default:
		   case WDR_MODE_NONE:	 /*linear mode*/
			   pstAeSnsDft->au8HistThresh[0] = 0xd;
			   pstAeSnsDft->au8HistThresh[1] = 0x28;
			   pstAeSnsDft->au8HistThresh[2] = 0x60;
			   pstAeSnsDft->au8HistThresh[3] = 0x80;
			   
			   pstAeSnsDft->u8AeCompensation = g_AeDft[0].u8AeCompensation;
			   
			   pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
			   pstAeSnsDft->u32MinIntTime = 2;
			   pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[0].u32MaxIntTimeTarget;
			   pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[0].u32MinIntTimeTarget;
			   
			   pstAeSnsDft->u32MaxAgain = 8192;  
			   pstAeSnsDft->u32MinAgain = 1024;
			   pstAeSnsDft->u32MaxAgainTarget = g_AeDft[0].u32MaxAgainTarget;
			   pstAeSnsDft->u32MinAgainTarget = g_AeDft[0].u32MinAgainTarget;
			   
			   pstAeSnsDft->u32MaxDgain = 2047;  
			   pstAeSnsDft->u32MinDgain = 128;
			   pstAeSnsDft->u32MaxDgainTarget = g_AeDft[0].u32MaxDgainTarget;
			   pstAeSnsDft->u32MinDgainTarget = g_AeDft[0].u32MinDgainTarget;
			 
			   pstAeSnsDft->u32ISPDgainShift = g_AeDft[0].u32ISPDgainShift;
			   pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[0].u32MinISPDgainTarget;
			   pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[0].u32MaxISPDgainTarget;    
		   break;
		   
		   case WDR_MODE_2To1_FRAME:
		   case WDR_MODE_2To1_FRAME_FULL_RATE: /*linear mode for ISP frame switching WDR*/
			   pstAeSnsDft->au8HistThresh[0] = 0xc;
			   pstAeSnsDft->au8HistThresh[1] = 0x18;
			   pstAeSnsDft->au8HistThresh[2] = 0x60;
			   pstAeSnsDft->au8HistThresh[3] = 0x80;
			   
			   pstAeSnsDft->u8AeCompensation = g_AeDft[1].u8AeCompensation;
			   
			   pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
			   pstAeSnsDft->u32MinIntTime = 2;
			   pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[1].u32MaxIntTimeTarget;
			   pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[1].u32MinIntTimeTarget;
			   
			   pstAeSnsDft->u32MaxAgain = 8192;  
			   pstAeSnsDft->u32MinAgain = 1024;
			   pstAeSnsDft->u32MaxAgainTarget = g_AeDft[1].u32MaxAgainTarget;
			   pstAeSnsDft->u32MinAgainTarget = g_AeDft[1].u32MinAgainTarget;
			   
			   pstAeSnsDft->u32MaxDgain = 2047;  
			   pstAeSnsDft->u32MinDgain = 128;
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

static HI_S32 ar0330_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
	   pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0078125;
	
	   pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
	   pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0078125;	
	
	   pstAeSnsDft->u32ISPDgainShift = 8;
	   pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	   pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;	  
	
	   switch(genSensorMode)
	   {
		   default:
		   case WDR_MODE_NONE:	 /*linear mode*/
			   pstAeSnsDft->au8HistThresh[0] = 0xd;
			   pstAeSnsDft->au8HistThresh[1] = 0x28;
			   pstAeSnsDft->au8HistThresh[2] = 0x60;
			   pstAeSnsDft->au8HistThresh[3] = 0x80;
			   
			   pstAeSnsDft->u8AeCompensation = 0x38;
			   
			   pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
			   pstAeSnsDft->u32MinIntTime = 2;	  
			   pstAeSnsDft->u32MaxIntTimeTarget = 65535;
			   pstAeSnsDft->u32MinIntTimeTarget = 2;
	
			   pstAeSnsDft->u32MaxAgain = 8192;  /* (8mult)/0.0078125 = 1024 */
			   pstAeSnsDft->u32MinAgain = 1024;
			   pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
			   pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
			   
			   pstAeSnsDft->u32MaxDgain = 2047; /* (15.875mult)16 / 0.0078125 = 2047 */
			   pstAeSnsDft->u32MinDgain = 128;
			   pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
			   pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;			 
		   break;
		   
		   case WDR_MODE_2To1_FRAME:
		   case WDR_MODE_2To1_FRAME_FULL_RATE: /*linear mode for ISP frame switching WDR*/
			   pstAeSnsDft->au8HistThresh[0] = 0xc;
			   pstAeSnsDft->au8HistThresh[1] = 0x18;
			   pstAeSnsDft->au8HistThresh[2] = 0x60;
			   pstAeSnsDft->au8HistThresh[3] = 0x80;
			   
			   pstAeSnsDft->u8AeCompensation = 0x38;
			   
			   pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
			   pstAeSnsDft->u32MinIntTime = 2;	  
			   pstAeSnsDft->u32MaxIntTimeTarget = 65535;
			   pstAeSnsDft->u32MinIntTimeTarget = 2;
	
			   pstAeSnsDft->u32MaxAgain = 8192;  /* (8mult)/0.0078125 = 1024 */
			   pstAeSnsDft->u32MinAgain = 1024;
			   pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
			   pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
			   
			   pstAeSnsDft->u32MaxDgain = 2047; /* (15.875mult)16 / 0.0078125 = 2047 */
			   pstAeSnsDft->u32MinDgain = 128;
			   pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
			   pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;			 
		   break;
	   }
	   return 0;
    
}

#endif

/* the function of sensor set fps */
static HI_VOID ar0330_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{    
    if (SENSOR_2M_1080p_MODE == gu8SensorImageMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 0.5))
        {
            gu32FullLinesStd = (FRAME_LINES_2M_1080p * 30) / f32Fps; 
        }
        else
        {
            printf("Not support Fps: %f\n", f32Fps);
            return;
        }        
    }
    else if (SENSOR_3M_1536p_MODE == gu8SensorImageMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 0.5))
        {
            #ifndef HI_FPGA
            gu32FullLinesStd = (FRAME_LINES_3M_1536p * 30) / f32Fps;
            #else
            gu32FullLinesStd = (FRAME_LINES_3M_1536p * 25) / f32Fps;
            #endif
        }
        else
        {
            printf("Not support Fps: %f\n", f32Fps);
            return;
        }
    }
    else if (SENSOR_3M_1296p_MODE == gu8SensorImageMode)
    {
        if ((f32Fps <= 30) && (f32Fps >= 0.5))
        {
            gu32FullLinesStd = (FRAME_LINES_3M_1296p * 30) / f32Fps;     
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
    
    gu32FullLinesStd = gu32FullLinesStd > 0xFFFF ? 0xFFFF : gu32FullLinesStd;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[3].u32Data = gu32FullLinesStd;
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[4].u32Data = gu32FullLinesStd;
    }
        
    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID ar0330_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
     u32FullLines = (u32FullLines > 0xFFFF) ? 0xFFFF : u32FullLines;
    gu32FullLines = u32FullLines;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[3].u32Data = gu32FullLines;
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[4].u32Data = gu32FullLines;
    }

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 2;
    
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ar0330_inttime_update(HI_U32 u32IntTime)
{
    static HI_BOOL bFirst = HI_TRUE; 
    
    if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
    {
        if (bFirst) /* short exposure */
        {
            g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;
            bFirst = HI_FALSE;
        }
        else        /* long exposure */
        {
            g_stSnsRegsInfo.astI2cData[3].u32Data = u32IntTime;
            bFirst = HI_TRUE;            
        }
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;
        bFirst = HI_TRUE;
    }
    
    return;
}

static  HI_U32  again_table[29] = 
{ 
    1024,1054,1096,1126,1167,1218,1259,1311,1362,1423,1484,1556,1638,1720,1822,
    1925,2048,2181,2345,2519,2734,2979,3276,3645,4096,4679,5458,6553,8192
};

static HI_VOID ar0330_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;
   
    if (*pu32AgainLin >= again_table[28])
    {
         *pu32AgainLin = again_table[28];
         *pu32AgainDb = 28;
         return ;
    }
    
    for(i = 1; i < 29; i++)
    {
        if(*pu32AgainLin < again_table[i])
        {
            *pu32AgainLin = again_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }
          
    return;
}

static HI_VOID ar0330_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
     HI_U32 u32AGainReg = 0;
       
    if(u32Again < 16)
    {
        u32AGainReg = 0x00 + u32Again;
    }
    else if(u32Again < 24)
    {
        u32AGainReg = 0x10 + (u32Again - 16) * 2;
    }
    else if(u32Again < 28)
    {
        u32AGainReg = 0x20 + (u32Again - 24) * 4;
    }
    else if(u32Again == 28)
    {
        u32AGainReg = 0x30;
    }

    g_stSnsRegsInfo.astI2cData[1].u32Data = u32AGainReg;
    g_stSnsRegsInfo.astI2cData[2].u32Data = u32Dgain;

    return;
}

/* Only used in WDR_MODE_2To1_LINE and WDR_MODE_2To1_FRAME mode */
static HI_VOID ar0330_get_inttime_max(HI_U32 u32Ratio, HI_U32 *pu32IntTimeMax)
{
    if(HI_NULL == pu32IntTimeMax)
    {
        printf("null pointer when get ae sensor IntTimeMax value!\n");
        return;
    }

    if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
    {
        *pu32IntTimeMax = (gu32FullLinesStd - 2) * 0x40 / DIV_0_TO_1(u32Ratio);
    }

    return;
}


HI_S32 ar0330_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ar0330_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ar0330_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ar0330_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ar0330_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ar0330_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ar0330_again_calc_table;
	 pstExpFuncs->pfn_cmos_get_inttime_max   = ar0330_get_inttime_max; 
 

    return 0;
}


/* AWB default parameter and function */
#ifdef INIFILE_CONFIG_MODE

static HI_S32 ar0330_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
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
        
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            pstAwbSnsDft->u16WbRefTemp = g_AwbDft[1].u16WbRefTemp;
            for(i= 0; i < 4; i++)
            {
                pstAwbSnsDft->au16GainOffset[i] = g_AwbDft[1].au16GainOffset[i];
            }
           
            for(i= 0; i < 6; i++)
            {
                pstAwbSnsDft->as32WbPara[i] = g_AwbDft[1].as32WbPara[i];
            }

            memcpy(&pstAwbSnsDft->stCcm, &g_AwbDft[1].stCcm, sizeof(AWB_CCM_S));            
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_AwbDft[1].stAgcTbl, sizeof(AWB_AGC_TABLE_S));
        break;
    }
    return 0;
}

#else

static AWB_CCM_S g_stAwbCcm =
{

	4915,
    {
       	0x1a8,	0x807c,	0x802c,
		0x8037,	0x144,	0x800d,
		0x800d,	0x807b,	0x188
    },
        
    3670,
    {
		0x18c,	0x805f,	0x802d,
		0x805c,	0x15e,	0x8002,
		0x801a,	0x8085,	0x19f

    },

    2460,
    {
		0x1b5,	0x8075, 0x8040,
		0x806d, 0x148,	0x24,
		0x806a, 0x8108, 0x272
    }
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */ 
	{0x8E,0x8A,0x86,0x82,0x78,0x6E,0x64,0x5F,0x58,0x4C,0x41,0x41,0x41,0x41,0x41,0x41}
};

static AWB_AGC_TABLE_S g_stAwbAgcTableFSWDR =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x80,0x80,0x7e,0x72,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
};

static HI_S32 ar0330_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
        if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4912;

	pstAwbSnsDft->au16GainOffset[0] = 0x169;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1c0;

    pstAwbSnsDft->as32WbPara[0] = 113;
    pstAwbSnsDft->as32WbPara[1] = -56;
    pstAwbSnsDft->as32WbPara[2] = -198;
    pstAwbSnsDft->as32WbPara[3] = 215732;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -169947;   //


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


HI_S32 ar0330_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ar0330_get_awb_default;

    return 0;
}


/* ISP default parameter and function */
#ifdef INIFILE_CONFIG_MODE

HI_U32 ar0330_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_AR0330_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_AR0330_HEIGHT;

    return 0;
}

#else

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
    
    /* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */

    /* sharpen_alt_d */
	{0x6e,0x64,0x5f,0x55,0x4b,0x41,0x28,0x23,0x19,0x19,0x19,0x19,0x19,0x19,0x19,0x19},
        
    /* sharpen_alt_ud */ 
	{0x78,0x6e,0x64,0x5a,0x4b,0x3c,0x32,0x28,0x1e,0x14,0x14,0x14,0x14,0x14,0x14,0x14},

    /* snr_thresh Max=0x54 */
	{0x5,0x8,0xf,0x1e,0x2d,0x3c,0x4b,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55}, 
	 	
    /* demosaic_lum_thresh */
    {0x50,0x50,0x4e,0x49,0x45,0x45,0x40,0x3a,0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x00,0x0a,0x12,0x1a,0x20,0x28,0x30,0x32,0x34,0x38,0x38,0x38,0x38,0x38,0x38,0x38},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37,0x37,0x35,0x35,0x35,0x35,0x35,0x35,0x35},

    /* rgbsharp_strength */
    {0x86,0x86,0x84,0x7a,0x60,0x50,0x40,0x30,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x10}
};


static ISP_CMOS_AGC_TABLE_S g_stIspAgcTableFSWDR =
{
    /* bvalid */
    1,
    
    /* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */

    /* sharpen_alt_d */
    {0x40,0x40,0x3e,0x3a,0x35,0x32,0x2e,0x28,0x24,0x20,0x1e,0x18,0x16,0x12,0x12,0x12},
        
    /* sharpen_alt_ud */
    {0x6a,0x6a,0x68,0x60,0x58,0x50,0x40,0x30,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x10},    
        
    /* snr_thresh Max=0x54 */
    {0x08,0x0a,0x0c,0x10,0x16,0x1b,0x22,0x28,0x2a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a},
        
    /* demosaic_lum_thresh */
    {0x50,0x50,0x4e,0x49,0x45,0x45,0x40,0x3a,0x30,0x20,0x20,0x20,0x20,0x20,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x00,0x0a,0x12,0x1a,0x20,0x28,0x30,0x32,0x34,0x38,0x38,0x38,0x38,0x38,0x38,0x38},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37,0x37,0x35,0x35,0x35,0x35,0x35,0x35,0x35},

    /* rgbsharp_strength */
    {0x86,0x86,0x84,0x7a,0x60,0x50,0x40,0x30,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x10}
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0xb,0x12,0x17,0x1a,0x1c,0x1e,0x20,0x21,0x23,0x24,0x25,
    0x26,0x27,0x28,0x28,0x29,0x2a,0x2b,0x2b,0x2c,0x2c,0x2d,0x2d,0x2e,0x2e,0x2f,0x2f,
    0x30,0x30,0x30,0x31,0x31,0x32,0x32,0x32,0x33,0x33,0x33,0x33,0x34,0x34,0x34,0x35,
    0x35,0x35,0x35,0x36,0x36,0x36,0x36,0x37,0x37,0x37,0x37,0x37,0x38,0x38,0x38,0x38,
    0x38,0x39,0x39,0x39,0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,
    0x3b,0x3b,0x3b,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,
    0x3d,0x3d,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3f,0x3f,0x3f,0x3f,0x3f,
    0x3f,0x3f,0x3f,0x3f,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x41,0x41},
    
    /* demosaic_weight_lut */
    {0x0, 0xb,0x12,0x17,0x1a,0x1c,0x1e,0x20,0x21,0x23,0x24,0x25,0x26,0x27,0x28,0x28,
    0x29,0x2a,0x2b,0x2b,0x2c,0x2c,0x2d,0x2d,0x2e,0x2e,0x2f,0x2f,0x30,0x30,0x30,0x31,
    0x31,0x32,0x32,0x32,0x33,0x33,0x33,0x33,0x34,0x34,0x34,0x35,0x35,0x35,0x35,0x36,
    0x36,0x36,0x36,0x37,0x37,0x37,0x37,0x37,0x38,0x38,0x38,0x38,0x38,0x39,0x39,0x39,
    0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,0x3b,0x3b,0x3b,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x3e,0x3e,
    0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,
    0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x41,0x41}
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTableFSWDR =
{
    /* bvalid */
    1,
   
    /* nosie_profile_weight_lut */

    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 47
    },
    
    /* demosaic_weight_lut */
    {
        3, 12, 17, 21, 23, 26, 27, 29, 30, 31, 33, 34, 35, 35, 36, 37, 38, 38, 39, 39, 40, 41, 41, 42, 42, 42, 43, 43, 44, 44, 44, 45, 45, 46, 46, 46, 46, 47, 47, 47, 48, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60
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
	// 0xa8,  //test
	0xb4,

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
    0xdc,

    /*aa_slope*/
    0xc8,

    /*va_slope*/
    0xb9,

    /*uu_slope*/
    0xa8,

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
    
#if 1 /* Normal mode */   
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
    3994, 4002, 4006, 4010, 4018, 4022, 4032, 4038, 4046, 4050, 4056, 4062, 4072, 4076, 4084, 4090, 4095}
#else  /* Infrared or Spotlight mode */
    {  0, 120, 220, 320, 416, 512, 592, 664, 736, 808, 880, 944, 1004, 1062, 1124, 1174,
    1226, 1276, 1328, 1380, 1432, 1472, 1516, 1556, 1596, 1636, 1680, 1720, 1756, 1792,
    1828, 1864, 1896, 1932, 1968, 2004, 2032, 2056, 2082, 2110, 2138, 2162, 2190, 2218,
    2242, 2270, 2294, 2314, 2338, 2358, 2382, 2402, 2426, 2446, 2470, 2490, 2514, 2534,
    2550, 2570, 2586, 2606, 2622, 2638, 2658, 2674, 2694, 2710, 2726, 2746, 2762, 2782,
    2798, 2814, 2826, 2842, 2854, 2870, 2882, 2898, 2910, 2924, 2936, 2952, 2964, 2980,
    2992, 3008, 3020, 3036, 3048, 3064, 3076, 3088, 3096, 3108, 3120, 3128, 3140, 3152,
    3160, 3172, 3184, 3192, 3204, 3216, 3224, 3236, 3248, 3256, 3268, 3280, 3288, 3300,
    3312, 3320, 3332, 3340, 3348, 3360, 3368, 3374, 3382, 3390, 3402, 3410, 3418, 3426,
    3434, 3446, 3454, 3462, 3470, 3478, 3486, 3498, 3506, 3514, 3522, 3530, 3542, 3550,
    3558, 3566, 3574, 3578, 3586, 3594, 3602, 3606, 3614, 3622, 3630, 3634, 3642, 3650,
    3658, 3662, 3670, 3678, 3686, 3690, 3698, 3706, 3710, 3718, 3722, 3726, 3734, 3738,
    3742, 3750, 3754, 3760, 3764, 3768, 3776, 3780, 3784, 3792, 3796, 3800, 3804, 3812,
    3816, 3820, 3824, 3832, 3836, 3840, 3844, 3848, 3856, 3860, 3864, 3868, 3872, 3876,
    3880, 3884, 3892, 3896, 3900, 3904, 3908, 3912, 3916, 3920, 3924, 3928, 3932, 3936,
    3940, 3944, 3948, 3952, 3956, 3960, 3964, 3968, 3972, 3972, 3976, 3980, 3984, 3988,
    3992, 3996, 4000, 4004, 4008, 4012, 4016, 4020, 4024, 4028, 4032, 4032, 4036, 4040,
    4044, 4048, 4052, 4056, 4056, 4060, 4064, 4068, 4072, 4072, 4076, 4080, 4084, 4086,
    4088, 4092, 4095} 
#endif
};

static ISP_CMOS_GAMMA_S g_stIspGammaFSWDR =
{
    /* bvalid */
    1,
    
#if 0    
    {
        0,   1,   2,   4,   8,  12,  17,  23,  30,  38,  47,  57,  68,  79,  92, 105, 120, 133, 147, 161, 176, 192, 209, 226, 243, 260, 278, 296, 315, 333, 351, 370, 390, 410, 431, 453, 474, 494, 515, 536, 558, 580, 602, 623, 644, 665, 686, 708, 730, 751, 773, 795, 818, 840, 862, 884, 907, 929, 951, 974, 998,1024,1051,1073,1096,1117,1139,1159,1181,1202,1223,1243,1261,1275,1293,1313,1332,1351,1371,1389,1408,1427,1446,1464,1482,1499,1516,1533,1549,1567,1583,1600,1616,1633,1650,1667,1683,1700,1716,1732,1749,1766,1782,1798,1815,1831,1847,1863,1880,1896,1912,1928,1945,1961,1977,1993,2009,2025,2041,2057,2073,2089,2104,2121,2137,2153,2168,2184,2200,2216,2231,2248,2263,2278,2294,2310,2326,2341,2357,2373,2388,2403,2419,2434,2450,2466,2481,2496,2512,2527,2543,2558,2573,2589,2604,2619,2635,2650,2665,2680,2696,2711,2726,2741,2757,2771,2787,2801,2817,2832,2847,2862,2877,2892,2907,2922,2937,2952,2967,2982,2997,3012,3027,3041,3057,3071,3086,3101,3116,3130,3145,3160,3175,3190,3204,3219,3234,3248,3263,3278,3293,3307,3322,3337,3351,3365,3380,3394,3409,3424,3438,3453,3468,3482,3497,3511,3525,3540,3554,3569,3584,3598,3612,3626,3641,3655,3670,3684,3699,3713,3727,3742,3756,3770,3784,3799,3813,3827,3841,3856,3870,3884,3898,3912,3927,3941,3955,3969,3983,3997,4011,4026,4039,4054,4068,4082,4095
    }
#else  /*higher  contrast*/
    {
        0,1,2,4,8,12,17,23,30,38,47,57,68,79,92,105,120,133,147,161,176,192,209,226,243,260,278,296,317,340,365,390,416,440,466,491,517,538,561,584,607,631,656,680,705,730,756,784,812,835,858,882,908,934,958,982,1008,1036,1064,1092,1119,1143,1167,1192,1218,1243,1269,1296,1323,1351,1379,1408,1434,1457,1481,1507,1531,1554,1579,1603,1628,1656,1683,1708,1732,1756,1780,1804,1829,1854,1877,1901,1926,1952,1979,2003,2024,2042,2062,2084,2106,2128,2147,2168,2191,2214,2233,2256,2278,2296,2314,2335,2352,2373,2391,2412,2431,2451,2472,2492,2513,2531,2547,2566,2581,2601,2616,2632,2652,2668,2688,2705,2721,2742,2759,2779,2796,2812,2826,2842,2857,2872,2888,2903,2920,2934,2951,2967,2983,3000,3015,3033,3048,3065,3080,3091,3105,3118,3130,3145,3156,3171,3184,3197,3213,3224,3240,3252,3267,3281,3295,3310,3323,3335,3347,3361,3372,3383,3397,3409,3421,3432,3447,3459,3470,3482,3497,3509,3521,3534,3548,3560,3572,3580,3592,3602,3613,3625,3633,3646,3657,3667,3679,3688,3701,3709,3719,3727,3736,3745,3754,3764,3773,3781,3791,3798,3806,3816,3823,3833,3840,3847,3858,3865,3872,3879,3888,3897,3904,3911,3919,3926,3933,3940,3948,3955,3962,3970,3973,3981,3988,3996,4003,4011,4018,4026,4032,4037,4045,4053,4057,4064,4072,4076,4084,4088,4095        
        //0,1,2,4,8,12,17,23,30,38,47,57,68,79,92,105,120,133,147,161,176,192,209,226,243,260,278,296,317,340,365,390,416,440,466,491,517,538,561,584,607,631,656,680,705,730,756,784,812,835,858,882,908,934,958,982,1008,1036,1064,1092,1119,1143,1167,1192,1218,1243,1269,1294,1320,1346,1372,1398,1424,1450,1476,1502,1528,1554,1580,1607,1633,1658,1684,1710,1735,1761,1786,1811,1836,1860,1884,1908,1932,1956,1979,2002,2024,2046,2068,2090,2112,2133,2154,2175,2196,2217,2237,2258,2278,2298,2318,2337,2357,2376,2395,2414,2433,2451,2469,2488,2505,2523,2541,2558,2575,2592,2609,2626,2642,2658,2674,2690,2705,2720,2735,2750,2765,2779,2793,2807,2821,2835,2848,2861,2874,2887,2900,2913,2925,2937,2950,2962,2974,2986,2998,3009,3021,3033,3044,3056,3067,3078,3088,3099,3109,3119,3129,3139,3148,3158,3168,3177,3187,3197,3207,3217,3227,3238,3248,3259,3270,3281,3292,3303,3313,3324,3335,3346,3357,3368,3379,3389,3400,3410,3421,3431,3441,3451,3461,3471,3481,3491,3501,3511,3521,3531,3541,3552,3562,3572,3583,3593,3604,3615,3625,3636,3646,3657,3668,3679,3689,3700,3711,3721,3732,3743,3753,3764,3774,3784,3795,3805,3816,3826,3837,3847,3858,3869,3880,3891,3902,3913,3925,3937,3949,3961,3973,3985,3997,4009,4022,4034,4046,4058,4071,4083,4095
    }
#endif
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

HI_U32 ar0330_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            pstDef->stDrc.bEnable               = HI_FALSE;
            pstDef->stDrc.u32BlackLevel         = 0x00;
            pstDef->stDrc.u32WhiteLevel         = 0x4FF; 
            pstDef->stDrc.u32SlopeMax           = 0x30;
            pstDef->stDrc.u32SlopeMin           = 0x00;
            pstDef->stDrc.u32VarianceSpace      = 0x04;
            pstDef->stDrc.u32VarianceIntensity  = 0x01;
            pstDef->stDrc.u32Asymmetry          = 0x14;    
			pstDef->stDrc.u32BrightEnhance      = 0xC8;

            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
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
            pstDef->stDrc.u32Asymmetry          = 0x14;    
			pstDef->stDrc.u32BrightEnhance      = 0xC8;

            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTableFSWDR, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTableFSWDR, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaicFSWDR, sizeof(ISP_CMOS_DEMOSAIC_S));        
            memcpy(&pstDef->stGamma, &g_stIspGammaFSWDR, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stGammafe, &g_stGammafeFSWDR, sizeof(ISP_CMOS_GAMMAFE_S));
            memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
        break;
    }
    pstDef->stSensorMaxResolution.u32MaxWidth  = 2304;
    pstDef->stSensorMaxResolution.u32MaxHeight = 1536;

    return 0;}

#endif

HI_U32 ar0330_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

	pstBlackLevel->au16BlackLevel[0] = 160; 
	pstBlackLevel->au16BlackLevel[1] = 168; 
	pstBlackLevel->au16BlackLevel[2] = 168; 
	pstBlackLevel->au16BlackLevel[3] = 168; 

    return 0;    
}

HI_VOID ar0330_set_pixel_detect(HI_BOOL bEnable)
{
    
     
    HI_U32 u32FullLines_5Fps = FRAME_LINES_3M_1536p;
    HI_U32 u32MaxExpTime_5Fps = FRAME_LINES_3M_1536p - 2;
    
    if (SENSOR_2M_1080p_MODE == gu8SensorImageMode)
    {
        u32FullLines_5Fps = FRAME_LINES_2M_1080p * 30 / 5;
    } 
    else if(SENSOR_3M_1536p_MODE == gu8SensorImageMode)
    {
        #ifndef HI_FPGA
        u32FullLines_5Fps = FRAME_LINES_3M_1536p * 30 / 5;
        #else
        u32FullLines_5Fps = FRAME_LINES_3M_1536p * 25 / 5;
        #endif
        
    }
    else if(SENSOR_3M_1296p_MODE == gu8SensorImageMode)
    {
        u32FullLines_5Fps = FRAME_LINES_3M_1296p * 30 / 5;
    }
    else
    {
        return;
    }

    u32FullLines_5Fps = (u32FullLines_5Fps > 0xFFFF) ? 0xFFFF : u32FullLines_5Fps;
    u32MaxExpTime_5Fps = u32FullLines_5Fps - 2; 
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(FRAME_LINES, u32FullLines_5Fps); /* 5fps */
        SENSOR_I2C_WRITE(EXPOSURE_TIME, u32MaxExpTime_5Fps); /* max exposure time */
        SENSOR_I2C_WRITE(ANALOG_GAIN, 0x1300);      /* AG, Context A */
        SENSOR_I2C_WRITE(DIGITAL_GAIN, 0x0080);  /* DG, Context A */
    }
    else /* setup for ISP 'normal mode' */
    {
        gu32FullLinesStd = (gu32FullLinesStd > 0xFFFF) ? 0xFFFF : gu32FullLinesStd;
        SENSOR_I2C_WRITE(FRAME_LINES, gu32FullLinesStd);    /* 30fps */
		bInit = HI_FALSE;
    }

    return;
}

HI_VOID ar0330_set_wdr_mode(HI_U8 u8Mode)
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

static HI_S32 ar0330_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;

	bInit = HI_FALSE;   
    
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if((pstSensorImageMode->u16Width <= 1920)&&(pstSensorImageMode->u16Height <= 1080))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_2M_1080p_MODE;
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
    else if((pstSensorImageMode->u16Width <= 2048)&&(pstSensorImageMode->u16Height <= 1536))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_3M_1536p_MODE;
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
    else if((pstSensorImageMode->u16Width <= 2304)&&(pstSensorImageMode->u16Height <= 1296))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_3M_1296p_MODE;
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

HI_U32 ar0330_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
   HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;
        g_stSnsRegsInfo.u32RegNum = 4;
        
        if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
        {
            g_stSnsRegsInfo.u32RegNum += 1;
        }        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = EXPOSURE_TIME;
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = ANALOG_GAIN;
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = DIGITAL_GAIN;
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = FRAME_LINES;
        
        if ((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode))
        {
            g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[3].u32RegAddr = EXPOSURE_TIME;
            g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[4].u32RegAddr = FRAME_LINES;        
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
            g_stSnsRegsInfo.astI2cData[3].bUpdate = HI_TRUE;
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

int  ar0330_set_inifile_path(const char *pcPath)
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

HI_VOID ar0330_global_init()
{   
		gu8SensorImageMode = SENSOR_3M_1536p_MODE;
		genSensorMode = WDR_MODE_NONE; 
		gu32FullLinesStd = FRAME_LINES_3M_1536p;
		gu32FullLines = FRAME_LINES_3M_1536p; 
		bInit = HI_FALSE;
		bSensorInit = HI_FALSE; 
		
		memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
		memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
		
#ifdef INIFILE_CONFIG_MODE 
		HI_S32 s32Ret = HI_SUCCESS;
		s32Ret = Cmos_LoadINIPara(pcName);
		if (HI_SUCCESS != s32Ret)
		{
			printf("Cmos_LoadINIPara failed!!!!!!\n");
		}
#else
	
#endif


}

void ar0330_reg_init()
{   
switch (gu8SensorImageMode)
    {
        case 1 :
            {
//                sensor_init_2M_1080p();
		   /*[TEST 1920x1080 30fps 12bit - Mipi 2lane, MCLK=24MHz]*/
			  SENSOR_I2C_WRITE(0x301A,0x0001); /*RESET_REGISTER = 4312*/
			  SENSOR_DELAY_MS(100);						/*Delay 100ms*/
								   
			  SENSOR_I2C_WRITE(0x3052,0xA114); 	  
			  SENSOR_I2C_WRITE(0x304A,0x0070); 
			  SENSOR_DELAY_MS(100);
									   
			  SENSOR_I2C_WRITE(0x31AE,0x0202); /*Output Interface Configured to 2 lane mipi*/			  
			  SENSOR_I2C_WRITE(0x301A,0x0058); /*Disable streaming*/		  
			  SENSOR_DELAY_MS(34);
									  
			  SENSOR_I2C_WRITE(0x3064,0x1802); /*Disable Embedded Data*/		  
			  SENSOR_I2C_WRITE(0x3078,0x0001); 
												   
			  /*OTPM V5*/						   
			  SENSOR_I2C_WRITE(0x3ED2,0x0146);   
			  SENSOR_I2C_WRITE(0x3EDA,0x88BC);   
			  SENSOR_I2C_WRITE(0x3EDC,0xAA63);   
												   
			  /*Toggle Flash on Each Frame*/	   
			  SENSOR_I2C_WRITE(0x3046,0x0000); /* Enable Flash Pin 0x4038*/		  
			  SENSOR_I2C_WRITE(0x3048,0x8480); /* Flash Pulse Length*/ 		  
			  SENSOR_I2C_WRITE(0x31E0,0x0203);
												 
			  /*Serial 12-bit PLL 98MHz*/		  
			  SENSOR_I2C_WRITE(0x302A,0x0006); /* vt_pix_clk_div*/ 		  
			  SENSOR_I2C_WRITE(0x302C,0x0002); /* vt_sys_clk_div*/ 		  
			  SENSOR_I2C_WRITE(0x302e,0x0002); /* pre_pll_clk_div*/		  
			  SENSOR_I2C_WRITE(0x3030,0x0031); /* pll_multiplier*/ 		  
			  SENSOR_I2C_WRITE(0x3036,0x000c); /* op_pix_clk_div*/ 		  
			  SENSOR_I2C_WRITE(0x3038,0x0001); /* op_sys_clk_div*/ 		  
			  SENSOR_I2C_WRITE(0x31AC,0x0C0C); /* data_format is 12-bit*/		 
			  SENSOR_I2C_WRITE(0x31B0,0x002D); /* FRAME PREAMBLE */ 
			  SENSOR_I2C_WRITE(0x31B2,0x0012); /* LINE PREAMBLE */
			  SENSOR_I2C_WRITE(0x31B4,0x3B44); /* MIPI TIMING 0*/
			  SENSOR_I2C_WRITE(0x31B6,0x314D); /* MIPI TIMING 1*/
			  SENSOR_I2C_WRITE(0x31B8,0x2089); /* MIPI TIMING 2*/
			  SENSOR_I2C_WRITE(0x31BA,0x0206); /* MIPI TIMING 3*/
			  SENSOR_I2C_WRITE(0x31BC,0x8005); /* MIPI TIMING 4*/
			  SENSOR_I2C_WRITE(0x31BE,0x2003);    
			  SENSOR_DELAY_MS(10); 					
												 
			  /*ARRAY READOUT SETTINGS*/		  
			  SENSOR_I2C_WRITE(0x3004,0x00C6); /* X_ADDR_START*/		  
			  SENSOR_I2C_WRITE(0x3008,0x0845); /* X_ADDR_END*/ 		  
			  SENSOR_I2C_WRITE(0x3002,0x00EA); /* Y_ADDR_START*/ 
			  SENSOR_I2C_WRITE(0x3006,0x0521); /* Y_ADDR_END*/ 		  
												 
			  /*Sub-sampling*/					   
			  SENSOR_I2C_WRITE(0x30A2,0x0001); /* X_ODD_INCREMENT*/		  
			  SENSOR_I2C_WRITE(0x30A6,0x0001); /* Y_ODD_INCREMENT*/  
			  SENSOR_I2C_WRITE(0x3040,0x3000); /* Row/column Bin*/
			  SENSOR_I2C_WRITE(0x3ED4,0x8F6C);
			  SENSOR_I2C_WRITE(0x3ED6,0x66CC);
												
			  /*Frame-Timing*/					   
			  SENSOR_I2C_WRITE(0x300C,0x04E0); /* LINE_LENGTH_PCK*/		  
			  SENSOR_I2C_WRITE(0x300A,0x051C); /* FRAME_LENGTH_LINES*/ 		  
			  SENSOR_I2C_WRITE(0x3014,0x0000); /* FINE_INTEGRATION_TIME*/			  
			  SENSOR_I2C_WRITE(0x3012,0x0514); /* Coarse_Integration_Time*/		  
			  SENSOR_I2C_WRITE(0x3042,0x0000); /* EXTRA_DELAY*/		  
			  SENSOR_I2C_WRITE(0x30BA,0x002C); /* Digital_Ctrl_Adc_High_Speed*/  
			  
			  SENSOR_DELAY_MS(30);
			  SENSOR_I2C_WRITE(0x3088,0x80BA);
			  SENSOR_I2C_WRITE(0x3086,0x0253); 									 
			  SENSOR_I2C_WRITE(0x301A,0x0004); /* Start Streaming*/  


			
			   printf("Aptina AR0330 sensor 2M-1080p 30fps reg init success!\n");

                bSensorInit = HI_TRUE;
                break;
            }              
        case 2 :
            {
         //      sensor_init_3M_1536p();
        #ifndef HI_FPGA
			    /*[TEST 2048x1536 30fps 12bit - Mipi 2lane, MCLK=24MHz]*/    
			    SENSOR_I2C_WRITE(0x301A, 0x0001); /*RESET_REGISTER = 4312*/
			    SENSOR_DELAY_MS(100);                         /*Delay 100ms*/
			                             
			    SENSOR_I2C_WRITE(0x3052, 0xA114);         
			    SENSOR_I2C_WRITE(0x304A, 0x0070); 
			    SENSOR_DELAY_MS(100); 
			                                    
			    SENSOR_I2C_WRITE(0x31AE, 0x0202); /*Output Interface Configured to 2 lane mipi*/          
			    SENSOR_I2C_WRITE(0x301A, 0x0058); /*Disable streaming*/           
			    SENSOR_DELAY_MS(34); 
			                     
			    SENSOR_I2C_WRITE(0x3064, 0x1802); /*Disable Embedded Data*/           
			    SENSOR_I2C_WRITE(0x3078, 0x0001); 
			                                 
			    /*OTPM V5*/                 
			    SENSOR_I2C_WRITE(0x3ED2, 0x0146);     
			    SENSOR_I2C_WRITE(0x3EDA, 0x88BC);     
			    SENSOR_I2C_WRITE(0x3EDC, 0xAA63);     
			                                 
			    /*Toggle Flash on Each Frame*/          
			    SENSOR_I2C_WRITE(0x3046, 0x0000); /* Enable Flash Pin 0x4038*/         
			    SENSOR_I2C_WRITE(0x3048, 0x8480); /* Flash Pulse Length*/       
			    SENSOR_I2C_WRITE(0x31E0, 0x0203);
			                                 
			    /*Serial 12-bit PLL 128MHz*/             
			    SENSOR_I2C_WRITE(0x302A, 0x0006); /* vt_pix_clk_div*/           
			    SENSOR_I2C_WRITE(0x302C, 0x0002); /* vt_sys_clk_div */      
			    SENSOR_I2C_WRITE(0x302e, 0x0001); /* pre_pll_clk_div*/          
			    SENSOR_I2C_WRITE(0x3030, 0x0020); /* pll_multiplier*/           
			    SENSOR_I2C_WRITE(0x3036, 0x000c); /* op_pix_clk_div */          
			    SENSOR_I2C_WRITE(0x3038, 0x0001); /* op_sys_clk_div */      
			    SENSOR_I2C_WRITE(0x31AC, 0x0C0C); /* data_format is 12-bit*/
			    SENSOR_I2C_WRITE(0x31B0, 0x0044); /* FRAME PREAMBLE */ 
			    SENSOR_I2C_WRITE(0x31B2, 0x0029); /* LINE PREAMBLE */
			    SENSOR_I2C_WRITE(0x31B4, 0x1A54); /* MIPI TIMING 0*/
			    SENSOR_I2C_WRITE(0x31B6, 0x11D3); /* MIPI TIMING 1*/
			    SENSOR_I2C_WRITE(0x31B8, 0x3049); /* MIPI TIMING 2*/
			    SENSOR_I2C_WRITE(0x31BA, 0x0208); /* MIPI TIMING 3*/
			    SENSOR_I2C_WRITE(0x31BC, 0x0007); /* MIPI TIMING 4*/
			    //SENSOR_I2C_WRITE(0x31BE, 0x2003);
			    SENSOR_DELAY_MS(10);                
			                                
			    /*ARRAY READOUT SETTINGS*/              
			    SENSOR_I2C_WRITE(0x3004, 0x0086); /* X_ADDR_START*/         
			    SENSOR_I2C_WRITE(0x3008, 0x0885); /* X_ADDR_END*/           
			    SENSOR_I2C_WRITE(0x3002, 0x0006); /* Y_ADDR_START*/ 
			    SENSOR_I2C_WRITE(0x3006, 0x0605); /* Y_ADDR_END*/           
			                                 
			    /*Sub-sampling*/                 
			    SENSOR_I2C_WRITE(0x30A2, 0x0001); /* X_ODD_INCREMENT*/          
			    SENSOR_I2C_WRITE(0x30A6, 0x0001); /* Y_ODD_INCREMENT*/  
			    SENSOR_I2C_WRITE(0x3040, 0x0000);
			    SENSOR_I2C_WRITE(0x3ED4, 0x8F6C);
			    SENSOR_I2C_WRITE(0x3ED6, 0x66CC);
			                                 
			    /*Frame-Timing*/                 
			    SENSOR_I2C_WRITE(0x300C, 0x04E0); /* LINE_LENGTH_PCK*/          
			    SENSOR_I2C_WRITE(0x300A, 0x06AD); /* FRAME_LENGTH_LINES*/           
			    SENSOR_I2C_WRITE(0x3014, 0x0000); /* FINE_INTEGRATION_TIME*/            
			    SENSOR_I2C_WRITE(0x3012, 0x06AC); /* Coarse_Integration_Time*/          
			    SENSOR_I2C_WRITE(0x3042, 0x0000); /* EXTRA_DELAY*/          
			    SENSOR_I2C_WRITE(0x30BA, 0x002C); /* Digital_Ctrl_Adc_High_Speed*/          

			    //SENSOR_I2C_WRITE(0x301A, 0x0000);    
			    SENSOR_DELAY_MS(30);
			    SENSOR_I2C_WRITE(0x3088, 0x80BA);
			    SENSOR_I2C_WRITE(0x3086, 0x0253);
			    SENSOR_I2C_WRITE(0x301A, 0x0004); /* Start Streaming*/          

			    printf("Aptina AR0330 sensor 3M-1536p 30fps init success!\n"); 
   		#else
			    /*[TEST 2048x1536 25fps 12bit - Mipi 2lane, MCLK=24MHz]*/    
			    SENSOR_I2C_WRITE(0x301A, 0x0001); /*RESET_REGISTER = 4312*/
			    SENSOR_DELAY_MS(100);                         /*Delay 100ms*/
			                             
			    SENSOR_I2C_WRITE(0x3052, 0xA114);         
			    SENSOR_I2C_WRITE(0x304A, 0x0070); 
			    SENSOR_DELAY_MS(100); 
			                                    
			    SENSOR_I2C_WRITE(0x31AE, 0x0202); /*Output Interface Configured to 2 lane mipi*/          
			    SENSOR_I2C_WRITE(0x301A, 0x0058); /*Disable streaming*/           
			    SENSOR_DELAY_MS(34); 
			                     
			    SENSOR_I2C_WRITE(0x3064, 0x1802); /*Disable Embedded Data*/           
			    SENSOR_I2C_WRITE(0x3078, 0x0001); 
			                                 
			    /*OTPM V5*/                 
			    SENSOR_I2C_WRITE(0x3ED2, 0x0146);     
			    SENSOR_I2C_WRITE(0x3EDA, 0x88BC);     
			    SENSOR_I2C_WRITE(0x3EDC, 0xAA63);     
			                                 
			    /*Toggle Flash on Each Frame*/          
			    SENSOR_I2C_WRITE(0x3046, 0x4038); /* Enable Flash Pin*/         
			    SENSOR_I2C_WRITE(0x3048, 0x8480); /* Flash Pulse Length*/       
			    SENSOR_I2C_WRITE(0x31E0, 0x0203);
			                                 
			    /*Serial 12-bit PLL 98MHz*/             
			    SENSOR_I2C_WRITE(0x302A, 0x0006); /* vt_pix_clk_div*/           
			    SENSOR_I2C_WRITE(0x302C, 0x0002); /* vt_sys_clk_div */      
			    SENSOR_I2C_WRITE(0x302e, 0x0002); /* pre_pll_clk_div*/          
			    SENSOR_I2C_WRITE(0x3030, 0x0031); /* pll_multiplier*/           
			    SENSOR_I2C_WRITE(0x3036, 0x000c); /* op_pix_clk_div */          
			    SENSOR_I2C_WRITE(0x3038, 0x0001); /* op_sys_clk_div */      
			    SENSOR_I2C_WRITE(0x31AC, 0x0C0C); /* data_format is 12-bit*/
			    SENSOR_I2C_WRITE(0x31B0, 0x0044); /* FRAME PREAMBLE */ 
			    SENSOR_I2C_WRITE(0x31B2, 0x0029); /* LINE PREAMBLE */
			    SENSOR_I2C_WRITE(0x31B4, 0x1A54); /* MIPI TIMING 0*/
			    SENSOR_I2C_WRITE(0x31B6, 0x11D3); /* MIPI TIMING 1*/
			    SENSOR_I2C_WRITE(0x31B8, 0x3049); /* MIPI TIMING 2*/
			    SENSOR_I2C_WRITE(0x31BA, 0x0208); /* MIPI TIMING 3*/
			    SENSOR_I2C_WRITE(0x31BC, 0x0007); /* MIPI TIMING 4*/
			    SENSOR_I2C_WRITE(0x31BE, 0x2003);
			    SENSOR_DELAY_MS(10);                
			                                
			    /*ARRAY READOUT SETTINGS*/              
			    SENSOR_I2C_WRITE(0x3004, 0x0086); /* X_ADDR_START*/         
			    SENSOR_I2C_WRITE(0x3008, 0x0885); /* X_ADDR_END*/           
			    SENSOR_I2C_WRITE(0x3002, 0x0006); /* Y_ADDR_START*/ 
			    SENSOR_I2C_WRITE(0x3006, 0x0605); /* Y_ADDR_END*/           
			                                 
			    /*Sub-sampling*/                 
			    SENSOR_I2C_WRITE(0x30A2, 0x0001); /* X_ODD_INCREMENT*/          
			    SENSOR_I2C_WRITE(0x30A6, 0x0001); /* Y_ODD_INCREMENT*/  
			    SENSOR_I2C_WRITE(0x3040, 0x0000);
			    SENSOR_I2C_WRITE(0x3ED4, 0x8F6C);
			    SENSOR_I2C_WRITE(0x3ED6, 0x66CC);
			                                 
			    /*Frame-Timing*/                 
			    SENSOR_I2C_WRITE(0x300C, 0x04E0); /* LINE_LENGTH_PCK*/          
			    SENSOR_I2C_WRITE(0x300A, 0x0622); /* FRAME_LENGTH_LINES*/           
			    SENSOR_I2C_WRITE(0x3014, 0x0000); /* FINE_INTEGRATION_TIME*/            
			    SENSOR_I2C_WRITE(0x3012, 0x05DC); /* Coarse_Integration_Time*/          
			    SENSOR_I2C_WRITE(0x3042, 0x0000); /* EXTRA_DELAY*/          
			    SENSOR_I2C_WRITE(0x30BA, 0x002C); /* Digital_Ctrl_Adc_High_Speed*/          
			    
			    SENSOR_DELAY_MS(30);
			    SENSOR_I2C_WRITE(0x3088, 0x80BA);
			    SENSOR_I2C_WRITE(0x3086, 0x0253);
			    SENSOR_I2C_WRITE(0x301A, 0x0004); /* Start Streaming*/          

			    printf("Aptina AR0330 sensor 3M-1536p 25fps init success!\n");
		#endif 							    
                bSensorInit = HI_TRUE;
                break;
            } 
        case 3 :
            {
				
            //    sensor_init_3M_1296p();


						/*[TEST 2304x1296 30fps 12bit - Mipi 2lane, MCLK=24MHz]*/

			    SENSOR_I2C_WRITE(0x301A, 0x0001); /*RESET_REGISTER = 4312*/
			    SENSOR_DELAY_MS(100);                         /*Delay 100ms*/
			                             
			    SENSOR_I2C_WRITE(0x3052, 0xA114);         
			    SENSOR_I2C_WRITE(0x304A, 0x0070); 
			    SENSOR_DELAY_MS(100); 
			                                    
			    SENSOR_I2C_WRITE(0x31AE, 0x0202); /*Output Interface Configured to 2 lane mipi*/          
			    SENSOR_I2C_WRITE(0x301A, 0x0058); /*Disable streaming*/           
			    SENSOR_DELAY_MS(34); 
			                     
			    SENSOR_I2C_WRITE(0x3064, 0x1802); /*Disable Embedded Data*/           
			    SENSOR_I2C_WRITE(0x3078, 0x0001); 
			                                 
			    /*OTPM V5*/                 
			    SENSOR_I2C_WRITE(0x3ED2, 0x0146);     
			    SENSOR_I2C_WRITE(0x3EDA, 0x88BC);     
			    SENSOR_I2C_WRITE(0x3EDC, 0xAA63);     
			                                 
			    /*Toggle Flash on Each Frame*/          
			    SENSOR_I2C_WRITE(0x3046, 0x0000); /* Enable Flash Pin 0x4038*/         
			    SENSOR_I2C_WRITE(0x3048, 0x8480); /* Flash Pulse Length*/       
			    SENSOR_I2C_WRITE(0x31E0, 0x0203);
			                                 
			    /*Serial 12-bit PLL 98MHz*/             
			    SENSOR_I2C_WRITE(0x302A, 0x0006); /* vt_pix_clk_div*/           
			    SENSOR_I2C_WRITE(0x302C, 0x0002); /* vt_sys_clk_div */      
			    SENSOR_I2C_WRITE(0x302e, 0x0002); /* pre_pll_clk_div*/          
			    SENSOR_I2C_WRITE(0x3030, 0x0031); /* pll_multiplier*/           
			    SENSOR_I2C_WRITE(0x3036, 0x000c); /* op_pix_clk_div */          
			    SENSOR_I2C_WRITE(0x3038, 0x0001); /* op_sys_clk_div */      
			    SENSOR_I2C_WRITE(0x31AC, 0x0C0C); /* data_format is 12-bit*/
			    SENSOR_I2C_WRITE(0x31B0, 0x002D); /* FRAME PREAMBLE */ 
			    SENSOR_I2C_WRITE(0x31B2, 0x0012); /* LINE PREAMBLE */
			    SENSOR_I2C_WRITE(0x31B4, 0x3B44); /* MIPI TIMING 0*/
			    SENSOR_I2C_WRITE(0x31B6, 0x314D); /* MIPI TIMING 1*/
			    SENSOR_I2C_WRITE(0x31B8, 0x2089); /* MIPI TIMING 2*/
			    SENSOR_I2C_WRITE(0x31BA, 0x0206); /* MIPI TIMING 3*/
			    SENSOR_I2C_WRITE(0x31BC, 0x8005); /* MIPI TIMING 4*/
			    SENSOR_I2C_WRITE(0x31BE, 0x2003);     
			    SENSOR_DELAY_MS(10);                
			                                
			    /*ARRAY READOUT SETTINGS*/              
			    SENSOR_I2C_WRITE(0x3004, 0x0006); /* X_ADDR_START*/         
			    SENSOR_I2C_WRITE(0x3008, 0x0905); /* X_ADDR_END*/           
			    SENSOR_I2C_WRITE(0x3002, 0x0078); /* Y_ADDR_START*/ 
			    SENSOR_I2C_WRITE(0x3006, 0x0587); /* Y_ADDR_END*/           
			                                 
			    /*Sub-sampling*/                 
			    SENSOR_I2C_WRITE(0x30A2, 0x0001); /* X_ODD_INCREMENT*/          
			    SENSOR_I2C_WRITE(0x30A6, 0x0001); /* Y_ODD_INCREMENT*/  
			    SENSOR_I2C_WRITE(0x3040, 0x0000);
			    SENSOR_I2C_WRITE(0x3ED4, 0x8F6C);
			    SENSOR_I2C_WRITE(0x3ED6, 0x66CC);
			                                 
			    /*Frame-Timing*/                 
			    SENSOR_I2C_WRITE(0x300C, 0x04E0); /* LINE_LENGTH_PCK*/          
			    SENSOR_I2C_WRITE(0x300A, 0x051C); /* FRAME_LENGTH_LINES*/           
			    SENSOR_I2C_WRITE(0x3014, 0x0000); /* FINE_INTEGRATION_TIME*/            
			    SENSOR_I2C_WRITE(0x3012, 0x0514); /* Coarse_Integration_Time*/          
			    SENSOR_I2C_WRITE(0x3042, 0x0000); /* EXTRA_DELAY*/          
			    SENSOR_I2C_WRITE(0x30BA, 0x002C); /* Digital_Ctrl_Adc_High_Speed*/          

			    SENSOR_DELAY_MS(30);
			    SENSOR_I2C_WRITE(0x3088, 0x80BA);
			    SENSOR_I2C_WRITE(0x3086, 0x0253);                                 
			    SENSOR_I2C_WRITE(0x301A, 0x0004); /* Start Streaming*/          
				
			    printf("Aptina AR0330 sensor 3M-1296p 30fps init success!\n");
				
			    bSensorInit = HI_TRUE;
                break;
            }         
        default :
            {
                printf("Not support!\n");              
            }
    }

return ;
}


HI_S32 ar0330_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ar0330_reg_init;
//	pstSensorExpFunc->pfn_cmos_sensor_exit = ar0330_sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ar0330_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = ar0330_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ar0330_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = ar0330_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ar0330_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ar0330_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = ar0330_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int ar0330_sensor_register_callback(void)
{
	ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ar0330_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, AR0330_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    ar0330_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, AR0330_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    ar0330_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, AR0330_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int ar0330_sensor_unregister_callback(void)
{
	ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, AR0330_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, AR0330_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
	strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, AR0330_ID);
    if (s32Ret)
    {
		printf("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

HI_S32 APTINA_AR0330_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)
{	
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0330_reg_init();

	ar0330_sensor_register_callback();

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
	printf("Aptina AR0330 sensor 1536P 25fps init success!\n");
	
	return 0;

}

int AR0330_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_AR0330_WIDTH;
        *ret_height = SENSOR_AR0330_HEIGHT;
        return 0;
    }
    return -1;
}
int AR0330_get_sensor_name(char *sensor_name)
{
	if(sensor_name != NULL)
	{
		memcpy(sensor_name,SENSOR_NAME,strlen(SENSOR_NAME));
		return 0;
	}
	return -1;
}

bool SENSOR_AR0330_probe()
{
    uint16_t ret_data1 = 0;
    SENSOR_I2C_READ(0x3000, &ret_data1);
    if(ret_data1 == AR0330_CHECK_DATA){
        sdk_sys->write_reg(0x200f0050, 0x2);//i2c0_scl
        sdk_sys->write_reg(0x200f0054, 0x2);//i2c0_sda
        sdk_sys->write_reg(0x2003002c, 0xE0007);//set sensor clock,sensor unreset, clk 24MHz, VI 250MHz
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





