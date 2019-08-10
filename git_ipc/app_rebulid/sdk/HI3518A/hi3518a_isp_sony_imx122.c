//080SDK

#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_SONY_IMX122_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_SONY_IMX122_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__IMX122_CMOS_H_)
#define __IMX122_CMOS_H_

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


#define EXPOSURE_ADDR (0x208) //2:chip_id, 0C: reg addr.

#define PGC_ADDR (0x21E)
#define VMAX_ADDR (0x205)

#define IMX122_ID 122

#define CMOS_IMX122_ISP_WRITE_SENSOR_ENABLE  1
#define SENSOR_720P_30FPS_MODE   1
#define SENSOR_720P_60FPS_MODE   2
#define SENSOR_1080P_30FPS_MODE  3

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/


#if CMOS_IMX122_ISP_WRITE_SENSOR_ENABLE
 static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
 static HI_BOOL gsbRegInit = HI_FALSE;
#endif


static HI_U32 gu32FullLinesStd = 1125;
static HI_U32 gu32FullLines = 1125;
static HI_U8 gu8SensorMode = 0;

HI_U8 gu8SensorImageMode = 3;


static AWB_CCM_S g_stAwbCcm =
{
	        5000,
    	{	0x015d, 0x805d, 0x0000,
    		0x8053, 0x017b, 0x8028,
    		0x0014, 0x80e6, 0x01d1
    	},
    	4000,
        {
            0x0195, 0x8065, 0x8030,
            0x8075, 0x01ac, 0x8037,
            0x0021, 0x80df, 0x01bd
        },
        2500,
        {
            0x017a, 0x8046, 0x8034,
            0x809c, 0x01a5, 0x8009,
            0x0024, 0x81b3, 0x028f
        }
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */
    {0x80,0x80,0x80,0x80,0x80,0x78,0x70,0x50}
	
};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
	    //sharpen_alt_d
   // {0x78,0x75,0x70,0x6b,0x64,0x5e,0x56,0x3c},
	//{0x88,0x85,0x80,0x7d,0x78,0x72,0x70,0x50},
	{0x50,0x4b,0x46,0x41,0x3c,0x32,0x28,0x1e},

    //sharpen_alt_ud
 //   {0x88,0x80,0x78,0x70,0x40,0x30,0x20,0x10},
    //{0xc8,0xc0,0xb8,0xb0,0xa8,0xa0,0x72,0x3c},
	 {0x4b,0x46,0x41,0x3c,0x32,0x28,0x1e,0x14},
    //snr_thresh
 //   {0x0a,0x16,0x20,0x26,0x2b,0x32,0x3a,0x50},
    //{0x06,0x08,0x0b,0x16,0x22,0x28,0x38,0x58},
	{0x8,0xe,0x14,0x1e,0x28,0x32,0x40,0x50},
	
    //demosaic_lum_thresh
    {0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80},

    //demosaic_np_offset
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},

    //ge_strength
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37},
};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut */
    {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x0c,0x11,0x14,0x17,0x19,0x1b,0x1c,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x24,0x25,0x26,0x26,0x27,0x28,0x28,0x29,0x29,0x2a,0x2a,0x2a,
    0x2b,0x2b,0x2c,0x2c,0x2c,0x2d,0x2d,0x2d,0x2e,0x2e,0x2e,0x2f,0x2f,0x2f,0x30,0x30,0x30,
    0x30,0x31,0x31,0x31,0x31,0x32,0x32,0x32,0x32,0x32,0x33,0x33,0x33,0x33,0x34,0x34,0x34,
    0x34,0x34,0x34,0x35,0x35,0x35,0x35,0x35,0x35,0x36,0x36,0x36,0x36,0x36,0x36,0x37,0x37,
    0x37,0x37,0x37,0x37,0x37,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x39,0x39,0x39,0x39,0x39,
    0x39,0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,0x3b,
    0x3b,0x3b,0x3b,0x3b,0x3b,0x3c,0x3c,0x3c,0x3c
    },

    /* demosaic_weight_lut */
    {
    0x04,0x0c,0x11,0x14,0x17,0x19,0x1b,0x1c,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x24,0x25,0x26,0x26,0x27,0x28,0x28,0x29,0x29,0x2a,0x2a,0x2a,
    0x2b,0x2b,0x2c,0x2c,0x2c,0x2d,0x2d,0x2d,0x2e,0x2e,0x2e,0x2f,0x2f,0x2f,0x30,0x30,0x30,
    0x30,0x31,0x31,0x31,0x31,0x32,0x32,0x32,0x32,0x32,0x33,0x33,0x33,0x33,0x34,0x34,0x34,
    0x34,0x34,0x34,0x35,0x35,0x35,0x35,0x35,0x35,0x36,0x36,0x36,0x36,0x36,0x36,0x37,0x37,
    0x37,0x37,0x37,0x37,0x37,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x39,0x39,0x39,0x39,0x39,
    0x39,0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,0x3b,
    0x3b,0x3b,0x3b,0x3b,0x3b,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c
    }
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    /*vh_slope*/
    0xf5,

    /*aa_slope*/
    0x98,

    /*va_slope*/
    0xe6,

    /*uu_slope*/
 //   0xBE,
 	//0xaa,
 	0xca,

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    0x32,

    /*aa_thresh*/
    0x5b,

    /*va_thresh*/
    0x52,

    /*uu_thresh*/
    0x40,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3,

};


