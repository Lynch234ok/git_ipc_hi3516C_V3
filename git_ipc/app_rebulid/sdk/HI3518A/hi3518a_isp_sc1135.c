#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_SC1135_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_SC1135_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__SC1135_CMOS_H_)
#define __SC1135_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"



//#include "ar0141_sensor_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define SC1135_ID 1135
#define IQSettingFromAR0130 0
#define IQSettingFromSOIH22 1 /* choose one*/


//#define YFTestCode
#define SENSORCLK_72M 0
#define SENSORCLK_63M 0
#define SENSORCLK_54M 0
#define SENSORCLK_54M960P 1
/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance                       */
/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c                                         */
#define CMOS_SC1135_ISP_WRITE_SENSOR_ENABLE (1)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/



static HI_U32 gu32FullLinesStd = 1000;

 static HI_U8  gu8SensorMode = 0;
 
 
 static const unsigned char sensor_i2c_addr  =	 0x60;		 /* I2C Address of SC1135 */
 static const unsigned int	sensor_addr_byte =	 2;
 static const unsigned int	sensor_data_byte =	 1;
 
#if CMOS_SC1135_ISP_WRITE_SENSOR_ENABLE
 static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif
static AWB_CCM_S g_stAwbCcm =
{
 
       /*5000,
       {
         0x01a2, 0x8080, 0x8022,
         0x8068, 0x0178, 0x8010,
         0x8008, 0x811A, 0x0222
       }, 
       
       3200,
       {
         0x01e0,0x807b,0x8065,
         0x804f,0x0165,0x8016,
         0x001f,0x8126,0x0206
       },
       
       2600,
       {
         0x0200, 0x80c3, 0x803D,
         0x8057, 0x015C, 0x8005,
         0x8020, 0x8290, 0x03b0
       }*/
       5000,
		{
			0x1dc,0x808d,0x804f,
			0x802b,0x18b,0x8060,
			0x13,  0x8118,0x204
		},
	
		4000,
		{
			0x01a9,0x802e,0x807b,
			0x8056,0x01d4,0x8074,
			0x800b,0x8110,0x021b
		},
	
		2500,
		{
		 0xc6, 0x9a, 0x8061,
		 0x80c6, 0x1bc,  0x9,
		 0x8081, 0x8281, 0x402
		 }
 
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,
    /* saturation */
    {0x88,0x88,0x88,0x80,0x80,0x70,0x58,0x48} //copy from H22
};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
    
    /* sharpen_alt_d */
    //{0xbc,0xa8,0x88,0x73,0x4c,0x38,0x10,0x01},
    //{0xbc,0x6d,0x65,0x60,0x4c,0x38,0x10,0x01},
    {0xbc,0x69,0x68,0x58,0x44,0x3c,0x32,0x28},

    /* sharpen_alt_ud */
    //{0x80,0xa0,0x8e,0x64,0x4c,0x38,0x1e,0x10}, 
    //{0x8e,0x56,0x50,0x4d,0x4c,0x38,0x1e,0x10},
    {0xC0,0x7a,0x64,0x5a,0x50,0x46,0x3c,0x32},

    /* snr_thresh */
    {0x20,0x10,0x20,0x24,0x28,0x30,0x37,0x3c},
    //{0x1d,0x1e,0x1f,0x30,0x30,0x30,0x30,0x30},

    /* demosaic_lum_thresh */
    {0x40,0x60,0x80,0x80,0x80,0x80,0x80,0x80},
        
    /* demosaic_np_offset */
    {0x0,0x7,0xd,0x15,0x1e,0x28,0x30,0x30},
    //{0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},

    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37}

};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,

