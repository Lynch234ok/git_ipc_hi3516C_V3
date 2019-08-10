#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_APTINA_AR0130_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_APTINA_AR0130_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

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
static HI_U32 gu32FullLinesStd = 1198;
static HI_U8 gu8SensorMode = 0;


#if CMOS_AR0130_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif

static AWB_CCM_S g_stAwbCcm =
{
 
   		5000,
		{
			0x191,0x8071,0x8020,
			0x8036,0x126,0xf,
			0xf,  0x80be,0x1ae
		},
	
		4000,
		{
			0x0193,0x8057,0x803c,
			0x8051,0x013c,0x14,
			0x8006,0x80bc,0x01c2
		},
	
		2500,
		{
		 0x01d4, 0x8040, 0x8094,
		 0x803f, 0x0fc,  0x42,
		 0x8020, 0x81d4, 0x2f4
		 }
 
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */    
    {0x88,0x88,0x88,0x80,0x76,0x6a,0x5a,0x38}
};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
#if 1
    /* sharpen_alt_d */
    //{0x70,0x68,0x58,0x48,0x34,0x30,0x20,0x18},
	{115,103,95,85,70,45,32,24},
	
    //sharpen_alt_ud
   	//{0xa0,0x90,0x6c,0x60,0x58,0x40,0x20,0x10},
	{100,70,70,70,62,50,32,16},

    //snr_thresh
    //{0x13,0x19,0x23,0x26,0x2c,0x62,0x78,0x98},
    {8,12,18,26,36,60,64,66},
#else
        /* sharpen_alt_d */
    {50,50,45,35,30,30,20,15},

    //sharpen_alt_ud
    {45,45,35,30,25,25,15,10},

    //snr_thresh
    {8,12,18,26,36,46,56,66},
#endif

    /* demosaic_lum_thresh */
    {0x50,0x50,0x40,0x40,0x30,0x30,0x20,0x20},
    //{0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80},    
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
	{0, 0, 0, 0, 0, 0, 3, 12, 16, 20, 22, 25, 26, 28, 29, 30,
	32, 33, 34, 34, 35, 36, 37, 37, 38, 38, 39, 39, 40, 40,
	41, 41, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46,
	46, 46, 47, 47, 47, 47, 48, 48, 48, 48, 49, 49, 49, 49,
	50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 52, 52, 52, 52,
	52, 52, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54,
	54, 55, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56,
	56, 56, 57, 57, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58,
	58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59},
	/*{0,  0,  0,  0,  0,  0,  11, 15, 17, 19, 20, 21, 22, 22, 23, 24,
    25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29,
    30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32,
    32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34,
    34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38},*/
    /*{0,  0,  0,  0,  0,  0,  0, 12, 17, 21, 24, 26, 28, 30, 31, 32, 
    33, 34, 35, 36, 37, 38, 39, 39, 40, 41, 41, 42, 42, 43, 43, 44,
    44, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49,
    50, 50, 50, 50, 51, 51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53,
    53, 53, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 56, 56, 56,
    56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58,
    58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60,
    60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 62, 62},*/
	/*{0,  0,  0,  0,  0,  0,  16, 20, 22, 24, 24, 24, 24, 24, 24, 24,
    25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29,
    30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32,
    32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34,
    34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38},*/

    /* demosaic_weight_lut */
	{3, 12, 16, 20, 22, 25, 26, 28, 29, 30, 32, 33, 34, 34, 35,
	36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
	43, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47, 47, 48,
	48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 50, 51, 51, 51,
	51, 51, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 54,
	54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 56, 56,
	56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57, 57,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59,
	59, 59, 59, 59, 59, 59, 59, 59} 
	/*{0 ,0,0,0,24,26,28,30,31,32,33,34,35,36,37,38,
    39,39,40,41,41,42,42,43,43,44,44,45,45,45,46,46,
    46,47,47,47,48,48,48,49,49,49,50,50,50,50,51,51,
    51,51,52,52,52,52,52,53,53,53,53,53,54,54,54,54,
    54,55,55,55,55,55,55,56,56,56,56,56,56,56,57,57,
    57,57,57,57,57,58,58,58,58,58,58,58,58,59,59,59,
    59,59,59,59,59,60,60,60,60,60,60,60,60,60,61,61,
    61,61,61,61,61,61,61,61,62,62,62,62,62,62,62,62}*/
    /*{0, 12, 17, 21, 24, 26, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 
    39, 39, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 45, 46, 46, 
    46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 50, 51, 51, 
    51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 54, 54, 54, 54, 
    54, 55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 57, 57, 
    57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 
    59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60, 60, 61, 61, 
    61, 61, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 62, 62}*/
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    220,
	//0xf5,
    /*aa_slope*/
    200,
	//0xb4,
    /*va_slope*/
    185,
	//0xe6,
    /*uu_slope*/
    188,
	//0xa0,
    /*sat_slope*/
    93,
	//0x5d,
    /*ac_slope*/
    160,
	//0xcf,
    /*vh_thresh*/
    0,
	//0x30,
    /*aa_thresh*/
    0,
	//0x5b,
    /*va_thresh*/
    0,
	//0x52,
    /*uu_thresh*/
    8,
	//0x40,
    /*sat_thresh*/
    0,
	//0x171,
    /*ac_thresh*/
    0x1b3
};

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,
    
