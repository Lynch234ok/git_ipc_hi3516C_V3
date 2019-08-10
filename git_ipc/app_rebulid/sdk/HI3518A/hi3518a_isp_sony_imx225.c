#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"


static  SENSOR_IMX225_DO_I2CRD sensor_i2c_read = NULL;
static  SENSOR_IMX225_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__IMX225_CMOS_H_)
#define __IMX225_CMOS_H_

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

#define CMOS_IMX225_ISP_WRITE_SENSOR_ENABLE (1)


#if CMOS_IMX225_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static HI_BOOL gsbRegInit = HI_FALSE;
#endif

#define EXPOSURE_ADDR (0x220) //2:chip_id, 0C: reg addr.

#define PGC_ADDR (0x214)
#define VMAX_ADDR (0x218)

#define IMX225_ID 225


// sns gain parameters  unit DB
#define SNS_GAIN_MAX 72
#define GAIN_PRECISE 0.1
#define GAIN_TBL_SIZE (int)(SNS_GAIN_MAX / GAIN_PRECISE + 1)

#define FULL_LINES_DEFAULT 1100 //qb
#define FULL_LINES_25FPS 1320
//#define FULL_LINES_DEFAULT 750//qb
//#define FULL_LINES_25FPS 900
#define FPS30 30
#define FPS25 25 



// usefull macro

#define HIGH_8BITS(x) ((x & 0xff00) >> 8)
#define LOW_8BITS(x) (x & 0x00ff)


/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

