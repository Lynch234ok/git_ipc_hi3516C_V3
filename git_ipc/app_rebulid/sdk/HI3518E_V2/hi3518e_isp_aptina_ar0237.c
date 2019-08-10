#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "ar0237"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x20, 2, 2))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x20, 2, 2))

#define SENSOR_AR0237_WIDTH 1920
#define SENSOR_AR0237_HEIGHT 1080
#define AR0237_CHECK_DATA (0x1256)//chip_version_reg
#define AR0238_CHECK_DATA (0x0256)//chip_version_reg
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__AR0237_CMOS_H_)
#define __AR0237_CMOS_H_

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

#define AR0237_ID 598

/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/
#ifdef INIFILE_CONFIG_MODE

static  AE_SENSOR_DEFAULT_S  g_AeDft[];
static  AWB_SENSOR_DEFAULT_S g_AwbDft[];
static  ISP_CMOS_DEFAULT_S   g_IspDft[];
static HI_S32 ar0237_LoadINIPara(const HI_CHAR *pcName);
#else

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr=0x20;
static unsigned int sensor_addr_byte=2;
static unsigned int sensor_data_byte=2;

#define FULL_LINES_MAX  (0xFFFF)

#define ANALOG_GAIN    (0x3060)
#define DIGITAL_GAIN   (0x305E)
#define FRAME_LINES    (0x300A)
#define EXPOSURE_TIME  (0x3012)
#define LINE_LEN_PCK   (0x300C)

#define SENSOR_2M_1080p30_MODE  (1) 
#define SENSOR_2M_1080p60_MODE  (2)

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define FRAME_LINES_2M_1080p  (1106+INCREASE_LINES)


// Max integration time for HDR mode: T1 max = min ( 70* 16, FLL*16/17), when ratio=16;
// we use a constant full lines in build-in wdr mode
#define LONG_EXP_SHT_CLIP     (FRAME_LINES_2M_1080p*16/17)

static HI_U8 gu8SensorImageMode = SENSOR_2M_1080p30_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = FRAME_LINES_2M_1080p;
static HI_U32 gu32FullLines = FRAME_LINES_2M_1080p; 
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


#define PATHLEN_MAX 256
#define CMOS_CFG_INI "ar0237_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/ar0237_cfg.ini";



/* AE default parameter and function */
#ifdef INIFILE_CONFIG_MODE
static HI_S32 ar0237_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }
    
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;
    
    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0.33;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0078125;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0078125; 
    
    switch(genSensorMode)
    {
        case WDR_MODE_NONE:   /*linear mode*/
        {
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = g_AeDft[0].u8AeCompensation;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[0].u32MaxIntTimeTarget;
            pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[0].u32MinIntTimeTarget;
    
            pstAeSnsDft->u32MaxAgain = 12288;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = g_AeDft[0].u32MaxAgainTarget;
            pstAeSnsDft->u32MinAgainTarget = g_AeDft[0].u32MinAgainTarget;
            
            pstAeSnsDft->u32MaxDgain = 2046;  
            pstAeSnsDft->u32MinDgain = 128;
            pstAeSnsDft->u32MaxDgainTarget = g_AeDft[0].u32MaxDgainTarget;
            pstAeSnsDft->u32MinDgainTarget = g_AeDft[0].u32MinDgainTarget;
          
            pstAeSnsDft->u32ISPDgainShift = g_AeDft[0].u32ISPDgainShift;
            pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[0].u32MinISPDgainTarget;
            pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[0].u32MaxISPDgainTarget;    
            break;  
        }
        case WDR_MODE_BUILT_IN:
        {
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
           
            pstAeSnsDft->u8AeCompensation = g_AeDft[1].u8AeCompensation;
            
            pstAeSnsDft->u32MaxIntTime = LONG_EXP_SHT_CLIP;  
            pstAeSnsDft->u32MinIntTime = 32;
            pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[1].u32MaxIntTimeTarget;
            pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[1].u32MinIntTimeTarget;
            
            pstAeSnsDft->u32MaxAgain = 2048; 
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = g_AeDft[1].u32MaxAgainTarget;
            pstAeSnsDft->u32MinAgainTarget = g_AeDft[1].u32MinAgainTarget;
            
            pstAeSnsDft->u32MaxDgain = 384;
            pstAeSnsDft->u32MinDgain = 128;
            pstAeSnsDft->u32MaxDgainTarget = g_AeDft[1].u32MaxDgainTarget;
            pstAeSnsDft->u32MinDgainTarget = g_AeDft[1].u32MinDgainTarget;

            pstAeSnsDft->u32ISPDgainShift = g_AeDft[1].u32ISPDgainShift;
            pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[1].u32MinISPDgainTarget;
            pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[1].u32MaxISPDgainTarget;

            break;
        }
        
        default:
            break;
    }
    return 0;
}

#else

static HI_S32 ar0237_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }
    
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;

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

    pstAeSnsDft->enIrisType = ISP_IRIS_DC_TYPE;
    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    switch(genSensorMode)
    {
        case WDR_MODE_NONE:   /*linear mode*/
        {
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x38;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;    
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 29030;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
            
            pstAeSnsDft->u32MaxDgain = 2046; 
            pstAeSnsDft->u32MinDgain = 128;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;            
            break; 
        }
        case WDR_MODE_BUILT_IN:
        {
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
           
            pstAeSnsDft->u8AeCompensation = 0x38;
            
            pstAeSnsDft->u32MaxIntTime = LONG_EXP_SHT_CLIP;  
            pstAeSnsDft->u32MinIntTime = 32;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;
            
            pstAeSnsDft->u32MaxAgain = 4096; 
            pstAeSnsDft->u32MinAgain = 1719;
            pstAeSnsDft->u32MaxAgainTarget = 8192;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
            
            pstAeSnsDft->u32MaxDgain = 384;
            pstAeSnsDft->u32MinDgain = 128;
            pstAeSnsDft->u32MaxDgainTarget = 384;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
            pstAeSnsDft->u32MaxISPDgainTarget = 8 << pstAeSnsDft->u32ISPDgainShift;

            break;
        }
        
        default:
            break;
    }
    
    return 0;
}

#endif

/* the function of sensor set fps */
static HI_VOID ar0237_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{    
    if (WDR_MODE_BUILT_IN == genSensorMode)
    {
        if (30 == f32Fps)
        {
            gu32FullLinesStd = FRAME_LINES_2M_1080p;
        }
        else if (25 == f32Fps)
        {
            gu32FullLinesStd = FRAME_LINES_2M_1080p * 30 /f32Fps;
        }
        else
        {
            printf("Not support Fps: %f\n", f32Fps);
            return;
        }

        pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd*16/17;
    }
    else
    {
        if (SENSOR_2M_1080p30_MODE == gu8SensorImageMode)
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
        else if (SENSOR_2M_1080p60_MODE == gu8SensorImageMode)
        {
            if ((f32Fps <= 60) && (f32Fps >= 0.5))
            {
                gu32FullLinesStd = (FRAME_LINES_2M_1080p * 60) / f32Fps;
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }
        }

        pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    }    
   
   
    gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;
    g_stSnsRegsInfo.astI2cData[3].u32Data = gu32FullLinesStd;
        
    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    gu32FullLines = gu32FullLinesStd;
    pstAeSnsDft->u32FullLines = gu32FullLines;

    return;
}
static HI_VOID ar0237_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;
    pstAeSnsDft->u32FullLines = gu32FullLines;
  
    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[3].u32Data = gu32FullLines;
    }
    else
    {
       pstAeSnsDft->u32FullLines = gu32FullLinesStd;
       return;
    }
  
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 2;
    
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ar0237_inttime_update(HI_U32 u32IntTime)
{
    g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;

    return;
}

static  HI_U32  again_table[51] = 
{ 
    //here 1024 means x1.52
    1024,1075,1132,1195,1265,1344,1434,1536,1654,1792,1814,1873,1935,2002,2074,
    2150,2233,2322,2419,2524,2639,2765,2903,3056,3226,3415,3629,3871,4147,4466,
    4838,5278,5806,6451,7258,7741,8294,8932,9677,10557,11612,12902,14515,15483,16589,
    17865,19354,21113,23224,25805,29030
};

static HI_VOID ar0237_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;
    HI_U32 u32InTimes;

    if(!pu32AgainDb || !pu32AgainLin)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
    
    u32InTimes = *pu32AgainLin;
   
    if (u32InTimes >= again_table[50])
    {
         *pu32AgainLin = again_table[50];
         *pu32AgainDb = 50;
         return ;
    }
    
    for(i = 1; i < 51; i++)
    {
        if(u32InTimes < again_table[i])
        {
            *pu32AgainLin = again_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }

    }
          
    return;    
}

static HI_VOID ar0237_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    static HI_BOOL bHCG = HI_FALSE;

    if(u32Again < 10)
    {
        if(HI_TRUE == bHCG)
        {
            g_stSnsRegsInfo.astI2cData[4].u32Data = 0x0000;
            g_stSnsRegsInfo.astI2cData[5].u32Data = 0x0B08;
            g_stSnsRegsInfo.astI2cData[6].u32Data = 0x1E13;
            g_stSnsRegsInfo.astI2cData[7].u32Data = 0x0080;
            
            if (WDR_MODE_NONE != genSensorMode)
            {
                g_stSnsRegsInfo.astI2cData[8].u32Data = 0x0480;
                g_stSnsRegsInfo.astI2cData[9].u32Data = 0x0480;
            }
            else
            {           
                g_stSnsRegsInfo.astI2cData[8].u32Data =  0x0080;
                g_stSnsRegsInfo.astI2cData[9].u32Data =  0x0080;
                g_stSnsRegsInfo.astI2cData[10].u32Data =  0x0080;
                g_stSnsRegsInfo.astI2cData[11].u32Data = 0x0080;
            } 
            bHCG = HI_FALSE;
        }
        
        if(u32Again < 5)
        {
            g_stSnsRegsInfo.astI2cData[1].u32Data = u32Again + 0xb;
        }
        else
        {
            //0x10 + (again - 5) * 2
            g_stSnsRegsInfo.astI2cData[1].u32Data = u32Again * 2 + 0x6;
        }
    }
    else
    {
        if(HI_FALSE == bHCG)
        {
            g_stSnsRegsInfo.astI2cData[4].u32Data = 0x0004;
            g_stSnsRegsInfo.astI2cData[5].u32Data = 0x1C0E;
            g_stSnsRegsInfo.astI2cData[6].u32Data = 0x4E39;
            g_stSnsRegsInfo.astI2cData[7].u32Data = 0x00B0;
            
            if (WDR_MODE_NONE != genSensorMode)
            {
                g_stSnsRegsInfo.astI2cData[8].u32Data = 0x0780;
                g_stSnsRegsInfo.astI2cData[9].u32Data = 0x0780;
            }
            else
            {           
                g_stSnsRegsInfo.astI2cData[8].u32Data =  0xFF80;
                g_stSnsRegsInfo.astI2cData[9].u32Data =  0xFF80;
                g_stSnsRegsInfo.astI2cData[10].u32Data =  0xFF80;
                g_stSnsRegsInfo.astI2cData[11].u32Data = 0xFF80;
            }
            
            bHCG = HI_TRUE;
        }
        
        if(u32Again < 26)
        {
            g_stSnsRegsInfo.astI2cData[1].u32Data = u32Again - 10;
        }
        else
        {
            //0x10 + (again - 26) * 2
            g_stSnsRegsInfo.astI2cData[1].u32Data = u32Again * 2 - 36;
        }
    }
    
    g_stSnsRegsInfo.astI2cData[2].u32Data = u32Dgain;
    

    return;
}

HI_S32 ar0237_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ar0237_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ar0237_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ar0237_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ar0237_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ar0237_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ar0237_again_calc_table;
 

    return 0;
}


/* AWB default parameter and function */
#ifdef INIFILE_CONFIG_MODE

