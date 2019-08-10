

#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"


static  SENSOR_SOIH42_DO_I2CRD sensor_i2c_read = NULL;
static  SENSOR_SOIH42_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)


#if !defined(__SOIH42_CMOS_H_)
#define __SOIH42_CMOS_H_


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_comm_isp.h"
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

#define SOIH42_ID 42

#define CMOS_SOIH42_ISP_WRITE_SENSOR_ENABLE (1)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/


 static const unsigned int sensor_i2c_addr = 0x60;
 static unsigned int sensor_addr_byte = 1;
 static unsigned int sensor_data_byte = 1;
 

//static HI_U32 gu32FullLinesStd = 767;
//static HI_U32 gu32FullLines = 767;


static HI_U32 gu32FullLinesStd = 750;
static HI_U32 gu32FullLines = 750;

static HI_U8 gu8SensorMode = 0;



 
#if CMOS_SOIH42_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif


static AWB_CCM_S g_stAwbCcm =
{
    5048,
    {   0x1f5, 0x80be, 0x8037,
        0x8045, 0x195, 0x8050,
        0x800e, 0x8146, 0x255
    },
    3193,
    {   0x1ee, 0x8017, 0x80d6,
        0x8053, 0x1d2, 0x807e,
        0x8023, 0x8176, 0x29a
    },
    2480,
    {   0x21d, 0x8091, 0x808c,
        0x8072, 0x19d, 0x802b,
        0x80b3, 0x8366, 0x519
    }

};


static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */
    {0x80,0x80,0x80,0x80,0x68,0x48,0x35,0x30}

};


static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
	/* bvalid */
	   1,
	   
	   /* sharpen_alt_d */
	//   {0xbc,0x6d,0x55,0x60,0x16,0x8e,0xd8,0x01},
	   {155,110,45,30,20,15,15,15},
		   
	   /* sharpen_alt_ud */
    // {0x8e,0x46,0x3e,0x40,0x2f,0x3d,0x1e,0x20},
	   {132,110,65,40,30,15,15,15},
		   
	   /* snr_thresh */
	 //  {0x1d,0x1e,0x1f,0x30,0x30,0x30,0x30,0x30},
	   {8,10,25,35,45,55,65,75},
		   
	   /* demosaic_lum_thresh */
	   {0x40,0x60,0x80,0x80,0x80,0x80,0x80,0x80},
		   
	   /* demosaic_np_offset */
	   {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},
		   
	   /* ge_strength */
	   {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37}


};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut */
    {
     0x19,0x20,0x24,0x27,0x29,0x2b,0x2d,0x2e,0x2f,0x31,0x32,0x33,0x34,0x34,0x35,0x36,0x37,\
    0x37,0x38,0x38,0x39,0x3a,0x3a,0x3b,0x3b,0x3b,0x3c,0x3c,0x3d,0x3d,0x3d,0x3e,0x3e,0x3e,\
    0x3f,0x3f,0x3f,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x42,0x42,0x42,0x42,0x43,0x43,0x43,\
    0x43,0x44,0x44,0x44,0x44,0x44,0x45,0x45,0x45,0x45,0x45,0x46,0x46,0x46,0x46,0x46,0x46,\
    0x47,0x47,0x47,0x47,0x47,0x47,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,\
    0x49,0x49,0x49,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4b,0x4b,0x4b,0x4b,0x4b,\
    0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4d,0x4d,0x4d,0x4d,\
    0x4d,0x4d,0x4d,0x4d,0x4d,0x4d,0x4e,0x4e,0x4e
    },

    /* demosaic_weight_lut */
    {
        0x19,0x20,0x24,0x27,0x29,0x2b,0x2d,0x2e,0x2f,0x31,0x32,0x33,0x34,0x34,0x35,0x36,0x37,\
    0x37,0x38,0x38,0x39,0x3a,0x3a,0x3b,0x3b,0x3b,0x3c,0x3c,0x3d,0x3d,0x3d,0x3e,0x3e,0x3e,\
    0x3f,0x3f,0x3f,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x42,0x42,0x42,0x42,0x43,0x43,0x43,\
    0x43,0x44,0x44,0x44,0x44,0x44,0x45,0x45,0x45,0x45,0x45,0x46,0x46,0x46,0x46,0x46,0x46,\
    0x47,0x47,0x47,0x47,0x47,0x47,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,\
    0x49,0x49,0x49,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4b,0x4b,0x4b,0x4b,0x4b,\
    0x4b,0x4b,0x4b,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4c,0x4d,0x4d,0x4d,0x4d,\
    0x4d,0x4d,0x4d,0x4d,0x4d,0x4d,0x4e,0x4e,0x4e
    }
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
	/* bvalid */
	   1,
	   
	   /*vh_slope*/
	   0xff,
	
	   /*aa_slope*/
	   0xe4,
	
	   /*va_slope*/
	   0xec,
	
	   /*uu_slope*/
	   0x9f,
	
	   /*sat_slope*/
	   0x5d,
	
	   /*ac_slope*/
	   0xcf,
	
	   /*vh_thresh*/
	   0x138,
	
	   /*aa_thresh*/
	   0xba,
	
	   /*va_thresh*/
	   0xda,
	
	   /*uu_thresh*/
	   0x148,
	
	   /*sat_thresh*/
	   0x171,
	
	   /*ac_thresh*/
	   0x1b3

};