#if 1//1IQSettingFromAR0130 //YF    
    /* nosie_profile_weight_lut */
    {0,  0,  0,  0,  0,  0,  0, 12, 17, 21, 24, 26, 28, 30, 31, 32, 
    33, 34, 35, 36, 37, 38, 39, 39, 40, 41, 41, 42, 42, 43, 43, 44,
    44, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49,
    50, 50, 50, 50, 51, 51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53,
    53, 53, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 56, 56, 56,
    56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58,
    58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60,
    60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 62, 62},

    /* demosaic_weight_lut */
    {0, 12, 17, 21, 24, 26, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 
    39, 39, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 45, 46, 46, 
    46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 50, 51, 51, 
    51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 54, 54, 54, 54, 
    54, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 57, 57, 
    57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 
    59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60, 60, 61, 61, 
    61, 61, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 62, 62}

#elif IQSettingFromSOIH22 //copy from H22
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
#endif

};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    204,

    /*aa_slope*/
    194,

    /*va_slope*/
    205,

    /*uu_slope*/
    160,

    /*sat_slope*/
    93,

    /*ac_slope*/
    160,

    /*vh_thresh*/
    0,

    /*aa_thresh*/
    0,

    /*va_thresh*/
    0,

    /*uu_thresh*/
    8,

    /*sat_thresh*/
    48,

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

HI_U32 sc1135_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));


#if IQSettingFromAR0130 //YF
    pstDef->stComm.u8Rggb           = 0x1;      //1: rggb 
    pstDef->stComm.u8BalanceFe      = 0x1;

    pstDef->stDenoise.u8SinterThresh= 0x23;
    pstDef->stDenoise.u8NoiseProfile= 0x1;      //0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
    pstDef->stDenoise.u16Nr0        = 0x0;
    pstDef->stDenoise.u16Nr1        = 546;

    pstDef->stDrc.u8DrcBlack        = 0x00;
    pstDef->stDrc.u8DrcVs           = 0x04;     // variance space
    pstDef->stDrc.u8DrcVi           = 0x01;     // variance intensity
    pstDef->stDrc.u8DrcSm           = 0x80;     // slope max
    pstDef->stDrc.u16DrcWl          = 0x4FF;    // white level

#elif IQSettingFromSOIH22 //copy from H22
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

#endif



    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));

    return 0;
}

HI_U32 sc1135_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    /*for (i=0; i<4; i++)
    {
        pstBlackLevel->au16BlackLevel[i] = 0xc0;//0xC8; //YF Test
    }*/

	pstBlackLevel->au16BlackLevel[0] = 194;
	pstBlackLevel->au16BlackLevel[1] = 195;
	pstBlackLevel->au16BlackLevel[2] = 195;
	pstBlackLevel->au16BlackLevel[3] = 194;

    return 0;    
}

HI_VOID sc1135_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
		SENSOR_I2C_WRITE(0x320e, 0x03);
        SENSOR_I2C_WRITE(0x320f, 0xe8);
    }
    else /* setup for ISP 'normal mode' */
    {
        //SENSOR_I2C_WRITE(0x320e, 0x04);
        //SENSOR_I2C_WRITE(0x320f, 0xb0);
#if SENSORCLK_72M
        SENSOR_I2C_WRITE(0x320e, 0x04);
        SENSOR_I2C_WRITE(0x320f, 0xe2);
#elif SENSORCLK_63M
		SENSOR_I2C_WRITE(0x320e, 0x04);
        SENSOR_I2C_WRITE(0x320f, 0xb0);
#else 
		SENSOR_I2C_WRITE(0x320e, 0x03);
        SENSOR_I2C_WRITE(0x320f, 0xe8);
#endif
    }


    return;
}

HI_VOID sc1135_set_wdr_mode(HI_U8 u8Mode)
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


static HI_S32 sc1135_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }


    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;

    pstAeSnsDft->u8AeCompensation = 0x40;

#if SENSORCLK_72M
		gu32FullLinesStd = 1250;//1000;
		pstAeSnsDft->u32LinesPer500ms = 1250*30/2;//1000*30/2;//800*25/2;//750*30/2;