static ISP_CMOS_SHADING_S g_stIspShading =
{
    /* bvalid */
    1,
    
    /*shading_center_r*/
    0x393, 0x1e7 ,

    /*shading_center_g*/
    0x3a2, 0x1e0,

    /*shading_center_b*/
    0x3cc, 0x1a7,

    /*shading_table_r*/
  {   
	0x1000,0x1008,0x101e,0x1036,0x104e,0x1066,0x107d,0x1094,0x10ab,0x10c1,0x10d6,0x10eb,
   0x10ff,0x1113,0x1126,0x1139,0x114c,0x115d,0x1170,0x1182,0x1193,0x11a4,0x11b4,0x11c5,
   0x11d5,0x11e6,0x11f5,0x1205,0x1215,0x1225,0x1234,0x1243,0x1253,0x1262,0x1271,0x1281,
   0x1290,0x129f,0x12ae,0x12be,0x12cd,0x12dd,0x12ed,0x12fc,0x130c,0x131c,0x132d,0x133d,
   0x134e,0x135f,0x1370,0x1381,0x1393,0x13a5,0x13b7,0x13c9,0x13dc,0x13ef,0x1403,0x1417,
   0x142b,0x143f,0x1455,0x146b,0x1481,0x1497,0x14af,0x14c6,0x14de,0x14f7,0x1511,0x152b,
   0x1545,0x1561,0x157d,0x1599,0x15b7,0x15d5,0x15f4,0x1614,0x1635,0x1657,0x1679,0x169d,
   0x16c1,0x16e7,0x170e,0x1735,0x175d,0x1787,0x17b4,0x17e0,0x180f,0x183e,0x186e,0x18a2,
   0x18d6,0x190b,0x1943,0x197c,0x19b7,0x19f5,0x1a34,0x1a74,0x1ab9,0x1aff,0x1b47,0x1b92,
   0x1be0,0x1c30,0x1c84,0x1cda,0x1d34,0x1d92,0x1df3,0x1e58,0x1ec1,0x1f2f,0x1fa1,0x2018,
   0x2094,0x2115,0x219d,0x222a,0x22be,0x235a,0x23fc,0x24a7,0x255d},

    /*shading_table_g*/
    {
	0x1000,0x1008,0x101c,0x1033,0x104a,0x1061,0x1077,0x108d,0x10a3,0x10b8,0x10cc,0x10e0,
   0x10f4,0x1107,0x111a,0x112c,0x113e,0x114f,0x1161,0x1171,0x1182,0x1192,0x11a3,0x11b2,
   0x11c2,0x11d2,0x11e1,0x11f0,0x1200,0x120f,0x121e,0x122d,0x123b,0x124a,0x1259,0x1268,
   0x1277,0x1286,0x1295,0x12a4,0x12b3,0x12c2,0x12d1,0x12e1,0x12f1,0x1300,0x1310,0x1320,
   0x1331,0x1341,0x1352,0x1363,0x1375,0x1386,0x1398,0x13ab,0x13bd,0x13d0,0x13e4,0x13f7,
   0x140b,0x1420,0x1435,0x144a,0x1460,0x1477,0x148e,0x14a5,0x14bd,0x14d6,0x14ef,0x1509,
   0x1523,0x153f,0x155a,0x1577,0x1594,0x15b2,0x15d1,0x15f1,0x1612,0x1633,0x1656,0x1679,
   0x169d,0x16c3,0x16e9,0x1711,0x173a,0x1764,0x178f,0x17bb,0x17e9,0x1819,0x1849,0x187c,
   0x18af,0x18e5,0x191c,0x1955,0x1990,0x19cd,0x1a0c,0x1a4d,0x1a90,0x1ad6,0x1b1e,0x1b69,
   0x1bb6,0x1c06,0x1c5a,0x1cb0,0x1d0a,0x1d67,0x1dc7,0x1e2c,0x1e95,0x1f02,0x1f73,0x1fea,
   0x2065,0x20e6,0x216d,0x21fa,0x228d,0x2328,0x23ca,0x2474,0x252e},

    /*shading_table_b*/
    {
	0x1000,0x1002,0x100e,0x101d,0x102d,0x103d,0x104d,0x105d,0x106c,0x107a,0x1088,0x1096,
	0x10a3,0x10b0,0x10bc,0x10c8,0x10d4,0x10df,0x10ea,0x10f4,0x10ff,0x1109,0x1113,0x111d,
	0x1127,0x1130,0x113a,0x1143,0x114c,0x1155,0x115e,0x1167,0x1170,0x1179,0x1182,0x118b,
	0x1194,0x119e,0x11a7,0x11b0,0x11ba,0x11c3,0x11cd,0x11d7,0x11e1,0x11eb,0x11f6,0x1200,
	0x120b,0x1216,0x1221,0x122d,0x1239,0x1245,0x1251,0x125e,0x126b,0x1279,0x1286,0x1294,
	0x12a3,0x12b2,0x12c1,0x12d1,0x12e1,0x12f2,0x1303,0x1315,0x1327,0x133a,0x134d,0x1361,
	0x1376,0x138b,0x13a1,0x13b7,0x13ce,0x13e6,0x13fe,0x1418,0x1432,0x144c,0x1468,0x1485,
	0x14a2,0x14c0,0x14e0,0x1500,0x1521,0x1543,0x1567,0x158b,0x15b1,0x15d8,0x1600,0x162a,
	0x1655,0x1681,0x16af,0x16df,0x1710,0x1743,0x1777,0x17ad,0x17e6,0x1820,0x185c,0x189b,
	0x18dc,0x191f,0x1965,0x19ad,0x19f9,0x1a47,0x1a98,0x1aed,0x1b44,0x1ba0,0x1bff,0x1c63,
	0x1cca,0x1d37,0x1da8,0x1e1e,0x1e99,0x1f1b,0x1fa2,0x2031,0x209b},

    /*shading_off_center_r_g_b*/
    0x629, 0x642, 0x618,

    /*shading_table_nobe_number*/
    129
};


HI_U32 imx122_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

    pstDef->stComm.u8Rggb           = 0x0;      //1: rggb  
    pstDef->stComm.u8BalanceFe      = 0x1;

    pstDef->stDenoise.u8SinterThresh= 0x20;
    pstDef->stDenoise.u8NoiseProfile= 0x1;      //0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
    pstDef->stDenoise.u16Nr0        = 0x0;
    pstDef->stDenoise.u16Nr1        = 1528;

    pstDef->stDrc.u8DrcBlack        = 0x00;
    pstDef->stDrc.u8DrcVs           = 0x02;     // variance space
    pstDef->stDrc.u8DrcVi           = 0x08;     // variance intensity
    pstDef->stDrc.u8DrcSm           = 0x80;     // slope max
    pstDef->stDrc.u16DrcWl          = 0x8ff;    // white level

    memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));            
    memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));	
	memcpy(&pstDef->stShading, &g_stIspShading, sizeof(ISP_CMOS_SHADING_S));

    return 0;
}

HI_U32 imx122_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
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
        pstBlackLevel->au16BlackLevel[i] = 0xf0;
    }

    return 0;    
}

HI_VOID imx122_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        //TODO: finish this.
        /* Sensor must be programmed for slow frame rate (5 fps and below)*/
        /* change frame rate to 3 fps by setting 1 frame length = 1125 * (30/3) */
        SENSOR_I2C_WRITE(VMAX_ADDR, 0xF2);
        SENSOR_I2C_WRITE(VMAX_ADDR + 1, 0x2B);

        /* Analog and Digital gains both must be programmed for their minimum values */
		SENSOR_I2C_WRITE(PGC_ADDR, 0x00);
        //SENSOR_I2C_WRITE(APGC_ADDR + 1, 0x00);
	    //SENSOR_I2C_WRITE(DPGC_ADDR, 0x00);
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(VMAX_ADDR, 0x65);
        SENSOR_I2C_WRITE(VMAX_ADDR + 1, 0x04);
    }

    return;
}

HI_VOID imx122_set_wdr_mode(HI_U8 u8Mode)
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




static HI_U32 digital_gain_table[61]=
{
    1024,  1060,  1097,  1136,  1176,  1217,  1260,  1304,  1350,  1397,  1446,   1497,  1550,   1604,  1661,   1719,  
    1780,  1842,  1907,  1974,  2043,  2115,  2189,  2266,  2346,  2428,  2514,   2602,  2693,   2788,  2886,   2987,  
    3092,  3201,  3314,  3430,  3551,  3675,  3805,  3938,  4077,  4220,  4368,   4522,  4681,   4845,  5015,   5192,  
    5374,  5563,  5758,  5961,  6170,  6387,  6611,  6844,  7084,  7333,  7591,   7858,  8134  
};


static HI_U32 analog_gain_table[81] =
{
     1024 , 1060 ,  1097 ,  1136 ,  1176,  1217 , 1260 ,  1304,  1350 ,  1397 ,  1446 ,  1497 , 1550 , 1604 ,  1661 ,  1719 , 
     1780 , 1842 ,  1907 ,  1974 ,  2043,  2115 , 2189 ,  2266,  2346 ,  2428 ,  2514 ,  2602 , 2693 , 2788 ,  2886 ,  2987 , 
     3092 , 3201 ,  3314 ,  3430 ,  3551,  3675 , 3805 ,  3938,  4077 ,  4220 ,  4368 ,  4522 , 4681 , 4845 ,  5015 ,  5192 , 
     5374 , 5563 ,  5758 ,  5961 ,  6170,  6387 , 6611 ,  6844,  7084 ,  7333 ,  7591 ,  7858 , 8134 , 8420 ,  8716 ,  9022 , 
     9339 , 9667 , 10007 , 10359 , 10723, 11099 ,11489 , 11893, 12311 , 12744 , 13192 , 13655 ,14135 ,14632 , 15146 , 15678 , 
    16229     

};

static HI_S32 imx122_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    switch(gu8SensorImageMode)
    {
      case SENSOR_720P_30FPS_MODE:
        
         pstAeSnsDft->u32MaxIntTime = 748;
         pstAeSnsDft->u32LinesPer500ms = 750*30/2;
         gu32FullLinesStd = 750;

      break;
      

      case SENSOR_720P_60FPS_MODE:
        
         pstAeSnsDft->u32MaxIntTime = 748;
         pstAeSnsDft->u32LinesPer500ms = 750*60/2;
         gu32FullLinesStd = 750;
        

      break;
      

      case SENSOR_1080P_30FPS_MODE:

         pstAeSnsDft->u32MaxIntTime = 1123;
         pstAeSnsDft->u32LinesPer500ms = 1125*30/2;
         gu32FullLinesStd = 1125;
         
      break;


      default:
      
      break;
        
    }

    
    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
    
    pstAeSnsDft->u8AeCompensation = 0x58;
    
    //pstAeSnsDft->u32LinesPer500ms = 1125*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = 1123;
    pstAeSnsDft->u32MinIntTime = 2;    
    pstAeSnsDft->u32MaxIntTimeTarget = 65535;
    pstAeSnsDft->u32MinIntTimeTarget = 2;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 0.3;	
	pstAeSnsDft->u32MaxAgain = 16229;	/*the max again value is 10bit precison, the same with the table*/
	pstAeSnsDft->u32MinAgain = 1024;
	pstAeSnsDft->u32MaxAgainTarget = 16229;
	pstAeSnsDft->u32MinAgainTarget = 1024;

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 0.3;	
	pstAeSnsDft->u32MaxDgain = 8134;  /*the max again value is 10bit precison, the same with the table*/ 
	pstAeSnsDft->u32MinDgain = 1024;
	pstAeSnsDft->u32MaxDgainTarget = 8134;
	pstAeSnsDft->u32MinDgainTarget = 1024;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MaxISPDgainTarget = (1 << pstAeSnsDft->u32ISPDgainShift) + (1 << (pstAeSnsDft->u32ISPDgainShift - 1));
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

