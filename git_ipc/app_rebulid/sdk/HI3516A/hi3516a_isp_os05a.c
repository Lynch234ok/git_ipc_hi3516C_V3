#include "sdk/sdk_debug.h"
#include "hi3516a.h"
#include "hi3516a_isp_sensor.h"
#include "sdk/sdk_sys.h"
#include "hi_isp_i2c.h"

#define SENSOR_NAME "os05a"


static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
    (sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x6c, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
    (sensor_i2c_write ? sensor_i2c_write((_add), (_data)) :_i2c_write((_add), (_data), 0x6c, 2, 1))

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#define SENSOR_OS05A_WIDTH 2592
#define SENSOR_OS05A_HEIGHT 1944
#define OS05A_CHECK_DATA_MSB (0x53) //0X300A
#define OS05A_CHECK_DATA_LSB (0x5)  //0X300B

#if !defined(__OS05A_CMOS_H_)
#define __OS05A_CMOS_H_

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
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "hi_comm_video.h"

#include "hi_spi.h"
*/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define OS05A_ID 06

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
static const unsigned int sensor_i2c_addr = 0x6c;
static unsigned int sensor_addr_byte = 2;
static unsigned int sensor_data_byte = 1;

#define FULL_LINES_MAX  (0xFFFF)

#define GAIN_ADDR     (0x350A)
#define VMAX_ADDR_H     (0x380E)
#define VMAX_ADDR_L     (0x380F)

#define SENSOR_AGAIN_H    (0x3508)
#define SENSOR_AGAIN_L   (0x3509)
#define SENSOR_SHORT_AGAIN_H    (0x350C)
#define SENSOR_SHORT_AGAIN_L   (0x350D)
#define SENSOR_GAIN_S   (0x340A)
#define EXPOSURE_TIME_LONG_H  (0x3501)
#define EXPOSURE_TIME_LONG_L  (0x3502)
#define EXPOSURE_TIME_SHORT_H  (0x3511)
#define EXPOSURE_TIME_SHORT_L  (0x3512)
#define LINE_LEN_PCK     (0x300C)  

#define INCREASE_LINES          (0) /* make real fps less than stand fps because NVR require*/
#define OS05A_SENSOR_5M_MODE    (0) 
#define OS05A_VMAX_5M30               (0xb69+INCREASE_LINES)
#define OS05A_VMAX_5M25_WDR           (0x85d+INCREASE_LINES)


static HI_U8 gu8SensorImageMode = OS05A_SENSOR_5M_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = OS05A_VMAX_5M30;
static HI_U32 gu32FullLines = OS05A_VMAX_5M30;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE; 

static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};

/*not use CMOS_CFG_INI */
#define PATHLEN_MAX 256
#define CMOS_CFG_INI "os05a_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/os05a_cfg.ini";

static HI_S32 os05a_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0625;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

    pstAeSnsDft->u32MaxAgain = 15872;
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;
    
    pstAeSnsDft->u32MaxDgain = 1;  
    pstAeSnsDft->u32MinDgain = 1;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;    
    
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
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 8;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;
            
        break;
        case WDR_MODE_2To1_LINE:
            /* FS WDR mode */
            pstAeSnsDft->au8HistThresh[0] = 0xc;
            pstAeSnsDft->au8HistThresh[1] = 0x18;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x38; 
            
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 8;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;
            

        break;
        }
    
    return 0;
}



#define AGAIN_NODE_NUM 64

static HI_U32 au32Again_table[AGAIN_NODE_NUM]=
{    
    1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,
    2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,
    4096,4352,4608,4864,5120,5376,5632,5888,6144,6400,6656,6912,7168,7424,7680,7936,
    8192,8704,9216,9728,10240,10752,11264,11776,12288,12800,13312,13824,14336,14848,15360,15872
};


