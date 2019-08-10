#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "ar0130"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x20, 2, 2))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x20, 2, 2))
	
#define SENSOR_AR0130_WIDTH 1280
#define SENSOR_AR0130_HEIGHT 960
#define AR0130_CHECK_DATA (0x2402)
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__AR0130_CMOS_H_)
#define __AR0130_CMOS_H_
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
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

#define AR0130_ID 130

/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance                       */
/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c                                         */
#define CMOS_AR0130_ISP_WRITE_SENSOR_ENABLE (1)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr = 0x20;
static unsigned int sensor_addr_byte = 0x2;
static unsigned int sensor_data_byte = 0x2;


/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

#define FULL_LINES_MAX  (0xFFFF)

#define EXPOSURE_TIME  (0x3012)
#define ANALOG_GAIN    (0x30B0)
#define DIGITAL_GAIN   (0x305E)
#define FRAME_LINES    (0x300A)
#define LINE_LEN_PCK   (0x300C)

#define SENSOR_720P_30FPS_MODE  (1) 
#define SENSOR_960P_30FPS_MODE  (2) 

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define FRAME_LINES_720P  (750+INCREASE_LINES)
#define FRAME_LINES_960P  (990+INCREASE_LINES)
#define LINE_LENGTH_PCK_720P_30FPS   (0x8BA) 
#define LINE_LENGTH_PCK_960P_30FPS   (0x693)

/////////////720p
/*
static HI_U8 gu8SensorImageMode = SENSOR_720P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = FRAME_LINES_720P;
static HI_U32 gu32FullLines = FRAME_LINES_720P; 
static HI_U32 gu32LineLength = LINE_LENGTH_PCK_720P_30FPS;
static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE; 
*/
////960p
static HI_U8 gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = FRAME_LINES_960P;
static HI_U32 gu32FullLines = FRAME_LINES_960P; 
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
#define CMOS_CFG_INI "ar0130_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/ar0130_cfg.ini";


/* AE default parameter and function */
#ifdef INIFILE_CONFIG_MODE

static HI_S32 ar0130_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
    pstAeSnsDft->stAgainAccu.f32Accuracy = 6;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.03125;
    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 32 << pstAeSnsDft->u32ISPDgainShift;
    
    switch(genSensorMode)
    {
        default:
        case WDR_MODE_NONE:   /*linear mode*/
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = g_AeDft[0].u8AeCompensation;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[0].u32MaxIntTimeTarget;
            pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[0].u32MinIntTimeTarget;
            
            pstAeSnsDft->u32MaxAgain = 23088;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = g_AeDft[0].u32MaxAgainTarget;
            pstAeSnsDft->u32MinAgainTarget = g_AeDft[0].u32MinAgainTarget;
            
            pstAeSnsDft->u32MaxDgain = 255;  
            pstAeSnsDft->u32MinDgain = 32;
            pstAeSnsDft->u32MaxDgainTarget = g_AeDft[0].u32MaxDgainTarget;
            pstAeSnsDft->u32MinDgainTarget = g_AeDft[0].u32MinDgainTarget;
          
            pstAeSnsDft->u32ISPDgainShift = g_AeDft[0].u32ISPDgainShift;
            pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[0].u32MinISPDgainTarget;
            pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[0].u32MaxISPDgainTarget;    
        break;
        
    }    
    return 0;
}

#else

static HI_S32 ar0130_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
    pstAeSnsDft->stAgainAccu.f32Accuracy = 6;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.03125;
    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 32 << pstAeSnsDft->u32ISPDgainShift;  

    pstAeSnsDft->enIrisType = ISP_IRIS_DC_TYPE;
    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;

    switch(genSensorMode)
    {
        default:
        case WDR_MODE_NONE:   /*linear mode*/
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x40;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
            pstAeSnsDft->u32MinIntTime = 2;    
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;

            pstAeSnsDft->u32MaxAgain = 23088;  
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
            pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
            
            pstAeSnsDft->u32MaxDgain = 255; 
            pstAeSnsDft->u32MinDgain = 32;
            pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
            pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;            
        break;

    }
    return 0;
}

#endif

