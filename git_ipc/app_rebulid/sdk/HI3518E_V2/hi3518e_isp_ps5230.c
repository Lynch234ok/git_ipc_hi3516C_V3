#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "ps5230"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x90, 1, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x90, 1, 1))

#define SENSOR_PS5230_WIDTH 1920
#define SENSOR_PS5230_HEIGHT 1080
#define PS5230_CHECK_DATA_LSB	(0x52)//0x0
#define PS5230_CHECK_DATA_MSB	(0x30)//0x1
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__PS5230_CMOS_H_)
#define __PS5230_CMOS_H_

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


#define PS5230_ID 5230

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

static const unsigned int sensor_i2c_addr=0x90;
static unsigned int sensor_addr_byte=1;
static unsigned int sensor_data_byte=1;

#define FULL_LINES_MAX  (0xFFFF)

#define EXPOSURE_ADDR_H       (0xc)
#define EXPOSURE_ADDR_L       (0xd)
#define AGC_ADDR_H                  (0x78)
#define AGC_ADDR_L                  (0x79)
#define VMAX_ADDR_H               (0xA)
#define VMAX_ADDR_L               (0xB)
#define BANK1_UDPDATE            (0x9)
#define PS5230_MIN_INT           (3)

#define INCREASE_LINES (1) /* make real fps less than stand fps because NVR require*/
#define VMAX_1080P30_LINEAR  (1124+INCREASE_LINES)



#define SENSOR_1080P_30FPS_MODE (1)

static HI_U8 gu8SensorImageMode = SENSOR_1080P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = VMAX_1080P30_LINEAR;
static HI_U32 gu32FullLines = VMAX_1080P30_LINEAR;

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
#define CMOS_CFG_INI "ps5230_cfg.ini"
static char pcName[PATHLEN_MAX] = "configs/ps5230_cfg.ini";

/* AE default parameter and function */
#ifdef INIFILE_CONFIG_MODE
static HI_S32 ps5230_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.1;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;
    
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
            
            pstAeSnsDft->u32MaxAgain = 16832*2;  
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
        
    }    
    return 0;
}

#else

static HI_S32 ps5230_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    memset(&pstAeSnsDft->stAERouteAttr, 0, sizeof(ISP_AE_ROUTE_S));
      
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;

    if((pstAeSnsDft->f32Fps == 25) || (pstAeSnsDft->f32Fps == 30))
    {
        pstAeSnsDft->stIntTimeAccu.f32Offset = 0.8045;
    }
    else
    {}    

    pstAeSnsDft->enIrisType = ISP_IRIS_DC_TYPE;
    memcpy(&pstAeSnsDft->stPirisAttr, &gstPirisAttr, sizeof(ISP_PIRIS_ATTR_S));
    pstAeSnsDft->enMaxIrisFNO = ISP_IRIS_F_NO_1_4;
    pstAeSnsDft->enMinIrisFNO = ISP_IRIS_F_NO_5_6;
    
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x38;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0.8045;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32MinIntTime = PS5230_MIN_INT;
    pstAeSnsDft->u32MaxIntTimeTarget =  65535;
    pstAeSnsDft->u32MinIntTimeTarget = PS5230_MIN_INT;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.38;
    pstAeSnsDft->u32MaxAgain = 32382; 
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1.0/256;
    pstAeSnsDft->u32MaxDgain = 256;  
    pstAeSnsDft->u32MinDgain = 256;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;    
    
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 16 << pstAeSnsDft->u32ISPDgainShift;
    

    return 0;
}
#endif


/* the function of sensor set fps */
static HI_VOID ps5230_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if ((f32Fps <= 30) && (f32Fps >= 0.5))
    {
        /* In 1080P30fps mode, the VMAX(FullLines) is VMAX_1080P30_LINEAR, 
             and there are (VMAX_1080P30_LINEAR*30) lines in 1 second,
             so in f32Fps mode, VMAX(FullLines) is (VMAX_1080P30_LINEAR*30)/f32Fps */
        gu32FullLinesStd = (VMAX_1080P30_LINEAR * 30) / f32Fps;
    }
    else
    {
        printf("Not support Fps: %f\n", f32Fps);
        return;
    }
       
    gu32FullLinesStd = (gu32FullLinesStd > FULL_LINES_MAX) ? FULL_LINES_MAX : gu32FullLinesStd;

        g_stSnsRegsInfo.astI2cData[4].u32Data = ((gu32FullLinesStd & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLinesStd & 0xFF);        


    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * f32Fps / 2;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    gu32FullLines = gu32FullLinesStd;

    pstAeSnsDft->u32FullLines = gu32FullLines;

    return;
}