HI_U32 soih42_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
    
    pstDef->stComm.u8Rggb           = 0x3;      //3: bggr  
    pstDef->stComm.u8BalanceFe      = 0x1;

    pstDef->stDenoise.u8SinterThresh= 0x15;
    pstDef->stDenoise.u8NoiseProfile= 0x0;      //0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
    pstDef->stDenoise.u16Nr0        = 0x0;
    pstDef->stDenoise.u16Nr1        = 0x0;

    pstDef->stDrc.u8DrcBlack        = 0x0;
    pstDef->stDrc.u8DrcVs           = 0x04;     // variance space
    pstDef->stDrc.u8DrcVi           = 0x08;     // variance intensity
    pstDef->stDrc.u8DrcSm           = 0xa0;     // slope max
    pstDef->stDrc.u16DrcWl          = 0x4ff;    // white level

    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));

    return 0;
}



HI_U32 soih42_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

 /*   pstBlackLevel->au16BlackLevel[0] = 1;
    pstBlackLevel->au16BlackLevel[1] = 1;
    pstBlackLevel->au16BlackLevel[2] = 1;
    pstBlackLevel->au16BlackLevel[3] = 1;
*/
    pstBlackLevel->au16BlackLevel[0] = 0x19;
    pstBlackLevel->au16BlackLevel[1] = 0;
    pstBlackLevel->au16BlackLevel[2] = 0;
    pstBlackLevel->au16BlackLevel[3] = 0xF;

    return 0;    

}




HI_VOID soih42_set_pixel_detect(HI_BOOL bEnable)
{
    HI_S32 s32FullLines;
    
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(0x00, 0x00);//again=1;
        SENSOR_I2C_WRITE(0x0d,0x01);//dgain=1;
        
        s32FullLines =((30*767)/5);
        SENSOR_I2C_WRITE(0x22, (s32FullLines & 0xff));
        SENSOR_I2C_WRITE(0x23, (s32FullLines >> 8));
    }
    else /* setup for ISP 'normal mode' */
    {
        s32FullLines = 767;//normal full lines is 767 lines ,30 fps
        SENSOR_I2C_WRITE(0x22, (s32FullLines & 0xff));
        SENSOR_I2C_WRITE(0x23, (s32FullLines >> 8));
    }

    return;

}