/* the function of sensor set fps */
static HI_VOID imx122_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if(gu8SensorImageMode == 3)
    {
        switch(u8Fps)
        {
            case 30:
                // Change the frame rate via changing the vertical blanking
                gu32FullLinesStd = 1125;
    			pstAeSnsDft->u32MaxIntTime = 1122;
                pstAeSnsDft->u32LinesPer500ms = 1125 * 30 / 2;
    			SENSOR_I2C_WRITE(VMAX_ADDR, 0x65);
    			SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x04);
            break;
            
            case 25:
                // Change the frame rate via changing the vertical blanking
                gu32FullLinesStd = 1350;
                pstAeSnsDft->u32MaxIntTime = 1347;
                pstAeSnsDft->u32LinesPer500ms = 1350 * 25 / 2;
    			SENSOR_I2C_WRITE(VMAX_ADDR, 0x46);
    			SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x05);
            break;
            
            default:
            break;
        }
    }
    else if(gu8SensorImageMode == 2)
    {
        switch(u8Fps)
        {
            case 60:
                // Change the frame rate via changing the vertical blanking
                gu32FullLinesStd = 750;
    			pstAeSnsDft->u32MaxIntTime = 748;
                pstAeSnsDft->u32LinesPer500ms = 750 * 60 / 2;
    			SENSOR_I2C_WRITE(VMAX_ADDR, 0xEE);
    			SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x02);
            break;
            
            case 50:
                // Change the frame rate via changing the vertical blanking
                gu32FullLinesStd = 900;
                pstAeSnsDft->u32MaxIntTime = 897;
                pstAeSnsDft->u32LinesPer500ms = 900 * 50 / 2;
    			SENSOR_I2C_WRITE(VMAX_ADDR, 0x84);
    			SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x03);
            break;
            
            default:
            break;
        }

    }
    else if(gu8SensorImageMode == 1)
    {
       switch(u8Fps)
       {
            case 30:
                // Change the frame rate via changing the vertical blanking
                gu32FullLinesStd = 750;
                pstAeSnsDft->u32MaxIntTime = 748;
                pstAeSnsDft->u32LinesPer500ms = 750 * 30 / 2;
                SENSOR_I2C_WRITE(VMAX_ADDR, 0xEE);
                SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x02);
            break;
            
            case 25:
                // Change the frame rate via changing the vertical blanking
                gu32FullLinesStd = 900;
                pstAeSnsDft->u32MaxIntTime = 897;
                pstAeSnsDft->u32LinesPer500ms = 900 * 25 / 2;
                SENSOR_I2C_WRITE(VMAX_ADDR, 0x84);
                SENSOR_I2C_WRITE(VMAX_ADDR+1, 0x03);
            break;
            
            default:
            break;
       }


    }
    else
    {

    }

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
        

    return;
}

static HI_VOID imx122_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    gu32FullLines = u16FullLines;

	SENSOR_I2C_WRITE(VMAX_ADDR, (gu32FullLines & 0x00ff));
	SENSOR_I2C_WRITE(VMAX_ADDR+1, ((gu32FullLines & 0xff00) >> 8));
    
    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 3;
    
    return;
}


static HI_VOID imx122_init_regs_info(HI_VOID)
{
#if CMOS_IMX122_ISP_WRITE_SENSOR_ENABLE
    HI_S32 i;

    if (HI_FALSE == gsbRegInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_SSP_TYPE;
        g_stSnsRegsInfo.u32RegNum = 3;
        for (i=0; i<3; i++)
        {
            g_stSnsRegsInfo.astSspData[i].u32DevAddr = 0x02;
            g_stSnsRegsInfo.astSspData[i].u32DevAddrByteNum = 1;
            g_stSnsRegsInfo.astSspData[i].u32RegAddrByteNum = 1;
            g_stSnsRegsInfo.astSspData[i].u32DataByteNum = 1;
        }
        g_stSnsRegsInfo.astSspData[0].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astSspData[0].u32RegAddr = 0x08;
        g_stSnsRegsInfo.astSspData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astSspData[1].u32RegAddr = 0x09;

        g_stSnsRegsInfo.astSspData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astSspData[2].u32RegAddr = 0x1E;
        
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_FALSE;

        gsbRegInit = HI_TRUE;
    }
#endif
    return;
}



/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID imx122_inttime_update(HI_U32 u32IntTime)
{
    HI_U32 u32Value = gu32FullLines - u32IntTime;
    
#if CMOS_IMX122_ISP_WRITE_SENSOR_ENABLE
    imx122_init_regs_info();

    g_stSnsRegsInfo.astSspData[0].u32Data = (u32Value & 0xFF);
    g_stSnsRegsInfo.astSspData[1].u32Data = (u32Value & 0xFF00) >> 8;

#else
    SENSOR_I2C_WRITE(EXPOSURE_ADDR, u32Value & 0xFF);
    SENSOR_I2C_WRITE(EXPOSURE_ADDR + 1, (u32Value & 0xFF00) >> 8);
#endif
    return;
}
static HI_VOID imx122_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
 
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;

    if (u32InTimes >= analog_gain_table[80])
    {
         pstAeSnsGainInfo->u32SnsTimes = analog_gain_table[80];
         pstAeSnsGainInfo->u32GainDb = 80;
         return ;
    }
    
    for(i = 1; i < 81; i++)
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

static HI_VOID imx122_dgain_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{

    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
 
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;
    if (u32InTimes >= digital_gain_table[60])
    {
         pstAeSnsGainInfo->u32SnsTimes = digital_gain_table[60];
         pstAeSnsGainInfo->u32GainDb = 60;
         return;
    }
    
    for(i = 1; i < 61; i++)
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

static HI_VOID imx122_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    HI_U32 u32Tmp = u32Again + u32Dgain;
    
    u32Tmp = u32Tmp > 0x8C ? 0x8C : u32Tmp;
    
#if CMOS_IMX122_ISP_WRITE_SENSOR_ENABLE 
    imx122_init_regs_info();
    g_stSnsRegsInfo.astSspData[2].u32Data = u32Tmp;
    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
#else
    SENSOR_I2C_WRITE(PGC_ADDR, u32Tmp);
#endif

    return;
}

static HI_S32 imx122_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 5000;

 /*   pstAwbSnsDft->au16GainOffset[0] = 0x1a9;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1e6;

    pstAwbSnsDft->as32WbPara[0] = 34;
    pstAwbSnsDft->as32WbPara[1] = 133;
    pstAwbSnsDft->as32WbPara[2] = -89;
    pstAwbSnsDft->as32WbPara[3] = 184401;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -136033;
*/
    pstAwbSnsDft->au16GainOffset[0] = 0x1B3;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1ea;

    pstAwbSnsDft->as32WbPara[0] = 66;
    pstAwbSnsDft->as32WbPara[1] = 61;
    pstAwbSnsDft->as32WbPara[2] = -128;
    pstAwbSnsDft->as32WbPara[3] = 193659;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -145778;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

static HI_S32 imx122_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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