static HI_VOID os05a_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
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
static HI_VOID os05a_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (OS05A_SENSOR_5M_MODE == gu8SensorImageMode)
    {
        if(genSensorMode == WDR_MODE_NONE)
        {
            if ((f32Fps <= 30) && (f32Fps >= 0.5))
            {
                gu32FullLinesStd = OS05A_VMAX_5M30 * 30 / f32Fps;
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }
        }
        else if(genSensorMode == WDR_MODE_2To1_LINE)
        {
            if ((f32Fps <= 25) && (f32Fps >= 0.5))
            {
                gu32FullLinesStd = OS05A_VMAX_5M25_WDR * 25 / f32Fps;
            }
            else
            {
                printf("Not support Fps: %f\n", f32Fps);
                return;
            }

        }
        else
        {
            printf("Not support sensor mode: %d\n", gu32FullLinesStd);
            return;
        }

    }
    else
    {
        printf("Not support! gu8SensorImageMode:%d, f32Fps:%f\n", gu8SensorImageMode, f32Fps);
        return;
    }

    gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[4].u32Data = ((gu32FullLinesStd & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLinesStd & 0xFF);
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[8].u32Data = ((gu32FullLinesStd & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[9].u32Data = (gu32FullLinesStd & 0xFF);
    }

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 8;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    gu32FullLines = gu32FullLinesStd;
    pstAeSnsDft->u32FullLines = gu32FullLines;

    return;
}

static HI_VOID os05a_slow_framerate_set(HI_U32 u32FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines= u32FullLines;
    pstAeSnsDft->u32FullLines = gu32FullLines;

    if (WDR_MODE_NONE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[4].u32Data = ((gu32FullLines & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLines & 0xFF);
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[8].u32Data = ((gu32FullLines & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[9].u32Data = (gu32FullLines & 0xFF);
    }

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 8;
    
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID os05a_inttime_update(HI_U32 u32IntTime)
{    
    static HI_BOOL bFirst = HI_TRUE;

     if (WDR_MODE_2To1_LINE == genSensorMode)
    {
        if(bFirst)/* short exposure */
        {
            g_stSnsRegsInfo.astI2cData[4].u32Data = ((u32IntTime & 0xFF00) >> 8);
            g_stSnsRegsInfo.astI2cData[5].u32Data = (u32IntTime & 0xFF);
            bFirst = HI_FALSE;
        }
        else/* long exposure */
        {
            g_stSnsRegsInfo.astI2cData[0].u32Data =  ((u32IntTime & 0xFF00) >> 8);
            g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime & 0xFF);
            bFirst = HI_TRUE;
        }   
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[0].u32Data =  ((u32IntTime & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime & 0xFF);

    }
    
    return;
}


static HI_VOID os05a_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    HI_U32 AGain_Reg = au32Again_table[u32Again] >> 3;
    

    g_stSnsRegsInfo.astI2cData[2].u32Data =  ((AGain_Reg & 0x3F00) >> 8);
    g_stSnsRegsInfo.astI2cData[3].u32Data = (AGain_Reg & 0x00FF);
    
    if (WDR_MODE_2To1_LINE == genSensorMode)
    {
        g_stSnsRegsInfo.astI2cData[6].u32Data = g_stSnsRegsInfo.astI2cData[2].u32Data;
        g_stSnsRegsInfo.astI2cData[7].u32Data = g_stSnsRegsInfo.astI2cData[3].u32Data;
    }
    
    return;
}

/* Only used in WDR_MODE_2To1_LINE and WDR_MODE_2To1_FRAME mode */
static HI_VOID os05a_get_inttime_max(HI_U32 u32Ratio, HI_U32 *pu32IntTimeMax)
{
    if(HI_NULL == pu32IntTimeMax)
    {
        printf("null pointer when get ae sensor IntTimeMax value!\n");
        return;
    }

    if (WDR_MODE_2To1_LINE == genSensorMode)
    {
        *pu32IntTimeMax = (gu32FullLines - 8) * 0x40 / (u32Ratio + 0x40);
    }

    return;
}


HI_S32 os05a_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = os05a_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = os05a_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= os05a_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = os05a_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = os05a_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = os05a_again_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = os05a_get_inttime_max;

    return 0;
}
#ifdef INIFILE_CONFIG_MODE
#else
static AWB_CCM_S g_stAwbCcm =
{
   4850,
   {
        0x01bf, 0x809f, 0x8020,
        0x804e, 0x01a8, 0x805a,
        0x0010, 0x80ee, 0x01dc
   },
   
   3100,
   {
        0x01b7, 0x807c, 0x803a,
        0x807e, 0x01c0, 0x8042,
        0x001c, 0x813c, 0x021f
   },
   
   2450,
   {     
        0x0183, 0x8078, 0x800a,
        0x8073, 0x0186, 0x8013,
        0x0010, 0x820a, 0x02fa
   }
};


static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x80,0x80,0x7e,0x72,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
};

static AWB_AGC_TABLE_S g_stAwbAgcTableFSWDR =
{
    /* bvalid */
    1,

    /* saturation */ 
    {0x80,0x80,0x7e,0x72,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38}
};




static HI_S32 os05a_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4850;

    pstAwbSnsDft->au16GainOffset[0] = 0x212;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1c8;

    pstAwbSnsDft->as32WbPara[0] = 63;
    pstAwbSnsDft->as32WbPara[1] = 76;
    pstAwbSnsDft->as32WbPara[2] = -117;
    pstAwbSnsDft->as32WbPara[3] = 119178;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -73499;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));

    switch (genSensorMode)
    {
        default:
        case WDR_MODE_NONE:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
        break;
        case WDR_MODE_2To1_LINE:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableFSWDR, sizeof(AWB_AGC_TABLE_S));
        break;
    }

    return 0;
}
#endif