static HI_VOID ps5230_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;
    gu32FullLines = u32FullLines;
    pstAeSnsDft->u32FullLines = gu32FullLines;
  
    g_stSnsRegsInfo.astI2cData[4].u32Data = ((gu32FullLines & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[5].u32Data = (gu32FullLines & 0xFF);        
  

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 2;

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ps5230_inttime_update(HI_U32 u32IntTime)
{
    HI_S32  u32IntTimeTmp;
       // printf("cmos_inttime_update! u32IntTime=%d\n", u32IntTime);

    u32IntTimeTmp = (u32IntTime < PS5230_MIN_INT) ? PS5230_MIN_INT:u32IntTime ;
    u32IntTimeTmp = gu32FullLinesStd - u32IntTimeTmp ;
	//printf("################# not u32IntTimeTmp=%d\n", u32IntTimeTmp);

    {
        g_stSnsRegsInfo.astI2cData[0].u32Data = ((u32IntTimeTmp & 0xFF00) >> 8);
        g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTimeTmp & 0xFF);
    }
	
    return;
}

static HI_U32 ad_gain_table[73]=
{    
	1536 ,1600 ,1664 ,1728 ,1792 ,1856 ,1920 ,1984 ,2048 ,2175 ,
	2305 ,2431 ,2561 ,2689 ,2817 ,2943 ,3073 ,3199 ,3329 ,3455 ,
	3585 ,3712 ,3841 ,3968 ,4096 ,4351 ,4609 ,4866 ,5121 ,5377 ,
	5630 ,5891 ,6141 ,6404 ,6658 ,6910 ,7170 ,7424 ,7682 ,7929 ,
	8192 ,8702 ,9218 ,9732 ,10230 ,10755 ,11275 ,11782 ,12300 ,12788 ,
	13315 ,13843 ,14315 ,14873 ,15364 ,15888 ,16384 ,17404 ,18477 ,19508 ,
	20460 ,21509 ,22550 ,23564 ,24528 ,25575 ,26546 ,27594 ,28728 ,29747 ,
	30615 ,31775 ,32768,   
};

static HI_VOID ps5230_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if((HI_NULL == pu32AgainLin) ||(HI_NULL == pu32AgainDb))
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

    if (*pu32AgainLin >= ad_gain_table[72])
    {
         *pu32AgainLin = ad_gain_table[72];
         *pu32AgainDb = 72;
         return ;
    }
    
    for (i = 1; i < 73; i++)
    {
        if (*pu32AgainLin < ad_gain_table[i])
        {
            *pu32AgainLin = ad_gain_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }
    
    return;
}

static HI_VOID ps5230_set_sns_bug()
{
    unsigned int sensor_nor_dac = 0;    
	unsigned int sensor_again =0;
    unsigned char bank1_e1,bank1_e2;
    HI_U16 data1,data2,data3,data4;
    SENSOR_I2C_WRITE(0xEF, 0x02);        // Bank2 
    SENSOR_I2C_READ(0x1B, &data1);
    SENSOR_I2C_READ(0x1C, &data2);
    sensor_nor_dac = ((data1&0x07)<<8)| (data2 & 0xFF);
    SENSOR_I2C_WRITE(0x0C, 0x30); //2016.11.11 
    SENSOR_I2C_WRITE(0x0D, 0x0F); //
    SENSOR_I2C_WRITE(0xEF, 0x01); 
    SENSOR_I2C_READ(0xE1, &data3);
    SENSOR_I2C_READ(0xE2, &data4);
    bank1_e1=data3;
    bank1_e2=data4;
    
	ISP_SATURATION_ATTR_S pstSatAttr;
	SOC_CHECK(HI_MPI_ISP_GetSaturationAttr(0,&pstSatAttr));

	sensor_again = (g_stSnsRegsInfo.astI2cData[2].u32Data<<8) | (g_stSnsRegsInfo.astI2cData[3].u32Data);

    if((sensor_nor_dac>28)||(sensor_again < 1638)||(pstSatAttr.enOpType==OP_TYPE_MANUAL && pstSatAttr.stManual.u8Saturation == 0)) //sensor gain > 2.5
    {
      
        if((bank1_e1&0x10)!=0x10)	
        {
			SENSOR_I2C_WRITE(0xE2, (bank1_e2&0xf0)|0x09);	  // VPUMP, VNG1,
        }
	    else	
	    {
			SENSOR_I2C_WRITE(0xE2, (bank1_e2&0xf0)|0x0d);	  // VPUMP, VNG5, 
	    }
		SENSOR_I2C_WRITE(0xE1, bank1_e1|0x10);	  // ON
    }
    else if((sensor_nor_dac<22)&&(sensor_again > 1638)&&(sensor_again < 2730)) // 1.5<sensor gain < 2.5
    {
       
        if((bank1_e2&0x05)==0x05)	
		{
		 	SENSOR_I2C_WRITE(0xE2, (bank1_e2&0xf0)|0x09);    // VPUMP, VNG1, 
		 	SENSOR_I2C_WRITE(0xE1, bank1_e1|0x10);    // ON 
		}
		else
		{
			SENSOR_I2C_WRITE(0xE2, bank1_e2&0xF7);    // VDDMA
		 	SENSOR_I2C_WRITE(0xE1, bank1_e1&0xEF);    // OFF			 
		}
    }   
    else if((sensor_nor_dac<22)&&(sensor_again > 2729)) //sensor gain < 1.5
    {
       
        if((bank1_e2&0x05)==0x05)	
		{
		 	SENSOR_I2C_WRITE(0xE2, (bank1_e2&0xf0)|0x08|0x01);    // VPUMP, VNG1, 
		 	SENSOR_I2C_WRITE(0xE1, bank1_e1|0x10);    // ON 
		}
		else
		{
			SENSOR_I2C_WRITE(0xE2, bank1_e2&0xF7);    // VDDMA
		 	SENSOR_I2C_WRITE(0xE1, bank1_e1&0xEF);    // OFF			 
		}
    }
    SENSOR_I2C_WRITE(0x09, 0x01);    // Bank1 update

    return;
}

static HI_VOID ps5230_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{  
    HI_U32 u32Tmp_Again = ad_gain_table[u32Again];

    u32Tmp_Again = (4096*1024)/u32Tmp_Again;
    
    g_stSnsRegsInfo.astI2cData[2].u32Data = ((u32Tmp_Again & 0xFF00) >> 8);
    g_stSnsRegsInfo.astI2cData[3].u32Data = (u32Tmp_Again & 0xFF);

	/*************************************************************
	   ps5230 has a bug. if the temperature is high,then the green will turn gray. 
		So it need to modify the sensor register every frame
	**************************************************************/
	   
	ps5230_set_sns_bug();

    return;
}

/* Only used in WDR_MODE_2To1_LINE and WDR_MODE_2To1_FRAME mode */
static HI_VOID ps5230_get_inttime_max(HI_U32 u32Ratio, HI_U32 *pu32IntTimeMax)
{
    if(HI_NULL == pu32IntTimeMax)
    {
        printf("null pointer when get ae sensor IntTimeMax value!\n");
        return;
    }

    return;
}

HI_S32 ps5230_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ps5230_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ps5230_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ps5230_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ps5230_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ps5230_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ps5230_again_calc_table;
    //pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
    pstExpFuncs->pfn_cmos_get_inttime_max   = ps5230_get_inttime_max;    

    return 0;
}

/* AWB default parameter and function */
#ifdef INIFILE_CONFIG_MODE

static HI_S32 ps5230_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
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
        0x0224,  0x80F8,  0x802C,
        0x8052,  0x01C2,  0x8070,       
        0x0024,  0x811C,  0x01F8    
    },  
    
    3633,    
    {       
        0x01F8,  0x80B5,  0x8043,       
        0x807D,  0x01E0,  0x8063,       
        0x0021,  0x8152,  0x0231    
    },
    
    2465,    
    {            
        0x01FB,  0x810C,  0x0011,        
        0x807D,  0x0163,  0x001A,       
        0x002B,  0x81FF,  0x02D4    
    }    
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,
	
    /*1,  2,  4,  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768*/
    /* saturation */   
    {0x7c,0x75,0x6a,0x60,0x58,0x50,0x48,0x40,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38}

};

static HI_S32 ps5230_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    pstAwbSnsDft->u16WbRefTemp = 5000;
    pstAwbSnsDft->au16GainOffset[0] = 0x23E;    
    pstAwbSnsDft->au16GainOffset[1] = 0x100;    
    pstAwbSnsDft->au16GainOffset[2] = 0x100;    
    pstAwbSnsDft->au16GainOffset[3] = 0x196;    
    pstAwbSnsDft->as32WbPara[0] = 55;    
    pstAwbSnsDft->as32WbPara[1] = 104;    
    pstAwbSnsDft->as32WbPara[2] = -97;    
    pstAwbSnsDft->as32WbPara[3] = 201958;    
    pstAwbSnsDft->as32WbPara[4] = 128;    
    pstAwbSnsDft->as32WbPara[5] = -152540;
    
    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));

    return 0;
}
#endif

HI_S32 ps5230_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ps5230_get_awb_default;

    return 0;
}


/* ISP default parameter and function */
#ifdef INIFILE_CONFIG_MODE

HI_U32 ps5230_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
            memcpy(&pstDef->stGe, &g_IspDft[0].stGe, sizeof(ISP_CMOS_GE_S));            
        break;
    }
    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_PS5230_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_PS5230_HEIGHT;

    return 0;
}

#else
#define DMNR_CALIB_CARVE_NUM_PS5230 9

float g_coef_calib_ps5230[DMNR_CALIB_CARVE_NUM_PS5230][4] = 
{
    {100.000000f, 2.000000f, 0.045427f, 7.218652f, }, 
    {200.000000f, 2.301030f, 0.045937f, 7.593602f, }, 
    {401.000000f, 2.603144f, 0.048095f, 8.075705f, }, 
    {800.000000f, 2.903090f, 0.050832f, 9.041181f, }, 
    {1622.000000f, 3.210051f, 0.057881f, 10.508446f, }, 
    {3317.000000f, 3.520746f, 0.067995f, 13.242065f, }, 
    {7466.000000f, 3.873088f, 0.087098f, 18.637381f, }, 
    {16821.000000f, 4.225852f, 0.117693f, 28.855566f, }, 
    {44916.000000f, 4.652401f, 0.204691f, 45.572727f, }, 
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
    60,/*u16VhLimit*/   
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

#if 0
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
#endif

static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
  //{100,200,400,800,1600,3200,6400,12800,25600,51200,102400,204800,409600,819200,1638400,3276800};
    {0,	  0,   0,  0,  0,   1,   1,    1,    1,    1,    1,     1,     1,     1,     1,       1},/* enPixSel = ~bEnLowLumaShoot */
    {30, 30, 32, 35,  35,  50,  60,  70,    120,  200,  250,   250,   250,   250,    250,    250},/*maxSharpAmt1 = SharpenUD*16 */
    {80, 80, 65, 50,  120,  160,  180,  200,    220,  250,  250,   250,   250,   250,    250,    250},/*maxEdgeAmt = SharpenD*16 */
    {0,  0,   0,  0,   0,  0,  20,   40,    90,    120,    180,     250,    250,     250,     250,       250},/*sharpThd2 = TextureNoiseThd*4 */
    {0,  0,   0,  0,   0,  0,  0,   0,    100,    200,    0,     0,    0,     0,     0,       0},/*edgeThd2 = EdgeNoiseThd*4 */
    {140, 140, 130, 100, 75,  45, 25,   10,  0,  0,   0,    0,   0,    0,    0,      0},/*overshootAmt*/
    {160, 160, 150, 130, 115,  100,90,  70,  30,  0,   0,    0,   0,    0,    0,     0},/*undershootAmt*/
};