#if 1    
    {0x0,0xd,0x20,0x3b,0x5b,0x82,0xaf,0xcb,0xf7,0x12f,0x16f,0x1b4,0x1fb,0x23f,0x27f,0x2ba,
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
		0xfff}
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

HI_U32 ar0130_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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

HI_U32 ar0130_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
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
        pstBlackLevel->au16BlackLevel[i] = 0xC8;
    }

    return 0;    
}

HI_VOID ar0130_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(0x300C, 0x4D58);	//5fps
        SENSOR_I2C_WRITE(0x3012, 0x05DA);	//max exposure lines
        SENSOR_I2C_WRITE(0x30B0, 0x1300);	//AG, Context A
        SENSOR_I2C_WRITE(0x305E, 0x0020);	//DG, Context A
        SENSOR_I2C_WRITE(0x3100, 0x0000);
	    SENSOR_I2C_WRITE(0x3ee4, 0xd208);
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(0x300C, 0x684);	//25fps
        SENSOR_I2C_WRITE(0x3ee4, 0xd308);
 	    SENSOR_I2C_WRITE(0x3100, 0x0004);
    }

    return;
}

HI_VOID ar0130_set_wdr_mode(HI_U8 u8Mode)
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

static HI_U32 analog_gain_table[6] =
{
     1024, 2048, 2886, 5772, 11544, 23088

};


static HI_VOID ar0130_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
 
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;

    if (u32InTimes >= analog_gain_table[5])
    {
         pstAeSnsGainInfo->u32SnsTimes = analog_gain_table[5];
         pstAeSnsGainInfo->u32GainDb = 5;
         return ;
    }
    
    for(i = 1; i < 6; i++)
    {
        if(u32InTimes < analog_gain_table[i])
        {

            pstAeSnsGainInfo->u32SnsTimes = analog_gain_table[i - 1];
            pstAeSnsGainInfo->u32GainDb = i - 1;
            break;
        }

    }

    return;
}

static HI_S32 ar0130_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = 1198;
    
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x40;
    
    pstAeSnsDft->u32LinesPer500ms = 1198*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = 1196;
    pstAeSnsDft->u32MinIntTime = 2;    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 6;    
    pstAeSnsDft->u32MaxAgain = 23088;  /* 9db + 18db(8) */
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = 23088;
    pstAeSnsDft->u32MinAgainTarget = 1024;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.03125;    
    pstAeSnsDft->u32MaxDgain = 255;  /* (8 - 0.3125) / 0.03125 = 255 */
    pstAeSnsDft->u32MinDgain = 32;
    pstAeSnsDft->u32MaxDgainTarget = 255;
    pstAeSnsDft->u32MinDgainTarget = 32;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