HI_S32 os05a_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = os05a_get_awb_default;

    return 0;
}

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
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
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2D,   

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
        0, 38406, 39281, 40156, 41031, 41907, 42782, 43657, 44532, 45407, 46282, 47158, 48033, 48908, 49783, 50658, 51533, 52409, 53284, 54159, 55034, 55909, 56784, 57660, 58535, 59410, 60285, 61160, 62035, 62911, 63786, 64661, 65535
    },

    /* gamma_fe1 */
    {
        0, 72, 145, 218, 293, 369, 446, 524, 604, 685, 767, 851, 937, 1024, 1113, 1204, 1297, 1391, 1489, 1590, 1692, 1798, 1907, 2020, 2136, 2258, 2383, 2515, 2652, 2798, 2952, 3116, 3295, 3490, 3708, 3961, 4272, 4721, 5954, 6407, 6719, 6972, 7190, 7386, 7564, 7729, 7884, 8029, 8167, 8298, 8424, 8545, 8662, 8774, 8883, 8990, 9092, 9192, 9289, 9385, 9478, 9569, 9658, 9745, 9831, 9915, 9997, 10078, 10158, 10236, 10313, 10389, 10463, 10538, 10610, 10682, 10752, 10823, 10891, 10959, 11026, 11094, 11159, 11224, 11289, 11352, 11415, 11477, 11539, 11600, 11660, 11720, 11780, 11838, 11897, 11954, 12012, 12069, 12125, 12181, 12236, 12291, 12346, 12399, 12453, 12507, 12559, 12612, 12664, 12716, 12768, 12818, 12869, 12919, 12970, 13020, 13069, 13118, 13166, 13215, 13263, 13311, 13358, 13405, 13453, 13500, 13546, 13592, 13638, 13684, 13730, 13775, 13820, 13864, 13909, 13953, 13997, 14041, 14085, 14128, 14172, 14214, 14257, 14299, 14342, 14384, 14426, 14468, 14509, 14551, 14592, 16213, 17654, 18942, 20118, 21208, 22227, 23189, 24101, 24971, 25804, 26603, 27373, 28118, 28838, 29538, 30219, 30881, 31527, 32156, 32772, 33375, 33964, 34541, 35107, 35663, 36208, 36745, 37272, 37790, 38301, 38803, 39298, 39785, 40267, 40741, 41210, 41672, 42128, 42580, 43026, 43466, 43901, 44332, 44757, 45179, 45596, 46008, 46417, 46821, 47222, 47619, 48011, 48400, 48785, 49168, 49547, 49924, 50296, 50666, 51033, 51397, 51758, 52116, 52472, 52825, 53175, 53522, 53868, 54211, 54551, 54889, 55225, 55558, 55889, 56218, 56545, 56870, 57193, 57514, 57833, 58150, 58465, 58778, 59090, 59399, 59708, 60014, 60318, 60621, 60922, 61222, 61520, 61816, 62111, 62403, 62695, 62985, 63275, 63562, 63848, 64132, 64416, 64698, 64978, 65258, 65535
    }
};



HI_U32 os05a_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            pstDef->stDrc.u32Asymmetry          = 0x14;
            pstDef->stDrc.u32BrightEnhance      = 0xC8;
            
            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
            memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
            memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));
        break;

        case WDR_MODE_2To1_LINE:
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
            memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpenFSWDR, sizeof(ISP_CMOS_RGBSHARPEN_S));            
            memcpy(&pstDef->stGamma, &g_stIspGammaFSWDR, sizeof(ISP_CMOS_GAMMA_S));
            memcpy(&pstDef->stGammafe, &g_stGammafeFSWDR, sizeof(ISP_CMOS_GAMMAFE_S));
        break;
        default:
        break;
    }

    pstDef->stSensorMaxResolution.u32MaxWidth  = 2592;
    pstDef->stSensorMaxResolution.u32MaxHeight = 1944;

    return 0;
}