static HI_U8  isp_image_mode_get(ISP_CMOS_SENSOR_IMAGE_MODE *pstSensorImageMode)
{
    if(HI_NULL == pstSensorImageMode)
    {
        printf("null pointer when set image mode111\n");
        return -1;
    }

    if((pstSensorImageMode->u16Width == 1280)&&(pstSensorImageMode->u16Height == 720))
    {
      if(pstSensorImageMode->u16Fps== 30)
      {
        gu8SensorImageMode = SENSOR_720P_30FPS_MODE;
      }
      else if(pstSensorImageMode->u16Fps== 60)
      {
          gu8SensorImageMode = SENSOR_720P_60FPS_MODE;
      }
      else
      {
         return -1;
      }
    }
    else if((pstSensorImageMode->u16Width == 1920)&&(pstSensorImageMode->u16Height == 1080))
    {
        if(pstSensorImageMode->u16Fps == 30)
        {
          gu8SensorImageMode = SENSOR_1080P_30FPS_MODE;
        }
        else
        {
           return -1;
        }
    }

    return 0;
}

static HI_S32 imx122_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE *pstSensorImageMode)
{
   HI_S32 s32Ret;
     
    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }
    
    s32Ret = isp_image_mode_get(pstSensorImageMode);
    if(s32Ret == -1 )
    {
     return -1;
    }
    
    switch(gu8SensorImageMode)
    {
      case SENSOR_720P_30FPS_MODE:
        
          //sensor_init_720p_30fps();

      break;
      
      case SENSOR_720P_60FPS_MODE:
        
          //sensor_init_720p_60fps();
          
      break;
      
      case SENSOR_1080P_30FPS_MODE:

           imx122_reg_init();

	  break;

      default:
      break;
        
    }

    return 0;
}

HI_VOID imx122_global_init()
{
     gu32FullLinesStd = 1350;
     gu32FullLines = 1350;
     gu8SensorMode = 0;

     gu8SensorImageMode = 3;
}