static HI_S32 ar0237_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
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
        
        case WDR_MODE_BUILT_IN:
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
    5050,
    {
		0x1EA,0x8082,0x8068,		
		0x805F,0x19E,0x803F,		
		0x800E,0x8096,0x1A4,
    },
        
    3932,
    {
		0x20E,0x806F,0x809F,			
		0x805D,0x17B,0x801E,			
		0x8010,0x80A6,0x1B6,   
    },

    2658,
    {
		0x1DD,0x8066,0x8077,		
		0x8058,0x16B,0x8013,		
		0x803D,0x809E,0x1DB
    }
};

static AWB_CCM_S g_stAwbCcmWDR =
{
    5050,
    {
        0x01B3, 0x80AB, 0x8008,
        0x805C, 0x0142, 0x001A,
        0x803D, 0x810C, 0x0249,
    },
        
    3932,
    {
        0x0130, 0x8031, 0x0001,
        0x8041, 0x010C, 0x0035,
        0x8021, 0x8095, 0x01B6,
    },

    2658,
    {
        0x012E, 0x8024, 0x800A,
        0x8060, 0x0138, 0x0028,
        0x8053, 0x80F5, 0x0248,
    }
};

static AWB_AGC_TABLE_S g_stAwbAgcTableLin =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x80,0x80,0x72,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38,0x38}
};

static AWB_AGC_TABLE_S g_stAwbAgcTableWDR =
{
    /* bvalid */
    1,

    /* saturation */
    {0x80,0x78,0x72,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38,0x38}
};


static HI_S32 ar0237_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;

    pstAwbSnsDft->au16GainOffset[0] = 386;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 495;

    pstAwbSnsDft->as32WbPara[0] = 104;
    pstAwbSnsDft->as32WbPara[1] = -44;
    pstAwbSnsDft->as32WbPara[2] = -195;
    pstAwbSnsDft->as32WbPara[3] = 206912;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -154676;
    
    
    switch (genSensorMode)
    {
        default:
        case WDR_MODE_NONE:
            
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableLin, sizeof(AWB_AGC_TABLE_S));
        break;

        case WDR_MODE_BUILT_IN:
            
            memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcmWDR, sizeof(AWB_CCM_S));
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableWDR, sizeof(AWB_AGC_TABLE_S));
        break;
    }
    
    return 0;
}

#endif


HI_S32 ar0237_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ar0237_get_awb_default;

    return 0;
}


/* ISP default parameter and function */
#ifdef INIFILE_CONFIG_MODE

HI_U32 ar0237_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            memcpy(&pstDef->stNoiseTbl, &g_IspDft[0].stNoiseTbl, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_IspDft[0].stDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_IspDft[0].stRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
            memcpy(&pstDef->stGamma, &g_IspDft[0].stGamma, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stGe, &g_IspDft[0].stGe, sizeof(ISP_CMOS_GE_S));            
        break;
        case WDR_MODE_BUILT_IN:            
            memcpy(&pstDef->stDrc, &g_IspDft[1].stDrc, sizeof(ISP_CMOS_DRC_S));

            memcpy(&pstDef->stNoiseTbl, &g_IspDft[1].stNoiseTbl, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_IspDft[1].stDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_IspDft[1].stRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));            
            memcpy(&pstDef->stGamma, &g_IspDft[1].stGamma, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stGammafe, &g_IspDft[1].stGammafe, sizeof(ISP_CMOS_GAMMAFE_S));
            memcpy(&pstDef->stGe, &g_IspDft[1].stGe, sizeof(ISP_CMOS_GE_S));
        break;

    }

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_AR0237_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_AR0237_HEIGHT;

    return 0;
}

#else


#define  DMNR_CALIB_CARVE_NUM_AR0230  (12)
static HI_FLOAT g_coef_calib_AR0230[DMNR_CALIB_CARVE_NUM_AR0230][HI_ISP_NR_CALIB_COEF_COL] = 
{
    {117.000000f, 2.068186f, 0.038048f, 8.539988f, }, 
    {234.000000f, 2.369216f, 0.038854f, 8.647237f, }, 
    {338.000000f, 2.528917f, 0.038449f, 8.986108f, }, 
    {507.000000f, 2.705008f, 0.039314f, 9.173982f, }, 
    {788.000000f, 2.896526f, 0.042165f, 9.135517f, }, 
    {945.000000f, 2.975432f, 0.042747f, 9.325232f, }, 
    {1182.000000f, 3.072618f, 0.044354f, 9.478379f, }, 
    {1583.000000f, 3.199481f, 0.046800f, 9.752023f, }, 
    {3135.000000f, 3.496238f, 0.053930f, 11.135441f, }, 
    {6201.000000f, 3.792462f, 0.063879f, 14.289218f, }, 
    {9014.000000f, 3.954918f, 0.070638f, 17.333263f, }, 
    {13598.000000f, 4.133475f, 0.078309f, 22.591230f, }, 
};

static HI_FLOAT g_coef_calib_AR0230WDR[DMNR_CALIB_CARVE_NUM_AR0230][HI_ISP_NR_CALIB_COEF_COL] = 
{
    {303.000000f, 2.481443f, 0.024227f, 13.260330f, }, 
    {389.000000f, 2.589950f, 0.026167f, 12.662060f, }, 
    {880.000000f, 2.944483f, 0.024233f, 14.136049f, }, 
    {1200.000000f, 3.079181f, 0.023359f, 14.992138f, }, 
    {1594.000000f, 3.202488f, 0.020216f, 18.146648f, }, 
    {2711.000000f, 3.433130f, 0.000000f, 27.000000f, }, 
    {5422.000000f, 3.734159f, 0.000000f, 26.000000f, }, 
    {6786.000000f, 3.831614f, 0.000000f, 26.000000f, }, 
    {9072.000000f, 3.957703f, 0.000000f, 25.000000f, }, 
    {9072.000000f, 3.957703f, 0.000000f, 25.000000f, }, 
    {9072.000000f, 3.957703f, 0.000000f, 25.000000f, }, 
    {9072.000000f, 3.957703f, 0.000000f, 25.000000f, }, 
};

static ISP_NR_ISO_PARA_TABLE_S g_stNrIsoParaTab[HI_ISP_NR_ISO_LEVEL_MAX] = 
{
     //u16Threshold//u8varStrength//u8fixStrength//u8LowFreqSlope	
       {1750,       256-224,             256-256,            0 },  //100    //                       //                                                
       {1750,       256-224,             256-256,            0 },  //200    // ISO //Thr//var//fix   // ISO //u8LowFreqSlope
       {1750,       256-224,             256-256,            0 },  //400    //{400,  1400, 80,256},  //{400 , 0  }
       {1750,       256-224,             256-256,            8 },  //800    //{800,  1400, 72,256},  //{600 , 2  }
       {1750,       256-208,             256-256,            6 },  //1600   //{1600, 1400, 64,256},  //{800 , 8  }
       {1750,       256-208,             256-256,            0 },  //3200   //{3200, 1400, 48,230},  //{1000, 12 }
       {1750,       256-208,             256-256,            0 },  //6400   //{6400, 1400, 48,210},  //{1600, 6  }
       {1750,       256-204,             256-256,            0 },  //12800  //{12000,1400, 32,180},  //{2400, 0  }
       {1625,       256-204,             256-256,            0 },  //25600  //{36000,1300, 48,160},  //           
       {1375,       256-204,             256-256,            0 },  //51200  //{64000,1100, 40,140},  //           
       {1250,       256-204,             256-256,            0 },  //102400 //{82000,1000, 30,128},  //           
       {1250,       256-192,             256-256,            0 },  //204800 //                       //
       {1250,       256-192,             256-256,            0 },  //409600 //                       //
       {1250,       256-192,             256-256,            0 },  //819200 //                       //
       {1250,       256-192,             256-256,            0 },  //1638400//                       //
       {1250,       256-192,             256-256,            0 },  //3276800//                       //
};

static ISP_NR_ISO_PARA_TABLE_S g_stNrIsoParaTabWDR[HI_ISP_NR_ISO_LEVEL_MAX] = 
{
     //u16Threshold//u8varStrength//u8fixStrength//u8LowFreqSlope	
       {1500,       256-240,            256-256,            0 },  //100    //                      //                                                
       {1500,       256-240,            256-256,            0 },  //200    // ISO //Thr//var//fix  // ISO //u8LowFreqSlope
       {1500,       256-240,            256-256,            0 },  //400    //{400,  1200,240,256}, //{400 , 0  }
       {1500,       256-220,            256-256,            4 },  //800    //{800,  1200,220,256}, //{600 , 2  }
       {1500,       256-200,            256-256,            4 },  //1600   //{1600, 1200,200,256}, //{800 , 4  }
       {1500,       256-200,            256-256,            0 },  //3200   //{3200, 1200,200,256}, //{1000, 6  }
       {1500,       256-200,            256-256,            0 },  //6400   //{6400, 1200,200,256}, //{1600, 4  }
       {1500,       256-200,            256-256,            0 },  //12800  //{12000,1200,200,256}, //{2400, 0  }
       {1500,       256-180,            256-256,            0 },  //25600  //{36000,1200,180,256}, //           
       {1375,       256-180,            256-256,            0 },  //51200  //{64000,1100,180,256}, //           
       {1375,       256-180,            256-256,            0 },  //102400 //{82000,1100,180,256}, //           
       {1375,       256-180,            256-256,            0 },  //204800 //                      //
       {1375,       256-180,            256-256,            0 },  //409600 //                      //
       {1375,       256-180,            256-256,            0 },  //819200 //                      //
       {1375,       256-180,            256-256,            0 },  //1638400//                      //
       {1375,       256-180,            256-256,            0 },  //3276800//                      //
};



static ISP_CMOS_DEMOSAIC_S g_stIspDemosaicLin =
{
	/*For Demosaic*/
	1, /*bEnable*/			
	12,/*u16VhLimit*/	
	8,/*u16VhOffset*/
	48,   /*u16VhSlope*/
	/*False Color*/
	1,    /*bFcrEnable*/
	{24,24,24,24,24,24,24,24,10, 0, 0, 0, 0, 0, 0, 0},    /*au8FcrStrength[ISP_AUTO_ISO_STENGTH_NUM]*/
	{24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24},    /*au8FcrThreshold[ISP_AUTO_ISO_STENGTH_NUM]*/	
	/*For Ahd*/
	400, /*u16UuSlope*/	
	{512,512,512,512,512,512,512,  400,  0,0,0,0,0,0,0,0}    /*au16NpOffset[ISP_AUTO_ISO_STENGTH_NUM]*/	
};


static ISP_CMOS_DEMOSAIC_S g_stIspDemosaicWDR =
{
	/*For Demosaic*/
	1, /*bEnable*/			
	24,/*u16VhLimit*/	
	16,/*u16VhOffset*/
	48,   /*u16VhSlope*/
	/*False Color*/
	1,    /*bFcrEnable*/
	{12,12,12,12,12,12,12,12, 5, 0, 0, 0, 0, 0, 0, 0},    /*au8FcrStrength[ISP_AUTO_ISO_STENGTH_NUM]*/
	{24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24},    /*au8FcrThreshold[ISP_AUTO_ISO_STENGTH_NUM]*/	
	/*For Ahd*/
	200, /*u16UuSlope*/	
	{512,512,512,512,512,512,512,400,0,0,0,0,0,0,0,0}    /*au16NpOffset[ISP_AUTO_ISO_STENGTH_NUM]*/
};

static ISP_CMOS_GE_S g_stIspGeLin =
{
	/*For GE*/
	0,    /*bEnable*/			
	7,    /*u8Slope*/	
	7,    /*u8Sensitivity*/
	4096, /*u16Threshold*/
	4096, /*u16SensiThreshold*/	
	{1024,1024,1024,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048}    /*au16Strength[ISP_AUTO_ISO_STENGTH_NUM]*/	
};

