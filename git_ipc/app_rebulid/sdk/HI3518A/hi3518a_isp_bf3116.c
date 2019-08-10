#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_BF3116_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_BF3116_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__BF3116_CMOS_H_)
#define __BF3116_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_sns_ctrl.h"
#include "hi_comm_isp.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define BF3116_ID 3116

/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance                       */
/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c                                         */
#define CMOS_BF3116_ISP_WRITE_SENSOR_ENABLE (1)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/



static const unsigned int sensor_i2c_addr = 0xdc;
static unsigned int sensor_addr_byte = 0x1;
static unsigned int sensor_data_byte = 0x1;


HI_U32 gu32FullLinesStd = 750;
HI_U8 gu8SensorMode = 0;



#if CMOS_BF3116_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif

static AWB_CCM_S g_stAwbCcm =
{
   /*  4850,
       {
         0x0139, 0x8022, 0x8017,
         0x8043, 0x0161, 0x801e,
         0x8008, 0x809f, 0x01a7
       }, 
       */
        4850,  //yang
       {
         0x0, 0x8022, 0x8017,
         0x8043, 0x0, 0x801e,
         0x8008, 0x809f, 0x01a0
       }, 
       
       3160,
       {
         0x016b,0x8022,0x8049,
         0x804e,0x01a0,0x8052,
         0x0003,0x8086,0x0182
       },
       
       2470,
       {
         0x0104, 0x0034, 0x8039,
         0x8064, 0x014b, 0x0018,
         0x804a, 0x8161, 0x02ab
       }
 
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */    
    {0x80,0x80,0x6C,0x48,0x44,0x40,0x3C,0x38}
};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,

    /* sharpen_alt_d */
   {50,50,45,35,30,30,20,15}, 
    
    /* sharpen_alt_ud */
   {45,45,35,30,25,25,15,10},  

    //snr_thresh
    {8,12,18,26,36,46,56,66},

    /* demosaic_lum_thresh */
    {0x50,0x50,0x40,0x40,0x30,0x30,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x35},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37}
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut */
    {0,0,0,0,10,16,20,23,25,27,29,31,32,33,34,35,36,37,38,38,39,40,40,41,41,42,42,43,43,
44,44,45,45,45,46,46,46,47,47,47,48,48,48,49,49,49,49,50,50,50,50,51,51,51,51,52,
52,52,52,52,53,53,53,53,53,54,54,54,54,54,55,55,55,55,55,55,56,56,56,56,56,56,56,
57,57,57,57,57,57,57,58,58,58,58,58,58,58,58,59,59,59,59,59,59,59,59,59,60,60,60,
60,60,60,60,60,60,61,61,61,61,61,61,61,61,61,61,62,62},

    /* demosaic_weight_lut */
    {0,10,16,20,23,25,27,29,31,32,33,34,35,36,37,38,38,39,40,40,41,41,42,42,43,43,
44,44,45,45,45,46,46,46,47,47,47,48,48,48,49,49,49,49,50,50,50,50,51,51,51,51,52,
52,52,52,52,53,53,53,53,53,54,54,54,54,54,55,55,55,55,55,55,56,56,56,56,56,56,56,
57,57,57,57,57,57,57,58,58,58,58,58,58,58,58,59,59,59,59,59,59,59,59,59,60,60,60,
60,60,60,60,60,60,61,61,61,61,61,61,61,61,61,61,62,62,62,62,62}
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xc9,

    /*aa_slope*/
    0xae,

    /*va_slope*/
    0xaa,

    /*uu_slope*/
    0xee,

    /*sat_slope*/
    93,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0,

    /*aa_thresh*/
    0,

    /*va_thresh*/
    0,

    /*uu_thresh*/
    6,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3
};





static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,
    
#if 0    
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

static HI_U32 Again_table[80]=
{
1024,1104,1143, 1217,1291,1387,1443,1515,1586,1666,1721,1805,1865,1943,1992,2064,2097,2222,2309,
2493,2618,2817,2901,3050,3173,3349,3461,3630,3727,3892,3974,4123,4136,4430,4590,4856,5039,5305,
5497,5772,5974,6258,6427,6788,7009,7300,7494,7769,8018,8618,8926,9506,9901,10529, 10835,11462,
11910,12483,12806,13355,13766,14360,14704,15247,15867,17026,17628,18806,19628,20943,21660,22905,23787,
24932,25561,26661,27429,28511,29090,30172
};

static HI_VOID bf3116_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
 
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;
   
    if (u32InTimes >= Again_table[79])
    {
         pstAeSnsGainInfo->u32SnsTimes = Again_table[79];
         pstAeSnsGainInfo->u32GainDb = 79;
         return ;
    }
    
    for(i = 1; i < 80; i++)
    {
        if(u32InTimes < Again_table[i])
        {
            pstAeSnsGainInfo->u32SnsTimes = Again_table[i - 1];
            pstAeSnsGainInfo->u32GainDb = i - 1;
            break;
        }

    }
          
    return;

}