/* the function of sensor set fps */
static HI_VOID ar0130_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch (genSensorMode)
    {
        default :
        case WDR_MODE_NONE :
            if (SENSOR_720P_30FPS_MODE == gu8SensorImageMode)
            {
                if ((f32Fps <= 30) && (f32Fps >= 0.5))
                {
                     //gu32LineLength = (LINE_LENGTH_PCK_720P_30FPS * 30) / f32Fps; 
                    gu32FullLinesStd = FRAME_LINES_720P * 30 / f32Fps;
                }
                else
                {
                    printf("Not support Fps: %f\n", f32Fps);
                    return;
                }
               //gu32FullLinesStd = FRAME_LINES_720P;
            }
            else if (SENSOR_960P_30FPS_MODE == gu8SensorImageMode)
            {
                if ((f32Fps <= 30) && (f32Fps >= 0.5))
                {
                    //gu32LineLength = (LINE_LENGTH_PCK_960P_30FPS * 30) / f32Fps; 
                    gu32FullLinesStd = FRAME_LINES_960P * 30 / f32Fps;
                }
                else
                {
                    printf("Not support Fps: %f\n", f32Fps);
                    return;
                }
                 //gu32FullLinesStd = FRAME_LINES_960P;  
            }
            else
            {
                printf("Not support! gu8SensorImageMode:%d, f32Fps:%f\n", gu8SensorImageMode, f32Fps);
                return;
            }

            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
            break;
      
    }
    
    gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;
    g_stSnsRegsInfo.astI2cData[4].u32Data = gu32FullLinesStd;

    //gu32LineLength = gu32LineLength > FULL_LINES_MAX ? FULL_LINES_MAX : gu32LineLength;
    //g_stSnsRegsInfo.astI2cData[5].u32Data = gu32LineLength;
    //SENSOR_I2C_WRITE(LINE_LEN_PCK, gu32LineLength);

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    gu32FullLines = gu32FullLinesStd;
    pstAeSnsDft->u32FullLines = gu32FullLines;

    return;
}


static HI_VOID ar0130_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;
    pstAeSnsDft->u32FullLines = gu32FullLines;

    g_stSnsRegsInfo.astI2cData[4].u32Data = gu32FullLines;

    //SENSOR_I2C_WRITE(FRAME_LINES, gu32FullLines);

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 2;
    
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ar0130_inttime_update(HI_U32 u32IntTime)
{
    g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;

    return;
}


static HI_U32 analog_gain_table[6] =
{
     1024, 2048, 2886, 5772, 11544, 23088

};


static HI_VOID ar0130_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{

	int i;

    if((HI_NULL == pu32AgainLin) ||(HI_NULL == pu32AgainDb))
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

    if (*pu32AgainLin >= analog_gain_table[5])
    {
         *pu32AgainLin = analog_gain_table[5];
         *pu32AgainDb = 5;
         return ;
    }
    
    for (i = 1; i < 6; i++)
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


static HI_VOID ar0130_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{

 
        if(u32Again >= 2)
        {
            g_stSnsRegsInfo.astI2cData[3].u32Data = 0x0004;
        }
        else
        {
            g_stSnsRegsInfo.astI2cData[3].u32Data = 0x0000;
        }
        
        switch(u32Again)
        {
            case 0:
            case 2:
                g_stSnsRegsInfo.astI2cData[1].u32Data = 0x1300;
                break;
            case 1:
            case 3:
                g_stSnsRegsInfo.astI2cData[1].u32Data = 0x1310;
                break;
            case 4:
                g_stSnsRegsInfo.astI2cData[1].u32Data = 0x1320;
                break;
            case 5:
                g_stSnsRegsInfo.astI2cData[1].u32Data = 0x1330;
                break;
        }
        
        g_stSnsRegsInfo.astI2cData[2].u32Data = u32Dgain;


        return;
}


HI_S32 ar0130_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ar0130_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ar0130_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ar0130_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ar0130_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ar0130_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ar0130_again_calc_table;

    return 0;
}


/* AWB default parameter and function */
#ifdef INIFILE_CONFIG_MODE

static HI_S32 ar0130_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
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
  
    }
    return 0;
}

#else

static AWB_CCM_S g_stAwbCcm =
{
    5120,
    {
		0x202,0x80BD,0x8045,	
		0x805C,0x18F,0x8033,	
		0x801B,0x80A8,0x1C3,

    },

    3633,
    {
		0x1E0,0x808D,0x8053,		
		0x806B,0x193,0x8028,		
		0x8026,0x80D9,0x1FF,

    },

    2449,
    {
		0x1E7,0x80A6,0x8041,			
		0x8060,0x170,0x8010,		
		0x8049,0x80BE,0x207,

    }

};

static AWB_AGC_TABLE_S g_stAwbAgcTableLin =
{
    /* bvalid */
    1,

    /* saturation */
	{115,115,115,112,105,90,75,75,56,56,56,56,56,56,56,56}
};


static HI_S32 ar0130_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;

    pstAwbSnsDft->au16GainOffset[0] = 0x16f;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1af;