void imx122_reg_init()
{
	// sequence according to "Flow  Power-on to Operation Start(Sensor Master Mode)

	// chip_id = 0x2

        SENSOR_I2C_WRITE(0x200, 0x31);
        SENSOR_I2C_WRITE(0x211, 0x00);
        SENSOR_I2C_WRITE(0x22D, 0x40);
        SENSOR_I2C_WRITE(0x202, 0x0F);
        SENSOR_I2C_WRITE(0x216, 0x3C);
        SENSOR_I2C_WRITE(0x217, 0x00);
        SENSOR_I2C_WRITE(0x214, 0x00);
        SENSOR_I2C_WRITE(0x215, 0x00);
        SENSOR_I2C_WRITE(0x218, 0xC0);
        SENSOR_I2C_WRITE(0x219, 0x07);
        SENSOR_I2C_WRITE(0x2CE, 0x16);
        SENSOR_I2C_WRITE(0x2CF, 0x82);
        SENSOR_I2C_WRITE(0x2D0, 0x00);
        SENSOR_I2C_WRITE(0x29A, 0x26);
        SENSOR_I2C_WRITE(0x29B, 0x02);
        SENSOR_I2C_WRITE(0x212, 0x82);
        SENSOR_I2C_WRITE(0x20F, 0x00);
        SENSOR_I2C_WRITE(0x20D, 0x00);
        SENSOR_I2C_WRITE(0x208, 0x00);
        SENSOR_I2C_WRITE(0x209, 0x00);
        SENSOR_I2C_WRITE(0x21E, 0x00);
        SENSOR_I2C_WRITE(0x220, 0xF0);
        SENSOR_I2C_WRITE(0x221, 0x00);
        SENSOR_I2C_WRITE(0x222, 0x40);
        SENSOR_I2C_WRITE(0x205, 0x65);
        SENSOR_I2C_WRITE(0x206, 0x04);
        SENSOR_I2C_WRITE(0x203, 0x4C);
        SENSOR_I2C_WRITE(0x204, 0x04);
        SENSOR_I2C_WRITE(0x23B, 0xE0);

        // master mode start
		SENSOR_I2C_WRITE(0x22C, 0x00);


		SENSOR_I2C_WRITE(0x201, 0x00);



		SENSOR_I2C_WRITE(0x207, 0x00);

		SENSOR_I2C_WRITE(0x20A, 0x00);
		SENSOR_I2C_WRITE(0x20B, 0x00);
		SENSOR_I2C_WRITE(0x20C, 0x00);

		SENSOR_I2C_WRITE(0x20E, 0x00);

		SENSOR_I2C_WRITE(0x210, 0x00);
		

		SENSOR_I2C_WRITE(0x213, 0x40);



		SENSOR_I2C_WRITE(0x21A, 0xC9);
		SENSOR_I2C_WRITE(0x21B, 0x04);
		SENSOR_I2C_WRITE(0x21C, 0x50);
		SENSOR_I2C_WRITE(0x21D, 0x00);

		SENSOR_I2C_WRITE(0x21F, 0x31);


		SENSOR_I2C_WRITE(0x223, 0x08);
		SENSOR_I2C_WRITE(0x224, 0x30);
		SENSOR_I2C_WRITE(0x225, 0x00);
		SENSOR_I2C_WRITE(0x226, 0x80);
		SENSOR_I2C_WRITE(0x227, 0x20);
		SENSOR_I2C_WRITE(0x228, 0x34);
		SENSOR_I2C_WRITE(0x229, 0x63);
		SENSOR_I2C_WRITE(0x22A, 0x00);
		SENSOR_I2C_WRITE(0x22B, 0x00);
		

		SENSOR_I2C_WRITE(0x22E, 0x00);
		SENSOR_I2C_WRITE(0x22F, 0x02);


	    
		SENSOR_I2C_WRITE(0x230, 0x30);
		SENSOR_I2C_WRITE(0x231, 0x20);
		SENSOR_I2C_WRITE(0x232, 0x00);
		SENSOR_I2C_WRITE(0x233, 0x14);
		SENSOR_I2C_WRITE(0x234, 0x20);
		SENSOR_I2C_WRITE(0x235, 0x60);
		SENSOR_I2C_WRITE(0x236, 0x00);
		SENSOR_I2C_WRITE(0x237, 0x23);
		SENSOR_I2C_WRITE(0x238, 0x01);
		SENSOR_I2C_WRITE(0x239, 0x00);
		SENSOR_I2C_WRITE(0x23A, 0xA8);
		SENSOR_I2C_WRITE(0x23B, 0xE0);
		SENSOR_I2C_WRITE(0x23C, 0x06);
		SENSOR_I2C_WRITE(0x23D, 0x00);
		SENSOR_I2C_WRITE(0x23E, 0x10);
		SENSOR_I2C_WRITE(0x23F, 0x00);
		SENSOR_I2C_WRITE(0x240, 0x42);
		SENSOR_I2C_WRITE(0x241, 0x23);
		SENSOR_I2C_WRITE(0x242, 0x3C);
		SENSOR_I2C_WRITE(0x243, 0x01);
		SENSOR_I2C_WRITE(0x244, 0x00);
		SENSOR_I2C_WRITE(0x245, 0x00);
		SENSOR_I2C_WRITE(0x246, 0x00);
		SENSOR_I2C_WRITE(0x247, 0x00);
		SENSOR_I2C_WRITE(0x248, 0x00);
		SENSOR_I2C_WRITE(0x249, 0x00);
		SENSOR_I2C_WRITE(0x24A, 0x00);
		SENSOR_I2C_WRITE(0x24B, 0x00);
		SENSOR_I2C_WRITE(0x24C, 0x01);
		SENSOR_I2C_WRITE(0x24D, 0x00);
		SENSOR_I2C_WRITE(0x24E, 0x01);
		SENSOR_I2C_WRITE(0x24F, 0x47);
		SENSOR_I2C_WRITE(0x250, 0x10);
		SENSOR_I2C_WRITE(0x251, 0x18);
		SENSOR_I2C_WRITE(0x252, 0x12);
		SENSOR_I2C_WRITE(0x253, 0x00);
		SENSOR_I2C_WRITE(0x254, 0x00);
		SENSOR_I2C_WRITE(0x255, 0x00);
		SENSOR_I2C_WRITE(0x256, 0x00);
		SENSOR_I2C_WRITE(0x257, 0x00);
		SENSOR_I2C_WRITE(0x258, 0xE0);
		SENSOR_I2C_WRITE(0x259, 0x01);
		SENSOR_I2C_WRITE(0x25A, 0xE0);
		SENSOR_I2C_WRITE(0x25B, 0x01);
		SENSOR_I2C_WRITE(0x25C, 0x00);
		SENSOR_I2C_WRITE(0x25D, 0x00);
		SENSOR_I2C_WRITE(0x25E, 0x00);
		SENSOR_I2C_WRITE(0x25F, 0x00);
		SENSOR_I2C_WRITE(0x260, 0x00);
		SENSOR_I2C_WRITE(0x261, 0x00);
		SENSOR_I2C_WRITE(0x262, 0x76);
		SENSOR_I2C_WRITE(0x263, 0x00);
		SENSOR_I2C_WRITE(0x264, 0x01);
		SENSOR_I2C_WRITE(0x265, 0x00);
		SENSOR_I2C_WRITE(0x266, 0x00);
		SENSOR_I2C_WRITE(0x267, 0x00);
		SENSOR_I2C_WRITE(0x268, 0x00);
		SENSOR_I2C_WRITE(0x269, 0x00);
		SENSOR_I2C_WRITE(0x26A, 0x00);
		SENSOR_I2C_WRITE(0x26B, 0x00);
		SENSOR_I2C_WRITE(0x26C, 0x00);
		SENSOR_I2C_WRITE(0x26D, 0x00);
		SENSOR_I2C_WRITE(0x26E, 0x00);
		SENSOR_I2C_WRITE(0x26F, 0x00);
		SENSOR_I2C_WRITE(0x270, 0x00);
		SENSOR_I2C_WRITE(0x271, 0x00);
		SENSOR_I2C_WRITE(0x272, 0x00);
		SENSOR_I2C_WRITE(0x273, 0x01);
		SENSOR_I2C_WRITE(0x274, 0x06);
		SENSOR_I2C_WRITE(0x275, 0x07);
		SENSOR_I2C_WRITE(0x276, 0x80);
		SENSOR_I2C_WRITE(0x277, 0x00);
		SENSOR_I2C_WRITE(0x278, 0x40);
		SENSOR_I2C_WRITE(0x279, 0x08);
		SENSOR_I2C_WRITE(0x27A, 0x00);
		SENSOR_I2C_WRITE(0x27B, 0x00);
		SENSOR_I2C_WRITE(0x27C, 0x10);
		SENSOR_I2C_WRITE(0x27D, 0x00);
		SENSOR_I2C_WRITE(0x27E, 0x00);
		SENSOR_I2C_WRITE(0x27F, 0x00);
		SENSOR_I2C_WRITE(0x280, 0x06);
		SENSOR_I2C_WRITE(0x281, 0x19);
		SENSOR_I2C_WRITE(0x282, 0x00);
		SENSOR_I2C_WRITE(0x283, 0x64);
		SENSOR_I2C_WRITE(0x284, 0x00);
		SENSOR_I2C_WRITE(0x285, 0x01);
		SENSOR_I2C_WRITE(0x286, 0x00);
		SENSOR_I2C_WRITE(0x287, 0x00);
		SENSOR_I2C_WRITE(0x288, 0x00);
		SENSOR_I2C_WRITE(0x289, 0x00);
		SENSOR_I2C_WRITE(0x28A, 0x00);
		SENSOR_I2C_WRITE(0x28B, 0x00);
		SENSOR_I2C_WRITE(0x28C, 0x00);
		SENSOR_I2C_WRITE(0x28D, 0x00);
		SENSOR_I2C_WRITE(0x28E, 0x00);
		SENSOR_I2C_WRITE(0x28F, 0x00);
		SENSOR_I2C_WRITE(0x290, 0x00);
		SENSOR_I2C_WRITE(0x291, 0x00);
		SENSOR_I2C_WRITE(0x292, 0x01);
		SENSOR_I2C_WRITE(0x293, 0x01);
		SENSOR_I2C_WRITE(0x294, 0x00);
		SENSOR_I2C_WRITE(0x295, 0xFF);
		SENSOR_I2C_WRITE(0x296, 0x0F);
		SENSOR_I2C_WRITE(0x297, 0x00);
		SENSOR_I2C_WRITE(0x298, 0x26);
		SENSOR_I2C_WRITE(0x299, 0x02);

		SENSOR_I2C_WRITE(0x29C, 0x9C);
		SENSOR_I2C_WRITE(0x29D, 0x01);
		SENSOR_I2C_WRITE(0x29E, 0x39);
		SENSOR_I2C_WRITE(0x29F, 0x03);
		SENSOR_I2C_WRITE(0x2A0, 0x01);
		SENSOR_I2C_WRITE(0x2A1, 0x05);
		SENSOR_I2C_WRITE(0x2A2, 0xD0);
		SENSOR_I2C_WRITE(0x2A3, 0x07);
		SENSOR_I2C_WRITE(0x2A4, 0x00);
		SENSOR_I2C_WRITE(0x2A5, 0x02);
		SENSOR_I2C_WRITE(0x2A6, 0x0B);
		SENSOR_I2C_WRITE(0x2A7, 0x0F);
		SENSOR_I2C_WRITE(0x2A8, 0x24);
		SENSOR_I2C_WRITE(0x2A9, 0x00);
		SENSOR_I2C_WRITE(0x2AA, 0x28);
		SENSOR_I2C_WRITE(0x2AB, 0x00);
		SENSOR_I2C_WRITE(0x2AC, 0xE8);
		SENSOR_I2C_WRITE(0x2AD, 0x04);
		SENSOR_I2C_WRITE(0x2AE, 0xEC);
		SENSOR_I2C_WRITE(0x2AF, 0x04);
		SENSOR_I2C_WRITE(0x2B0, 0x00);
		SENSOR_I2C_WRITE(0x2B1, 0x00);
		SENSOR_I2C_WRITE(0x2B2, 0x03);
		SENSOR_I2C_WRITE(0x2B3, 0x05);
		SENSOR_I2C_WRITE(0x2B4, 0x00);
		SENSOR_I2C_WRITE(0x2B5, 0x0F);
		SENSOR_I2C_WRITE(0x2B6, 0x10);
		SENSOR_I2C_WRITE(0x2B7, 0x00);
		SENSOR_I2C_WRITE(0x2B8, 0x28);
		SENSOR_I2C_WRITE(0x2B9, 0x00);
		SENSOR_I2C_WRITE(0x2BA, 0xBF);
		SENSOR_I2C_WRITE(0x2BB, 0x07);
		SENSOR_I2C_WRITE(0x2BC, 0xCF);
		SENSOR_I2C_WRITE(0x2BD, 0x07);
		SENSOR_I2C_WRITE(0x2BE, 0xCF);
		SENSOR_I2C_WRITE(0x2BF, 0x07);
		SENSOR_I2C_WRITE(0x2C0, 0xCF);
		SENSOR_I2C_WRITE(0x2C1, 0x07);
		SENSOR_I2C_WRITE(0x2C2, 0xD0);
		SENSOR_I2C_WRITE(0x2C3, 0x07);
		SENSOR_I2C_WRITE(0x2C4, 0x01);
		SENSOR_I2C_WRITE(0x2C5, 0x02);
		SENSOR_I2C_WRITE(0x2C6, 0x03);
		SENSOR_I2C_WRITE(0x2C7, 0x04);
		SENSOR_I2C_WRITE(0x2C8, 0x05);
		SENSOR_I2C_WRITE(0x2C9, 0x06);
		SENSOR_I2C_WRITE(0x2CA, 0x07);
		SENSOR_I2C_WRITE(0x2CB, 0x08);
		SENSOR_I2C_WRITE(0x2CC, 0x01);
		SENSOR_I2C_WRITE(0x2CD, 0x03);

		SENSOR_I2C_WRITE(0x2D1, 0x00);
		SENSOR_I2C_WRITE(0x2D2, 0x00);
		SENSOR_I2C_WRITE(0x2D3, 0x00);
		SENSOR_I2C_WRITE(0x2D4, 0x00);
		SENSOR_I2C_WRITE(0x2D5, 0x00);
		SENSOR_I2C_WRITE(0x2D6, 0x00);
		SENSOR_I2C_WRITE(0x2D7, 0x00);
		SENSOR_I2C_WRITE(0x2D8, 0x00);
		SENSOR_I2C_WRITE(0x2D9, 0x00);
		SENSOR_I2C_WRITE(0x2DA, 0x00);
		SENSOR_I2C_WRITE(0x2DB, 0x00);
		SENSOR_I2C_WRITE(0x2DC, 0x00);
		SENSOR_I2C_WRITE(0x2DD, 0x00);
		SENSOR_I2C_WRITE(0x2DE, 0x00);
		SENSOR_I2C_WRITE(0x2DF, 0x00);
		SENSOR_I2C_WRITE(0x2E0, 0x00);
		SENSOR_I2C_WRITE(0x2E1, 0x00);
		SENSOR_I2C_WRITE(0x2E2, 0x00);
		SENSOR_I2C_WRITE(0x2E3, 0x00);
		SENSOR_I2C_WRITE(0x2E4, 0x00);
		SENSOR_I2C_WRITE(0x2E5, 0x00);
		SENSOR_I2C_WRITE(0x2E6, 0x00);
		SENSOR_I2C_WRITE(0x2E7, 0x00);
		SENSOR_I2C_WRITE(0x2E8, 0x00);
		SENSOR_I2C_WRITE(0x2E9, 0x00);
		SENSOR_I2C_WRITE(0x2EA, 0x00);
		SENSOR_I2C_WRITE(0x2EB, 0x00);
		SENSOR_I2C_WRITE(0x2EC, 0x00);
		SENSOR_I2C_WRITE(0x2ED, 0x00);
		SENSOR_I2C_WRITE(0x2EE, 0x00);
		SENSOR_I2C_WRITE(0x2EF, 0x00);
		SENSOR_I2C_WRITE(0x2F0, 0x00);
		SENSOR_I2C_WRITE(0x2F1, 0x00);
		SENSOR_I2C_WRITE(0x2F2, 0x00);
		SENSOR_I2C_WRITE(0x2F3, 0x00);
		SENSOR_I2C_WRITE(0x2F4, 0x00);
		SENSOR_I2C_WRITE(0x2F5, 0x00);
		SENSOR_I2C_WRITE(0x2F6, 0x00);
		SENSOR_I2C_WRITE(0x2F7, 0x00);
		SENSOR_I2C_WRITE(0x2F8, 0x00);
		SENSOR_I2C_WRITE(0x2F9, 0x00);
		SENSOR_I2C_WRITE(0x2FA, 0x00);
		SENSOR_I2C_WRITE(0x2FB, 0x00);
		SENSOR_I2C_WRITE(0x2FC, 0x00);
		SENSOR_I2C_WRITE(0x2FD, 0x00);
		SENSOR_I2C_WRITE(0x2FE, 0x00);
		SENSOR_I2C_WRITE(0x2FF, 0x00);

		// chip_id = 0x3
		
		SENSOR_I2C_WRITE(0x300, 0x00);
		SENSOR_I2C_WRITE(0x301, 0x00);
		SENSOR_I2C_WRITE(0x302, 0x00);
		SENSOR_I2C_WRITE(0x303, 0x00);
		SENSOR_I2C_WRITE(0x304, 0x00);
		SENSOR_I2C_WRITE(0x305, 0x00);
		SENSOR_I2C_WRITE(0x306, 0x00);
		SENSOR_I2C_WRITE(0x307, 0xFA);
		SENSOR_I2C_WRITE(0x308, 0xFA);
		SENSOR_I2C_WRITE(0x309, 0x41);
		SENSOR_I2C_WRITE(0x30A, 0x31);
		SENSOR_I2C_WRITE(0x30B, 0x38);
		SENSOR_I2C_WRITE(0x30C, 0x04);
		SENSOR_I2C_WRITE(0x30D, 0x00);
		SENSOR_I2C_WRITE(0x30E, 0x1A);
		SENSOR_I2C_WRITE(0x30F, 0x10);
		SENSOR_I2C_WRITE(0x310, 0x00);
		SENSOR_I2C_WRITE(0x311, 0x00);
		SENSOR_I2C_WRITE(0x312, 0x10);
		SENSOR_I2C_WRITE(0x313, 0x00);
		SENSOR_I2C_WRITE(0x314, 0x00);
		SENSOR_I2C_WRITE(0x315, 0x06);
		SENSOR_I2C_WRITE(0x316, 0x33);
		SENSOR_I2C_WRITE(0x317, 0x0D);
		SENSOR_I2C_WRITE(0x318, 0x00);
		SENSOR_I2C_WRITE(0x319, 0x00);
		SENSOR_I2C_WRITE(0x31A, 0x00);
		SENSOR_I2C_WRITE(0x31B, 0x00);
		SENSOR_I2C_WRITE(0x31C, 0x00);
		SENSOR_I2C_WRITE(0x31D, 0x00);
		SENSOR_I2C_WRITE(0x31E, 0x00);
		SENSOR_I2C_WRITE(0x31F, 0x00);
		SENSOR_I2C_WRITE(0x320, 0x00);
		SENSOR_I2C_WRITE(0x321, 0x80);
		SENSOR_I2C_WRITE(0x322, 0x0C);
		SENSOR_I2C_WRITE(0x323, 0x00);
		SENSOR_I2C_WRITE(0x324, 0x00);
		SENSOR_I2C_WRITE(0x325, 0x00);
		SENSOR_I2C_WRITE(0x326, 0x00);
		SENSOR_I2C_WRITE(0x327, 0x00);
		SENSOR_I2C_WRITE(0x328, 0x05);
		SENSOR_I2C_WRITE(0x329, 0x80);
		SENSOR_I2C_WRITE(0x32A, 0x00);
		SENSOR_I2C_WRITE(0x32B, 0x00);
		SENSOR_I2C_WRITE(0x32C, 0x04);
		SENSOR_I2C_WRITE(0x32D, 0x04);
		SENSOR_I2C_WRITE(0x32E, 0x00);
		SENSOR_I2C_WRITE(0x32F, 0x00);
		SENSOR_I2C_WRITE(0x330, 0x9B);
		SENSOR_I2C_WRITE(0x331, 0x71);
		SENSOR_I2C_WRITE(0x332, 0x33);
		SENSOR_I2C_WRITE(0x333, 0x37);
		SENSOR_I2C_WRITE(0x334, 0xB3);
		SENSOR_I2C_WRITE(0x335, 0x19);
		SENSOR_I2C_WRITE(0x336, 0x97);
		SENSOR_I2C_WRITE(0x337, 0xB1);
		SENSOR_I2C_WRITE(0x338, 0x19);
		SENSOR_I2C_WRITE(0x339, 0x01);
		SENSOR_I2C_WRITE(0x33A, 0x50);
		SENSOR_I2C_WRITE(0x33B, 0x00);
		SENSOR_I2C_WRITE(0x33C, 0x35);
		SENSOR_I2C_WRITE(0x33D, 0xB0);
		SENSOR_I2C_WRITE(0x33E, 0x03);
		SENSOR_I2C_WRITE(0x33F, 0xD1);
		SENSOR_I2C_WRITE(0x340, 0x71);
		SENSOR_I2C_WRITE(0x341, 0x1D);
		SENSOR_I2C_WRITE(0x342, 0x00);
		SENSOR_I2C_WRITE(0x343, 0x00);
		SENSOR_I2C_WRITE(0x344, 0x00);
		SENSOR_I2C_WRITE(0x345, 0x00);
		SENSOR_I2C_WRITE(0x346, 0x02);
		SENSOR_I2C_WRITE(0x347, 0x30);
		SENSOR_I2C_WRITE(0x348, 0x00);
		SENSOR_I2C_WRITE(0x349, 0x00);
		SENSOR_I2C_WRITE(0x34A, 0x00);
		SENSOR_I2C_WRITE(0x34B, 0x03);
		SENSOR_I2C_WRITE(0x34C, 0x00);
		SENSOR_I2C_WRITE(0x34D, 0x02);
		SENSOR_I2C_WRITE(0x34E, 0x10);
		SENSOR_I2C_WRITE(0x34F, 0xA0);
		SENSOR_I2C_WRITE(0x350, 0x00);
		SENSOR_I2C_WRITE(0x351, 0x07);
		SENSOR_I2C_WRITE(0x352, 0x40);
		SENSOR_I2C_WRITE(0x353, 0x80);
		SENSOR_I2C_WRITE(0x354, 0x00);
		SENSOR_I2C_WRITE(0x355, 0x02);
		SENSOR_I2C_WRITE(0x356, 0x50);
		SENSOR_I2C_WRITE(0x357, 0x02);
		SENSOR_I2C_WRITE(0x358, 0x23);
		SENSOR_I2C_WRITE(0x359, 0xE4);
		SENSOR_I2C_WRITE(0x35A, 0x45);
		SENSOR_I2C_WRITE(0x35B, 0x33);
		SENSOR_I2C_WRITE(0x35C, 0x79);
		SENSOR_I2C_WRITE(0x35D, 0xD1);
		SENSOR_I2C_WRITE(0x35E, 0xCC);
		SENSOR_I2C_WRITE(0x35F, 0x2F);
		SENSOR_I2C_WRITE(0x360, 0xB6);
		SENSOR_I2C_WRITE(0x361, 0xA1);
		SENSOR_I2C_WRITE(0x362, 0x17);
		SENSOR_I2C_WRITE(0x363, 0xCB);
		SENSOR_I2C_WRITE(0x364, 0xE8);
		SENSOR_I2C_WRITE(0x365, 0xC5);
		SENSOR_I2C_WRITE(0x366, 0x32);
		SENSOR_I2C_WRITE(0x367, 0xC0);
		SENSOR_I2C_WRITE(0x368, 0xA8);
		SENSOR_I2C_WRITE(0x369, 0xC6);
		SENSOR_I2C_WRITE(0x36A, 0x5E);
		SENSOR_I2C_WRITE(0x36B, 0x20);
		SENSOR_I2C_WRITE(0x36C, 0x63);
		SENSOR_I2C_WRITE(0x36D, 0x0D);
		SENSOR_I2C_WRITE(0x36E, 0x6D);
		SENSOR_I2C_WRITE(0x36F, 0x44);
		SENSOR_I2C_WRITE(0x370, 0xA6);
		SENSOR_I2C_WRITE(0x371, 0x32);
		SENSOR_I2C_WRITE(0x372, 0x24);
		SENSOR_I2C_WRITE(0x373, 0x50);
		SENSOR_I2C_WRITE(0x374, 0xC4);
		SENSOR_I2C_WRITE(0x375, 0x2F);
		SENSOR_I2C_WRITE(0x376, 0xF4);
		SENSOR_I2C_WRITE(0x377, 0x42);
		SENSOR_I2C_WRITE(0x378, 0x82);
		SENSOR_I2C_WRITE(0x379, 0x13);
		SENSOR_I2C_WRITE(0x37A, 0x90);
		SENSOR_I2C_WRITE(0x37B, 0x00);
		SENSOR_I2C_WRITE(0x37C, 0x10);
		SENSOR_I2C_WRITE(0x37D, 0x8A);
		SENSOR_I2C_WRITE(0x37E, 0x60);
		SENSOR_I2C_WRITE(0x37F, 0xC4);
		SENSOR_I2C_WRITE(0x380, 0x2F);
		SENSOR_I2C_WRITE(0x381, 0x84);
		SENSOR_I2C_WRITE(0x382, 0xF1);
		SENSOR_I2C_WRITE(0x383, 0x0B);
		SENSOR_I2C_WRITE(0x384, 0xCD);
		SENSOR_I2C_WRITE(0x385, 0x70);
		SENSOR_I2C_WRITE(0x386, 0x42);
		SENSOR_I2C_WRITE(0x387, 0x16);
		SENSOR_I2C_WRITE(0x388, 0x00);
		SENSOR_I2C_WRITE(0x389, 0x61);
		SENSOR_I2C_WRITE(0x38A, 0x0B);
		SENSOR_I2C_WRITE(0x38B, 0x29);
		SENSOR_I2C_WRITE(0x38C, 0x74);
		SENSOR_I2C_WRITE(0x38D, 0x81);
		SENSOR_I2C_WRITE(0x38E, 0x10);
		SENSOR_I2C_WRITE(0x38F, 0xBA);
		SENSOR_I2C_WRITE(0x390, 0x18);
		SENSOR_I2C_WRITE(0x391, 0x22);
		SENSOR_I2C_WRITE(0x392, 0x11);
		SENSOR_I2C_WRITE(0x393, 0xE9);
		SENSOR_I2C_WRITE(0x394, 0x60);
		SENSOR_I2C_WRITE(0x395, 0x07);
		SENSOR_I2C_WRITE(0x396, 0x09);
		SENSOR_I2C_WRITE(0x397, 0xF6);
		SENSOR_I2C_WRITE(0x398, 0x40);
		SENSOR_I2C_WRITE(0x399, 0x02);
		SENSOR_I2C_WRITE(0x39A, 0x3C);
		SENSOR_I2C_WRITE(0x39B, 0x00);
		SENSOR_I2C_WRITE(0x39C, 0x00);
		SENSOR_I2C_WRITE(0x39D, 0x00);
		SENSOR_I2C_WRITE(0x39E, 0x00);
		SENSOR_I2C_WRITE(0x39F, 0x00);
		SENSOR_I2C_WRITE(0x3A0, 0x80);
		SENSOR_I2C_WRITE(0x3A1, 0x0B);
		SENSOR_I2C_WRITE(0x3A2, 0x64);
		SENSOR_I2C_WRITE(0x3A3, 0x90);
		SENSOR_I2C_WRITE(0x3A4, 0x8D);
		SENSOR_I2C_WRITE(0x3A5, 0x6E);
		SENSOR_I2C_WRITE(0x3A6, 0x98);
		SENSOR_I2C_WRITE(0x3A7, 0x40);
		SENSOR_I2C_WRITE(0x3A8, 0x05);
		SENSOR_I2C_WRITE(0x3A9, 0xD1);
		SENSOR_I2C_WRITE(0x3AA, 0xA8);
		SENSOR_I2C_WRITE(0x3AB, 0x86);
		SENSOR_I2C_WRITE(0x3AC, 0x09);
		SENSOR_I2C_WRITE(0x3AD, 0x54);
		SENSOR_I2C_WRITE(0x3AE, 0x10);
		SENSOR_I2C_WRITE(0x3AF, 0x8D);
		SENSOR_I2C_WRITE(0x3B0, 0x6A);
		SENSOR_I2C_WRITE(0x3B1, 0xE8);
		SENSOR_I2C_WRITE(0x3B2, 0x82);
		SENSOR_I2C_WRITE(0x3B3, 0x17);
		SENSOR_I2C_WRITE(0x3B4, 0x1C);
		SENSOR_I2C_WRITE(0x3B5, 0x60);
		SENSOR_I2C_WRITE(0x3B6, 0xC1);
		SENSOR_I2C_WRITE(0x3B7, 0x31);
		SENSOR_I2C_WRITE(0x3B8, 0xAE);
		SENSOR_I2C_WRITE(0x3B9, 0xD1);
		SENSOR_I2C_WRITE(0x3BA, 0x81);
		SENSOR_I2C_WRITE(0x3BB, 0x16);
		SENSOR_I2C_WRITE(0x3BC, 0x20);
		SENSOR_I2C_WRITE(0x3BD, 0x03);
		SENSOR_I2C_WRITE(0x3BE, 0x1B);
		SENSOR_I2C_WRITE(0x3BF, 0x24);
		SENSOR_I2C_WRITE(0x3C0, 0xE0);
		SENSOR_I2C_WRITE(0x3C1, 0xC1);
		SENSOR_I2C_WRITE(0x3C2, 0x33);
		SENSOR_I2C_WRITE(0x3C3, 0xBE);
		SENSOR_I2C_WRITE(0x3C4, 0x51);
		SENSOR_I2C_WRITE(0x3C5, 0x82);
		SENSOR_I2C_WRITE(0x3C6, 0x1E);
		SENSOR_I2C_WRITE(0x3C7, 0x40);
		SENSOR_I2C_WRITE(0x3C8, 0x03);
		SENSOR_I2C_WRITE(0x3C9, 0x1C);
		SENSOR_I2C_WRITE(0x3CA, 0x34);
		SENSOR_I2C_WRITE(0x3CB, 0xD0);
		SENSOR_I2C_WRITE(0x3CC, 0x81);
		SENSOR_I2C_WRITE(0x3CD, 0x02);
		SENSOR_I2C_WRITE(0x3CE, 0x16);
		SENSOR_I2C_WRITE(0x3CF, 0x00);
		SENSOR_I2C_WRITE(0x3D0, 0x02);
		SENSOR_I2C_WRITE(0x3D1, 0x04);
		SENSOR_I2C_WRITE(0x3D2, 0x00);
		SENSOR_I2C_WRITE(0x3D3, 0x00);
		SENSOR_I2C_WRITE(0x3D4, 0x00);
		SENSOR_I2C_WRITE(0x3D5, 0x80);
		SENSOR_I2C_WRITE(0x3D6, 0x00);
		SENSOR_I2C_WRITE(0x3D7, 0x00);
		SENSOR_I2C_WRITE(0x3D8, 0x23);
		SENSOR_I2C_WRITE(0x3D9, 0x01);
		SENSOR_I2C_WRITE(0x3DA, 0x03);
		SENSOR_I2C_WRITE(0x3DB, 0x02);
		SENSOR_I2C_WRITE(0x3DC, 0x00);
		SENSOR_I2C_WRITE(0x3DD, 0x00);
		SENSOR_I2C_WRITE(0x3DE, 0x00);
		SENSOR_I2C_WRITE(0x3DF, 0x00);
		SENSOR_I2C_WRITE(0x3E0, 0x22);
		SENSOR_I2C_WRITE(0x3E1, 0x00);
		SENSOR_I2C_WRITE(0x3E2, 0x00);
		SENSOR_I2C_WRITE(0x3E3, 0x00);
		SENSOR_I2C_WRITE(0x3E4, 0x3F);
		SENSOR_I2C_WRITE(0x3E5, 0x17);
		SENSOR_I2C_WRITE(0x3E6, 0x15);
		SENSOR_I2C_WRITE(0x3E7, 0x00);
		SENSOR_I2C_WRITE(0x3E8, 0x00);
		SENSOR_I2C_WRITE(0x3E9, 0x00);
		SENSOR_I2C_WRITE(0x3EA, 0x00);
		SENSOR_I2C_WRITE(0x3EB, 0x00);
		SENSOR_I2C_WRITE(0x3EC, 0x00);
		SENSOR_I2C_WRITE(0x3ED, 0x00);
		SENSOR_I2C_WRITE(0x3EE, 0x00);
		SENSOR_I2C_WRITE(0x3EF, 0x00);
		SENSOR_I2C_WRITE(0x3F0, 0x00);
		SENSOR_I2C_WRITE(0x3F1, 0x00);
		SENSOR_I2C_WRITE(0x3F2, 0x00);
		SENSOR_I2C_WRITE(0x3F3, 0x00);
		SENSOR_I2C_WRITE(0x3F4, 0x00);
		SENSOR_I2C_WRITE(0x3F5, 0x00);
		SENSOR_I2C_WRITE(0x3F6, 0x00);
		SENSOR_I2C_WRITE(0x3F7, 0x00);
		SENSOR_I2C_WRITE(0x3F8, 0x00);
		SENSOR_I2C_WRITE(0x3F9, 0x00);
		SENSOR_I2C_WRITE(0x3FA, 0x00);
		SENSOR_I2C_WRITE(0x3FB, 0x00);
		SENSOR_I2C_WRITE(0x3FC, 0x00);
		SENSOR_I2C_WRITE(0x3FD, 0x00);
		SENSOR_I2C_WRITE(0x3FE, 0x00);
		SENSOR_I2C_WRITE(0x3FF, 0x00);
	
	// standby cancel
//	SENSOR_I2C_WRITE(0x200, 0x30);

	// waiting for internal regular stabilization
	//usleep(200000);
	
	
      // SENSOR_I2C_WRITE(0x226, 0x00);
       
       // standby cancel
       SENSOR_I2C_WRITE(0x200, 0x30);
	// XVS,XHS output start
	//SENSOR_I2C_WRITE(0x229, 0xC0);

	// waiting for image stabilization
	usleep(200000);

	printf("-------Sony IMX122 Sensor Initial OK!---(reg)----\n");
}