static const HI_U32 gain_table[GAIN_TBL_SIZE] = {

1024,  	  1035,     1047,     1059,     1072,     1084,     1097,     1109,     1122,     1135,     1148,     1162,    1175,      1189,     1203,     1217,  
1231,  	  1245,     1259,     1274,     1289,     1304,     1319,     1334,     1349,     1365,     1381,     1397,    1413,      1429,     1446,     1463,  
1480,  	  1497,     1514,     1532,     1549,     1567,     1585,     1604,     1622,     1641,     1660,     1679,    1699,      1719,     1739,     1759,  
1779,  	  1800,     1820,     1842,     1863,     1884,     1906,     1928,     1951,     1973,     1996,     2019,    2043,      2066,     2090,     2114,  
2139,  	  2164,     2189,     2214,     2240,     2266,     2292,     2318,     2345,     2373,     2400,     2428,    2456,      2484,     2513,     2542,  
2572,  	  2601,     2632,     2662,     2693,     2724,     2756,     2788,     2820,     2852,     2886,     2919,    2953,      2987,     3022,     3057,  
3092,  	  3128,     3164,     3201,     3238,     3275,     3313,     3351,     3390,     3430,     3469,     3509,    3550,      3591,     3633,     3675,  
3717,  	  3760,     3804,     3848,     3893,     3938,     3983,     4029,     4076,     4123,     4171,     4219,    4268,      4318,     4368,     4418,  
4469,  	  4521,     4574,     4627,     4680,     4734,     4789,     4845,     4901,     4957,     5015,     5073,    5132,      5191,     5251,     5312,  
5374,  	  5436,     5499,     5562,     5627,     5692,     5758,     5825,     5892,     5960,     6029,     6099,    6170,      6241,     6313,     6387,  
6461,  	  6535,     6611,     6688,     6765,     6843,     6923,     7003,     7084,     7166,     7249,     7333,    7418,      7504,     7591,     7678,  
7767,  	  7857,     7948,     8040,     8133,     8228,     8323,     8419,     8517,     8615,     8715,     8816,    8918,      9021,     9126,     9232,  
9339,  	  9447,     9556,     9667,     9779,     9892,     10006,    10122,    10240,    10358,    10478,    10599,   10722,     10846,    10972,    11099,  
11227,    11358,    11489,    11622,    11757,    11893,    12031,    12170,    12311,    12453,    12598,    12743,   12891,     13040,    13191,    13344,  
13499,    13655,    13813,    13973,    14135,    14298,    14464,    14631,    14801,    14972,    15146,    15321,   15498,     15678,    15859,    16043,  
16229,    16417,    16607,    16799,    16994,    17191,    17390,    17591,    17795,    18001,    18209,    18420,   18633,     18849,    19067,    19288,  
19512,    19737,    19966,    20197,    20431,    20668,    20907,    21149,    21394,    21642,    21892,    22146,   22402,     22662,    22924,    23190,  
23458,    23730,    24005,    24283,    24564,    24848,    25136,    25427,    25721,    26019,    26321,    26625,   26934,     27246,    27561,    27880,  
28203,    28530,    28860,    29194,    29532,    29874,    30220,    30570,    30924,    31282,    31644,    32011,   32382,     32756,    33136,    33519,  
33908,    34300,    34697,    35099,    35506,    35917,    36333,    36753,    37179,    37610,    38045,    38486,   38931,     39382,    39838,    40299,  
40766,    41238,    41716,    42199,    42687,    43182,    43682,    44187,    44699,    45217,    45740,    46270,   46806,     47348,    47896,    48451,  
49012,    49579,    50153,    50734,    51321,    51916,    52517,    53125,    53740,    54362,    54992,    55629,   56273,     56924,    57584,    58250,  
58925,    59607,    60297,    60996,    61702,    62416,    63139,    63870,    64610,    65358,    66115,    66880,   67655,     68438,    69231,    70032,  
70843,    71663,    72493,    73333,    74182,    75041,    75910,    76789,    77678,    78577,    79487,    80408,   81339,     82280,    83233,    84197,  
85172,    86158,    87156,    88165,    89186,    90219,    91263,    92320,    93389,    94471,    95564,    96671,   97790,     98923,    100068,   101227,  
102399,   103585,   104784,   105998,   107225,   108467,   109722,   110993,   112278,   113578,   114893,   116224,  117570,    118931,   120308,   121701,  
123110,   124536,   125978,   127437,   128912,   130405,   131915,   133443,   134988,   136551,   138132,   139732,  141350,    142986,   144642,   146317,  
148011,   149725,   151459,   153212,   154987,   156781,   158597,   160433,   162291,   164170,   166071,   167994,  169939,    171907,   173897,   175911,  
177948,   180009,   182093,   184201,   186334,   188492,   190675,   192882,   195116,   197375,   199661,   201973,  204311,    206677,   209070,   211491,  
213940,   216417,   218923,   221458,   224023,   226617,   229241,   231895,   234580,   237297,   240044,   242824,  245636,    248480,   251357,   254268,  
257212,   260190,   263203,   266251,   269334,   272452,   275607,   278799,   282027,   285293,   288596,   291938,  295318,    298738,   302197,   305696,  
309236,   312817,   316439,   320103,   323810,   327559,   331352,   335189,   339070,   342996,   346968,   350986,  355050,    359161,   363320,   367527,  
371782,   376088,   380442,   384848,   389304,   393812,   398372,   402985,   407651,   412371,   417146,   421976,  426863,    431805,   436805,   441863,  
446980,   452155,   457391,   462687,   468045,   473465,   478947,   484493,   490103,   495778,   501519,   507326,  513200,    519143,   525154,   531235,  
537386,   543609,   549904,   556271,   562712,   569228,   575819,   582487,   589232,   596055,   602957,   609938,  617001,    624145,   631373,   638683,  
646079,   653560,   661128,   668783,   676527,   684361,   692285,   700301,   708411,   716613,   724911,   733305,  741796,    750386,   759075,   767864,  
776755,   785750,   794848,   804052,   813362,   822780,   832308,   841945,   851694,   861556,   871532,   881624,  891833,    902160,   912606,   923173,  
933863,   944677,   955615,   966681,   977874,   989197,   1000651,  1012238,  1023959,  1035816,  1047810,  1059943, 1072216,   1084632,  1097191,  1109895,  
1122748,  1135748,  1148899,  1162202,  1175660,  1189273,  1203044,  1216975,  1231066,  1245321,  1259741,  1274328, 1289084,   1304010,  1319110,  1334385,  
1349836,  1365466,  1381276,  1397271,  1413451,  1429817,  1446373,  1463121,  1480063,  1497201,  1514538,  1532075, 1549815,   1567761,  1585915,  1604278,  
1622855,  1641646,  1660655,  1679885,  1699336,  1719013,  1738918,  1759053,  1779423,  1800027,  1820870,  1841954, 1863282,   1884858,  1906684,  1928761,  
1951095,  1973687,  1996542,  2019660,  2043046,  2066703,  2090633,  2114842,  2139330,  2164102,  2189161,  2214509, 2240153,   2266092,  2292331,  2318875,  
2345725,  2372888,  2400364,  2428158,  2456274,  2484716,  2513488,  2542592,  2572034,  2601816,  2631942,  2662420, 2693248,   2724434,  2755980,  2787892,  
2820175,  2852831,  2885864,  2919280,  2953083,  2987279,  3021869,  3056860,  3092255,  3128061,  3164283,  3200923, 3237987,   3275480,  3313408,  3351776,  
3390587,  3429847,  3469562,  3509736,  3550378,  3591488,  3633075,  3675143,  3717698,  3760748,  3804294,  3848345, 3892905,   3937982,  3983583,  4029709,  
4076370 };