static HI_S32 ar0130_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID ar0130_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch(u8Fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 1198;
            pstAeSnsDft->u32MaxIntTime = 1196;
            pstAeSnsDft->u32LinesPer500ms = 1198 * 30 / 2;
            SENSOR_I2C_WRITE(0x300c, 0x56e);
        break;
        
        case 25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = 1198;
            pstAeSnsDft->u32MaxIntTime = 1196;
            pstAeSnsDft->u32LinesPer500ms = 1198 * 25 / 2;
            SENSOR_I2C_WRITE(0x300c, 0x684);
        break;
        
        default:
        break;
    }

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID ar0130_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    SENSOR_I2C_WRITE(0x300A, u16FullLines);

    pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;
    
    return;
}

static HI_VOID ar0130_init_regs_info(HI_VOID)
{
#if CMOS_AR0130_ISP_WRITE_SENSOR_ENABLE
    HI_S32 i;
    static HI_BOOL bInit = HI_FALSE;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u32RegNum = 4;
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3012;
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x30B0;
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x305E;
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3100;
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_FALSE;

        bInit = HI_TRUE;
    }
#endif
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ar0130_inttime_update(HI_U32 u32IntTime)
{
#if CMOS_AR0130_ISP_WRITE_SENSOR_ENABLE
    ar0130_init_regs_info();
    g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;
#else
    SENSOR_I2C_WRITE(0x3012, u32IntTime);
#endif
    return;
}

static HI_VOID ar0130_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
#if CMOS_AR0130_ISP_WRITE_SENSOR_ENABLE
    ar0130_init_regs_info();
    
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
    
        HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
#else
        if(u32Again >= 2)
        {
            SENSOR_I2C_WRITE(0x3100, 0x0004);
        }
        else
        {
            SENSOR_I2C_WRITE(0x3100, 0x0000);
        }
        
        switch(u32Again)
        {
            case 0:
            case 2:
                SENSOR_I2C_WRITE(0x30B0, 0x1300);
                break;
            case 1:
            case 3:
                SENSOR_I2C_WRITE(0x30B0, 0x1310);
                break;
            case 4:
                SENSOR_I2C_WRITE(0x30B0, 0x1320);
                break;
            case 5:
                SENSOR_I2C_WRITE(0x30B0, 0x1330);
                break;
        }
    
        SENSOR_I2C_WRITE(0x305E, u32Dgain);
    
#endif
    return;
}

static HI_S32 ar0130_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;

    pstAwbSnsDft->au16GainOffset[0] = 0x015d;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x019e;

    pstAwbSnsDft->as32WbPara[0] = 112;
    pstAwbSnsDft->as32WbPara[1] = -51;
    pstAwbSnsDft->as32WbPara[2] = -195;
    pstAwbSnsDft->as32WbPara[3] = 215799;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -170366;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

static HI_VOID ar0130_global_init()
{
	gu8SensorMode = 0;
}

void ar0130_reg_init()
{
	SENSOR_I2C_WRITE( 0x301A, 0x0001 ); // RESET_REGISTER
    SENSOR_DELAY_MS(200); //ms
    SENSOR_I2C_WRITE( 0x301A, 0x10D8 );    // RESET_REGISTER
    SENSOR_DELAY_MS(200); //ms
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
    SENSOR_DELAY_MS(200); //ms
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
    SENSOR_DELAY_MS(500 );//ms                 
    SENSOR_I2C_WRITE( 0x301A, 0x10D8); // RESET_REGISTER
    SENSOR_DELAY_MS(500); //ms                   
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
    SENSOR_I2C_WRITE(0x3002, 0x0004);  //Y_ADDR_START
    SENSOR_I2C_WRITE(0x3004, 0x0002);   // X_ADDR_START
    SENSOR_I2C_WRITE(0x3006, 0x03c3);  // Y_ADDR_END
    SENSOR_I2C_WRITE(0x3008, 0x0501);  // X_ADDR_END
    SENSOR_I2C_WRITE(0x300A, 0x04ae);  // FRAME_LENGTH_LINES  ///ddd 990  
    SENSOR_I2C_WRITE(0x300C, 0x056e);  // LINE_LENGTH_PCK	///ddd 1683 0x693 2016 0x7e0

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
    SENSOR_DELAY_MS(100); //ms
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

    SENSOR_DELAY_MS(200);  //DELAY= 200
}