/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/


 HI_S32 imx122_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
 {
	 memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));
 
	 pstSensorExpFunc->pfn_cmos_sensor_init = imx122_reg_init;
	 pstSensorExpFunc->pfn_cmos_sensor_global_init = imx122_global_init;
	 pstSensorExpFunc->pfn_cmos_get_isp_default = imx122_get_isp_default;
	 pstSensorExpFunc->pfn_cmos_get_isp_black_level = imx122_get_isp_black_level;
	 pstSensorExpFunc->pfn_cmos_set_pixel_detect = imx122_set_pixel_detect;
	 pstSensorExpFunc->pfn_cmos_set_wdr_mode = imx122_set_wdr_mode;
	 pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = imx122_get_sensor_max_resolution;
     pstSensorExpFunc->pfn_cmos_set_image_mode = imx122_set_image_mode;
 
	 return 0;
 }
 
 HI_S32 imx122_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
 {
	 memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));
 
	 pstExpFuncs->pfn_cmos_get_ae_default	 = imx122_get_ae_default;
	 pstExpFuncs->pfn_cmos_fps_set			 = imx122_fps_set;
	 pstExpFuncs->pfn_cmos_slow_framerate_set= imx122_slow_framerate_set;    
	 pstExpFuncs->pfn_cmos_inttime_update	 = imx122_inttime_update;
	 pstExpFuncs->pfn_cmos_gains_update 	 = imx122_gains_update;
	 pstExpFuncs->pfn_cmos_again_calc_table  = imx122_again_calc_table;   
	 pstExpFuncs->pfn_cmos_dgain_calc_table  = imx122_dgain_calc_table;
 
	 return 0;
 }
 
 HI_S32 imx122_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
 {
	 memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));
 
	 pstExpFuncs->pfn_cmos_get_awb_default = imx122_get_awb_default;
 
	 return 0;
 }
 
 int imx122_sensor_register_callback(void)
 {
	 HI_S32 s32Ret;
	 ALG_LIB_S stLib;
	 ISP_SENSOR_REGISTER_S stIspRegister;
	 AE_SENSOR_REGISTER_S  stAeRegister;
	 AWB_SENSOR_REGISTER_S stAwbRegister;
 
	 imx122_init_sensor_exp_function(&stIspRegister.stSnsExp);
	 s32Ret = HI_MPI_ISP_SensorRegCallBack(IMX122_ID, &stIspRegister);
	 if (s32Ret)
	 {
		 printf("sensor register callback function failed!\n");
		 return s32Ret;
	 }
	 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AE_LIB_NAME);
	 imx122_init_ae_exp_function(&stAeRegister.stSnsExp);
	 s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, IMX122_ID, &stAeRegister);
	 if (s32Ret)
	 {
		 printf("sensor register callback function to ae lib failed!\n");
		 return s32Ret;
	 }
 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
	 imx122_init_awb_exp_function(&stAwbRegister.stSnsExp);
	 s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, IMX122_ID, &stAwbRegister);
	 if (s32Ret)
	 {
		 printf("sensor register callback function to ae lib failed!\n");
		 return s32Ret;
	 }
	 
	 return 0;
 }
 
 int imx122_sensor_unregister_callback(void)
 {
	 HI_S32 s32Ret;
	 ALG_LIB_S stLib;
 
	 s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IMX122_ID);
	 if (s32Ret)
	 {
		 printf("sensor unregister callback function failed!\n");
		 return s32Ret;
	 }
	 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AE_LIB_NAME);
	 s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, IMX122_ID);
	 if (s32Ret)
	 {
		 printf("sensor unregister callback function to ae lib failed!\n");
		 return s32Ret;
	 }
 
	 stLib.s32Id = 0;
	 strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
	 s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, IMX122_ID);
	 if (s32Ret)
	 {
		 printf("sensor unregister callback function to ae lib failed!\n");
		 return s32Ret;
	 }
	 
	 return 0;
 }



void SONY_IMX122_init(SENSOR_SONY_IMX122_DO_I2CRD do_i2c_read, SENSOR_SONY_IMX122_DO_I2CWR do_i2c_write)
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	imx122_reg_init();

	imx122_sensor_register_callback();

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
	printf("Sony IMX122 sensor 1080P30fps init success!\n");

}

#ifdef __cplusplus
#if __cplusplus
 }
#endif
#endif /* End of #ifdef __cplusplus */
 
#endif // __IMX104_CMOS_H_

