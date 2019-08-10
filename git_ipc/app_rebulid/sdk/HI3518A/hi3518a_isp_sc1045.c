#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"


static  SENSOR_SC1045_DO_I2CRD sensor_i2c_read = NULL;
static  SENSOR_SC1045_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)


#if !defined(__SC1045_CMOS_H_)
#define __SC1045_CMOS_H_

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

#define SC1045_ID 1045

/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance                       */
/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c                                         */
#define CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE (1)

/*change this value to 1 to make the image looks more sharpen*/    
#define CMOS_SC1045_MORE_SHARPEN (0)

/* To change the mode of slow framerate. When the value is 0, add the line numbers to slow framerate.
 * When the value is 1, add the line length to slow framerate. */
#define CMOS_SC1045_SLOW_FRAMERATE_MODE (0)

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static   const unsigned int sensor_i2c_addr=0x60;
static   unsigned int sensor_addr_byte=2;
static    unsigned int sensor_data_byte=1;

HI_U8 gu8SensorMode = 0;

static HI_U32 gu8Fps = 25;
static HI_U32 gu32FullLinesStd = 750;
static HI_U32 gu32FullLines = 750;

#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif

static AWB_CCM_S g_stAwbCcm =
{
    4850,
    {   0x0205, 0x80f2, 0x8013,
        0x8067, 0x01af, 0x8048,
        0x11,   0x81cb, 0x02b9
    },
    3160,
    {   0x01f3, 0x808f, 0x8064,
        0x808e, 0x01ac, 0x801e,
        0x8004, 0x822a, 0x032e
    },
    2470,
    {   0x01f9, 0x8050, 0x80a9,
        0x808d, 0x018e, 0x8001,
        0x8047, 0x830e, 0x0455
    }
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */
    //{0x80,0x80,0x80,0x80,0x68,0x48,0x35,0x30}
    {0x80,0x80,0x74,0x64,0x50,0x44,0x38,0x36}
};


static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,

    /* sharpen_alt_d */
    //{80,78,76,73,70,65,60,54},
    {100,97,94,89,82,75,67,59},  
    
    /* sharpen_alt_ud */
    {145,140,135,131,129,125,120,118},
        
    /* snr_thresh */
    {0x19,0x1e,0x2d,0x32,0x39,0x3f,0x48,0x4b},
        
    /* demosaic_lum_thresh */
    {0x50,0x50,0x40,0x40,0x30,0x30,0x20,0x20},
        
    /* demosaic_np_offset */
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55}

};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut */
    {0, 0,  0,  0,  0,  0,  0,  0,  12, 18, 22, 25, 28, 30, 31, 33,
    34, 35, 36, 37, 38, 39, 40, 40, 41, 42, 42, 43, 43, 44, 44, 45,
    45, 46, 46, 47, 47, 47, 48, 48, 49, 49, 49, 50, 50, 50, 50, 51,
    51, 51, 52, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 54, 54, 55,
    55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57,
    58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60,
    60, 60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 61, 61, 61, 62, 62,
    62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63, 63},

    /* demosaic_weight_lut */
    {0, 12, 18, 22, 25, 28, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 
    40, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 47, 48, 
    48, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 52, 52, 52, 52, 53, 
    53, 53, 53, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 56, 56, 56,
    56, 56, 56, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 59, 
    59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60, 61, 61,
    61, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 62, 62, 62, 
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63}
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xcd,

    /*aa_slope*/
    0xbf,
#if CMOS_SC1045_MORE_SHARPEN
    /*va_slope*/
    0xc1,

    /*uu_slope*/
    0xc8,
#else
    /*va_slope*/
    0xc1,

    /*uu_slope*/
    0xa0,
#endif
    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0x10,

    /*aa_thresh*/
    0x10,

    /*va_thresh*/
    0x10,

    /*uu_thresh*/
    0xa,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3
};