#elif SENSORCLK_63M
		gu32FullLinesStd = 1200;
		pstAeSnsDft->u32LinesPer500ms = 1200*30/2;//1000*30/2;//800*25/2;//750*30/2;
#else//45M
		gu32FullLinesStd = 1000;
		pstAeSnsDft->u32LinesPer500ms = 1000*30/2;//1000*30/2;//800*25/2;//750*30/2;
#endif

    
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 2;//998;//796;//748;
    pstAeSnsDft->u32MinIntTime = 2;
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;//2;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;//AE_ACCURACY_DB;//AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 1;//0.0588*64;//1 / 0x40;    
    pstAeSnsDft->u32MaxAgain = 15400;//~0x40 * 106.25;//1700;//~0x40 * 24.094;
    pstAeSnsDft->u32MinAgain = 1024;//0x40;//1024;
    pstAeSnsDft->u32MaxAgainTarget = 15400;//~0x40 * 106.25;//1700;//0x40 * 24.094;
    pstAeSnsDft->u32MinAgainTarget = 1024;//0x40;//1024;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;//0.0588;//0.03125;    
    pstAeSnsDft->u32MaxDgain = 4*1024;//17;//32;//255;  /* (8 - 0.3125) / 0.03125 = 255 */
    pstAeSnsDft->u32MinDgain = 1024;//17;//32;
    pstAeSnsDft->u32MaxDgainTarget = 4*1024;//17;//32;//255;
    pstAeSnsDft->u32MinDgainTarget = 1024;//17;//32;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 4/*1*/ << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

static HI_S32 sc1135_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
    if (HI_NULL == pstSensorMaxResolution)
    {
        printf("null pointer when get sensor max resolution \n");
        return -1;
    }

    memset(pstSensorMaxResolution, 0, sizeof(ISP_CMOS_SENSOR_MAX_RESOLUTION));

    pstSensorMaxResolution->u32MaxWidth  = 1280;
    pstSensorMaxResolution->u32MaxHeight = 960;

    return 0;
}


/* the function of sensor set fps */
static HI_VOID sc1135_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch(u8Fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
#if SENSORCLK_72M
            gu32FullLinesStd = 1250;//1000;
            pstAeSnsDft->u32MaxIntTime = 1248;//998;
            pstAeSnsDft->u32LinesPer500ms = 1250*30/2;//1000 * 30 / 2;
	    	SENSOR_I2C_WRITE(0x320c, 0x07);//1920
	   		SENSOR_I2C_WRITE(0x320d, 0x80);
#elif SENSORCLK_63M
			gu32FullLinesStd = 1200;//1000;
            pstAeSnsDft->u32MaxIntTime = 1198;//998;
            pstAeSnsDft->u32LinesPer500ms = 1200*30/2;//1000 * 30 / 2;
	    	SENSOR_I2C_WRITE(0x320c, 0x06);//1920
	   		SENSOR_I2C_WRITE(0x320d, 0xd6);
#else 
			gu32FullLinesStd = 1000;
            pstAeSnsDft->u32MaxIntTime = 998;
            pstAeSnsDft->u32LinesPer500ms = 1000 * 30 / 2;

#if SENSORCLK_54M960P		
	    	SENSOR_I2C_WRITE(0x320c, 0x07);//54M@960P
			SENSOR_I2C_WRITE(0x320d, 0x08);
#else	    	
			gu32FullLinesStd = 1000;
            pstAeSnsDft->u32MaxIntTime = 998;
            pstAeSnsDft->u32LinesPer500ms = 1000 * 30 / 2;
	    	SENSOR_I2C_WRITE(0x3011, 0x46);//1920
#endif//SENSORCLK_54M960P
#endif
        break;
        
        case 25:
            // Change the frame rate via changing the vertical blanking
#if SENSORCLK_72M            
			gu32FullLinesStd = 1250;//1000;
            pstAeSnsDft->u32MaxIntTime = 1248;//998;
            pstAeSnsDft->u32LinesPer500ms = 1250*25/2;//1000 * 30 / 2;
			SENSOR_I2C_WRITE(0x320c, 0x09);//2100
	    	SENSOR_I2C_WRITE(0x320d, 0x00);
#elif SENSORCLK_63M
			gu32FullLinesStd = 1200;//1000;
            pstAeSnsDft->u32MaxIntTime = 1198;//998;
            pstAeSnsDft->u32LinesPer500ms = 1200*25/2;//1000 * 30 / 2;
			SENSOR_I2C_WRITE(0x320c, 0x08);//2100
	    	SENSOR_I2C_WRITE(0x320d, 0x34);
#else 
			gu32FullLinesStd = 1000;
			pstAeSnsDft->u32MaxIntTime = 998;
			pstAeSnsDft->u32LinesPer500ms = 1000 * 30 / 2;

#if SENSORCLK_54M960P		
			SENSOR_I2C_WRITE(0x320c, 0x08);//54M@960P
			SENSOR_I2C_WRITE(0x320d, 0x70);
#else
			SENSOR_I2C_WRITE(0x3011, 0x66);//1920
#endif //SENSORCLK_54M960P			
#endif
        break;
        
        default:
        break;
    }
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID sc1135_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    SENSOR_I2C_WRITE(0x320e, (u16FullLines>>8)&0xff);
    SENSOR_I2C_WRITE(0x320f, u16FullLines&0xff);
    pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;

