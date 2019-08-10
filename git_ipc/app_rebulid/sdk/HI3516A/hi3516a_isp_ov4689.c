#include "sdk/sdk_debug.h"
#include "hi3516a.h"
#include "hi3516a_isp_sensor.h"
#include "hi_i2c.h"
#include <fcntl.h>
#include <unistd.h>
#include "sdk/sdk_sys.h"
#include "hi_isp_i2c.h"

#define SENSOR_NAME "ov4689"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) :_i2c_read((_add), (_ret_data), 0x42, 2, 1))
	
#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x42, 2, 1))

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#define SENSOR_OV4689_WIDTH 2592
#define SENSOR_OV4689_HEIGHT 1520
#define OV4689_CHECK_DATA (0x46)

#if !defined(__OV4689_CMOS_H_)
#define __OV4689_CMOS_H_

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

#define OV4689_ID 4689

/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/
#ifdef INIFILE_CONFIG_MODE

static AE_SENSOR_DEFAULT_S  g_AeDft[];
static AWB_SENSOR_DEFAULT_S g_AwbDft[];
static ISP_CMOS_DEFAULT_S   g_IspDft[];
extern HI_S32 Cmos_LoadINIPara(const HI_CHAR *pcName);
#else

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static unsigned int sensor_i2c_addr = 0x42;
static unsigned int sensor_addr_byte = 2;
static unsigned int sensor_data_byte = 1;



#define SENSOR_4M_30FPS_MODE         (0)
#define SENSOR_1080P_60FPS_MODE      (1)
#define SENSOR_1080P_30FPS_MODE      (2)
#define SENSOR_2304_1296_30FPS_MODE  (3)
#define SENSOR_2048_1520_30FPS_MODE  (4)
#define SENSOR_720p_120FPS_MODE (5)
#define SENSOR_720p_180FPS_MODE (6)

#define OV4689_VMAX_4M30      (1632)
#define OV4689_VMAX_1080P     (1123)
#define OV4689_VMAX_1080P_WDR_LINE (1312)

#define OV4689_VMAX_2304_1296 (1849)
#define OV4689_VMAX_2048_1520 (2128)
#define OV4689_VMAX_720P  (773)

static HI_U8 gu8SensorImageMode = SENSOR_4M_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;
static HI_BOOL bSensorInit = HI_FALSE; 

static HI_U32 gu32FullLinesStd = 1632; 
static HI_U32 gu32FullLines = 1632;
static HI_BOOL bInit = HI_FALSE;


ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};

#define PATHLEN_MAX 256
#define CMOS_CFG_INI "ov4689_cfg.ini"
//static char pcName[PATHLEN_MAX] = "configs/ov4689_cfg.ini";

/* AE default parameter and function */

#ifdef INIFILE_CONFIG_MODE



static HI_S32 ov4689_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;
	
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
             
             pstAeSnsDft->u32MaxAgain = 16384; 
             pstAeSnsDft->u32MinAgain = 1024;
             pstAeSnsDft->u32MaxAgainTarget = g_AeDft[0].u32MaxAgainTarget;
             pstAeSnsDft->u32MinAgainTarget = g_AeDft[0].u32MinAgainTarget;
             
             pstAeSnsDft->u32MaxDgain = 1;  
             pstAeSnsDft->u32MinDgain = 1;
             pstAeSnsDft->u32MaxDgainTarget = g_AeDft[0].u32MaxDgainTarget;
             pstAeSnsDft->u32MinDgainTarget = g_AeDft[0].u32MinDgainTarget;
    
             pstAeSnsDft->u32ISPDgainShift = g_AeDft[0].u32ISPDgainShift;
             pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[0].u32MinISPDgainTarget;
             pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[0].u32MaxISPDgainTarget;                  
        break;
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE: 
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
           
            pstAeSnsDft->u8AeCompensation = g_AeDft[2].u8AeCompensation;
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[2].u32MaxIntTimeTarget;
            pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[2].u32MinIntTimeTarget;
            
            pstAeSnsDft->u32MaxAgain = 16384;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = g_AeDft[2].u32MaxAgainTarget;
            pstAeSnsDft->u32MinAgainTarget = g_AeDft[2].u32MinAgainTarget;
            
            pstAeSnsDft->u32MaxDgain = 1;  
            pstAeSnsDft->u32MinDgain = 1;
            pstAeSnsDft->u32MaxDgainTarget = g_AeDft[2].u32MaxDgainTarget;
            pstAeSnsDft->u32MinDgainTarget = g_AeDft[2].u32MinDgainTarget;

            pstAeSnsDft->u32ISPDgainShift = g_AeDft[2].u32ISPDgainShift;
            pstAeSnsDft->u32MinISPDgainTarget = g_AeDft[2].u32MinISPDgainTarget;
            pstAeSnsDft->u32MaxISPDgainTarget = g_AeDft[2].u32MaxISPDgainTarget;            
        break; 
        case WDR_MODE_2To1_LINE:
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;

            pstAeSnsDft->u8AeCompensation = g_AeDft[1].u8AeCompensation;

            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;  
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = g_AeDft[1].u32MaxIntTimeTarget;
            pstAeSnsDft->u32MinIntTimeTarget = g_AeDft[1].u32MinIntTimeTarget;

            pstAeSnsDft->u32MaxAgain = 16384;
            pstAeSnsDft->u32MinAgain = 1024;
            pstAeSnsDft->u32MaxAgainTarget = g_AeDft[1].u32MaxAgainTarget;
            pstAeSnsDft->u32MinAgainTarget = g_AeDft[1].u32MinAgainTarget;

            pstAeSnsDft->u32MaxDgain = 1;
            pstAeSnsDft->u32MinDgain = 1;
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

static HI_S32 ov4689_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;
    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift; 
    
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
    pstAeSnsDft->u32MinIntTime = 2;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    pstAeSnsDft->u32MaxAgain = 16832;  
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->u32MaxDgain = 1;  
    pstAeSnsDft->u32MinDgain = 1;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;
    switch(genSensorMode)
    {
        default:
        case WDR_MODE_NONE:   /*linear mode*/
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x38;
        break;

        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
           
            pstAeSnsDft->u8AeCompensation = 0x38;
        break;
  
        case WDR_MODE_2To1_LINE:
            pstAeSnsDft->au8HistThresh[0] = 0xC;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
           
            pstAeSnsDft->u8AeCompensation = 0x40;
        break;
    }
    
    switch(gu8SensorImageMode)
    {
        case 1:
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*60/2;
        break;
        
        case 5:
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*120/2;
        break;
            
        case 6:
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*180/2;
        break;
        
        default:
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;
        break;
    }
    return 0;
}
#endif

/* the function of sensor set fps */
static HI_VOID ov4689_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch (gu8SensorImageMode)
    {
        case SENSOR_4M_30FPS_MODE:
            if(genSensorMode == WDR_MODE_NONE ||genSensorMode == WDR_MODE_2To1_FRAME ||genSensorMode == WDR_MODE_2To1_FRAME_FULL_RATE)
            {
                if ((f32Fps <= 30) && (f32Fps >= 0.5))
                {
                    gu32FullLinesStd = (OV4689_VMAX_4M30 * 30) / f32Fps;
                }
                else
                {
                    printf("Not support Fps: %f\n", f32Fps);
                    return;
                }
            }
            else
            {

                if((f32Fps <= 25) && (f32Fps >= 0.5))    
				{                
					gu32FullLinesStd = OV4689_VMAX_4M30 *25 /(f32Fps);      
				}           
				else              
				{                  
					//gu32FullLinesStd = OV4689_VMAX_4M30;         
					printf("Not support Fps: %f\n", f32Fps);                  
					return;             
				}
            }
            
        break;
        
        case SENSOR_2304_1296_30FPS_MODE:
            if ((f32Fps <= 30) && (f32Fps >= 0.5))
            {
                if(WDR_MODE_2To1_LINE == genSensorMode)
                {
                    gu32FullLinesStd = (1340 * 30) / f32Fps;
                }
                else
                {
                    gu32FullLinesStd = (OV4689_VMAX_2304_1296 * 30) / f32Fps;
                }
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }
        break;
        
        case SENSOR_2048_1520_30FPS_MODE:
            if ((f32Fps <= 30) && (f32Fps >= 0.5))
            {
                if(WDR_MODE_2To1_LINE == genSensorMode)
                {
                   gu32FullLinesStd = (0x620 * 30) / f32Fps;
                }
                else
                {
                    gu32FullLinesStd = (0x850 * 30) / f32Fps;
                }
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }

        break;
        
        case SENSOR_1080P_60FPS_MODE:
            if ((f32Fps <= 60) && (f32Fps >= 0.5))
            {
                gu32FullLinesStd = (OV4689_VMAX_1080P * 60) / f32Fps;
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }            
        break;

        case SENSOR_1080P_30FPS_MODE:
            if ((f32Fps <= 30) && (f32Fps >= 0.5))
            {

				if(WDR_MODE_2To1_LINE == genSensorMode) 	
				{				
					gu32FullLinesStd = (OV4689_VMAX_1080P_WDR_LINE * 30) / f32Fps;		
				} 		
				else 			
				{				
					gu32FullLinesStd = (OV4689_VMAX_1080P * 30) / f32Fps;		
				}
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }            
        break;

        case SENSOR_720p_120FPS_MODE:
            if ((f32Fps <= 120) && (f32Fps >= 0.5))
            {
                gu32FullLinesStd = (OV4689_VMAX_720P * 120) / f32Fps;
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }            
        break;
        
        case SENSOR_720p_180FPS_MODE:
            if ((f32Fps <= 180) && (f32Fps >= 0.5))
            {
                gu32FullLinesStd = (OV4689_VMAX_720P * 180) / f32Fps;
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }            
        break;

        default:
        break;
    }
    
    gu32FullLinesStd = gu32FullLinesStd > 0x7FFF ? 0x7FFF : gu32FullLinesStd;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLinesStd & 0xFF);
        g_stSnsRegsInfo.astI2cData[6].u32Data = (gu32FullLinesStd & 0x7F00) >> 8;
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[10].u32Data = (gu32FullLinesStd & 0xFF);
        g_stSnsRegsInfo.astI2cData[11].u32Data = (gu32FullLinesStd & 0x7F00) >> 8;
    }

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID ov4689_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > 0x7FFF) ? 0x7FFF : u32FullLines;
    gu32FullLines = u32FullLines;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLines & 0xFF);
        g_stSnsRegsInfo.astI2cData[6].u32Data = (gu32FullLines & 0x7F00) >> 8;
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[10].u32Data = (gu32FullLines & 0xFF);
        g_stSnsRegsInfo.astI2cData[11].u32Data = (gu32FullLines & 0x7F00) >> 8;
    }

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;
    
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ov4689_inttime_update(HI_U32 u32IntTime)
{
    static HI_BOOL bFirst = HI_TRUE;

    if (WDR_MODE_2To1_LINE == genSensorMode)
    {
        if (bFirst) /* short exposure */
        {
            g_stSnsRegsInfo.astI2cData[5].u32Data = u32IntTime >> 12;
            g_stSnsRegsInfo.astI2cData[6].u32Data = (u32IntTime & 0xFF0) >> 4;
            g_stSnsRegsInfo.astI2cData[7].u32Data = (u32IntTime & 0xF) << 4;
            bFirst = HI_FALSE;
        }
        else /* long exposure */
        {
            g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime >> 12;
            g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime & 0xFF0) >> 4;
            g_stSnsRegsInfo.astI2cData[2].u32Data = (u32IntTime & 0xF) << 4;
            bFirst = HI_TRUE;
        }
    }
    else if (WDR_MODE_2To1_FRAME == genSensorMode ||WDR_MODE_2To1_FRAME_FULL_RATE ==genSensorMode)
    {
        if (bFirst) /* short exposure */
        {
            g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime >> 12;
            g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime & 0xFF0) >> 4;
            g_stSnsRegsInfo.astI2cData[2].u32Data = (u32IntTime & 0xF) << 4;
            bFirst = HI_FALSE;
            
        }
        else /* long exposure */
        {
            g_stSnsRegsInfo.astI2cData[5].u32Data = u32IntTime >> 12;
            g_stSnsRegsInfo.astI2cData[6].u32Data = (u32IntTime & 0xFF0) >> 4;
            g_stSnsRegsInfo.astI2cData[7].u32Data = (u32IntTime & 0xF) << 4;
            bFirst = HI_TRUE;
        }
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime >> 12;
        g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime & 0xFF0) >> 4;
        g_stSnsRegsInfo.astI2cData[2].u32Data = (u32IntTime & 0xF) << 4;
        bFirst = HI_TRUE;
    }

    return;
}

static HI_U32 gain_table[520]=
{
     1024,  1032,  1040,  1048,  1056,  1064,  1072,  1080,  1088,  1096,  1104,  1112,  1120,  1128,
     1136,  1144,  1152,  1160,  1168,  1176,  1184,  1192,  1200,  1208,  1216,  1224,  1232,  1240,
     1248,  1256,  1264,  1272,  1280,  1288,  1296,  1304,  1312,  1320,  1328,  1336,  1344,  1352,
     1360,  1368,  1376,  1384,  1392,  1400,  1408,  1416,  1424,  1432,  1440,  1448,  1456,  1464,
     1472,  1480,  1488,  1496,  1504,  1512,  1520,  1528,  1536,  1544,  1552,  1560,  1568,  1576,
     1584,  1592,  1600,  1608,  1616,  1624,  1632,  1640,  1648,  1656,  1664,  1672,  1680,  1688,
     1696,  1704,  1712,  1720,  1728,  1736,  1744,  1752,  1760,  1768,  1776,  1784,  1792,  1800,
     1808,  1816,  1824,  1832,  1840,  1848,  1856,  1864,  1872,  1880,  1888,  1896,  1904,  1912,
     1920,  1928,  1936,  1944,  1952,  1960,  1968,  1976,  1984,  1992,  2000,  2008,  2016,  2024,
     2032,  2040,  2048,  2064,  2080,  2096,  2112,  2128,  2144,  2160,  2176,  2192,  2208,  2224,
     2240,  2256,  2272,  2288,  2304,  2320,  2336,  2352,  2368,  2384,  2400,  2416,  2432,  2448,
     2464,  2480,  2496,  2512,  2528,  2544,  2560,  2576,  2592,  2608,  2624,  2640,  2656,  2672,
     2688,  2704,  2720,  2736,  2752,  2768,  2784,  2800,  2816,  2832,  2848,  2864,  2880,  2896,
     2912,  2928,  2944,  2960,  2976,  2992,  3008,  3024,  3040,  3056,  3072,  3088,  3104,  3120,
     3136,  3152,  3168,  3184,  3200,  3216,  3232,  3248,  3264,  3280,  3296,  3312,  3328,  3344,
     3360,  3376,  3392,  3408,  3424,  3440,  3456,  3472,  3488,  3504,  3520,  3536,  3552,  3568,
     3584,  3600,  3616,  3632,  3648,  3664,  3680,  3696,  3712,  3728,  3744,  3760,  3776,  3792,
     3808,  3824,  3840,  3856,  3872,  3888,  3904,  3920,  3936,  3952,  3968,  3984,  4000,  4016,
     4032,  4048,  4064,  4080,  4096,  4128,  4160,  4192,  4224,  4256,  4288,  4320,  4352,  4384,
     4416,  4448,  4480,  4512,  4544,  4576,  4608,  4640,  4672,  4704,  4736,  4768,  4800,  4832,
     4864,  4896,  4928,  4960,  4992,  5024,  5056,  5088,  5120,  5152,  5184,  5216,  5248,  5280,
     5312,  5344,  5376,  5408,  5440,  5472,  5504,  5536,  5568,  5600,  5632,  5664,  5696,  5728,
     5760,  5792,  5824,  5856,  5888,  5920,  5952,  5984,  6016,  6048,  6080,  6112,  6144,  6176,
     6208,  6240,  6272,  6304,  6336,  6368,  6400,  6432,  6464,  6496,  6528,  6560,  6592,  6624,
     6656,  6688,  6720,  6752,  6784,  6816,  6848,  6880,  6912,  6944,  6976,  7008,  7040,  7072,
     7104,  7136,  7168,  7200,  7232,  7264,  7296,  7328,  7360,  7392,  7424,  7456,  7488,  7520,
     7552,  7584,  7616,  7648,  7680,  7712,  7744,  7776,  7808,  7840,  7872,  7904,  7936,  7968,
     8000,  8032,  8064,  8096,  8128,  8160,  8192,  8256,  8320,  8384,  8448,  8512,  8576,  8640,
     8704,  8768,  8832,  8896,  8960,  9024,  9088,  9152,  9216,  9280,  9344,  9408,  9472,  9536,
     9600,  9664,  9728,  9792,  9856,  9920,  9984, 10048, 10112, 10176, 10240, 10304, 10368, 10432,
     10496, 10560, 10624, 10688, 10752, 10816, 10880, 10944, 11008, 11072, 11136, 11200, 11264, 11328,
     11392, 11456, 11520, 11584, 11648, 11712, 11776, 11840, 11904, 11968, 12032, 12096, 12160, 12224,
     12288, 12352, 12416, 12480, 12544, 12608, 12672, 12736, 12800, 12864, 12928, 12992, 13056, 13120,
     13184, 13248, 13312, 13376, 13440, 13504, 13568, 13632, 13696, 13760, 13824, 13888, 13952, 14016,
     14080, 14144, 14208, 14272, 14336, 14400, 14464, 14528, 14592, 14656, 14720, 14784, 14848, 14912,
     14976, 15040, 15104, 15168, 15232, 15296, 15360, 15424, 15488, 15552, 15616, 15680, 15744, 15808,
     15872, 15936, 16000, 16064, 16128, 16192, 16256, 16320, 16384, 16448, 16512, 16576, 16640, 16704,
     16768, 16832
};


static HI_VOID ov4689_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if (*pu32AgainLin >= gain_table[519])
    {
         *pu32AgainLin = gain_table[519];
         *pu32AgainDb = 519;
         return ;
    }
    
    for (i = 1; i < 520; i++)
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

static HI_VOID ov4689_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{    
	HI_U32 u32AGainReg = 0;

    if(u32Again < 128)
    {
        u32AGainReg = 0;
        u32Again += 128;    //128~255
    }
    else if(u32Again < 256)
    {
        u32AGainReg = 1;
        u32Again -= 8;      //120~247
    }
    else if(u32Again < 384)
    {
        u32AGainReg = 3;
        u32Again -= 140;    //116~243
    }
    else
    {
        u32AGainReg = 7;
        u32Again -= 264;    //120~255
    }

    g_stSnsRegsInfo.astI2cData[3].u32Data = u32AGainReg;
    g_stSnsRegsInfo.astI2cData[4].u32Data = u32Again; 
    
    if (WDR_MODE_2To1_LINE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[8].u32Data = g_stSnsRegsInfo.astI2cData[3].u32Data;
        g_stSnsRegsInfo.astI2cData[9].u32Data = g_stSnsRegsInfo.astI2cData[4].u32Data;
    }

    return;
}

/* Only used in WDR_MODE_2To1_LINE mode */
static HI_VOID ov4689_get_inttime_max(HI_U32 u32Ratio, HI_U32 *pu32IntTimeMax)
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
    else if ((WDR_MODE_2To1_LINE == genSensorMode))
    {
        /* Short + Long < 1V - 4; 
           Ratio = Long * 0x40 / Short */
           
        *pu32IntTimeMax = (gu32FullLinesStd - 50) * 0x40 / (u32Ratio + 0x40);
        if(gu8SensorImageMode == SENSOR_1080P_30FPS_MODE)
        {
            *pu32IntTimeMax = (gu32FullLinesStd - 200) * 0x40 / (u32Ratio + 0x40);
        }
		 else if((gu8SensorImageMode == SENSOR_2304_1296_30FPS_MODE) || (gu8SensorImageMode == SENSOR_2048_1520_30FPS_MODE))
        {
            *pu32IntTimeMax = (gu32FullLinesStd - 200) * 0x40 / (u32Ratio + 0x40);
        }
    }
    return;
}

