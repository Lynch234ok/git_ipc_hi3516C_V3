

#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"


static  SENSOR_SOIH22_DO_I2CRD sensor_i2c_read = NULL;
static  SENSOR_SOIH22_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)


#if !defined(__SOIH22_CMOS_H_)
#define __SOIH22_CMOS_H_


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

#define SOIH22_ID 22

#define CMOS_SOIH22_ISP_WRITE_SENSOR_ENABLE (1)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/


 static const unsigned int sensor_i2c_addr = 0x60;
 static unsigned int sensor_addr_byte = 1;
 static unsigned int sensor_data_byte = 1;
 

static HI_U32 gu32FullLinesStd = 767;
static HI_U32 gu32FullLines = 767;

static HI_U8 gu8SensorMode = 0;



 
#if CMOS_SOIH22_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif


static AWB_CCM_S g_stAwbCcm =
{
      5048,
      {   0x17a, 0x806f, 0x8021,
          0x8042, 0x1a8, 0x8066,
          0x8001, 0x812a, 0x236
      },
      3193,
      {   0x1ee, 0x8017, 0x80d6,
          0x8053, 0x1d2, 0x807e,
          0x8023, 0x8176, 0x29a
      },
      2480,
      {   0x1aa, 0x806e, 0x803c,
          0x807d, 0x190, 0x8013,
          0x8098, 0x826c, 0x404
      }
};


static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */
  //  {0x80,0x80,0x80,0x80,0x68,0x48,0x35,0x30}
    {0x80,  0x80,  0x80,  0x74,  0x46,  0x32,  0x2d,   0x20}
};


static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
    
    /* sharpen_alt_d */
   // {0xbc,0x6d,0x55,0x60,0x16,0x8e,0xd8,0x01},
	{0x98,  0x88,  0x80,  0x70,  0x80,  0x8e,  0xef,   0x01},
        
    /* sharpen_alt_ud */
    //{0x8e,0x46,0x3e,0x40,0x2f,0x3d,0x1e,0x20},
	{0xa8,   0x90,  0x60,   0x50,   0x78,   0x50,   0x20,   0x01},
        
    /* snr_thresh */
    //{0x1d,0x1e,0x1f,0x30,0x30,0x30,0x30,0x30},
	{0x0a,  0x10,  0x20,  0x2a,  0x2d,  0x3c,   0x41,  0x50},
        
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
   // 0x9f,
    0xc0,

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



static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,
    