/*
    pstAwbSnsDft->as32WbPara[0] = 0x0049;
    pstAwbSnsDft->as32WbPara[1] = 0x000E;
    pstAwbSnsDft->as32WbPara[2] = -0x00A9;
    pstAwbSnsDft->as32WbPara[3] = 0x35A3D;
    pstAwbSnsDft->as32WbPara[4] = 0x80;
    pstAwbSnsDft->as32WbPara[5] = -0x2A004;

	*/
    pstAwbSnsDft->as32WbPara[0] = 106;
    pstAwbSnsDft->as32WbPara[1] = -51;
    pstAwbSnsDft->as32WbPara[2] = -201;
    pstAwbSnsDft->as32WbPara[3] = 233836;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -189069;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    
    switch (genSensorMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableLin, sizeof(AWB_AGC_TABLE_S));
        break;

    }
    
    return 0;
}

#endif


HI_S32 ar0130_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ar0130_get_awb_default;

    return 0;
}


/* ISP default parameter and function */
#ifdef INIFILE_CONFIG_MODE

HI_U32 ar0130_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            memcpy(&pstDef->stGe, &g_IspDft[1].stGe, sizeof(ISP_CMOS_GE_S));            
        break;
        
    }
    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_AR0130_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_AR0130_HEIGHT;

    return 0;
}

#else



#define  DMNR_CALIB_CARVE_NUM_AR0130  (13)
static HI_FLOAT g_coef_calib_ar0130[DMNR_CALIB_CARVE_NUM_AR0130][HI_ISP_NR_CALIB_COEF_COL] = 
{
    {100.000000f,   2.000000f, 0.035900f, 9.163901f}, 
    {611.000000f,   2.786041f, 0.039199f, 9.662953f}, 
    {1104.000000f,  3.042969f, 0.042015f, 10.098933f}, 
    {2317.000000f,  3.364926f, 0.046898f, 11.608693f}, 
    {4625.000000f,  3.665112f, 0.053912f, 14.938090f}, 
    {5923.000000f,  3.772542f, 0.056674f, 17.071514f}, 
    {8068.000000f,  3.906766f, 0.061652f, 20.065725f}, 
    {9985.000000f,  3.999348f, 0.063942f, 22.888287f}, 
    {20395.000000f, 4.309524f, 0.062935f, 41.496876f}, 
    {40067.000000f, 4.602787f, 0.053383f, 77.137268f}, 
    {49630.000000f, 4.695744f, 0.030656f, 104.676781f},
    {60637.000000f, 4.782738f, 0.000000f, 138.000000f}, 
    {89225.000000f, 4.950487f, 0.000000f, 171.000000f}, 
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


static ISP_CMOS_DEMOSAIC_S g_stIspDemosaicLin =
{
	/*For Demosaic*/
	1, /*bEnable*/			
	50,/*u16VhLimit*/	
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


static ISP_CMOS_GE_S g_stIspGeLin =
{
	/*For GE*/
	0,    /*bEnable*/			
	7,    /*u8Slope*/	
	7,    /*u8Sensitivity*/
	4096, /*u16Threshold*/
	4096, /*u16SensiThreshold*/		
	{1024,1024,1024,2048,2048,2048,2048,  2048,  2048,2048,2048,2048,2048,2048,2048,2048}    /*au16Strength[ISP_AUTO_ISO_STENGTH_NUM]*/	
};



static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800}; //ISO
	{0,		0,		0,		0,		0,		0,		0,		1,		1,		1,		1,		1,		1,		1,		1,			1},/* bEnLowLumaShoot */

  	{50,48,46,43,40,40,40,22,20,18,15,12,12,12,12,12},/*SharpenUD*/
  	{63,54,50,48,45,42,40,32,32,32,31,31,31,31,31,31},/*SharpenD*/

  	{ 10,   10,  	12,  	14, 	16, 	18,  	19,  	20,    	22,    	24,    	26,    	28,    	28,    	28,    	28,    		28},/*TextureNoiseThd*/
  	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},/*EdgeNoiseThd*/
  	{80,80,80,80,80,70,60,60,0,0,0,0,0,0,0,0},/*overshoot*/
  	{80,80,80,80,80,70,60,60,0,0,0,0,0,0,0,0},/*undershoot*/
};


static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800}; //ISO
	{1,		2,		4,		6,		8,		10,		12,		14,		16,		18,		20,		22,		22,		22,		22,			22},      /*u8UvnrThreshold*/
 	{0,		0,		0,		0,		0,		0,		0,		0,		1,		1,		2,		2,		2,		2,		2,			2},       /*ColorCast*/
 	{0,		0,		0,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,			34}  /*u8UvnrStrength*/
};