HI_S32 ov4689_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ov4689_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ov4689_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ov4689_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ov4689_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ov4689_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ov4689_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = NULL;
    pstExpFuncs->pfn_cmos_get_inttime_max   = ov4689_get_inttime_max;

    return 0;
}


/* AWB default parameter and function */
#ifdef INIFILE_CONFIG_MODE

static HI_S32 ov4689_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
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
        case WDR_MODE_2To1_LINE:
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
 
		
   4784,
   {
	   0x1B3,0x808A,0x8029,	   
	   0x8068,0x1DF,0x8077,	   
	   0x8019,0x80A2,0x1BB
   },
   
   3625,
   {
	   0x180,0x8046,0x803A,		   
	   0x807D,0x1B5,0x8038,		   
	   0x8019,0x80A0,0x1B9
   },
   2372,
   {     
	   0x14E,0x8049,0x8005,		   
	   0x8094,0x1A4,0x8010,		   
	   0x8032,0x80BD,0x1EF	   
   }

};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */ 
//	{0x7D,0x7E,0x7D,0x7A,0x76,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
	{0x82,0x80,0x7E,0x7C,0x76,0x60,0x3C,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32}
};

static AWB_AGC_TABLE_S g_stAwbAgcTableFSWDR =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x80,0x80,0x80,0x78,0x70,0x68,0x58,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38,0x38}

};

static HI_S32 ov4689_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

	
    pstAwbSnsDft->u16WbRefTemp = 4784;
    pstAwbSnsDft->au16GainOffset[0] = 526;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 524;

    pstAwbSnsDft->as32WbPara[0] = 67;
    pstAwbSnsDft->as32WbPara[1] = 67;
    pstAwbSnsDft->as32WbPara[2] = -121;
    pstAwbSnsDft->as32WbPara[3] = 185840;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -134394;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));

    switch (genSensorMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
        break;
        case WDR_MODE_2To1_LINE:
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableFSWDR, sizeof(AWB_AGC_TABLE_S));
        break;
    }

    return 0;
}
#endif


HI_S32 ov4689_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ov4689_get_awb_default;

    return 0;
}


/* ISP default parameter and function */
#ifdef INIFILE_CONFIG_MODE

HI_U32 ov4689_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            
        case WDR_MODE_2To1_LINE:
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

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_OV4689_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_OV4689_HEIGHT;

    return 0;
}

#else

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
    
    /* 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800 */

    /* sharpen_alt_d */
    {0x6E,0x64,0x63,0x62,0x62,0x50,0x50,0x46,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14},

    /* sharpen_alt_ud */
    {0xA5,0xA5,0x89,0x89,0x88,0x64,0x37,0x28,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14},
        
    /* snr_thresh Max=0x54 */
    {0x19,0x19,0x23,0x34,0x36,0x3D,0x44,0x58,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
    /* demosaic_lum_thresh */
    {0x50,0x50,0x4e,0x49,0x45,0x45,0x40,0x3a,0x3a,0x30,0x30,0x2a,0x20,0x20,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x00,0x0a,0x12,0x1a,0x20,0x28,0x30,0x32,0x34,0x36,0x38,0x38,0x38,0x38,0x38,0x38},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37,0x37,0x35,0x35,0x35,0x35,0x35,0x35,0x35},

    /* rgbsharp_strength */
	{0x87,0x87,0x7D,0x7D,0x64,0x59,0x4B,0x46,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14}
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,

    /* nosie_profile_weight_lut */
    {0x0, 0x0, 0x8, 0xF, 0x13, 0x16, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x23, 0x25, 
    0x25, 0x26, 0x27, 0x27, 0x27, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2C, 0x2D, 
    0x2D, 0x2D, 0x2E, 0x2E, 0x2E, 0x2F, 0x2F, 0x2F, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 
    0x32, 0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 
    0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x38,
    0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x3A, 0x3A, 
    0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 
    0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3D, 0x3D},   

    /* demosaic_weight_lut */
    {0x8, 0xF, 0x13, 0x16, 0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x23, 0x25, 0x25, 0x26, 
    0x27, 0x27, 0x27, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B, 0x2C, 0x2C, 0x2C, 0x2D, 0x2D, 0x2D, 
    0x2E, 0x2E, 0x2E, 0x2F, 0x2F, 0x2F, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x32, 0x32, 
    0x32, 0x32, 0x33, 0x33, 0x33, 0x33, 0x33, 0x34, 0x34, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 0x35, 0x35, 
    0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x38, 0x38, 0x38, 
    0x38, 0x38, 0x38, 0x38, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x3A, 0x3A, 0x3A, 0x3A, 
    0x3A, 0x3A, 0x3A, 0x3A, 0x3A, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3B, 0x3C, 0x3C, 
    0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3D, 0x3D, 0x3D, 0x3D}        
    
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xbb,

    /*aa_slope*/
    0xb8,

    /*va_slope*/
    0xb4,

    /*uu_slope*/
    0xb8,

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
        0,   1,   2,   4,   8,  12,  17,  23,  30,  38,  47,  57,  68,  79,  92, 105, 120, 133, 147, 161,
        176, 192, 209, 226, 243, 260, 278, 296, 315, 333, 351, 370, 390, 410, 431, 453, 474, 494, 515,
        536, 558, 580, 602, 623, 644, 665, 686, 708, 730, 751, 773, 795, 818, 840, 862, 884, 907, 929,
        951, 974, 998,1024,1051,1073,1096,1117,1139,1159,1181,1202,1223,1243,1261,1275,1293,1313,
        1332,1351,1371,1389,1408,1427,1446,1464,1482,1499,1516,1533,1549,1567,1583,1600,1616,1633,
        1650,1667,1683,1700,1716,1732,1749,1766,1782,1798,1815,1831,1847,1863,1880,1896,1912,1928,
        1945,1961,1977,1993,2009,2025,2041,2057,2073,2089,2104,2121,2137,2153,2168,2184,2200,2216,
        2231,2248,2263,2278,2294,2310,2326,2341,2357,2373,2388,2403,2419,2434,2450,2466,2481,2496,
        2512,2527,2543,2558,2573,2589,2604,2619,2635,2650,2665,2680,2696,2711,2726,2741,2757,2771,
        2787,2801,2817,2832,2847,2862,2877,2892,2907,2922,2937,2952,2967,2982,2997,3012,3027,3041,
        3057,3071,3086,3101,3116,3130,3145,3160,3175,3190,3204,3219,3234,3248,3263,3278,3293,3307,
        3322,3337,3351,3365,3380,3394,3409,3424,3438,3453,3468,3482,3497,3511,3525,3540,3554,3569,
        3584,3598,3612,3626,3641,3655,3670,3684,3699,3713,3727,3742,3756,3770,3784,3799,3813,3827,
        3841,3856,3870,3884,3898,3912,3927,3941,3955,3969,3983,3997,4011,4026,4039,4054,4068,4082,
        4095
    }
#else  /*higher  contrast*/
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


HI_U32 ov4689_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
        case WDR_MODE_2To1_LINE:
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

            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));        
            memcpy(&pstDef->stGamma, &g_stIspGammaFSWDR, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stGammafe, &g_stGammafeFSWDR, sizeof(ISP_CMOS_GAMMAFE_S));
        break;

    }
    
    pstDef->stSensorMaxResolution.u32MaxWidth  = 2592;
    pstDef->stSensorMaxResolution.u32MaxHeight = 1520;
    
    return 0;
}
#endif

HI_U32 ov4689_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
          
    for (i=0; i<4; i++)
    {
        pstBlackLevel->au16BlackLevel[i] = 0x3f; /*12bit,0x40 */
    }

    return 0;    
}

HI_VOID ov4689_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps, u32MaxIntTime_5Fps;
    
    if (WDR_MODE_2To1_LINE == genSensorMode)
    {   
        return;
    }
    else
    {
        if (SENSOR_4M_30FPS_MODE == gu8SensorImageMode)
        {
            u32FullLines_5Fps = (OV4689_VMAX_4M30 * 30) / 5;
        }
        else if (SENSOR_1080P_60FPS_MODE == gu8SensorImageMode)
        {
            u32FullLines_5Fps = (OV4689_VMAX_1080P * 60) / 5;
        }
        else  if (SENSOR_1080P_30FPS_MODE == gu8SensorImageMode)
        {
            u32FullLines_5Fps = (OV4689_VMAX_1080P * 30) / 5;
        }
        else  if (SENSOR_2304_1296_30FPS_MODE == gu8SensorImageMode)
        {
            u32FullLines_5Fps = (OV4689_VMAX_2304_1296 * 30) / 5;
        }
        else  if (SENSOR_2048_1520_30FPS_MODE == gu8SensorImageMode)
        {
            u32FullLines_5Fps = (OV4689_VMAX_2048_1520 * 30) / 5;
        }
        else  if (SENSOR_720p_120FPS_MODE == gu8SensorImageMode)
        {
            u32FullLines_5Fps = (OV4689_VMAX_720P * 120) / 5;
        }
        else  if (SENSOR_720p_180FPS_MODE == gu8SensorImageMode)
        {
            u32FullLines_5Fps = (OV4689_VMAX_720P * 180) / 5;
        }
        else
        {
            return;
        }
    }

    u32FullLines_5Fps = (u32FullLines_5Fps > 0xFFFF) ? 0xFFFF : u32FullLines_5Fps;
    u32MaxIntTime_5Fps = u32FullLines_5Fps - 4;

    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE (0x380F, u32FullLines_5Fps & 0xFF); 
        SENSOR_I2C_WRITE (0x380E, (u32FullLines_5Fps & 0x7F00) >> 8); 
        SENSOR_I2C_WRITE(0x3500, (u32MaxIntTime_5Fps>>12));
        SENSOR_I2C_WRITE(0x3501, ((u32MaxIntTime_5Fps & 0xFF0) >> 4));
        SENSOR_I2C_WRITE(0x3502, ((u32MaxIntTime_5Fps & 0xF) << 4));
        
        SENSOR_I2C_WRITE(0x3508, 0x00);
        SENSOR_I2C_WRITE(0x3509, 0x80);            
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE (0x380F, gu32FullLinesStd& 0xFF); 
        SENSOR_I2C_WRITE (0x380E, (gu32FullLinesStd & 0x7F00) >> 8); 
		bInit = HI_FALSE;

	}

    return;
}

HI_VOID ov4689_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            genSensorMode = WDR_MODE_NONE;
            printf("linear mode\n");
        break;

        case WDR_MODE_2To1_LINE:
            genSensorMode = WDR_MODE_2To1_LINE;
            printf("2to1 line WDR mode\n");
        break;

        case WDR_MODE_2To1_FRAME:
            genSensorMode = WDR_MODE_2To1_FRAME;
        break;

        case WDR_MODE_2To1_FRAME_FULL_RATE:
            genSensorMode = WDR_MODE_2To1_FRAME_FULL_RATE;
        break;
        
        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    
    return;
}

