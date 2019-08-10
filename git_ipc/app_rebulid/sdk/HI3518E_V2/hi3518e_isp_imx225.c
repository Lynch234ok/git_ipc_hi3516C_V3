#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"

#define SENSOR_NAME "imx225"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _spi_read((_add), (_ret_data)))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _spi_write((_add), (_data)))

#define IMX225_CHECK_DATA	(0x11)//0X21C
#define SENSOR_IMX225_WIDTH 1280
#define SENSOR_IMX225_HEIGHT 960
#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__IMX225_CMOS_H_)
#define __IMX225_CMOS_H_

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

#define IMX225_ID 225

/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/
#if 0

static AE_SENSOR_DEFAULT_S  g_AeDft[];
static AWB_SENSOR_DEFAULT_S g_AwbDft[];
static ISP_CMOS_DEFAULT_S   g_IspDft[];
static HI_S32 Cmos_LoadINIPara(const HI_CHAR *pcName)

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
#define HIGH_8BITS(x) ((x & 0xff00) >> 8)
#define LOW_8BITS(x) (x & 0x00ff)

static HI_U32 gu32FullLinesStd = 1325; 
static HI_U32 gu32FullLines = 1325;

#define FULL_LINES_MAX  (0xFFFF)
#define SENSOR_960P_30FPS_MODE  (1) 

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define VMAX_960P30    (1100+INCREASE_LINES)
#define VMAX_960P25     (1325+INCREASE_LINES)

#define VMAX_ADDR (0x218)  //vhs-register
#define EXPOSURE_ADDR (0x220)
#define PGC_ADDR (0x214)


static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};

static HI_U8 gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE; 


/* AE default parameter and function */

static HI_S32 imx225_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
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
            
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*25/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
    pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 1;
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
static HI_VOID imx225_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	HI_U32 fps=f32Fps;
		
    switch(fps)
    {
        case 30:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = VMAX_960P30;
			pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 1 ;
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 30 / 2;
			SENSOR_I2C_WRITE(VMAX_ADDR, LOW_8BITS(VMAX_960P30));
			SENSOR_I2C_WRITE(VMAX_ADDR+1, HIGH_8BITS(VMAX_960P30));
        break;        
        case 25:
            // Change the frame rate via changing the vertical blanking
            gu32FullLinesStd = VMAX_960P25;
            pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 1;
            pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 25 / 2;
			SENSOR_I2C_WRITE(VMAX_ADDR, LOW_8BITS(VMAX_960P25));
			SENSOR_I2C_WRITE(VMAX_ADDR+1, HIGH_8BITS(VMAX_960P25));
        break;
        default:
			gu32FullLinesStd=VMAX_960P30*30/fps;
			pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 1 ;
			pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * fps / 2;
			SENSOR_I2C_WRITE(VMAX_ADDR, LOW_8BITS(gu32FullLinesStd));
			SENSOR_I2C_WRITE(VMAX_ADDR+1, HIGH_8BITS(gu32FullLinesStd));       
        break;
    }
		pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

    return;
}

static HI_VOID imx225_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    gu32FullLines = u32FullLines;

	SENSOR_I2C_WRITE(VMAX_ADDR, LOW_8BITS(gu32FullLines));
	SENSOR_I2C_WRITE(VMAX_ADDR+1, HIGH_8BITS(gu32FullLines));
    
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 3;
    
    return;

}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID imx225_inttime_update(HI_U32 u32IntTime)
{
    HI_U32 u32Value = gu32FullLines - u32IntTime - 1;
		
   // g_stSnsRegsInfo.astSspData[0].u32Data = LOW_8BITS(u32Value);
    //g_stSnsRegsInfo.astSspData[1].u32Data = HIGH_8BITS(u32Value); 
	SENSOR_I2C_WRITE(0x220,LOW_8BITS(u32Value));
	SENSOR_I2C_WRITE(0x221,HIGH_8BITS(u32Value)); 

    return;
}

// sns gain parameters  unit DB
#define SNS_GAIN_MAX 72
#define GAIN_PRECISE 0.1
#define GAIN_TBL_SIZE (721)//#define GAIN_TBL_SIZE (int)(SNS_GAIN_MAX / GAIN_PRECISE + 1)