static ISP_CMOS_SHADING_S g_stIspShading =
{

    /*SC1045*/
    /* bvalid */
    1,
    
    /*shading_center_r*/
    0x257, 0x1a8,

    /*shading_center_g*/
    0x24c, 0x1b5,

    /*shading_center_b*/
    0x24f, 0x1b3,

    /*shading_table_r*/
    {0x1000,0x1000,0x1004,0x100b,0x1013,0x101c,0x1025,0x102f,0x1039,0x1044,0x104f,0x1059,
    0x1064,0x1070,0x107b,0x1086,0x1092,0x109e,0x10a9,0x10b5,0x10c1,0x10cd,0x10da,0x10e6,
    0x10f2,0x10ff,0x110b,0x1118,0x1124,0x1131,0x113e,0x114b,0x1158,0x1165,0x1172,0x117f,
    0x118c,0x1199,0x11a7,0x11b3,0x11c2,0x11cf,0x11dd,0x11eb,0x11f9,0x1207,0x1215,0x1223,
    0x1231,0x123f,0x124d,0x125c,0x126a,0x1279,0x1287,0x1296,0x12a5,0x12b4,0x12c3,0x12d2,
    0x12e1,0x12f0,0x12fe,0x130f,0x131e,0x132e,0x133d,0x134d,0x135d,0x136d,0x137d,0x138d,
    0x139d,0x13ae,0x13be,0x13cf,0x13df,0x13f0,0x1401,0x1412,0x1423,0x1434,0x1445,0x1457,
    0x1468,0x147a,0x148b,0x149d,0x14af,0x14c1,0x14d3,0x14e6,0x14f8,0x150b,0x151d,0x1530,
    0x1543,0x1556,0x1569,0x157c,0x1590,0x15a3,0x15b7,0x15cb,0x15df,0x15f3,0x1607,0x161b,
    0x162f,0x1645,0x1659,0x166e,0x1682,0x1699,0x16ae,0x16c4,0x16d9,0x16ef,0x1705,0x171c,
    0x1731,0x1749,0x175f,0x1776,0x178d,0x17a4,0x17bc,0x17d3,0x17e9},

    /*shading_table_g*/
    {0x1000,0x1000,0x1003,0x1009,0x1010,0x1018,0x1021,0x102a,0x1033,0x103d,0x1048,0x1052,
    0x105d,0x1067,0x1072,0x107e,0x1089,0x1094,0x10a0,0x10ac,0x10b8,0x10c4,0x10d0,0x10dc,
    0x10e8,0x10f5,0x1101,0x110e,0x111a,0x1127,0x1134,0x1141,0x114e,0x115b,0x1168,0x1175,
    0x1182,0x1190,0x119d,0x11aa,0x11b8,0x11c6,0x11d3,0x11e1,0x11ef,0x11fd,0x120b,0x1219,
    0x1227,0x1235,0x1243,0x1251,0x1260,0x126e,0x127c,0x128b,0x1299,0x12a8,0x12b7,0x12c5,
    0x12d4,0x12e3,0x12f2,0x1301,0x1310,0x131f,0x132e,0x133d,0x134c,0x135c,0x136b,0x137a,
    0x138a,0x1399,0x13a9,0x13b8,0x13c8,0x13d7,0x13e7,0x13f7,0x1407,0x1416,0x1426,0x1436,
    0x1446,0x1456,0x1466,0x1476,0x1487,0x1497,0x14a7,0x14b7,0x14c8,0x14d8,0x14e9,0x14f9,
    0x150a,0x151a,0x152b,0x153b,0x154c,0x155d,0x156d,0x157e,0x158f,0x15a0,0x15b1,0x15c2,
    0x15d3,0x15e4,0x15f5,0x1606,0x1617,0x1628,0x1639,0x164a,0x165b,0x166d,0x167e,0x168f,
    0x16a1,0x16b2,0x16c3,0x16d5,0x16e6,0x16f8,0x1709,0x171b,0x172a},

    /*shading_table_b*/
    {0x1000,0x1000,0x1001,0x1006,0x100b,0x1011,0x1018,0x1020,0x1028,0x1030,0x1038,0x1041,
    0x104a,0x1053,0x105c,0x1066,0x1070,0x1079,0x1083,0x108d,0x1098,0x10a2,0x10ac,0x10b7,
    0x10c1,0x10cc,0x10d7,0x10e2,0x10ed,0x10f8,0x1103,0x110e,0x111a,0x1125,0x1130,0x113c,
    0x1148,0x1153,0x115f,0x116b,0x1177,0x1183,0x118f,0x119b,0x11a7,0x11b3,0x11bf,0x11cb,
    0x11d8,0x11e4,0x11f1,0x11fd,0x120a,0x1216,0x1223,0x122f,0x123c,0x1249,0x1256,0x1263,
    0x126f,0x127c,0x1289,0x1296,0x12a3,0x12b1,0x12be,0x12cb,0x12d8,0x12e5,0x12f3,0x1300,
    0x130d,0x131b,0x1328,0x1336,0x1343,0x1351,0x135e,0x136c,0x1379,0x1387,0x1395,0x13a2,
    0x13b0,0x13be,0x13cb,0x13d9,0x13e7,0x13f5,0x1403,0x1411,0x141e,0x142c,0x143a,0x1448,
    0x1456,0x1464,0x1472,0x1480,0x148e,0x149c,0x14aa,0x14b8,0x14c6,0x14d4,0x14e2,0x14f0,
    0x14fe,0x150c,0x151b,0x1529,0x1537,0x1545,0x1553,0x1561,0x156f,0x157d,0x158b,0x1599,
    0x15a7,0x15b6,0x15c4,0x15d2,0x15e0,0x15ee,0x15fc,0x160a,0x1614},

    /*shading_off_center_r_g_b*/
    0xd09, 0xc86, 0xca2,


    /*shading_table_nobe_number*/
    129
};

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,
#if 1