HI_U32 os05a_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
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
        pstBlackLevel->au16BlackLevel[i] = 256; /*12bit,0xC8 */

    }
    return 0;    
}

HI_VOID os05a_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps = OS05A_VMAX_5M30;
    HI_U32 u32MaxExpTime_5Fps = OS05A_VMAX_5M30 - 8;
    
    if ((OS05A_SENSOR_5M_MODE == gu8SensorImageMode) && (WDR_MODE_NONE == genSensorMode))
    {
        u32FullLines_5Fps = OS05A_VMAX_5M30 * 30 / 5;
    }
    else
    {
        printf("Please run pixel detect in 5M linear mode!\n");
        return;
    }

    u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
    u32MaxExpTime_5Fps = u32FullLines_5Fps - 8;
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR_L, u32FullLines_5Fps & 0xFF); 
        SENSOR_I2C_WRITE(VMAX_ADDR_H, (u32FullLines_5Fps & 0xFF00) >> 8 );
        
        SENSOR_I2C_WRITE(EXPOSURE_TIME_LONG_H, (u32MaxExpTime_5Fps & 0xFF00) >> 8);    /* shutter */
        SENSOR_I2C_WRITE(EXPOSURE_TIME_LONG_L, u32MaxExpTime_5Fps & 0xFF);
        
        SENSOR_I2C_WRITE(SENSOR_AGAIN_H, 0x00); //gain
        SENSOR_I2C_WRITE(SENSOR_AGAIN_L, 0x90);
    }
    else /* setup for ISP 'normal mode' */
    {
        
        gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;
        SENSOR_I2C_WRITE (VMAX_ADDR_L, gu32FullLinesStd & 0xFF); 
        SENSOR_I2C_WRITE (VMAX_ADDR_H, (gu32FullLinesStd & 0xFF00) >> 8);
        bInit = HI_FALSE;
    }

    return;
}

HI_VOID os05a_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            genSensorMode = WDR_MODE_NONE;
            gu32FullLinesStd = OS05A_VMAX_5M30;
            printf("linear mode\n");
        break;

        case WDR_MODE_2To1_LINE:
            genSensorMode = WDR_MODE_2To1_LINE;
            gu32FullLinesStd = OS05A_VMAX_5M25_WDR;
            printf("2to1 half-rate line WDR mode\n");
        break;

        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }

    gu32FullLines = gu32FullLinesStd;
    
    return;
}

static HI_S32 os05a_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
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
        if (WDR_MODE_2To1_LINE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= 25)
            {
                u8SensorImageMode = OS05A_SENSOR_5M_MODE;
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
        else if(WDR_MODE_NONE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = OS05A_SENSOR_5M_MODE;
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

HI_U32 os05a_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;
    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;        
        g_stSnsRegsInfo.u32RegNum = 6;
        
        if ((WDR_MODE_2To1_LINE == genSensorMode))
        {
            g_stSnsRegsInfo.u32RegNum += 4; 
        }
        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }        
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0; 
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = EXPOSURE_TIME_LONG_H;        // Long shutter   
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = EXPOSURE_TIME_LONG_L;        // Long shutter 
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = SENSOR_AGAIN_H;        // Again  
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = SENSOR_AGAIN_L;        // Again
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0; 
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = VMAX_ADDR_H;        // Vmax
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = VMAX_ADDR_L;        // Vmax
        
        if (WDR_MODE_2To1_LINE == genSensorMode )
        {
            g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[4].u32RegAddr = EXPOSURE_TIME_SHORT_H;     // Short shutter   
            g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[5].u32RegAddr = EXPOSURE_TIME_SHORT_L;     // Short shutter
            g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[6].u32RegAddr = SENSOR_SHORT_AGAIN_H;     // Again
            g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = 0;
            g_stSnsRegsInfo.astI2cData[7].u32RegAddr = SENSOR_SHORT_AGAIN_L;     // Again 
            g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[8].u32RegAddr = VMAX_ADDR_H;    // Vmax
            g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = 1;
            g_stSnsRegsInfo.astI2cData[9].u32RegAddr = VMAX_ADDR_L;    // Vmax
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

int  os05a_set_inifile_path(const char *pcPath)
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

HI_VOID os05a_global_init()
{   
    gu8SensorImageMode = OS05A_SENSOR_5M_MODE;
    genSensorMode = WDR_MODE_NONE;   
    gu32FullLinesStd = OS05A_VMAX_5M30;
    gu32FullLines = OS05A_VMAX_5M30;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 
    
    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));  
}