#if 1
    HI_U16 u16Temp = u16FullLines - 0x2e8;
    SENSOR_I2C_WRITE(0x3336, (u16Temp>>8)&0xff);
    SENSOR_I2C_WRITE(0x3337, u16Temp&0xff);

	SENSOR_I2C_WRITE(0x3338, (u16FullLines>>8)&0xff);
    SENSOR_I2C_WRITE(0x3339, u16FullLines&0xff);

    HI_U16 u16RegH = 0, u16RegL = 0;
    SENSOR_I2C_READ(0x320c, &u16RegH);
	SENSOR_I2C_READ(0x320d, &u16RegL);

	u16Temp = (u16RegH<<8) | u16RegL;
	u16Temp -= 0x20;
	SENSOR_I2C_WRITE(0x3320, (u16Temp>>8)&0xff);
    SENSOR_I2C_WRITE(0x3321, u16Temp&0xff);
	
#endif     
    return;
}

static HI_VOID sc1135_init_regs_info(HI_VOID)
{
#if CMOS_SC1135_ISP_WRITE_SENSOR_ENABLE
    HI_S32 i;
    static HI_BOOL bInit = HI_FALSE;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u32RegNum = 4;
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = 0x60;//sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = 2;//sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = 1;//sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3e01;//0x3012;
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3e02;//0x30B0;
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3e09;//0x305E;
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_TRUE;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3e08;//0x3100;
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_FALSE;

        bInit = HI_TRUE;
    }
#endif
    return;
}

static unsigned char Sensor_denoise_table[100] = 
{
 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,
 0xd9,0xd2,0xcc,0xc5,0xbf,0xb8,0xb1,0xab,0xa4,0x9e,
 0x97,0x90,0x8a,0x83,0x7d,0x76,0x6f,0x69,0x62,0x5c,
 0x55,0x4e,0x48,0x41,0x3b,0x34,0x2d,0x27,0x20,0x1a,
};

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID sc1135_inttime_update(HI_U32 u32IntTime)
{

#if CMOS_SC1135_ISP_WRITE_SENSOR_ENABLE
    sc1135_init_regs_info();
    //g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;
    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 4) & 0xFF;
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime << 4) & 0xF0;

#else
    //SENSOR_I2C_WRITE(0x3012, u32IntTime);
    SENSOR_I2C_WRITE(0x3e01, (u32IntTime >> 4) & 0xFF);
    SENSOR_I2C_WRITE(0x3e02, (u32IntTime << 4) & 0xF0);