#if 1    
	//new data  yang
	{  80,115,149,181,212,242,274,310,346,386,431,479,530,583,638,693,
		748,804,861,918,976,1035,1094,1149,1203,1253,1302,1347,1392,1432,1472,1507,
		1542,1582,1622,1659,1696,1730,1765,1797,1829,1859,1890,1918,1947,1976,2005,2032,
		2059,2086,2112,2138,2163,2187,2211,2235,2259,2282,2304,2326,2347,2366,2386,2406,
		2425,2442,2458,2474,2489,2504,2519,2534,2548,2562,2577,2591,2606,2620,2634,2649,
		2663,2678,2692,2706,2719,2733,2746,2760,2774,2787,2801,2814,2828,2842,2855,2869,
		2882,2896,2910,2923,2937,2950,2963,2976,2989,3002,3014,3027,3040,3053,3066,3078,
		3091,3102,3114,3125,3136,3147,3158,3170,3181,3190,3200,3210,3219,3229,3238,3248,
		3258,3267,3277,3286,3296,3306,3315,3325,3334,3344,3354,3363,3373,3382,3392,3402,
		3411,3419,3427,3435,3443,3451,3459,3467,3474,3480,3486,3493,3499,3506,3512,3518,
		3525,3531,3538,3544,3550,3557,3563,3570,3576,3582,3589,3595,3602,3606,3611,3616,
  		3621,3626,3630,3635,3640,3645,3650,3654,3659,3664,3669,3674,3678,3683,3688,3693,
  		3698,3702,3707,3712,3717,3722,3726,3731,3736,3741,3746,3750,3755,3760,3765,3770,
  		3774,3779,3784,3789,3794,3798,3803,3808,3813,3818,3822,3827,3832,3837,3842,3846,
  		3851,3856,3861,3866,3870,3875,3880,3885,3890,3894,3899,3904,3909,3914,3918,3923,
  		3928,3933,3938,3942,3947,3952,3955,3958,3962,3965,3968,3971,3974,3978,3981,3984,3987,}






  /*  {0x0,0xd,0x20,0x3b,0x5b,0x82,0xaf,0xcb,0xf7,0x12f,0x16f,0x1b4,0x1fb,0x23f,0x27f,0x2ba,
		0x2f0,0x322,0x351,0x37c,0x3a5,0x3cc,0x3f1,0x415,0x438,0x45c,0x47f,0x4a3,0x4c8,0x4ee,0x514,0x53b,
		0x562,0x58a,0x5b1,0x5d9,0x601,0x629,0x651,0x679,0x6a0,0x6c7,0x6ee,0x714,0x73a,0x75f,0x783,0x7a7,
		0x7ca,0x7eb,0x80c,0x82c,0x84b,0x869,0x886,0x8a2,0x8bc,0x8d6,0x8ee,0x904,0x91a,0x92e,0x940,0x951,
		0x961,0x96f,0x97b,0x986,0x98f,0x99d,0x9aa,0x9b8,0x9c6,0x9d4,0x9e2,0x9f0,0x9fe,0xa0b,0xa19,0xa27,
		0xa35,0xa42,0xa50,0xa5d,0xa6b,0xa78,0xa86,0xa93,0xaa0,0xaad,0xab9,0xac6,0xad3,0xadf,0xaeb,0xaf7,
		0xb03,0xb0f,0xb1a,0xb26,0xb31,0xb3b,0xb46,0xb50,0xb5b,0xb65,0xb6e,0xb78,0xb81,0xb8a,0xb93,0xb9b,
		0xba3,0xbab,0xbb3,0xbba,0xbc1,0xbc8,0xbce,0xbd4,0xbda,0xbdf,0xbe4,0xbe9,0xbee,0xbf2,0xbf6,0xbf9,
		0xbfc,0xbff,0xc03,0xc07,0xc0b,0xc0f,0xc13,0xc17,0xc1a,0xc1e,0xc22,0xc26,0xc2a,0xc2e,0xc32,0xc36,
		0xc3a,0xc3e,0xc42,0xc46,0xc4a,0xc4e,0xc53,0xc57,0xc5c,0xc61,0xc66,0xc6b,0xc70,0xc75,0xc7a,0xc80,
		0xc86,0xc8c,0xc92,0xc98,0xc9f,0xca5,0xcac,0xcb3,0xcba,0xcc2,0xcc9,0xcd0,0xcd7,0xcde,0xce5,0xcec,
		0xcf2,0xcf8,0xcfe,0xd04,0xd09,0xd0e,0xd12,0xd16,0xd1a,0xd1c,0xd1f,0xd23,0xd27,0xd2a,0xd2e,0xd31,
		0xd35,0xd38,0xd3c,0xd3f,0xd43,0xd46,0xd49,0xd4d,0xd50,0xd54,0xd58,0xd5c,0xd60,0xd64,0xd68,0xd6d,
		0xd72,0xd77,0xd7c,0xd81,0xd87,0xd8d,0xd93,0xd9a,0xda1,0xda9,0xdb0,0xdb9,0xdc1,0xdca,0xdd4,0xdde,
		0xde8,0xdf3,0xdfe,0xe0a,0xe17,0xe24,0xe32,0xe40,0xe4f,0xe5e,0xe6f,0xe7f,0xe91,0xea3,0xeb6,0xeca,
		0xedf,0xef4,0xf0a,0xf20,0xf36,0xf4c,0xf62,0xf78,0xf8c,0xfa0,0xfb3,0xfc4,0xfd4,0xfe2,0xfee,0xff7,
		0xfff}                                  */
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



HI_U32 soih22_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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



HI_U32 soih22_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{

    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    pstBlackLevel->au16BlackLevel[0] = 0x24;
	pstBlackLevel->au16BlackLevel[1] = 0x1c;
	pstBlackLevel->au16BlackLevel[2] = 0x1c;
	pstBlackLevel->au16BlackLevel[3] = 0x24;

    return 0;    
}




HI_VOID soih22_set_pixel_detect(HI_BOOL bEnable)
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


HI_VOID soih22_set_wdr_mode(HI_U8 u8Mode)
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

