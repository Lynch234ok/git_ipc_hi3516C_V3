#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_APTINA_AR0330_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_APTINA_AR0330_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__AR0330_CMOS_H_)
#define __AR0330_CMOS_H_

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

#define AR0330_ID 330

/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance                       */
/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c                                         */
#define CMOS_AR0330_ISP_WRITE_SENSOR_ENABLE (1)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr = 0x20;
static unsigned int sensor_addr_byte = 0x2;
static unsigned int sensor_data_byte = 0x2;
static HI_U32 gu32FullLinesStd = 1288;
static HI_U8 gu8SensorMode = 0;

#if CMOS_AR0330_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif


static AWB_CCM_S g_stAwbCcm =
{
#if 1//cal by Green Lew
		
	5000,
	{
		0x0161, 0x8047, 0x801a,
		0x8032, 0x0125, 0xc,
		0x4, 0x8097, 0x0192
	},

	4000,
	{
		0x0172, 0x8040, 0x8032,
		0x8041, 0x012b, 0x15,
		0x8002, 0x809f, 0x01a1
	},

	2500,
	{
		0x170, 0x8020, 0x8050,
		0x806c, 0x0133, 0x38,
		0x8028, 0x8179, 0x2a1
	}
#else //cal by M
		
	5000,
	{
		0x0186, 0x804e, 0x8038,
		0x8028, 0x010c, 0x1c,
		0xa, 0x80a5, 0x019b
	},

	4000,
	{
		0x015f, 0x8027, 0x8038,
		0x8067, 0x013f, 0x28,
		0x803a, 0x8151, 0x028b
	},

	2500,
	{
		0x167, 0x803f, 0x8028,
		0x8046, 0x0129, 0x1d,
		0x8002, 0x80b0, 0x1b2
	}
#endif
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */    
    {0x90,0x90,0x88,0x78,0x70,0x40,0x3C,0x38}
};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,

    /* sharpen_alt_d */
    {0x70,0x68,0x60,0x50,0x34,0x30,0x28,0x28},

    //sharpen_alt_ud
    {0x90,0x88,0x80,0x70,0x60,0x50,0x40,0x40},

    //snr_thresh
    {0x13,0x19,0x20,0x26,0x32,0x46,0x48,0x50},

    /* demosaic_lum_thresh */
    {0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80},
        
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
    {0,  0,  0,  0,  0,  0,  11, 15, 17, 19, 20, 21, 22, 22, 23, 24,
    25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29,
    30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32,
    32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34,
    34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38},
    
    /* demosaic_weight_lut */
    {0, 11, 15, 17, 19, 20, 21, 22, 23, 23, 24, 25, 25, 26, 26, 26,
    27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30,
    31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33,
    33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35,
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38}
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,

    /*vh_slope*/
    0xe9,

    /*aa_slope*/
    0xd3,

    /*va_slope*/
    0xcc,

    /*uu_slope*/
    0xc2,

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0xd8,

    /*aa_thresh*/
    0xd5,

    /*va_thresh*/
    0xb6,

    /*uu_thresh*/
    0xc4,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3

};

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,
    