#endif
#if 0 //reduce noise
	SENSOR_I2C_WRITE(0x331e, Sensor_denoise_table[(u32IntTime*100/gu32FullLinesStd) < 100 ? (u32IntTime*100/gu32FullLinesStd):100]);
#endif

    return;
}



static const HI_U16 sensor_gain_map[48] = {
	0x10, 0x11, 0x12,0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a,0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x30, 0x31, 0x32, 0x33,0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b,0x3c, 0x3d, 0x3e, 0x3f,
	0x70, 0x71, 0x72, 0x73,0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b,0x7c, 0x7d, 0x7e, 0x7f,
};


static HI_U16 gainmap_gain2index(HI_U16 Gain)
{
	HI_U16 i = 0;
	for(i = 0; i < sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0]); i++){
		if(sensor_gain_map[i] == Gain){
			break;
		}
	}
	return i;
}

//return limited Again
static HI_U32 sc1135_Again_limit(HI_U32 Again)
{
#define SENSOR_BLC_TOP_VALUE (0x58)
#define SENSOR_BLC_BOT_VALUE (0x45)
#define SENSOR_AGAIN_ADAPT_STEP (1)
#define SENSOR_MAX_AGAIN (0xff)

	HI_U32 ret_Again;
	HI_U16 BLC_top = SENSOR_BLC_TOP_VALUE, BLC_bot = SENSOR_BLC_BOT_VALUE, BLC_reg, Again_step = SENSOR_AGAIN_ADAPT_STEP, gain_index;
	static HI_U16 MaxAgain = SENSOR_MAX_AGAIN;
	SENSOR_I2C_READ(0X3911,&BLC_reg);

	gain_index = gainmap_gain2index(MaxAgain);

	if(BLC_reg > BLC_top){//>0x58
		if(gain_index>0){
			//limit max Again by step
			gain_index -= SENSOR_AGAIN_ADAPT_STEP;
		}
		MaxAgain = sensor_gain_map[gain_index];
	}else if(BLC_reg < BLC_bot){//<0x45
		//release Again limit by step
		gain_index += SENSOR_AGAIN_ADAPT_STEP;
		if(gain_index > sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0])-1){
			gain_index = sizeof(sensor_gain_map)/sizeof(sensor_gain_map[0]) - 1;
		}
		MaxAgain = sensor_gain_map[gain_index];
	}else{//0x45 < BLC_reg < 0x58
		//do nothing
	}
	ret_Again = Again > MaxAgain ? MaxAgain : Again;
	//printf("limit gain:ret_Again=%d;Again=%d;MaxAgain=%d\n", ret_Again, Again, MaxAgain);
	return ret_Again;
}


static  HI_U32   anolog_gain_table[64] = 
{ 

    1024,	1085,	1146,	1208,	1269,	1321,	1382,	1444,	1505,	1566,	1628,	1690,	1751,	1802,	1864,	1925,                       
    2048,	2170,	2292,	2416,	2538,	2642,	2764,	2888,	3010,	3132,	3256,	3380,	3502,	3604,	3728,	3850,                       
    4096,	4340,	4584,	4832,	5076,	5284,	5528,	5776,	6020,	6264,	6512,	6760,	7004,	7208,	7456,	7700,                       
    8192,	8680,	9168,	9664,	10152,	10568,	11056,	11552,	12040,	12528,	13024,	13520,	14008,	14416,	14912,	15400
};

static  HI_U32   digital_gain_table[3] = 
{ 
    1024,2048,4096
};