static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
	/*��ֵ�˲��л���UVNR��ISO��ֵ*/
	/*UVNR�л�����ֵ�˲���ISO��ֵ*/
    /*0.0   -> disable��(0.0, 1.0]  -> weak��(1.0, 2.0]  -> normal��(2.0, 10.0) -> strong*/
	/*��˹�˲����ı�׼��*/
  //{100,	200,	400,	800,	1600,	3200,	6400,	12800,	25600,	51200,	102400,	204800,	409600,	819200,	1638400,	3276800};
	{1,	    2,      4,      5,      7,      48,     32,     16,     16,     16,      16,     16,     16,     16,     16,        16},      /*UVNRThreshold*/
 	{0,		0,		0,		0,		0,		0,		0,		0,		0,		1,			1,		2,		2,		2,		2,		2},  /*Coring_lutLimit*/
	{0,		0,		0,		16,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,		34,			34}  /*UVNR_blendRatio*/
};

static ISP_CMOS_DPC_S g_stCmosDpc = 
{
	//0,/*IR_channel*/
	//0,/*IR_position*/
	{70,100,170,244,248,250,252,252,252,252,252,252,252,252,252,252},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},/*au16BlendRatio[16]*/
};	

static ISP_CMOS_GE_S g_stIspGe =
{
	/*For GE*/
	0,    /*bEnable*/			
	7,    /*u8Slope*/	
	7,    /*u8Sensitivity*/
	4096, /*u16Threshold*/
	4096, /*u16SensiThreshold*/	
	{1024,1024,1024,2048,2048,2048,2048,  2048,  2048,2048,2048,2048,2048,2048,2048,2048}    /*au16Strength[ISP_AUTO_ISO_STENGTH_NUM]*/	
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

HI_U32 ps5230_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
     
    memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
    memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));			
	//memcpy(&pstDef->stGammafe, &g_stGammafeFSWDR, sizeof(ISP_CMOS_GAMMAFE_S));
	
    pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_PS5230;
    pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_ps5230;
    //memcpy(&pstDef->stNoiseTbl.stNrCommPara, &g_stNrCommPara,sizeof(ISP_NR_COMM_PARA_S));
    memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);

    memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
    memcpy(&pstDef->stDpc,        &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));
    //memcpy(&pstDef->stLsc,       &g_stCmosLscTable,       sizeof(ISP_LSC_CABLI_TABLE_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = 1920;
    pstDef->stSensorMaxResolution.u32MaxHeight = 1080;

    return 0;
}
#endif

HI_U32 ps5230_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
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
        pstBlackLevel->au16BlackLevel[i] = 0x0; 
    }
    return 0;  
    
}

HI_VOID ps5230_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U32 u32FullLines_5Fps = 0;
    //HI_U32 u32MaxIntTime_5Fps = 0;
    
  
    if ((WDR_MODE_NONE == genSensorMode) && (SENSOR_1080P_30FPS_MODE == gu8SensorImageMode))
    {
        u32FullLines_5Fps = (VMAX_1080P30_LINEAR * 30) / 5;
    }
    else
    {
        return;
    }
    
    u32FullLines_5Fps = (u32FullLines_5Fps > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines_5Fps;
    //u32MaxIntTime_5Fps = u32FullLines_5Fps - 2;

    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR_H, (u32FullLines_5Fps & 0xFF00) >> 8);  /* 5fps */
        SENSOR_I2C_WRITE(VMAX_ADDR_L, u32FullLines_5Fps & 0xFF);           /* 5fps */
        SENSOR_I2C_WRITE(EXPOSURE_ADDR_H, (PS5230_MIN_INT & 0xFF00) >> 8);      /* max exposure lines, int_time=VMAX-config.*/
        SENSOR_I2C_WRITE(EXPOSURE_ADDR_L, PS5230_MIN_INT& 0xFF);               /* max exposure lines */
        SENSOR_I2C_WRITE(AGC_ADDR_H, 0x0A);                                    /* min AG */
        SENSOR_I2C_WRITE(AGC_ADDR_L, 0xAB);                                    /* min AG */

    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR_H, (gu32FullLinesStd & 0xFF00) >> 8);
        SENSOR_I2C_WRITE(VMAX_ADDR_L, gu32FullLinesStd & 0xFF);

        bInit = HI_FALSE;
    }
    return;
}