void os05a_reg_init()
{
    SENSOR_I2C_WRITE(0x4600, 0x01);
    //SENSOR_DELAY_MS(200);
    SENSOR_I2C_WRITE(0x4601, 0x04);
    SENSOR_I2C_WRITE(0x4603, 0x00);
    SENSOR_I2C_WRITE(0x0103, 0x01);
    SENSOR_I2C_WRITE(0x0303, 0x01);
    SENSOR_I2C_WRITE(0x0305, 0x27);
    SENSOR_I2C_WRITE(0x0306, 0x00);
    SENSOR_I2C_WRITE(0x0307, 0x00);
    SENSOR_I2C_WRITE(0x0308, 0x03);
    SENSOR_I2C_WRITE(0x0309, 0x04);
    SENSOR_I2C_WRITE(0x032a, 0x00);
    SENSOR_I2C_WRITE(0x031e, 0x09);
    SENSOR_I2C_WRITE(0x0325, 0x48);
    SENSOR_I2C_WRITE(0x0328, 0x07);
    SENSOR_I2C_WRITE(0x300d, 0x11);
    SENSOR_I2C_WRITE(0x300e, 0x11);
    SENSOR_I2C_WRITE(0x300f, 0x11);
    SENSOR_I2C_WRITE(0x3010, 0x01);
    SENSOR_I2C_WRITE(0x3012, 0x41);
    SENSOR_I2C_WRITE(0x3016, 0xf0);
    SENSOR_I2C_WRITE(0x3018, 0xf0);
    SENSOR_I2C_WRITE(0x3028, 0xf0);
    SENSOR_I2C_WRITE(0x301e, 0x98);
    SENSOR_I2C_WRITE(0x3010, 0x04);
    SENSOR_I2C_WRITE(0x3011, 0x06);
    SENSOR_I2C_WRITE(0x3031, 0xa9);
    SENSOR_I2C_WRITE(0x3103, 0x48);
    SENSOR_I2C_WRITE(0x3104, 0x01);
    SENSOR_I2C_WRITE(0x3106, 0x10);
    SENSOR_I2C_WRITE(0x3400, 0x04);
    SENSOR_I2C_WRITE(0x3025, 0x03);
    SENSOR_I2C_WRITE(0x3425, 0x51);
    SENSOR_I2C_WRITE(0x3428, 0x01);
    SENSOR_I2C_WRITE(0x3406, 0x08);
    SENSOR_I2C_WRITE(0x3408, 0x03);
    SENSOR_I2C_WRITE(0x3501, 0x08);
    SENSOR_I2C_WRITE(0x3502, 0x6f);
    SENSOR_I2C_WRITE(0x3505, 0x83);
    SENSOR_I2C_WRITE(0x3508, 0x00);
    SENSOR_I2C_WRITE(0x3509, 0x80);
    SENSOR_I2C_WRITE(0x350a, 0x04);
    SENSOR_I2C_WRITE(0x350b, 0x00);
    SENSOR_I2C_WRITE(0x350c, 0x00);
    SENSOR_I2C_WRITE(0x350d, 0x80);
    SENSOR_I2C_WRITE(0x350e, 0x04);
    SENSOR_I2C_WRITE(0x350f, 0x00);
    SENSOR_I2C_WRITE(0x3600, 0x00);
    SENSOR_I2C_WRITE(0x3626, 0xff);
    SENSOR_I2C_WRITE(0x3605, 0x50);
    SENSOR_I2C_WRITE(0x3609, 0xb5);
    SENSOR_I2C_WRITE(0x3610, 0x69);
    SENSOR_I2C_WRITE(0x360c, 0x01);
    SENSOR_I2C_WRITE(0x3628, 0xa4);
    SENSOR_I2C_WRITE(0x3629, 0x6a);
    SENSOR_I2C_WRITE(0x362d, 0x10);
    SENSOR_I2C_WRITE(0x3660, 0x43);
    SENSOR_I2C_WRITE(0x3661, 0x06);
    SENSOR_I2C_WRITE(0x3662, 0x00);
    SENSOR_I2C_WRITE(0x3663, 0x28);
    SENSOR_I2C_WRITE(0x3664, 0x0d);
    SENSOR_I2C_WRITE(0x366a, 0x38);
    SENSOR_I2C_WRITE(0x366b, 0xa0);
    SENSOR_I2C_WRITE(0x366d, 0x00);
    SENSOR_I2C_WRITE(0x366e, 0x00);
    SENSOR_I2C_WRITE(0x3680, 0x00);
    SENSOR_I2C_WRITE(0x3621, 0x81);
    SENSOR_I2C_WRITE(0x3634, 0x31);
    SENSOR_I2C_WRITE(0x3620, 0x00);
    SENSOR_I2C_WRITE(0x3622, 0x00);
    SENSOR_I2C_WRITE(0x362a, 0xd0);
    SENSOR_I2C_WRITE(0x362e, 0x8c);
    SENSOR_I2C_WRITE(0x362f, 0x98);
    SENSOR_I2C_WRITE(0x3630, 0xb0);
    SENSOR_I2C_WRITE(0x3631, 0xd7);
    SENSOR_I2C_WRITE(0x3701, 0x0f);
    SENSOR_I2C_WRITE(0x3737, 0x02);
    SENSOR_I2C_WRITE(0x3741, 0x04);
    SENSOR_I2C_WRITE(0x373c, 0x0f);
    SENSOR_I2C_WRITE(0x373b, 0x02);
    SENSOR_I2C_WRITE(0x3705, 0x00);
    SENSOR_I2C_WRITE(0x3706, 0x50);
    SENSOR_I2C_WRITE(0x370a, 0x00);
    SENSOR_I2C_WRITE(0x370b, 0xe4);
    SENSOR_I2C_WRITE(0x3709, 0x4a);
    SENSOR_I2C_WRITE(0x3714, 0x21);
    SENSOR_I2C_WRITE(0x371c, 0x00);
    SENSOR_I2C_WRITE(0x371d, 0x08);
    SENSOR_I2C_WRITE(0x375e, 0x0b);
    SENSOR_I2C_WRITE(0x3776, 0x10);
    SENSOR_I2C_WRITE(0x3781, 0x02);
    SENSOR_I2C_WRITE(0x3782, 0x04);
    SENSOR_I2C_WRITE(0x3783, 0x02);
    SENSOR_I2C_WRITE(0x3784, 0x08);
    SENSOR_I2C_WRITE(0x3785, 0x08);
    SENSOR_I2C_WRITE(0x3788, 0x01);
    SENSOR_I2C_WRITE(0x3789, 0x01);
    SENSOR_I2C_WRITE(0x3797, 0x04);
    SENSOR_I2C_WRITE(0x3800, 0x00);
    SENSOR_I2C_WRITE(0x3801, 0x00);
    SENSOR_I2C_WRITE(0x3802, 0x00);
    SENSOR_I2C_WRITE(0x3803, 0x0c);
    SENSOR_I2C_WRITE(0x3804, 0x0e);
    SENSOR_I2C_WRITE(0x3805, 0xff);
    SENSOR_I2C_WRITE(0x3806, 0x08);
    SENSOR_I2C_WRITE(0x3807, 0x6f);
    SENSOR_I2C_WRITE(0x3808, 0x0a);
    SENSOR_I2C_WRITE(0x3809, 0x20);
    SENSOR_I2C_WRITE(0x380a, 0x07);
    SENSOR_I2C_WRITE(0x380b, 0x98);
    SENSOR_I2C_WRITE(0x380c, 0x04);
    SENSOR_I2C_WRITE(0x380d, 0xd0);
    SENSOR_I2C_WRITE(0x380e, 0x08);
    SENSOR_I2C_WRITE(0x380f, 0x8f);
    SENSOR_I2C_WRITE(0x3813, 0x04);
    SENSOR_I2C_WRITE(0x3814, 0x01);
    SENSOR_I2C_WRITE(0x3815, 0x01);
    SENSOR_I2C_WRITE(0x3816, 0x01);
    SENSOR_I2C_WRITE(0x3817, 0x01);
    SENSOR_I2C_WRITE(0x381c, 0x00);
    SENSOR_I2C_WRITE(0x3820, 0x00);
    SENSOR_I2C_WRITE(0x3821, 0x04);
    SENSOR_I2C_WRITE(0x3823, 0x18);
    SENSOR_I2C_WRITE(0x3826, 0x00);
    SENSOR_I2C_WRITE(0x3827, 0x01);
    SENSOR_I2C_WRITE(0x3832, 0x02);
    SENSOR_I2C_WRITE(0x383c, 0x48);
    SENSOR_I2C_WRITE(0x383d, 0xff);
    SENSOR_I2C_WRITE(0x3843, 0x20);
    SENSOR_I2C_WRITE(0x382d, 0x08);
    SENSOR_I2C_WRITE(0x3d85, 0x0b);
    SENSOR_I2C_WRITE(0x3d84, 0x40);
    SENSOR_I2C_WRITE(0x3d8c, 0x63);
    SENSOR_I2C_WRITE(0x3d8d, 0x00);
    SENSOR_I2C_WRITE(0x4000, 0x78);
    SENSOR_I2C_WRITE(0x4001, 0x2b);
    SENSOR_I2C_WRITE(0x4005, 0x40);
    SENSOR_I2C_WRITE(0x4028, 0x2f);
    SENSOR_I2C_WRITE(0x400a, 0x01);
    SENSOR_I2C_WRITE(0x4010, 0x12);
    SENSOR_I2C_WRITE(0x4008, 0x02);
    SENSOR_I2C_WRITE(0x4009, 0x0d);
    SENSOR_I2C_WRITE(0x401a, 0x58);
    SENSOR_I2C_WRITE(0x4050, 0x00);
    SENSOR_I2C_WRITE(0x4051, 0x01);
    SENSOR_I2C_WRITE(0x4052, 0x00);
    SENSOR_I2C_WRITE(0x4053, 0x80);
    SENSOR_I2C_WRITE(0x4054, 0x00);
    SENSOR_I2C_WRITE(0x4055, 0x80);
    SENSOR_I2C_WRITE(0x4056, 0x00);
    SENSOR_I2C_WRITE(0x4057, 0x80);
    SENSOR_I2C_WRITE(0x4058, 0x00);
    SENSOR_I2C_WRITE(0x4059, 0x80);
    SENSOR_I2C_WRITE(0x430b, 0xff);
    SENSOR_I2C_WRITE(0x430c, 0xff);
    SENSOR_I2C_WRITE(0x430d, 0x00);
    SENSOR_I2C_WRITE(0x430e, 0x00);
    SENSOR_I2C_WRITE(0x4501, 0x18);
    SENSOR_I2C_WRITE(0x4502, 0x00);
    SENSOR_I2C_WRITE(0x4643, 0x00);
    SENSOR_I2C_WRITE(0x4640, 0x01);
    SENSOR_I2C_WRITE(0x4641, 0x04);
    SENSOR_I2C_WRITE(0x480e, 0x00);
    SENSOR_I2C_WRITE(0x4813, 0x00);
    SENSOR_I2C_WRITE(0x4815, 0x2b);
    SENSOR_I2C_WRITE(0x486e, 0x36);
    SENSOR_I2C_WRITE(0x486f, 0x84);
    SENSOR_I2C_WRITE(0x4860, 0x00);
    SENSOR_I2C_WRITE(0x4861, 0xa0);
    SENSOR_I2C_WRITE(0x484b, 0x05);
    SENSOR_I2C_WRITE(0x4850, 0x00);
    SENSOR_I2C_WRITE(0x4851, 0xaa);
    SENSOR_I2C_WRITE(0x4852, 0xff);
    SENSOR_I2C_WRITE(0x4853, 0x8a);
    SENSOR_I2C_WRITE(0x4854, 0x08);
    SENSOR_I2C_WRITE(0x4855, 0x30);
    SENSOR_I2C_WRITE(0x4800, 0x00);
    SENSOR_I2C_WRITE(0x4837, 0x19);
    SENSOR_I2C_WRITE(0x5000, 0xc9);
    SENSOR_I2C_WRITE(0x5001, 0x43);
    SENSOR_I2C_WRITE(0x5211, 0x03);
    SENSOR_I2C_WRITE(0x5291, 0x03);
    SENSOR_I2C_WRITE(0x520d, 0x0f);
    SENSOR_I2C_WRITE(0x520e, 0xfd);
    SENSOR_I2C_WRITE(0x520f, 0xf5);
    SENSOR_I2C_WRITE(0x5210, 0xf5);
    SENSOR_I2C_WRITE(0x528d, 0x02);
    SENSOR_I2C_WRITE(0x528e, 0xfd);
    SENSOR_I2C_WRITE(0x528f, 0xf5);
    SENSOR_I2C_WRITE(0x5290, 0xf5);
    SENSOR_I2C_WRITE(0x5004, 0x40);
    SENSOR_I2C_WRITE(0x5005, 0x00);
    SENSOR_I2C_WRITE(0x5180, 0x00);
    SENSOR_I2C_WRITE(0x5181, 0x10);
    SENSOR_I2C_WRITE(0x5182, 0x0f);
    SENSOR_I2C_WRITE(0x5183, 0xff);
    SENSOR_I2C_WRITE(0x580b, 0x03);
    SENSOR_I2C_WRITE(0x4700, 0x2b);
    SENSOR_I2C_WRITE(0x4e00, 0x2b);
    SENSOR_I2C_WRITE(0x4000, 0x79);
    SENSOR_I2C_WRITE(0x380c, 0x04);
    SENSOR_I2C_WRITE(0x380d, 0xd0);
    SENSOR_I2C_WRITE(0x380e, 0x0b);
    SENSOR_I2C_WRITE(0x380f, 0x6a);
    //SENSOR_I2C_WRITE(0x380e, 0x08);
    //SENSOR_I2C_WRITE(0x380f, 0x8f);
    //SENSOR_I2C_WRITE(0x3501, 0x00);
    //SENSOR_I2C_WRITE(0x3502, 0x6f);
  
    //SENSOR_I2C_WRITE(0x3511, 0x00);
    //SENSOR_I2C_WRITE(0x3512, 0x02);
    SENSOR_I2C_WRITE(0x3503, 0x88);
    SENSOR_I2C_WRITE(0x3501, 0xb);
    SENSOR_I2C_WRITE(0x3502, 0x4a);
    SENSOR_I2C_WRITE(0x0100, 0x01);
    //SENSOR_DELAY_MS(10);
    printf("-------OV ov05a Sensor linear 5M30fps Initial OK!-------\n");
    bSensorInit = HI_TRUE;
}