static HI_S32 ov4689_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;   
    HI_U8 u8MaxFrameRate = 30;
    bInit = HI_FALSE;
    
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }
    if((pstSensorImageMode->u16Width <= 1280)&&(pstSensorImageMode->u16Height <= 720))
    {
        if (pstSensorImageMode->f32Fps <= 120)
        {
            u8SensorImageMode = SENSOR_720p_120FPS_MODE;
        }
        else if (pstSensorImageMode->f32Fps <= 180)
        {
            u8SensorImageMode = SENSOR_720p_180FPS_MODE;
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
    else if((pstSensorImageMode->u16Width <= 1920)&&(pstSensorImageMode->u16Height <= 1080))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_1080P_30FPS_MODE;
        }
        else if (pstSensorImageMode->f32Fps <= 60)
        {
            u8SensorImageMode = SENSOR_1080P_60FPS_MODE;
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
    else if((pstSensorImageMode->u16Width <= 2048)&&(pstSensorImageMode->u16Height <= 1520))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_2048_1520_30FPS_MODE;
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
            u8SensorImageMode = SENSOR_2304_1296_30FPS_MODE;
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
    else if((pstSensorImageMode->u16Width <= 2592)&&(pstSensorImageMode->u16Height <= 1520))
    {
		if(WDR_MODE_2To1_LINE == genSensorMode)
	    {  
	        u8MaxFrameRate = 25;    
		}       
		else 
		{         
			u8MaxFrameRate = 30;   
		}     
		
		if (pstSensorImageMode->f32Fps <= u8MaxFrameRate)
		{
            u8SensorImageMode = SENSOR_4M_30FPS_MODE;
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

HI_U32 ov4689_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;        
        g_stSnsRegsInfo.u32RegNum = 7;
        
        if ((WDR_MODE_2To1_LINE == genSensorMode) || (WDR_MODE_2To1_FRAME == genSensorMode) || (WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode))
        {
            g_stSnsRegsInfo.u32RegNum += 5; 
        }
        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }        
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3500;
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3501;
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3502;
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3508;
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 1;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x3509;
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x380F;
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x380E;
        if (WDR_MODE_2To1_LINE == genSensorMode )
        {
            g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x350A;
            g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x350B;
            g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[7].u32RegAddr = 0x350C;
            g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[8].u32RegAddr = 0x350E;
            g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[9].u32RegAddr = 0x350F;
            g_stSnsRegsInfo.astI2cData[10].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[10].u32RegAddr = 0x380F;
            g_stSnsRegsInfo.astI2cData[11].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[11].u32RegAddr = 0x380E;
        }    
        else if ((WDR_MODE_2To1_FRAME == genSensorMode) || (WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode))
        {
            g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x3500;
            g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x3501;
            g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[7].u32RegAddr = 0x3502;
            g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[8].u32RegAddr = 0x350E;
            g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[9].u32RegAddr = 0x350F;
            g_stSnsRegsInfo.astI2cData[10].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[10].u32RegAddr = 0x380F;
            g_stSnsRegsInfo.astI2cData[11].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[11].u32RegAddr = 0x380E;            
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
/*
static int  ov4689_set_inifile_path(const char *pcPath)
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
*/
HI_VOID ov4689_global_init()
{   
    gu8SensorImageMode = SENSOR_4M_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;

    gu32FullLinesStd = 1632; 
    gu32FullLines = 1632;
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

  

//////////////////////////////sensor_init//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


static void ov4689_delay_ms(int ms) { 
    usleep(ms*1000);
}


void ov4689_init_4M50();
void ov4689_init_4M30();
void ov4689_wdr_4M_25_init();
void ov4689_wdr_4M_15_init();
void ov4689_wdr_1080p30_init();
void ov4689_wdr_2304_1296_30_init();
void ov4689_wdr_2048_1520_30_init();
void ov4689_linear_2304_1296_30_init();
void ov4689_linear_2048_1520_30_init();
void ov4689_linear_1080p60_init();
void ov4689_linear_1080p30_init();
void ov4689_linear_720p_120fps_init();
void ov4689_linear_720p_180fps_init();

void ov4689_init()
{
//    ov4689_i2c_init();
    switch (gu8SensorImageMode)
    {
        case 0: // 4M30
           if(WDR_MODE_2To1_LINE == genSensorMode)
            {
                ov4689_wdr_4M_25_init();
            }
            else if(WDR_MODE_NONE == genSensorMode)
            {
                ov4689_init_4M30();
            }else if((WDR_MODE_2To1_FRAME_FULL_RATE == genSensorMode) ||WDR_MODE_2To1_FRAME == genSensorMode)
            {
                ov4689_init_4M30();
            }            
        break;
        
        case 1: // 1080P60
            ov4689_linear_1080p60_init();
// //       break;
        case 2: // 1080P30
            if(WDR_MODE_2To1_LINE == genSensorMode)
            {
                ov4689_wdr_1080p30_init();
            }
            else
            {
                ov4689_linear_1080p30_init();
            }
            break;
        case 3: // 2304*1296 30
            if(WDR_MODE_2To1_LINE == genSensorMode)
            {
                ov4689_wdr_2304_1296_30_init();
            }
            else
            {
                ov4689_linear_2304_1296_30_init();
            }
            break;
        case 4: // 2048*1520 30
            if(WDR_MODE_2To1_LINE == genSensorMode)
            {
                ov4689_wdr_2048_1520_30_init();
            }
            else
            {
                ov4689_linear_2048_1520_30_init();
            }
            break;

        case 5: // 720 120
            if(WDR_MODE_NONE == genSensorMode)
            {
                ov4689_linear_720p_120fps_init();
            }
            else
            {
                
            }
        break;

        case 6: // 720 180
            if(WDR_MODE_NONE == genSensorMode)
            {
                ov4689_linear_720p_180fps_init();
            }
            else
            {
                
            }
        break;
        default:
            printf("Not support this mode\n");
    }
    
}

void ov4689_exit()
{
  //  ov4689_i2c_exit();

    return;
}

void ov4689_enable_wdr_mode()
{
 SENSOR_I2C_WRITE(0x3841, 0x03);                                                                                    
    SENSOR_I2C_WRITE(0x3846, 0x08);                                                                                      
    SENSOR_I2C_WRITE(0x3847, 0x06);                                                                                       
    SENSOR_I2C_WRITE(0x4800, 0x0C);                                                                                     
    SENSOR_I2C_WRITE(0x376e, 0x01);                                                                                         
                                                           
    SENSOR_I2C_WRITE(0x350B, 0x08);                                                                                       
    SENSOR_I2C_WRITE(0x3511, 0x01);                                                                                    
    SENSOR_I2C_WRITE(0x3517, 0x00);                                                                                    
    SENSOR_I2C_WRITE(0x351d, 0x00);  
              
    SENSOR_I2C_WRITE(0x3847, 0x06); 

    SENSOR_I2C_WRITE(0x0100, 0x01);
    SENSOR_I2C_WRITE(0x301a, 0xf9);

    SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    SENSOR_I2C_WRITE(0x301a, 0xf0);
}
void ov4689_init_comm_first()
{
    SENSOR_I2C_WRITE(0x3020, 0x93);
    SENSOR_I2C_WRITE(0x3021, 0x03);
    SENSOR_I2C_WRITE(0x3022, 0x01);
    SENSOR_I2C_WRITE(0x3031, 0x0a);
    SENSOR_I2C_WRITE(0x3305, 0xf1);
    SENSOR_I2C_WRITE(0x3307, 0x04);
    SENSOR_I2C_WRITE(0x3309, 0x29);
    SENSOR_I2C_WRITE(0x3500, 0x00);
    SENSOR_I2C_WRITE(0x3501, 0x45);
    SENSOR_I2C_WRITE(0x3502, 0xB0);
    SENSOR_I2C_WRITE(0x3503, 0x77);
    SENSOR_I2C_WRITE(0x3504, 0x00);
    SENSOR_I2C_WRITE(0x3505, 0x00);
    SENSOR_I2C_WRITE(0x3506, 0x00);
    SENSOR_I2C_WRITE(0x3507, 0x00);
    SENSOR_I2C_WRITE(0x3508, 0x00);
    SENSOR_I2C_WRITE(0x3509, 0x80);
    SENSOR_I2C_WRITE(0x350a, 0x00);
    SENSOR_I2C_WRITE(0x350b, 0x00);
    SENSOR_I2C_WRITE(0x350c, 0x00);
    SENSOR_I2C_WRITE(0x350d, 0x00);
    SENSOR_I2C_WRITE(0x350e, 0x00);
    SENSOR_I2C_WRITE(0x350f, 0x80);
    SENSOR_I2C_WRITE(0x3510, 0x00);
    SENSOR_I2C_WRITE(0x3511, 0x00);
    SENSOR_I2C_WRITE(0x3512, 0x00);
    SENSOR_I2C_WRITE(0x3513, 0x00);
    SENSOR_I2C_WRITE(0x3514, 0x00);
    SENSOR_I2C_WRITE(0x3515, 0x80);
    SENSOR_I2C_WRITE(0x3516, 0x00);
    SENSOR_I2C_WRITE(0x3517, 0x00);
    SENSOR_I2C_WRITE(0x3518, 0x00);
    SENSOR_I2C_WRITE(0x3519, 0x00);
    SENSOR_I2C_WRITE(0x351a, 0x00);
    SENSOR_I2C_WRITE(0x351b, 0x80);
    SENSOR_I2C_WRITE(0x351c, 0x00);
    SENSOR_I2C_WRITE(0x351d, 0x00);
    SENSOR_I2C_WRITE(0x351e, 0x00);
    SENSOR_I2C_WRITE(0x351f, 0x00);
    SENSOR_I2C_WRITE(0x3520, 0x00);
    SENSOR_I2C_WRITE(0x3521, 0x80);
    SENSOR_I2C_WRITE(0x3522, 0x08);
    SENSOR_I2C_WRITE(0x3524, 0x08);
    SENSOR_I2C_WRITE(0x3526, 0x08);
    SENSOR_I2C_WRITE(0x3528, 0x08);
    SENSOR_I2C_WRITE(0x352a, 0x08);
    SENSOR_I2C_WRITE(0x3602, 0x00);
    SENSOR_I2C_WRITE(0x3604, 0x02);
    SENSOR_I2C_WRITE(0x3605, 0x00);
    SENSOR_I2C_WRITE(0x3606, 0x00);
    SENSOR_I2C_WRITE(0x3607, 0x00);
    SENSOR_I2C_WRITE(0x3609, 0x12);
    SENSOR_I2C_WRITE(0x360a, 0x40);
    SENSOR_I2C_WRITE(0x360c, 0x08);
    SENSOR_I2C_WRITE(0x360f, 0xe5);
    SENSOR_I2C_WRITE(0x3608, 0x8f);
    SENSOR_I2C_WRITE(0x3611, 0x00);
    SENSOR_I2C_WRITE(0x3613, 0xf7);
    SENSOR_I2C_WRITE(0x3616, 0x58);
    SENSOR_I2C_WRITE(0x3619, 0x99);
    SENSOR_I2C_WRITE(0x361b, 0x60);
    SENSOR_I2C_WRITE(0x361c, 0x7a);
    SENSOR_I2C_WRITE(0x361e, 0x79);
    SENSOR_I2C_WRITE(0x361f, 0x02);
    SENSOR_I2C_WRITE(0x3632, 0x00);
    SENSOR_I2C_WRITE(0x3633, 0x10);
    SENSOR_I2C_WRITE(0x3634, 0x10);
    SENSOR_I2C_WRITE(0x3635, 0x10);
    SENSOR_I2C_WRITE(0x3636, 0x15);
    SENSOR_I2C_WRITE(0x3646, 0x86);
    SENSOR_I2C_WRITE(0x364a, 0x0b);
    SENSOR_I2C_WRITE(0x3700, 0x17);
    SENSOR_I2C_WRITE(0x3701, 0x22);
    SENSOR_I2C_WRITE(0x3703, 0x10);
    SENSOR_I2C_WRITE(0x370a, 0x37);
    SENSOR_I2C_WRITE(0x3705, 0x00);
    SENSOR_I2C_WRITE(0x3706, 0x63);
    SENSOR_I2C_WRITE(0x3709, 0x3c);
    SENSOR_I2C_WRITE(0x370b, 0x01);
    SENSOR_I2C_WRITE(0x370c, 0x30);
    SENSOR_I2C_WRITE(0x3710, 0x24);
    SENSOR_I2C_WRITE(0x3711, 0x0c);
    SENSOR_I2C_WRITE(0x3716, 0x00);
    SENSOR_I2C_WRITE(0x3720, 0x28);
    SENSOR_I2C_WRITE(0x3729, 0x7b);
    SENSOR_I2C_WRITE(0x372a, 0x84);
    SENSOR_I2C_WRITE(0x372b, 0xbd);
    SENSOR_I2C_WRITE(0x372c, 0xbc);
    SENSOR_I2C_WRITE(0x372e, 0x52);
    SENSOR_I2C_WRITE(0x373c, 0x0e);
    SENSOR_I2C_WRITE(0x373e, 0x33);
    SENSOR_I2C_WRITE(0x3743, 0x10);
    SENSOR_I2C_WRITE(0x3744, 0x88);
    SENSOR_I2C_WRITE(0x3745, 0xc0);
    SENSOR_I2C_WRITE(0x374a, 0x43);
    SENSOR_I2C_WRITE(0x374c, 0x00);
    SENSOR_I2C_WRITE(0x374e, 0x23);
    SENSOR_I2C_WRITE(0x3751, 0x7b);
    SENSOR_I2C_WRITE(0x3752, 0x84);
    SENSOR_I2C_WRITE(0x3753, 0xbd);
    SENSOR_I2C_WRITE(0x3754, 0xbc);
    SENSOR_I2C_WRITE(0x3756, 0x52);
    SENSOR_I2C_WRITE(0x375c, 0x00);
    SENSOR_I2C_WRITE(0x3760, 0x00);
    SENSOR_I2C_WRITE(0x3761, 0x00);
    SENSOR_I2C_WRITE(0x3762, 0x00);
    SENSOR_I2C_WRITE(0x3763, 0x00);
    SENSOR_I2C_WRITE(0x3764, 0x00);
    SENSOR_I2C_WRITE(0x3767, 0x04);
    SENSOR_I2C_WRITE(0x3768, 0x04);
    SENSOR_I2C_WRITE(0x3769, 0x08);
    SENSOR_I2C_WRITE(0x376a, 0x08);
    SENSOR_I2C_WRITE(0x376b, 0x20);
    SENSOR_I2C_WRITE(0x376c, 0x00);
    SENSOR_I2C_WRITE(0x376d, 0x00);
    SENSOR_I2C_WRITE(0x376e, 0x00);
    SENSOR_I2C_WRITE(0x3773, 0x00);
    SENSOR_I2C_WRITE(0x3774, 0x51);
    SENSOR_I2C_WRITE(0x3776, 0xbd);
    SENSOR_I2C_WRITE(0x3777, 0xbd);
    SENSOR_I2C_WRITE(0x3781, 0x18);
    SENSOR_I2C_WRITE(0x3783, 0x25);   
    printf("fist init ok!\n");
}

void ov4689_init_comm_second()
{
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x08);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);
    SENSOR_I2C_WRITE(0x4020, 0x00);
    SENSOR_I2C_WRITE(0x4021, 0x10);
    SENSOR_I2C_WRITE(0x4022, 0x06);
    SENSOR_I2C_WRITE(0x4023, 0x13);
    SENSOR_I2C_WRITE(0x4024, 0x07);
    SENSOR_I2C_WRITE(0x4025, 0x40);
    SENSOR_I2C_WRITE(0x4026, 0x07);
    SENSOR_I2C_WRITE(0x4027, 0x50);
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    SENSOR_I2C_WRITE(0x4601, 0x41);
    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x56);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    //SENSOR_I2C_WRITE(0x0100, 0x01);
    ov4689_delay_ms(10);
    SENSOR_I2C_WRITE(0x3105, 0x11);
    //SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    //SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);
    printf("second init ok\n");
}

void ov4689_init_4M50()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01);

    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x0300, 0x00);
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0x19);
    SENSOR_I2C_WRITE(0x0303, 0x00);
    SENSOR_I2C_WRITE(0x0304, 0x03);
    SENSOR_I2C_WRITE(0x0305, 0x01);
    SENSOR_I2C_WRITE(0x0306, 0x01);
    SENSOR_I2C_WRITE(0x030A, 0x00);
    SENSOR_I2C_WRITE(0x030b, 0x00);
    SENSOR_I2C_WRITE(0x030c, 0x00);
    SENSOR_I2C_WRITE(0x030d, 0x1e);
    SENSOR_I2C_WRITE(0x030e, 0x04);
    SENSOR_I2C_WRITE(0x030f, 0x01);
    //SENSOR_I2C_WRITE(0x030f, 0x03);
    
    SENSOR_I2C_WRITE(0x0311, 0x00);
    SENSOR_I2C_WRITE(0x0312, 0x01);
    SENSOR_I2C_WRITE(0x031e, 0x00);
    SENSOR_I2C_WRITE(0x3000, 0x20);
    SENSOR_I2C_WRITE(0x3002, 0x00);
    SENSOR_I2C_WRITE(0x3018, 0x72);
    SENSOR_I2C_WRITE(0x3020, 0x93);
    SENSOR_I2C_WRITE(0x3021, 0x03);
    SENSOR_I2C_WRITE(0x3022, 0x01);
    SENSOR_I2C_WRITE(0x3031, 0x0a);
    SENSOR_I2C_WRITE(0x3305, 0xf1);
    SENSOR_I2C_WRITE(0x3307, 0x04);
    SENSOR_I2C_WRITE(0x3309, 0x29);
    SENSOR_I2C_WRITE(0x3500, 0x00);
    SENSOR_I2C_WRITE(0x3501, 0x45);
    SENSOR_I2C_WRITE(0x3502, 0xB0);
    SENSOR_I2C_WRITE(0x3503, 0x77);
    SENSOR_I2C_WRITE(0x3504, 0x00);
    SENSOR_I2C_WRITE(0x3505, 0x00);
    SENSOR_I2C_WRITE(0x3506, 0x00);
    SENSOR_I2C_WRITE(0x3507, 0x00);
    SENSOR_I2C_WRITE(0x3508, 0x00);
    SENSOR_I2C_WRITE(0x3509, 0x80);
    SENSOR_I2C_WRITE(0x350a, 0x00);
    SENSOR_I2C_WRITE(0x350b, 0x00);
    SENSOR_I2C_WRITE(0x350c, 0x00);
    SENSOR_I2C_WRITE(0x350d, 0x00);
    SENSOR_I2C_WRITE(0x350e, 0x00);
    SENSOR_I2C_WRITE(0x350f, 0x80);
    SENSOR_I2C_WRITE(0x3510, 0x00);
    SENSOR_I2C_WRITE(0x3511, 0x00);
    SENSOR_I2C_WRITE(0x3512, 0x00);
    SENSOR_I2C_WRITE(0x3513, 0x00);
    SENSOR_I2C_WRITE(0x3514, 0x00);
    SENSOR_I2C_WRITE(0x3515, 0x80);
    SENSOR_I2C_WRITE(0x3516, 0x00);
    SENSOR_I2C_WRITE(0x3517, 0x00);
    SENSOR_I2C_WRITE(0x3518, 0x00);
    SENSOR_I2C_WRITE(0x3519, 0x00);
    SENSOR_I2C_WRITE(0x351a, 0x00);
    SENSOR_I2C_WRITE(0x351b, 0x80);
    SENSOR_I2C_WRITE(0x351c, 0x00);
    SENSOR_I2C_WRITE(0x351d, 0x00);
    SENSOR_I2C_WRITE(0x351e, 0x00);
    SENSOR_I2C_WRITE(0x351f, 0x00);
    SENSOR_I2C_WRITE(0x3520, 0x00);
    SENSOR_I2C_WRITE(0x3521, 0x80);
    SENSOR_I2C_WRITE(0x3522, 0x08);
    SENSOR_I2C_WRITE(0x3524, 0x08);
    SENSOR_I2C_WRITE(0x3526, 0x08);
    SENSOR_I2C_WRITE(0x3528, 0x08);
    SENSOR_I2C_WRITE(0x352a, 0x08);
    SENSOR_I2C_WRITE(0x3602, 0x00);
    SENSOR_I2C_WRITE(0x3604, 0x02);
    SENSOR_I2C_WRITE(0x3605, 0x00);
    SENSOR_I2C_WRITE(0x3606, 0x00);
    SENSOR_I2C_WRITE(0x3607, 0x00);
    SENSOR_I2C_WRITE(0x3609, 0x12);
    SENSOR_I2C_WRITE(0x360a, 0x40);
    SENSOR_I2C_WRITE(0x360c, 0x08);
    SENSOR_I2C_WRITE(0x360f, 0xe5);
    SENSOR_I2C_WRITE(0x3608, 0x8f);
    SENSOR_I2C_WRITE(0x3611, 0x00);
    SENSOR_I2C_WRITE(0x3613, 0xf7);
    SENSOR_I2C_WRITE(0x3616, 0x58);
    SENSOR_I2C_WRITE(0x3619, 0x99);
    SENSOR_I2C_WRITE(0x361b, 0x60);
    SENSOR_I2C_WRITE(0x361c, 0x7a);
    SENSOR_I2C_WRITE(0x361e, 0x79);
    SENSOR_I2C_WRITE(0x361f, 0x02);
    SENSOR_I2C_WRITE(0x3632, 0x00);
    SENSOR_I2C_WRITE(0x3633, 0x10);
    SENSOR_I2C_WRITE(0x3634, 0x10);
    SENSOR_I2C_WRITE(0x3635, 0x10);
    SENSOR_I2C_WRITE(0x3636, 0x15);
    SENSOR_I2C_WRITE(0x3646, 0x86);
    SENSOR_I2C_WRITE(0x364a, 0x0b);
    SENSOR_I2C_WRITE(0x3700, 0x17);
    SENSOR_I2C_WRITE(0x3701, 0x22);
    SENSOR_I2C_WRITE(0x3703, 0x10);
    SENSOR_I2C_WRITE(0x370a, 0x37);
    SENSOR_I2C_WRITE(0x3705, 0x00);
    SENSOR_I2C_WRITE(0x3706, 0x63);
    SENSOR_I2C_WRITE(0x3709, 0x3c);
    SENSOR_I2C_WRITE(0x370b, 0x01);
    SENSOR_I2C_WRITE(0x370c, 0x30);
    SENSOR_I2C_WRITE(0x3710, 0x24);
    SENSOR_I2C_WRITE(0x3711, 0x0c);
    SENSOR_I2C_WRITE(0x3716, 0x00);
    SENSOR_I2C_WRITE(0x3720, 0x28);
    SENSOR_I2C_WRITE(0x3729, 0x7b);
    SENSOR_I2C_WRITE(0x372a, 0x84);
    SENSOR_I2C_WRITE(0x372b, 0xbd);
    SENSOR_I2C_WRITE(0x372c, 0xbc);
    SENSOR_I2C_WRITE(0x372e, 0x52);
    SENSOR_I2C_WRITE(0x373c, 0x0e);
    SENSOR_I2C_WRITE(0x373e, 0x33);
    SENSOR_I2C_WRITE(0x3743, 0x10);
    SENSOR_I2C_WRITE(0x3744, 0x88);
    SENSOR_I2C_WRITE(0x3745, 0xc0);
    SENSOR_I2C_WRITE(0x374a, 0x43);
    SENSOR_I2C_WRITE(0x374c, 0x00);
    SENSOR_I2C_WRITE(0x374e, 0x23);
    SENSOR_I2C_WRITE(0x3751, 0x7b);
    SENSOR_I2C_WRITE(0x3752, 0x84);
    SENSOR_I2C_WRITE(0x3753, 0xbd);
    SENSOR_I2C_WRITE(0x3754, 0xbc);
    SENSOR_I2C_WRITE(0x3756, 0x52);
    SENSOR_I2C_WRITE(0x375c, 0x00);
    SENSOR_I2C_WRITE(0x3760, 0x00);
    SENSOR_I2C_WRITE(0x3761, 0x00);
    SENSOR_I2C_WRITE(0x3762, 0x00);
    SENSOR_I2C_WRITE(0x3763, 0x00);
    SENSOR_I2C_WRITE(0x3764, 0x00);
    SENSOR_I2C_WRITE(0x3767, 0x04);
    SENSOR_I2C_WRITE(0x3768, 0x04);
    SENSOR_I2C_WRITE(0x3769, 0x08);
    SENSOR_I2C_WRITE(0x376a, 0x08);
    SENSOR_I2C_WRITE(0x376b, 0x20);
    SENSOR_I2C_WRITE(0x376c, 0x00);
    SENSOR_I2C_WRITE(0x376d, 0x00);
    SENSOR_I2C_WRITE(0x376e, 0x00);
    SENSOR_I2C_WRITE(0x3773, 0x00);
    SENSOR_I2C_WRITE(0x3774, 0x51);
    SENSOR_I2C_WRITE(0x3776, 0xbd);
    SENSOR_I2C_WRITE(0x3777, 0xbd);
    SENSOR_I2C_WRITE(0x3781, 0x18);
    SENSOR_I2C_WRITE(0x3783, 0x25);
    SENSOR_I2C_WRITE(0x3800, 0x00);
    SENSOR_I2C_WRITE(0x3801, 0x38);
    SENSOR_I2C_WRITE(0x3802, 0x00);
    SENSOR_I2C_WRITE(0x3803, 0x04);
    SENSOR_I2C_WRITE(0x3804, 0x0a);
    SENSOR_I2C_WRITE(0x3805, 0x67);
    SENSOR_I2C_WRITE(0x3806, 0x05);
    SENSOR_I2C_WRITE(0x3807, 0xfb);
    SENSOR_I2C_WRITE(0x3808, 0x0a);
    SENSOR_I2C_WRITE(0x3809, 0x20);
    SENSOR_I2C_WRITE(0x380a, 0x05);
    SENSOR_I2C_WRITE(0x380b, 0xf0);

    SENSOR_I2C_WRITE(0x380c, 0x05);
    SENSOR_I2C_WRITE(0x380d, 0xc0);
     //SENSOR_I2C_WRITE(0x380c, 0x07);
    //SENSOR_I2C_WRITE(0x380d, 0x30);

    SENSOR_I2C_WRITE(0x380e, 0x06);
    SENSOR_I2C_WRITE(0x380f, 0x60);
    
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x08);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);
    SENSOR_I2C_WRITE(0x4020, 0x00);
    SENSOR_I2C_WRITE(0x4021, 0x10);
    SENSOR_I2C_WRITE(0x4022, 0x08);
    SENSOR_I2C_WRITE(0x4023, 0xB3);
    SENSOR_I2C_WRITE(0x4024, 0x09);
    SENSOR_I2C_WRITE(0x4025, 0xE0);
    SENSOR_I2C_WRITE(0x4026, 0x09);
    SENSOR_I2C_WRITE(0x4027, 0xF0);
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    SENSOR_I2C_WRITE(0x4601, 0x41);
    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x1a);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    //SENSOR_I2C_WRITE(0x0100, 0x01);

    SENSOR_I2C_WRITE(0x3105, 0x11);
    //SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    //SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);

	SENSOR_I2C_WRITE(0x4810,0x00);
    SENSOR_I2C_WRITE(0x4811,0x01);

    bSensorInit = HI_TRUE;
        
}
void ov4689_init_4M30()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103,0x01);//mode 1

    SENSOR_I2C_WRITE(0x3638,0x00);//analog control  

    SENSOR_I2C_WRITE(0x0300,0x02); //PLL    clk
    SENSOR_I2C_WRITE(0x0301,0x00);
    SENSOR_I2C_WRITE(0x0302,0xf4);    
    SENSOR_I2C_WRITE(0x0303,0x03);
    SENSOR_I2C_WRITE(0x0304,0x03);  
	
    SENSOR_I2C_WRITE(0x030A,0x01);    
    SENSOR_I2C_WRITE(0x030b,0x00);    
    SENSOR_I2C_WRITE(0x030d,0x1e);    
    SENSOR_I2C_WRITE(0x030e,0x04);    
    SENSOR_I2C_WRITE(0x030f,0x01);
    SENSOR_I2C_WRITE(0x0312,0x01);    
    SENSOR_I2C_WRITE(0x031e,0x00);   


	
    SENSOR_I2C_WRITE(0x3000,0x20);  //SC FSIN OUTPUT enable  
    SENSOR_I2C_WRITE(0x3002,0x00);  //input  
    SENSOR_I2C_WRITE(0x3018,0x72);  // 4line mode
    SENSOR_I2C_WRITE(0x3020,0x93);  // switch to pll clk
    SENSOR_I2C_WRITE(0x3021,0x03);    
    SENSOR_I2C_WRITE(0x3022,0x01); //enable MIPI when sleep   
    SENSOR_I2C_WRITE(0x3031,0x0a);   //10bit mode 

    SENSOR_I2C_WRITE(0x3305,0xf1);  // ASRAM control
    SENSOR_I2C_WRITE(0x3307,0x04);    
    SENSOR_I2C_WRITE(0x3309,0x29);   
	
    SENSOR_I2C_WRITE(0x3500,0x00);   //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ø¹ï¿?
    SENSOR_I2C_WRITE(0x3501,0x45);
    SENSOR_I2C_WRITE(0x3502,0xB0);    
    SENSOR_I2C_WRITE(0x3503,0x77);    
    SENSOR_I2C_WRITE(0x3504,0x00);    
    SENSOR_I2C_WRITE(0x3505,0x00);    
    SENSOR_I2C_WRITE(0x3506,0x00);    
    SENSOR_I2C_WRITE(0x3507,0x00);    
    SENSOR_I2C_WRITE(0x3508,0x00);    
    SENSOR_I2C_WRITE(0x3509,0x80);    
    SENSOR_I2C_WRITE(0x350a,0x00);    
    SENSOR_I2C_WRITE(0x350b,0x00);    
    SENSOR_I2C_WRITE(0x350c,0x00);    
    SENSOR_I2C_WRITE(0x350d,0x00);    
    SENSOR_I2C_WRITE(0x350e,0x00);    
    SENSOR_I2C_WRITE(0x350f,0x80);    
    SENSOR_I2C_WRITE(0x3510,0x00);    
    SENSOR_I2C_WRITE(0x3511,0x00);    
    SENSOR_I2C_WRITE(0x3512,0x00);    
    SENSOR_I2C_WRITE(0x3513,0x00);    
    SENSOR_I2C_WRITE(0x3514,0x00);    
    SENSOR_I2C_WRITE(0x3515,0x80);    
    SENSOR_I2C_WRITE(0x3516,0x00);    
    SENSOR_I2C_WRITE(0x3517,0x00);    
    SENSOR_I2C_WRITE(0x3518,0x00);    
    SENSOR_I2C_WRITE(0x3519,0x00);    
    SENSOR_I2C_WRITE(0x351a,0x00);    
    SENSOR_I2C_WRITE(0x351b,0x80);    
    SENSOR_I2C_WRITE(0x351c,0x00);    
    SENSOR_I2C_WRITE(0x351d,0x00);    
    SENSOR_I2C_WRITE(0x351e,0x00);    
    SENSOR_I2C_WRITE(0x351f,0x00);    
    SENSOR_I2C_WRITE(0x3520,0x00);    
    SENSOR_I2C_WRITE(0x3521,0x80);    
    SENSOR_I2C_WRITE(0x3522,0x08);    
    SENSOR_I2C_WRITE(0x3524,0x08);    
    SENSOR_I2C_WRITE(0x3526,0x08);    
    SENSOR_I2C_WRITE(0x3528,0x08);    
    SENSOR_I2C_WRITE(0x352a,0x08); 

	
    SENSOR_I2C_WRITE(0x3602,0x00);    
    SENSOR_I2C_WRITE(0x3604,0x02);    
    SENSOR_I2C_WRITE(0x3605,0x00);    
    SENSOR_I2C_WRITE(0x3606,0x00);    
    SENSOR_I2C_WRITE(0x3607,0x00);    
    SENSOR_I2C_WRITE(0x3609,0x12);    
    SENSOR_I2C_WRITE(0x360a,0x40);    
    SENSOR_I2C_WRITE(0x360c,0x08);    
    SENSOR_I2C_WRITE(0x360f,0xe5);    
    SENSOR_I2C_WRITE(0x3608,0x8f);    
    SENSOR_I2C_WRITE(0x3611,0x00);    
    SENSOR_I2C_WRITE(0x3613,0xf7);    
    SENSOR_I2C_WRITE(0x3616,0x58);    
    SENSOR_I2C_WRITE(0x3619,0x99);    
    SENSOR_I2C_WRITE(0x361b,0x60);    
    SENSOR_I2C_WRITE(0x361c,0x7a);    
    SENSOR_I2C_WRITE(0x361e,0x79);    
    SENSOR_I2C_WRITE(0x361f,0x02);    
    SENSOR_I2C_WRITE(0x3632,0x00);    
    SENSOR_I2C_WRITE(0x3633,0x10);    
    SENSOR_I2C_WRITE(0x3634,0x10);    
    SENSOR_I2C_WRITE(0x3635,0x10);    
    SENSOR_I2C_WRITE(0x3636,0x15);    
    SENSOR_I2C_WRITE(0x3646,0x86);    
    SENSOR_I2C_WRITE(0x364a,0x0b);  

	
    SENSOR_I2C_WRITE(0x3700,0x17);    
    SENSOR_I2C_WRITE(0x3701,0x22);    
    SENSOR_I2C_WRITE(0x3703,0x10);    
    SENSOR_I2C_WRITE(0x370a,0x37);    
    SENSOR_I2C_WRITE(0x3705,0x00);    
    SENSOR_I2C_WRITE(0x3706,0x63);    
    SENSOR_I2C_WRITE(0x3709,0x3c);    
    SENSOR_I2C_WRITE(0x370b,0x01);    
    SENSOR_I2C_WRITE(0x370c,0x30);    
    SENSOR_I2C_WRITE(0x3710,0x24);    
    SENSOR_I2C_WRITE(0x3711,0x0c);    
    SENSOR_I2C_WRITE(0x3716,0x00);    
    SENSOR_I2C_WRITE(0x3720,0x28);    
    SENSOR_I2C_WRITE(0x3729,0x7b);    
    SENSOR_I2C_WRITE(0x372a,0x84);    
    SENSOR_I2C_WRITE(0x372b,0xbd);    
    SENSOR_I2C_WRITE(0x372c,0xbc);    
    SENSOR_I2C_WRITE(0x372e,0x52);    
    SENSOR_I2C_WRITE(0x373c,0x0e);    
    SENSOR_I2C_WRITE(0x373e,0x33);    
    SENSOR_I2C_WRITE(0x3743,0x10);    
    SENSOR_I2C_WRITE(0x3744,0x88);
    SENSOR_I2C_WRITE(0x3745,0xc0);    
    SENSOR_I2C_WRITE(0x374a,0x43);    
    SENSOR_I2C_WRITE(0x374c,0x00);    
    SENSOR_I2C_WRITE(0x374e,0x23);    
    SENSOR_I2C_WRITE(0x3751,0x7b);    
    SENSOR_I2C_WRITE(0x3752,0x84);    
    SENSOR_I2C_WRITE(0x3753,0xbd);    
    SENSOR_I2C_WRITE(0x3754,0xbc);    
    SENSOR_I2C_WRITE(0x3756,0x52);    
    SENSOR_I2C_WRITE(0x375c,0x00);    
    SENSOR_I2C_WRITE(0x3760,0x00);    
    SENSOR_I2C_WRITE(0x3761,0x00);    
    SENSOR_I2C_WRITE(0x3762,0x00);    
    SENSOR_I2C_WRITE(0x3763,0x00);    
    SENSOR_I2C_WRITE(0x3764,0x00);    
    SENSOR_I2C_WRITE(0x3767,0x04);    
    SENSOR_I2C_WRITE(0x3768,0x04);    
    SENSOR_I2C_WRITE(0x3769,0x08);    
    SENSOR_I2C_WRITE(0x376a,0x08);    
    SENSOR_I2C_WRITE(0x376b,0x20);    
    SENSOR_I2C_WRITE(0x376c,0x00);    
    SENSOR_I2C_WRITE(0x376d,0x00);    
    SENSOR_I2C_WRITE(0x376e,0x00);    
    SENSOR_I2C_WRITE(0x3773,0x00);    
    SENSOR_I2C_WRITE(0x3774,0x51);    
    SENSOR_I2C_WRITE(0x3776,0xbd);    
    SENSOR_I2C_WRITE(0x3777,0xbd);    
    SENSOR_I2C_WRITE(0x3781,0x18);    
    SENSOR_I2C_WRITE(0x3783,0x25);  


	
    SENSOR_I2C_WRITE(0x3800,0x00);  //timing control  
    SENSOR_I2C_WRITE(0x3801,0x38);    
    SENSOR_I2C_WRITE(0x3802,0x00);    
    SENSOR_I2C_WRITE(0x3803,0x04);    
    SENSOR_I2C_WRITE(0x3804,0x0a);
    SENSOR_I2C_WRITE(0x3805,0x67);    
    SENSOR_I2C_WRITE(0x3806,0x05);    
    SENSOR_I2C_WRITE(0x3807,0xfb);    
    SENSOR_I2C_WRITE(0x3808,0x0a);    
    SENSOR_I2C_WRITE(0x3809,0x20);    
    SENSOR_I2C_WRITE(0x380a,0x05);
    SENSOR_I2C_WRITE(0x380b,0xf0);    
    SENSOR_I2C_WRITE(0x380c,0x09);    
    SENSOR_I2C_WRITE(0x380d,0x94);    
    SENSOR_I2C_WRITE(0x380e,0x06);    
    SENSOR_I2C_WRITE(0x380f,0x60);    
    SENSOR_I2C_WRITE(0x3810,0x00);    
    SENSOR_I2C_WRITE(0x3811,0x08);    
    SENSOR_I2C_WRITE(0x3812,0x00);    
    SENSOR_I2C_WRITE(0x3813,0x04);    
    SENSOR_I2C_WRITE(0x3814,0x01);    
    SENSOR_I2C_WRITE(0x3815,0x01);    
    SENSOR_I2C_WRITE(0x3819,0x01);    
    SENSOR_I2C_WRITE(0x3820,0x00);    
    SENSOR_I2C_WRITE(0x3821,0x06);    
    SENSOR_I2C_WRITE(0x3829,0x00);    
    SENSOR_I2C_WRITE(0x382a,0x01);    
    SENSOR_I2C_WRITE(0x382b,0x01);    
    SENSOR_I2C_WRITE(0x382d,0x7f);    
    SENSOR_I2C_WRITE(0x3830,0x04);    
    SENSOR_I2C_WRITE(0x3836,0x01);    
    SENSOR_I2C_WRITE(0x3841,0x02);    
    SENSOR_I2C_WRITE(0x3846,0x08);    
    SENSOR_I2C_WRITE(0x3847,0x07);    
    SENSOR_I2C_WRITE(0x3d85,0x36);    
    SENSOR_I2C_WRITE(0x3d8c,0x71);    
    SENSOR_I2C_WRITE(0x3d8d,0xcb);    
    SENSOR_I2C_WRITE(0x3f0a,0x00);  


	
    SENSOR_I2C_WRITE(0x4000,0x71);  //BLC register  
    SENSOR_I2C_WRITE(0x4001,0x40);    
    SENSOR_I2C_WRITE(0x4002,0x04);    
    SENSOR_I2C_WRITE(0x4003,0x14);    
    SENSOR_I2C_WRITE(0x400e,0x00);    
    SENSOR_I2C_WRITE(0x4011,0x00);    
    SENSOR_I2C_WRITE(0x401a,0x00);    
    SENSOR_I2C_WRITE(0x401b,0x00);    
    SENSOR_I2C_WRITE(0x401c,0x00);    
    SENSOR_I2C_WRITE(0x401d,0x00);    
    SENSOR_I2C_WRITE(0x401f,0x00);    
    SENSOR_I2C_WRITE(0x4020,0x00);    
    SENSOR_I2C_WRITE(0x4021,0x10);    
    SENSOR_I2C_WRITE(0x4022,0x06);    
    SENSOR_I2C_WRITE(0x4023,0x13);    
    SENSOR_I2C_WRITE(0x4024,0x07);    
    SENSOR_I2C_WRITE(0x4025,0x40);    
    SENSOR_I2C_WRITE(0x4026,0x07);    
    SENSOR_I2C_WRITE(0x4027,0x50);    
    SENSOR_I2C_WRITE(0x4028,0x00);    
    SENSOR_I2C_WRITE(0x4029,0x02);    
    SENSOR_I2C_WRITE(0x402a,0x06);    
    SENSOR_I2C_WRITE(0x402b,0x04);    
    SENSOR_I2C_WRITE(0x402c,0x02);    
    SENSOR_I2C_WRITE(0x402d,0x02);    
    SENSOR_I2C_WRITE(0x402e,0x0e);    
    SENSOR_I2C_WRITE(0x402f,0x04);

	
    SENSOR_I2C_WRITE(0x4302,0xff);    
    SENSOR_I2C_WRITE(0x4303,0xff);    
    SENSOR_I2C_WRITE(0x4304,0x00);    
    SENSOR_I2C_WRITE(0x4305,0x00);    
    SENSOR_I2C_WRITE(0x4306,0x00);    
    SENSOR_I2C_WRITE(0x4308,0x02);    
    SENSOR_I2C_WRITE(0x4500,0x6c);    
    SENSOR_I2C_WRITE(0x4501,0xc4);    
    SENSOR_I2C_WRITE(0x4502,0x40);    
    SENSOR_I2C_WRITE(0x4503,0x01);    
    SENSOR_I2C_WRITE(0x4601,0x41);    
    SENSOR_I2C_WRITE(0x4800,0x04);    
    SENSOR_I2C_WRITE(0x4813,0x08);    
    SENSOR_I2C_WRITE(0x481f,0x40);    
    SENSOR_I2C_WRITE(0x4829,0x78);    
    SENSOR_I2C_WRITE(0x4837,0x56);
    SENSOR_I2C_WRITE(0x4b00,0x2a);    
    SENSOR_I2C_WRITE(0x4b0d,0x00);    
    SENSOR_I2C_WRITE(0x4d00,0x04);    
    SENSOR_I2C_WRITE(0x4d01,0x42);    
    SENSOR_I2C_WRITE(0x4d02,0xd1);    
    SENSOR_I2C_WRITE(0x4d03,0x93);    
    SENSOR_I2C_WRITE(0x4d04,0xf5);    
    SENSOR_I2C_WRITE(0x4d05,0xc1);    
    SENSOR_I2C_WRITE(0x5000,0xf3);    
    SENSOR_I2C_WRITE(0x5001,0x11);    
    SENSOR_I2C_WRITE(0x5004,0x00);    
    SENSOR_I2C_WRITE(0x500a,0x00);    
    SENSOR_I2C_WRITE(0x500b,0x00);    
    SENSOR_I2C_WRITE(0x5032,0x00);    
    SENSOR_I2C_WRITE(0x5040,0x00);    
    SENSOR_I2C_WRITE(0x5050,0x0c);    
    SENSOR_I2C_WRITE(0x8000,0x00);    
    SENSOR_I2C_WRITE(0x8001,0x00);    
    SENSOR_I2C_WRITE(0x8002,0x00);    
    SENSOR_I2C_WRITE(0x8003,0x00);    
    SENSOR_I2C_WRITE(0x8004,0x00);    
    SENSOR_I2C_WRITE(0x8005,0x00);    
    SENSOR_I2C_WRITE(0x8006,0x00);    
    SENSOR_I2C_WRITE(0x8007,0x00);    
    SENSOR_I2C_WRITE(0x8008,0x00);    
    SENSOR_I2C_WRITE(0x3638,0x00);    
    SENSOR_I2C_WRITE(0x3105,0x31);    
    SENSOR_I2C_WRITE(0x301a,0xf9);    
    SENSOR_I2C_WRITE(0x3508,0x07);    
    SENSOR_I2C_WRITE(0x484b,0x05);    
    SENSOR_I2C_WRITE(0x4805,0x03);    
    SENSOR_I2C_WRITE(0x3601,0x01);    
    SENSOR_I2C_WRITE(0x0100,0x01);    

    usleep(10000);         
    SENSOR_I2C_WRITE(0x3105,0x11);    
    SENSOR_I2C_WRITE(0x301a,0xf1);    
    SENSOR_I2C_WRITE(0x4805,0x00);    
    SENSOR_I2C_WRITE(0x301a,0xf0);    
    SENSOR_I2C_WRITE(0x3208,0x00);    
    SENSOR_I2C_WRITE(0x302a,0x00);    
    SENSOR_I2C_WRITE(0x302a,0x00);    
    SENSOR_I2C_WRITE(0x302a,0x00);    
    SENSOR_I2C_WRITE(0x302a,0x00);    
    SENSOR_I2C_WRITE(0x302a,0x00);    
    SENSOR_I2C_WRITE(0x3601,0x00);    
    SENSOR_I2C_WRITE(0x3638,0x00);    
    SENSOR_I2C_WRITE(0x3208,0x10);    
    SENSOR_I2C_WRITE(0x3208,0xa0);

	SENSOR_I2C_WRITE(0x4810,0x00);
    SENSOR_I2C_WRITE(0x4811,0x01);
   
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 4M30fps Initial OK!-------\n");
}