HI_VOID soih42_set_wdr_mode(HI_U8 u8Mode)
{
    switch(u8Mode)
    {
        //sensor mode 0
        case 0:
            gu8SensorMode = 0;
        break;
        //sensor mode 1
        case 1:
            gu8SensorMode = 1;
        break;

        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    
    return;
}

static HI_S32 soih42_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = 750;
    
    pstAeSnsDft->u32LinesPer500ms = 750*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;
    /* 1280 * 736 */
    /* 1296 * 816 */ /* reg: 0x22/23/24/25 */    
    //gu8Fps = 30;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
  //  pstAeSnsDft->u32MaxIntTime = 767;
  
  pstAeSnsDft->u32MaxIntTime = 750;
    pstAeSnsDft->u32MinIntTime = 8;
    
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x40;
    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    /* 1(1+1/16), 1(1+2/16), ... , 2(1+1/16), ... , 16(1+15/16) */
    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_DB;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 6;
    pstAeSnsDft->u32MaxAgain = 3;  /* Max : 18DB*/
    pstAeSnsDft->u32MinAgain = 0;
    pstAeSnsDft->u32MaxAgainTarget = 3;
    pstAeSnsDft->u32MinAgainTarget = 0;
    

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0625;
    pstAeSnsDft->u32MaxDgain = 31;  /* 1 ~ 31/16, unit is 1/16 */
    pstAeSnsDft->u32MinDgain = 16;
    pstAeSnsDft->u32MaxDgainTarget = 31;
    pstAeSnsDft->u32MinDgainTarget = 16; 

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;

}

static HI_S32 soih42_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
    if (HI_NULL == pstSensorMaxResolution)
    {
        printf("null pointer when get sensor max resolution \n");
        return -1;
    }

    memset(pstSensorMaxResolution, 0, sizeof(ISP_CMOS_SENSOR_MAX_RESOLUTION));

    pstSensorMaxResolution->u32MaxWidth  = 1280;
    pstSensorMaxResolution->u32MaxHeight = 720;

    return 0;
}


/* the function of sensor set fps */
static HI_VOID soih42_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch(u8Fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 750;
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd;
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 30 / 2;
            //SENSOR_I2C_WRITE(0x22, gu32FullLinesStd & 0xff);
            //SENSOR_I2C_WRITE(0x23, (gu32FullLinesStd >> 8));
        break;
        
        case 25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = (30 * 750) / 25;
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd;
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 25 / 2;
            //SENSOR_I2C_WRITE(0x22, gu32FullLinesStd & 0xff);
            //SENSOR_I2C_WRITE(0x23, (gu32FullLinesStd >> 8));
        break;
        
        default:
        break;
    }

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;

}

static HI_VOID soih42_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    SENSOR_I2C_WRITE(0x22, u16FullLines&0xff);
    SENSOR_I2C_WRITE(0x23, (u16FullLines >> 8));

    pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;
    gu32FullLines = u16FullLines;

    return;

}

static HI_VOID soih42_init_regs_info(HI_VOID)
{
#if CMOS_SOIH42_ISP_WRITE_SENSOR_ENABLE
		HI_S32 i;
		static HI_BOOL bInit = HI_FALSE;
	
		if (HI_FALSE == bInit)
		{
			g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
			g_stSnsRegsInfo.u32RegNum = 3;
			for (i=0; i<3; i++)
			{
				g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
				g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
				g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
			}
			g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;
			g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x01;
			g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
			g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x02;
			g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x00;
			//g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_TRUE;
			//g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x0d;
			g_stSnsRegsInfo.bDelayCfgIspDgain = HI_TRUE;
	
			bInit = HI_TRUE;
		}
#endif
		return;

}




/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID soih42_inttime_update(HI_U32 u32IntTime)
{
#if CMOS_SOIH42_ISP_WRITE_SENSOR_ENABLE
		soih42_init_regs_info();
		g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime & 0xFF;
		g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime >> 8) & 0xFF;