#if 1    
    {0  ,120 ,220 ,310 ,390 ,470 ,540 ,610 ,670 ,730 ,786 ,842 ,894 ,944 ,994 ,1050,    
    1096,1138,1178,1218,1254,1280,1314,1346,1378,1408,1438,1467,1493,1519,1543,1568,    
    1592,1615,1638,1661,1683,1705,1726,1748,1769,1789,1810,1830,1849,1869,1888,1907,    
    1926,1945,1963,1981,1999,2017,2034,2052,2069,2086,2102,2119,2136,2152,2168,2184,    
    2200,2216,2231,2247,2262,2277,2292,2307,2322,2337,2351,2366,2380,2394,2408,2422,    
    2436,2450,2464,2477,2491,2504,2518,2531,2544,2557,2570,2583,2596,2609,2621,2634,    
    2646,2659,2671,2683,2696,2708,2720,2732,2744,2756,2767,2779,2791,2802,2814,2825,    
    2837,2848,2859,2871,2882,2893,2904,2915,2926,2937,2948,2959,2969,2980,2991,3001,    
    3012,3023,3033,3043,3054,3064,3074,3085,3095,3105,3115,3125,3135,3145,3155,3165,    
    3175,3185,3194,3204,3214,3224,3233,3243,3252,3262,3271,3281,3290,3300,3309,3318,    
    3327,3337,3346,3355,3364,3373,3382,3391,3400,3409,3418,3427,3436,3445,3454,3463,    
    3471,3480,3489,3498,3506,3515,3523,3532,3540,3549,3557,3566,3574,3583,3591,3600,    
    3608,3616,3624,3633,3641,3649,3657,3665,3674,3682,3690,3698,3706,3714,3722,3730,    
    3738,3746,3754,3762,3769,3777,3785,3793,3801,3808,3816,3824,3832,3839,3847,3855,    
    3862,3870,3877,3885,3892,3900,3907,3915,3922,3930,3937,3945,3952,3959,3967,3974,    
    3981,3989,3996,4003,4010,4018,4025,4032,4039,4046,4054,4061,4068,4075,4082,4089,4095}
#else  /*higher  contrast*/
    {0  , 54 , 106, 158, 209, 259, 308, 356, 403, 450, 495, 540, 584, 628, 670, 713,
    754 ,795 , 835, 874, 913, 951, 989,1026,1062,1098,1133,1168,1203,1236,1270,1303,
    1335,1367,1398,1429,1460,1490,1520,1549,1578,1607,1635,1663,1690,1717,1744,1770,
    1796,1822,1848,1873,1897,1922,1946,1970,1993,2017,2040,2062,2085,2107,2129,2150,
    2172,2193,2214,2235,2255,2275,2295,2315,2335,2354,2373,2392,2411,2429,2447,2465,
    2483,2501,2519,2536,2553,2570,2587,2603,2620,2636,2652,2668,2684,2700,2715,2731,
    2746,2761,2776,2790,2805,2819,2834,2848,2862,2876,2890,2903,2917,2930,2944,2957,
    2970,2983,2996,3008,3021,3033,3046,3058,3070,3082,3094,3106,3118,3129,3141,3152,
    3164,3175,3186,3197,3208,3219,3230,3240,3251,3262,3272,3282,3293,3303,3313,3323,
    3333,3343,3352,3362,3372,3381,3391,3400,3410,3419,3428,3437,3446,3455,3464,3473,
    3482,3490,3499,3508,3516,3525,3533,3541,3550,3558,3566,3574,3582,3590,3598,3606,
    3614,3621,3629,3637,3644,3652,3660,3667,3674,3682,3689,3696,3703,3711,3718,3725,
    3732,3739,3746,3752,3759,3766,3773,3779,3786,3793,3799,3806,3812,3819,3825,3831,
    3838,3844,3850,3856,3863,3869,3875,3881,3887,3893,3899,3905,3910,3916,3922,3928,
    3933,3939,3945,3950,3956,3962,3967,3973,3978,3983,3989,3994,3999,4005,4010,4015,
    4020,4026,4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095,4095}
#endif
};

static  HI_U32   analog_gain_table[29]={ 
     128,131,136,140,145,152,157,163,170,177,
     185,194,204,215,227,240,256,272,293,314,
     341,372,409,455,512,584,682,819,1024
};


static  HI_U32   gain_table[29] = 
{ 
    1024,1054,1096,1126,1167,1218,1259,1311,1362,1423,1484,1556,1638,1720,1822,1925,
    2048,2181,2345,2519,2734,2979,3276,3645,4096,4679,5458,6553,8192
};