HI_VOID ps5230_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            if (SENSOR_1080P_30FPS_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = VMAX_1080P30_LINEAR;
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

int  ps5230_set_inifile_path(const char *pcPath)
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

HI_U32 ps5230_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;
        
        g_stSnsRegsInfo.u32RegNum = 7;
        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        /* Shutter (Shutter Long) */
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = EXPOSURE_ADDR_H;
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = EXPOSURE_ADDR_L;

        /* AG */
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = AGC_ADDR_H;
        g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = AGC_ADDR_L;

        /* VMAX */
        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = VMAX_ADDR_H;
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = VMAX_ADDR_L;
        
        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = BANK1_UDPDATE;
        g_stSnsRegsInfo.astI2cData[6].u32Data= 1;

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
        g_stSnsRegsInfo.astI2cData[6].bUpdate = HI_TRUE;

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

static HI_S32 ps5230_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;
    
    bInit = HI_FALSE;    
        
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if ((pstSensorImageMode->u16Width <= 1920) && (pstSensorImageMode->u16Height <= 1080))
    {
        if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = SENSOR_1080P_30FPS_MODE;
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
HI_VOID ps5230_global_init()
{     
    gu8SensorImageMode = SENSOR_1080P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_1080P30_LINEAR;
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

static void ps5230_reg_init()
{  
	SENSOR_I2C_WRITE (0xEF,0x00); 
	SENSOR_I2C_WRITE (0x06,0x02); 
	SENSOR_I2C_WRITE (0x0B,0x00); 
	SENSOR_I2C_WRITE (0x0C,0xA0); 
	SENSOR_I2C_WRITE (0x10,0x01); 
	SENSOR_I2C_WRITE (0x12,0x80); 
	SENSOR_I2C_WRITE (0x13,0x00); 
	SENSOR_I2C_WRITE (0x14,0xFF); 
	SENSOR_I2C_WRITE (0x15,0x03); 
	SENSOR_I2C_WRITE (0x16,0xFF); 
	SENSOR_I2C_WRITE (0x17,0xFF); 
	SENSOR_I2C_WRITE (0x18,0xFF); 
	SENSOR_I2C_WRITE (0x19,0x64); 
	SENSOR_I2C_WRITE (0x1A,0x64); 
	SENSOR_I2C_WRITE (0x1B,0x64); 
	SENSOR_I2C_WRITE (0x1C,0x64); 
	SENSOR_I2C_WRITE (0x1D,0x64); 
	SENSOR_I2C_WRITE (0x1E,0x64); 
	SENSOR_I2C_WRITE (0x1F,0x64); 
	SENSOR_I2C_WRITE (0x20,0x64); 
	SENSOR_I2C_WRITE (0x21,0x00); 
	SENSOR_I2C_WRITE (0x22,0x00); 
	SENSOR_I2C_WRITE (0x23,0x00); 
	SENSOR_I2C_WRITE (0x24,0x00); 
	SENSOR_I2C_WRITE (0x25,0x00); 
	SENSOR_I2C_WRITE (0x26,0x00); 
	SENSOR_I2C_WRITE (0x27,0x00); 
	SENSOR_I2C_WRITE (0x28,0x00); 
	SENSOR_I2C_WRITE (0x29,0x64); 
	SENSOR_I2C_WRITE (0x2A,0x64); 
	SENSOR_I2C_WRITE (0x2B,0x64); 
	SENSOR_I2C_WRITE (0x2C,0x64); 
	SENSOR_I2C_WRITE (0x2D,0x64); 
	SENSOR_I2C_WRITE (0x2E,0x64); 
	SENSOR_I2C_WRITE (0x2F,0x64); 
	SENSOR_I2C_WRITE (0x30,0x64); 
	SENSOR_I2C_WRITE (0x31,0x0F); 
	SENSOR_I2C_WRITE (0x32,0x00); 
	SENSOR_I2C_WRITE (0x33,0x64); 
	SENSOR_I2C_WRITE (0x34,0x64); 
	SENSOR_I2C_WRITE (0x89,0x10); 
	SENSOR_I2C_WRITE (0x8B,0x00); 
	SENSOR_I2C_WRITE (0x8C,0x00); 
	SENSOR_I2C_WRITE (0x8D,0x00); 
	SENSOR_I2C_WRITE (0x8E,0x00); 
	SENSOR_I2C_WRITE (0x8F,0x00); 
	SENSOR_I2C_WRITE (0x90,0x02); 
	SENSOR_I2C_WRITE (0x91,0x00); 
	SENSOR_I2C_WRITE (0x92,0x11); 
	SENSOR_I2C_WRITE (0x93,0x10); 
	SENSOR_I2C_WRITE (0x94,0x00); 
	SENSOR_I2C_WRITE (0x95,0x00); 
	SENSOR_I2C_WRITE (0x96,0x00); 
	SENSOR_I2C_WRITE (0x97,0x00); 
	SENSOR_I2C_WRITE (0x99,0x00); 
	SENSOR_I2C_WRITE (0x9A,0x00); 
	SENSOR_I2C_WRITE (0x9B,0x09); 
	SENSOR_I2C_WRITE (0x9C,0x00); 
	SENSOR_I2C_WRITE (0x9D,0x00); 
	SENSOR_I2C_WRITE (0x9E,0x40); 
	SENSOR_I2C_WRITE (0x9F,0x00); 
	SENSOR_I2C_WRITE (0xA0,0x0A); 
	SENSOR_I2C_WRITE (0xA1,0x00); 
	SENSOR_I2C_WRITE (0xA2,0x1E); 
	SENSOR_I2C_WRITE (0xA3,0x07); 
	SENSOR_I2C_WRITE (0xA4,0xFF); 
	SENSOR_I2C_WRITE (0xA5,0x03); 
	SENSOR_I2C_WRITE (0xA6,0xFF); 
	SENSOR_I2C_WRITE (0xA7,0x00); 
	SENSOR_I2C_WRITE (0xA8,0x00); 
	SENSOR_I2C_WRITE (0xA9,0x11); 
	SENSOR_I2C_WRITE (0xAA,0x23); 
	SENSOR_I2C_WRITE (0xAB,0x23); 
	SENSOR_I2C_WRITE (0xAD,0x00); 
	SENSOR_I2C_WRITE (0xAE,0x00); 
	SENSOR_I2C_WRITE (0xAF,0x00); 
	SENSOR_I2C_WRITE (0xB0,0x00); 
	SENSOR_I2C_WRITE (0xB1,0x00); 
	SENSOR_I2C_WRITE (0xBE,0x15); 
	SENSOR_I2C_WRITE (0xBF,0x00); 
	SENSOR_I2C_WRITE (0xC0,0x10); 
	SENSOR_I2C_WRITE (0xC7,0x10); 
	SENSOR_I2C_WRITE (0xC8,0x01); 
	SENSOR_I2C_WRITE (0xC9,0x00); 
	SENSOR_I2C_WRITE (0xCA,0x55); 
	SENSOR_I2C_WRITE (0xCB,0x06); 
	SENSOR_I2C_WRITE (0xCC,0x09); 
	SENSOR_I2C_WRITE (0xCD,0x00); 
	SENSOR_I2C_WRITE (0xCE,0xA2); 
	SENSOR_I2C_WRITE (0xCF,0x00); 
	SENSOR_I2C_WRITE (0xD0,0x02); 
	SENSOR_I2C_WRITE (0xD1,0x10); 
	SENSOR_I2C_WRITE (0xD2,0x1E); 
	SENSOR_I2C_WRITE (0xD3,0x19); 
	SENSOR_I2C_WRITE (0xD4,0x04); 
	SENSOR_I2C_WRITE (0xD5,0x18); 
	SENSOR_I2C_WRITE (0xD6,0xC8); 
	SENSOR_I2C_WRITE (0xF0,0x00); 
	SENSOR_I2C_WRITE (0xF1,0x00); 
	SENSOR_I2C_WRITE (0xF2,0x00); 
	SENSOR_I2C_WRITE (0xF3,0x00); 
	SENSOR_I2C_WRITE (0xF4,0x00); 
	SENSOR_I2C_WRITE (0xF5,0x40); 
	SENSOR_I2C_WRITE (0xF6,0x00); 
	SENSOR_I2C_WRITE (0xF7,0x00); 
	SENSOR_I2C_WRITE (0xF8,0x00); 
	SENSOR_I2C_WRITE (0xED,0x01); 
	SENSOR_I2C_WRITE (0xEF,0x01); 
	SENSOR_I2C_WRITE (0x02,0xFF); 
	SENSOR_I2C_WRITE (0x03,0x01); 
	SENSOR_I2C_WRITE (0x04,0x45); 
	SENSOR_I2C_WRITE (0x05,0x03); 
	SENSOR_I2C_WRITE (0x06,0xFF); 
	SENSOR_I2C_WRITE (0x07,0x00); 
	SENSOR_I2C_WRITE (0x08,0x00); 
	SENSOR_I2C_WRITE (0x09,0x00); 
	SENSOR_I2C_WRITE (0x0A,0x04); 
	SENSOR_I2C_WRITE (0x0B,0x64); 
	SENSOR_I2C_WRITE (0x0C,0x00); 
	SENSOR_I2C_WRITE (0x0D,0x02); 
	SENSOR_I2C_WRITE (0x0E,0x00); 
	SENSOR_I2C_WRITE (0x0F,0x92); 
	SENSOR_I2C_WRITE (0x10,0x00); 
	SENSOR_I2C_WRITE (0x11,0x01); 
	SENSOR_I2C_WRITE (0x12,0x00); 
	SENSOR_I2C_WRITE (0x13,0x92); 
	SENSOR_I2C_WRITE (0x14,0x01); 
	SENSOR_I2C_WRITE (0x15,0x00); 
	SENSOR_I2C_WRITE (0x16,0x00); 
	SENSOR_I2C_WRITE (0x17,0x00); 
	SENSOR_I2C_WRITE (0x1A,0x00); 
	SENSOR_I2C_WRITE (0x1B,0x07); 
	SENSOR_I2C_WRITE (0x1C,0x90); 
	SENSOR_I2C_WRITE (0x1D,0x04); 
	SENSOR_I2C_WRITE (0x1E,0x4A); 
	SENSOR_I2C_WRITE (0x1F,0x00); 
	SENSOR_I2C_WRITE (0x20,0x00); 
	SENSOR_I2C_WRITE (0x21,0x00); 
	SENSOR_I2C_WRITE (0x22,0xD4); 
	SENSOR_I2C_WRITE (0x23,0x10); 
	SENSOR_I2C_WRITE (0x24,0xA0); 
	SENSOR_I2C_WRITE (0x25,0x00); 
	SENSOR_I2C_WRITE (0x26,0x08); 
	SENSOR_I2C_WRITE (0x27,0x08); 
	SENSOR_I2C_WRITE (0x28,0x98); 
	SENSOR_I2C_WRITE (0x29,0x0F); 
	SENSOR_I2C_WRITE (0x2A,0x08); 
	SENSOR_I2C_WRITE (0x2B,0x97); 
	SENSOR_I2C_WRITE (0x2C,0x10); 
	SENSOR_I2C_WRITE (0x2D,0x06); 
	SENSOR_I2C_WRITE (0x2E,0x3C); 
	SENSOR_I2C_WRITE (0x2F,0x10); 
	SENSOR_I2C_WRITE (0x30,0x05); 
	SENSOR_I2C_WRITE (0x31,0x3E); 
	SENSOR_I2C_WRITE (0x32,0x10); 
	SENSOR_I2C_WRITE (0x33,0x05); 
	SENSOR_I2C_WRITE (0x34,0x00); 
	SENSOR_I2C_WRITE (0x35,0x00); 
	SENSOR_I2C_WRITE (0x36,0x00); 
	SENSOR_I2C_WRITE (0x37,0xFE); 
	SENSOR_I2C_WRITE (0x38,0x55); 
	SENSOR_I2C_WRITE (0x39,0x00); 
	SENSOR_I2C_WRITE (0x3A,0x18); 
	SENSOR_I2C_WRITE (0x3B,0x16); 
	SENSOR_I2C_WRITE (0x3C,0x02); 
	SENSOR_I2C_WRITE (0x3D,0x02); 
	SENSOR_I2C_WRITE (0x3E,0x11); 
	SENSOR_I2C_WRITE (0x3F,0x26); 
	SENSOR_I2C_WRITE (0x40,0x04); 
	SENSOR_I2C_WRITE (0x41,0x15); 
	SENSOR_I2C_WRITE (0x42,0x40); 
	SENSOR_I2C_WRITE (0x43,0x14); 
	SENSOR_I2C_WRITE (0x44,0x05); 
	SENSOR_I2C_WRITE (0x45,0x15); 
	SENSOR_I2C_WRITE (0x46,0x14); 
	SENSOR_I2C_WRITE (0x47,0x05); 
	SENSOR_I2C_WRITE (0x48,0x1F); 
	SENSOR_I2C_WRITE (0x49,0x0A); 
	SENSOR_I2C_WRITE (0x4A,0x12); 
	SENSOR_I2C_WRITE (0x4B,0x08); 
	SENSOR_I2C_WRITE (0x4C,0x26); 
	SENSOR_I2C_WRITE (0x4D,0x1E); 
	SENSOR_I2C_WRITE (0x4E,0x01); 
	SENSOR_I2C_WRITE (0x4F,0x0E); 
	SENSOR_I2C_WRITE (0x50,0x04); 
	SENSOR_I2C_WRITE (0x51,0x05); 
	SENSOR_I2C_WRITE (0x52,0x23); 
	SENSOR_I2C_WRITE (0x53,0x04); 
	SENSOR_I2C_WRITE (0x54,0x00); 
	SENSOR_I2C_WRITE (0x55,0x00); 
	SENSOR_I2C_WRITE (0x56,0x78); 
	SENSOR_I2C_WRITE (0x57,0x00); 
	SENSOR_I2C_WRITE (0x58,0x92); 
	SENSOR_I2C_WRITE (0x59,0x00); 
	SENSOR_I2C_WRITE (0x5A,0xD6); 
	SENSOR_I2C_WRITE (0x5B,0x00); 
	SENSOR_I2C_WRITE (0x5C,0x2E); 
	SENSOR_I2C_WRITE (0x5D,0x01); 
	SENSOR_I2C_WRITE (0x5E,0x9E); 
	SENSOR_I2C_WRITE (0x5F,0x00); 
	SENSOR_I2C_WRITE (0x60,0x3C); 
	SENSOR_I2C_WRITE (0x61,0x08); 
	SENSOR_I2C_WRITE (0x62,0x4C); 
	SENSOR_I2C_WRITE (0x63,0x62); 
	SENSOR_I2C_WRITE (0x64,0x02); 
	SENSOR_I2C_WRITE (0x65,0x00); 
	SENSOR_I2C_WRITE (0x66,0x00); 
	SENSOR_I2C_WRITE (0x67,0x00); 
	SENSOR_I2C_WRITE (0x68,0x00); 
	SENSOR_I2C_WRITE (0x69,0x00); 
	SENSOR_I2C_WRITE (0x6A,0x00); 
	SENSOR_I2C_WRITE (0x6B,0x92); 
	SENSOR_I2C_WRITE (0x6C,0x00); 
	SENSOR_I2C_WRITE (0x6D,0x64); 
	SENSOR_I2C_WRITE (0x6E,0x01); 
	SENSOR_I2C_WRITE (0x6F,0xEE); 
	SENSOR_I2C_WRITE (0x70,0x00); 
	SENSOR_I2C_WRITE (0x71,0x64); 
	SENSOR_I2C_WRITE (0x72,0x08); 
	SENSOR_I2C_WRITE (0x73,0x98); 
	SENSOR_I2C_WRITE (0x74,0x00); 
	SENSOR_I2C_WRITE (0x75,0x00); 
	SENSOR_I2C_WRITE (0x76,0x00); 
	SENSOR_I2C_WRITE (0x77,0x00); 
	SENSOR_I2C_WRITE (0x78,0x0A); 
	SENSOR_I2C_WRITE (0x79,0xAB); 
	SENSOR_I2C_WRITE (0x7A,0x50); 
	SENSOR_I2C_WRITE (0x7B,0x40); 
	SENSOR_I2C_WRITE (0x7C,0x40); 
	SENSOR_I2C_WRITE (0x7D,0x00); 
	SENSOR_I2C_WRITE (0x7E,0x01); 
	SENSOR_I2C_WRITE (0x7F,0x00); 
	SENSOR_I2C_WRITE (0x80,0x00); 
	SENSOR_I2C_WRITE (0x87,0x00); 
	SENSOR_I2C_WRITE (0x88,0x08); 
	SENSOR_I2C_WRITE (0x89,0x01); 
	SENSOR_I2C_WRITE (0x8A,0x04); 
	SENSOR_I2C_WRITE (0x8B,0x4A); 
	SENSOR_I2C_WRITE (0x8C,0x00); 
	SENSOR_I2C_WRITE (0x8D,0x01); 
	SENSOR_I2C_WRITE (0x8E,0x00); 
	SENSOR_I2C_WRITE (0x90,0x00); 
	SENSOR_I2C_WRITE (0x91,0x01); 
	SENSOR_I2C_WRITE (0x92,0x80); 
	SENSOR_I2C_WRITE (0x93,0x00); 
	SENSOR_I2C_WRITE (0x94,0xFF); 
	SENSOR_I2C_WRITE (0x95,0x00); 
	SENSOR_I2C_WRITE (0x96,0x00); 
	SENSOR_I2C_WRITE (0x97,0x01); 
	SENSOR_I2C_WRITE (0x98,0x02); 
	SENSOR_I2C_WRITE (0x99,0x09); 
	SENSOR_I2C_WRITE (0x9A,0x60); 
	SENSOR_I2C_WRITE (0x9B,0x02); 
	SENSOR_I2C_WRITE (0x9C,0x60); 
	SENSOR_I2C_WRITE (0x9D,0x00); 
	SENSOR_I2C_WRITE (0x9E,0x00); 
	SENSOR_I2C_WRITE (0x9F,0x00); 
	SENSOR_I2C_WRITE (0xA0,0x00); 
	SENSOR_I2C_WRITE (0xA1,0x00); 
	SENSOR_I2C_WRITE (0xA2,0x00); 
	SENSOR_I2C_WRITE (0xA3,0x00); 
	SENSOR_I2C_WRITE (0xA4,0x14); 
	SENSOR_I2C_WRITE (0xA5,0x04); 
	SENSOR_I2C_WRITE (0xA6,0x38); 
	SENSOR_I2C_WRITE (0xA7,0x00); 
	SENSOR_I2C_WRITE (0xA8,0x08); 
	SENSOR_I2C_WRITE (0xA9,0x07); 
	SENSOR_I2C_WRITE (0xAA,0x80); 
	SENSOR_I2C_WRITE (0xAB,0x01); 
	SENSOR_I2C_WRITE (0xAD,0x00); 
	SENSOR_I2C_WRITE (0xAE,0x00); 
	SENSOR_I2C_WRITE (0xAF,0x00); 
	SENSOR_I2C_WRITE (0xB0,0x50); 
	SENSOR_I2C_WRITE (0xB1,0x00); 
	SENSOR_I2C_WRITE (0xB2,0x00); 
	SENSOR_I2C_WRITE (0xB3,0x00); 
	SENSOR_I2C_WRITE (0xB4,0x50); 
	SENSOR_I2C_WRITE (0xB5,0x07); 
	SENSOR_I2C_WRITE (0xB6,0x80); 
	SENSOR_I2C_WRITE (0xB7,0x82); 
	SENSOR_I2C_WRITE (0xB8,0x0A); 
	SENSOR_I2C_WRITE (0xCD,0x95); 
	SENSOR_I2C_WRITE (0xCE,0xF6); 
	SENSOR_I2C_WRITE (0xCF,0x01); 
	SENSOR_I2C_WRITE (0xD0,0x42); 
	SENSOR_I2C_WRITE (0xD1,0x30); 
	SENSOR_I2C_WRITE (0xD2,0x20); 
	SENSOR_I2C_WRITE (0xD3,0x1C); 
	SENSOR_I2C_WRITE (0xD4,0x00); 
	SENSOR_I2C_WRITE (0xD5,0x01); 
	SENSOR_I2C_WRITE (0xD6,0x00); 
	SENSOR_I2C_WRITE (0xD7,0x07); 
	SENSOR_I2C_WRITE (0xD8,0x84); 
	SENSOR_I2C_WRITE (0xD9,0x54); 
	SENSOR_I2C_WRITE (0xDA,0x70); 
	SENSOR_I2C_WRITE (0xDB,0x70); 
	SENSOR_I2C_WRITE (0xDC,0x10); 
	SENSOR_I2C_WRITE (0xDD,0x60); 
	SENSOR_I2C_WRITE (0xDE,0x50); 
	SENSOR_I2C_WRITE (0xDF,0x43); 
	SENSOR_I2C_WRITE (0xE0,0x70); 
	SENSOR_I2C_WRITE (0xE1,0x01); 
	SENSOR_I2C_WRITE (0xE2,0x35); 
	SENSOR_I2C_WRITE (0xE3,0x21); 
	SENSOR_I2C_WRITE (0xE4,0x66); 
	SENSOR_I2C_WRITE (0xE6,0x00); 
	SENSOR_I2C_WRITE (0xE7,0x00); 
	SENSOR_I2C_WRITE (0xF5,0x02); 
	SENSOR_I2C_WRITE (0xF6,0xA8); 
	SENSOR_I2C_WRITE (0xF7,0x03); 
	SENSOR_I2C_WRITE (0xF0,0x00); 
	SENSOR_I2C_WRITE (0xF4,0x02); 
	SENSOR_I2C_WRITE (0xF2,0x1F); 
	SENSOR_I2C_WRITE (0xF1,0x16); 
	SENSOR_I2C_WRITE (0xF5,0x12); 
	SENSOR_I2C_WRITE (0x09,0x01); 
	SENSOR_I2C_WRITE (0xEF,0x02); 
	SENSOR_I2C_WRITE (0x00,0x4A);//0X4A 
	SENSOR_I2C_WRITE (0x01,0xA0); 
	SENSOR_I2C_WRITE (0x02,0x03); 
	SENSOR_I2C_WRITE (0x03,0x00); 
	SENSOR_I2C_WRITE (0x04,0x00); 
	SENSOR_I2C_WRITE (0x05,0x30); 
	SENSOR_I2C_WRITE (0x06,0x04); 
	SENSOR_I2C_WRITE (0x07,0x01); 
	SENSOR_I2C_WRITE (0x08,0x03); 
	SENSOR_I2C_WRITE (0x09,0x00); 
	SENSOR_I2C_WRITE (0x0A,0x30); 
	SENSOR_I2C_WRITE (0x0B,0x00); 
	SENSOR_I2C_WRITE (0x0C,0x30); //0X00 
	SENSOR_I2C_WRITE (0x0D,0x0F); //0X08
	SENSOR_I2C_WRITE (0x0E,0x30); //0X20
	SENSOR_I2C_WRITE (0x0F,0x1B); 
	SENSOR_I2C_WRITE (0x10,0x00); 
	SENSOR_I2C_WRITE (0x11,0x00); 
	SENSOR_I2C_WRITE (0x12,0x00); 
	SENSOR_I2C_WRITE (0x13,0x00); 
	SENSOR_I2C_WRITE (0x14,0x40); 
	SENSOR_I2C_WRITE (0x15,0x00); 
	SENSOR_I2C_WRITE (0x16,0x00); 
	SENSOR_I2C_WRITE (0x17,0x00); 
	SENSOR_I2C_WRITE (0x18,0x06); 
	SENSOR_I2C_WRITE (0x19,0x01); 
	SENSOR_I2C_WRITE (0x1A,0x00); 
	SENSOR_I2C_WRITE (0x30,0xFF); 
	SENSOR_I2C_WRITE (0x31,0x06); 
	SENSOR_I2C_WRITE (0x32,0x07); 
	SENSOR_I2C_WRITE (0x33,0x83);//0X85 
	SENSOR_I2C_WRITE (0x34,0x00); 
	SENSOR_I2C_WRITE (0x35,0x00); 
	SENSOR_I2C_WRITE (0x36,0x00); 
	SENSOR_I2C_WRITE (0x37,0x01); 
	SENSOR_I2C_WRITE (0x38,0x00); 
	SENSOR_I2C_WRITE (0x39,0x00); 
	SENSOR_I2C_WRITE (0x3A,0xCE); 
	SENSOR_I2C_WRITE (0x3B,0x17); 
	SENSOR_I2C_WRITE (0x3C,0x64); 
	SENSOR_I2C_WRITE (0x3D,0x04); 
	SENSOR_I2C_WRITE (0x3E,0x00); 
	SENSOR_I2C_WRITE (0x3F,0x0A); 
	SENSOR_I2C_WRITE (0x40,0x04); 
	SENSOR_I2C_WRITE (0x41,0x05); 
	SENSOR_I2C_WRITE (0x42,0x04); 
	SENSOR_I2C_WRITE (0x43,0x05); 
	SENSOR_I2C_WRITE (0x45,0x00); 
	SENSOR_I2C_WRITE (0x46,0x00); 
	SENSOR_I2C_WRITE (0x47,0x00); 
	SENSOR_I2C_WRITE (0x48,0x00); 
	SENSOR_I2C_WRITE (0x49,0x00); 
	SENSOR_I2C_WRITE (0x4A,0x00); 
	SENSOR_I2C_WRITE (0x4B,0x00); 
	SENSOR_I2C_WRITE (0x4C,0x00); 
	SENSOR_I2C_WRITE (0x4D,0x00); 
	SENSOR_I2C_WRITE (0x88,0x07); 
	SENSOR_I2C_WRITE (0x89,0x22); 
	SENSOR_I2C_WRITE (0x8A,0x00); 
	SENSOR_I2C_WRITE (0x8B,0x14); 
	SENSOR_I2C_WRITE (0x8C,0x00); 
	SENSOR_I2C_WRITE (0x8D,0x00); 
	SENSOR_I2C_WRITE (0x8E,0x52); 
	SENSOR_I2C_WRITE (0x8F,0x60); 
	SENSOR_I2C_WRITE (0x90,0x20); 
	SENSOR_I2C_WRITE (0x91,0x00); 
	SENSOR_I2C_WRITE (0x92,0x01); 
	SENSOR_I2C_WRITE (0x9E,0x00); 
	SENSOR_I2C_WRITE (0x9F,0x40); 
	SENSOR_I2C_WRITE (0xA0,0x30); 
	SENSOR_I2C_WRITE (0xA1,0x00); 
	SENSOR_I2C_WRITE (0xA2,0x00); 
	SENSOR_I2C_WRITE (0xA3,0x03); 
	SENSOR_I2C_WRITE (0xA4,0xC0); 
	SENSOR_I2C_WRITE (0xA5,0x00); 
	SENSOR_I2C_WRITE (0xA6,0x00); 
	SENSOR_I2C_WRITE (0xA7,0x00); 
	SENSOR_I2C_WRITE (0xA8,0x00); 
	SENSOR_I2C_WRITE (0xA9,0x00); 
	SENSOR_I2C_WRITE (0xAA,0x00); 
	SENSOR_I2C_WRITE (0xAB,0x00); 
	SENSOR_I2C_WRITE (0xAC,0x00); 
	SENSOR_I2C_WRITE (0xB7,0x00); 
	SENSOR_I2C_WRITE (0xB8,0x00); 
	SENSOR_I2C_WRITE (0xB9,0x00); 
	SENSOR_I2C_WRITE (0xBA,0x00); 
	SENSOR_I2C_WRITE (0xBB,0x00); 
	SENSOR_I2C_WRITE (0xBC,0x00); 
	SENSOR_I2C_WRITE (0xBD,0x00); 
	SENSOR_I2C_WRITE (0xBE,0x00); 
	SENSOR_I2C_WRITE (0xBF,0x00); 
	SENSOR_I2C_WRITE (0xC0,0x00); 
	SENSOR_I2C_WRITE (0xC1,0x00); 
	SENSOR_I2C_WRITE (0xC2,0x00); 
	SENSOR_I2C_WRITE (0xC3,0x00); 
	SENSOR_I2C_WRITE (0xC4,0x00); 
	SENSOR_I2C_WRITE (0xC5,0x00); 
	SENSOR_I2C_WRITE (0xC6,0x00); 
	SENSOR_I2C_WRITE (0xC7,0x00); 
	SENSOR_I2C_WRITE (0xC8,0x00); 
	SENSOR_I2C_WRITE (0xC9,0x00); 
	SENSOR_I2C_WRITE (0xCA,0x00); 
	SENSOR_I2C_WRITE (0xED,0x01); 
	SENSOR_I2C_WRITE (0xEF,0x05); 
	SENSOR_I2C_WRITE (0x03,0x10); 
	SENSOR_I2C_WRITE (0x04,0xE0); 
	SENSOR_I2C_WRITE (0x05,0x01); 
	SENSOR_I2C_WRITE (0x06,0x00); 
	SENSOR_I2C_WRITE (0x07,0x80); 
	SENSOR_I2C_WRITE (0x08,0x02); 
	SENSOR_I2C_WRITE (0x09,0x09); 
	SENSOR_I2C_WRITE (0x0A,0x04); 
	SENSOR_I2C_WRITE (0x0B,0x06); 
	SENSOR_I2C_WRITE (0x0C,0x0C); 
	SENSOR_I2C_WRITE (0x0D,0xA1); 
	SENSOR_I2C_WRITE (0x0E,0x00); 
	SENSOR_I2C_WRITE (0x0F,0x00); 
	SENSOR_I2C_WRITE (0x10,0x01); 
	SENSOR_I2C_WRITE (0x11,0x00); 
	SENSOR_I2C_WRITE (0x12,0x00); 
	SENSOR_I2C_WRITE (0x13,0x00); 
	SENSOR_I2C_WRITE (0x14,0xB8); 
	SENSOR_I2C_WRITE (0x15,0x07); 
	SENSOR_I2C_WRITE (0x16,0x06); 
	SENSOR_I2C_WRITE (0x17,0x06); 
	SENSOR_I2C_WRITE (0x18,0x03); 
	SENSOR_I2C_WRITE (0x19,0x04); 
	SENSOR_I2C_WRITE (0x1A,0x06); 
	SENSOR_I2C_WRITE (0x1B,0x06); 
	SENSOR_I2C_WRITE (0x1C,0x07); 
	SENSOR_I2C_WRITE (0x1D,0x08); 
	SENSOR_I2C_WRITE (0x1E,0x1A); 
	SENSOR_I2C_WRITE (0x1F,0x00); 
	SENSOR_I2C_WRITE (0x20,0x00); 
	SENSOR_I2C_WRITE (0x21,0x1E); 
	SENSOR_I2C_WRITE (0x22,0x1E); 
	SENSOR_I2C_WRITE (0x23,0x01); 
	SENSOR_I2C_WRITE (0x24,0x04); 
	SENSOR_I2C_WRITE (0x27,0x00); 
	SENSOR_I2C_WRITE (0x28,0x00); 
	SENSOR_I2C_WRITE (0x2A,0x08); 
	SENSOR_I2C_WRITE (0x2B,0x02); 
	SENSOR_I2C_WRITE (0x2C,0xA4); 
	SENSOR_I2C_WRITE (0x2D,0x06); 
	SENSOR_I2C_WRITE (0x2E,0x00); 
	SENSOR_I2C_WRITE (0x2F,0x05); 
	SENSOR_I2C_WRITE (0x30,0xE0); 
	SENSOR_I2C_WRITE (0x31,0x01); 
	SENSOR_I2C_WRITE (0x32,0x00); 
	SENSOR_I2C_WRITE (0x33,0x00); 
	SENSOR_I2C_WRITE (0x34,0x00); 
	SENSOR_I2C_WRITE (0x35,0x00); 
	SENSOR_I2C_WRITE (0x36,0x00); 
	SENSOR_I2C_WRITE (0x37,0x00); 
	SENSOR_I2C_WRITE (0x38,0x0E); 
	SENSOR_I2C_WRITE (0x39,0x01); 
	SENSOR_I2C_WRITE (0x3A,0x02); 
	SENSOR_I2C_WRITE (0x3B,0x01); 
	SENSOR_I2C_WRITE (0x3C,0x00); 
	SENSOR_I2C_WRITE (0x3D,0x00); 
	SENSOR_I2C_WRITE (0x3E,0x00); 
	SENSOR_I2C_WRITE (0x3F,0x00); 
	SENSOR_I2C_WRITE (0x40,0x16); 
	SENSOR_I2C_WRITE (0x41,0x26); 
	SENSOR_I2C_WRITE (0x42,0x00); 
	SENSOR_I2C_WRITE (0x47,0x05); 
	SENSOR_I2C_WRITE (0x48,0x07); 
	SENSOR_I2C_WRITE (0x49,0x01); 
	SENSOR_I2C_WRITE (0x4D,0x02); 
	SENSOR_I2C_WRITE (0x4F,0x00); 
	SENSOR_I2C_WRITE (0x54,0x05); 
	SENSOR_I2C_WRITE (0x55,0x01); 
	SENSOR_I2C_WRITE (0x56,0x05); 
	SENSOR_I2C_WRITE (0x57,0x01); 
	SENSOR_I2C_WRITE (0x58,0x02); 
	SENSOR_I2C_WRITE (0x59,0x01); 
	SENSOR_I2C_WRITE (0x5B,0x00); 
	SENSOR_I2C_WRITE (0x5C,0x03); 
	SENSOR_I2C_WRITE (0x5D,0x00); 
	SENSOR_I2C_WRITE (0x5E,0x07); 
	SENSOR_I2C_WRITE (0x5F,0x08); 
	SENSOR_I2C_WRITE (0x60,0x00); 
	SENSOR_I2C_WRITE (0x61,0x00); 
	SENSOR_I2C_WRITE (0x62,0x00); 
	SENSOR_I2C_WRITE (0x63,0x28); 
	SENSOR_I2C_WRITE (0x64,0x30); 
	SENSOR_I2C_WRITE (0x65,0x9E); 
	SENSOR_I2C_WRITE (0x66,0xB9); 
	SENSOR_I2C_WRITE (0x67,0x52); 
	SENSOR_I2C_WRITE (0x68,0x70); 
	SENSOR_I2C_WRITE (0x69,0x4E); 
	SENSOR_I2C_WRITE (0x70,0x00); 
	SENSOR_I2C_WRITE (0x71,0x00); 
	SENSOR_I2C_WRITE (0x72,0x00); 
	SENSOR_I2C_WRITE (0x90,0x04); 
	SENSOR_I2C_WRITE (0x91,0x01); 
	SENSOR_I2C_WRITE (0x92,0x00); 
	SENSOR_I2C_WRITE (0x93,0x00); 
	SENSOR_I2C_WRITE (0x94,0x03); 
	SENSOR_I2C_WRITE (0x96,0x00); 
	SENSOR_I2C_WRITE (0x97,0x01); 
	SENSOR_I2C_WRITE (0x98,0x01); 
	SENSOR_I2C_WRITE (0xA0,0x00); 
	SENSOR_I2C_WRITE (0xA1,0x01); 
	SENSOR_I2C_WRITE (0xA2,0x00); 
	SENSOR_I2C_WRITE (0xA3,0x01); 
	SENSOR_I2C_WRITE (0xA4,0x00); 
	SENSOR_I2C_WRITE (0xA5,0x01); 
	SENSOR_I2C_WRITE (0xA6,0x00); 
	SENSOR_I2C_WRITE (0xA7,0x00); 
	SENSOR_I2C_WRITE (0xAA,0x00); 
	SENSOR_I2C_WRITE (0xAB,0x0F); 
	SENSOR_I2C_WRITE (0xAC,0x08); 
	SENSOR_I2C_WRITE (0xAD,0x09); 
	SENSOR_I2C_WRITE (0xAE,0x0A); 
	SENSOR_I2C_WRITE (0xAF,0x0B); 
	SENSOR_I2C_WRITE (0xB0,0x00); 
	SENSOR_I2C_WRITE (0xB1,0x00); 
	SENSOR_I2C_WRITE (0xB2,0x01); 
	SENSOR_I2C_WRITE (0xB3,0x00); 
	SENSOR_I2C_WRITE (0xB4,0x00); 
	SENSOR_I2C_WRITE (0xB5,0x0A); 
	SENSOR_I2C_WRITE (0xB6,0x0A); 
	SENSOR_I2C_WRITE (0xB7,0x0A); 
	SENSOR_I2C_WRITE (0xB8,0x0A); 
	SENSOR_I2C_WRITE (0xB9,0x00); 
	SENSOR_I2C_WRITE (0xBA,0x00); 
	SENSOR_I2C_WRITE (0xBB,0x00); 
	SENSOR_I2C_WRITE (0xBC,0x00); 
	SENSOR_I2C_WRITE (0xBD,0x00); 
	SENSOR_I2C_WRITE (0xBE,0x00); 
	SENSOR_I2C_WRITE (0xBF,0x00); 
	SENSOR_I2C_WRITE (0xC0,0x00); 
	SENSOR_I2C_WRITE (0xC1,0x00); 
	SENSOR_I2C_WRITE (0xC2,0x00); 
	SENSOR_I2C_WRITE (0xC3,0x00); 
	SENSOR_I2C_WRITE (0xC4,0x00); 
	SENSOR_I2C_WRITE (0xC5,0x00); 
	SENSOR_I2C_WRITE (0xC6,0x00); 
	SENSOR_I2C_WRITE (0xC7,0x00); 
	SENSOR_I2C_WRITE (0xC8,0x00); 
	SENSOR_I2C_WRITE (0xD3,0x80); 
	SENSOR_I2C_WRITE (0xD4,0x00); 
	SENSOR_I2C_WRITE (0xD5,0x00); 
	SENSOR_I2C_WRITE (0xD6,0x03); 
	SENSOR_I2C_WRITE (0xD7,0x77); 
	SENSOR_I2C_WRITE (0xD8,0x00); 
	SENSOR_I2C_WRITE (0xED,0x01); 
	SENSOR_I2C_WRITE (0xEF,0x00); 
	SENSOR_I2C_WRITE (0x11,0x00);  
	SENSOR_I2C_WRITE (0xEF,0x01); 
	SENSOR_I2C_WRITE (0x02,0xFB); 
	SENSOR_I2C_WRITE (0x05,0x31); 
	SENSOR_I2C_WRITE (0x09,0x01);     

    bSensorInit = HI_TRUE;
    printf("======================================================================\n");
    printf("======PrimeSensor_PS5230 1080P25fps(Parallel port) init success!======\n");
    printf("======================================================================\n");

    return;
}


HI_S32 ps5230_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ps5230_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ps5230_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = ps5230_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ps5230_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = ps5230_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ps5230_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ps5230_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = ps5230_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
 
int ps5230_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ps5230_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, PS5230_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    ps5230_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, PS5230_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    ps5230_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, PS5230_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int ps5230_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, PS5230_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, PS5230_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, PS5230_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int PrimeSensor_PS5230_init(SENSOR_DO_I2CRD do_i2c_read, 
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	ps5230_register_callback();
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
	printf("PrimeSensor_PS5230 1080P30fps init success!\n");
	return s32Ret;
}

int PS5230_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_PS5230_WIDTH;
        *ret_height = SENSOR_PS5230_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_PS5230_probe() {
    uint16_t ret_data1 = 0;
    uint16_t ret_data2 = 0;

    SENSOR_I2C_READ(0x0, &ret_data1);
    SENSOR_I2C_READ(0x1, &ret_data2);
	
    if (ret_data1 == PS5230_CHECK_DATA_LSB && ret_data2 == PS5230_CHECK_DATA_MSB) {
		//set i2c pinmux
		sdk_sys->write_reg(0x200f0040, 0x2);
		sdk_sys->write_reg(0x200f0044, 0x2);
		
        sdk_sys->write_reg(0x200f0080, 0x1);
        sdk_sys->write_reg(0x200f0088, 0x1);
        sdk_sys->write_reg(0x200f008c, 0x2);
        sdk_sys->write_reg(0x200f0090, 0x2);
        sdk_sys->write_reg(0x200f0094, 0x1);
#if defined(HI3518E_V2)
        sdk_sys->write_reg(0x2003002c, 0xb4001);	 // sensor unreset, clk 27MHz, VI 99MHz
#elif defined(HI3516C_V2)
        sdk_sys->write_reg(0x2003002c, 0xb4005);	 // sensor unreset, clk 27MHz, VI 148.5MHz
#endif

        return true;
    }
    return false;
}
int PS5230_get_sensor_name(char *sensor_name)
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