#if 0
			{0	,120 ,220 ,310 ,390 ,470 ,540 ,610 ,670 ,730 ,786 ,842 ,894 ,944 ,994 ,1050,	
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
#else
    {0, 54, 106, 158, 209, 259, 308, 356, 403, 450, 495, 540, 584, 628, 670, 713, 754, 795, 
    835, 874, 913, 951, 989,1026,1062,1098,1133,1168,1203,1236,1270,1303, 
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
    4020,4026,4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095, 
    4095
	}
#endif
	
#else 

    {0  ,27  ,60  ,100 ,140 ,178 ,216 ,242 ,276 ,312 ,346 ,380 ,412 ,444 ,476 ,508,
    540 ,572 ,604 ,636 ,667 ,698 ,729 ,760 ,791 ,822 ,853 ,884 ,915 ,945 ,975 ,1005,
    1035,1065,1095,1125,1155,1185,1215,1245,1275,1305,1335,1365,1395,1425,1455,1485,
    1515,1544,1573,1602,1631,1660,1689,1718,1746,1774,1802,1830,1858,1886,1914,1942,
    1970,1998,2026,2054,2082,2110,2136,2162,2186,2220,2244,2268,2292,2316,2340,2362,
    2384,2406,2428,2448,2468,2488,2508,2528,2548,2568,2588,2608,2628,2648,2668,2688,
    2708,2728,2748,2768,2788,2808,2828,2846,2862,2876,2890,2903,2917,2930,2944,2957,
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


static  HI_U32   gain_table[64] = 
{ 
    1024, 1084, 1145, 1205, 1265,  1325,  1385,   1446,  1506,  1566,  1626,  1687,  1747,  1807,  1868,  1927,
    2048, 2169, 2290, 2410, 2592,  2650,  2771,   2891,  3013,  3131,  3252,  3373,  3494,  3615,  3736,  3854,
    4096, 4338, 4579, 4821, 5059,  5300,  5542,   5784,  6025,  6263,  6504,  6746,  6988,  7229,  7471,  7709,
    8192, 8675, 9159, 9642, 10117, 10600, 11083,  11567, 12050, 12526, 13009, 13492, 13976, 14456, 14942, 15417
};

static  HI_U32   digital_gain_table[5] = 
{ 
    1024,2048,4096,8192,16384
};


static  HI_U32 SC1045_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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

    pstDef->stDrc.u8DrcBlack        = 0x00;
    pstDef->stDrc.u8DrcVs           = 0x04;     // variance space
    pstDef->stDrc.u8DrcVi           = 0x08;     // variance intensity
    pstDef->stDrc.u8DrcSm           = 0xa0;     // slope max
    pstDef->stDrc.u16DrcWl          = 0x4ff;    // white level

    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stShading, &g_stIspShading, sizeof(ISP_CMOS_SHADING_S));
    memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));

    return 0;
}

HI_U32 SC1045_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    pstBlackLevel->au16BlackLevel[0] = 258;
    pstBlackLevel->au16BlackLevel[1] = 258;
    pstBlackLevel->au16BlackLevel[2] = 258;
    pstBlackLevel->au16BlackLevel[3] = 258;

    return 0;    
}