HI_U32 ar0330_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

    pstDef->stComm.u8Rggb = 0x1;      //1: rggb 
    pstDef->stComm.u8BalanceFe = 0x1;

    pstDef->stDenoise.u8SinterThresh = 0x23;
    pstDef->stDenoise.u8NoiseProfile= 0x1;      //0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
    pstDef->stDenoise.u16Nr0 = 0x0;
    pstDef->stDenoise.u16Nr1 = 546;

    pstDef->stDrc.u8DrcBlack = 0x00;
    pstDef->stDrc.u8DrcVs = 0x04;     // variance space
    pstDef->stDrc.u8DrcVi = 0x01;     // variance intensity
    pstDef->stDrc.u8DrcSm = 0x30;     // slope max
    pstDef->stDrc.u16DrcWl = 0x4FF;    // white level

    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));

    return 0;
}

HI_U32 ar0330_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{  
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

	pstBlackLevel->au16BlackLevel[0] = 171;
	pstBlackLevel->au16BlackLevel[1] = 169;
    pstBlackLevel->au16BlackLevel[2] = 172;      
	pstBlackLevel->au16BlackLevel[3] = 172;
	
    return 0;    
}

HI_VOID ar0330_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(0x300A, 0x1E30);	//5fps
        SENSOR_I2C_WRITE(0x3012, 0x1E28);	//max exposure lines
        SENSOR_I2C_WRITE(0x3060, 0x1300);	//AG, Context A
        SENSOR_I2C_WRITE(0x305E, 0x0128);	//DG, Context A
    }
    else /* setup for ISP 'normal mode' */
    {
       SENSOR_I2C_WRITE(0x300A, 0x444);	//30fps
    }

    return;
}

HI_VOID ar0330_set_wdr_mode(HI_U8 u8Mode)
{
    switch(u8Mode)
    {
        //sensor mode 0
        case 0:
            gu8SensorMode = 0;
            // TODO:
        break;
        //sensor mode 1
        case 1:
            gu8SensorMode = 1;
             // TODO:
        break;

        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    
    return;
}

static HI_S32 ar0330_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = 1288;
    
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x80;
    
    pstAeSnsDft->u32LinesPer500ms = 1288*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = 1286;
    pstAeSnsDft->u32MinIntTime = 2;    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0078125;
    pstAeSnsDft->u32MaxAgain = 8192;  /* 8/0.0078125= 1024 */
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = 8192;
    pstAeSnsDft->u32MinAgainTarget = 1024;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0078125;    
    pstAeSnsDft->u32MaxDgain = 2047;/* 16 / 0.0078125 = 2047;*/
    pstAeSnsDft->u32MinDgain = 128;
    pstAeSnsDft->u32MaxDgainTarget = 2047;
    pstAeSnsDft->u32MinDgainTarget = 128;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

static HI_S32 ar0330_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
    if (HI_NULL == pstSensorMaxResolution)
    {
        printf("null pointer when get sensor max resolution \n");
        return -1;
    }

    memset(pstSensorMaxResolution, 0, sizeof(ISP_CMOS_SENSOR_MAX_RESOLUTION));

    pstSensorMaxResolution->u32MaxWidth  = 1920;
    pstSensorMaxResolution->u32MaxHeight = 1080;

    return 0;
}


/* the function of sensor set fps */
static HI_VOID ar0330_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch(u8Fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 1288;
            pstAeSnsDft->u32MaxIntTime = 1286;
            pstAeSnsDft->u32LinesPer500ms = 1288 * 30 / 2;
             SENSOR_I2C_WRITE(0x300A, 0x508);
        break;
        
        case 25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 1545;
            pstAeSnsDft->u32MaxIntTime = 1543;
            pstAeSnsDft->u32LinesPer500ms = 1545 * 25 / 2;
           SENSOR_I2C_WRITE(0x300A, 0x609);
        break;
        
        default:
        break;
    }

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID ar0330_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

    SENSOR_I2C_WRITE(0x300A, u16FullLines);

    pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;
    
    return;
}

static HI_VOID ar0330_init_regs_info(HI_VOID)
{
    
#if CMOS_AR0330_ISP_WRITE_SENSOR_ENABLE

    HI_S32 i;
    static HI_BOOL bInit = HI_FALSE;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u32RegNum = 3;
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3012;
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3060;
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x305E;

        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_FALSE;

        bInit = HI_TRUE;
    }