static ISP_CMOS_DPC_S g_stCmosDpc = 
{
	//0,/*IR_channel*/
	//0,/*IR_position*/
	{70,130,244,248,250,252,252,252,252,252,252,252,252,252,252,252},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0,0,0x23,0x80,0xD0,0xF0,0xF0,0xF0,0xF0},/*au16BlendRatio[16]*/

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


HI_U32 ar0130_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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

			memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
			memcpy(&pstDef->stDemosaic, &g_stIspDemosaicLin, sizeof(ISP_CMOS_DEMOSAIC_S));
			memcpy(&pstDef->stGe, &g_stIspGeLin, sizeof(ISP_CMOS_GE_S));			

			pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_AR0130;
			pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef	  = (HI_FLOAT (*)[4])g_coef_calib_ar0130;
			//memcpy(&pstDef->stNoiseTbl.stNrCommPara, &g_stNrCommPara,sizeof(ISP_NR_COMM_PARA_S));
			memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);
			

			memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
			memcpy(&pstDef->stUvnr, 	  &g_stIspUVNR, 	  sizeof(ISP_CMOS_UVNR_S));
			memcpy(&pstDef->stDpc,		 &g_stCmosDpc,		 sizeof(ISP_CMOS_DPC_S));
		break;

	}
	pstDef->stSensorMaxResolution.u32MaxWidth  = 1280;
	pstDef->stSensorMaxResolution.u32MaxHeight = 720;

	return 0;
}


#endif



HI_U32 ar0130_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    HI_S32  i;
	static HI_U32 j = 0;
	if(++j%250 == 0){
		 SENSOR_I2C_WRITE(0x3180, 0xc000);
		 usleep(90000);//after 1 frame
		 SENSOR_I2C_WRITE(0x3180, 0x8000);
	}
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_TRUE;//HI_FALSE;

    switch (genSensorMode)
    {
        default :
        case WDR_MODE_NONE :
            for (i=0; i<4; i++)
            {
                pstBlackLevel->au16BlackLevel[i] = 0xC8;
            }
            break;

    }

    return 0;    
}


HI_VOID ar0130_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps = FRAME_LINES_720P;
    
    if (SENSOR_720P_30FPS_MODE == gu8SensorImageMode)
    {
        u32FullLines_5Fps = LINE_LENGTH_PCK_720P_30FPS * 30 / 5;
    } 
    else if(SENSOR_960P_30FPS_MODE == gu8SensorImageMode)
    {
        u32FullLines_5Fps = LINE_LENGTH_PCK_960P_30FPS * 30 / 5;
    }
    else
    {
        return;
    }

    u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(LINE_LEN_PCK, u32FullLines_5Fps);  /* 5fps */
        SENSOR_I2C_WRITE(EXPOSURE_TIME, 0x118);     /* max exposure time */ 
        SENSOR_I2C_WRITE(ANALOG_GAIN, 0x1300);      /* AG, Context A */
        SENSOR_I2C_WRITE(DIGITAL_GAIN, 0x0020);     /* DG, Context A */
    }
    else /* setup for ISP 'normal mode' */
    {
        if(SENSOR_720P_30FPS_MODE == gu8SensorImageMode)
        {
            SENSOR_I2C_WRITE(LINE_LEN_PCK, LINE_LENGTH_PCK_720P_30FPS);   
        }
        else if(SENSOR_960P_30FPS_MODE == gu8SensorImageMode)
        {
            SENSOR_I2C_WRITE(LINE_LEN_PCK, LINE_LENGTH_PCK_960P_30FPS);   
        }
        else
        {
        
        }
        bInit = HI_FALSE;
    }

    return;
}





HI_VOID ar0130_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            genSensorMode = WDR_MODE_NONE;

            /* Disable DCG */
            SENSOR_I2C_WRITE(0x3100, 0x0000);

            /* Disable 1.25x analog gain */
            SENSOR_I2C_WRITE(0x3EE4, 0xD208);
            
            printf("linear mode\n");
        break;

        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    
    return;
}


static HI_S32 ar0130_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;
    
    bInit = HI_FALSE;    
        
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if((pstSensorImageMode->u16Width <= 1280)&&(pstSensorImageMode->u16Height <= 720))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_720P_30FPS_MODE;
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
    else if((pstSensorImageMode->u16Width <= 1280)&&(pstSensorImageMode->u16Height <= 960))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_960P_30FPS_MODE;
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

HI_U32 ar0130_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;
        g_stSnsRegsInfo.u32RegNum = 5;
             
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
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3100;
	    g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = FRAME_LINES;
        //g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 1;
        //g_stSnsRegsInfo.astI2cData[5].u32RegAddr = LINE_LEN_PCK;
        
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

int  ar0130_sensor_set_inifile_path(const char *pcPath)
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