void ov4689_wdr_4M_25_init()
{
    //line wdr mode 25fps
    ov4689_init_4M50();
    ov4689_enable_wdr_mode();
	bSensorInit = HI_TRUE;		
	printf("-------OV4689 Sensor 4M25fps  WDR Initial OK!-------\n");

}

void ov4689_wdr_4M_15_init()
{
    ov4689_init_4M30();
    ov4689_enable_wdr_mode();
    printf("--------ov4689 4M15fps  WDR init ok!---------\n");
}

void ov4689_linear_1080p60_init()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01);

    SENSOR_I2C_WRITE(0x3638, 0x00);    
    SENSOR_I2C_WRITE(0x0300, 0x02);    
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0xf4);    
    SENSOR_I2C_WRITE(0x0303, 0x03);
    SENSOR_I2C_WRITE(0x0304, 0x03);    
    SENSOR_I2C_WRITE(0x030A, 0x01);    
    SENSOR_I2C_WRITE(0x030b, 0x00);    
    SENSOR_I2C_WRITE(0x030d, 0x1e);    
    SENSOR_I2C_WRITE(0x030e, 0x04);    
    SENSOR_I2C_WRITE(0x030f, 0x01);
    SENSOR_I2C_WRITE(0x0312, 0x01);    
    SENSOR_I2C_WRITE(0x031e, 0x00);    
    SENSOR_I2C_WRITE(0x3000, 0x20);    
    SENSOR_I2C_WRITE(0x3002, 0x00);    
    SENSOR_I2C_WRITE(0x3018, 0x72);    
    ov4689_init_comm_first();    
    SENSOR_I2C_WRITE(0x3800, 0x01);    
    SENSOR_I2C_WRITE(0x3801, 0x88);    
    SENSOR_I2C_WRITE(0x3802, 0x00);    
    SENSOR_I2C_WRITE(0x3803, 0xe0);    
    SENSOR_I2C_WRITE(0x3804, 0x09);    
    SENSOR_I2C_WRITE(0x3805, 0x17);    
    SENSOR_I2C_WRITE(0x3806, 0x05);    
    SENSOR_I2C_WRITE(0x3807, 0x1f);    
    SENSOR_I2C_WRITE(0x3808, 0x07);    
    SENSOR_I2C_WRITE(0x3809, 0x80);    
    SENSOR_I2C_WRITE(0x380a, 0x04);    
    SENSOR_I2C_WRITE(0x380b, 0x38);    
    SENSOR_I2C_WRITE(0x380c, 0x06);    
    SENSOR_I2C_WRITE(0x380d, 0xF4);    
    SENSOR_I2C_WRITE(0x380e, 0x04);    
    SENSOR_I2C_WRITE(0x380f, 0x63);    
    //ov4689_init_comm_second();
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x08);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);
    SENSOR_I2C_WRITE(0x4020, 0x00);
    SENSOR_I2C_WRITE(0x4021, 0x10);
    SENSOR_I2C_WRITE(0x4022, 0x06);
    SENSOR_I2C_WRITE(0x4023, 0x13);
    SENSOR_I2C_WRITE(0x4024, 0x07);
    SENSOR_I2C_WRITE(0x4025, 0x40);
    SENSOR_I2C_WRITE(0x4026, 0x07);
    SENSOR_I2C_WRITE(0x4027, 0x50);
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    SENSOR_I2C_WRITE(0x4601, 0x77);
    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x56);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    SENSOR_I2C_WRITE(0x0100, 0x01);
    ov4689_delay_ms(10);
    SENSOR_I2C_WRITE(0x3105, 0x11);
    SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 1080P 60fps Linear Mode Initial OK!-------\n");
}