static HI_S32 soih22_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = 767;
    
    pstAeSnsDft->u32LinesPer500ms = 767*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;
    /* 1280 * 736 */
    /* 1296 * 816 */ /* reg: 0x22/23/24/25 */    
    //gu8Fps = 30;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = 1767;
    pstAeSnsDft->u32MinIntTime = 8;
    
    pstAeSnsDft->au8HistThresh[0] = 0x10;
    pstAeSnsDft->au8HistThresh[1] = 0x40;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x70;
    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    /* 1(1+1/16), 1(1+2/16), ... , 2(1+1/16), ... , 16(1+15/16) */
    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_DB;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 6;
    pstAeSnsDft->u32MaxAgain = 6;  /* 1, 2, 4, 8, 16, 32, 64 (0~24db, unit is 6db) */
    pstAeSnsDft->u32MinAgain = 0;
    pstAeSnsDft->u32MaxAgainTarget = 4;
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

static HI_S32 soih22_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID soih22_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch(u8Fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 767;
            //pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd;
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 30 / 2;
            SENSOR_I2C_WRITE(0x22, gu32FullLinesStd & 0xff);
            SENSOR_I2C_WRITE(0x23, (gu32FullLinesStd >> 8));
        break;
        
        case 25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = (30 * 767) / 25;
            //pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd;
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 25 / 2;
            SENSOR_I2C_WRITE(0x22, gu32FullLinesStd & 0xff);
            SENSOR_I2C_WRITE(0x23, (gu32FullLinesStd >> 8));
        break;
        
        default:
        break;
    }

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID soih22_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    SENSOR_I2C_WRITE(0x22, u16FullLines&0xff);
    SENSOR_I2C_WRITE(0x23, (u16FullLines >> 8));

    //pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;
    gu32FullLines = u16FullLines;

    return;
}







static HI_VOID soih22_init_regs_info(HI_VOID)
{
#if CMOS_SOIH22_ISP_WRITE_SENSOR_ENABLE
    HI_S32 i;
    static HI_BOOL bInit = HI_FALSE;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u32RegNum = 4;
        for (i=0; i<4; i++)
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
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_TRUE;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x0d;
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_TRUE;

        bInit = HI_TRUE;
    }
#endif
    return;
}




/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID soih22_inttime_update(HI_U32 u32IntTime)
{
#if CMOS_SOIH22_ISP_WRITE_SENSOR_ENABLE
    soih22_init_regs_info();
    g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime & 0xFF;
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime >> 8) & 0xFF;
#else 
    //refresh the sensor setting every frame to avoid defect pixel error
    SENSOR_I2C_WRITE(0x01, u32IntTime&0xFF);
    SENSOR_I2C_WRITE(0x02, (u32IntTime>>8)&0xFF);  
#endif
    return;
}

static HI_VOID soih22_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    HI_U8 u8High, u8Low, u8Dg;
    switch (u32Again)
    {
        case 0 :    /* 0db, 1 multiplies */
            u8High = 0x00;
            u8Dg   = 0x00;
            break;
        case 1 :    /* 6db, 2 multiplies */
            u8High = 0x10;
            u8Dg   = 0x00;
            break;
        case 2 :    /* 12db, 4 multiplies */
            u8High = 0x30;
            u8Dg   = 0x00;
            break;
        case 3 :    /* 18db, 8 multiplies */
            u8High = 0x70;
            u8Dg   = 0x00;
            break;
        case 4 :    /* 24db, 16 multiplies */
            u8High = 0xf0;
            u8Dg   = 0x00;
            break;
        case 5 :    /* 30db, 32 multiplies */
            u8High = 0xf0;
            u8Dg   = 0x00;
            break;
        case 6 :    /* 36db, 64 multiplies */
            u8High = 0xf0;
            u8Dg   = 0x00;
            break;
        default:
            u8High = 0x00;
            u8Dg   = 0x00;
            break;
    }

    u8Low = (u32Dgain - 16) & 0xf;

#if CMOS_SOIH22_ISP_WRITE_SENSOR_ENABLE
    soih22_init_regs_info();
    g_stSnsRegsInfo.astI2cData[2].u32Data = (u8High | u8Low);
    g_stSnsRegsInfo.astI2cData[3].u32Data = u8Dg;
    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
	//soih22_reset_OB_reg(u32Again);
#else
    SENSOR_I2C_WRITE(0x00, (u8High | u8Low));
    SENSOR_I2C_WRITE(0x0d, u8Dg);
#endif

    return;
}