static HI_U32 gu32FullLinesStd = FULL_LINES_DEFAULT;
static HI_U32 gu32FullLines = FULL_LINES_DEFAULT;

static AWB_CCM_S g_stAwbCcm =
{

    5000,
    {
        0x01DA, 0x80BC, 0x801E,
        0x8055, 0x01A4, 0x804F,
        0x0020, 0x80C5, 0x01A5,
    },
    4000,
    {
        0x01D2, 0x80A3, 0x802F,
        0x807D, 0x01B9, 0x803C,
        0x0022, 0x80CB, 0x01A9,
    },
    2856,
    {
        0x01D1, 0x80AC, 0x8025,
        0x8081, 0x01AF, 0x802E,
        0x002F, 0x819D, 0x026E,
    }
   
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */   
    {0x80,0x80,0x80,0x75,0x60,0x55,0x50,0x48}

};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,

    /* sharpen_alt_d */
   {80,70,60,45,40,35,25,15}, 
    
    /* sharpen_alt_ud */
   {40,40,35,30,25,25,15,10},    
    
    /* snr_thresh    */
   {10,16,24,34,39,44,49,54},  
    
    /*  demosaic_lum_thresh   */
   {0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80}, 
    
    /* demosaic_np_offset   */
   {0,0xa,16,24,32,40,48,56},  
    
    /* ge_strength    */
   {0x82,0x55,0x55,0x55,0x55,0x55,0x37,0x37}, 
    

};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,
    
    //nosie_profile_weight_lut
   {
     0,  0,  0,  0,  0,  0,  0,  0,  7,  13, 17, 19, 22, 24, 25, 27, 
     28, 29, 30, 31, 32, 33, 33, 35, 35, 36, 36, 37, 38, 38, 39, 39, 
     39, 40, 40, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 44, 45, 
     45, 45, 46, 46, 46, 46, 47, 47, 47, 47, 48, 48, 48, 48, 48, 49, 
     49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 51, 
     52, 52, 52, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 54, 54, 
     54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 56, 56, 
     56, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57 
   }, 

    //demosaic_weight_lut
   {
	7,  13, 17, 19, 22, 24, 25, 27, 28, 29, 30, 31, 32, 33, 33, 35, 
	35, 36, 36, 37, 38, 38, 39, 39, 39, 40, 40, 41, 41, 42, 42, 42, 
	43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 46, 46, 46, 46, 47, 47, 
	47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 50, 50, 50, 50, 
	50, 50, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 53, 
	53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 55, 55, 
	55, 55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 
	57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57 
   } 
     
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
     /* bvalid */
     1,
    
    /*vh_slope*/
     220,
     
     /*aa_slope*/
     200,
     
     /*va_slope*/
     185,
     
     /*uu_slope*/
     210,
     
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
     0,
     
     /*ac_thresh*/
     0x1b3,

};

HI_U32 imx225_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

    pstDef->stComm.u8Rggb           = 0x2;      //2: gbrg 
    pstDef->stComm.u8BalanceFe      = 0x1;

    pstDef->stDenoise.u8SinterThresh= 0x8;
    pstDef->stDenoise.u8NoiseProfile= 0x1;      //0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
    pstDef->stDenoise.u16Nr0        = 0x0;
    pstDef->stDenoise.u16Nr1        = 455;

    pstDef->stDrc.u8DrcBlack        = 0x00;
    pstDef->stDrc.u8DrcVs           = 0x04;     // variance space
    pstDef->stDrc.u8DrcVi           = 0x08;     // variance intensity
    pstDef->stDrc.u8DrcSm           = 0xa0;     // slope max
    pstDef->stDrc.u16DrcWl          = 0x8ff;    // white level

    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));

    return 0;
}

HI_U32 imx225_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
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
        pstBlackLevel->au16BlackLevel[i] = 0xF0;
    }

    return 0;    
}