void ov4689_wdr_1080p30_init()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01);

    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x0300, 0x00);
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0x19);
    SENSOR_I2C_WRITE(0x0303, 0x00);
    SENSOR_I2C_WRITE(0x0304, 0x03);
    SENSOR_I2C_WRITE(0x0305, 0x01);
    SENSOR_I2C_WRITE(0x0306, 0x01);
    SENSOR_I2C_WRITE(0x030A, 0x00);
    SENSOR_I2C_WRITE(0x030b, 0x00);
    SENSOR_I2C_WRITE(0x030c, 0x00);
    SENSOR_I2C_WRITE(0x030d, 0x1e);
    SENSOR_I2C_WRITE(0x030e, 0x04);
    SENSOR_I2C_WRITE(0x030f, 0x01);
    SENSOR_I2C_WRITE(0x0311, 0x00);
    SENSOR_I2C_WRITE(0x0312, 0x01);
    SENSOR_I2C_WRITE(0x031e, 0x00);
    SENSOR_I2C_WRITE(0x3000, 0x20);
    SENSOR_I2C_WRITE(0x3002, 0x00);
    SENSOR_I2C_WRITE(0x3018, 0x72);
    ov4689_init_comm_first();
    SENSOR_I2C_WRITE(0x3800, 0x01);
    SENSOR_I2C_WRITE(0x3801, 0x88);
    SENSOR_I2C_WRITE(0x3802, 0x00);
    SENSOR_I2C_WRITE(0x3803, 0xe0);
    SENSOR_I2C_WRITE(0x3804, 0x09);
    SENSOR_I2C_WRITE(0x3805, 0x17);
    SENSOR_I2C_WRITE(0x3806, 0x05);
    SENSOR_I2C_WRITE(0x3807, 0x1f);
    SENSOR_I2C_WRITE(0x3808, 0x07);
    SENSOR_I2C_WRITE(0x3809, 0x80);
    SENSOR_I2C_WRITE(0x380a, 0x04);
    SENSOR_I2C_WRITE(0x380b, 0x38);
    SENSOR_I2C_WRITE(0x380c, 0x05);
    SENSOR_I2C_WRITE(0x380d, 0xF4);
    SENSOR_I2C_WRITE(0x380e, 0x05);
    SENSOR_I2C_WRITE(0x380f, 0x20);
    //ov4689_init_comm_second();
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x08);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);
    SENSOR_I2C_WRITE(0x4020, 0x00);
    SENSOR_I2C_WRITE(0x4021, 0x10);
    SENSOR_I2C_WRITE(0x4022, 0x06);
    SENSOR_I2C_WRITE(0x4023, 0x13);
    SENSOR_I2C_WRITE(0x4024, 0x07);
    SENSOR_I2C_WRITE(0x4025, 0x40);
    SENSOR_I2C_WRITE(0x4026, 0x07);
    SENSOR_I2C_WRITE(0x4027, 0x50);
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    SENSOR_I2C_WRITE(0x4601, 0x41);
    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x1a);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    //SENSOR_I2C_WRITE(0x0100, 0x01);
    ov4689_delay_ms(10);
    SENSOR_I2C_WRITE(0x3105, 0x11);
    //SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    //SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);

    
    SENSOR_I2C_WRITE(0x3841, 0x03);                                                                                    
    SENSOR_I2C_WRITE(0x3846, 0x08);                                                                                    
    SENSOR_I2C_WRITE(0x3847, 0x07);                                                                                    
    SENSOR_I2C_WRITE(0x4800, 0x0C);                                                                                    
    SENSOR_I2C_WRITE(0x376e, 0x01);                                                                                                               
    SENSOR_I2C_WRITE(0x350B, 0x08);                                                                                    
    SENSOR_I2C_WRITE(0x3511, 0x01);                                                                                    
    SENSOR_I2C_WRITE(0x3517, 0x00);                                                                                    
    SENSOR_I2C_WRITE(0x351d, 0x00);
    SENSOR_I2C_WRITE(0x3847, 0x07);

    SENSOR_I2C_WRITE(0x0100, 0x01);
    SENSOR_I2C_WRITE(0x301a, 0xf9);

    SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    SENSOR_I2C_WRITE(0x301a, 0xf0);
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 1080P 30fps WDR Mode Initial OK!-------\n");
}

void ov4689_linear_1080p30_init()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01);

    SENSOR_I2C_WRITE(0x3638, 0x00);   
    SENSOR_I2C_WRITE(0x0300, 0x02);   
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0x7C);   
    SENSOR_I2C_WRITE(0x0303, 0x03);
    SENSOR_I2C_WRITE(0x0304, 0x03);   
    SENSOR_I2C_WRITE(0x030A, 0x01);   
    SENSOR_I2C_WRITE(0x030b, 0x00);   
    SENSOR_I2C_WRITE(0x030d, 0x1e);   
    SENSOR_I2C_WRITE(0x030e, 0x04);   
    SENSOR_I2C_WRITE(0x030f, 0x03);
    SENSOR_I2C_WRITE(0x0312, 0x01);   
    SENSOR_I2C_WRITE(0x031e, 0x00);   
    SENSOR_I2C_WRITE(0x3000, 0x20);   
    SENSOR_I2C_WRITE(0x3002, 0x00);   
    SENSOR_I2C_WRITE(0x3018, 0x72);   
    ov4689_init_comm_first();   
    SENSOR_I2C_WRITE(0x3800, 0x01);   
    SENSOR_I2C_WRITE(0x3801, 0x88);   
    SENSOR_I2C_WRITE(0x3802, 0x00);   
    SENSOR_I2C_WRITE(0x3803, 0xe0);   
    SENSOR_I2C_WRITE(0x3804, 0x09);   
    SENSOR_I2C_WRITE(0x3805, 0x17);   
    SENSOR_I2C_WRITE(0x3806, 0x05);   
    SENSOR_I2C_WRITE(0x3807, 0x1f);   
    SENSOR_I2C_WRITE(0x3808, 0x07);   
    SENSOR_I2C_WRITE(0x3809, 0x80);   
    SENSOR_I2C_WRITE(0x380a, 0x04);   
    SENSOR_I2C_WRITE(0x380b, 0x38);   
    SENSOR_I2C_WRITE(0x380c, 0x06);   
    SENSOR_I2C_WRITE(0x380d, 0xF4);   
    SENSOR_I2C_WRITE(0x380e, 0x04);   
    SENSOR_I2C_WRITE(0x380f, 0x63);   
    //ov4689_init_comm_second();
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x08);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);
    SENSOR_I2C_WRITE(0x4020, 0x00);
    SENSOR_I2C_WRITE(0x4021, 0x10);
    SENSOR_I2C_WRITE(0x4022, 0x06);
    SENSOR_I2C_WRITE(0x4023, 0x13);
    SENSOR_I2C_WRITE(0x4024, 0x07);
    SENSOR_I2C_WRITE(0x4025, 0x40);
    SENSOR_I2C_WRITE(0x4026, 0x07);
    SENSOR_I2C_WRITE(0x4027, 0x50);
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    SENSOR_I2C_WRITE(0x4601, 0x77);
    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x56);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    SENSOR_I2C_WRITE(0x0100, 0x01);
    ov4689_delay_ms(10);
    SENSOR_I2C_WRITE(0x3105, 0x11);
    SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 1080P 30fps Linear Mode Initial OK!-------\n");

}

void ov4689_wdr_2304_1296_30_init()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01);

    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x0300, 0x00);
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0x19);
    SENSOR_I2C_WRITE(0x0303, 0x00);
    SENSOR_I2C_WRITE(0x0304, 0x03);
    SENSOR_I2C_WRITE(0x0305, 0x01);
    SENSOR_I2C_WRITE(0x0306, 0x01);
    SENSOR_I2C_WRITE(0x030A, 0x00);
    SENSOR_I2C_WRITE(0x030b, 0x00);
    SENSOR_I2C_WRITE(0x030c, 0x00);
    SENSOR_I2C_WRITE(0x030d, 0x1e);
    SENSOR_I2C_WRITE(0x030e, 0x04);
    SENSOR_I2C_WRITE(0x030f, 0x01);
    
    
    SENSOR_I2C_WRITE(0x0311, 0x00);
    SENSOR_I2C_WRITE(0x0312, 0x01);
    SENSOR_I2C_WRITE(0x031e, 0x00);
    SENSOR_I2C_WRITE(0x3000, 0x20);
    SENSOR_I2C_WRITE(0x3002, 0x00);
    SENSOR_I2C_WRITE(0x3018, 0x72);
    SENSOR_I2C_WRITE(0x3020, 0x93);
    SENSOR_I2C_WRITE(0x3021, 0x03);
    SENSOR_I2C_WRITE(0x3022, 0x01);
    SENSOR_I2C_WRITE(0x3031, 0x0a);
    SENSOR_I2C_WRITE(0x3305, 0xf1);
    SENSOR_I2C_WRITE(0x3307, 0x04);
    SENSOR_I2C_WRITE(0x3309, 0x29);
    SENSOR_I2C_WRITE(0x3500, 0x00);
    SENSOR_I2C_WRITE(0x3501, 0x45);
    SENSOR_I2C_WRITE(0x3502, 0xB0);
    SENSOR_I2C_WRITE(0x3503, 0x77);
    SENSOR_I2C_WRITE(0x3504, 0x00);
    SENSOR_I2C_WRITE(0x3505, 0x00);
    SENSOR_I2C_WRITE(0x3506, 0x00);
    SENSOR_I2C_WRITE(0x3507, 0x00);
    SENSOR_I2C_WRITE(0x3508, 0x00);
    SENSOR_I2C_WRITE(0x3509, 0x80);
    SENSOR_I2C_WRITE(0x350a, 0x00);
    SENSOR_I2C_WRITE(0x350b, 0x00);
    SENSOR_I2C_WRITE(0x350c, 0x00);
    SENSOR_I2C_WRITE(0x350d, 0x00);
    SENSOR_I2C_WRITE(0x350e, 0x00);
    SENSOR_I2C_WRITE(0x350f, 0x80);
    SENSOR_I2C_WRITE(0x3510, 0x00);
    SENSOR_I2C_WRITE(0x3511, 0x00);
    SENSOR_I2C_WRITE(0x3512, 0x00);
    SENSOR_I2C_WRITE(0x3513, 0x00);
    SENSOR_I2C_WRITE(0x3514, 0x00);
    SENSOR_I2C_WRITE(0x3515, 0x80);
    SENSOR_I2C_WRITE(0x3516, 0x00);
    SENSOR_I2C_WRITE(0x3517, 0x00);
    SENSOR_I2C_WRITE(0x3518, 0x00);
    SENSOR_I2C_WRITE(0x3519, 0x00);
    SENSOR_I2C_WRITE(0x351a, 0x00);
    SENSOR_I2C_WRITE(0x351b, 0x80);
    SENSOR_I2C_WRITE(0x351c, 0x00);
    SENSOR_I2C_WRITE(0x351d, 0x00);
    SENSOR_I2C_WRITE(0x351e, 0x00);
    SENSOR_I2C_WRITE(0x351f, 0x00);
    SENSOR_I2C_WRITE(0x3520, 0x00);
    SENSOR_I2C_WRITE(0x3521, 0x80);
    SENSOR_I2C_WRITE(0x3522, 0x08);
    SENSOR_I2C_WRITE(0x3524, 0x08);
    SENSOR_I2C_WRITE(0x3526, 0x08);
    SENSOR_I2C_WRITE(0x3528, 0x08);
    SENSOR_I2C_WRITE(0x352a, 0x08);
    SENSOR_I2C_WRITE(0x3602, 0x00);
    SENSOR_I2C_WRITE(0x3604, 0x02);
    SENSOR_I2C_WRITE(0x3605, 0x00);
    SENSOR_I2C_WRITE(0x3606, 0x00);
    SENSOR_I2C_WRITE(0x3607, 0x00);
    SENSOR_I2C_WRITE(0x3609, 0x12);
    SENSOR_I2C_WRITE(0x360a, 0x40);
    SENSOR_I2C_WRITE(0x360c, 0x08);
    SENSOR_I2C_WRITE(0x360f, 0xe5);
    SENSOR_I2C_WRITE(0x3608, 0x8f);
    SENSOR_I2C_WRITE(0x3611, 0x00);
    SENSOR_I2C_WRITE(0x3613, 0xf7);
    SENSOR_I2C_WRITE(0x3616, 0x58);
    SENSOR_I2C_WRITE(0x3619, 0x99);
    SENSOR_I2C_WRITE(0x361b, 0x60);
    SENSOR_I2C_WRITE(0x361c, 0x7a);
    SENSOR_I2C_WRITE(0x361e, 0x79);
    SENSOR_I2C_WRITE(0x361f, 0x02);
    SENSOR_I2C_WRITE(0x3632, 0x00);
    SENSOR_I2C_WRITE(0x3633, 0x10);
    SENSOR_I2C_WRITE(0x3634, 0x10);
    SENSOR_I2C_WRITE(0x3635, 0x10);
    SENSOR_I2C_WRITE(0x3636, 0x15);
    SENSOR_I2C_WRITE(0x3646, 0x86);
    SENSOR_I2C_WRITE(0x364a, 0x0b);
    SENSOR_I2C_WRITE(0x3700, 0x17);
    SENSOR_I2C_WRITE(0x3701, 0x22);
    SENSOR_I2C_WRITE(0x3703, 0x10);
    SENSOR_I2C_WRITE(0x370a, 0x37);
    SENSOR_I2C_WRITE(0x3705, 0x00);
    SENSOR_I2C_WRITE(0x3706, 0x63);
    SENSOR_I2C_WRITE(0x3709, 0x3c);
    SENSOR_I2C_WRITE(0x370b, 0x01);
    SENSOR_I2C_WRITE(0x370c, 0x30);
    SENSOR_I2C_WRITE(0x3710, 0x24);
    SENSOR_I2C_WRITE(0x3711, 0x0c);
    SENSOR_I2C_WRITE(0x3716, 0x00);
    SENSOR_I2C_WRITE(0x3720, 0x28);
    SENSOR_I2C_WRITE(0x3729, 0x7b);
    SENSOR_I2C_WRITE(0x372a, 0x84);
    SENSOR_I2C_WRITE(0x372b, 0xbd);
    SENSOR_I2C_WRITE(0x372c, 0xbc);
    SENSOR_I2C_WRITE(0x372e, 0x52);
    SENSOR_I2C_WRITE(0x373c, 0x0e);
    SENSOR_I2C_WRITE(0x373e, 0x33);
    SENSOR_I2C_WRITE(0x3743, 0x10);
    SENSOR_I2C_WRITE(0x3744, 0x88);
    SENSOR_I2C_WRITE(0x3745, 0xc0);
    SENSOR_I2C_WRITE(0x374a, 0x43);
    SENSOR_I2C_WRITE(0x374c, 0x00);
    SENSOR_I2C_WRITE(0x374e, 0x23);
    SENSOR_I2C_WRITE(0x3751, 0x7b);
    SENSOR_I2C_WRITE(0x3752, 0x84);
    SENSOR_I2C_WRITE(0x3753, 0xbd);
    SENSOR_I2C_WRITE(0x3754, 0xbc);
    SENSOR_I2C_WRITE(0x3756, 0x52);
    SENSOR_I2C_WRITE(0x375c, 0x00);
    SENSOR_I2C_WRITE(0x3760, 0x00);
    SENSOR_I2C_WRITE(0x3761, 0x00);
    SENSOR_I2C_WRITE(0x3762, 0x00);
    SENSOR_I2C_WRITE(0x3763, 0x00);
    SENSOR_I2C_WRITE(0x3764, 0x00);
    SENSOR_I2C_WRITE(0x3767, 0x04);
    SENSOR_I2C_WRITE(0x3768, 0x04);
    SENSOR_I2C_WRITE(0x3769, 0x08);
    SENSOR_I2C_WRITE(0x376a, 0x08);
    SENSOR_I2C_WRITE(0x376b, 0x20);
    SENSOR_I2C_WRITE(0x376c, 0x00);
    SENSOR_I2C_WRITE(0x376d, 0x00);
    SENSOR_I2C_WRITE(0x376e, 0x00);
    SENSOR_I2C_WRITE(0x3773, 0x00);
    SENSOR_I2C_WRITE(0x3774, 0x51);
    SENSOR_I2C_WRITE(0x3776, 0xbd);
    SENSOR_I2C_WRITE(0x3777, 0xbd);
    SENSOR_I2C_WRITE(0x3781, 0x18);
    SENSOR_I2C_WRITE(0x3783, 0x25);
    
    SENSOR_I2C_WRITE(0x3800,0x00);
    SENSOR_I2C_WRITE(0x3801,0xc8);
    SENSOR_I2C_WRITE(0x3802,0x00);
    SENSOR_I2C_WRITE(0x3803,0x74);
    SENSOR_I2C_WRITE(0x3804,0x09);
    SENSOR_I2C_WRITE(0x3805,0xd7);
    SENSOR_I2C_WRITE(0x3806,0x05);
    SENSOR_I2C_WRITE(0x3807,0x8b);
    SENSOR_I2C_WRITE(0x3808,0x09);
    SENSOR_I2C_WRITE(0x3809,0x00);
    SENSOR_I2C_WRITE(0x380a,0x05);
    SENSOR_I2C_WRITE(0x380b,0x10);
    SENSOR_I2C_WRITE(0x380c,0x05);
    SENSOR_I2C_WRITE(0x380d,0xee);
    SENSOR_I2C_WRITE(0x380e,0x05);
    SENSOR_I2C_WRITE(0x380f,0x2f);
    SENSOR_I2C_WRITE(0x3810,0x00);
    SENSOR_I2C_WRITE(0x3811,0x08);
    SENSOR_I2C_WRITE(0x3812,0x00);
    SENSOR_I2C_WRITE(0x3813,0x04);

    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);

    SENSOR_I2C_WRITE(0x4020,0x00);
    SENSOR_I2C_WRITE(0x4021,0x10);
    SENSOR_I2C_WRITE(0x4022,0x07);
    SENSOR_I2C_WRITE(0x4023,0x93);
    SENSOR_I2C_WRITE(0x4024,0x08);
    SENSOR_I2C_WRITE(0x4025,0xc0);
    SENSOR_I2C_WRITE(0x4026,0x08);
    SENSOR_I2C_WRITE(0x4027,0xd0);
    
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    
    SENSOR_I2C_WRITE(0x4600,0x00);
    SENSOR_I2C_WRITE(0x4601,0x41);

    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x1a);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    //SENSOR_I2C_WRITE(0x0100, 0x01);

    SENSOR_I2C_WRITE(0x3105, 0x11);
    //SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    //SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);

    ov4689_enable_wdr_mode();
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 2304_1296 30fps WDR Mode Initial OK!-------\n");
}