static HI_S32 soih22_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    
    pstAwbSnsDft->u16WbRefTemp = 5048;

    pstAwbSnsDft->au16GainOffset[0] = 0x162;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0X18c;

    pstAwbSnsDft->as32WbPara[0] = 129;
    pstAwbSnsDft->as32WbPara[1] = -29;
    pstAwbSnsDft->as32WbPara[2] = -155;
    pstAwbSnsDft->as32WbPara[3] = 198120;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -151168;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

static HI_VOID soih22_global_init()
{
	gu8SensorMode = 0;
}

void soih22_reg_init()
{
    SENSOR_I2C_WRITE(0x0e, 0x1D);
    SENSOR_I2C_WRITE(0x0f, 0x0B);
    SENSOR_I2C_WRITE(0x10, 0x26);
    SENSOR_I2C_WRITE(0x11, 0x80);
    SENSOR_I2C_WRITE(0x1B, 0x4F);
    SENSOR_I2C_WRITE(0x1D, 0xFF);

    SENSOR_I2C_WRITE(0x1E, 0x9F);
    SENSOR_I2C_WRITE(0x20, 0x72);
    SENSOR_I2C_WRITE(0x21, 0x06);
    SENSOR_I2C_WRITE(0x22, 0xFF);
    SENSOR_I2C_WRITE(0x23, 0x02);
    SENSOR_I2C_WRITE(0x24, 0x00);
    SENSOR_I2C_WRITE(0x25, 0xE0);
    SENSOR_I2C_WRITE(0x26, 0x25);
    SENSOR_I2C_WRITE(0x27, 0xE9);
    SENSOR_I2C_WRITE(0x28, 0x0D);
    SENSOR_I2C_WRITE(0x29, 0x00);
    SENSOR_I2C_WRITE(0x2C, 0x00);
    SENSOR_I2C_WRITE(0x2D, 0x08);
    SENSOR_I2C_WRITE(0x2E, 0xC4);
    SENSOR_I2C_WRITE(0x2F, 0x20);
    SENSOR_I2C_WRITE(0x6C, 0x90);
    SENSOR_I2C_WRITE(0x2A, 0xD4);
    SENSOR_I2C_WRITE(0x30, 0x90);
    SENSOR_I2C_WRITE(0x31, 0x10);
    SENSOR_I2C_WRITE(0x32, 0x10);
    SENSOR_I2C_WRITE(0x33, 0x10);
    SENSOR_I2C_WRITE(0x34, 0x32);
    SENSOR_I2C_WRITE(0x14, 0x80);
    SENSOR_I2C_WRITE(0x18, 0xD5);
    SENSOR_I2C_WRITE(0x19, 0x10);

    SENSOR_I2C_WRITE(0x0d, 0x00);
    SENSOR_I2C_WRITE(0x1f, 0x00);

    SENSOR_I2C_WRITE(0x13, 0x87);
    SENSOR_I2C_WRITE(0x4A, 0x03);
    SENSOR_I2C_WRITE(0x49, 0x10); 

	SENSOR_I2C_WRITE(0x69, 0x32);
	//for OB calc
	/*SENSOR_I2C_WRITE(0x1a, 0x08);
	SENSOR_I2C_WRITE(0x02, 0x00);
	SENSOR_I2C_WRITE(0x01, 0x01);
	SENSOR_I2C_WRITE(0x04, 0x00);
	SENSOR_I2C_WRITE(0x03, 0x16);
	SENSOR_I2C_WRITE(0x69, 0x32);*/
    
    return ;
}




/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 soih22_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = soih22_reg_init;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = soih22_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = soih22_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = soih22_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = soih22_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = soih22_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = soih22_get_sensor_max_resolution;

    return 0;
}


HI_S32 soih22_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = soih22_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = soih22_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= soih22_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = soih22_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = soih22_gains_update;

    return 0;
}


HI_S32 soih22_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = soih22_get_awb_default;

    return 0;
}




int soih22_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    soih22_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(SOIH22_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    soih22_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, SOIH22_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    soih22_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, SOIH22_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int soih22_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(SOIH22_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, SOIH22_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, SOIH22_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}



void SOIH22_init(SENSOR_SOIH22_DO_I2CRD do_i2c_read, SENSOR_SOIH22_DO_I2CWR do_i2c_write)
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	soih22_reg_init();

	soih22_sensor_register_callback();

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

#endif // __SOIH22_CMOS_H_