HI_U32 bf3116_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

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

    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));

    return 0;
}

HI_U32 bf3116_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
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
        pstBlackLevel->au16BlackLevel[i] = 131;
    }

    return 0;    
}

HI_VOID bf3116_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
	     SENSOR_I2C_WRITE(0xfe,0x00);			//5 fps  //0x1194   4500-750 + 4 = 0xEAA	     
	     SENSOR_I2C_WRITE(0x0b, 0xaa);
	     SENSOR_I2C_WRITE(0x0c,0x0e);
	     SENSOR_I2C_WRITE(0xfe,0x01);
	     SENSOR_I2C_WRITE(0x10,0x01);   
	     SENSOR_I2C_WRITE(0x0e, 0x11);
         SENSOR_I2C_WRITE(0x0f, 0x92);
    }
    else /* setup for ISP 'normal mode' */
    {
	     SENSOR_I2C_WRITE(0xfe,0x00);
	     SENSOR_I2C_WRITE(0x0b, 0x04);
	     SENSOR_I2C_WRITE(0x0c,0x00);	     
	     SENSOR_I2C_WRITE(0xfe,0x01);
    }

    return;
}

HI_VOID bf3116_set_wdr_mode(HI_U8 u8Mode)
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

static HI_S32 bf3116_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
    
    pstAeSnsDft->u32LinesPer500ms = 750*30/2;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;

    gu32FullLinesStd = 750;
	pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
		

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = 748;
    pstAeSnsDft->u32MinIntTime = 2;    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 6;    
    pstAeSnsDft->u32MaxAgain = 30172;  /* 18db / 6db = 3 */
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = 30172;
    pstAeSnsDft->u32MinAgainTarget = 1024;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 1;    
    pstAeSnsDft->u32MaxDgain = 1;  /* 8 / 0.03125 = 256; 22.5 / 0.03125 = 720 */
    pstAeSnsDft->u32MinDgain = 1;
    pstAeSnsDft->u32MaxDgainTarget = 1;
    pstAeSnsDft->u32MinDgainTarget = 1;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}



static HI_S32 bf3116_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID bf3116_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch(u8Fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 750;
            pstAeSnsDft->u32MaxIntTime = 748;
            pstAeSnsDft->u32LinesPer500ms = 750 * 30 / 2;


	     SENSOR_I2C_WRITE(0xfe,0x00);
	     SENSOR_I2C_WRITE(0x0b, 0x04);
	     SENSOR_I2C_WRITE(0x0c,0x00);
	     SENSOR_I2C_WRITE(0xfe,0x01);
	     
	     
		printf("set frame rate = 30fps\n");
        break;
        
        case 25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 900;
            pstAeSnsDft->u32MaxIntTime = 898;
            pstAeSnsDft->u32LinesPer500ms = 750 * 25 / 2;

	     //0x0c --- high 8 bits	//900  900 - 750 +4
	     SENSOR_I2C_WRITE(0xfe,0x00);
	     SENSOR_I2C_WRITE(0x0b, 0x9A);
	     SENSOR_I2C_WRITE(0x0c,0x00);
	     SENSOR_I2C_WRITE(0xfe,0x01);

	     printf("set frame rate = 25fps\n");

        break;
        
        default:
        break;
    }
	pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID bf3116_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

     HI_U16 u16VblankLine = u16FullLines -750 + 4;
     SENSOR_I2C_WRITE(0xfe,0x00);
     SENSOR_I2C_WRITE(0x0b, u16VblankLine & 0x000000ff);
     SENSOR_I2C_WRITE(0x0c,( u16VblankLine >> 8) & 0x000000ff);
     SENSOR_I2C_WRITE(0xfe,0x01);
     pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;
    
    return;
}

static HI_VOID bf3116_init_regs_info(HI_VOID)
{
#if CMOS_BF3116_ISP_WRITE_SENSOR_ENABLE
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
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x0e;		// integration time (high 8-bit)
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x0f;		// integration time (low 8-bit)
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x10;		//global gain
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_TRUE;

        bInit = HI_TRUE;
    }
#endif
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID bf3116_inttime_update(HI_U32 u32IntTime)
{
#if CMOS_BF3116_ISP_WRITE_SENSOR_ENABLE
    bf3116_init_regs_info();
    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime>>8)&0x000000ff;
    g_stSnsRegsInfo.astI2cData[1].u32Data = u32IntTime&0x000000ff;
    
#else
   SENSOR_I2C_WRITE(0x0e, (u32IntTime>>8)&0x000000ff);
   SENSOR_I2C_WRITE(0x0f, u32IntTime&0x000000ff);
#endif
    return;
}

static HI_VOID bf3116_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
#if CMOS_BF3116_ISP_WRITE_SENSOR_ENABLE
    bf3116_init_regs_info();

    g_stSnsRegsInfo.astI2cData[2].u32Data = u32Again;
    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