static HI_VOID sc1135_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

    //printf("cmos_again_calc_table u32InTimes %d.\n",u32InTimes);
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;


   
    if (u32InTimes >= anolog_gain_table[63])
    {    
         pstAeSnsGainInfo->u32SnsTimes = anolog_gain_table[63];
         pstAeSnsGainInfo->u32GainDb = 63;
         return ;
    }
    for(i = 1; i < 65; i++)
    {
        if(u32InTimes < anolog_gain_table[i])
        {
           
            pstAeSnsGainInfo->u32SnsTimes = anolog_gain_table[i - 1];
            pstAeSnsGainInfo->u32GainDb = i - 1;
            break;
        }
    }

    return;

}


static HI_VOID sc1135_dgain_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
	
  //  printf("cmos_dgain_calc_table u32InTimes %d.\n",u32InTimes);
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;
    
    if (u32InTimes >= digital_gain_table[2])
    {
         pstAeSnsGainInfo->u32SnsTimes = digital_gain_table[2];
         pstAeSnsGainInfo->u32GainDb = 2;
         return ;
    }
    
    for(i = 1; i < 3; i++)
    {
        if(u32InTimes < digital_gain_table[i])
        {
            pstAeSnsGainInfo->u32SnsTimes = digital_gain_table[i - 1];
            pstAeSnsGainInfo->u32GainDb = i - 1;
            break;
        }

    } 
    return;

}


static HI_VOID sc1135_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{

	HI_U32 u32GReg = 0, u32Temp = 1, u32DGReg = 0;//u32Dgain;
    HI_U8 u8AgainHigh, u8AgainLow;
	HI_U8 u8DgainReg;
	HI_U8 u8Temp1=0;

	//printf("******************u32Again %d, u32Dgain %d\n",u32Again, u32Dgain);
	if (u32Again >= 1 && u32Again < 0x10)
		u8AgainHigh = 0x0;
	else if (u32Again >= 0x10 && u32Again < 0x20)
		u8AgainHigh = 0x1;
	else if (u32Again >= 0x20 && u32Again < 0x30)
		u8AgainHigh = 0x3;
	else if (u32Again >= 0x30 && u32Again < 0x40)
		u8AgainHigh = 0x7;
	else 
		u8AgainHigh = 0x0;

    u8AgainLow = u32Again & 0xf;

    if (u32Dgain == 0)
    {
        u32DGReg = 0;
    }
    else if (u32Dgain == 1) 
    {
        u32DGReg = 1;
    }
    else if (u32Dgain == 2)
    {
        u32DGReg = 3;
    }
    else
    {
        u32DGReg = 3;
    }
	//printf("43543******************u32AgainReg %x, u32DGReg %x\n",(u8AgainHigh<<4) | u8AgainLow, u32DGReg);
	#if CMOS_SC1135_ISP_WRITE_SENSOR_ENABLE
	    sc1135_init_regs_info();
	    g_stSnsRegsInfo.astI2cData[2].u32Data =  sc1135_Again_limit((u8AgainHigh<<4) | u8AgainLow);
	    g_stSnsRegsInfo.astI2cData[3].u32Data = (u32DGReg);
	    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
	#else
	    SENSOR_I2C_WRITE(0x3e09, sc1135_Again_limit(u32GReg));
	#endif
//2016.04.11 denoise logic
	#if 1
	
		u8Temp1=sc1135_Again_limit((u8AgainHigh<<4) | u8AgainLow);
		//printf("********0x3e09:0x%x*****\n",u8Temp1);
		
		if(u8Temp1<=0x10)
		{
			SENSOR_I2C_WRITE(0x3630,0xd0);
			SENSOR_I2C_WRITE(0x3631,0x80);
		}
		else if((u8Temp1>0x10)&&(u8Temp1<0x7E))
		{
			SENSOR_I2C_WRITE(0x3630,0x60);
			SENSOR_I2C_WRITE(0x3631,0x8e);
		}
		else
		{
			SENSOR_I2C_WRITE(0x3630,0x60);
			SENSOR_I2C_WRITE(0x3631,0x8c);
		}

	#endif

	return;

}