HI_S32 os05a_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = os05a_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = os05a_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = os05a_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = os05a_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = os05a_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = os05a_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = os05a_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = os05a_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int os05a_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    os05a_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, OS05A_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    os05a_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, OS05A_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    os05a_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, OS05A_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int os05a_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, OS05A_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, OS05A_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, OS05A_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}
//<ziv>need to fix
HI_S32 OS05A_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)
{   
    //SENSOR_EXP_FUNC_S sensor_exp_func;

    // init i2c buf
    sensor_i2c_read = do_i2c_read;
    sensor_i2c_write = do_i2c_write;

       //ar0521_reg_init();

    os05a_register_callback();

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
    printf(" os05a sensor 1536P 25fps init success!\n");
    
    return 0;

}

int OS05A_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_OS05A_WIDTH;
        *ret_height = SENSOR_OS05A_HEIGHT;
        return 0;
    }
    return -1;
}
int OS05A_get_sensor_name(char *sensor_name)
{
    if(sensor_name != NULL)
    {
        memcpy(sensor_name,SENSOR_NAME,strlen(SENSOR_NAME));
        return 0;
    }
    return -1;
}

bool SENSOR_OS05A_probe()
{
    uint16_t ret_data1 = 0;
    uint16_t ret_data2 = 0;

    uint16_t ret_data10 = 0;
    uint16_t ret_data11 = 0;
    uint16_t ret_data12 = 0;
    uint16_t ret_data13 = 0;

    sdk_sys->read_reg(0x200f0050, &ret_data10); //i2c0_scl
    sdk_sys->read_reg(0x200f0054, &ret_data11); //i2c0_sda
    sdk_sys->read_reg(0x2003002c, &ret_data12);// sensor unreset, clk 24MHz, VI 297MHz
    sdk_sys->read_reg(0x20030104, &ret_data13); //Sensor 24M

    sdk_sys->write_reg(0x200f0050, 0x2); //i2c0_scl
    sdk_sys->write_reg(0x200f0054, 0x2); //i2c0_sda
    sdk_sys->write_reg(0x2003002c, 0xC0003);// sensor unreset, clk 24MHz, VI 297MHz
    sdk_sys->write_reg(0x20030104, 0x0); //Sensor 24M

    SENSOR_I2C_READ(0x300A, &ret_data1);
    SENSOR_I2C_READ(0x300B, &ret_data2);

    printf("\033[33m [DEBUG][%d] \033[0m \n",__LINE__);
    if(OS05A_CHECK_DATA_MSB ==  ret_data1 && OS05A_CHECK_DATA_LSB == ret_data2)
    {
        return true;
    }

    sdk_sys->write_reg(0x200f0050, ret_data10);
    sdk_sys->write_reg(0x200f0054, ret_data11);
    sdk_sys->write_reg(0x2003002c, ret_data12);
    sdk_sys->write_reg(0x20030104, ret_data13);
    return false;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif 