#else
	SENSOR_I2C_WRITE(0x10, u32Again);
#endif
    return;
}

static HI_S32 bf3116_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4850;

    pstAwbSnsDft->au16GainOffset[0] = 0x0152;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x016f;

    pstAwbSnsDft->as32WbPara[0] = 71;
    pstAwbSnsDft->as32WbPara[1] = 80;
    pstAwbSnsDft->as32WbPara[2] = -104;
    pstAwbSnsDft->as32WbPara[3] = 188199;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -137127;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

static HI_VOID bf3116_global_init()
{
	gu8SensorMode = 0;
}


void bf3116_reg_init()
{
	//[Register Begin]

	SENSOR_I2C_WRITE(0xfe,0x80);
	SENSOR_I2C_WRITE(0xfe,0x00);		//reset all register

	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0x3d,0xff);
	SENSOR_I2C_WRITE(0xe8,0x10);
	SENSOR_I2C_WRITE(0x00,0x60);
	SENSOR_I2C_WRITE(0x02,0x10);
	SENSOR_I2C_WRITE(0xfe,0x01);
	SENSOR_I2C_WRITE(0x04,0x4f);
	SENSOR_I2C_WRITE(0x00,0x38);
	SENSOR_I2C_WRITE(0x10,0x10);
	SENSOR_I2C_WRITE(0x0e,0x01);
	SENSOR_I2C_WRITE(0x0f,0xcb);
	SENSOR_I2C_WRITE(0x09,0x02);
	SENSOR_I2C_WRITE(0x1b,0x10);
	SENSOR_I2C_WRITE(0x42,0x50);
	SENSOR_I2C_WRITE(0x43,0x20);
	SENSOR_I2C_WRITE(0x44,0x00);
	SENSOR_I2C_WRITE(0x45,0x08);
	SENSOR_I2C_WRITE(0x46,0x00);
	SENSOR_I2C_WRITE(0x47,0xd8);
	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0xe1,0x03); //input 27MHz.
	SENSOR_I2C_WRITE(0xe2,0x2b);
	SENSOR_I2C_WRITE(0xe3,0x15); 
	SENSOR_I2C_WRITE(0xe4,0x58);
	SENSOR_I2C_WRITE(0xe5,0x41);
	SENSOR_I2C_WRITE(0xe7,0x00);

	SENSOR_I2C_WRITE(0xe9,0x20);	// clk,vsync,hsync,data current drive

	SENSOR_I2C_WRITE(0xec,0x12);
	SENSOR_I2C_WRITE(0xb2,0x91);
	SENSOR_I2C_WRITE(0xb0,0x00);
	SENSOR_I2C_WRITE(0xb1,0x00);
	SENSOR_I2C_WRITE(0xb2,0x90);
	SENSOR_I2C_WRITE(0xb3,0x00);
	//SENSOR_I2C_WRITE(0xf1,0x01); //
	SENSOR_I2C_WRITE(0xf1,0x01); //
	SENSOR_I2C_WRITE(0x6f,0xd2);
	SENSOR_I2C_WRITE(0x72,0x00);
	SENSOR_I2C_WRITE(0x73,0x00);
	SENSOR_I2C_WRITE(0xe8,0x00);
	SENSOR_I2C_WRITE(0xea,0x11);
	SENSOR_I2C_WRITE(0x82,0x27);
	SENSOR_I2C_WRITE(0x80,0x0c); 
	SENSOR_I2C_WRITE(0x81,0x8e); 
	SENSOR_I2C_WRITE(0x85,0x06); 

	SENSOR_I2C_WRITE(0xfe,0x01);		//page 1 selection

	SENSOR_I2C_WRITE(0xf1,0x80);		//disable all the isp function inside the sensor



	//[Register END]


	    printf("BYD BF3116 sensor 720P30fps reg init success!\n");
	    

}


/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 bf3116_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = bf3116_reg_init;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = bf3116_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = bf3116_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = bf3116_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = bf3116_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = bf3116_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = bf3116_get_sensor_max_resolution;

    return 0;
}

HI_S32 bf3116_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = bf3116_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = bf3116_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= bf3116_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = bf3116_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = bf3116_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = bf3116_again_calc_table;

    return 0;
}

HI_S32 bf3116_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = bf3116_get_awb_default;

    return 0;
}

int bf3116_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    bf3116_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(BF3116_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    bf3116_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, BF3116_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    bf3116_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, BF3116_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int bf3116_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(BF3116_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, BF3116_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, BF3116_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

void BF3116_init(SENSOR_BF3116_DO_I2CRD do_i2c_read, SENSOR_BF3116_DO_I2CWR do_i2c_write)
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	bf3116_reg_init();

	bf3116_sensor_register_callback();

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
	printf("BYD BF3116 sensor 720P30fps init success!\n");


}





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // __IMX104_CMOS_H_

