HI_VOID imx225_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        /* Sensor must be programmed for slow frame rate (5 fps and below)*/
        /* change frame rate to 5 fps by setting 1 frame length = 750 * 30 / 5 */
        SENSOR_I2C_WRITE(VMAX_ADDR, 0x94);
        SENSOR_I2C_WRITE(VMAX_ADDR + 1, 0x11);

        /* max Exposure time */
		SENSOR_I2C_WRITE(EXPOSURE_ADDR, 0x00);
		SENSOR_I2C_WRITE(EXPOSURE_ADDR + 1, 0x00);

        /* Analog and Digital gains both must be programmed for their minimum values */
		SENSOR_I2C_WRITE(PGC_ADDR, 0x00);
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR, 0xEE);
        SENSOR_I2C_WRITE(VMAX_ADDR + 1, 0x02);
    }
    
    return;
}

HI_VOID imx225_set_wdr_mode(HI_U8 u8Mode)
{
    return;
}

static HI_S32 imx225_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = FULL_LINES_DEFAULT;
    
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x40;
    
    pstAeSnsDft->u32LinesPer500ms = FULL_LINES_DEFAULT * FPS30 / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = FULL_LINES_DEFAULT - 1;
    pstAeSnsDft->u32MinIntTime = 3;    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 3;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.1;    
    pstAeSnsDft->u32MaxAgain = 4076610; 
    pstAeSnsDft->u32MinAgain = 1024;
    pstAeSnsDft->u32MaxAgainTarget = 4076610;
    pstAeSnsDft->u32MinAgainTarget = 1024;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.1;    
    pstAeSnsDft->u32MaxDgain = 1024; 
    pstAeSnsDft->u32MinDgain = 1024;
    pstAeSnsDft->u32MaxDgainTarget = 1024;
    pstAeSnsDft->u32MinDgainTarget = 1024;

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

/* the function of sensor set fps */
static HI_VOID imx225_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

	printf("fucn(%s::%d FPS30 = %d )\n", __func__, __LINE__, FPS30);
    switch(u8Fps)
    {
        case FPS30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = FULL_LINES_DEFAULT;
			pstAeSnsDft->u32MaxIntTime = FULL_LINES_DEFAULT - 1 ;
            pstAeSnsDft->u32LinesPer500ms = FULL_LINES_DEFAULT * 30 / 2;
			SENSOR_I2C_WRITE(VMAX_ADDR, LOW_8BITS(FULL_LINES_DEFAULT));
			SENSOR_I2C_WRITE(VMAX_ADDR+1, HIGH_8BITS(FULL_LINES_DEFAULT));
        break;
        
        case FPS25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = FULL_LINES_25FPS;
            pstAeSnsDft->u32MaxIntTime = FULL_LINES_25FPS - 1;
            pstAeSnsDft->u32LinesPer500ms = FULL_LINES_25FPS * FPS25 / 2;
			SENSOR_I2C_WRITE(VMAX_ADDR, LOW_8BITS(FULL_LINES_25FPS));
			SENSOR_I2C_WRITE(VMAX_ADDR+1, HIGH_8BITS(FULL_LINES_25FPS));
        break;
        
        default:
        break;
    }


    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_S32 imx225_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
{
    if (HI_NULL == pstSensorMaxResolution)
    {
        printf("null pointer when get sensor max resolution \n");
        return -1;
    }

    memset(pstSensorMaxResolution, 0, sizeof(ISP_CMOS_SENSOR_MAX_RESOLUTION));

    pstSensorMaxResolution->u32MaxWidth  = 1280;
    pstSensorMaxResolution->u32MaxHeight = 960;
    printf("func(%s::%d ) pstSensorMaxResolution->u32MaxWidth   = %d pstSensorMaxResolution->u32MaxHeight   = %d )\n", __func__, __LINE__, pstSensorMaxResolution->u32MaxWidth  , pstSensorMaxResolution->u32MaxHeight);
    return 0;
}


static HI_VOID imx225_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    
    gu32FullLines = u16FullLines;

	SENSOR_I2C_WRITE(VMAX_ADDR, LOW_8BITS(gu32FullLines));
	SENSOR_I2C_WRITE(VMAX_ADDR+1, HIGH_8BITS(gu32FullLines));
    
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 3;
    
    return;
}