HI_VOID SC1045_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        /* 5 fps */
        SENSOR_I2C_WRITE(0x2d, 0xd2); 
        SENSOR_I2C_WRITE(0x2e, 0x0f); 
        
        /* min gain */
        SENSOR_I2C_WRITE(0x0, 0x00);

        /* max exposure time*/
       SENSOR_I2C_WRITE(0x10, 0xf8);
        SENSOR_I2C_WRITE(0x16, 0x12);
    }
    else /* setup for ISP 'normal mode' */
    {
       SENSOR_I2C_WRITE(0x2d, 0x0);
        SENSOR_I2C_WRITE(0x2e, 0x0);
    }

    return;
}

HI_VOID SC1045_set_wdr_mode(HI_U8 u8Mode)
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

static HI_S32 SC1045_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = 900;
    
    pstAeSnsDft->u32LinesPer500ms = 898*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;
   
    gu8Fps = 25;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = 898;
    pstAeSnsDft->u32MinIntTime = 2;
    
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x50;
    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0078125;
    pstAeSnsDft->u32MaxAgain = 16384;  /* 8/0.0078125= 1024 */
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = 16384;
    pstAeSnsDft->u32MinAgainTarget = 1024;


	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 6;   
	pstAeSnsDft->u32MaxDgain = 4*1024;//16*1024;  /* 9db + 18db(8) */
    pstAeSnsDft->u32MinDgain = 1024;
    pstAeSnsDft->u32MaxDgainTarget = 4*1024;//16*1024;
    pstAeSnsDft->u32MinDgainTarget = 1024;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

static HI_S32 SC1045_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID  SC1045_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	
printf("############################################\n");
printf("u8Fps %d.\n", u8Fps);
printf("############################################\n");

//return;

    switch(u8Fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 750;
            pstAeSnsDft->u32MaxIntTime = 748;
            pstAeSnsDft->u32LinesPer500ms = 748* 30 / 2;
            SENSOR_I2C_WRITE(0x320e, 0x02);
           SENSOR_I2C_WRITE(0x320f, 0xee);
        break;
        
        case 25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 900;
            pstAeSnsDft->u32MaxIntTime = 898;
            pstAeSnsDft->u32LinesPer500ms = 898 * 25 / 2;
            SENSOR_I2C_WRITE(0x320e, 0x03);
            SENSOR_I2C_WRITE(0x320f, 0x84);
        break;
        
        default:
        break;
    }

    if(1 == gu8SensorMode)
    {
        pstAeSnsDft->u32MaxIntTime = 46;
    }

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    gu32FullLines = gu32FullLinesStd;
	
    return;
}

static HI_VOID SC1045_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

	SENSOR_I2C_WRITE(0x320e, (u16FullLines>>8)&0xf);
	SENSOR_I2C_WRITE(0x320f, u16FullLines&0xff);
	pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;

         HI_U32 u32Temp = u16FullLines - 0x2ee;
	SENSOR_I2C_WRITE(0x3338, (u16FullLines>>8)&0xf);
	SENSOR_I2C_WRITE(0x3339, u16FullLines&0xff);
	
	SENSOR_I2C_WRITE(0x3336, (u32Temp>>8)&0xf);
	SENSOR_I2C_WRITE(0x3337, u32Temp&0xff);

    return;
}

static HI_VOID SC1045_init_regs_info(HI_VOID)
{

#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
    HI_S32 i;
    static HI_BOOL bInit = HI_FALSE;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u32RegNum = 4;

		
        for (i=0; i<5; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;    //exp
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3e01;
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;    //exp
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3e02;
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;    //Ag
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3e09;
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_TRUE;    //Dg
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3e0f;

        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_TRUE;

        bInit = HI_TRUE;
    }
#endif
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID SC1045_inttime_update(HI_U32 u32IntTime)
{
#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
    HI_U32 u32Temp = 0;
    SC1045_init_regs_info();

    u32Temp = u32IntTime & 0xF;
	u32Temp = u32Temp << 4;
    g_stSnsRegsInfo.astI2cData[1].u32Data = u32Temp;
    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime>>4)&0xFF;

	//printf("................................................... %#x,\n",u32IntTime);
	
#else 
    //refresh the sensor setting every frame to avoid defect pixel error
    HI_U32 u32Temp = 0;
	
    u32Temp = u32IntTime & 0xF ;
	u32Temp = u32Temp << 4;
    SENSOR_I2C_WRITE(0x3e02, u32Temp);

	u32Temp = (u32IntTime>>4)&0xFF;
    SENSOR_I2C_WRITE(0x3e01, u32Temp);
//	printf("cmos_inttime_update set time high bit 0x3e01:%#x!\n", u32Temp);
#endif
    return;
}