#else 
		//refresh the sensor setting every frame to avoid defect pixel error
		SENSOR_I2C_WRITE(0x01, u32IntTime&0xFF);
		SENSOR_I2C_WRITE(0x02, (u32IntTime>>8)&0xFF);	
#endif
		return;

}

static HI_VOID soih42_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
		HI_U8 u8High, u8Low, u8Dg;
		switch (u32Again)
		{
			case 0 :	/* 0db, 1 multiplies */
				u8High = 0x00;
				u8Dg   = 0x00;
				break;
			case 1 :	/* 6db, 2 multiplies */
				u8High = 0x010;
				u8Dg   = 0x00;
				break;
			case 2 :	/* 12db, 4 multiplies */
				u8High = 0x020;
				u8Dg   = 0x00;
				break;
			case 3 :	/* 18db, 8 multiplies */
				u8High = 0x030;
				u8Dg   = 0x00;
				break;
			default:
				u8High = 0x00;
				u8Dg   = 0x00;
				break;
		}
	
		u8Low = (u32Dgain - 16) & 0xf;
	
#if CMOS_SOIH42_ISP_WRITE_SENSOR_ENABLE
		u8Dg = 0x40;    //update
		soih42_init_regs_info();
		g_stSnsRegsInfo.astI2cData[2].u32Data = (u8High | u8Low);
		g_stSnsRegsInfo.astI2cData[3].u32Data = u8Dg;
		HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
#else
		SENSOR_I2C_WRITE(0x00, (u8High | u8Low));
		//SENSOR_I2C_WRITE(0x0d, u8Dg);
#endif
	
		return;

}



static HI_S32 soih42_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    
    pstAwbSnsDft->u16WbRefTemp = 5048;

    pstAwbSnsDft->au16GainOffset[0] = 0x152;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0X1ab;

    pstAwbSnsDft->as32WbPara[0] = 139;
    pstAwbSnsDft->as32WbPara[1] = -51;
    pstAwbSnsDft->as32WbPara[2] = -167;
    pstAwbSnsDft->as32WbPara[3] = 184226;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -136368;


    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;

}

static HI_VOID soih42_global_init()
{
	gu8SensorMode = 0;
}