#endif
    return;
}



/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ar0330_inttime_update(HI_U32 u32IntTime)
{
    static HI_U32 _last_exposure_time = 0xFFFFFFFF;

    if(_last_exposure_time == u32IntTime)
    {
        return ;
    }else
    {
        _last_exposure_time = u32IntTime;
    }
    
#if CMOS_AR0330_ISP_WRITE_SENSOR_ENABLE
    ar0330_init_regs_info();
    g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;
#else
    SENSOR_I2C_WRITE(0x3012, u32IntTime);
#endif
    return;
}


static HI_VOID ar0330_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
 
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;
   
    if (u32InTimes >= gain_table[28])
    {
         pstAeSnsGainInfo->u32SnsTimes = gain_table[28];
         pstAeSnsGainInfo->u32GainDb = 28;
         return ;
    }
    
    for(i = 1; i < 29; i++)
    {
        if(u32InTimes < gain_table[i])
        {
            pstAeSnsGainInfo->u32SnsTimes = gain_table[i - 1];
            pstAeSnsGainInfo->u32GainDb = i - 1;
            break;
        }

    }
          
    return;

}


static HI_VOID ar0330_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    static HI_U32 _last_again = 0xFFFFFFFF;
    static HI_U32 _last_dgain = 0xFFFFFFFF;
    static HI_U32 _last_exposure = 0xFFFFFFFF;
 
    if((_last_again ==  u32Again ) && (_last_dgain == u32Dgain)&& g_stSnsRegsInfo.astI2cData[0].u32Data == _last_exposure)
    {
        return ;
    }
    else
    {
        _last_again = u32Again;
        _last_dgain= u32Dgain;
        _last_exposure = g_stSnsRegsInfo.astI2cData[0].u32Data;
    }
    
#if CMOS_AR0330_ISP_WRITE_SENSOR_ENABLE
    ar0330_init_regs_info();
       
    if( u32Again < 16)
    {
        g_stSnsRegsInfo.astI2cData[1].u32Data = 0x00 + u32Again;
    }else if( u32Again <24)
    {
        g_stSnsRegsInfo.astI2cData[1].u32Data = 0x10 + (u32Again - 16) * 2;
    }else if( u32Again <28)
    {
        g_stSnsRegsInfo.astI2cData[1].u32Data = 0x20 + (u32Again - 24) * 4;
    }else if( u32Again == 28)
    {
        g_stSnsRegsInfo.astI2cData[1].u32Data = 0x30;
    }
  
    g_stSnsRegsInfo.astI2cData[2].u32Data = u32Dgain;


    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
#endif
    return;
}

static HI_S32 ar0330_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4900;

    pstAwbSnsDft->au16GainOffset[0] = 0x16c;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1c2;

    pstAwbSnsDft->as32WbPara[0] = 114;
    pstAwbSnsDft->as32WbPara[1] = -50;
    pstAwbSnsDft->as32WbPara[2] = -193;
    pstAwbSnsDft->as32WbPara[3] = 220171;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -174549;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

static HI_VOID ar0330_global_init()
{
	gu8SensorMode = 0;
}