HI_VOID ar0130_sensor_global_init()
{ /////720p  
 /*   gu8SensorImageMode = SENSOR_720P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE; 
    gu32FullLinesStd = FRAME_LINES_720P;
    gu32FullLines = FRAME_LINES_720P; 
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 
*/

    gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE; 
    gu32FullLinesStd = FRAME_LINES_960P;
    gu32FullLines = FRAME_LINES_960P; 
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

static void delay_ms(int ms) { 
    usleep(ms*1000);
}

void ar0130_reg_init_720p()
{//[720p30]

	SENSOR_I2C_WRITE( 0x301A, 0x0001 );    // RESET_REGISTER
    delay_ms(200); //ms
    SENSOR_I2C_WRITE( 0x301A, 0x10D8 );    // RESET_REGISTER
    delay_ms(200); //ms
    //Linear Mode Setup
    //AR0130 Rev1 Linear sequencer load  8-2-2011
    SENSOR_I2C_WRITE( 0x3088, 0x8000 );// SEQ_CTRL_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0225 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x5050 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2D26 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0828 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0D17 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0926 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0028 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0526 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0xA728 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0725 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x8080 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2917 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0525 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0040 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2702 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1616 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2706 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1736 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x26A6 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1703 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x26A4 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x171F );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2805 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2620 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2804 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2520 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2027 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0017 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1E25 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0020 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2117 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1028 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x051B );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1703 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2706 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1703 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1747 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2660 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x17AE );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2500 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x9027 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0026 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1828 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x002E );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2A28 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x081E );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0831 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1440 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x4014 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2020 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1410 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1034 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1014 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0020 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x4013 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1802 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1470 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x7004 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1470 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x7003 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1470 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x7017 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2002 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2002 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x5004 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2004 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x5022 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0314 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0020 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0314 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x0050 );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2C2C );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x3086, 0x2C2C );// SEQ_DATA_PORT
    SENSOR_I2C_WRITE( 0x309E, 0x0000 );// DCDS_PROG_START_ADDR
    SENSOR_I2C_WRITE( 0x30E4, 0x6372 );// ADC_BITS_6_7
    SENSOR_I2C_WRITE( 0x30E2, 0x7253 );// ADC_BITS_4_5
    SENSOR_I2C_WRITE( 0x30E0, 0x5470 );// ADC_BITS_2_3
    SENSOR_I2C_WRITE( 0x30E6, 0xC4CC );// ADC_CONFIG1
    SENSOR_I2C_WRITE( 0x30E8, 0x8050 );// ADC_CONFIG2
    delay_ms(200); //ms
    SENSOR_I2C_WRITE( 0x3082, 0x0029 );    // OPERATION_MODE_CTRL
    //AR0130 Rev1 Optimized settings
    SENSOR_I2C_WRITE( 0x301E, 0x00C8); // DATA_PEDESTAL
    SENSOR_I2C_WRITE( 0x3EDA, 0x0F03); // DAC_LD_14_15
    SENSOR_I2C_WRITE( 0x3EDE, 0xC005); // DAC_LD_18_19
    SENSOR_I2C_WRITE( 0x3ED8, 0x09EF); // DAC_LD_12_13
    SENSOR_I2C_WRITE( 0x3EE2, 0xA46B); // DAC_LD_22_23
    SENSOR_I2C_WRITE( 0x3EE0, 0x047D); // DAC_LD_20_21
    SENSOR_I2C_WRITE( 0x3EDC, 0x0070); // DAC_LD_16_17
    SENSOR_I2C_WRITE( 0x3044, 0x0404); // DARK_CONTROL
    SENSOR_I2C_WRITE( 0x3EE6, 0x8303); // DAC_LD_26_27
    SENSOR_I2C_WRITE( 0x3EE4, 0xD208); // DAC_LD_24_25
    SENSOR_I2C_WRITE( 0x3ED6, 0x00BD); // DAC_LD_10_11
    SENSOR_I2C_WRITE( 0x30B0, 0x1300); // DIGITAL_TEST
    SENSOR_I2C_WRITE( 0x30D4, 0xE007); // COLUMN_CORRECTION
    SENSOR_I2C_WRITE( 0x301A, 0x10DC); // RESET_REGISTER
    delay_ms(500 );//ms                 
    SENSOR_I2C_WRITE( 0x301A, 0x10D8); // RESET_REGISTER
    delay_ms(500); //ms                   
    SENSOR_I2C_WRITE( 0x3044, 0x0400); // DARK_CONTROL
                                    
    SENSOR_I2C_WRITE( 0x3012, 0x02A0); // COARSE_INTEGRATION_TIME

    
    //720p 30fps Setup                   
    SENSOR_I2C_WRITE( 0x3032, 0x0000); // DIGITAL_BINNING
    SENSOR_I2C_WRITE( 0x3002, 0x0002); // Y_ADDR_START
    SENSOR_I2C_WRITE( 0x3004, 0x0000); // X_ADDR_START
    SENSOR_I2C_WRITE( 0x3006, 0x02D1);//Row End (A) = 721
    SENSOR_I2C_WRITE( 0x3008, 0x04FF);//Column End (A) = 1279
    SENSOR_I2C_WRITE( 0x300A, 0x02EA);//Frame Lines (A) = 746
    SENSOR_I2C_WRITE( 0x300C, 0x08ba);
    SENSOR_I2C_WRITE( 0x3012, 0x0133);//Coarse_IT_Time (A) = 307
    SENSOR_I2C_WRITE( 0x306e, 0x9211);//Coarse_IT_Time (A) = 307


    //Enable Parallel Mode
    SENSOR_I2C_WRITE( 0x301A, 0x10D8); // RESET_REGISTER
    SENSOR_I2C_WRITE( 0x31D0, 0x0001); // HDR_COMP
    
    //PLL Enabled 27Mhz to 50Mhz
    SENSOR_I2C_WRITE( 0x302A, 0x0009 );//VT_PIX_CLK_DIV = 9
    SENSOR_I2C_WRITE( 0x302C, 0x0001 );//VT_SYS_CLK_DIV = 1
    SENSOR_I2C_WRITE( 0x302E, 0x0003 );//PRE_PLL_CLK_DIV = 3
    SENSOR_I2C_WRITE( 0x3030, 0x0032 );//PLL_MULTIPLIER = 50
    SENSOR_I2C_WRITE( 0x30B0, 0x1300 );    // DIGITAL_TEST
    delay_ms(100); //ms
    SENSOR_I2C_WRITE( 0x301A, 0x10DC );    // RESET_REGISTER
    SENSOR_I2C_WRITE( 0x301A, 0x10DC );    // RESET_REGISTER
                                          
    //exposure                            
    SENSOR_I2C_WRITE( 0x3012, 0x0671 );    // COARSE_INTEGRATION_TIME
    SENSOR_I2C_WRITE( 0x30B0, 0x1330 );    // DIGITAL_TEST
    SENSOR_I2C_WRITE( 0x3056, 0x003B );    // GREEN1_GAIN
    SENSOR_I2C_WRITE( 0x305C, 0x003B );    // GREEN2_GAIN
    SENSOR_I2C_WRITE( 0x305A, 0x003B );    // RED_GAIN
    SENSOR_I2C_WRITE( 0x3058, 0x003B );    // BLUE_GAIN
    //High Conversion gain                
    SENSOR_I2C_WRITE( 0x3100, 0x0004 );    // AE_CTRL_REG


    //LOAD= Disable Embedded Data and Stats
    SENSOR_I2C_WRITE(0x3064, 0x1802);  // SMIA_TEST, EMBEDDED_STATS_EN, 0x0000
    SENSOR_I2C_WRITE(0x3064, 0x1802);  // SMIA_TEST, EMBEDDED_DATA, 0x0000 

    SENSOR_I2C_WRITE(0x30BA, 0x0008);       //20120502

    SENSOR_I2C_WRITE(0x3EE4, 0xD308);  //the default value former is 0xd208

    SENSOR_I2C_WRITE(0x301A, 0x10DC);  // RESET_REGISTER

    delay_ms(200);  //DELAY= 200
    
	printf("Aptina AR0130 sensor reg init success!\n");

	bSensorInit = HI_TRUE;
   }

void ar0130_reg_init_960p()
{
	SENSOR_I2C_WRITE( 0x301A, 0x0001 ); // RESET_REGISTER
	delay_ms(200); //ms
	SENSOR_I2C_WRITE( 0x301A, 0x10D8 );	// RESET_REGISTER
	delay_ms(200); //ms
	//Linear Mode Setup
	//AR0130 Rev1 Linear sequencer load  8-2-2011
	SENSOR_I2C_WRITE( 0x3088, 0x8000 );// SEQ_CTRL_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0225 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x5050 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2D26 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0828 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0D17 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0926 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0028 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0526 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0xA728 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0725 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x8080 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2917 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0525 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0040 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2702 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1616 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2706 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1736 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x26A6 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1703 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x26A4 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x171F );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2805 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2620 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2804 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2520 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2027 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0017 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1E25 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0020 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2117 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1028 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x051B );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1703 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2706 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1703 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1747 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2660 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x17AE );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2500 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x9027 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0026 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1828 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x002E );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2A28 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x081E );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0831 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1440 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x4014 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2020 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1410 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1034 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1014 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0020 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x4013 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1802 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1470 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x7004 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1470 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x7003 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1470 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x7017 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2002 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2002 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x5004 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2004 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x1400 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x5022 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0314 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0020 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0314 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x0050 );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2C2C );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x3086, 0x2C2C );// SEQ_DATA_PORT
	SENSOR_I2C_WRITE( 0x309E, 0x0000 );// DCDS_PROG_START_ADDR
	SENSOR_I2C_WRITE( 0x30E4, 0x6372 );// ADC_BITS_6_7
	SENSOR_I2C_WRITE( 0x30E2, 0x7253 );// ADC_BITS_4_5
	SENSOR_I2C_WRITE( 0x30E0, 0x5470 );// ADC_BITS_2_3
	SENSOR_I2C_WRITE( 0x30E6, 0xC4CC );// ADC_CONFIG1
	SENSOR_I2C_WRITE( 0x30E8, 0x8050 );// ADC_CONFIG2
	delay_ms(200); //ms
	SENSOR_I2C_WRITE( 0x3082, 0x0029 );	// OPERATION_MODE_CTRL
	//AR0130 Rev1 Optimized settings
	SENSOR_I2C_WRITE( 0x301E, 0x00C8); // DATA_PEDESTAL
	SENSOR_I2C_WRITE( 0x3EDA, 0x0F03); // DAC_LD_14_15
	SENSOR_I2C_WRITE( 0x3EDE, 0xC005); // DAC_LD_18_19
	SENSOR_I2C_WRITE( 0x3ED8, 0x09EF); // DAC_LD_12_13
	SENSOR_I2C_WRITE( 0x3EE2, 0xA46B); // DAC_LD_22_23
	SENSOR_I2C_WRITE( 0x3EE0, 0x047D); // DAC_LD_20_21
	SENSOR_I2C_WRITE( 0x3EDC, 0x0070); // DAC_LD_16_17
	SENSOR_I2C_WRITE( 0x3044, 0x0404); // DARK_CONTROL
	SENSOR_I2C_WRITE( 0x3EE6, 0x8303); // DAC_LD_26_27
	SENSOR_I2C_WRITE( 0x3EE4, 0xD208); // DAC_LD_24_25
	SENSOR_I2C_WRITE( 0x3ED6, 0x00BD); // DAC_LD_10_11
	SENSOR_I2C_WRITE( 0x30B0, 0x1300); // DIGITAL_TEST
	SENSOR_I2C_WRITE( 0x30D4, 0xE007); // COLUMN_CORRECTION
	SENSOR_I2C_WRITE( 0x301A, 0x10DC); // RESET_REGISTER
	delay_ms(500 );//ms 				
	SENSOR_I2C_WRITE( 0x301A, 0x10D8); // RESET_REGISTER
	delay_ms(500); //ms 				  
	SENSOR_I2C_WRITE( 0x3044, 0x0400); // DARK_CONTROL
						
	SENSOR_I2C_WRITE( 0x3012, 0x02A0); // COARSE_INTEGRATION_TIME


	//960p 30fps Setup		 
	SENSOR_I2C_WRITE( 0x3032, 0x0000); // DIGITAL_BINNING

	/*SENSOR_I2C_WRITE( 0x3002, 0x0002); // Y_ADDR_START
	SENSOR_I2C_WRITE( 0x3004, 0x0000); // X_ADDR_START
	SENSOR_I2C_WRITE( 0x3006, 0x02D1);//Row End (A) = 721
	SENSOR_I2C_WRITE( 0x3008, 0x04FF);//Column End (A) = 1279
	SENSOR_I2C_WRITE( 0x300A, 0x02EA);//Frame Lines (A) = 746


	SENSOR_I2C_WRITE( 0x300C, 0x08ba);
	*/
	SENSOR_I2C_WRITE(0x3002, 0x0004);	//Y_ADDR_START
	SENSOR_I2C_WRITE(0x3004, 0x0002);	 // X_ADDR_START
	SENSOR_I2C_WRITE(0x3006, 0x03c3);	// Y_ADDR_END
	SENSOR_I2C_WRITE(0x3008, 0x0501);	// X_ADDR_END
	SENSOR_I2C_WRITE(0x300A, 0x03de);	// FRAME_LENGTH_LINES  ///ddd 990  
	SENSOR_I2C_WRITE(0x300C, 0x0693);	// LINE_LENGTH_PCK	///ddd 1683 0x693 2016 0x7e0

	SENSOR_I2C_WRITE( 0x3012, 0x0133);//Coarse_IT_Time (A) = 307
	SENSOR_I2C_WRITE( 0x306e, 0x9211);//Coarse_IT_Time (A) = 307


	//Enable Parallel Mode
	SENSOR_I2C_WRITE( 0x301A, 0x10D8); // RESET_REGISTER
	SENSOR_I2C_WRITE( 0x31D0, 0x0001); // HDR_COMP

	//PLL Enabled 27Mhz to 50Mhz
	SENSOR_I2C_WRITE( 0x302A, 0x0009 );//VT_PIX_CLK_DIV = 9
	SENSOR_I2C_WRITE( 0x302C, 0x0001 );//VT_SYS_CLK_DIV = 1
	SENSOR_I2C_WRITE( 0x302E, 0x0003 );//PRE_PLL_CLK_DIV = 3
	SENSOR_I2C_WRITE( 0x3030, 0x0032 );//PLL_MULTIPLIER = 50
	SENSOR_I2C_WRITE( 0x30B0, 0x1300 );	// DIGITAL_TEST
	delay_ms(100); //ms
	SENSOR_I2C_WRITE( 0x301A, 0x10DC );	// RESET_REGISTER
	SENSOR_I2C_WRITE( 0x301A, 0x10DC );	// RESET_REGISTER
							  
	//exposure							  
	SENSOR_I2C_WRITE( 0x3012, 0x0671 );	// COARSE_INTEGRATION_TIME
	SENSOR_I2C_WRITE( 0x30B0, 0x1330 );	// DIGITAL_TEST
	SENSOR_I2C_WRITE( 0x3056, 0x003B );	// GREEN1_GAIN
	SENSOR_I2C_WRITE( 0x305C, 0x003B );	// GREEN2_GAIN
	SENSOR_I2C_WRITE( 0x305A, 0x003B );	// RED_GAIN
	SENSOR_I2C_WRITE( 0x3058, 0x003B );	// BLUE_GAIN
	//High Conversion gain				  
	SENSOR_I2C_WRITE( 0x3100, 0x0004 );	// AE_CTRL_REG


	//LOAD= Disable Embedded Data and Stats
	SENSOR_I2C_WRITE(0x3064, 0x1802);	// SMIA_TEST, EMBEDDED_STATS_EN, 0x0000
	SENSOR_I2C_WRITE(0x3064, 0x1802);	// SMIA_TEST, EMBEDDED_DATA, 0x0000 

	SENSOR_I2C_WRITE(0x30BA, 0x0008);		 //20120502

	SENSOR_I2C_WRITE(0x3EE4, 0xD308);	//the default value former is 0xd208

	SENSOR_I2C_WRITE(0x301A, 0x10DC);	// RESET_REGISTER

	delay_ms(200);	//DELAY= 200

	printf("Aptina AR0130 sensor 960P30fps init success!\n");

	bSensorInit = HI_TRUE;
}

HI_S32 ar0130_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ar0130_reg_init_960p;
  //  pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ar0130_sensor_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = ar0130_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ar0130_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = ar0130_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ar0130_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ar0130_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = ar0130_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int ar0130_sensor_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ar0130_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, AR0130_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    ar0130_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, AR0130_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    ar0130_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, AR0130_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int ar0130_sensor_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, AR0130_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, AR0130_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, AR0130_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int APTINA_AR0130_init(SENSOR_DO_I2CRD do_i2c_read, 
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	ar0130_sensor_register_callback();
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
	printf("Aptina AR0130 sensor 960P30fps init success!\n");
	return s32Ret;
}



int AR0130_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_AR0130_WIDTH;
        *ret_height = SENSOR_AR0130_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_AR0130_probe()
{
    uint16_t ret_data1 = 0;
	
    SENSOR_I2C_READ(0x3000, &ret_data1);
	
    if(ret_data1 == AR0130_CHECK_DATA){
		//set i2c pinmux
		sdk_sys->write_reg(0x200f0040, 0x2);
	    sdk_sys->write_reg(0x200f0044, 0x2);
		
        sdk_sys->write_reg(0x200f007c, 0x1);     //cmos pinmux
        sdk_sys->write_reg(0x200f0080, 0x1);
        sdk_sys->write_reg(0x200f0084, 0x1);
        sdk_sys->write_reg(0x200f0088, 0x1);
        sdk_sys->write_reg(0x200f008c, 0x2);
        sdk_sys->write_reg(0x200f0090, 0x2);
        sdk_sys->write_reg(0x200f0094, 0x1);
        sdk_sys->write_reg(0x2003002c, 0xb4001);    //sensor unreset, clk 27MHz, VI 99MHz

        return true;
    }
    return false;
}
int AR0130_get_sensor_name(char *sensor_name)
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

#endif // __IMX104_CMOS_H_