static HI_U32 analog_gain_table[GAIN_TBL_SIZE] =
{
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
	4076370     
};

static HI_VOID imx225_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
	int i;
    if (*pu32AgainLin >= analog_gain_table[GAIN_TBL_SIZE-1])
    {
         *pu32AgainLin = analog_gain_table[GAIN_TBL_SIZE-1];
         *pu32AgainDb = GAIN_TBL_SIZE-1;
         return ;
    }
    
    for (i = 1; i < GAIN_TBL_SIZE; i++)
    {
        if (*pu32AgainLin < analog_gain_table[i])
        {
            *pu32AgainLin = analog_gain_table[i - 1];
            *pu32AgainDb = i - 1;
            break;
        }
    }
    return;
}

static HI_VOID imx225_dgain_calc_table(HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
	*pu32DgainLin = 1024;
	*pu32DgainDb = 0;
	return ;

}

static HI_VOID imx225_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{  
	HI_U32 u32Tmp = u32Again;
	u32Tmp = u32Tmp > (GAIN_TBL_SIZE - 1) ? (GAIN_TBL_SIZE - 1) : u32Tmp;
   // g_stSnsRegsInfo.astI2cData[2].u32Data = u32Tmp & 0xff; 
    //g_stSnsRegsInfo.astI2cData[3].u32Data = (u32Tmp >> 8) & 0xf;  
   SENSOR_I2C_WRITE(0x214, u32Tmp & 0xff);
   SENSOR_I2C_WRITE(0x215,(u32Tmp >> 8) & 0xf); 

    return;
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
    {0x82,0x82,0x80,0x7c,0x70,0x69,0x5c,0x5c,0x57,0x57,0x50,0x50,0x58,0x48,0x40,0x38}
};

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


HI_S32 imx225_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = imx225_get_awb_default;

    return 0;
}

#define DMNR_CALIB_CARVE_NUM_IMX225 7

float g_coef_calib_imx225[DMNR_CALIB_CARVE_NUM_IMX225][4] = 
{
    {100.000000f, 2.000000f, 0.036662f, 8.948229f, }, 

    {211.000000f, 2.324282f, 0.036960f, 9.225851f, }, 

    {441.000000f, 2.644439f, 0.038574f, 9.328546f, }, 

    {822.000000f, 2.914872f, 0.041038f, 9.492638f, }, 

    {1659.000000f, 3.219846f, 0.046599f, 9.762368f, }, 

    {3273.000000f, 3.514946f, 0.054696f, 10.539679f, }, 

    {4217.000000f, 3.625004f, 0.057062f, 11.466070f, }, 

};