static HI_VOID imx225_init_regs_info(HI_VOID)
{
#if 1
#if CMOS_IMX225_ISP_WRITE_SENSOR_ENABLE
    HI_S32 i;

    if (HI_FALSE == gsbRegInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_SSP_TYPE;
        for (i=0; i<4; i++)
        {
            g_stSnsRegsInfo.astSspData[i].u32DevAddr = 0x02;
            g_stSnsRegsInfo.astSspData[i].u32DevAddrByteNum = 1;
            g_stSnsRegsInfo.astSspData[i].u32RegAddrByteNum = 1;
            g_stSnsRegsInfo.astSspData[i].u32DataByteNum = 1;
        }
        g_stSnsRegsInfo.u32RegNum = 4;
        g_stSnsRegsInfo.astSspData[0].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astSspData[0].u32RegAddr = 0x20;
        g_stSnsRegsInfo.astSspData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astSspData[1].u32RegAddr = 0x21;
        g_stSnsRegsInfo.astSspData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astSspData[2].u32RegAddr = 0x14;
        g_stSnsRegsInfo.astSspData[3].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astSspData[3].u32RegAddr = 0x15;
    
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_FALSE;

        gsbRegInit = HI_TRUE;
    }
#endif
#else
    HI_S32 i;

    if (HI_FALSE == gsbRegInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        for (i=0; i<4; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = 0x34;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = 0x2;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = 0x1;
        }
        g_stSnsRegsInfo.u32RegNum = 4;
        g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3020;
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3021;
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3014;
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3015;
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_FALSE;

        gsbRegInit = HI_TRUE;
    }
#endif
    return;
}



/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID imx225_inttime_update(HI_U32 u32IntTime)
{
    HI_U32 u32Value = gu32FullLines - u32IntTime - 1;
    

#if CMOS_IMX225_ISP_WRITE_SENSOR_ENABLE
    imx225_init_regs_info();
    g_stSnsRegsInfo.astSspData[0].u32Data = LOW_8BITS(u32Value);
    g_stSnsRegsInfo.astSspData[1].u32Data = HIGH_8BITS(u32Value);

    //g_stSnsRegsInfo.astI2cData[0].u32Data = LOW_8BITS(u32Value);
    //g_stSnsRegsInfo.astI2cData[1].u32Data = HIGH_8BITS(u32Value);
//printf("######### %d,    %d \n",g_stSnsRegsInfo.astI2cData[0].u32Data,g_stSnsRegsInfo.astI2cData[1].u32Data);
#else
    SENSOR_I2C_WRITE(EXPOSURE_ADDR, LOW_8BITS(u32Value));
    SENSOR_I2C_WRITE(EXPOSURE_ADDR + 1, HIGH_8BITS(u32Value));

#endif
    return;
}




static HI_VOID imx225_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    HI_U32 mid, high, low;
    HI_S32 s32Match = HI_FAILURE;
    
    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }

    // binary search method to enhance efficiency
    
    low = 0;
    high = GAIN_TBL_SIZE - 1;

    do 
    {
        mid = (low + high) / 2;

        
        if (u32InTimes < gain_table[mid+1] && u32InTimes >= gain_table[mid])
        {
            s32Match = HI_SUCCESS;
            break;
        }
        
        if ( u32InTimes < gain_table[mid]) 
        {
            high = mid - 1;
        }
        else 
        {
            low = mid + 1;
        }      
            
    } while(low <= high);
 

    if (HI_SUCCESS == s32Match)
    {
        pstAeSnsGainInfo->u32GainDb = mid;
        pstAeSnsGainInfo->u32SnsTimes = gain_table[mid];
		
    }
    else
    {
        pstAeSnsGainInfo->u32GainDb = GAIN_TBL_SIZE - 1;
        pstAeSnsGainInfo->u32SnsTimes = gain_table[GAIN_TBL_SIZE - 1];
    }
  // printf("######### %d,    %d \n",pstAeSnsGainInfo->u32GainDb,pstAeSnsGainInfo->u32SnsTimes );
    return;    

}

static HI_VOID imx225_dgain_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{

    // merge dgain to again table
    
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;
    
    return;
}


static HI_VOID imx225_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    // ignore dgain
    HI_U32 u32Tmp = u32Again;

    
   u32Tmp = u32Tmp > (GAIN_TBL_SIZE - 1) ? (GAIN_TBL_SIZE - 1) : u32Tmp;
   
   