/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 ar0130_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ar0130_reg_init;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = ar0130_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = ar0130_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ar0130_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ar0130_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ar0130_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = ar0130_get_sensor_max_resolution;

    return 0;
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

HI_S32 ar0130_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ar0130_get_awb_default;

    return 0;
}

int ar0130_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ar0130_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(AR0130_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    ar0130_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, AR0130_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    ar0130_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, AR0130_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
	
    return 0;
}

int ar0130_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(AR0130_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, AR0130_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, AR0130_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

static int af_register_callback(ISP_AF_REGISTER_S *pAfRegister)
{
	HI_S32 s32Ret = 0;
	ALG_LIB_S stLib;
	
	stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_ISP_AfLibRegCallBack(&stLib,pAfRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to AF lib failed!\n");
        return s32Ret;
    }
	return s32Ret;
}

void APTINA_AR0130_init(SENSOR_APTINA_AR0130_DO_I2CRD do_i2c_read, 
	SENSOR_APTINA_AR0130_DO_I2CWR do_i2c_write,
	ISP_AF_REGISTER_S *pAfRegister)
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	ar0130_reg_init();

	ar0130_sensor_register_callback();
	af_register_callback(pAfRegister);

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
	printf("Aptina AR0130 sensor 720P30fps init success!\n");
		
}

#if 0
#define SET_VI_DEV_ATTR_AR0130_720P(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xfff00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 720;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}\

#define SET_VI_DEV_ATTR_AR0130_960P(info) \
{\
			info.enIntfMode = VI_MODE_DIGITAL_CAMERA;\
			info.enWorkMode = VI_WORK_MODE_1Multiplex;\
			info.au32CompMask[0] = 0xfff00000;\
			info.au32CompMask[1] = 0x00000000;\
			info.enScanMode = VI_SCAN_PROGRESSIVE;\
			info.s32AdChnId[0] = -1;\
			info.s32AdChnId[1] = -1;\
			info.s32AdChnId[2] = -1;\
			info.s32AdChnId[3] = -1;\
			info.enDataSeq = VI_INPUT_DATA_YUYV;\
			info.stSynCfg.enVsync = VI_VSYNC_PULSE;\
			info.stSynCfg.enVsyncNeg = VI_VSYNC_NEG_HIGH;\
			info.stSynCfg.enHsync = VI_HSYNC_VALID_SINGNAL;\
			info.stSynCfg.enHsyncNeg = VI_HSYNC_NEG_HIGH;\
			info.stSynCfg.enVsyncValid = VI_VSYNC_VALID_SINGAL;\
			info.stSynCfg.enVsyncValidNeg = VI_VSYNC_VALID_NEG_HIGH;\
			info.stSynCfg.stTimingBlank.u32HsyncHfb = 0;\
			info.stSynCfg.stTimingBlank.u32HsyncAct = 1280;\
			info.stSynCfg.stTimingBlank.u32HsyncHbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVact = 960;\
			info.stSynCfg.stTimingBlank.u32VsyncVbb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbfb = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbact = 0;\
			info.stSynCfg.stTimingBlank.u32VsyncVbbb = 0;\
			info.enDataPath = VI_PATH_ISP;\
			info.enInputDataType = VI_DATA_TYPE_RGB;\
			info.bDataRev = HI_FALSE;\
}\