void ov4689_wdr_2048_1520_30_init()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01);

    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x0300, 0x00);
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0x19);
    SENSOR_I2C_WRITE(0x0303, 0x00);
    SENSOR_I2C_WRITE(0x0304, 0x03);
    SENSOR_I2C_WRITE(0x0305, 0x01);
    SENSOR_I2C_WRITE(0x0306, 0x01);
    SENSOR_I2C_WRITE(0x030A, 0x00);
    SENSOR_I2C_WRITE(0x030b, 0x00);
    SENSOR_I2C_WRITE(0x030c, 0x00);
    SENSOR_I2C_WRITE(0x030d, 0x1e);
    SENSOR_I2C_WRITE(0x030e, 0x04);
    SENSOR_I2C_WRITE(0x030f, 0x01);
    
    
    SENSOR_I2C_WRITE(0x0311, 0x00);
    SENSOR_I2C_WRITE(0x0312, 0x01);
    SENSOR_I2C_WRITE(0x031e, 0x00);
    SENSOR_I2C_WRITE(0x3000, 0x20);
    SENSOR_I2C_WRITE(0x3002, 0x00);
    SENSOR_I2C_WRITE(0x3018, 0x72);
    SENSOR_I2C_WRITE(0x3020, 0x93);
    SENSOR_I2C_WRITE(0x3021, 0x03);
    SENSOR_I2C_WRITE(0x3022, 0x01);
    SENSOR_I2C_WRITE(0x3031, 0x0a);
    SENSOR_I2C_WRITE(0x3305, 0xf1);
    SENSOR_I2C_WRITE(0x3307, 0x04);
    SENSOR_I2C_WRITE(0x3309, 0x29);
    SENSOR_I2C_WRITE(0x3500, 0x00);
    SENSOR_I2C_WRITE(0x3501, 0x45);
    SENSOR_I2C_WRITE(0x3502, 0xB0);
    SENSOR_I2C_WRITE(0x3503, 0x77);
    SENSOR_I2C_WRITE(0x3504, 0x00);
    SENSOR_I2C_WRITE(0x3505, 0x00);
    SENSOR_I2C_WRITE(0x3506, 0x00);
    SENSOR_I2C_WRITE(0x3507, 0x00);
    SENSOR_I2C_WRITE(0x3508, 0x00);
    SENSOR_I2C_WRITE(0x3509, 0x80);
    SENSOR_I2C_WRITE(0x350a, 0x00);
    SENSOR_I2C_WRITE(0x350b, 0x00);
    SENSOR_I2C_WRITE(0x350c, 0x00);
    SENSOR_I2C_WRITE(0x350d, 0x00);
    SENSOR_I2C_WRITE(0x350e, 0x00);
    SENSOR_I2C_WRITE(0x350f, 0x80);
    SENSOR_I2C_WRITE(0x3510, 0x00);
    SENSOR_I2C_WRITE(0x3511, 0x00);
    SENSOR_I2C_WRITE(0x3512, 0x00);
    SENSOR_I2C_WRITE(0x3513, 0x00);
    SENSOR_I2C_WRITE(0x3514, 0x00);
    SENSOR_I2C_WRITE(0x3515, 0x80);
    SENSOR_I2C_WRITE(0x3516, 0x00);
    SENSOR_I2C_WRITE(0x3517, 0x00);
    SENSOR_I2C_WRITE(0x3518, 0x00);
    SENSOR_I2C_WRITE(0x3519, 0x00);
    SENSOR_I2C_WRITE(0x351a, 0x00);
    SENSOR_I2C_WRITE(0x351b, 0x80);
    SENSOR_I2C_WRITE(0x351c, 0x00);
    SENSOR_I2C_WRITE(0x351d, 0x00);
    SENSOR_I2C_WRITE(0x351e, 0x00);
    SENSOR_I2C_WRITE(0x351f, 0x00);
    SENSOR_I2C_WRITE(0x3520, 0x00);
    SENSOR_I2C_WRITE(0x3521, 0x80);
    SENSOR_I2C_WRITE(0x3522, 0x08);
    SENSOR_I2C_WRITE(0x3524, 0x08);
    SENSOR_I2C_WRITE(0x3526, 0x08);
    SENSOR_I2C_WRITE(0x3528, 0x08);
    SENSOR_I2C_WRITE(0x352a, 0x08);
    SENSOR_I2C_WRITE(0x3602, 0x00);
    SENSOR_I2C_WRITE(0x3604, 0x02);
    SENSOR_I2C_WRITE(0x3605, 0x00);
    SENSOR_I2C_WRITE(0x3606, 0x00);
    SENSOR_I2C_WRITE(0x3607, 0x00);
    SENSOR_I2C_WRITE(0x3609, 0x12);
    SENSOR_I2C_WRITE(0x360a, 0x40);
    SENSOR_I2C_WRITE(0x360c, 0x08);
    SENSOR_I2C_WRITE(0x360f, 0xe5);
    SENSOR_I2C_WRITE(0x3608, 0x8f);
    SENSOR_I2C_WRITE(0x3611, 0x00);
    SENSOR_I2C_WRITE(0x3613, 0xf7);
    SENSOR_I2C_WRITE(0x3616, 0x58);
    SENSOR_I2C_WRITE(0x3619, 0x99);
    SENSOR_I2C_WRITE(0x361b, 0x60);
    SENSOR_I2C_WRITE(0x361c, 0x7a);
    SENSOR_I2C_WRITE(0x361e, 0x79);
    SENSOR_I2C_WRITE(0x361f, 0x02);
    SENSOR_I2C_WRITE(0x3632, 0x00);
    SENSOR_I2C_WRITE(0x3633, 0x10);
    SENSOR_I2C_WRITE(0x3634, 0x10);
    SENSOR_I2C_WRITE(0x3635, 0x10);
    SENSOR_I2C_WRITE(0x3636, 0x15);
    SENSOR_I2C_WRITE(0x3646, 0x86);
    SENSOR_I2C_WRITE(0x364a, 0x0b);
    SENSOR_I2C_WRITE(0x3700, 0x17);
    SENSOR_I2C_WRITE(0x3701, 0x22);
    SENSOR_I2C_WRITE(0x3703, 0x10);
    SENSOR_I2C_WRITE(0x370a, 0x37);
    SENSOR_I2C_WRITE(0x3705, 0x00);
    SENSOR_I2C_WRITE(0x3706, 0x63);
    SENSOR_I2C_WRITE(0x3709, 0x3c);
    SENSOR_I2C_WRITE(0x370b, 0x01);
    SENSOR_I2C_WRITE(0x370c, 0x30);
    SENSOR_I2C_WRITE(0x3710, 0x24);
    SENSOR_I2C_WRITE(0x3711, 0x0c);
    SENSOR_I2C_WRITE(0x3716, 0x00);
    SENSOR_I2C_WRITE(0x3720, 0x28);
    SENSOR_I2C_WRITE(0x3729, 0x7b);
    SENSOR_I2C_WRITE(0x372a, 0x84);
    SENSOR_I2C_WRITE(0x372b, 0xbd);
    SENSOR_I2C_WRITE(0x372c, 0xbc);
    SENSOR_I2C_WRITE(0x372e, 0x52);
    SENSOR_I2C_WRITE(0x373c, 0x0e);
    SENSOR_I2C_WRITE(0x373e, 0x33);
    SENSOR_I2C_WRITE(0x3743, 0x10);
    SENSOR_I2C_WRITE(0x3744, 0x88);
    SENSOR_I2C_WRITE(0x3745, 0xc0);
    SENSOR_I2C_WRITE(0x374a, 0x43);
    SENSOR_I2C_WRITE(0x374c, 0x00);
    SENSOR_I2C_WRITE(0x374e, 0x23);
    SENSOR_I2C_WRITE(0x3751, 0x7b);
    SENSOR_I2C_WRITE(0x3752, 0x84);
    SENSOR_I2C_WRITE(0x3753, 0xbd);
    SENSOR_I2C_WRITE(0x3754, 0xbc);
    SENSOR_I2C_WRITE(0x3756, 0x52);
    SENSOR_I2C_WRITE(0x375c, 0x00);
    SENSOR_I2C_WRITE(0x3760, 0x00);
    SENSOR_I2C_WRITE(0x3761, 0x00);
    SENSOR_I2C_WRITE(0x3762, 0x00);
    SENSOR_I2C_WRITE(0x3763, 0x00);
    SENSOR_I2C_WRITE(0x3764, 0x00);
    SENSOR_I2C_WRITE(0x3767, 0x04);
    SENSOR_I2C_WRITE(0x3768, 0x04);
    SENSOR_I2C_WRITE(0x3769, 0x08);
    SENSOR_I2C_WRITE(0x376a, 0x08);
    SENSOR_I2C_WRITE(0x376b, 0x20);
    SENSOR_I2C_WRITE(0x376c, 0x00);
    SENSOR_I2C_WRITE(0x376d, 0x00);
    SENSOR_I2C_WRITE(0x376e, 0x00);
    SENSOR_I2C_WRITE(0x3773, 0x00);
    SENSOR_I2C_WRITE(0x3774, 0x51);
    SENSOR_I2C_WRITE(0x3776, 0xbd);
    SENSOR_I2C_WRITE(0x3777, 0xbd);
    SENSOR_I2C_WRITE(0x3781, 0x18);
    SENSOR_I2C_WRITE(0x3783, 0x25);
    
    SENSOR_I2C_WRITE(0x3800,0x01);
    SENSOR_I2C_WRITE(0x3801,0x48);
    SENSOR_I2C_WRITE(0x3802,0x00);
    SENSOR_I2C_WRITE(0x3803,0x04);
    SENSOR_I2C_WRITE(0x3804,0x09);
    SENSOR_I2C_WRITE(0x3805,0x57);
    SENSOR_I2C_WRITE(0x3806,0x05);
    SENSOR_I2C_WRITE(0x3807,0xfb);
    SENSOR_I2C_WRITE(0x3808,0x08);
    SENSOR_I2C_WRITE(0x3809,0x00);
    SENSOR_I2C_WRITE(0x380a,0x05);
    SENSOR_I2C_WRITE(0x380b,0xf0);
    
    SENSOR_I2C_WRITE(0x380c,0x05);
    SENSOR_I2C_WRITE(0x380d,0x28);
    SENSOR_I2C_WRITE(0x380e,0x05);
    SENSOR_I2C_WRITE(0x380f,0x80);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    SENSOR_I2C_WRITE(0x3810,0x00);
    SENSOR_I2C_WRITE(0x3811,0x08);
    SENSOR_I2C_WRITE(0x3812,0x00);
    SENSOR_I2C_WRITE(0x3813,0x04);

    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);

    SENSOR_I2C_WRITE(0x4020,0x00);
    SENSOR_I2C_WRITE(0x4021,0x10);
    SENSOR_I2C_WRITE(0x4022,0x06);
    SENSOR_I2C_WRITE(0x4023,0x93);
    SENSOR_I2C_WRITE(0x4024,0x07);
    SENSOR_I2C_WRITE(0x4025,0xc0);
    SENSOR_I2C_WRITE(0x4026,0x07);
    SENSOR_I2C_WRITE(0x4027,0xd0);
    
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    
    SENSOR_I2C_WRITE(0x4600,0x00);
    SENSOR_I2C_WRITE(0x4601,0x41);

    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x1a);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    //SENSOR_I2C_WRITE(0x0100, 0x01);

    SENSOR_I2C_WRITE(0x3105, 0x11);
    //SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    //SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);

    ov4689_enable_wdr_mode();
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 2304_1296 30fps WDR Mode Initial OK!-------\n");
}

void ov4689_linear_2304_1296_30_init()
{   
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01); 

    SENSOR_I2C_WRITE(0x3638, 0x00);    
    SENSOR_I2C_WRITE(0x0300, 0x02);    
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0xf4);    
    SENSOR_I2C_WRITE(0x0303, 0x03);
    SENSOR_I2C_WRITE(0x0304, 0x03);    
    SENSOR_I2C_WRITE(0x030A, 0x01);    
    SENSOR_I2C_WRITE(0x030b, 0x00);    
    SENSOR_I2C_WRITE(0x030d, 0x1e);    
    SENSOR_I2C_WRITE(0x030e, 0x04);    
    SENSOR_I2C_WRITE(0x030f, 0x01);
    SENSOR_I2C_WRITE(0x0312, 0x01);    
    SENSOR_I2C_WRITE(0x031e, 0x00);    
    SENSOR_I2C_WRITE(0x3000, 0x20);    
    SENSOR_I2C_WRITE(0x3002, 0x00);    
    SENSOR_I2C_WRITE(0x3018, 0x72);    
    ov4689_init_comm_first();   
    SENSOR_I2C_WRITE(0x3800, 0x00);    
    SENSOR_I2C_WRITE(0x3801, 0xc8);    
    SENSOR_I2C_WRITE(0x3802, 0x00);    
    SENSOR_I2C_WRITE(0x3803, 0x74);    
    SENSOR_I2C_WRITE(0x3804, 0x09);    
    SENSOR_I2C_WRITE(0x3805, 0xd7);    
    SENSOR_I2C_WRITE(0x3806, 0x05);    
    SENSOR_I2C_WRITE(0x3807, 0x8b);    
    SENSOR_I2C_WRITE(0x3808, 0x09);    
    SENSOR_I2C_WRITE(0x3809, 0x00);    
    SENSOR_I2C_WRITE(0x380a, 0x05);    
    SENSOR_I2C_WRITE(0x380b, 0x10);    
    SENSOR_I2C_WRITE(0x380c, 0x08);    
    SENSOR_I2C_WRITE(0x380d, 0x74);    
    SENSOR_I2C_WRITE(0x380e, 0x05);    
    SENSOR_I2C_WRITE(0x380f, 0x77);    
    //ov4689_init_comm_second();
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x08);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);
    SENSOR_I2C_WRITE(0x4020, 0x00);
    SENSOR_I2C_WRITE(0x4021, 0x10);
    SENSOR_I2C_WRITE(0x4022, 0x06);
    SENSOR_I2C_WRITE(0x4023, 0x13);
    SENSOR_I2C_WRITE(0x4024, 0x07);
    SENSOR_I2C_WRITE(0x4025, 0x40);
    SENSOR_I2C_WRITE(0x4026, 0x07);
    SENSOR_I2C_WRITE(0x4027, 0x50);
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    SENSOR_I2C_WRITE(0x4601, 0x77);
    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x56);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    SENSOR_I2C_WRITE(0x0100, 0x01);
    ov4689_delay_ms(10);
    SENSOR_I2C_WRITE(0x3105, 0x11);
    SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 3M 2304*1296 30fps Linear Mode Initial OK!-------\n");
}


void ov4689_linear_2048_1520_30_init()
{   
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103, 0x01);

    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x0300, 0x02);
    SENSOR_I2C_WRITE(0x0301, 0x00);
    SENSOR_I2C_WRITE(0x0302, 0xf4);
    SENSOR_I2C_WRITE(0x0303, 0x03);
    SENSOR_I2C_WRITE(0x0304, 0x03);
    SENSOR_I2C_WRITE(0x030A, 0x01);
    SENSOR_I2C_WRITE(0x030b, 0x00);
    SENSOR_I2C_WRITE(0x030d, 0x1e);
    SENSOR_I2C_WRITE(0x030e, 0x04);
    SENSOR_I2C_WRITE(0x030f, 0x01);
    SENSOR_I2C_WRITE(0x0312, 0x01);
    SENSOR_I2C_WRITE(0x031e, 0x00);
    SENSOR_I2C_WRITE(0x3000, 0x20);
    SENSOR_I2C_WRITE(0x3002, 0x00);
    SENSOR_I2C_WRITE(0x3018, 0x72);
    ov4689_init_comm_first();
    SENSOR_I2C_WRITE(0x3800, 0x01);
    SENSOR_I2C_WRITE(0x3801, 0x48);
    SENSOR_I2C_WRITE(0x3802, 0x00);
    SENSOR_I2C_WRITE(0x3803, 0x04);
    SENSOR_I2C_WRITE(0x3804, 0x09);
    SENSOR_I2C_WRITE(0x3805, 0x57);
    SENSOR_I2C_WRITE(0x3806, 0x05);
    SENSOR_I2C_WRITE(0x3807, 0xfb);
    SENSOR_I2C_WRITE(0x3808, 0x08);
    SENSOR_I2C_WRITE(0x3809, 0x00);
    SENSOR_I2C_WRITE(0x380a, 0x05);
    SENSOR_I2C_WRITE(0x380b, 0xf0);
    SENSOR_I2C_WRITE(0x380c, 0x07);
    SENSOR_I2C_WRITE(0x380d, 0x74);
    SENSOR_I2C_WRITE(0x380e, 0x08);
    SENSOR_I2C_WRITE(0x380f, 0x50);
    //ov4689_init_comm_second();
    SENSOR_I2C_WRITE(0x3810, 0x00);
    SENSOR_I2C_WRITE(0x3811, 0x08);
    SENSOR_I2C_WRITE(0x3812, 0x00);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3819, 0x01);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x06);
    SENSOR_I2C_WRITE(0x3829, 0x00);
    SENSOR_I2C_WRITE(0x382a, 0x01);
    SENSOR_I2C_WRITE(0x382b, 0x01);
    SENSOR_I2C_WRITE(0x382d, 0x7f);
    SENSOR_I2C_WRITE(0x3830, 0x04);
    SENSOR_I2C_WRITE(0x3836, 0x01);
    SENSOR_I2C_WRITE(0x3841, 0x02);
    SENSOR_I2C_WRITE(0x3846, 0x08);
    SENSOR_I2C_WRITE(0x3847, 0x07);
    SENSOR_I2C_WRITE(0x3d85, 0x36);
    SENSOR_I2C_WRITE(0x3d8c, 0x71);
    SENSOR_I2C_WRITE(0x3d8d, 0xcb);
    SENSOR_I2C_WRITE(0x3f0a, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x71);
    SENSOR_I2C_WRITE(0x4001, 0x40);
    SENSOR_I2C_WRITE(0x4002, 0x04);
    SENSOR_I2C_WRITE(0x4003, 0x14);
    SENSOR_I2C_WRITE(0x400e, 0x00);
    SENSOR_I2C_WRITE(0x4011, 0x00);
    SENSOR_I2C_WRITE(0x401a, 0x00);
    SENSOR_I2C_WRITE(0x401b, 0x00);
    SENSOR_I2C_WRITE(0x401c, 0x00);
    SENSOR_I2C_WRITE(0x401d, 0x00);
    SENSOR_I2C_WRITE(0x401f, 0x00);
    SENSOR_I2C_WRITE(0x4020, 0x00);
    SENSOR_I2C_WRITE(0x4021, 0x10);
    SENSOR_I2C_WRITE(0x4022, 0x06);
    SENSOR_I2C_WRITE(0x4023, 0x13);
    SENSOR_I2C_WRITE(0x4024, 0x07);
    SENSOR_I2C_WRITE(0x4025, 0x40);
    SENSOR_I2C_WRITE(0x4026, 0x07);
    SENSOR_I2C_WRITE(0x4027, 0x50);
    SENSOR_I2C_WRITE(0x4028, 0x00);
    SENSOR_I2C_WRITE(0x4029, 0x02);
    SENSOR_I2C_WRITE(0x402a, 0x06);
    SENSOR_I2C_WRITE(0x402b, 0x04);
    SENSOR_I2C_WRITE(0x402c, 0x02);
    SENSOR_I2C_WRITE(0x402d, 0x02);
    SENSOR_I2C_WRITE(0x402e, 0x0e);
    SENSOR_I2C_WRITE(0x402f, 0x04);
    SENSOR_I2C_WRITE(0x4302, 0xff);
    SENSOR_I2C_WRITE(0x4303, 0xff);
    SENSOR_I2C_WRITE(0x4304, 0x00);
    SENSOR_I2C_WRITE(0x4305, 0x00);
    SENSOR_I2C_WRITE(0x4306, 0x00);
    SENSOR_I2C_WRITE(0x4308, 0x02);
    SENSOR_I2C_WRITE(0x4500, 0x6c);
    SENSOR_I2C_WRITE(0x4501, 0xc4);
    SENSOR_I2C_WRITE(0x4502, 0x40);
    SENSOR_I2C_WRITE(0x4503, 0x01);
    SENSOR_I2C_WRITE(0x4601, 0x77);
    SENSOR_I2C_WRITE(0x4800, 0x04);
    SENSOR_I2C_WRITE(0x4813, 0x08);
    SENSOR_I2C_WRITE(0x481f, 0x40);
    SENSOR_I2C_WRITE(0x4829, 0x78);
    SENSOR_I2C_WRITE(0x4837, 0x56);
    SENSOR_I2C_WRITE(0x4b00, 0x2a);
    SENSOR_I2C_WRITE(0x4b0d, 0x00);
    SENSOR_I2C_WRITE(0x4d00, 0x04);
    SENSOR_I2C_WRITE(0x4d01, 0x42);
    SENSOR_I2C_WRITE(0x4d02, 0xd1);
    SENSOR_I2C_WRITE(0x4d03, 0x93);
    SENSOR_I2C_WRITE(0x4d04, 0xf5);
    SENSOR_I2C_WRITE(0x4d05, 0xc1);
    SENSOR_I2C_WRITE(0x5000, 0xf3);
    SENSOR_I2C_WRITE(0x5001, 0x11);
    SENSOR_I2C_WRITE(0x5004, 0x00);
    SENSOR_I2C_WRITE(0x500a, 0x00);
    SENSOR_I2C_WRITE(0x500b, 0x00);
    SENSOR_I2C_WRITE(0x5032, 0x00);
    SENSOR_I2C_WRITE(0x5040, 0x00);
    SENSOR_I2C_WRITE(0x5050, 0x0c);
    SENSOR_I2C_WRITE(0x8000, 0x00);
    SENSOR_I2C_WRITE(0x8001, 0x00);
    SENSOR_I2C_WRITE(0x8002, 0x00);
    SENSOR_I2C_WRITE(0x8003, 0x00);
    SENSOR_I2C_WRITE(0x8004, 0x00);
    SENSOR_I2C_WRITE(0x8005, 0x00);
    SENSOR_I2C_WRITE(0x8006, 0x00);
    SENSOR_I2C_WRITE(0x8007, 0x00);
    SENSOR_I2C_WRITE(0x8008, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3105, 0x31);
    SENSOR_I2C_WRITE(0x301a, 0xf9);
    SENSOR_I2C_WRITE(0x3508, 0x07);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4805, 0x03);
    SENSOR_I2C_WRITE(0x3601, 0x01);
    SENSOR_I2C_WRITE(0x0100, 0x01);
    ov4689_delay_ms(10);
    SENSOR_I2C_WRITE(0x3105, 0x11);
    SENSOR_I2C_WRITE(0x301a, 0xf1);
    SENSOR_I2C_WRITE(0x4805, 0x00);
    SENSOR_I2C_WRITE(0x301a, 0xf0);
    SENSOR_I2C_WRITE(0x3208, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x302a, 0x00);
    SENSOR_I2C_WRITE(0x3601, 0x00);
    SENSOR_I2C_WRITE(0x3638, 0x00);
    SENSOR_I2C_WRITE(0x3208, 0x10);
    SENSOR_I2C_WRITE(0x3208, 0xa0);
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 3M 2048*1520 30fps Linear Mode Initial OK!-------\n");
}