static HI_S32 sc1135_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;

    pstAwbSnsDft->au16GainOffset[0] = 0x0181;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x0189;

    pstAwbSnsDft->as32WbPara[0] = 126;
    pstAwbSnsDft->as32WbPara[1] = -23;
    pstAwbSnsDft->as32WbPara[2] = -153;
    pstAwbSnsDft->as32WbPara[3] = 187913;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -140857;


    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_VOID sc1135_global_init()
{

   gu8SensorMode = 0;
   
}

void sc1135_reg_init()
{
	 SENSOR_I2C_WRITE(0x3000,0x01);//manualstreamenbale
	 SENSOR_I2C_WRITE(0x3003,0x01);//softreset
	 SENSOR_I2C_WRITE(0x3400,0x53);
	 SENSOR_I2C_WRITE(0x3416,0xc0);
	 SENSOR_I2C_WRITE(0x3d08,0x00);
	 SENSOR_I2C_WRITE(0x3e03,0x03);
	 SENSOR_I2C_WRITE(0x3928,0x00);
	 SENSOR_I2C_WRITE(0x3630,0x58);
	 SENSOR_I2C_WRITE(0x3612,0x00);
	 SENSOR_I2C_WRITE(0x3632,0x41);
	 SENSOR_I2C_WRITE(0x3635,0x00); //20160328
	// SENSOR_I2C_WRITE(0x3500,0x10);
	 SENSOR_I2C_WRITE(0x3620,0x44);
	 SENSOR_I2C_WRITE(0x3633,0x7F);//
	 SENSOR_I2C_WRITE(0x3601,0x1A);//
	 SENSOR_I2C_WRITE(0x3780,0x0b);
	 SENSOR_I2C_WRITE(0x3300,0x33);
	 SENSOR_I2C_WRITE(0x3301,0x38);
	 SENSOR_I2C_WRITE(0x3302,0x30);
	 SENSOR_I2C_WRITE(0x3303,0x80); //20160307B
	 SENSOR_I2C_WRITE(0x3304,0x18);
	 SENSOR_I2C_WRITE(0x3305,0x72);
	 SENSOR_I2C_WRITE(0x331e,0xa0);
	 SENSOR_I2C_WRITE(0x321e,0x00);
	 SENSOR_I2C_WRITE(0x321f,0x0a);
	 SENSOR_I2C_WRITE(0x3216,0x0a);
	 //SENSOR_I2C_WRITE(0x3115,0x0a);
	 SENSOR_I2C_WRITE(0x3332,0x38);
	 SENSOR_I2C_WRITE(0x5054,0x82);
	 SENSOR_I2C_WRITE(0x3622,0x26);
	 SENSOR_I2C_WRITE(0x3907,0x02);
	 SENSOR_I2C_WRITE(0x3908,0x00);
	 SENSOR_I2C_WRITE(0x3601,0x18);
	 SENSOR_I2C_WRITE(0x3315,0x44);
	 SENSOR_I2C_WRITE(0x3308,0x40);
	 SENSOR_I2C_WRITE(0x3223,0x22);//vysncmode[5]
	 SENSOR_I2C_WRITE(0x3e0e,0x50);
	 /*DPC*/
	// SENSOR_I2C_WRITE(0x3101,0x9b);
	// SENSOR_I2C_WRITE(0x3114,0x03);
	 //SENSOR_I2C_WRITE(0x3115,0xd1);
	 SENSOR_I2C_WRITE(0x3211,0x60);
	 SENSOR_I2C_WRITE(0x5780,0xff);
	 SENSOR_I2C_WRITE(0x5781,0x04); //20160328
	 SENSOR_I2C_WRITE(0x5785,0x0A); //20160328
	 SENSOR_I2C_WRITE(0x5000,0x66);   
	 SENSOR_I2C_WRITE(0x3e0f,0x90);  
	 SENSOR_I2C_WRITE(0x3631,0x80);
	 SENSOR_I2C_WRITE(0x3310,0x83);
	 SENSOR_I2C_WRITE(0x3336,0x01);
	SENSOR_I2C_WRITE(0x3337,0x00);
	SENSOR_I2C_WRITE(0x3338,0x03);
	 SENSOR_I2C_WRITE(0x3339,0xe8);
	 SENSOR_I2C_WRITE(0x3335,0x06);//
	SENSOR_I2C_WRITE(0x3880,0x00); 

#if 1 //960P
			

	/*27Minput54Moutputpixelclockfrequency*/
	SENSOR_I2C_WRITE(0x3010,0x07);
	SENSOR_I2C_WRITE(0x3011,0x46);
	SENSOR_I2C_WRITE(0x3004,0x04);
	SENSOR_I2C_WRITE(0x3610,0x2b);

	/*configFramelengthandwidth*/
	SENSOR_I2C_WRITE(0x320c,0x07); //1800
	SENSOR_I2C_WRITE(0x320d,0x08); //1000
	SENSOR_I2C_WRITE(0x320e,0x03);
	SENSOR_I2C_WRITE(0x320f,0xe8);

	/*configOutputwindowposition*/
	SENSOR_I2C_WRITE(0x3210,0x00);
	SENSOR_I2C_WRITE(0x3211,0x60);
	SENSOR_I2C_WRITE(0x3212,0x00);
	SENSOR_I2C_WRITE(0x3213,0x04);//if the vlaue is 0x05 ,the picture is red

	/*configOutputimagesize*/
	SENSOR_I2C_WRITE(0x3208,0x05);
	SENSOR_I2C_WRITE(0x3209,0x00);
	SENSOR_I2C_WRITE(0x320a,0x03);
	SENSOR_I2C_WRITE(0x320b,0xc0);

	/*configFramestartphysicalposition*/
	SENSOR_I2C_WRITE(0x3202,0x00);
	SENSOR_I2C_WRITE(0x3203,0x08);
	SENSOR_I2C_WRITE(0x3206,0x03);
	SENSOR_I2C_WRITE(0x3207,0xcf);

	/*powerconsumptionreductionforXiongMai*/
	SENSOR_I2C_WRITE(0x3330,0x0d);
	SENSOR_I2C_WRITE(0x3320,0x06);
	SENSOR_I2C_WRITE(0x3321,0xe8);
	SENSOR_I2C_WRITE(0x3322,0x01);
	SENSOR_I2C_WRITE(0x3323,0xc0);
	SENSOR_I2C_WRITE(0x3600,0x54);
			
	
#endif 
		printf("SC1135 sensor 960P30fps 63MHz init success!\n\n");
	

}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 sc1135_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{	
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = sc1135_reg_init;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sc1135_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = sc1135_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = sc1135_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = sc1135_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = sc1135_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = sc1135_get_sensor_max_resolution;


    return 0;
}

HI_S32 sc1135_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{

    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = sc1135_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = sc1135_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= sc1135_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = sc1135_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = sc1135_gains_update;
  	 pstExpFuncs->pfn_cmos_again_calc_table  = sc1135_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table	= sc1135_dgain_calc_table;

    return 0;
}

HI_S32 sc1135_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = sc1135_get_awb_default;

    return 0;
}

int sc1135_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    sc1135_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(SC1135_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    sc1135_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, SC1135_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    sc1135_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, SC1135_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int sc1135_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(SC1135_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, SC1135_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, SC1135_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }  
    return 0;	
}

void SC1135_init(SENSOR_SC1135_DO_I2CRD do_i2c_read, SENSOR_SC1135_DO_I2CWR do_i2c_write)
{// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	sc1135_reg_init();

	sc1135_sensor_register_callback();

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
	printf("SC1135 sensor 720P30fps init success!\n");

return 0;

}













































#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // __IMX104_CMOS_H_