static ISP_CMOS_GE_S g_stIspGeWDR =
{
	/*For GE*/
	0,    /*bEnable*/			
	7,    /*u8Slope*/	
	7,    /*u8Sensitivity*/
	4096, /*u16Threshold*/
	4096, /*u16SensiThreshold*/	
	{1024,1024,1024,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048}    /*au16Strength[ISP_AUTO_ISO_STENGTH_NUM]*/	
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
         1,    68,   118,   163,   205,   244,   282,   318,   353,   388,   421,   454,   485,   517,   547,   577,
       607,   636,   665,   693,   721,   748,   776,   802,   829,   855,   881,   907,   932,   957,   982,  1007,
      1031,  1055,  1079,  1103,  1127,  1150,  1173,  1196,  1219,  1241,  1264,  1286,  1309,  1330,  1352,  1374,
      1395,  1417,  1438,  1459,  1480,  1500,  1521,  1542,  1562,  1582,  1603,  1622,  1642,  1662,  1682,  1702,
      1721,  1740,  1760,  1779,  1798,  1817,  1836,  1854,  1873,  1892,  1910,  1928,  1947,  1965,  1983,  2001,
      2019,  2037,  2054,  2072,  2090,  2107,  2125,  2142,  2159,  2177,  2194,  2211,  2228,  2245,  2262,  2278,
      2295,  2312,  2328,  2345,  2361,  2378,  2394,  2410,  2427,  2443,  2459,  2475,  2491,  2507,  2523,  2538,
      2554,  2570,  2585,  2601,  2616,  2632,  2647,  2663,  2678,  2693,  2708,  2723,  2739,  2754,  2769,  2784,
      2799,  2813,  2828,  2843,  2858,  2872,  2887,  2902,  2916,  2931,  2945,  2960,  2974,  2988,  3003,  3017,
      3031,  3045,  3059,  3073,  3087,  3102,  3115,  3129,  3143,  3157,  3171,  3185,  3199,  3212,  3226,  3240,
      3253,  3267,  3280,  3294,  3307,  3321,  3334,  3348,  3361,  3542,  3718,  3889,  4057,  4220,  4381,  4539,
      4694,  4847,  4999,  5149,  5297,  5448,  5599,  5749,  5899,  6048,  6196,  6344,  6492,  6639,  6787,  6934,
      7082,  7300,  7518,  7737,  7957,  8177,  8398,  8620,  8844,  9293,  9747, 10205, 10667, 11640, 12629, 13633,
     14648, 15674, 16708, 17749, 18796, 19846, 20900, 21956, 23014, 24074, 25135, 26196, 27259, 28321, 29384, 30447,
     31510, 32573, 33637, 34700, 35764, 36827, 37888, 38950, 40013, 41077, 42140, 43204, 44267, 45330, 46394, 47457,
     48521, 49584, 50648, 51711, 52775, 53838, 54902, 55965, 57028, 58092, 59155, 60219, 61282, 62346, 63409, 64475,
     65535
	}
};

static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800}; //ISO
	{0,		0,		0,		0,		1,		1,		1,		1,		1,		1,		1,		1,		1,		1,		1,			1},/* bEnLowLumaShoot */
    {40,	40,		35, 	35, 	35, 	30,  	30,  	25,    	21,    	15,    	12,    	12,    	12,    	12,    	12,    		12},/*SharpenUD*/
    {30,	30,		32,	 	32, 	35, 	35,  	37, 	37, 	40, 	45, 	50, 	50, 	50, 	50, 	50, 		50},/*SharpenD*/
    {10,    10,  	12,  	14, 	16, 	18,  	20,  	22,    	24,    	26,    	28,    	30,    	30,    	30,    	30,    		30},/*TextureNoiseThd*/
    {10,    10,  	12,  	14, 	16, 	18,  	20,  	22,    	24,    	26,    	28,    	30,    	30,    	30,    	30,    		30},/*EdgeNoiseThd*/
    {  150, 140,  	120,  	110,  	110,  	60,    	40,    	30,    	20,    	10,    	0,    	0,    	0,    	0,    	0,    		0},/*overshoot*/
    {  160, 160,  	160,  	160, 	200, 	200,  	200,  	200,   	200,  	220,  	255,  	255,   	255,  	255,  	255,  		255},/*undershoot*/
};

static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpenWDR =
{      
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800}; //ISO
	{  0,	 0,		 0,		 0,		 1,		 1,		 1,		 1,		 1,		 1,		 1,		 1,		 1,		 1,		1,			1},/* bEnLowLumaShoot */
    { 55,	55,		55, 	55, 	50, 	45,  	40,  	40,    	40,    	40,    	43,    	50,    	50,    	50,    	50,    		50},/*SharpenUD*/
    { 50,	50,		50, 	50, 	50, 	40,  	40, 	45, 	45, 	45, 	40, 	40, 	40, 	40, 	40, 		40},/*SharpenD*/
    {10,    10,  	12,  	14, 	16, 	18,  	20,  	22,    	24,    	26,    	28,    	30,    	30,    	30,    	30,    		30},/*TextureNoiseThd*/
    {10,    10,  	12,  	14, 	16, 	14,  	12,  	8,    	4,    	8,    	12,    	16,    	24,    	30,    	30,    		30},/*EdgeNoiseThd*/
    { 70,   70,  	70,   	60,   	60,     60,     60,     70,    	70,    	80,    	80,    	80,    	80,    	80,    	80,    		80},/*overshoot*/
    {170,  170,    170,  	160, 	160,   160,    160,    150,    160,    170,    200,    200,  	200,  	200,  	200,  		200},/*undershoot*/
};

static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800}; //ISO
	{1,		2,		4,		6,		8,		10,		12,		14,		16,		18,		20,		22,		22,		22,		22,			22},      /*u8UvnrThreshold*/
 	{0,		0,		0,		0,		0,		0,		0,		0,		1,		1,		2,		2,		2,		2,		2,			2},       /*ColorCast*/
 	{0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,			0}  /*u8UvnrStrength*/
};

static ISP_CMOS_DPC_S g_stCmosDpc = 
{
	//0,/*IR_channel*/
	//0,/*IR_position*/
	{70,130,244,248,250,252,252,252,252,252,252,252,252,252,252,252},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0,0,0x23,0x80,0xD0,0xF0,0xF0,0xF0,0xF0},/*au16BlendRatio[16]*/
};

static ISP_CMOS_DPC_S g_stCmosDpcWDR = 
{
	//0,/*IR_channel*/
	//0,/*IR_position*/
	{70,130,244,248,250,252,252,252,252,252,252,252,252,252,252,252},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0,0,0x23,0x80,0xD0,0xF0,0xF0,0xF0,0xF0},/*au16BlendRatio[16]*/
};

static ISP_CMOS_COMPANDER_S g_stCmosCompander =
{
	12,
	16,
	32,
	512,
	80,
	2048,
	108,
	16384,
	124,
	32768,
	129,
	32768
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

static ISP_CMOS_DRC_S g_stIspDrcWDR =
{
    1,
    10,
    0,
    2,
    192,
    54,
    0,
    0,
    0,
    {900,900,900,900,900,900,900,900,900,900,900,900,900,900,900,900,890,880,870,860,850,840,830,820,810,800,790,780,770,760,750,740,720}
};

HI_U32 ar0237_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            
            memcpy(&pstDef->stDrc, &g_stIspDrcLin, sizeof(ISP_CMOS_DRC_S));
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaicLin, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stGe, &g_stIspGeLin, sizeof(ISP_CMOS_GE_S));			
			
			pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_AR0230;
			pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_AR0230;
			memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);
			

			memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
			memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
			memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));
            
        break;
        case WDR_MODE_BUILT_IN:
            
            memcpy(&pstDef->stDrc, &g_stIspDrcWDR, sizeof(ISP_CMOS_DRC_S));
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaicWDR, sizeof(ISP_CMOS_DEMOSAIC_S));	
            memcpy(&pstDef->stGe, &g_stIspGeWDR, sizeof(ISP_CMOS_GE_S));

			pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_AR0230;
			pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_AR0230WDR;
			memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTabWDR[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);
			
            memcpy(&pstDef->stGammafe, &g_stGammafe, sizeof(ISP_CMOS_GAMMAFE_S));
         
			memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpenWDR, sizeof(ISP_CMOS_RGBSHARPEN_S));
			memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
			memcpy(&pstDef->stDpc,       &g_stCmosDpcWDR,       sizeof(ISP_CMOS_DPC_S));
			memcpy(&pstDef->stCompander,  &g_stCmosCompander, sizeof(ISP_CMOS_COMPANDER_S));
			
        break;
    }

    pstDef->stSensorMaxResolution.u32MaxWidth  = 1920;
    pstDef->stSensorMaxResolution.u32MaxHeight = 1080;

    return 0;
}

#endif

HI_U32 ar0237_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    switch (genSensorMode)
    {
        default :
        case WDR_MODE_NONE :
            for (i=0; i<4; i++)
            {
                pstBlackLevel->au16BlackLevel[i] = 0xA8;
            }
            break;
        case WDR_MODE_BUILT_IN :
            for (i=0; i<4; i++)
            {
                pstBlackLevel->au16BlackLevel[i] = 0xA8;
            }
            break;
    }

    return 0;    
}

HI_VOID ar0237_set_pixel_detect(HI_BOOL bEnable)
{
    
    HI_U32 u32FullLines_5Fps = FRAME_LINES_2M_1080p * 30 / 5;
    HI_U32 u32MaxExpTime_5Fps = u32FullLines_5Fps - 2;
    
    if (WDR_MODE_BUILT_IN == genSensorMode)
    {
        return;
    }

    u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
    u32MaxExpTime_5Fps = u32FullLines_5Fps - 2; 
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(FRAME_LINES, u32FullLines_5Fps); /* 5fps */
        SENSOR_I2C_WRITE(EXPOSURE_TIME, u32MaxExpTime_5Fps); /* max exposure time */
        SENSOR_I2C_WRITE(ANALOG_GAIN, 0x0);      /* AG, Context A */
        SENSOR_I2C_WRITE(DIGITAL_GAIN, 0x0080);  /* DG, Context A */
    }
    else /* setup for ISP 'normal mode' */
    {
        gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;
        SENSOR_I2C_WRITE(FRAME_LINES, gu32FullLinesStd);    /* 30fps */
        bInit = HI_FALSE;
    }

    return;
}

HI_VOID ar0237_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            genSensorMode = WDR_MODE_NONE;
        break;

        case WDR_MODE_BUILT_IN:
            genSensorMode = WDR_MODE_BUILT_IN;
        break;

        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    return;
}    