void ov4689_linear_720p_120fps_init()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103,0x01);

    SENSOR_I2C_WRITE(0x3638,0x00); 
    SENSOR_I2C_WRITE(0x0300,0x02); 
    SENSOR_I2C_WRITE(0x0302,0x32); 
    SENSOR_I2C_WRITE(0x0303,0x00);
    SENSOR_I2C_WRITE(0x0304,0x03); 
    SENSOR_I2C_WRITE(0x030b,0x00); 
    SENSOR_I2C_WRITE(0x030d,0x1e); 
    SENSOR_I2C_WRITE(0x030e,0x04); 
    SENSOR_I2C_WRITE(0x030f,0x02); 
    SENSOR_I2C_WRITE(0x0312,0x01); 
    SENSOR_I2C_WRITE(0x031e,0x00); 
    SENSOR_I2C_WRITE(0x3000,0x20); 
    SENSOR_I2C_WRITE(0x3002,0x00); 
    SENSOR_I2C_WRITE(0x3018,0x72); 
    SENSOR_I2C_WRITE(0x3020,0x93); 
    SENSOR_I2C_WRITE(0x3021,0x03); 
    SENSOR_I2C_WRITE(0x3022,0x01); 
    SENSOR_I2C_WRITE(0x3031,0x0a); 
    SENSOR_I2C_WRITE(0x303f,0x0c);
    SENSOR_I2C_WRITE(0x3305,0xf1); 
    SENSOR_I2C_WRITE(0x3307,0x04); 
    SENSOR_I2C_WRITE(0x3309,0x29); 
    SENSOR_I2C_WRITE(0x3500,0x00); 
    SENSOR_I2C_WRITE(0x3501,0x30); 
    SENSOR_I2C_WRITE(0x3502,0x00); 
    SENSOR_I2C_WRITE(0x3503,0x77); 
    SENSOR_I2C_WRITE(0x3504,0x00); 
    SENSOR_I2C_WRITE(0x3505,0x00); 
    SENSOR_I2C_WRITE(0x3506,0x00); 
    SENSOR_I2C_WRITE(0x3507,0x00); 
    SENSOR_I2C_WRITE(0x3508,0x00); 
    SENSOR_I2C_WRITE(0x3509,0x80); 
    SENSOR_I2C_WRITE(0x350a,0x00); 
    SENSOR_I2C_WRITE(0x350b,0x00); 
    SENSOR_I2C_WRITE(0x350c,0x00); 
    SENSOR_I2C_WRITE(0x350d,0x00); 
    SENSOR_I2C_WRITE(0x350e,0x00); 
    SENSOR_I2C_WRITE(0x350f,0x80); 
    SENSOR_I2C_WRITE(0x3510,0x00); 
    SENSOR_I2C_WRITE(0x3511,0x00); 
    SENSOR_I2C_WRITE(0x3512,0x00); 
    SENSOR_I2C_WRITE(0x3513,0x00); 
    SENSOR_I2C_WRITE(0x3514,0x00); 
    SENSOR_I2C_WRITE(0x3515,0x80); 
    SENSOR_I2C_WRITE(0x3516,0x00); 
    SENSOR_I2C_WRITE(0x3517,0x00); 
    SENSOR_I2C_WRITE(0x3518,0x00); 
    SENSOR_I2C_WRITE(0x3519,0x00); 
    SENSOR_I2C_WRITE(0x351a,0x00); 
    SENSOR_I2C_WRITE(0x351b,0x80); 
    SENSOR_I2C_WRITE(0x351c,0x00); 
    SENSOR_I2C_WRITE(0x351d,0x00); 
    SENSOR_I2C_WRITE(0x351e,0x00); 
    SENSOR_I2C_WRITE(0x351f,0x00); 
    SENSOR_I2C_WRITE(0x3520,0x00); 
    SENSOR_I2C_WRITE(0x3521,0x80); 
    SENSOR_I2C_WRITE(0x3522,0x08); 
    SENSOR_I2C_WRITE(0x3524,0x08); 
    SENSOR_I2C_WRITE(0x3526,0x08); 
    SENSOR_I2C_WRITE(0x3528,0x08); 
    SENSOR_I2C_WRITE(0x352a,0x08); 
    SENSOR_I2C_WRITE(0x3602,0x00); 
    SENSOR_I2C_WRITE(0x3603,0x40); 
    SENSOR_I2C_WRITE(0x3604,0x02); 
    SENSOR_I2C_WRITE(0x3605,0x00); 
    SENSOR_I2C_WRITE(0x3606,0x00); 
    SENSOR_I2C_WRITE(0x3607,0x00); 
    SENSOR_I2C_WRITE(0x3609,0x12); 
    SENSOR_I2C_WRITE(0x360a,0x40); 
    SENSOR_I2C_WRITE(0x360c,0x08); 
    SENSOR_I2C_WRITE(0x360f,0xe5); 
    SENSOR_I2C_WRITE(0x3608,0x8f); 
    SENSOR_I2C_WRITE(0x3611,0x00); 
    SENSOR_I2C_WRITE(0x3613,0xf7); 
    SENSOR_I2C_WRITE(0x3616,0x58); 
    SENSOR_I2C_WRITE(0x3619,0x99); 
    SENSOR_I2C_WRITE(0x361b,0x60); 
    SENSOR_I2C_WRITE(0x361c,0x7a); 
    SENSOR_I2C_WRITE(0x361e,0x79); 
    SENSOR_I2C_WRITE(0x361f,0x02); 
    SENSOR_I2C_WRITE(0x3632,0x05); 
    SENSOR_I2C_WRITE(0x3633,0x10); 
    SENSOR_I2C_WRITE(0x3634,0x10); 
    SENSOR_I2C_WRITE(0x3635,0x10); 
    SENSOR_I2C_WRITE(0x3636,0x15); 
    SENSOR_I2C_WRITE(0x3646,0x86); 
    SENSOR_I2C_WRITE(0x364a,0x0b); 
    SENSOR_I2C_WRITE(0x3700,0x17); 
    SENSOR_I2C_WRITE(0x3701,0x22); 
    SENSOR_I2C_WRITE(0x3703,0x10); 
    SENSOR_I2C_WRITE(0x370a,0x37); 
    SENSOR_I2C_WRITE(0x3705,0x00); 
    SENSOR_I2C_WRITE(0x3706,0x63); 
    SENSOR_I2C_WRITE(0x3709,0x3c); 
    SENSOR_I2C_WRITE(0x370b,0x01); 
    SENSOR_I2C_WRITE(0x370c,0x30); 
    SENSOR_I2C_WRITE(0x3710,0x24); 
    SENSOR_I2C_WRITE(0x3711,0x0c); 
    SENSOR_I2C_WRITE(0x3716,0x00); 
    SENSOR_I2C_WRITE(0x3720,0x28); 
    SENSOR_I2C_WRITE(0x3729,0x7b); 
    SENSOR_I2C_WRITE(0x372a,0x84); 
    SENSOR_I2C_WRITE(0x372b,0xbd); 
    SENSOR_I2C_WRITE(0x372c,0xbc); 
    SENSOR_I2C_WRITE(0x372e,0x52); 
    SENSOR_I2C_WRITE(0x373c,0x0e); 
    SENSOR_I2C_WRITE(0x373e,0x33); 
    SENSOR_I2C_WRITE(0x3743,0x10); 
    SENSOR_I2C_WRITE(0x3744,0x88); 
    SENSOR_I2C_WRITE(0x3745,0xc0); 
    SENSOR_I2C_WRITE(0x374a,0x43); 
    SENSOR_I2C_WRITE(0x374c,0x00); 
    SENSOR_I2C_WRITE(0x374e,0x23); 
    SENSOR_I2C_WRITE(0x3751,0x7b); 
    SENSOR_I2C_WRITE(0x3752,0x84); 
    SENSOR_I2C_WRITE(0x3753,0xbd); 
    SENSOR_I2C_WRITE(0x3754,0xbc); 
    SENSOR_I2C_WRITE(0x3756,0x52); 
    SENSOR_I2C_WRITE(0x375c,0x00); 
    SENSOR_I2C_WRITE(0x3760,0x00); 
    SENSOR_I2C_WRITE(0x3761,0x00); 
    SENSOR_I2C_WRITE(0x3762,0x00); 
    SENSOR_I2C_WRITE(0x3763,0x00); 
    SENSOR_I2C_WRITE(0x3764,0x00); 
    SENSOR_I2C_WRITE(0x3767,0x04); 
    SENSOR_I2C_WRITE(0x3768,0x04); 
    SENSOR_I2C_WRITE(0x3769,0x08); 
    SENSOR_I2C_WRITE(0x376a,0x08); 
    SENSOR_I2C_WRITE(0x376b,0x40); 
    SENSOR_I2C_WRITE(0x376c,0x00); 
    SENSOR_I2C_WRITE(0x376d,0x00); 
    SENSOR_I2C_WRITE(0x376e,0x00); 
    SENSOR_I2C_WRITE(0x3773,0x00); 
    SENSOR_I2C_WRITE(0x3774,0x51); 
    SENSOR_I2C_WRITE(0x3776,0xbd); 
    SENSOR_I2C_WRITE(0x3777,0xbd); 
    SENSOR_I2C_WRITE(0x3781,0x18); 
    SENSOR_I2C_WRITE(0x3783,0x25); 
    SENSOR_I2C_WRITE(0x3798,0x1b); 
    SENSOR_I2C_WRITE(0x3800,0x00);
    SENSOR_I2C_WRITE(0x3801,0x48); 
    SENSOR_I2C_WRITE(0x3802,0x00); 
    SENSOR_I2C_WRITE(0x3803,0x2C); 
    SENSOR_I2C_WRITE(0x3804,0x0a); 
    SENSOR_I2C_WRITE(0x3805,0x57); 
    SENSOR_I2C_WRITE(0x3806,0x05); 
    SENSOR_I2C_WRITE(0x3807,0xD3); 
    SENSOR_I2C_WRITE(0x3808,0x05); 
    SENSOR_I2C_WRITE(0x3809,0x00); 
    SENSOR_I2C_WRITE(0x380a,0x02); 
    SENSOR_I2C_WRITE(0x380b,0xD0); 
    SENSOR_I2C_WRITE(0x380c,0x03); 
    SENSOR_I2C_WRITE(0x380d,0x5C); 
    SENSOR_I2C_WRITE(0x380e,0x03); 
    SENSOR_I2C_WRITE(0x380f,0x05);
    SENSOR_I2C_WRITE(0x3810,0x00); 
    SENSOR_I2C_WRITE(0x3811,0x04); 
    SENSOR_I2C_WRITE(0x3812,0x00); 
    SENSOR_I2C_WRITE(0x3813,0x02); 
    SENSOR_I2C_WRITE(0x3814,0x03); 
    SENSOR_I2C_WRITE(0x3815,0x01); 
    SENSOR_I2C_WRITE(0x3819,0x01); 
    SENSOR_I2C_WRITE(0x3820,0x10); 
    SENSOR_I2C_WRITE(0x3821,0x07); 
    SENSOR_I2C_WRITE(0x3829,0x00); 
    SENSOR_I2C_WRITE(0x382a,0x03); 
    SENSOR_I2C_WRITE(0x382b,0x01); 
    SENSOR_I2C_WRITE(0x382d,0x7f); 
    SENSOR_I2C_WRITE(0x3830,0x08); 
    SENSOR_I2C_WRITE(0x3836,0x02); 
    SENSOR_I2C_WRITE(0x3837,0x00); 
    SENSOR_I2C_WRITE(0x3841,0x02); 
    SENSOR_I2C_WRITE(0x3846,0x08); 
    SENSOR_I2C_WRITE(0x3847,0x07); 
    SENSOR_I2C_WRITE(0x3d85,0x36); 
    SENSOR_I2C_WRITE(0x3d8c,0x71); 
    SENSOR_I2C_WRITE(0x3d8d,0xcb); 
    SENSOR_I2C_WRITE(0x3f0a,0x00); 
    SENSOR_I2C_WRITE(0x4000,0x71); 
    SENSOR_I2C_WRITE(0x4001,0x50); 
    SENSOR_I2C_WRITE(0x4002,0x04); 
    SENSOR_I2C_WRITE(0x4003,0x14); 
    SENSOR_I2C_WRITE(0x400e,0x00); 
    SENSOR_I2C_WRITE(0x4011,0x00); 
    SENSOR_I2C_WRITE(0x401a,0x00); 
    SENSOR_I2C_WRITE(0x401b,0x00); 
    SENSOR_I2C_WRITE(0x401c,0x00); 
    SENSOR_I2C_WRITE(0x401d,0x00); 
    SENSOR_I2C_WRITE(0x401f,0x00); 
    SENSOR_I2C_WRITE(0x4020,0x00); 
    SENSOR_I2C_WRITE(0x4021,0x10); 
    SENSOR_I2C_WRITE(0x4022,0x03); 
    SENSOR_I2C_WRITE(0x4023,0x93); 
    SENSOR_I2C_WRITE(0x4024,0x04);
    SENSOR_I2C_WRITE(0x4025,0xC0); 
    SENSOR_I2C_WRITE(0x4026,0x04); 
    SENSOR_I2C_WRITE(0x4027,0xD0); 
    SENSOR_I2C_WRITE(0x4028,0x00); 
    SENSOR_I2C_WRITE(0x4029,0x02); 
    SENSOR_I2C_WRITE(0x402a,0x06); 
    SENSOR_I2C_WRITE(0x402b,0x04); 
    SENSOR_I2C_WRITE(0x402c,0x02); 
    SENSOR_I2C_WRITE(0x402d,0x02); 
    SENSOR_I2C_WRITE(0x402e,0x0e); 
    SENSOR_I2C_WRITE(0x402f,0x04); 
    SENSOR_I2C_WRITE(0x4302,0xff); 
    SENSOR_I2C_WRITE(0x4303,0xff); 
    SENSOR_I2C_WRITE(0x4304,0x00); 
    SENSOR_I2C_WRITE(0x4305,0x00); 
    SENSOR_I2C_WRITE(0x4306,0x00); 
    SENSOR_I2C_WRITE(0x4308,0x02); 
    SENSOR_I2C_WRITE(0x4500,0x6c); 
    SENSOR_I2C_WRITE(0x4501,0xc4); 
    SENSOR_I2C_WRITE(0x4502,0x44); 
    SENSOR_I2C_WRITE(0x4503,0x01); 
    SENSOR_I2C_WRITE(0x4601,0x4F); 
    SENSOR_I2C_WRITE(0x4800,0x04); 
    SENSOR_I2C_WRITE(0x4813,0x08); 
    SENSOR_I2C_WRITE(0x481f,0x40); 
    SENSOR_I2C_WRITE(0x4829,0x78); 
    SENSOR_I2C_WRITE(0x4837,0x1b); 
    SENSOR_I2C_WRITE(0x4b00,0x2a); 
    SENSOR_I2C_WRITE(0x4b0d,0x00); 
    SENSOR_I2C_WRITE(0x4d00,0x04); 
    SENSOR_I2C_WRITE(0x4d01,0x42); 
    SENSOR_I2C_WRITE(0x4d02,0xd1); 
    SENSOR_I2C_WRITE(0x4d03,0x93); 
    SENSOR_I2C_WRITE(0x4d04,0xf5); 
    SENSOR_I2C_WRITE(0x4d05,0xc1); 
    SENSOR_I2C_WRITE(0x5000,0xf3); 
    SENSOR_I2C_WRITE(0x5001,0x11); 
    SENSOR_I2C_WRITE(0x5004,0x00); 
    SENSOR_I2C_WRITE(0x500a,0x00); 
    SENSOR_I2C_WRITE(0x500b,0x00); 
    SENSOR_I2C_WRITE(0x5032,0x00); 
    SENSOR_I2C_WRITE(0x5040,0x00); 
    SENSOR_I2C_WRITE(0x5050,0x3c); 
    SENSOR_I2C_WRITE(0x5500,0x00); 
    SENSOR_I2C_WRITE(0x5501,0x10); 
    SENSOR_I2C_WRITE(0x5502,0x01); 
    SENSOR_I2C_WRITE(0x5503,0x0f); 
    SENSOR_I2C_WRITE(0x8000,0x00); 
    SENSOR_I2C_WRITE(0x8001,0x00); 
    SENSOR_I2C_WRITE(0x8002,0x00); 
    SENSOR_I2C_WRITE(0x8003,0x00); 
    SENSOR_I2C_WRITE(0x8004,0x00); 
    SENSOR_I2C_WRITE(0x8005,0x00); 
    SENSOR_I2C_WRITE(0x8006,0x00); 
    SENSOR_I2C_WRITE(0x8007,0x00); 
    SENSOR_I2C_WRITE(0x8008,0x00); 
    SENSOR_I2C_WRITE(0x3638,0x00); 
    SENSOR_I2C_WRITE(0x0100,0x01); 
    //SENSOR_I2C_WRITE(0x0300,0x60); 
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 720120fps Linear Mode Initial OK!-------\n");
}
void ov4689_linear_720p_180fps_init()
{
    usleep(200000);
    SENSOR_I2C_WRITE(0x0103,0x01); 

    SENSOR_I2C_WRITE(0x3638,0x00); 
    SENSOR_I2C_WRITE(0x0300,0x02); 
    SENSOR_I2C_WRITE(0x0302,0x32); 
    SENSOR_I2C_WRITE(0x0303,0x00);
    SENSOR_I2C_WRITE(0x0304,0x03); 
    SENSOR_I2C_WRITE(0x030b,0x00); 
    SENSOR_I2C_WRITE(0x030d,0x1e); 
    SENSOR_I2C_WRITE(0x030e,0x04); 
    SENSOR_I2C_WRITE(0x030f,0x01); 
    SENSOR_I2C_WRITE(0x0312,0x01); 
    SENSOR_I2C_WRITE(0x031e,0x00); 
    SENSOR_I2C_WRITE(0x3000,0x20); 
    SENSOR_I2C_WRITE(0x3002,0x00); 
    SENSOR_I2C_WRITE(0x3018,0x72); 
    SENSOR_I2C_WRITE(0x3020,0x93); 
    SENSOR_I2C_WRITE(0x3021,0x03); 
    SENSOR_I2C_WRITE(0x3022,0x01); 
    SENSOR_I2C_WRITE(0x3031,0x0a); 
    SENSOR_I2C_WRITE(0x303f,0x0c);
    SENSOR_I2C_WRITE(0x3305,0xf1); 
    SENSOR_I2C_WRITE(0x3307,0x04); 
    SENSOR_I2C_WRITE(0x3309,0x29); 
    SENSOR_I2C_WRITE(0x3500,0x00); 
    SENSOR_I2C_WRITE(0x3501,0x30); 
    SENSOR_I2C_WRITE(0x3502,0x00); 
    SENSOR_I2C_WRITE(0x3503,0x77); 
    SENSOR_I2C_WRITE(0x3504,0x00); 
    SENSOR_I2C_WRITE(0x3505,0x00); 
    SENSOR_I2C_WRITE(0x3506,0x00); 
    SENSOR_I2C_WRITE(0x3507,0x00); 
    SENSOR_I2C_WRITE(0x3508,0x00); 
    SENSOR_I2C_WRITE(0x3509,0x80); 
    SENSOR_I2C_WRITE(0x350a,0x00); 
    SENSOR_I2C_WRITE(0x350b,0x00); 
    SENSOR_I2C_WRITE(0x350c,0x00); 
    SENSOR_I2C_WRITE(0x350d,0x00); 
    SENSOR_I2C_WRITE(0x350e,0x00); 
    SENSOR_I2C_WRITE(0x350f,0x80); 
    SENSOR_I2C_WRITE(0x3510,0x00); 
    SENSOR_I2C_WRITE(0x3511,0x00); 
    SENSOR_I2C_WRITE(0x3512,0x00); 
    SENSOR_I2C_WRITE(0x3513,0x00); 
    SENSOR_I2C_WRITE(0x3514,0x00); 
    SENSOR_I2C_WRITE(0x3515,0x80); 
    SENSOR_I2C_WRITE(0x3516,0x00); 
    SENSOR_I2C_WRITE(0x3517,0x00); 
    SENSOR_I2C_WRITE(0x3518,0x00); 
    SENSOR_I2C_WRITE(0x3519,0x00); 
    SENSOR_I2C_WRITE(0x351a,0x00); 
    SENSOR_I2C_WRITE(0x351b,0x80); 
    SENSOR_I2C_WRITE(0x351c,0x00); 
    SENSOR_I2C_WRITE(0x351d,0x00); 
    SENSOR_I2C_WRITE(0x351e,0x00); 
    SENSOR_I2C_WRITE(0x351f,0x00); 
    SENSOR_I2C_WRITE(0x3520,0x00); 
    SENSOR_I2C_WRITE(0x3521,0x80); 
    SENSOR_I2C_WRITE(0x3522,0x08); 
    SENSOR_I2C_WRITE(0x3524,0x08); 
    SENSOR_I2C_WRITE(0x3526,0x08); 
    SENSOR_I2C_WRITE(0x3528,0x08); 
    SENSOR_I2C_WRITE(0x352a,0x08); 
    SENSOR_I2C_WRITE(0x3602,0x00); 
    SENSOR_I2C_WRITE(0x3603,0x40); 
    SENSOR_I2C_WRITE(0x3604,0x02); 
    SENSOR_I2C_WRITE(0x3605,0x00); 
    SENSOR_I2C_WRITE(0x3606,0x00); 
    SENSOR_I2C_WRITE(0x3607,0x00); 
    SENSOR_I2C_WRITE(0x3609,0x12); 
    SENSOR_I2C_WRITE(0x360a,0x40); 
    SENSOR_I2C_WRITE(0x360c,0x08); 
    SENSOR_I2C_WRITE(0x360f,0xe5); 
    SENSOR_I2C_WRITE(0x3608,0x8f); 
    SENSOR_I2C_WRITE(0x3611,0x00); 
    SENSOR_I2C_WRITE(0x3613,0xf7); 
    SENSOR_I2C_WRITE(0x3616,0x58); 
    SENSOR_I2C_WRITE(0x3619,0x99); 
    SENSOR_I2C_WRITE(0x361b,0x60); 
    SENSOR_I2C_WRITE(0x361c,0x7a); 
    SENSOR_I2C_WRITE(0x361e,0x79); 
    SENSOR_I2C_WRITE(0x361f,0x02); 
    SENSOR_I2C_WRITE(0x3632,0x05); 
    SENSOR_I2C_WRITE(0x3633,0x10); 
    SENSOR_I2C_WRITE(0x3634,0x10); 
    SENSOR_I2C_WRITE(0x3635,0x10); 
    SENSOR_I2C_WRITE(0x3636,0x15); 
    SENSOR_I2C_WRITE(0x3646,0x86); 
    SENSOR_I2C_WRITE(0x364a,0x0b); 
    SENSOR_I2C_WRITE(0x3700,0x17); 
    SENSOR_I2C_WRITE(0x3701,0x22); 
    SENSOR_I2C_WRITE(0x3703,0x10); 
    SENSOR_I2C_WRITE(0x370a,0x37); 
    SENSOR_I2C_WRITE(0x3705,0x00); 
    SENSOR_I2C_WRITE(0x3706,0x63); 
    SENSOR_I2C_WRITE(0x3709,0x3c); 
    SENSOR_I2C_WRITE(0x370b,0x01); 
    SENSOR_I2C_WRITE(0x370c,0x30); 
    SENSOR_I2C_WRITE(0x3710,0x24); 
    SENSOR_I2C_WRITE(0x3711,0x0c); 
    SENSOR_I2C_WRITE(0x3716,0x00); 
    SENSOR_I2C_WRITE(0x3720,0x28); 
    SENSOR_I2C_WRITE(0x3729,0x7b); 
    SENSOR_I2C_WRITE(0x372a,0x84); 
    SENSOR_I2C_WRITE(0x372b,0xbd); 
    SENSOR_I2C_WRITE(0x372c,0xbc); 
    SENSOR_I2C_WRITE(0x372e,0x52); 
    SENSOR_I2C_WRITE(0x373c,0x0e); 
    SENSOR_I2C_WRITE(0x373e,0x33); 
    SENSOR_I2C_WRITE(0x3743,0x10); 
    SENSOR_I2C_WRITE(0x3744,0x88); 
    SENSOR_I2C_WRITE(0x3745,0xc0); 
    SENSOR_I2C_WRITE(0x374a,0x43); 
    SENSOR_I2C_WRITE(0x374c,0x00); 
    SENSOR_I2C_WRITE(0x374e,0x23); 
    SENSOR_I2C_WRITE(0x3751,0x7b); 
    SENSOR_I2C_WRITE(0x3752,0x84); 
    SENSOR_I2C_WRITE(0x3753,0xbd); 
    SENSOR_I2C_WRITE(0x3754,0xbc); 
    SENSOR_I2C_WRITE(0x3756,0x52); 
    SENSOR_I2C_WRITE(0x375c,0x00); 
    SENSOR_I2C_WRITE(0x3760,0x00); 
    SENSOR_I2C_WRITE(0x3761,0x00); 
    SENSOR_I2C_WRITE(0x3762,0x00); 
    SENSOR_I2C_WRITE(0x3763,0x00); 
    SENSOR_I2C_WRITE(0x3764,0x00); 
    SENSOR_I2C_WRITE(0x3767,0x04); 
    SENSOR_I2C_WRITE(0x3768,0x04); 
    SENSOR_I2C_WRITE(0x3769,0x08); 
    SENSOR_I2C_WRITE(0x376a,0x08); 
    SENSOR_I2C_WRITE(0x376b,0x40); 
    SENSOR_I2C_WRITE(0x376c,0x00); 
    SENSOR_I2C_WRITE(0x376d,0x00); 
    SENSOR_I2C_WRITE(0x376e,0x00); 
    SENSOR_I2C_WRITE(0x3773,0x00); 
    SENSOR_I2C_WRITE(0x3774,0x51); 
    SENSOR_I2C_WRITE(0x3776,0xbd); 
    SENSOR_I2C_WRITE(0x3777,0xbd); 
    SENSOR_I2C_WRITE(0x3781,0x18); 
    SENSOR_I2C_WRITE(0x3783,0x25); 
    SENSOR_I2C_WRITE(0x3798,0x1b); 
    SENSOR_I2C_WRITE(0x3800,0x00);
    SENSOR_I2C_WRITE(0x3801,0x48); 
    SENSOR_I2C_WRITE(0x3802,0x00); 
    SENSOR_I2C_WRITE(0x3803,0x2C); 
    SENSOR_I2C_WRITE(0x3804,0x0a); 
    SENSOR_I2C_WRITE(0x3805,0x57); 
    SENSOR_I2C_WRITE(0x3806,0x05); 
    SENSOR_I2C_WRITE(0x3807,0xD3); 
    SENSOR_I2C_WRITE(0x3808,0x05); 
    SENSOR_I2C_WRITE(0x3809,0x00); 
    SENSOR_I2C_WRITE(0x380a,0x02); 
    SENSOR_I2C_WRITE(0x380b,0xD0); 
    SENSOR_I2C_WRITE(0x380c,0x03); 
    SENSOR_I2C_WRITE(0x380d,0x5C); 
    SENSOR_I2C_WRITE(0x380e,0x03); 
    SENSOR_I2C_WRITE(0x380f,0x05);
    SENSOR_I2C_WRITE(0x3810,0x00); 
    SENSOR_I2C_WRITE(0x3811,0x04); 
    SENSOR_I2C_WRITE(0x3812,0x00); 
    SENSOR_I2C_WRITE(0x3813,0x02); 
    SENSOR_I2C_WRITE(0x3814,0x03); 
    SENSOR_I2C_WRITE(0x3815,0x01); 
    SENSOR_I2C_WRITE(0x3819,0x01); 
    SENSOR_I2C_WRITE(0x3820,0x10); 
    SENSOR_I2C_WRITE(0x3821,0x07); 
    SENSOR_I2C_WRITE(0x3829,0x00); 
    SENSOR_I2C_WRITE(0x382a,0x03); 
    SENSOR_I2C_WRITE(0x382b,0x01); 
    SENSOR_I2C_WRITE(0x382d,0x7f); 
    SENSOR_I2C_WRITE(0x3830,0x08); 
    SENSOR_I2C_WRITE(0x3836,0x02); 
    SENSOR_I2C_WRITE(0x3837,0x00); 
    SENSOR_I2C_WRITE(0x3841,0x02); 
    SENSOR_I2C_WRITE(0x3846,0x08); 
    SENSOR_I2C_WRITE(0x3847,0x07); 
    SENSOR_I2C_WRITE(0x3d85,0x36); 
    SENSOR_I2C_WRITE(0x3d8c,0x71); 
    SENSOR_I2C_WRITE(0x3d8d,0xcb); 
    SENSOR_I2C_WRITE(0x3f0a,0x00); 
    SENSOR_I2C_WRITE(0x4000,0x71); 
    SENSOR_I2C_WRITE(0x4001,0x50); 
    SENSOR_I2C_WRITE(0x4002,0x04); 
    SENSOR_I2C_WRITE(0x4003,0x14); 
    SENSOR_I2C_WRITE(0x400e,0x00); 
    SENSOR_I2C_WRITE(0x4011,0x00); 
    SENSOR_I2C_WRITE(0x401a,0x00); 
    SENSOR_I2C_WRITE(0x401b,0x00); 
    SENSOR_I2C_WRITE(0x401c,0x00); 
    SENSOR_I2C_WRITE(0x401d,0x00); 
    SENSOR_I2C_WRITE(0x401f,0x00); 
    SENSOR_I2C_WRITE(0x4020,0x00); 
    SENSOR_I2C_WRITE(0x4021,0x10); 
    SENSOR_I2C_WRITE(0x4022,0x03); 
    SENSOR_I2C_WRITE(0x4023,0x93); 
    SENSOR_I2C_WRITE(0x4024,0x04);
    SENSOR_I2C_WRITE(0x4025,0xC0); 
    SENSOR_I2C_WRITE(0x4026,0x04); 
    SENSOR_I2C_WRITE(0x4027,0xD0); 
    SENSOR_I2C_WRITE(0x4028,0x00); 
    SENSOR_I2C_WRITE(0x4029,0x02); 
    SENSOR_I2C_WRITE(0x402a,0x06); 
    SENSOR_I2C_WRITE(0x402b,0x04); 
    SENSOR_I2C_WRITE(0x402c,0x02); 
    SENSOR_I2C_WRITE(0x402d,0x02); 
    SENSOR_I2C_WRITE(0x402e,0x0e); 
    SENSOR_I2C_WRITE(0x402f,0x04); 
    SENSOR_I2C_WRITE(0x4302,0xff); 
    SENSOR_I2C_WRITE(0x4303,0xff); 
    SENSOR_I2C_WRITE(0x4304,0x00); 
    SENSOR_I2C_WRITE(0x4305,0x00); 
    SENSOR_I2C_WRITE(0x4306,0x00); 
    SENSOR_I2C_WRITE(0x4308,0x02); 
    SENSOR_I2C_WRITE(0x4500,0x6c); 
    SENSOR_I2C_WRITE(0x4501,0xc4); 
    SENSOR_I2C_WRITE(0x4502,0x44); 
    SENSOR_I2C_WRITE(0x4503,0x01); 
    SENSOR_I2C_WRITE(0x4601,0x4F); 
    SENSOR_I2C_WRITE(0x4800,0x04); 
    SENSOR_I2C_WRITE(0x4813,0x08); 
    SENSOR_I2C_WRITE(0x481f,0x40); 
    SENSOR_I2C_WRITE(0x4829,0x78); 
    SENSOR_I2C_WRITE(0x4837,0x1b); 
    SENSOR_I2C_WRITE(0x4b00,0x2a); 
    SENSOR_I2C_WRITE(0x4b0d,0x00); 
    SENSOR_I2C_WRITE(0x4d00,0x04); 
    SENSOR_I2C_WRITE(0x4d01,0x42); 
    SENSOR_I2C_WRITE(0x4d02,0xd1); 
    SENSOR_I2C_WRITE(0x4d03,0x93); 
    SENSOR_I2C_WRITE(0x4d04,0xf5); 
    SENSOR_I2C_WRITE(0x4d05,0xc1); 
    SENSOR_I2C_WRITE(0x5000,0xf3); 
    SENSOR_I2C_WRITE(0x5001,0x11); 
    SENSOR_I2C_WRITE(0x5004,0x00); 
    SENSOR_I2C_WRITE(0x500a,0x00); 
    SENSOR_I2C_WRITE(0x500b,0x00); 
    SENSOR_I2C_WRITE(0x5032,0x00); 
    SENSOR_I2C_WRITE(0x5040,0x00); 
    SENSOR_I2C_WRITE(0x5050,0x3c); 
    SENSOR_I2C_WRITE(0x5500,0x00); 
    SENSOR_I2C_WRITE(0x5501,0x10); 
    SENSOR_I2C_WRITE(0x5502,0x01); 
    SENSOR_I2C_WRITE(0x5503,0x0f); 
    SENSOR_I2C_WRITE(0x8000,0x00); 
    SENSOR_I2C_WRITE(0x8001,0x00); 
    SENSOR_I2C_WRITE(0x8002,0x00); 
    SENSOR_I2C_WRITE(0x8003,0x00); 
    SENSOR_I2C_WRITE(0x8004,0x00); 
    SENSOR_I2C_WRITE(0x8005,0x00); 
    SENSOR_I2C_WRITE(0x8006,0x00); 
    SENSOR_I2C_WRITE(0x8007,0x00); 
    SENSOR_I2C_WRITE(0x8008,0x00); 
    SENSOR_I2C_WRITE(0x3638,0x00); 
    SENSOR_I2C_WRITE(0x0100,0x01); 
    //SENSOR_I2C_WRITE(0x0300,0x60); 
    bSensorInit = HI_TRUE;
    printf("-------OV4689 Sensor 720180fps Linear Mode Initial OK!-------\n");
}

                                                                 

		
////////////////////////////////sensor_init_over//////////////////////////////////////////////////////////////