#if CMOS_IMX225_ISP_WRITE_SENSOR_ENABLE   
    imx225_init_regs_info();
    g_stSnsRegsInfo.astSspData[2].u32Data = u32Tmp & 0xff;;
    g_stSnsRegsInfo.astSspData[3].u32Data = (u32Tmp >> 8) & 0xf;

    //g_stSnsRegsInfo.astI2cData[2].u32Data = u32Tmp & 0xff; //qb
    //g_stSnsRegsInfo.astI2cData[3].u32Data = (u32Tmp >> 8) & 0xf;
    
    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
	//printf("######### %d,    %d,    %d,    %d\n",g_stSnsRegsInfo.astI2cData[0].u32Data,g_stSnsRegsInfo.astI2cData[1].u32Data,g_stSnsRegsInfo.astI2cData[2].u32Data,g_stSnsRegsInfo.astI2cData[3].u32Data);
#else
    SENSOR_I2C_WRITE(PGC_ADDR, u32Tmp);
#endif
    return;
}

static HI_S32 imx225_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;

    pstAwbSnsDft->au16GainOffset[0] = 0x1d3;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1ea;

    pstAwbSnsDft->as32WbPara[0] = -554;
    pstAwbSnsDft->as32WbPara[1] = 1332;
    pstAwbSnsDft->as32WbPara[2] = 521;
    pstAwbSnsDft->as32WbPara[3] = 171796;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -120703;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_VOID imx225_global_init()
{
    return;
}