static HI_S32 ar0237_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
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
            u8SensorImageMode = SENSOR_2M_1080p30_MODE;
        }
        else
        {
            u8SensorImageMode = SENSOR_2M_1080p60_MODE;
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

HI_U32 ar0237_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{

    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;
        g_stSnsRegsInfo.u32RegNum = 12;
        for (i=0; i < g_stSnsRegsInfo.u32RegNum; i++)
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
        
        // related registers in DCG mode
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x3100;
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x3206;
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x3208;
        g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[7].u32RegAddr = 0x3202;
        if (WDR_MODE_NONE != genSensorMode)
        {
            g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[8].u32RegAddr = 0x3096;
            g_stSnsRegsInfo.astI2cData[8].u32Data = 0x480;
            g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[9].u32RegAddr = 0x3098;
            g_stSnsRegsInfo.astI2cData[9].u32Data = 0x480;
            g_stSnsRegsInfo.u32RegNum = 10;
        }
        else
        {
            g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[8].u32RegAddr = 0x3176;
            g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[9].u32RegAddr = 0x3178;
            g_stSnsRegsInfo.astI2cData[10].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[10].u32RegAddr = 0x317A;
            g_stSnsRegsInfo.astI2cData[11].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[11].u32RegAddr = 0x317C;
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

int  ar0237_set_inifile_path(const char *pcPath)
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

HI_VOID ar0237_global_init()
{   
    gu8SensorImageMode = SENSOR_2M_1080p30_MODE;
    genSensorMode = WDR_MODE_NONE;       
    gu32FullLinesStd = FRAME_LINES_2M_1080p;
    gu32FullLines = FRAME_LINES_2M_1080p; 
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 
    
    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    
#ifdef INIFILE_CONFIG_MODE 
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = ar0237_LoadINIPara(pcName);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Cmos_LoadINIPara failed!!!!!!\n");
    }
#else

#endif

}

static void delay_ms(int ms) { 
    usleep(ms*1000);
}

void ar0237_reg_init()
{
	SENSOR_I2C_WRITE(0x301A, 0x0001); // RESET_REGISTER - [0:00:06.713]
    delay_ms(100);
    SENSOR_I2C_WRITE(0x301A, 0x10D8); // RESET_REGISTER - [0:00:06.743]
    SENSOR_I2C_WRITE(0x3088, 0x8000); // SEQ_CTRL_PORT - [0:00:25.262]
    SENSOR_I2C_WRITE(0x3086, 0x4558); // SEQ_DATA_PORT - [0:00:25.268]
    SENSOR_I2C_WRITE(0x3086, 0x72A6); // SEQ_DATA_PORT - [0:00:25.270]
    SENSOR_I2C_WRITE(0x3086, 0x4A31); // SEQ_DATA_PORT - [0:00:25.273]
    SENSOR_I2C_WRITE(0x3086, 0x4342); // SEQ_DATA_PORT - [0:00:25.276]
    SENSOR_I2C_WRITE(0x3086, 0x8E03); // SEQ_DATA_PORT - [0:00:25.279]
    SENSOR_I2C_WRITE(0x3086, 0x2A14); // SEQ_DATA_PORT - [0:00:25.282]
    SENSOR_I2C_WRITE(0x3086, 0x4578); // SEQ_DATA_PORT - [0:00:25.288]
    SENSOR_I2C_WRITE(0x3086, 0x7B3D); // SEQ_DATA_PORT - [0:00:25.291]
    SENSOR_I2C_WRITE(0x3086, 0xFF3D); // SEQ_DATA_PORT - [0:00:25.294]
    SENSOR_I2C_WRITE(0x3086, 0xFF3D); // SEQ_DATA_PORT - [0:00:25.297]
    SENSOR_I2C_WRITE(0x3086, 0xEA2A); // SEQ_DATA_PORT - [0:00:25.301]
    SENSOR_I2C_WRITE(0x3086, 0x043D); // SEQ_DATA_PORT - [0:00:25.303]
    SENSOR_I2C_WRITE(0x3086, 0x102A); // SEQ_DATA_PORT - [0:00:25.306]
    SENSOR_I2C_WRITE(0x3086, 0x052A); // SEQ_DATA_PORT - [0:00:25.310]
    SENSOR_I2C_WRITE(0x3086, 0x1535); // SEQ_DATA_PORT - [0:00:25.313]
    SENSOR_I2C_WRITE(0x3086, 0x2A05); // SEQ_DATA_PORT - [0:00:25.316]
    SENSOR_I2C_WRITE(0x3086, 0x3D10); // SEQ_DATA_PORT - [0:00:25.318]
    SENSOR_I2C_WRITE(0x3086, 0x4558); // SEQ_DATA_PORT - [0:00:25.321]
    SENSOR_I2C_WRITE(0x3086, 0x2A04); // SEQ_DATA_PORT - [0:00:25.325]
    SENSOR_I2C_WRITE(0x3086, 0x2A14); // SEQ_DATA_PORT - [0:00:25.328]
    SENSOR_I2C_WRITE(0x3086, 0x3DFF); // SEQ_DATA_PORT - [0:00:25.330]
    SENSOR_I2C_WRITE(0x3086, 0x3DFF); // SEQ_DATA_PORT - [0:00:25.332]
    SENSOR_I2C_WRITE(0x3086, 0x3DEA); // SEQ_DATA_PORT - [0:00:25.334]
    SENSOR_I2C_WRITE(0x3086, 0x2A04); // SEQ_DATA_PORT - [0:00:25.336]
    SENSOR_I2C_WRITE(0x3086, 0x622A); // SEQ_DATA_PORT - [0:00:25.339]
    SENSOR_I2C_WRITE(0x3086, 0x288E); // SEQ_DATA_PORT - [0:00:25.341]
    SENSOR_I2C_WRITE(0x3086, 0x0036); // SEQ_DATA_PORT - [0:00:25.343]
    SENSOR_I2C_WRITE(0x3086, 0x2A08); // SEQ_DATA_PORT - [0:00:25.345]
    SENSOR_I2C_WRITE(0x3086, 0x3D64); // SEQ_DATA_PORT - [0:00:25.348]
    SENSOR_I2C_WRITE(0x3086, 0x7A3D); // SEQ_DATA_PORT - [0:00:25.350]
    SENSOR_I2C_WRITE(0x3086, 0x0444); // SEQ_DATA_PORT - [0:00:25.352]
    SENSOR_I2C_WRITE(0x3086, 0x2C4B); // SEQ_DATA_PORT - [0:00:25.354]
    SENSOR_I2C_WRITE(0x3086, 0xA403); // SEQ_DATA_PORT - [0:00:25.357]
    SENSOR_I2C_WRITE(0x3086, 0x430D); // SEQ_DATA_PORT - [0:00:25.359]
    SENSOR_I2C_WRITE(0x3086, 0x2D46); // SEQ_DATA_PORT - [0:00:25.361]
    SENSOR_I2C_WRITE(0x3086, 0x4316); // SEQ_DATA_PORT - [0:00:25.363]
    SENSOR_I2C_WRITE(0x3086, 0x2A90); // SEQ_DATA_PORT - [0:00:25.366]
    SENSOR_I2C_WRITE(0x3086, 0x3E06); // SEQ_DATA_PORT - [0:00:25.368]
    SENSOR_I2C_WRITE(0x3086, 0x2A98); // SEQ_DATA_PORT - [0:00:25.370]
    SENSOR_I2C_WRITE(0x3086, 0x5F16); // SEQ_DATA_PORT - [0:00:25.372]
    SENSOR_I2C_WRITE(0x3086, 0x530D); // SEQ_DATA_PORT - [0:00:25.375]
    SENSOR_I2C_WRITE(0x3086, 0x1660); // SEQ_DATA_PORT - [0:00:25.377]
    SENSOR_I2C_WRITE(0x3086, 0x3E4C); // SEQ_DATA_PORT - [0:00:25.379]
    SENSOR_I2C_WRITE(0x3086, 0x2904); // SEQ_DATA_PORT - [0:00:25.381]
    SENSOR_I2C_WRITE(0x3086, 0x2984); // SEQ_DATA_PORT - [0:00:25.384]
    SENSOR_I2C_WRITE(0x3086, 0x8E03); // SEQ_DATA_PORT - [0:00:25.386]
    SENSOR_I2C_WRITE(0x3086, 0x2AFC); // SEQ_DATA_PORT - [0:00:25.388]
    SENSOR_I2C_WRITE(0x3086, 0x5C1D); // SEQ_DATA_PORT - [0:00:25.391]
    SENSOR_I2C_WRITE(0x3086, 0x5754); // SEQ_DATA_PORT - [0:00:25.393]
    SENSOR_I2C_WRITE(0x3086, 0x495F); // SEQ_DATA_PORT - [0:00:25.395]
    SENSOR_I2C_WRITE(0x3086, 0x5305); // SEQ_DATA_PORT - [0:00:25.397]
    SENSOR_I2C_WRITE(0x3086, 0x5307); // SEQ_DATA_PORT - [0:00:25.400]
    SENSOR_I2C_WRITE(0x3086, 0x4D2B); // SEQ_DATA_PORT - [0:00:25.402]
    SENSOR_I2C_WRITE(0x3086, 0xF810); // SEQ_DATA_PORT - [0:00:25.405]
    SENSOR_I2C_WRITE(0x3086, 0x164C); // SEQ_DATA_PORT - [0:00:25.407]
    SENSOR_I2C_WRITE(0x3086, 0x0955); // SEQ_DATA_PORT - [0:00:25.409]
    SENSOR_I2C_WRITE(0x3086, 0x562B); // SEQ_DATA_PORT - [0:00:25.412]
    SENSOR_I2C_WRITE(0x3086, 0xB82B); // SEQ_DATA_PORT - [0:00:25.414]
    SENSOR_I2C_WRITE(0x3086, 0x984E); // SEQ_DATA_PORT - [0:00:25.416]
    SENSOR_I2C_WRITE(0x3086, 0x1129); // SEQ_DATA_PORT - [0:00:25.419]
    SENSOR_I2C_WRITE(0x3086, 0x9460); // SEQ_DATA_PORT - [0:00:25.421]
    SENSOR_I2C_WRITE(0x3086, 0x5C19); // SEQ_DATA_PORT - [0:00:25.423]
    SENSOR_I2C_WRITE(0x3086, 0x5C1B); // SEQ_DATA_PORT - [0:00:25.425]
    SENSOR_I2C_WRITE(0x3086, 0x4548); // SEQ_DATA_PORT - [0:00:25.428]
    SENSOR_I2C_WRITE(0x3086, 0x4508); // SEQ_DATA_PORT - [0:00:25.430]
    SENSOR_I2C_WRITE(0x3086, 0x4588); // SEQ_DATA_PORT - [0:00:25.432]
    SENSOR_I2C_WRITE(0x3086, 0x29B6); // SEQ_DATA_PORT - [0:00:25.435]
    SENSOR_I2C_WRITE(0x3086, 0x8E01); // SEQ_DATA_PORT - [0:00:25.437]
    SENSOR_I2C_WRITE(0x3086, 0x2AF8); // SEQ_DATA_PORT - [0:00:25.439]
    SENSOR_I2C_WRITE(0x3086, 0x3E02); // SEQ_DATA_PORT - [0:00:25.442]
    SENSOR_I2C_WRITE(0x3086, 0x2AFA); // SEQ_DATA_PORT - [0:00:25.444]
    SENSOR_I2C_WRITE(0x3086, 0x3F09); // SEQ_DATA_PORT - [0:00:25.446]
    SENSOR_I2C_WRITE(0x3086, 0x5C1B); // SEQ_DATA_PORT - [0:00:25.448]
    SENSOR_I2C_WRITE(0x3086, 0x29B2); // SEQ_DATA_PORT - [0:00:25.451]
    SENSOR_I2C_WRITE(0x3086, 0x3F0C); // SEQ_DATA_PORT - [0:00:25.453]
    SENSOR_I2C_WRITE(0x3086, 0x3E03); // SEQ_DATA_PORT - [0:00:25.456]
    SENSOR_I2C_WRITE(0x3086, 0x3E15); // SEQ_DATA_PORT - [0:00:25.458]
    SENSOR_I2C_WRITE(0x3086, 0x5C13); // SEQ_DATA_PORT - [0:00:25.461]
    SENSOR_I2C_WRITE(0x3086, 0x3F11); // SEQ_DATA_PORT - [0:00:25.463]
    SENSOR_I2C_WRITE(0x3086, 0x3E0F); // SEQ_DATA_PORT - [0:00:25.466]
    SENSOR_I2C_WRITE(0x3086, 0x5F2B); // SEQ_DATA_PORT - [0:00:25.468]
    SENSOR_I2C_WRITE(0x3086, 0x902B); // SEQ_DATA_PORT - [0:00:25.470]
    SENSOR_I2C_WRITE(0x3086, 0x803E); // SEQ_DATA_PORT - [0:00:25.473]
    SENSOR_I2C_WRITE(0x3086, 0x062A); // SEQ_DATA_PORT - [0:00:25.475]
    SENSOR_I2C_WRITE(0x3086, 0xF23F); // SEQ_DATA_PORT - [0:00:25.477]
    SENSOR_I2C_WRITE(0x3086, 0x103E); // SEQ_DATA_PORT - [0:00:25.480]
    SENSOR_I2C_WRITE(0x3086, 0x0160); // SEQ_DATA_PORT - [0:00:25.482]
    SENSOR_I2C_WRITE(0x3086, 0x29A2); // SEQ_DATA_PORT - [0:00:25.484]
    SENSOR_I2C_WRITE(0x3086, 0x29A3); // SEQ_DATA_PORT - [0:00:25.486]
    SENSOR_I2C_WRITE(0x3086, 0x5F4D); // SEQ_DATA_PORT - [0:00:25.489]
    SENSOR_I2C_WRITE(0x3086, 0x1C2A); // SEQ_DATA_PORT - [0:00:25.491]
    SENSOR_I2C_WRITE(0x3086, 0xFA29); // SEQ_DATA_PORT - [0:00:25.493]
    SENSOR_I2C_WRITE(0x3086, 0x8345); // SEQ_DATA_PORT - [0:00:25.496]
    SENSOR_I2C_WRITE(0x3086, 0xA83E); // SEQ_DATA_PORT - [0:00:25.498]
    SENSOR_I2C_WRITE(0x3086, 0x072A); // SEQ_DATA_PORT - [0:00:25.500]
    SENSOR_I2C_WRITE(0x3086, 0xFB3E); // SEQ_DATA_PORT - [0:00:25.503]
    SENSOR_I2C_WRITE(0x3086, 0x6745); // SEQ_DATA_PORT - [0:00:25.505]
    SENSOR_I2C_WRITE(0x3086, 0x8824); // SEQ_DATA_PORT - [0:00:25.507]
    SENSOR_I2C_WRITE(0x3086, 0x3E08); // SEQ_DATA_PORT - [0:00:25.510]
    SENSOR_I2C_WRITE(0x3086, 0x2AFA); // SEQ_DATA_PORT - [0:00:25.512]
    SENSOR_I2C_WRITE(0x3086, 0x5D29); // SEQ_DATA_PORT - [0:00:25.515]
    SENSOR_I2C_WRITE(0x3086, 0x9288); // SEQ_DATA_PORT - [0:00:25.517]
    SENSOR_I2C_WRITE(0x3086, 0x102B); // SEQ_DATA_PORT - [0:00:25.519]
    SENSOR_I2C_WRITE(0x3086, 0x048B); // SEQ_DATA_PORT - [0:00:25.522]
    SENSOR_I2C_WRITE(0x3086, 0x1686); // SEQ_DATA_PORT - [0:00:25.524]
    SENSOR_I2C_WRITE(0x3086, 0x8D48); // SEQ_DATA_PORT - [0:00:25.527]
    SENSOR_I2C_WRITE(0x3086, 0x4D4E); // SEQ_DATA_PORT - [0:00:25.529]
    SENSOR_I2C_WRITE(0x3086, 0x2B80); // SEQ_DATA_PORT - [0:00:25.532]
    SENSOR_I2C_WRITE(0x3086, 0x4C0B); // SEQ_DATA_PORT - [0:00:25.534]
    SENSOR_I2C_WRITE(0x3086, 0x3F36); // SEQ_DATA_PORT - [0:00:25.537]
    SENSOR_I2C_WRITE(0x3086, 0x2AF2); // SEQ_DATA_PORT - [0:00:25.539]
    SENSOR_I2C_WRITE(0x3086, 0x3F10); // SEQ_DATA_PORT - [0:00:25.541]
    SENSOR_I2C_WRITE(0x3086, 0x3E01); // SEQ_DATA_PORT - [0:00:25.544]
    SENSOR_I2C_WRITE(0x3086, 0x6029); // SEQ_DATA_PORT - [0:00:25.546]
    SENSOR_I2C_WRITE(0x3086, 0x8229); // SEQ_DATA_PORT - [0:00:25.549]
    SENSOR_I2C_WRITE(0x3086, 0x8329); // SEQ_DATA_PORT - [0:00:25.551]
    SENSOR_I2C_WRITE(0x3086, 0x435C); // SEQ_DATA_PORT - [0:00:25.554]
    SENSOR_I2C_WRITE(0x3086, 0x155F); // SEQ_DATA_PORT - [0:00:25.556]
    SENSOR_I2C_WRITE(0x3086, 0x4D1C); // SEQ_DATA_PORT - [0:00:25.559]
    SENSOR_I2C_WRITE(0x3086, 0x2AFA); // SEQ_DATA_PORT - [0:00:25.561]
    SENSOR_I2C_WRITE(0x3086, 0x4558); // SEQ_DATA_PORT - [0:00:25.564]
    SENSOR_I2C_WRITE(0x3086, 0x8E00); // SEQ_DATA_PORT - [0:00:25.566]
    SENSOR_I2C_WRITE(0x3086, 0x2A98); // SEQ_DATA_PORT - [0:00:25.568]
    SENSOR_I2C_WRITE(0x3086, 0x3F0A); // SEQ_DATA_PORT - [0:00:25.571]
    SENSOR_I2C_WRITE(0x3086, 0x4A0A); // SEQ_DATA_PORT - [0:00:25.573]
    SENSOR_I2C_WRITE(0x3086, 0x4316); // SEQ_DATA_PORT - [0:00:25.576]
    SENSOR_I2C_WRITE(0x3086, 0x0B43); // SEQ_DATA_PORT - [0:00:25.578]
    SENSOR_I2C_WRITE(0x3086, 0x168E); // SEQ_DATA_PORT - [0:00:25.581]
    SENSOR_I2C_WRITE(0x3086, 0x032A); // SEQ_DATA_PORT - [0:00:25.583]
    SENSOR_I2C_WRITE(0x3086, 0x9C45); // SEQ_DATA_PORT - [0:00:25.586]
    SENSOR_I2C_WRITE(0x3086, 0x783F); // SEQ_DATA_PORT - [0:00:25.588]
    SENSOR_I2C_WRITE(0x3086, 0x072A); // SEQ_DATA_PORT - [0:00:25.591]
    SENSOR_I2C_WRITE(0x3086, 0x9D3E); // SEQ_DATA_PORT - [0:00:25.593]
    SENSOR_I2C_WRITE(0x3086, 0x305D); // SEQ_DATA_PORT - [0:00:25.596]
    SENSOR_I2C_WRITE(0x3086, 0x2944); // SEQ_DATA_PORT - [0:00:25.598]
    SENSOR_I2C_WRITE(0x3086, 0x8810); // SEQ_DATA_PORT - [0:00:25.601]
    SENSOR_I2C_WRITE(0x3086, 0x2B04); // SEQ_DATA_PORT - [0:00:25.603]
    SENSOR_I2C_WRITE(0x3086, 0x530D); // SEQ_DATA_PORT - [0:00:25.606]
    SENSOR_I2C_WRITE(0x3086, 0x4558); // SEQ_DATA_PORT - [0:00:25.608]
    SENSOR_I2C_WRITE(0x3086, 0x3E08); // SEQ_DATA_PORT - [0:00:25.611]
    SENSOR_I2C_WRITE(0x3086, 0x8E01); // SEQ_DATA_PORT - [0:00:25.613]
    SENSOR_I2C_WRITE(0x3086, 0x2A98); // SEQ_DATA_PORT - [0:00:25.616]
    SENSOR_I2C_WRITE(0x3086, 0x8E00); // SEQ_DATA_PORT - [0:00:25.618]
    SENSOR_I2C_WRITE(0x3086, 0x76A7); // SEQ_DATA_PORT - [0:00:25.621]
    SENSOR_I2C_WRITE(0x3086, 0x77A7); // SEQ_DATA_PORT - [0:00:25.623]
    SENSOR_I2C_WRITE(0x3086, 0x4644); // SEQ_DATA_PORT - [0:00:25.626]
    SENSOR_I2C_WRITE(0x3086, 0x1616); // SEQ_DATA_PORT - [0:00:25.628]
    SENSOR_I2C_WRITE(0x3086, 0xA57A); // SEQ_DATA_PORT - [0:00:25.631]
    SENSOR_I2C_WRITE(0x3086, 0x1244); // SEQ_DATA_PORT - [0:00:25.633]
    SENSOR_I2C_WRITE(0x3086, 0x4B18); // SEQ_DATA_PORT - [0:00:25.636]
    SENSOR_I2C_WRITE(0x3086, 0x4A04); // SEQ_DATA_PORT - [0:00:25.638]
    SENSOR_I2C_WRITE(0x3086, 0x4316); // SEQ_DATA_PORT - [0:00:25.640]
    SENSOR_I2C_WRITE(0x3086, 0x0643); // SEQ_DATA_PORT - [0:00:25.643]
    SENSOR_I2C_WRITE(0x3086, 0x1605); // SEQ_DATA_PORT - [0:00:25.645]
    SENSOR_I2C_WRITE(0x3086, 0x4316); // SEQ_DATA_PORT - [0:00:25.647]
    SENSOR_I2C_WRITE(0x3086, 0x0743); // SEQ_DATA_PORT - [0:00:25.649]
    SENSOR_I2C_WRITE(0x3086, 0x1658); // SEQ_DATA_PORT - [0:00:25.651]
    SENSOR_I2C_WRITE(0x3086, 0x4316); // SEQ_DATA_PORT - [0:00:25.654]
    SENSOR_I2C_WRITE(0x3086, 0x5A43); // SEQ_DATA_PORT - [0:00:25.656]
    SENSOR_I2C_WRITE(0x3086, 0x1645); // SEQ_DATA_PORT - [0:00:25.658]
    SENSOR_I2C_WRITE(0x3086, 0x588E); // SEQ_DATA_PORT - [0:00:25.660]
    SENSOR_I2C_WRITE(0x3086, 0x032A); // SEQ_DATA_PORT - [0:00:25.663]
    SENSOR_I2C_WRITE(0x3086, 0x9C45); // SEQ_DATA_PORT - [0:00:25.665]
    SENSOR_I2C_WRITE(0x3086, 0x787B); // SEQ_DATA_PORT - [0:00:25.667]
    SENSOR_I2C_WRITE(0x3086, 0x3F07); // SEQ_DATA_PORT - [0:00:25.669]
    SENSOR_I2C_WRITE(0x3086, 0x2A9D); // SEQ_DATA_PORT - [0:00:25.671]
    SENSOR_I2C_WRITE(0x3086, 0x530D); // SEQ_DATA_PORT - [0:00:25.674]
    SENSOR_I2C_WRITE(0x3086, 0x8B16); // SEQ_DATA_PORT - [0:00:25.676]
    SENSOR_I2C_WRITE(0x3086, 0x863E); // SEQ_DATA_PORT - [0:00:25.678]
    SENSOR_I2C_WRITE(0x3086, 0x2345); // SEQ_DATA_PORT - [0:00:25.680]
    SENSOR_I2C_WRITE(0x3086, 0x5825); // SEQ_DATA_PORT - [0:00:25.683]
    SENSOR_I2C_WRITE(0x3086, 0x3E10); // SEQ_DATA_PORT - [0:00:25.685]
    SENSOR_I2C_WRITE(0x3086, 0x8E01); // SEQ_DATA_PORT - [0:00:25.687]
    SENSOR_I2C_WRITE(0x3086, 0x2A98); // SEQ_DATA_PORT - [0:00:25.689]
    SENSOR_I2C_WRITE(0x3086, 0x8E00); // SEQ_DATA_PORT - [0:00:25.692]
    SENSOR_I2C_WRITE(0x3086, 0x3E10); // SEQ_DATA_PORT - [0:00:25.694]
    SENSOR_I2C_WRITE(0x3086, 0x8D60); // SEQ_DATA_PORT - [0:00:25.696]
    SENSOR_I2C_WRITE(0x3086, 0x1244); // SEQ_DATA_PORT - [0:00:25.698]
    SENSOR_I2C_WRITE(0x3086, 0x4BB9); // SEQ_DATA_PORT - [0:00:25.701]
    SENSOR_I2C_WRITE(0x3086, 0x2C2C); // SEQ_DATA_PORT - [0:00:25.703]
    SENSOR_I2C_WRITE(0x3086, 0x2C2C); // SEQ_DATA_PORT - [0:00:25.705]
    SENSOR_I2C_WRITE(0x301A, 0x10D8); // RESET_REGISTER - [0:00:25.710]
    SENSOR_I2C_WRITE(0x30B0, 0x1A38); // DIGITAL_TEST - [0:00:25.715]
    SENSOR_I2C_WRITE(0x31AC, 0x0C0C); // DATA_FORMAT_BITS - [0:00:25.717]
    SENSOR_I2C_WRITE(0x302A, 0x0008); // VT_PIX_CLK_DIV - [0:00:25.723]
    SENSOR_I2C_WRITE(0x302C, 0x0001); // VT_SYS_CLK_DIV - [0:00:25.728]
    SENSOR_I2C_WRITE(0x302E, 0x0002); // PRE_PLL_CLK_DIV - [0:00:25.744]
    SENSOR_I2C_WRITE(0x3030, 0x002C); // PLL_MULTIPLIER - [0:00:25.755]
    SENSOR_I2C_WRITE(0x3036, 0x000C); // OP_PIX_CLK_DIV - [0:00:25.760]
    SENSOR_I2C_WRITE(0x3038, 0x0001); // OP_SYS_CLK_DIV - [0:00:25.766]
    SENSOR_I2C_WRITE(0x3002, 0x0000); // Y_ADDR_START - [0:00:25.771]
    SENSOR_I2C_WRITE(0x3004, 0x0000); // X_ADDR_START - [0:00:25.803]
    SENSOR_I2C_WRITE(0x3006, 0x043F); // Y_ADDR_END - [0:00:25.812]
    SENSOR_I2C_WRITE(0x3008, 0x0787); // X_ADDR_END - [0:00:25.820]
    SENSOR_I2C_WRITE(0x300A, 0x0452); // FRAME_LENGTH_LINES - [0:00:25.825]
    SENSOR_I2C_WRITE(0x300C, 0x045E); // LINE_LENGTH_PCK - [0:00:25.831]
    SENSOR_I2C_WRITE(0x3012, 0x0416); // COARSE_INTEGRATION_TIME - [0:00:25.836]
    SENSOR_I2C_WRITE(0x30A2, 0x0001); // X_ODD_INC - [0:00:25.838]
    SENSOR_I2C_WRITE(0x30A6, 0x0001); // Y_ODD_INC - [0:00:25.841]
    SENSOR_I2C_WRITE(0x30AE, 0x0001); // X_ODD_INC_CB - [0:00:25.852]
    SENSOR_I2C_WRITE(0x30A8, 0x0001); // Y_ODD_INC_CB - [0:00:25.857]
    SENSOR_I2C_WRITE(0x3040, 0x0000); // READ_MODE - [0:00:25.869]
    SENSOR_I2C_WRITE(0x31AE, 0x0301); // SERIAL_FORMAT - [0:00:25.885]
    SENSOR_I2C_WRITE(0x3082, 0x0009); // OPERATION_MODE_CTRL - [0:00:25.889]
    SENSOR_I2C_WRITE(0x30BA, 0x760C); // DIGITAL_CTRL - [0:00:25.894]
    SENSOR_I2C_WRITE(0x3100, 0x0000); // AECTRLREG - [0:00:25.900]
    SENSOR_I2C_WRITE(0x3060, 0x000B); // GAIN - [0:00:25.900]
    SENSOR_I2C_WRITE(0x31D0, 0x0000); // COMPANDING - [0:00:25.926]
    SENSOR_I2C_WRITE(0x3064, 0x1802); // SMIA_TEST - [0:00:25.970]
    SENSOR_I2C_WRITE(0x3EEE, 0xA0AA); // DAC_LD_34_35 - [0:00:25.975]
    SENSOR_I2C_WRITE(0x30BA, 0x762C); // DIGITAL_CTRL - [0:00:25.977]
    SENSOR_I2C_WRITE(0x3F4A, 0x0F70); // DELTA_DK_PIX_THRES - [0:00:25.983]
    SENSOR_I2C_WRITE(0x309E, 0x016C); // HIDY_PROG_START_ADDR - [0:00:25.988]
    SENSOR_I2C_WRITE(0x3092, 0x006F); // ROW_NOISE_CONTROL - [0:00:25.994]
    SENSOR_I2C_WRITE(0x3EE4, 0x9937); // DAC_LD_24_25 - [0:00:25.999]
    SENSOR_I2C_WRITE(0x3EE6, 0x3863); // DAC_LD_26_27 - [0:00:26.004]
    SENSOR_I2C_WRITE(0x3EEC, 0x3B0C); // DAC_LD_32_33 - [0:00:26.009]
    SENSOR_I2C_WRITE(0x30B0, 0x1A3A); // DIGITAL_TEST - [0:00:26.015]
    SENSOR_I2C_WRITE(0x30B0, 0x1A3A); // DIGITAL_TEST - [0:00:26.020]
    SENSOR_I2C_WRITE(0x30BA, 0x762C); // DIGITAL_CTRL - [0:00:26.025]
    SENSOR_I2C_WRITE(0x30B0, 0x1A3A); // DIGITAL_TEST - [0:00:26.031]
    SENSOR_I2C_WRITE(0x30B0, 0x0A3A); // DIGITAL_TEST - [0:00:26.036]
    SENSOR_I2C_WRITE(0x3EEA, 0x2838); // DAC_LD_30_31 - [0:00:26.041]
    SENSOR_I2C_WRITE(0x3ECC, 0x4E2D); // DAC_LD_0_1 - [0:00:26.052]
    SENSOR_I2C_WRITE(0x3ED2, 0xFEA6); // DAC_LD_6_7 - [0:00:26.055]
    SENSOR_I2C_WRITE(0x3ED6, 0x2CB3); // DAC_LD_10_11 - [0:00:26.060]
    SENSOR_I2C_WRITE(0x3EEA, 0x2819); // DAC_LD_30_31 - [0:00:26.062]
    SENSOR_I2C_WRITE(0x30B0, 0x1A3A); // DIGITAL_TEST - [0:00:26.068]
    SENSOR_I2C_WRITE(0x306E, 0xfD18); // DATAPATH_SELECT - [0:00:26.073]//0x2d18//0x2418 0x2D18 0xfD18
    SENSOR_I2C_WRITE(0x301A, 0x10DC); // RESET_REGISTER - [0:00:26.076]
 
    printf("===================================================================\n");
    printf("====Aptina AR0237 sensor linear DVP 2M-1080p 25fps init success====\n");
    printf("===================================================================\n");
 
    bSensorInit = HI_TRUE;

}
void ar0237_wdr_1080p30_init()
{
    // ---------------------hdr start-------------------------

    //[HiSPi HDR 1080p30 - 4 Lane_ALTM on]
    // Reset
    SENSOR_I2C_WRITE( 0x301A, 0x0001);     // RESET_REGISTER
    delay_ms( 200);
    SENSOR_I2C_WRITE( 0x301A, 0x10D8);     // RESET_REGISTER


    //LOAD = HDR Mode Sequencer - Rev1.2

    //[HDR Mode Sequencer - Rev1.2]
    //$Revision: 40442 $
    SENSOR_I2C_WRITE(0x301A, 0x0059 );
    delay_ms( 200);
    SENSOR_I2C_WRITE(0x3088, 0x8000 );
    SENSOR_I2C_WRITE(0x3086, 0x4558 );
    SENSOR_I2C_WRITE(0x3086, 0x729B );
    SENSOR_I2C_WRITE(0x3086, 0x4A31 );
    SENSOR_I2C_WRITE(0x3086, 0x4342 );
    SENSOR_I2C_WRITE(0x3086, 0x8E03 );
    SENSOR_I2C_WRITE(0x3086, 0x2A14 );
    SENSOR_I2C_WRITE(0x3086, 0x4578 );
    SENSOR_I2C_WRITE(0x3086, 0x7B3D );
    SENSOR_I2C_WRITE(0x3086, 0xFF3D );
    SENSOR_I2C_WRITE(0x3086, 0xFF3D );
    SENSOR_I2C_WRITE(0x3086, 0xEA2A );
    SENSOR_I2C_WRITE(0x3086, 0x043D );
    SENSOR_I2C_WRITE(0x3086, 0x102A );
    SENSOR_I2C_WRITE(0x3086, 0x052A );
    SENSOR_I2C_WRITE(0x3086, 0x1535 );
    SENSOR_I2C_WRITE(0x3086, 0x2A05 );
    SENSOR_I2C_WRITE(0x3086, 0x3D10 );
    SENSOR_I2C_WRITE(0x3086, 0x4558 );
    SENSOR_I2C_WRITE(0x3086, 0x2A04 );
    SENSOR_I2C_WRITE(0x3086, 0x2A14 );
    SENSOR_I2C_WRITE(0x3086, 0x3DFF );
    SENSOR_I2C_WRITE(0x3086, 0x3DFF );
    SENSOR_I2C_WRITE(0x3086, 0x3DEA );
    SENSOR_I2C_WRITE(0x3086, 0x2A04 );
    SENSOR_I2C_WRITE(0x3086, 0x622A );
    SENSOR_I2C_WRITE(0x3086, 0x288E );
    SENSOR_I2C_WRITE(0x3086, 0x0036 );
    SENSOR_I2C_WRITE(0x3086, 0x2A08 );
    SENSOR_I2C_WRITE(0x3086, 0x3D64 );
    SENSOR_I2C_WRITE(0x3086, 0x7A3D );
    SENSOR_I2C_WRITE(0x3086, 0x0444 );
    SENSOR_I2C_WRITE(0x3086, 0x2C4B );
    SENSOR_I2C_WRITE(0x3086, 0x8F00 );
    SENSOR_I2C_WRITE(0x3086, 0x430C );
    SENSOR_I2C_WRITE(0x3086, 0x2D63 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x8E03 );
    SENSOR_I2C_WRITE(0x3086, 0x2AFC );
    SENSOR_I2C_WRITE(0x3086, 0x5C1D );
    SENSOR_I2C_WRITE(0x3086, 0x5754 );
    SENSOR_I2C_WRITE(0x3086, 0x495F );
    SENSOR_I2C_WRITE(0x3086, 0x5305 );
    SENSOR_I2C_WRITE(0x3086, 0x5307 );
    SENSOR_I2C_WRITE(0x3086, 0x4D2B );
    SENSOR_I2C_WRITE(0x3086, 0xF810 );
    SENSOR_I2C_WRITE(0x3086, 0x164C );
    SENSOR_I2C_WRITE(0x3086, 0x0855 );
    SENSOR_I2C_WRITE(0x3086, 0x562B );
    SENSOR_I2C_WRITE(0x3086, 0xB82B );
    SENSOR_I2C_WRITE(0x3086, 0x984E );
    SENSOR_I2C_WRITE(0x3086, 0x1129 );
    SENSOR_I2C_WRITE(0x3086, 0x0429 );
    SENSOR_I2C_WRITE(0x3086, 0x8429 );
    SENSOR_I2C_WRITE(0x3086, 0x9460 );
    SENSOR_I2C_WRITE(0x3086, 0x5C19 );
    SENSOR_I2C_WRITE(0x3086, 0x5C1B );
    SENSOR_I2C_WRITE(0x3086, 0x4548 );
    SENSOR_I2C_WRITE(0x3086, 0x4508 );
    SENSOR_I2C_WRITE(0x3086, 0x4588 );
    SENSOR_I2C_WRITE(0x3086, 0x29B6 );
    SENSOR_I2C_WRITE(0x3086, 0x8E01 );
    SENSOR_I2C_WRITE(0x3086, 0x2AF8 );
    SENSOR_I2C_WRITE(0x3086, 0x3E02 );
    SENSOR_I2C_WRITE(0x3086, 0x2AFA );
    SENSOR_I2C_WRITE(0x3086, 0x3F09 );
    SENSOR_I2C_WRITE(0x3086, 0x5C1B );
    SENSOR_I2C_WRITE(0x3086, 0x29B2 );
    SENSOR_I2C_WRITE(0x3086, 0x3F0C );
    SENSOR_I2C_WRITE(0x3086, 0x3E02 );
    SENSOR_I2C_WRITE(0x3086, 0x3E13 );
    SENSOR_I2C_WRITE(0x3086, 0x5C13 );
    SENSOR_I2C_WRITE(0x3086, 0x3F11 );
    SENSOR_I2C_WRITE(0x3086, 0x3E0B );
    SENSOR_I2C_WRITE(0x3086, 0x5F2B );
    SENSOR_I2C_WRITE(0x3086, 0x902A );
    SENSOR_I2C_WRITE(0x3086, 0xF22B );
    SENSOR_I2C_WRITE(0x3086, 0x803E );
    SENSOR_I2C_WRITE(0x3086, 0x043F );
    SENSOR_I2C_WRITE(0x3086, 0x0660 );
    SENSOR_I2C_WRITE(0x3086, 0x29A2 );
    SENSOR_I2C_WRITE(0x3086, 0x29A3 );
    SENSOR_I2C_WRITE(0x3086, 0x5F4D );
    SENSOR_I2C_WRITE(0x3086, 0x192A );
    SENSOR_I2C_WRITE(0x3086, 0xFA29 );
    SENSOR_I2C_WRITE(0x3086, 0x8345 );
    SENSOR_I2C_WRITE(0x3086, 0xA83E );
    SENSOR_I2C_WRITE(0x3086, 0x072A );
    SENSOR_I2C_WRITE(0x3086, 0xFB3E );
    SENSOR_I2C_WRITE(0x3086, 0x2945 );
    SENSOR_I2C_WRITE(0x3086, 0x8821 );
    SENSOR_I2C_WRITE(0x3086, 0x3E08 );
    SENSOR_I2C_WRITE(0x3086, 0x2AFA );
    SENSOR_I2C_WRITE(0x3086, 0x5D29 );
    SENSOR_I2C_WRITE(0x3086, 0x9288 );
    SENSOR_I2C_WRITE(0x3086, 0x102B );
    SENSOR_I2C_WRITE(0x3086, 0x048B );
    SENSOR_I2C_WRITE(0x3086, 0x1685 );
    SENSOR_I2C_WRITE(0x3086, 0x8D48 );
    SENSOR_I2C_WRITE(0x3086, 0x4D4E );
    SENSOR_I2C_WRITE(0x3086, 0x2B80 );
    SENSOR_I2C_WRITE(0x3086, 0x4C0B );
    SENSOR_I2C_WRITE(0x3086, 0x603F );
    SENSOR_I2C_WRITE(0x3086, 0x282A );
    SENSOR_I2C_WRITE(0x3086, 0xF23F );
    SENSOR_I2C_WRITE(0x3086, 0x0F29 );
    SENSOR_I2C_WRITE(0x3086, 0x8229 );
    SENSOR_I2C_WRITE(0x3086, 0x8329 );
    SENSOR_I2C_WRITE(0x3086, 0x435C );
    SENSOR_I2C_WRITE(0x3086, 0x155F );
    SENSOR_I2C_WRITE(0x3086, 0x4D19 );
    SENSOR_I2C_WRITE(0x3086, 0x2AFA );
    SENSOR_I2C_WRITE(0x3086, 0x4558 );
    SENSOR_I2C_WRITE(0x3086, 0x8E00 );
    SENSOR_I2C_WRITE(0x3086, 0x2A98 );
    SENSOR_I2C_WRITE(0x3086, 0x3F06 );
    SENSOR_I2C_WRITE(0x3086, 0x1244 );
    SENSOR_I2C_WRITE(0x3086, 0x4A04 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x0543 );
    SENSOR_I2C_WRITE(0x3086, 0x1658 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x5A43 );
    SENSOR_I2C_WRITE(0x3086, 0x1606 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x0743 );
    SENSOR_I2C_WRITE(0x3086, 0x168E );
    SENSOR_I2C_WRITE(0x3086, 0x032A );
    SENSOR_I2C_WRITE(0x3086, 0x9C45 );
    SENSOR_I2C_WRITE(0x3086, 0x787B );
    SENSOR_I2C_WRITE(0x3086, 0x3F07 );
    SENSOR_I2C_WRITE(0x3086, 0x2A9D );
    SENSOR_I2C_WRITE(0x3086, 0x3E2E );
    SENSOR_I2C_WRITE(0x3086, 0x4558 );
    SENSOR_I2C_WRITE(0x3086, 0x253E );
    SENSOR_I2C_WRITE(0x3086, 0x068E );
    SENSOR_I2C_WRITE(0x3086, 0x012A );
    SENSOR_I2C_WRITE(0x3086, 0x988E );
    SENSOR_I2C_WRITE(0x3086, 0x0012 );
    SENSOR_I2C_WRITE(0x3086, 0x444B );
    SENSOR_I2C_WRITE(0x3086, 0x0343 );
    SENSOR_I2C_WRITE(0x3086, 0x2D46 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0xA343 );
    SENSOR_I2C_WRITE(0x3086, 0x165D );
    SENSOR_I2C_WRITE(0x3086, 0x0D29 );
    SENSOR_I2C_WRITE(0x3086, 0x4488 );
    SENSOR_I2C_WRITE(0x3086, 0x102B );
    SENSOR_I2C_WRITE(0x3086, 0x0453 );
    SENSOR_I2C_WRITE(0x3086, 0x0D8B );
    SENSOR_I2C_WRITE(0x3086, 0x1685 );
    SENSOR_I2C_WRITE(0x3086, 0x448E );
    SENSOR_I2C_WRITE(0x3086, 0x032A );
    SENSOR_I2C_WRITE(0x3086, 0xFC5C );
    SENSOR_I2C_WRITE(0x3086, 0x1D8D );
    SENSOR_I2C_WRITE(0x3086, 0x6057 );
    SENSOR_I2C_WRITE(0x3086, 0x5449 );
    SENSOR_I2C_WRITE(0x3086, 0x5F53 );
    SENSOR_I2C_WRITE(0x3086, 0x0553 );
    SENSOR_I2C_WRITE(0x3086, 0x074D );
    SENSOR_I2C_WRITE(0x3086, 0x2BF8 );
    SENSOR_I2C_WRITE(0x3086, 0x1016 );
    SENSOR_I2C_WRITE(0x3086, 0x4C08 );
    SENSOR_I2C_WRITE(0x3086, 0x5556 );
    SENSOR_I2C_WRITE(0x3086, 0x2BB8 );
    SENSOR_I2C_WRITE(0x3086, 0x2B98 );
    SENSOR_I2C_WRITE(0x3086, 0x4E11 );
    SENSOR_I2C_WRITE(0x3086, 0x2904 );
    SENSOR_I2C_WRITE(0x3086, 0x2984 );
    SENSOR_I2C_WRITE(0x3086, 0x2994 );
    SENSOR_I2C_WRITE(0x3086, 0x605C );
    SENSOR_I2C_WRITE(0x3086, 0x195C );
    SENSOR_I2C_WRITE(0x3086, 0x1B45 );
    SENSOR_I2C_WRITE(0x3086, 0x4845 );
    SENSOR_I2C_WRITE(0x3086, 0x0845 );
    SENSOR_I2C_WRITE(0x3086, 0x8829 );
    SENSOR_I2C_WRITE(0x3086, 0xB68E );
    SENSOR_I2C_WRITE(0x3086, 0x012A );
    SENSOR_I2C_WRITE(0x3086, 0xF83E );
    SENSOR_I2C_WRITE(0x3086, 0x022A );
    SENSOR_I2C_WRITE(0x3086, 0xFA3F );
    SENSOR_I2C_WRITE(0x3086, 0x095C );
    SENSOR_I2C_WRITE(0x3086, 0x1B29 );
    SENSOR_I2C_WRITE(0x3086, 0xB23F );
    SENSOR_I2C_WRITE(0x3086, 0x0C3E );
    SENSOR_I2C_WRITE(0x3086, 0x023E );
    SENSOR_I2C_WRITE(0x3086, 0x135C );
    SENSOR_I2C_WRITE(0x3086, 0x133F );
    SENSOR_I2C_WRITE(0x3086, 0x113E );
    SENSOR_I2C_WRITE(0x3086, 0x0B5F );
    SENSOR_I2C_WRITE(0x3086, 0x2B90 );
    SENSOR_I2C_WRITE(0x3086, 0x2AF2 );
    SENSOR_I2C_WRITE(0x3086, 0x2B80 );
    SENSOR_I2C_WRITE(0x3086, 0x3E04 );
    SENSOR_I2C_WRITE(0x3086, 0x3F06 );
    SENSOR_I2C_WRITE(0x3086, 0x6029 );
    SENSOR_I2C_WRITE(0x3086, 0xA229 );
    SENSOR_I2C_WRITE(0x3086, 0xA35F );
    SENSOR_I2C_WRITE(0x3086, 0x4D1C );
    SENSOR_I2C_WRITE(0x3086, 0x2AFA );
    SENSOR_I2C_WRITE(0x3086, 0x2983 );
    SENSOR_I2C_WRITE(0x3086, 0x45A8 );
    SENSOR_I2C_WRITE(0x3086, 0x3E07 );
    SENSOR_I2C_WRITE(0x3086, 0x2AFB );
    SENSOR_I2C_WRITE(0x3086, 0x3E29 );
    SENSOR_I2C_WRITE(0x3086, 0x4588 );
    SENSOR_I2C_WRITE(0x3086, 0x243E );
    SENSOR_I2C_WRITE(0x3086, 0x082A );
    SENSOR_I2C_WRITE(0x3086, 0xFA5D );
    SENSOR_I2C_WRITE(0x3086, 0x2992 );
    SENSOR_I2C_WRITE(0x3086, 0x8810 );
    SENSOR_I2C_WRITE(0x3086, 0x2B04 );
    SENSOR_I2C_WRITE(0x3086, 0x8B16 );
    SENSOR_I2C_WRITE(0x3086, 0x868D );
    SENSOR_I2C_WRITE(0x3086, 0x484D );
    SENSOR_I2C_WRITE(0x3086, 0x4E2B );
    SENSOR_I2C_WRITE(0x3086, 0x804C );
    SENSOR_I2C_WRITE(0x3086, 0x0B60 );
    SENSOR_I2C_WRITE(0x3086, 0x3F28 );
    SENSOR_I2C_WRITE(0x3086, 0x2AF2 );
    SENSOR_I2C_WRITE(0x3086, 0x3F0F );
    SENSOR_I2C_WRITE(0x3086, 0x2982 );
    SENSOR_I2C_WRITE(0x3086, 0x2983 );
    SENSOR_I2C_WRITE(0x3086, 0x2943 );
    SENSOR_I2C_WRITE(0x3086, 0x5C15 );
    SENSOR_I2C_WRITE(0x3086, 0x5F4D );
    SENSOR_I2C_WRITE(0x3086, 0x1C2A );
    SENSOR_I2C_WRITE(0x3086, 0xFA45 );
    SENSOR_I2C_WRITE(0x3086, 0x588E );
    SENSOR_I2C_WRITE(0x3086, 0x002A );
    SENSOR_I2C_WRITE(0x3086, 0x983F );
    SENSOR_I2C_WRITE(0x3086, 0x064A );
    SENSOR_I2C_WRITE(0x3086, 0x739D );
    SENSOR_I2C_WRITE(0x3086, 0x0A43 );
    SENSOR_I2C_WRITE(0x3086, 0x160B );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x8E03 );
    SENSOR_I2C_WRITE(0x3086, 0x2A9C );
    SENSOR_I2C_WRITE(0x3086, 0x4578 );
    SENSOR_I2C_WRITE(0x3086, 0x3F07 );
    SENSOR_I2C_WRITE(0x3086, 0x2A9D );
    SENSOR_I2C_WRITE(0x3086, 0x3E12 );
    SENSOR_I2C_WRITE(0x3086, 0x4558 );
    SENSOR_I2C_WRITE(0x3086, 0x3F04 );
    SENSOR_I2C_WRITE(0x3086, 0x8E01 );
    SENSOR_I2C_WRITE(0x3086, 0x2A98 );
    SENSOR_I2C_WRITE(0x3086, 0x8E00 );
    SENSOR_I2C_WRITE(0x3086, 0x9176 );
    SENSOR_I2C_WRITE(0x3086, 0x9C77 );
    SENSOR_I2C_WRITE(0x3086, 0x9C46 );
    SENSOR_I2C_WRITE(0x3086, 0x4416 );
    SENSOR_I2C_WRITE(0x3086, 0x1690 );
    SENSOR_I2C_WRITE(0x3086, 0x7A12 );
    SENSOR_I2C_WRITE(0x3086, 0x444B );
    SENSOR_I2C_WRITE(0x3086, 0x4A00 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x6343 );
    SENSOR_I2C_WRITE(0x3086, 0x1608 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x5043 );
    SENSOR_I2C_WRITE(0x3086, 0x1665 );
    SENSOR_I2C_WRITE(0x3086, 0x4316 );
    SENSOR_I2C_WRITE(0x3086, 0x6643 );
    SENSOR_I2C_WRITE(0x3086, 0x168E );
    SENSOR_I2C_WRITE(0x3086, 0x032A );
    SENSOR_I2C_WRITE(0x3086, 0x9C45 );
    SENSOR_I2C_WRITE(0x3086, 0x783F );
    SENSOR_I2C_WRITE(0x3086, 0x072A );
    SENSOR_I2C_WRITE(0x3086, 0x9D5D );
    SENSOR_I2C_WRITE(0x3086, 0x0C29 );
    SENSOR_I2C_WRITE(0x3086, 0x4488 );
    SENSOR_I2C_WRITE(0x3086, 0x102B );
    SENSOR_I2C_WRITE(0x3086, 0x0453 );
    SENSOR_I2C_WRITE(0x3086, 0x0D8B );
    SENSOR_I2C_WRITE(0x3086, 0x1686 );
    SENSOR_I2C_WRITE(0x3086, 0x3E1F );
    SENSOR_I2C_WRITE(0x3086, 0x4558 );
    SENSOR_I2C_WRITE(0x3086, 0x283E );
    SENSOR_I2C_WRITE(0x3086, 0x068E );
    SENSOR_I2C_WRITE(0x3086, 0x012A );
    SENSOR_I2C_WRITE(0x3086, 0x988E );
    SENSOR_I2C_WRITE(0x3086, 0x008D );
    SENSOR_I2C_WRITE(0x3086, 0x6012 );
    SENSOR_I2C_WRITE(0x3086, 0x444B );
    SENSOR_I2C_WRITE(0x3086, 0x2C2C );
    SENSOR_I2C_WRITE(0x3086, 0x2C2C );


    //LOAD= AR0230 REV1.2 Optimized Settings

    //[AR0230 REV1.2 Optimized Settings]
    //$Revision: 40442 $
    SENSOR_I2C_WRITE( 0x2436, 0x000E );  
    SENSOR_I2C_WRITE( 0x320C, 0x0180 );                  
    SENSOR_I2C_WRITE( 0x320E, 0x0300 );                       
    SENSOR_I2C_WRITE( 0x3210, 0x0500 );                        
    SENSOR_I2C_WRITE( 0x3204, 0x0B6D );
    SENSOR_I2C_WRITE( 0x30FE, 0x0080 );                             
    SENSOR_I2C_WRITE( 0x3ED8, 0x7B99 );
    SENSOR_I2C_WRITE( 0x3EDC, 0x9BA8 );
    SENSOR_I2C_WRITE( 0x3EDA, 0x9B9B );
    SENSOR_I2C_WRITE( 0x3092, 0x006F );
    SENSOR_I2C_WRITE( 0x3EEC, 0x1C04 );
    SENSOR_I2C_WRITE( 0x30BA, 0x779C );
    SENSOR_I2C_WRITE( 0x3EF6, 0xA70F );
    SENSOR_I2C_WRITE( 0x3044, 0x0410 );
    SENSOR_I2C_WRITE( 0x3ED0, 0xFF44 );
    SENSOR_I2C_WRITE( 0x3ED4, 0x031F );
    SENSOR_I2C_WRITE( 0x30FE, 0x0080 );
    SENSOR_I2C_WRITE( 0x3EE2, 0x8866 );
    SENSOR_I2C_WRITE( 0x3EE4, 0x6623 );
    SENSOR_I2C_WRITE( 0x3EE6, 0x2263 );
    SENSOR_I2C_WRITE( 0x30E0, 0x4283 );
    SENSOR_I2C_WRITE( 0x30F0, 0x1283 );


    SENSOR_I2C_WRITE( 0x301A, 0x0058 );//RESET_REGISTER
    SENSOR_I2C_WRITE( 0x30B0, 0x0118 );
    SENSOR_I2C_WRITE( 0x31AC, 0x100C );

    //PLL_settings - 4 Lane 12-bit HiSPi
    //MCLK=27Mhz PCLK=74.25Mhz
    SENSOR_I2C_WRITE( 0x302A, 0x0006 );    
    SENSOR_I2C_WRITE( 0x302C, 0x0001 );    
    SENSOR_I2C_WRITE( 0x302E, 0x0004 );    
    SENSOR_I2C_WRITE( 0x3030, 0x0042 );    
    SENSOR_I2C_WRITE( 0x3036, 0x000C );    
    SENSOR_I2C_WRITE( 0x3038, 0x0001 );    


    //Sensor output setup
    SENSOR_I2C_WRITE( 0x3002, 0x0000 );
    SENSOR_I2C_WRITE( 0x3004, 0x0000 );
    SENSOR_I2C_WRITE( 0x3006, 0x0437 );
    SENSOR_I2C_WRITE( 0x3008, 0x0787 );
    SENSOR_I2C_WRITE( 0x300A, 0x0465 );//FRAME_LENGTH_LINES 1125
    SENSOR_I2C_WRITE( 0x300C, 0x0898 );//LINE_LENGTH_PCK 2200
    SENSOR_I2C_WRITE( 0x3012, 0x0416 );//COARSE_INTEGRATION_TIME
    SENSOR_I2C_WRITE( 0x30A2, 0x0001 );
    SENSOR_I2C_WRITE( 0x30A6, 0x0001 ); 
    SENSOR_I2C_WRITE( 0x3040, 0x0000 );

    //HDR Mode 16x Setup
    SENSOR_I2C_WRITE( 0x3082, 0x0008 );
    SENSOR_I2C_WRITE( 0x31E0, 0x0200 );

#if 1
    // ALTM Disabled
    SENSOR_I2C_WRITE(0x2400, 0x0003);     
    SENSOR_I2C_WRITE(0x301E, 0x00A8);     
    SENSOR_I2C_WRITE(0x2450, 0x0000);     
    SENSOR_I2C_WRITE(0x320A, 0x0080);     
    SENSOR_I2C_WRITE(0x31D0, 0x0001); 

#else
    //LOAD= ALTM Enabled
    //[ALTM Enabled]
    //$Revision: 40442 $

    SENSOR_I2C_WRITE( 0x2420, 0x0000 );
    SENSOR_I2C_WRITE( 0x2440, 0x0004 );
    SENSOR_I2C_WRITE( 0x2442, 0x0080 );
    SENSOR_I2C_WRITE( 0x301E, 0x0000 );
    SENSOR_I2C_WRITE( 0x2450, 0x0000 );
    SENSOR_I2C_WRITE( 0x320A, 0x0080 );
    SENSOR_I2C_WRITE( 0x31D0, 0x0000 );
    SENSOR_I2C_WRITE( 0x2400, 0x0002 );
    SENSOR_I2C_WRITE( 0x2410, 0x0005 );
    SENSOR_I2C_WRITE( 0x2412, 0x002D );
    SENSOR_I2C_WRITE( 0x2444, 0xF400 );
    SENSOR_I2C_WRITE( 0x2446, 0x0001 );
    SENSOR_I2C_WRITE( 0x2438, 0x0010 ); 
    SENSOR_I2C_WRITE( 0x243A, 0x0012 );  
    SENSOR_I2C_WRITE( 0x243C, 0xFFFF );  
    SENSOR_I2C_WRITE( 0x243E, 0x0100 );
#endif    

    //LOAD= Motion Compensation On
    //[Motion Compensation On]
    SENSOR_I2C_WRITE( 0x3190, 0x0000 );//DLO disabled
    SENSOR_I2C_WRITE( 0x318A, 0x0E74 );//  
    SENSOR_I2C_WRITE( 0x318C, 0xC000 );//
    SENSOR_I2C_WRITE( 0x3192, 0x0400 );//
    SENSOR_I2C_WRITE( 0x3198, 0x2050 );//modified at 20150407, prev value 0x183c
    //LOAD= HDR Mode Low Conversion Gain
    //[HDR Mode Low Conversion Gain]
    SENSOR_I2C_WRITE( 0x3060, 0x000B ); //ANALOG_GAIN 1.5x Minimum analog Gain for LCG
    SENSOR_I2C_WRITE( 0x3096, 0x0480 ); 
    SENSOR_I2C_WRITE( 0x3098, 0x0480 ); 
    SENSOR_I2C_WRITE( 0x3206, 0x0B08 ); 
    SENSOR_I2C_WRITE( 0x3208, 0x1E13 );
    SENSOR_I2C_WRITE( 0x3202, 0x0080 ); 
    SENSOR_I2C_WRITE( 0x3200, 0x0002 ); 
    SENSOR_I2C_WRITE( 0x3100, 0x0000 );
    //LOAD= HDR Mode High Conversion Gain
    SENSOR_I2C_WRITE( 0x30BA, 0x779C );
    SENSOR_I2C_WRITE( 0x318E, 0x0200 );
    SENSOR_I2C_WRITE( 0x3064, 0x1802 ); // should be 0x1802
    SENSOR_I2C_WRITE( 0x31AE, 0x0304 );
    SENSOR_I2C_WRITE( 0x31C6, 0x0400 );//HISPI_CONTROL_STATUS: HispiSP Packetized
    SENSOR_I2C_WRITE( 0x306E, 0x9210 );//DATAPATH_SELECT[9]=1 VDD_SLVS=1.8V
    SENSOR_I2C_WRITE( 0x301A, 0x005C );//Start streaming
    delay_ms(33);

    // ---------------------hdr end-------------------------
    
    printf("Aptina AR0237 sensor wdr 2M-1080p 30fps init success!\n");
	bSensorInit = HI_TRUE;

}


HI_S32 ar0237_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ar0237_reg_init;
   // pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ar0237_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = ar0237_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ar0237_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = ar0237_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ar0237_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ar0237_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = ar0237_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int ar0237_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ar0237_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, AR0237_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    ar0237_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, AR0237_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    ar0237_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, AR0237_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int ar0237_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, AR0237_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, AR0237_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, AR0237_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int APTINA_AR0237_init(SENSOR_DO_I2CRD do_i2c_read, 
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0237_reg_init();

	ar0237_register_callback();
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
	printf("Aptina AR0237 sensor 1080P30fps init success!\n");
	return s32Ret;
}
int AR0237_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_AR0237_WIDTH;
        *ret_height = SENSOR_AR0237_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_AR0237_probe()
{
    uint16_t ret_data1 = 0;
	
    SENSOR_I2C_READ(0x3000, &ret_data1);
	
    if(ret_data1 == AR0237_CHECK_DATA || ret_data1 == AR0238_CHECK_DATA){
        //set i2c pinmux
		sdk_sys->write_reg(0x200f0040, 0x2);
	    sdk_sys->write_reg(0x200f0044, 0x2);
		
        sdk_sys->write_reg(0x2003002c, 0xb4005);     // sensor unreset, clk 27MHz, VI 148.5MHz
        sdk_sys->write_reg(0x20030104, 0x1);

        sdk_sys->write_reg(0x200f0080, 0x1);
        sdk_sys->write_reg(0x200f0088, 0x1);
        sdk_sys->write_reg(0x200f008c, 0x2);
        sdk_sys->write_reg(0x200f0090, 0x2);
        sdk_sys->write_reg(0x200f0094, 0x1);
        return true;
    }
    return false;
}

int AR0237_get_sensor_name(char *sensor_name)
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