HI_S32 ov4689_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ov4689_init;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ov4689_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = ov4689_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ov4689_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = ov4689_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ov4689_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ov4689_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = ov4689_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int ov4689_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ov4689_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, OV4689_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    ov4689_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, OV4689_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    ov4689_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, OV4689_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int ov4689_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, OV4689_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, OV4689_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, OV4689_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


HI_S32 OV_OV4689_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)
{
		// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;
	ov4689_register_callback();
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
	printf("OV OV4689 sensor 1536P 25fps init success!\n");


	
return s32Ret;
}
int OV4689_get_sensor_name(char *sensor_name)
{
	if(sensor_name != NULL)
	{
		memcpy(sensor_name,SENSOR_NAME,strlen(SENSOR_NAME));
		return 0;
	}
	return -1;
}

int OV4689_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_OV4689_WIDTH;
        *ret_height = SENSOR_OV4689_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_OV4689_probe()
{
    uint16_t ret_data1 = 0;
    SENSOR_I2C_READ(0x300A, &ret_data1);		//TODO
    if(ret_data1 == OV4689_CHECK_DATA){		//TODO
        sdk_sys->write_reg(0x200f0050, 0x2);//i2c0_scl
        sdk_sys->write_reg(0x200f0054, 0x2);//i2c0_sda
        sdk_sys->write_reg(0x2003002c, 0xE0007);//set sensor clock,sensor unreset, clk 24MHz, VI 250MHz
        sdk_sys->write_reg(0x20030104, 0x0);
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