void soih42_reg_init()
{
 //   SENSOR_I2C_WRITE(0x0e, 0x1D);
   SENSOR_I2C_WRITE(0x12, 0x40);
                                       //DVP Setting
    SENSOR_I2C_WRITE(0x0D, 0x40);
    SENSOR_I2C_WRITE(0x1F, 0x04);
                                           //PLL Setting
    SENSOR_I2C_WRITE(0x0E, 0x1D);
    SENSOR_I2C_WRITE(0x0F, 0x09);
    SENSOR_I2C_WRITE(0x10, 0x1E);
    SENSOR_I2C_WRITE(0x11, 0x80);
                                            //Frame/Window
    SENSOR_I2C_WRITE(0x20, 0x40);
    SENSOR_I2C_WRITE(0x21, 0x06);
    SENSOR_I2C_WRITE(0x22, 0x84);
    SENSOR_I2C_WRITE(0x23, 0x03);
    SENSOR_I2C_WRITE(0x24, 0x00);
    SENSOR_I2C_WRITE(0x25, 0xD0);
    SENSOR_I2C_WRITE(0x26, 0x25);
  //  SENSOR_I2C_WRITE(0x27, 0x49);                                             //3B 
    SENSOR_I2C_WRITE(0x27, 0x3B);//0x45(0x09=0x00/0x80ʱ)                                             //3B
    SENSOR_I2C_WRITE(0x28, 0x0d);                                             //0D
    SENSOR_I2C_WRITE(0x29, 0x01);
    SENSOR_I2C_WRITE(0x2A, 0x24);
    SENSOR_I2C_WRITE(0x2B, 0x29);
    SENSOR_I2C_WRITE(0x2C, 0x04);//0x0(0x09=0x00/0x80ʱ)                                             //03
    SENSOR_I2C_WRITE(0x2D, 0x00);
    SENSOR_I2C_WRITE(0x2E, 0xB9);
    SENSOR_I2C_WRITE(0x2F, 0x00);
                                           //Sensor Timing
    SENSOR_I2C_WRITE(0x30, 0x92);
    SENSOR_I2C_WRITE(0x31, 0x0A);
    SENSOR_I2C_WRITE(0x32, 0xAA);
    SENSOR_I2C_WRITE(0x33, 0x14);
    SENSOR_I2C_WRITE(0x34, 0x38);
    SENSOR_I2C_WRITE(0x35, 0x54);
    SENSOR_I2C_WRITE(0x42, 0x41);
    SENSOR_I2C_WRITE(0x43, 0x50);                                             //40
                                         //Interface
    SENSOR_I2C_WRITE(0x1D, 0xFF);
    SENSOR_I2C_WRITE(0x1E, 0x9F);
    SENSOR_I2C_WRITE(0x6C, 0x90);
    SENSOR_I2C_WRITE(0x73, 0xB3);
    SENSOR_I2C_WRITE(0x70, 0x68);
    SENSOR_I2C_WRITE(0x76, 0x40);
    SENSOR_I2C_WRITE(0x77, 0x06);
	SENSOR_I2C_WRITE(0x48, 0x60);
                                          //Array/AnADC/PWC
    SENSOR_I2C_WRITE(0x60, 0xA4);
    SENSOR_I2C_WRITE(0x61, 0xFF);
    SENSOR_I2C_WRITE(0x62, 0x40);
    SENSOR_I2C_WRITE(0x65, 0x00);	
    SENSOR_I2C_WRITE(0x66, 0x20);      //add  yang
    SENSOR_I2C_WRITE(0x67, 0x30);
    SENSOR_I2C_WRITE(0x68, 0x04);
    SENSOR_I2C_WRITE(0x69, 0x74);
    SENSOR_I2C_WRITE(0x6F, 0x04);
	SENSOR_I2C_WRITE(0x63, 0x51);//0x0
	SENSOR_I2C_WRITE(0x6A, 0x09);
                                          //AE/AG/ABLC
    SENSOR_I2C_WRITE(0x13, 0x87);
    SENSOR_I2C_WRITE(0x14, 0x80);
    SENSOR_I2C_WRITE(0x16, 0xC0);
    SENSOR_I2C_WRITE(0x17, 0x40);
    SENSOR_I2C_WRITE(0x18, 0xE1);
    SENSOR_I2C_WRITE(0x38, 0x35);
    SENSOR_I2C_WRITE(0x39, 0x98);
    SENSOR_I2C_WRITE(0x4a, 0x03);
    SENSOR_I2C_WRITE(0x49, 0x08);                                             //10
                                          //INI End
    SENSOR_I2C_WRITE(0x12, 0x00);
                                                 //PWDN Setting
    
    return ;
}




/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 soih42_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = soih42_reg_init;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = soih42_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = soih42_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = soih42_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = soih42_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = soih42_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = soih42_get_sensor_max_resolution;

    return 0;
}


HI_S32 soih42_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = soih42_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = soih42_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= soih42_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = soih42_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = soih42_gains_update;

    return 0;
}


HI_S32 soih42_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = soih42_get_awb_default;

    return 0;
}




int soih42_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    soih42_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(SOIH42_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    soih42_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, SOIH42_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    soih42_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, SOIH42_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int soih42_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(SOIH42_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, SOIH42_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, SOIH42_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}



void SOIH42_init(SENSOR_SOIH42_DO_I2CRD do_i2c_read, SENSOR_SOIH42_DO_I2CWR do_i2c_write)
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	soih42_reg_init();

	soih42_sensor_register_callback();

    ALG_LIB_S stLib;
	HI_S32 s32Ret;
    /* 1. register ae lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AE_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 2. register awb lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AWB_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 3. register af lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_AF_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AF_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // __SOIH42_CMOS_H_