static HI_S32 imx225_reg_init()
{	
		SENSOR_I2C_WRITE(0x200, 0x01); //Standby
		usleep(200000);
			// chip_id = 0x2
#if 0// 720p
		SENSOR_I2C_WRITE(0x205, 0x01); //12bit
		SENSOR_I2C_WRITE(0x207, 0x10);
		SENSOR_I2C_WRITE(0x209, 0x01);
		SENSOR_I2C_WRITE(0x20f, 0x00);
		SENSOR_I2C_WRITE(0x212, 0x2c);
		SENSOR_I2C_WRITE(0x213, 0x01);
		SENSOR_I2C_WRITE(0x216, 0x09);
		SENSOR_I2C_WRITE(0x218, 0xee);
		SENSOR_I2C_WRITE(0x219, 0x02);
		SENSOR_I2C_WRITE(0x21b, 0xc8);
		SENSOR_I2C_WRITE(0x21c, 0x19);
		SENSOR_I2C_WRITE(0x21d, 0xc2);
	
		SENSOR_I2C_WRITE(0x246, 0x03);
		SENSOR_I2C_WRITE(0x247, 0x06);
		SENSOR_I2C_WRITE(0x248, 0xc2);
	
		SENSOR_I2C_WRITE(0x25c, 0x20);
		SENSOR_I2C_WRITE(0x25D, 0x00);
		SENSOR_I2C_WRITE(0x25E, 0x20);
		SENSOR_I2C_WRITE(0x25F, 0x00);
	
		SENSOR_I2C_WRITE(0x270, 0x02);
		SENSOR_I2C_WRITE(0x271, 0x01);
		SENSOR_I2C_WRITE(0x29E, 0x22);
		SENSOR_I2C_WRITE(0x2A5, 0xFB);
		SENSOR_I2C_WRITE(0x2A6, 0x02);
		SENSOR_I2C_WRITE(0x2B3, 0xFF);
		SENSOR_I2C_WRITE(0x2B4, 0x01);
		SENSOR_I2C_WRITE(0x2B5, 0x42);
		SENSOR_I2C_WRITE(0x2B8, 0x10);
		SENSOR_I2C_WRITE(0x2C2, 0x01);
	
	
#endif
	
#if 1//960p
		SENSOR_I2C_WRITE(0x205, 0x01); //12bit
		SENSOR_I2C_WRITE(0x207, 0x00);
		SENSOR_I2C_WRITE(0x209, 0x01);
		SENSOR_I2C_WRITE(0x20f, 0x00);
		SENSOR_I2C_WRITE(0x212, 0x2c);
		SENSOR_I2C_WRITE(0x213, 0x01);
		SENSOR_I2C_WRITE(0x216, 0x09);
		
		  SENSOR_I2C_WRITE(0x218, 0x4c);
		  SENSOR_I2C_WRITE(0x219, 0x04);
		  SENSOR_I2C_WRITE(0x21b, 0x90);
		  SENSOR_I2C_WRITE(0x21c, 0x11);
		  
		SENSOR_I2C_WRITE(0x21d, 0xc2);
	
		SENSOR_I2C_WRITE(0x246, 0x03);
		SENSOR_I2C_WRITE(0x247, 0x06);
		SENSOR_I2C_WRITE(0x248, 0xc0);//
		SENSOR_I2C_WRITE(0x249, 0x0a);
		
		SENSOR_I2C_WRITE(0x25c, 0x20);
		SENSOR_I2C_WRITE(0x25D, 0x00);
		SENSOR_I2C_WRITE(0x25E, 0x20);
		SENSOR_I2C_WRITE(0x25F, 0x00);
	
		SENSOR_I2C_WRITE(0x270, 0x02);
		SENSOR_I2C_WRITE(0x271, 0x01);
		SENSOR_I2C_WRITE(0x29E, 0x22);
		SENSOR_I2C_WRITE(0x2A5, 0xFB);
		SENSOR_I2C_WRITE(0x2A6, 0x02);
		SENSOR_I2C_WRITE(0x2B3, 0xFF);
		SENSOR_I2C_WRITE(0x2B4, 0x01);
		SENSOR_I2C_WRITE(0x2B5, 0x42);
		SENSOR_I2C_WRITE(0x2B8, 0x10);
		SENSOR_I2C_WRITE(0x2C2, 0x01);
#endif
		
		// chip_id = 0x3
		SENSOR_I2C_WRITE(0x30F, 0x0F);
		SENSOR_I2C_WRITE(0x310, 0x0E);
		SENSOR_I2C_WRITE(0x311, 0xE7);
		SENSOR_I2C_WRITE(0x312, 0x9C);
		SENSOR_I2C_WRITE(0x313, 0x83);
		SENSOR_I2C_WRITE(0x314, 0x10);
		SENSOR_I2C_WRITE(0x315, 0x42);
		SENSOR_I2C_WRITE(0x328, 0x1E);
		SENSOR_I2C_WRITE(0x3ED, 0x38);
	
		// chip_id = 0x4
		SENSOR_I2C_WRITE(0x40C, 0xCF);
		SENSOR_I2C_WRITE(0x44C, 0x40);
		SENSOR_I2C_WRITE(0x44D, 0x03);
		SENSOR_I2C_WRITE(0x461, 0xE0);
		SENSOR_I2C_WRITE(0x462, 0x02);
		SENSOR_I2C_WRITE(0x46E, 0x2F);
		SENSOR_I2C_WRITE(0x46F, 0x30);
		SENSOR_I2C_WRITE(0x470, 0x03);
		SENSOR_I2C_WRITE(0x498, 0x00);
		SENSOR_I2C_WRITE(0x49A, 0x12);
		SENSOR_I2C_WRITE(0x49B, 0xF1);
		SENSOR_I2C_WRITE(0x49C, 0x0C);
	
		usleep(200000);
		SENSOR_I2C_WRITE(0x200, 0x00); //release standy
		usleep(200000);
		SENSOR_I2C_WRITE(0x202, 0x00); //Master mose start
		usleep(200000);
		SENSOR_I2C_WRITE(0x249, 0x80); //XVS & XHS output
		usleep(200000);
	
		printf("Sony IMX225 Sensor Initial OK!\n", __func__, __LINE__);
	
		return ;	
}


/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

HI_S32 imx225_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{  
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = imx225_reg_init;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = imx225_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = imx225_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = imx225_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = imx225_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = imx225_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = imx225_get_sensor_max_resolution;

    return 0;
}

HI_S32 imx225_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = imx225_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = imx225_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= imx225_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = imx225_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = imx225_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = imx225_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = imx225_dgain_calc_table;

    return 0;
}

HI_S32 imx225_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = imx225_get_awb_default;

    return 0;
}

int imx225_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    imx225_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IMX225_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    imx225_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, IMX225_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    imx225_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, IMX225_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int imx225_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IMX225_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, IMX225_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, IMX225_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

void SONY_IMX225_init(SENSOR_IMX225_DO_I2CRD do_i2c_read, SENSOR_IMX225_DO_I2CWR do_i2c_write)
{// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	imx225_reg_init();

	imx225_register_callback();

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
	printf("imx225 sensor 960 init success!\n");

return 0;

}



#ifdef __cplusplus
#if __cplusplus
 }
#endif
#endif /* End of #ifdef __cplusplus */
 
#endif // __IMX104_CMOS_H_