//chang sensor mode
int AR0130_sensor_mode_set(uint8_t u8Mode)//0: 720P 1: 960P
{
    VI_DEV_ATTR_S vi_dev_attr;
	VI_CHN_ATTR_S vin_chn_attr;
	VPSS_GRP_ATTR_S vpss_grp_attr;
	ISP_IMAGE_ATTR_S isp_image_attr;
    HI_MPI_VI_GetChnAttr(0, &vin_chn_attr);
	HI_MPI_ISP_GetImageAttr(&isp_image_attr);
    switch(u8Mode)
    {
        case 0:/*720P line mode*/
            gu8ResolutionMode = 0;
        break;  
        case 1:/*960P line mode*/
            gu8ResolutionMode = 1;
        break;
        default:
            printf("NOT support this mode!\n");
            return -1;
        break;
    }
    if(gu8ResolutionMode == 0)//720P init sequence
    {
        SENSOR_I2C_WRITE( 0x3002, 0x0002); // Y_ADDR_STARTSENSOR_I2C_WRITE( 0x3004, 0x0000); // X_ADDR_START
        SENSOR_I2C_WRITE( 0x3006, 0x02D1);//Row End (A) = 721
        SENSOR_I2C_WRITE( 0x3008, 0x04FF);//Column End (A) = 1279
        SENSOR_I2C_WRITE( 0x300A, 0x02EA);//Frame Lines (A) = 746
        SENSOR_I2C_WRITE( 0x300C, 0x08ba);
		SET_VI_DEV_ATTR_AR0130_720P(vi_dev_attr);
		isp_image_attr.enBayer = BAYER_GRBG;
   		isp_image_attr.u16Width = 1280;
    	isp_image_attr.u16Height = 720;
		vin_chn_attr.stCapRect.u32Width = 1280;
		vin_chn_attr.stCapRect.u32Height = 720;
		vin_chn_attr.stDestSize.u32Width = 1280;
		vin_chn_attr.stDestSize.u32Height = 720;
		vpss_grp_attr.u32MaxW = 1280;
		vpss_grp_attr.u32MaxH = 720;
    }else if(gu8ResolutionMode == 1)//960P init sequence
    {
        SENSOR_I2C_WRITE(0x3002, 0x0004);  // Y_ADDR_START
        SENSOR_I2C_WRITE(0x3004, 0x0002); 	// X_ADDR_START
        SENSOR_I2C_WRITE(0x3006, 0x03c3); 	// Y_ADDR_END
        SENSOR_I2C_WRITE(0x3008, 0x0501); 	// X_ADDR_END
        SENSOR_I2C_WRITE(0x300A, 0x04ae); 	// FRAME_LENGTH_LINES  ///ddd 990  
        SENSOR_I2C_WRITE(0x300C, 0x056e); 	// LINE_LENGTH_PCK	///ddd 1683 0x693 2016 0x7e0
        SET_VI_DEV_ATTR_AR0130_960P(vi_dev_attr);
        isp_image_attr.enBayer = BAYER_GRBG;
   		isp_image_attr.u16Width = 1280;
    	isp_image_attr.u16Height = 960;
		vin_chn_attr.stCapRect.u32Width = 1280;
		vin_chn_attr.stCapRect.u32Height = 960;
		vin_chn_attr.stDestSize.u32Width = 1280;
		vin_chn_attr.stDestSize.u32Height = 960;
		vpss_grp_attr.u32MaxW = 1280;
		vpss_grp_attr.u32MaxH = 960;
    }    
	//HI_MPI_ISP_SetImageAttr(&isp_image_attr);	
	//HI_MPI_VI_SetDevAttr(0, &vi_dev_attr);
	//HI_MPI_VI_SetChnAttr(0, &vin_chn_attr);
    return 0;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // __AR0130_CMOS_H_