static HI_VOID SC1045_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
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


   
    if (u32InTimes >= gain_table[63])
    {    
         pstAeSnsGainInfo->u32SnsTimes = gain_table[63];
         pstAeSnsGainInfo->u32GainDb = 63;
         return ;
    }
    for(i = 1; i < 65; i++)
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

static HI_VOID SC1045_dgain_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
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
    
    if (u32InTimes >= digital_gain_table[4])
    {
         pstAeSnsGainInfo->u32SnsTimes = digital_gain_table[4];
         pstAeSnsGainInfo->u32GainDb = 4;
         return ;
    }
    
    for(i = 1; i < 5; i++)
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

static const HI_U16 sensor_gain_map[48] = {
	0x10, 0x11, 0x12, 0x13, 0x14,0x15, 0x16, 0x17, 0x18, 0x19,
	0x1a, 0x1b, 0x1c,0x1d, 0x1e, 0x1f, 0x30, 0x31,
	0x32, 0x33, 0x34, 0x35,0x36, 0x37, 0x38, 0x39,
	0x3a, 0x3b, 0x3c, 0x3d,0x3e, 0x3f, 0x70, 0x71,
	0x72, 0x73, 0x74, 0x75,0x76, 0x77, 0x78, 0x79,
	0x7a, 0x7b, 0x7c, 0x7d,0x7e, 0x7f
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
static HI_U32 sc1045_Again_limit(HI_U32 Again)
{
#define SENSOR_BLC_TOP_VALUE (0x1c)
#define SENSOR_BLC_BOT_VALUE (0x19)
#define SENSOR_AGAIN_ADAPT_STEP (1)
#define SENSOR_MAX_AGAIN (0x7f)

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


static HI_VOID SC1045_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
         static  HI_U8 u8AgainHigh, u8AgainLow, u8Temp;
	static HI_U8 u8DgainReg;
	
//printf("cmos_gains_update  input u32Again %#x, u32Dgain %#x\n", u32Again, u32Dgain);
	//Again

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

    switch(u32Dgain)               
	{
        case 0:
			u8DgainReg = 0x84;
			break;
			
		case 1:
			u8DgainReg = 0xa4;
			break;
			
	    case 2:
			u8DgainReg = 0xe4;
			break;
			
		case 3:
			u8DgainReg = 0xe5;
			break;
			
		case 4:
			u8DgainReg = 0xe7;
			break;

		default:
			u8DgainReg = 0x84;
            break;
	}
	
	
#if CMOS_SC1045_ISP_WRITE_SENSOR_ENABLE
    SC1045_init_regs_info();
    g_stSnsRegsInfo.astI2cData[2].u32Data = sc1045_Again_limit(u8AgainHigh << 4 | u8AgainLow);
	g_stSnsRegsInfo.astI2cData[3].u32Data = u8DgainReg;

    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
    if (5 == g_stSnsRegsInfo.u32RegNum)
    {
        g_stSnsRegsInfo.u32RegNum = 4;
    }
#else
    SENSOR_I2C_WRITE(0x3e09, sc1045_Again_limit(u8AgainHigh << 4 | u8AgainLow));
    SENSOR_I2C_WRITE(0x3e0f, u8DgainReg);
#endif

    return;
}

static HI_S32 SC1045_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    
    pstAwbSnsDft->u16WbRefTemp = 4850;

    pstAwbSnsDft->au16GainOffset[0] = 0x0190;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x016b;

    pstAwbSnsDft->as32WbPara[0] = 128;
    pstAwbSnsDft->as32WbPara[1] = -19;
    pstAwbSnsDft->as32WbPara[2] = -147;
    pstAwbSnsDft->as32WbPara[3] = 147419;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -98513;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_VOID SC1045_global_init()
{

   gu8SensorMode = 0;
   
}

void SC1045_reg_init()
{    
#if 0
	//20150608
    	SENSOR_I2C_WRITE(0x3003,0x01);
   	SENSOR_I2C_WRITE(0x3000,0x00);
    //sensor_write_register(0x3004,0x44);
	SENSOR_I2C_WRITE(0x3d08,0x00);
	SENSOR_I2C_WRITE(0x3e01,0x2e);
    SENSOR_I2C_WRITE(0x3631,0x84);
    SENSOR_I2C_WRITE(0x3e09,0x00);
    SENSOR_I2C_WRITE(0x3e0f,0x84);
	SENSOR_I2C_WRITE(0x3907,0x03);
    printf("11111111111111111111111111111111111111111111111111111\n");
#if 1 
	//30 fps
	SENSOR_I2C_WRITE(0x3010,0x01);
	SENSOR_I2C_WRITE(0x3011,0xc6);
	SENSOR_I2C_WRITE(0x3004,0x45);
#else
	SENSOR_I2C_WRITE(0x3010,0x21);
	SENSOR_I2C_WRITE(0x3011,0x86);
	SENSOR_I2C_WRITE(0x3004,0x44);
#endif

    SENSOR_I2C_WRITE(0x3622,0x0e);
    SENSOR_I2C_WRITE(0x3600,0x94);
    SENSOR_I2C_WRITE(0x3610,0x03);
	SENSOR_I2C_WRITE(0x3620,0x84);
    SENSOR_I2C_WRITE(0x3634,0x00);
    SENSOR_I2C_WRITE(0x3622,0x0e);
    SENSOR_I2C_WRITE(0x3633,0x2c);
	SENSOR_I2C_WRITE(0x3630,0x88);
    SENSOR_I2C_WRITE(0x3635,0x80);
	
    SENSOR_I2C_WRITE(0x3610,0x03);
    SENSOR_I2C_WRITE(0X3310,0X83);
	SENSOR_I2C_WRITE(0X3336,0X00);
   SENSOR_I2C_WRITE(0X3337,0X00);
    SENSOR_I2C_WRITE(0X3338,0X02);
    SENSOR_I2C_WRITE(0x3339,0xe0);
	SENSOR_I2C_WRITE(0X331E,0XA0);
   SENSOR_I2C_WRITE(0x3335,0x1a);
	
    SENSOR_I2C_WRITE(0x3330,0x0d);
    SENSOR_I2C_WRITE(0x3320,0x07);
	SENSOR_I2C_WRITE(0x3321,0x60);
    SENSOR_I2C_WRITE(0x3322,0x02);
    SENSOR_I2C_WRITE(0x3323,0xc0);
    SENSOR_I2C_WRITE(0X3315,0X44);
	SENSOR_I2C_WRITE(0X331e,0Xe0);
	SENSOR_I2C_WRITE(0X3308,0X40);
    SENSOR_I2C_WRITE(0x3303,0xa0);
   SENSOR_I2C_WRITE(0x3304,0x60);
    SENSOR_I2C_WRITE(0x3000,0x01);

#else

#if 0
//27M input  36M pClk 25 fps

SENSOR_I2C_WRITE(0x3003,0x01);
SENSOR_I2C_WRITE(0x3000,0x00);
SENSOR_I2C_WRITE(0x3004,0x44);
//SENSOR_I2C_WRITE(0x3005,0xf2);
SENSOR_I2C_WRITE(0x3d08,0x00);
SENSOR_I2C_WRITE(0x3e01,0x2e);
SENSOR_I2C_WRITE(0x3631,0x81);//88
SENSOR_I2C_WRITE(0x3e09,0x00);
SENSOR_I2C_WRITE(0x3e0f,0x84);
SENSOR_I2C_WRITE(0x3907,0x03);
SENSOR_I2C_WRITE(0x3622,0x0e);

SENSOR_I2C_WRITE(0x3600,0x0094);
SENSOR_I2C_WRITE(0x3610,0x0003);
SENSOR_I2C_WRITE(0x3620,0x0084);
SENSOR_I2C_WRITE(0x3634,0x0000);
SENSOR_I2C_WRITE(0x3622,0x000e);
SENSOR_I2C_WRITE(0x3633,0x0028);

SENSOR_I2C_WRITE(0x3630,0x0088);
SENSOR_I2C_WRITE(0x3635,0x0080);
SENSOR_I2C_WRITE(0x3610,0x0003);

SENSOR_I2C_WRITE(0X3310,0X0083);//a3
SENSOR_I2C_WRITE(0X3336,0X0000);
SENSOR_I2C_WRITE(0X3337,0X0000);
SENSOR_I2C_WRITE(0X3338,0X0002);
SENSOR_I2C_WRITE(0x3339,0x00e0);
SENSOR_I2C_WRITE(0X331E,0X00A0);
SENSOR_I2C_WRITE(0x3335,0x001a);

SENSOR_I2C_WRITE(0x3330,0x000d);
SENSOR_I2C_WRITE(0x3320,0x0005);
SENSOR_I2C_WRITE(0x3321,0x0060);
SENSOR_I2C_WRITE(0x3322,0x0002);
SENSOR_I2C_WRITE(0x3323,0x00c0);
SENSOR_I2C_WRITE(0X3315,0X0044);

SENSOR_I2C_WRITE(0X331e,0Xe0);//56
SENSOR_I2C_WRITE(0x3335,0x3a);


SENSOR_I2C_WRITE(0X3308,0X0040);
SENSOR_I2C_WRITE(0x3303,0x00a0);
SENSOR_I2C_WRITE(0x3304,0x0060);
SENSOR_I2C_WRITE(0x3000,0x01);
SENSOR_I2C_WRITE(0x3416,0x40);

#else
SENSOR_I2C_WRITE(0x3003,0x01);//soft reset
SENSOR_I2C_WRITE(0x3000,0x00);//pause for reg writing
SENSOR_I2C_WRITE(0x3010,0x01);//output format 43.2M 1920X750 30fps
SENSOR_I2C_WRITE(0x3011,0xc6);
SENSOR_I2C_WRITE(0x3004,0x45);
SENSOR_I2C_WRITE(0x3e03,0x03);//exp and gain
SENSOR_I2C_WRITE(0x3600,0x94);//0607 reduce power 
SENSOR_I2C_WRITE(0x3610,0x03);
SENSOR_I2C_WRITE(0x3634,0x00);// reduce power
SENSOR_I2C_WRITE(0x3620,0x84);
SENSOR_I2C_WRITE(0x3631,0x85);// txvdd 0909
SENSOR_I2C_WRITE(0x3907,0x03);
SENSOR_I2C_WRITE(0x3622,0x0e); 
SENSOR_I2C_WRITE(0x3633,0x2c);//0825
SENSOR_I2C_WRITE(0x3630,0x88);
SENSOR_I2C_WRITE(0x3635,0x80);
SENSOR_I2C_WRITE(0x3310,0X83);//prechg tx auto ctrl [5] 0825
SENSOR_I2C_WRITE(0x3336,0x00);
SENSOR_I2C_WRITE(0x3337,0x00);
SENSOR_I2C_WRITE(0x3338,0x02);
SENSOR_I2C_WRITE(0x3339,0xee);
SENSOR_I2C_WRITE(0x331E,0xa0); //    
SENSOR_I2C_WRITE(0x3335,0x10);   
SENSOR_I2C_WRITE(0x3315,0X44);
SENSOR_I2C_WRITE(0x3308,0X40);
SENSOR_I2C_WRITE(0x3330,0x0d);// sal_en timing,cancel the fpn in low light
SENSOR_I2C_WRITE(0x3320,0x05);//0825
SENSOR_I2C_WRITE(0x3321,0x60);
SENSOR_I2C_WRITE(0x3322,0x02);
SENSOR_I2C_WRITE(0x3323,0xc0);
SENSOR_I2C_WRITE(0x3303,0xa0);
SENSOR_I2C_WRITE(0x3304,0x60);
SENSOR_I2C_WRITE(0x3d04,0x04);//0806    vsync gen mode
SENSOR_I2C_WRITE(0x3d08,0x02);
SENSOR_I2C_WRITE(0x3000,0x01); //recover 
#endif

#endif

	return ;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 SC1045_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = SC1045_reg_init;
    pstSensorExpFunc->pfn_cmos_sensor_global_init =SC1045_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = SC1045_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = SC1045_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = SC1045_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = SC1045_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = SC1045_get_sensor_max_resolution;

    return 0;
}

HI_S32 SC1045_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = SC1045_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = SC1045_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= SC1045_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = SC1045_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = SC1045_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = SC1045_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = SC1045_dgain_calc_table;
	
    return 0;
}

HI_S32 SC1045_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = SC1045_get_awb_default;

    return 0;
}

int SC1045_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    SC1045_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(SC1045_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    SC1045_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, SC1045_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    SC1045_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, SC1045_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int  SC1045_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(SC1045_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, SC1045_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, SC1045_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


void SC1045_init(SENSOR_SC1045_DO_I2CRD do_i2c_read, SENSOR_SC1045_DO_I2CWR do_i2c_write)
{// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	SC1045_reg_init();

	SC1045_sensor_register_callback();

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
	printf("SC1045 sensor 720P30fps init success!\n");

return 0;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif // __SC1045_CMOS_H_