static ISP_NR_ISO_PARA_TABLE_S g_stNrIsoParaTab[HI_ISP_NR_ISO_LEVEL_MAX] = 
{
     //u16Threshold//u8varStrength//u8fixStrength//u8LowFreqSlope	
       {1500,       256-96,             256-256,            0 },  //100    //                      //           
       {1500,       256-96,             256-256,            0 },  //200    // ISO                  // ISO //u8LowFreqSlope
       {1500,       256-96,             256-256,            0 },  //400    //{400,  1200, 96,256}, //{400 , 0  }
       {1750,       256-80,             256-256,            2 },  //800    //{800,  1400, 80,256}, //{600 , 2  }
       {1500,       256-72,             256-256,            6 },  //1600   //{1600, 1200, 72,256}, //{800 , 8  }
       {1500,       256-64,             256-256,           12 },  //3200   //{3200, 1200, 64,256}, //{1000, 12 }
       {1375,       256-56,             256-256,            6 },  //6400   //{6400, 1100, 56,256}, //{1600, 6  }
       {1375,       256-48,             256-256,            0 },  //12800  //{12000,1100, 48,256}, //{2400, 0  }
       {1375,       256-48,             256-256,            0 },  //25600  //{36000,1100, 48,256}, //
       {1375,      256-128,             256-128,            0 },  //51200  //{64000,1100, 96,256}, //
       {1250,      256-192,             256-256,            0 },  //102400 //{82000,1000,240,256}, //
       {1250,      256-224,             256-256,            0 },  //204800 //                           //
       {1250,      256-224,             256-256,            0 },  //409600 //                           //
       {1250,      256-224,             256-256,            0 },  //819200 //                           //
       {1250,      256-240,             256-256,            0 },  //1638400//                           //
       {1250,      256-240,             256-256,            0 },  //3276800//                           //
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
static ISP_CMOS_RGBSHARPEN_S g_stIspRgbSharpen =
{      
  //{100,200,400,800,1600,3200,6400,12800,25600,51200,102400,204800,409600,819200,1638400,3276800};
    {0,	  0,   0,  0,   0,   0,   0,    0,    0,    1,    1,     1,     1,     1,     1,       1},/* enPixSel */
    {40, 45,  45, 50,  50,  55,  55,   60,   60,   70,   80,    90,   110,   120,   120,     120},/*SharpenUD*/
    {20, 20,  30, 45,  30,  35,  35,   40,   50,   60,   70,    90,   110,   120,   120,     120},/*SharpenD*/
    {0,   2,   4,  6,   6,  12,  30,   60,   80,    0,    0,     0,    0,     0,     0,        0},/*NoiseThd*/
    {2,   4,   8, 16,  25,  11,  12,    0,    0,    0,    0,     0,    0,     0,     0,        0},/*EdgeThd2*/
    {220,230, 200,175, 150, 120, 110,  95,   80,   70,   40,    20,   20,    20,    20,       20},/*overshootAmt*/
    {210,220, 190,140, 135, 130, 110,  95,   75,   60,   50,    50,   50,    50,    50,       50},/*undershootAmt*/
};

static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
   //{100,	200, 400, 800, 1600, 3200, 6400, 12800,	25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800};
	  {1,	  2,   4,   5,    7,   10,   12,    16,    18,    20,     22,     24,     24,     24,      24,      24},  /*UVNRThreshold*/
 	  {0,	  0,   0,   0,	  0, 	0,    0,     0,     0,	   1,      1,      2,      2,      2,       2,       2},  /*Coring_lutLimit*/
      {0,	  0,   0,  16,   34,   34,   34,    34,    34,    34,     34,     34,     34,     34,      34,      34}   /*UVNR_blendRatio*/
};

static ISP_CMOS_DPC_S g_stCmosDpc = 
{
	//0,/*IR_channel*/
	//0,/*IR_position*/
	{70,150,240,248,250,252,252,252,252,252,252,252,252,252,252,252},/*au16Strength[16]*/
	{0,0,0,0,0,0,0,0x30,0x60,0x80,0x80,0x80,0xE5,0xE5,0xE5,0xE5},/*au16BlendRatio[16]*/
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

HI_U32 imx225_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{   
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
    
    memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));	

    pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_IMX225;
    pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_imx225;
	memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);
	
    memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
	memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
	memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = 1280;
    pstDef->stSensorMaxResolution.u32MaxHeight = 960;

    return 0;
}


HI_U32 imx225_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
            
    pstBlackLevel->au16BlackLevel[0] = 0xf0;
    pstBlackLevel->au16BlackLevel[1] = 0xf0;
    pstBlackLevel->au16BlackLevel[2] = 0xf0;
    pstBlackLevel->au16BlackLevel[3] = 0xf0;

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