static void ar0330_reg_init(){
#if 0
	SENSOR_I2C_WRITE(0x301A, 0x10D9);
	SENSOR_DELAY_MS(100);
    SENSOR_I2C_WRITE(0x301A, 0x10D8);

	
    SENSOR_I2C_WRITE(0x30FE, 0x0080);
    SENSOR_I2C_WRITE(0x31E0, 0x0303);
    SENSOR_I2C_WRITE(0x3ECE, 0x08FF);
    SENSOR_I2C_WRITE(0x3ED0, 0xE4F6);
    SENSOR_I2C_WRITE(0x3ED2, 0x0146);
    SENSOR_I2C_WRITE(0x3ED4, 0x8F6C);
    SENSOR_I2C_WRITE(0x3ED6, 0x66CC);
    SENSOR_I2C_WRITE(0x3ED8, 0x8C42);
    SENSOR_I2C_WRITE(0x3EDA, 0x889B);
    SENSOR_I2C_WRITE(0x3EDC, 0x8863);
    SENSOR_I2C_WRITE(0x3EDE, 0xAA04);
    SENSOR_I2C_WRITE(0x3EE0, 0x15F0);
    SENSOR_I2C_WRITE(0x3EE6, 0x008C);
    SENSOR_I2C_WRITE(0x3EE8, 0x2024);
    SENSOR_I2C_WRITE(0x3EEA, 0xFF1F);
    SENSOR_I2C_WRITE(0x3F06, 0x046A);
    SENSOR_I2C_WRITE(0x3064, 0x1802);

	
    SENSOR_I2C_WRITE(0x3EDA, 0x88BC);
    SENSOR_I2C_WRITE(0x3EDC, 0xAA63);

	
    SENSOR_I2C_WRITE(0x305E, 0x00A0);

	
    SENSOR_I2C_WRITE(0x302A, 0x0006);
    SENSOR_I2C_WRITE(0x302C, 0x0001);
    SENSOR_I2C_WRITE(0x302E, 0x0002);
    SENSOR_I2C_WRITE(0x3030, 0x0025);
    SENSOR_I2C_WRITE(0x3036, 0x000C);
    SENSOR_I2C_WRITE(0x3038, 0x0001);
    SENSOR_I2C_WRITE(0x31AC, 0x0C0C);

	
    SENSOR_I2C_WRITE(0x31AE, 0x0301);
    SENSOR_I2C_WRITE(0x3002, 0x00EA);
    SENSOR_I2C_WRITE(0x3004, 0x00C6);
    SENSOR_I2C_WRITE(0x3006, 0x0521);
    SENSOR_I2C_WRITE(0x3008, 0x0845);
    SENSOR_I2C_WRITE(0x300A, 0x0444);
    SENSOR_I2C_WRITE(0x300C, 0x0469);
    SENSOR_I2C_WRITE(0x3012, 0x0147);
    SENSOR_I2C_WRITE(0x3014, 0x0000);
    SENSOR_I2C_WRITE(0x30A2, 0x0001);
    SENSOR_I2C_WRITE(0x30A6, 0x0001);
    SENSOR_I2C_WRITE(0x308C, 0x0006);
    SENSOR_I2C_WRITE(0x308A, 0x0006);
    SENSOR_I2C_WRITE(0x3090, 0x0605);
    SENSOR_I2C_WRITE(0x308E, 0x0905);
    SENSOR_I2C_WRITE(0x30AA, 0x0B94);
    SENSOR_I2C_WRITE(0x303E, 0x04E0);
    SENSOR_I2C_WRITE(0x3016, 0x0B92);
    SENSOR_I2C_WRITE(0x3018, 0x0000);
    SENSOR_I2C_WRITE(0x30AE, 0x0001);
    SENSOR_I2C_WRITE(0x30A8, 0x0001);
    SENSOR_I2C_WRITE(0x3040, 0x0000);
    SENSOR_I2C_WRITE(0x3042, 0x0000);
    SENSOR_I2C_WRITE(0x30BA, 0x002C);

	
    SENSOR_I2C_WRITE(0x3088, 0x80BA);
    SENSOR_I2C_WRITE(0x3086, 0x0253);
    SENSOR_I2C_WRITE(0x301A, 0x10DC);
#else
SENSOR_I2C_WRITE(0x301A, 0x10D9);	  
SENSOR_DELAY_MS(100);	   
SENSOR_I2C_WRITE(0x301A, 0x10D8);
SENSOR_I2C_WRITE(0x30FE, 0x0080);	 
SENSOR_I2C_WRITE(0x31E0, 0x0303);    
SENSOR_I2C_WRITE(0x3ECE, 0x08FF);	 
SENSOR_I2C_WRITE(0x3ED0, 0xE4F6);   
SENSOR_I2C_WRITE(0x3ED2, 0x0146);	
SENSOR_I2C_WRITE(0x3ED4, 0x8F6C);  
SENSOR_I2C_WRITE(0x3ED6, 0x66CC);	
SENSOR_I2C_WRITE(0x3ED8, 0x8C42);   
SENSOR_I2C_WRITE(0x3EDA, 0x889B);	
SENSOR_I2C_WRITE(0x3EDC, 0x8863);   
SENSOR_I2C_WRITE(0x3EDE, 0xAA04);	
SENSOR_I2C_WRITE(0x3EE0, 0x15F0);  
SENSOR_I2C_WRITE(0x3EE6, 0x008C);
SENSOR_I2C_WRITE(0x3EE8, 0x2024);   
SENSOR_I2C_WRITE(0x3EEA, 0xFF1F);	
SENSOR_I2C_WRITE(0x3F06, 0x046A);  
SENSOR_I2C_WRITE(0x3064, 0x1802);	 
SENSOR_I2C_WRITE(0x3EDA, 0x88BC);
SENSOR_I2C_WRITE(0x3EDC, 0xAA63); 	
SENSOR_I2C_WRITE(0x305E, 0x00A0);	  
SENSOR_I2C_WRITE(0x302A, 0x0006);	
SENSOR_I2C_WRITE(0x302C, 0x0001);  
SENSOR_I2C_WRITE(0x302E, 0x0003);	
SENSOR_I2C_WRITE(0x3030, 0x0040);  
SENSOR_I2C_WRITE(0x3036, 0x000C);	 
SENSOR_I2C_WRITE(0x3038, 0x0001);   
SENSOR_I2C_WRITE(0x31AC, 0x0C0C);	   
SENSOR_I2C_WRITE(0x31AE, 0x0301);	
SENSOR_I2C_WRITE(0x3002, 0x00EA);   
SENSOR_I2C_WRITE(0x3004, 0x00C6);	 
SENSOR_I2C_WRITE(0x3006, 0x0521);   
SENSOR_I2C_WRITE(0x3008, 0x0845);	
SENSOR_I2C_WRITE(0x300A, 0x0508);  
SENSOR_I2C_WRITE(0x300C, 0x04DA);	 
SENSOR_I2C_WRITE(0x3012, 0x0147); 
SENSOR_I2C_WRITE(0x3014, 0x0000);	
SENSOR_I2C_WRITE(0x30A2, 0x0001); 
SENSOR_I2C_WRITE(0x30A6, 0x0001);	 
SENSOR_I2C_WRITE(0x308C, 0x0006);   
SENSOR_I2C_WRITE(0x308A, 0x0006);	 
SENSOR_I2C_WRITE(0x3090, 0x0605);   
SENSOR_I2C_WRITE(0x308E, 0x0905);	 
SENSOR_I2C_WRITE(0x30AA, 0x0B94);  
SENSOR_I2C_WRITE(0x303E, 0x04E0);	 
SENSOR_I2C_WRITE(0x3016, 0x0B92);  
SENSOR_I2C_WRITE(0x3018, 0x0000);	
SENSOR_I2C_WRITE(0x30AE, 0x0001);  
SENSOR_I2C_WRITE(0x30A8, 0x0001);	 
SENSOR_I2C_WRITE(0x3040, 0x0000);   
SENSOR_I2C_WRITE(0x3042, 0x0000);	 
SENSOR_I2C_WRITE(0x30BA, 0x002C); 	
SENSOR_I2C_WRITE(0x3088, 0x80BA);	
SENSOR_I2C_WRITE(0x3086, 0x0253);  
SENSOR_I2C_WRITE(0x301A, 0x10DC);


#endif
    printf("Aptina AR0330 sensor 1080p 30fps reg init success!\n");
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 ar0330_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
	{
		memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

		pstSensorExpFunc->pfn_cmos_sensor_init = ar0330_reg_init;
		pstSensorExpFunc->pfn_cmos_sensor_global_init = ar0330_global_init;
		pstSensorExpFunc->pfn_cmos_get_isp_default = ar0330_get_isp_default;
		pstSensorExpFunc->pfn_cmos_get_isp_black_level = ar0330_get_isp_black_level;
		pstSensorExpFunc->pfn_cmos_set_pixel_detect = ar0330_set_pixel_detect;
		pstSensorExpFunc->pfn_cmos_set_wdr_mode = ar0330_set_wdr_mode;
		pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = ar0330_get_sensor_max_resolution;

		return 0;
	}

HI_S32 ar0330_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
	{
		memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

		pstExpFuncs->pfn_cmos_get_ae_default = ar0330_get_ae_default;
		pstExpFuncs->pfn_cmos_fps_set = ar0330_fps_set;
		pstExpFuncs->pfn_cmos_slow_framerate_set= ar0330_slow_framerate_set;	  
		pstExpFuncs->pfn_cmos_inttime_update = ar0330_inttime_update;
		pstExpFuncs->pfn_cmos_gains_update = ar0330_gains_update;
		pstExpFuncs->pfn_cmos_again_calc_table	= ar0330_again_calc_table;

		return 0;
	}

HI_S32 ar0330_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
 {
	 memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
 
	 pstExpFuncs->pfn_cmos_get_awb_default = ar0330_get_awb_default;
 
	 return 0;
 }
 
 int ar0330_sensor_register_callback(void)
 {
	 HI_S32 s32Ret;
	 ALG_LIB_S stLib;
	 ISP_SENSOR_REGISTER_S stIspRegister;
	 AE_SENSOR_REGISTER_S  stAeRegister;
	 AWB_SENSOR_REGISTER_S stAwbRegister;
 
	 ar0330_init_sensor_exp_function(&stIspRegister.stSnsExp);
	 s32Ret = HI_MPI_ISP_SensorRegCallBack(AR0330_ID, &stIspRegister);
	 if (s32Ret)
	 {
		 printf("sensor register callback function failed!\n");
		 return s32Ret;
	 }
	 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AE_LIB_NAME);
	 ar0330_init_ae_exp_function(&stAeRegister.stSnsExp);
	 s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, AR0330_ID, &stAeRegister);
	 if (s32Ret)
	 {
		 printf("sensor register callback function to ae lib failed!\n");
		 return s32Ret;
	 }
 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
	 ar0330_init_awb_exp_function(&stAwbRegister.stSnsExp);
	 s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, AR0330_ID, &stAwbRegister);
	 if (s32Ret)
	 {
		 printf("sensor register callback function to ae lib failed!\n");
		 return s32Ret;
	 }
	 
	 return 0;
 }
 
 int ar0330_sensor_unregister_callback(void)
 {
	 HI_S32 s32Ret;
	 ALG_LIB_S stLib;
 
	 s32Ret = HI_MPI_ISP_SensorUnRegCallBack(AR0330_ID);
	 if (s32Ret)
	 {
		 printf("sensor unregister callback function failed!\n");
		 return s32Ret;
	 }
	 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AE_LIB_NAME);
	 s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, AR0330_ID);
	 if (s32Ret)
	 {
		 printf("sensor unregister callback function to ae lib failed!\n");
		 return s32Ret;
	 }
 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
	 s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, AR0330_ID);
	 if (s32Ret)
	 {
		 printf("sensor unregister callback function to ae lib failed!\n");
		 return s32Ret;
	 }
	 
	 return 0;
 }

void APTINA_AR0330_init(SENSOR_APTINA_AR0330_DO_I2CRD do_i2c_read, SENSOR_APTINA_AR0330_DO_I2CWR do_i2c_write)
{	
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	ar0330_reg_init();

	ar0330_sensor_register_callback();

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
	printf("Aptina AR0330 sensor 1080P30fps init success!\n");
	
	return 0;

}

#ifdef __cplusplus
#if __cplusplus
 }
#endif
#endif /* End of #ifdef __cplusplus */
 
#endif // __AR0330_CMOS_H_