static HI_S32 imx225_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;
    
    bInit = HI_FALSE;    
        
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if((pstSensorImageMode->u16Width <= 1280)&&(pstSensorImageMode->u16Height <= 960))
    {
        if (pstSensorImageMode->f32Fps <= 30)
        {
            u8SensorImageMode = SENSOR_960P_30FPS_MODE;
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


HI_U32 imx225_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{
    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_SSP_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;        
        g_stSnsRegsInfo.u32RegNum = 4;        
        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            g_stSnsRegsInfo.astSspData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astSspData[i].u32DevAddr = 0x02;
            g_stSnsRegsInfo.astSspData[i].u32DevAddrByteNum = 1;
            g_stSnsRegsInfo.astSspData[i].u32RegAddrByteNum = 1;
            g_stSnsRegsInfo.astSspData[i].u32DataByteNum = 1;
        }        
        g_stSnsRegsInfo.astSspData[0].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astSspData[0].u32RegAddr = 0x20;
        g_stSnsRegsInfo.astSspData[1].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astSspData[1].u32RegAddr = 0x21;
        g_stSnsRegsInfo.astSspData[2].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astSspData[2].u32RegAddr = 0x14;   
        g_stSnsRegsInfo.astSspData[3].u8DelayFrmNum = 0;
        g_stSnsRegsInfo.astSspData[3].u32RegAddr = 0x15;
      
        bInit = HI_TRUE;
    }
    else
    {
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {
            if (g_stSnsRegsInfo.astSspData[i].u32Data == g_stPreSnsRegsInfo.astSspData[i].u32Data)
            {
                g_stSnsRegsInfo.astSspData[i].bUpdate = HI_FALSE;
            }
            else
            {
                g_stSnsRegsInfo.astSspData[i].bUpdate = HI_TRUE;
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

HI_VOID imx225_global_init()
{   
    gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_960P25; 
    gu32FullLines = VMAX_960P25;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));

}

static void imx225_reg_init()
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
 
 
#else//960P
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
	 
	 printf("======================================================\n");
	 printf("======18EV200 Sony IMX225 960P Sensor Initial OK!=====\n");
	 printf("======================================================\n");
 	 bSensorInit = HI_TRUE;
	 return ;

}

HI_S32 imx225_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = imx225_reg_init;
    //pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = imx225_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = imx225_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = imx225_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = imx225_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = imx225_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = imx225_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = imx225_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int imx225_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    imx225_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, IMX225_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    imx225_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, IMX225_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    imx225_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, IMX225_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int imx225_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, IMX225_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, IMX225_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, IMX225_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to awb lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int SONY_IMX225_init(SENSOR_DO_I2CRD do_i2c_read, 
	SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	imx225_register_callback();
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
	return s32Ret;
}
int IMX225_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
    if(ret_width && ret_height){
        *ret_width = SENSOR_IMX225_WIDTH;
        *ret_height = SENSOR_IMX225_HEIGHT;
        return 0;
    }
    return -1;
}

bool SENSOR_IMX225_probe()
{
    uint16_t ret_data1 = 0;
    uint32_t reg_val1 = 0;
    uint32_t reg_val2 = 0;
    uint32_t reg_val3 = 0;
    uint32_t reg_val4 = 0;

	//save i2c pinmux
    sdk_sys->read_reg(0x200f0040, &reg_val1);
    sdk_sys->read_reg(0x200f0044, &reg_val2);
    sdk_sys->read_reg(0x200f0048, &reg_val3);
    sdk_sys->read_reg(0x200f004c, &reg_val4);

	//set spi pinmux
    sdk_sys->write_reg(0x200f0040, 0x1);
    sdk_sys->write_reg(0x200f0044, 0x1);
    sdk_sys->write_reg(0x200f0048, 0x1);
    sdk_sys->write_reg(0x200f004c, 0x1);

    SENSOR_I2C_READ(0x21c,&ret_data1);
    if(ret_data1 == IMX225_CHECK_DATA){

        //cmos pinmux
        sdk_sys->write_reg(0x200f0080, 0x1);
        sdk_sys->write_reg(0x200f0088, 0x1);
        sdk_sys->write_reg(0x200f008c, 0x2);
        sdk_sys->write_reg(0x200f0090, 0x2);
        sdk_sys->write_reg(0x200f0094, 0x1);
        sdk_sys->write_reg(0x2003002c, 0x94001);
        return true;
    }

	//recover i2c pinmux
    sdk_sys->write_reg(0x200f0040, reg_val1);
    sdk_sys->write_reg(0x200f0044, reg_val2);
    sdk_sys->write_reg(0x200f0048, reg_val3);
    sdk_sys->write_reg(0x200f004c, reg_val4);
    return false;
}

int IMX225_get_sensor_name(char *sensor_name)
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

#endif /* __SOISC1135_CMOS_H_ */



