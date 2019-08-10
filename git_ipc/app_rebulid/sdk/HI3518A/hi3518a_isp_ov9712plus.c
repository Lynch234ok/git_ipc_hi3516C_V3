#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"


static SENSOR_OV9712_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_OV9712_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__OV9712PLUS_CMOS_H_)
#define __OV9712PLUS_CMOS_H_



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

#define OV9712PLUS_ID 9712

	/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
	/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance 					  */
	/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c										  */
#define CMOS_OV9712PLUS_ISP_WRITE_SENSOR_ENABLE (1)
	
	/*change this value to 1 to make the image looks more sharpen*/    
#define CMOS_OV9712PLUS_MORE_SHARPEN (1)
	
	/* To change the mode of slow framerate. When the value is 0, add the line numbers to slow framerate.
	 * When the value is 1, add the line length to slow framerate. */
#define CMOS_OV9712PLUS_SLOW_FRAMERATE_MODE (1)
	
	/****************************************************************************
	 * local variables															  *
	 ****************************************************************************/


static const unsigned int sensor_i2c_addr = 0x60;
static unsigned int sensor_addr_byte = 0x1;
static unsigned int sensor_data_byte = 0x1;



static HI_U8 gu8SensorMode = 0;
static HI_U32 gu8Fps = 30;
static HI_U32 gu32FullLinesStd = 810;
static HI_U32 gu32FullLines = 810;

#if CMOS_OV9712PLUS_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif


static AWB_CCM_S g_stAwbCcm =
{
     5000,
     {
		0x012f, 0x8027, 0x8008,
		0x8021, 0x011d, 0x0004,
		0x0013, 0x80dd, 0x01ca
     },
     
     3200,
     {
	    0x0103,0x8002,0x8001,
		0x8007,0x00d3,0x0034,
		0x0032,0x81cb,0x029a
     },
     2800,
     {
		0x0140,0x8030,0x8021,
		0x8017,0x00e6,0x0030,
		0x0000,0x81ad,0x02be
     }
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */
    {0x90,0x90,0x80,0x80,0x68,0x48,0x35,0x30}
};


static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,
    
#if CMOS_OV9712PLUS_MORE_SHARPEN
    //sharpen_alt_d
    //{80,75,70,65,55,45,35,20},
    //{0x78,0x6d,0x63,0x62,0x61,0x60,0x5f,0x5e},
    {120,90,70,65,55,45,35,20},

    //sharpen_alt_ud
    //{75,70,65,60,50,40,25,10},
    //{0xa0,0x88,0x78,0x6E,0x60,0x44,0x2f,0x20},
    {90,82,75,68,58,50,25,10},

    //snr_thresh
    {0x23,0x28,0x2b,0x35,0x3f,0x46,0x4b,0x4f},
#else    
    /* sharpen_alt_d */
    {0x8e,0x8b,0x88,0x83,0x7d,0x76,0x75,0x74},
        
    /* sharpen_alt_ud */
    {0x8f,0x89,0x7e,0x78,0x6f,0x44,0x40,0x35},
        
    /* snr_thresh */
    {0x19,0x1e,0x2d,0x32,0x39,0x3f,0x48,0x4b},
#endif    
        
    //demosaic_lum_thresh
    {80,64,64,48,48,32,32,16},
    //{0x40,0x60,0x80,0x80,0x80,0x80,0x80,0x80},

    //demosaic_np_offset
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x30},

    //ge_strength
    {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55},

};

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTable =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut */
    {   
    0x00,0x04,0x10,0x16,0x1a,0x1d,0x1f,0x21,0x23,0x24,0x26,0x27,0x28,0x29,0x2a,0x2a,
    0x2b,0x2c,0x2d,0x2d,0x2e,0x2e,0x2f,0x2f,0x30,0x30,0x31,0x31,0x32,0x32,0x33,0x33,
    0x33,0x34,0x34,0x34,0x35,0x35,0x35,0x36,0x36,0x36,0x37,0x37,0x37,0x37,0x38,0x38,
    0x38,0x38,0x39,0x39,0x39,0x39,0x39,0x3a,0x3a,0x3a,0x3a,0x3b,0x3b,0x3b,0x3b,0x3b,
    0x3b,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3d,0x3d,0x3d,0x3d,0x3d,0x3d,0x3e,0x3e,0x3e,
    0x3e,0x3e,0x3e,0x3e,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x40,0x40,0x40,0x40,0x40,
    0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x42,0x42,0x42,0x42,
    0x42,0x42,0x42,0x42,0x42,0x42,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43
    },

    /* demosaic_weight_lut */
    {0, 27, 31, 33, 35, 36, 37, 38, 39, 40, 40, 41, 41, 42, 42, 43,
    43, 43, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47,
    47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49,
    49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51,
    51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
    53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55}
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    205,
 //	0xda,

    /*aa_slope*/
    191,
    //0xa9,
#if CMOS_OV9712PLUS_MORE_SHARPEN
    /*va_slope*/
    193,
  //  0x193,

    /*uu_slope*/
    200,
#else
    /*va_slope*/
    0xec,

    /*uu_slope*/
    0x89,
#endif

    /*sat_slope*/
    0x5d,

    /*ac_slope*/
    0xcf,

    /*vh_thresh*/
    16,
 //   0xa9,

    /*aa_thresh*/
    10,
 //    0x23,

    /*va_thresh*/
    10,
 //   0xa6,

    /*uu_thresh*/
    10,
  //  0x2d,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3
};

static ISP_CMOS_SHADING_S g_stIspShading =
{
    /* bvalid */
    1,
    
    /*shading_center_r*/
    0x28f, 0x16c,

    /*shading_center_g*/
    0x28e, 0x177,

    /*shading_center_b*/
    0x293, 0x16c,

    /*shading_table_r*/
  {   
    0x1000,0x100c,0x1035,0x1065,0x1099,0x10cd,0x1102,0x1137,0x116c,0x11a1,0x11d5,0x1209,
    0x123d,0x126f,0x12a2,0x12d3,0x1303,0x1335,0x1365,0x1394,0x13c2,0x13f0,0x141c,0x1449,
    0x1475,0x149f,0x14c9,0x14f3,0x151b,0x1543,0x156a,0x1590,0x15b5,0x15d9,0x15fd,0x1620,
    0x1642,0x1663,0x1684,0x16a3,0x16c2,0x16e0,0x16fd,0x1719,0x1735,0x1750,0x176a,0x1783,
    0x179b,0x17b3,0x17ca,0x17e0,0x17f5,0x180a,0x181e,0x1831,0x1843,0x1855,0x1866,0x1877,
    0x1887,0x1896,0x18a4,0x18b2,0x18bf,0x18cb,0x18d7,0x18e3,0x18ef,0x18f8,0x1903,0x190d,
    0x1916,0x191e,0x1926,0x192d,0x1935,0x193c,0x1942,0x1948,0x194d,0x1953,0x1958,0x195d,
    0x1961,0x1964,0x1968,0x196c,0x196f,0x1972,0x1975,0x1977,0x1978,0x197b,0x197d,0x197e,
    0x1981,0x1982,0x1983,0x1984,0x1986,0x1987,0x1988,0x1989,0x198a,0x198a,0x198a,0x198c,
    0x198d,0x198e,0x198e,0x198f,0x1990,0x1991,0x1992,0x1993,0x1994,0x1996,0x1997,0x1999,
    0x199a,0x199c,0x199d,0x199f,0x19a2,0x19a5,0x19a7,0x19aa,0x19ac},

    /*shading_table_g*/
    {
    0x1000,0x1005,0x1028,0x1055,0x1087,0x10bb,0x10f0,0x1125,0x115b,0x1190,0x11c5,0x11fa,
    0x122f,0x1263,0x1296,0x12c9,0x12fc,0x132d,0x135e,0x138f,0x13be,0x13ed,0x141b,0x1449,
    0x1475,0x14a1,0x14cc,0x14f6,0x1520,0x1548,0x1570,0x1597,0x15bd,0x15e2,0x1606,0x1629,
    0x164b,0x166d,0x168e,0x16ad,0x16cc,0x16ea,0x1707,0x1724,0x173f,0x175a,0x1773,0x178c,
    0x17a4,0x17bc,0x17d2,0x17e8,0x17fc,0x1810,0x1824,0x1836,0x1848,0x1859,0x1869,0x1879,
    0x1888,0x1896,0x18a4,0x18b1,0x18bd,0x18c9,0x18d4,0x18df,0x18e9,0x18f2,0x18fb,0x1904,
    0x190c,0x1914,0x191b,0x1921,0x1928,0x192d,0x1933,0x1938,0x193d,0x1941,0x1945,0x1949,
    0x194d,0x1950,0x1953,0x1956,0x1959,0x195b,0x195d,0x195f,0x1961,0x1963,0x1965,0x1966,
    0x1967,0x1969,0x196a,0x196b,0x196d,0x196e,0x196f,0x1970,0x1972,0x1973,0x1974,0x1975,
    0x1977,0x1978,0x197a,0x197c,0x197e,0x1980,0x1982,0x1984,0x1987,0x1989,0x198c,0x198f,
    0x1992,0x1996,0x199a,0x199e,0x19a2,0x19a6,0x19ab,0x19b0,0x19b4},

    /*shading_table_b*/
    {
    0x1000,0x1000,0x1011,0x102f,0x1054,0x107c,0x10a5,0x10cf,0x10fa,0x1125,0x1150,0x117c,
    0x11a7,0x11d1,0x11fc,0x1226,0x1250,0x1279,0x12a2,0x12ca,0x12f2,0x131a,0x1340,0x1367,
    0x138c,0x13b1,0x13d6,0x13fa,0x141d,0x143f,0x1461,0x1483,0x14a3,0x14c3,0x14e3,0x1502,
    0x1520,0x153d,0x155a,0x1576,0x1591,0x15ac,0x15c6,0x15e0,0x15f8,0x1610,0x1628,0x163f,
    0x1655,0x166b,0x1680,0x1694,0x16a8,0x16bb,0x16cd,0x16df,0x16f1,0x1701,0x1712,0x1721,
    0x1730,0x173f,0x174d,0x175b,0x1768,0x1774,0x1780,0x178c,0x1797,0x17a2,0x17ac,0x17b6,
    0x17bf,0x17c8,0x17d1,0x17d9,0x17e1,0x17e8,0x17ef,0x17f6,0x17fd,0x1803,0x1809,0x180f,
    0x1814,0x1819,0x181e,0x1822,0x1827,0x182b,0x182f,0x1833,0x1836,0x183a,0x183d,0x1840,
    0x1843,0x1846,0x1849,0x184b,0x184e,0x1850,0x1853,0x1855,0x1857,0x185a,0x185c,0x185e,
    0x1860,0x1862,0x1864,0x1867,0x1869,0x186b,0x186d,0x1870,0x1872,0x1875,0x1877,0x187a,
    0x187d,0x1880,0x1883,0x1886,0x1889,0x188c,0x1890,0x1894,0x1895},

    /*shading_off_center_r_g_b*/
    0xef0, 0xec3, 0xecd,

    /*shading_table_nobe_number*/
    129
};

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,
    
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
		0xfff
    /*0x0, 0x36, 0x6a, 0x9e, 0xd1, 0x103, 0x134, 0x164, 0x193, 0x1c2, 0x1ef, 0x21c, 0x248, 0x274, 0x29e, 0x2c9, 
		0x2f2, 0x31b, 0x343, 0x36a, 0x391, 0x3b7, 0x3dd, 0x402, 0x426, 0x44a, 0x46d, 0x490, 0x4b3, 0x4d4, 0x4f6, 0x517, 
		0x537, 0x557, 0x576, 0x595, 0x5b4, 0x5d2, 0x5f0, 0x60d, 0x62a, 0x647, 0x663, 0x67f, 0x69a, 0x6b5, 0x6d0, 0x6ea, 
		0x704, 0x71e, 0x738, 0x751, 0x769, 0x782, 0x79a, 0x7b2, 0x7c9, 0x7e1, 0x7f8, 0x80e, 0x825, 0x83b, 0x851, 0x866, 
		0x87c, 0x891, 0x8a6, 0x8bb, 0x8cf, 0x8e3, 0x8f7, 0x90b, 0x91f, 0x932, 0x945, 0x958, 0x96b, 0x97d, 0x98f, 0x9a1, 
		0x9b3, 0x9c5, 0x9d7, 0x9e8, 0x9f9, 0xa0a, 0xa1b, 0xa2b, 0xa3c, 0xa4c, 0xa5c, 0xa6c, 0xa7c, 0xa8c, 0xa9b, 0xaab, 
		0xaba, 0xac9, 0xad8, 0xae6, 0xaf5, 0xb03, 0xb12, 0xb20, 0xb2e, 0xb3c, 0xb4a, 0xb57, 0xb65, 0xb72, 0xb80, 0xb8d, 
		0xb9a, 0xba7, 0xbb4, 0xbc0, 0xbcd, 0xbd9, 0xbe6, 0xbf2, 0xbfe, 0xc0a, 0xc16, 0xc22, 0xc2e, 0xc39, 0xc45, 0xc50, 
		0xc5c, 0xc67, 0xc72, 0xc7d, 0xc88, 0xc93, 0xc9e, 0xca8, 0xcb3, 0xcbe, 0xcc8, 0xcd2, 0xcdd, 0xce7, 0xcf1, 0xcfb, 
		0xd05, 0xd0f, 0xd18, 0xd22, 0xd2c, 0xd35, 0xd3f, 0xd48, 0xd52, 0xd5b, 0xd64, 0xd6d, 0xd76, 0xd7f, 0xd88, 0xd91, 
		0xd9a, 0xda2, 0xdab, 0xdb4, 0xdbc, 0xdc5, 0xdcd, 0xdd5, 0xdde, 0xde6, 0xdee, 0xdf6, 0xdfe, 0xe06, 0xe0e, 0xe16, 
		0xe1e, 0xe25, 0xe2d, 0xe35, 0xe3c, 0xe44, 0xe4c, 0xe53, 0xe5a, 0xe62, 0xe69, 0xe70, 0xe77, 0xe7f, 0xe86, 0xe8d, 
		0xe94, 0xe9b, 0xea2, 0xea8, 0xeaf, 0xeb6, 0xebd, 0xec3, 0xeca, 0xed1, 0xed7, 0xede, 0xee4, 0xeeb, 0xef1, 0xef7, 
		0xefe, 0xf04, 0xf0a, 0xf10, 0xf17, 0xf1d, 0xf23, 0xf29, 0xf2f, 0xf35, 0xf3b, 0xf41, 0xf46, 0xf4c, 0xf52, 0xf58, 
		0xf5d, 0xf63, 0xf69, 0xf6e, 0xf74, 0xf7a, 0xf7f, 0xf85, 0xf8a, 0xf8f, 0xf95, 0xf9a, 0xf9f, 0xfa5, 0xfaa, 0xfaf, 
		0xfb4, 0xfba, 0xfbf, 0xfc4, 0xfc9, 0xfce, 0xfd3, 0xfd8, 0xfdd, 0xfe2, 0xfe7, 0xfec, 0xff1, 0xff5, 0xffa, 0xfff, 
		0xfff*/}
};



HI_U32 ov9712plus_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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





HI_U32 ov9712plus_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    pstBlackLevel->au16BlackLevel[0] = 80;
    pstBlackLevel->au16BlackLevel[1] = 64;
    pstBlackLevel->au16BlackLevel[2] = 64;
    pstBlackLevel->au16BlackLevel[3] = 64;

   
    return 0;    
} 



HI_VOID ov9712plus_set_pixel_detect(HI_BOOL bEnable)
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



HI_VOID ov9712plus_set_wdr_mode(HI_U8 u8Mode)
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


static HI_S32 ov9712plus_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = 810;
    
    pstAeSnsDft->u32LinesPer500ms = 810*30/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;
    
    //gu8Fps = 30;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->u32MaxIntTime = 806;
    pstAeSnsDft->u32MinIntTime = 2;
    
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
    pstAeSnsDft->u32MaxAgain = 4;  /* 1, 2, 4, ... 16 (0~24db, unit is 6db) */
    pstAeSnsDft->u32MinAgain = 0;
    pstAeSnsDft->u32MaxAgainTarget = 4;
    pstAeSnsDft->u32MinAgainTarget = 0;
    

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0625;
    pstAeSnsDft->u32MaxDgain = 31;  /* 1 ~ 31/16, unit is 1/16 */
    pstAeSnsDft->u32MinDgain = 16;
    pstAeSnsDft->u32MaxDgainTarget = 32;
    pstAeSnsDft->u32MinDgainTarget = 16; 

    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

static HI_S32 ov9712plus_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID ov9712plus_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 tp = 1692;
#if 0
    switch(fps)
    {
        case 30:
            tp = 1688;
            break;
        case 25:
            tp = 2028;
            break;
        default:
            break;
    }
#endif
    tp = 1692 * 30 / u8Fps + 3;
    SENSOR_I2C_WRITE(0x2a, tp & 0xfc);
    SENSOR_I2C_WRITE(0x2b, (tp & 0xff00) >> 8);

    pstAeSnsDft->u32MaxIntTime = 806;
    gu32FullLinesStd = 810;
    gu8Fps = u8Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * u8Fps / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    
    return;
}

static HI_VOID ov9712plus_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
/* Mode 1 : slow framerate by add the time of each line. */
#if CMOS_OV9712PLUS_SLOW_FRAMERATE_MODE
    HI_U32 u32Tp = 50760;   /* (0x69c * 30) = 50760*/
    HI_U16 u16SlowFrameRate = (u16FullLines << 8) / gu32FullLinesStd;
    u32Tp = (((u32Tp * u16SlowFrameRate) / gu8Fps) >> 8) + 3;
    if (u32Tp > 0x2000)     /* the register 0x2a adn 0x2b's max value is 0x2000 */
    {
        u32Tp = 0x2000;
        u16SlowFrameRate = ((gu8Fps * u32Tp) << 8) / 50760;
        printf("Warning! The slow_framerate is out of ov9712's range!\n");
    }

    
    pstAeSnsDft->u32LinesPer500ms = gu32FullLines * (gu8Fps << 8) / (2*u16SlowFrameRate);
    pstAeSnsDft->u32MaxIntTime = ((gu32FullLines * u16SlowFrameRate) >> 8) - 4;
    #if CMOS_OV9712_ISP_WRITE_SENSOR_ENABLE
    g_stSnsRegsInfo.u32RegNum = 5;
    g_stSnsRegsInfo.astI2cData[3].u32Data = u32Tp & 0xfc;
    g_stSnsRegsInfo.astI2cData[4].u32Data = (u32Tp & 0xff00) >> 8;
    #else
    SENSOR_I2C_WRITE(0x2a, u32Tp & 0xfc);
    SENSOR_I2C_WRITE(0x2b, (u32Tp & 0xff00) >> 8);    
    #endif
#else
/* Mode 2 : slow framerate by add the lines of each frame. */
    #if CMOS_OV9712PLUS_ISP_WRITE_SENSOR_ENABLE
    HI_U32 u32VblankingLines;
    static HI_U32 u32LastVblankingLines = 0;

    gu32FullLines = u16FullLines;
    u32VblankingLines = gu32FullLines - gu32FullLinesStd;

    /*avoid flicker in slow frame rate*/
    if(u32LastVblankingLines < u32VblankingLines)
    {
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[4].bDelayCfg = HI_FALSE;        
    }
    else
    {
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_TRUE;
        g_stSnsRegsInfo.astI2cData[4].bDelayCfg = HI_TRUE;
    }
    g_stSnsRegsInfo.u32RegNum = 5;
    g_stSnsRegsInfo.astI2cData[3].u32Data = u32VblankingLines & 0xff;
    g_stSnsRegsInfo.astI2cData[4].u32Data = (u32VblankingLines & 0xff00) >> 8;
    u32LastVblankingLines = u32VblankingLines;
    #else
    HI_U32 u32VblankingLines;

    gu32FullLines = u16FullLines;
    u32VblankingLines = gu32FullLines - gu32FullLinesStd;
    
    SENSOR_I2C_WRITE(0x2d, u32VblankingLines & 0xff);
    SENSOR_I2C_WRITE(0x2e, (u32VblankingLines & 0xff00) >> 8);
	#endif

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;
#endif
    return;
}



static HI_VOID ov9712plus_init_regs_info(HI_VOID)
{
#if CMOS_OV9712PLUS_ISP_WRITE_SENSOR_ENABLE
    HI_S32 i;
    static HI_BOOL bInit = HI_FALSE;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u32RegNum = 3;
        for (i=0; i<5; i++)
        {
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }
        g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x10;
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x16;
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_TRUE;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x00;
    #if CMOS_OV9712PLUS_SLOW_FRAMERATE_MODE
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_TRUE;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x2a;
        g_stSnsRegsInfo.astI2cData[4].bDelayCfg = HI_TRUE;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x2b;
    #else
        g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x2d;
        g_stSnsRegsInfo.astI2cData[4].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x2e;
    #endif
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_TRUE;

        bInit = HI_TRUE;
    }
#endif
    return;
}



/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ov9712plus_inttime_update(HI_U32 u32IntTime)
{
#if CMOS_OV9712PLUS_ISP_WRITE_SENSOR_ENABLE
    ov9712plus_init_regs_info();
    g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime & 0xFF;
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime >> 8) & 0xFF;
#else 
    //refresh the sensor setting every frame to avoid defect pixel error
    SENSOR_I2C_WRITE(0x10, u32IntTime&0xFF);
    SENSOR_I2C_WRITE(0x16, (u32IntTime>>8)&0xFF);
#endif
    return;
}


static HI_VOID ov9712plus_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
    HI_U8 u8High, u8Low;
    switch (u32Again)
    {
        case 0 :    /* 0db, 1 multiplies */
            u8High = 0x00;
            break;
        case 1 :    /* 6db, 2 multiplies */
            u8High = 0x10;
            break;
        case 2 :    /* 12db, 4 multiplies */
            u8High = 0x30;
            break;
        case 3 :    /* 18db, 8 multiplies */
            u8High = 0x70;
            break;
        case 4 :    /* 24db, 16 multiplies */
            u8High = 0xf0;
            break;
        default:
            u8High = 0x00;
            break;
    }

    u8Low = (u32Dgain - 16) & 0xf;

#if CMOS_OV9712PLUS_ISP_WRITE_SENSOR_ENABLE
    ov9712plus_init_regs_info();
    g_stSnsRegsInfo.astI2cData[2].u32Data = (u8High | u8Low);
    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
    if (5 == g_stSnsRegsInfo.u32RegNum)
    {
        g_stSnsRegsInfo.u32RegNum = 3;
    }
#else
    SENSOR_I2C_WRITE(0x00, (u8High | u8Low));
#endif

    return;
}



static HI_S32 ov9712plus_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    
    pstAwbSnsDft->u16WbRefTemp = 5000;

    pstAwbSnsDft->au16GainOffset[0] = 0x1dc;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x1b6;
    //{0x1dc, 0x100, 0x100, 0x1b6}, 

    // WB curve parameters, must keep consistent with reference color temperature.
    //{-129,613,228,172790,128,-121159},

    pstAwbSnsDft->as32WbPara[0] = -129;
    pstAwbSnsDft->as32WbPara[1] = 613;
    pstAwbSnsDft->as32WbPara[2] = 228;
    pstAwbSnsDft->as32WbPara[3] = 172790;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -121159;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

static HI_VOID ov9712plus_global_init()
{
	gu8SensorMode = 0;
}

void ov9712plus_reg_init()
{
		//Reset 
	SENSOR_I2C_WRITE(0x12, 0x80);
	SENSOR_I2C_WRITE(0x09, 0x10);

	//Core Settings
	SENSOR_I2C_WRITE(0x1e, 0x07);
	SENSOR_I2C_WRITE(0x5f, 0x18);
	SENSOR_I2C_WRITE(0x69, 0x04);
	SENSOR_I2C_WRITE(0x65, 0x2a);
	SENSOR_I2C_WRITE(0x68, 0x0a);
	SENSOR_I2C_WRITE(0x39, 0x28);
	SENSOR_I2C_WRITE(0x4d, 0x90);
	SENSOR_I2C_WRITE(0xc1, 0x80);
	SENSOR_I2C_WRITE(0x0c, 0x30);
	SENSOR_I2C_WRITE(0x6d, 0x02);

	//DSP
	//SENSOR_I2C_WRITE(0x96, 0xf1);
	SENSOR_I2C_WRITE(0x96, 0x01);
	SENSOR_I2C_WRITE(0xbc, 0x68);

	//Resolution and Format
	SENSOR_I2C_WRITE(0x12, 0x00);
	SENSOR_I2C_WRITE(0x3b, 0x00);
	SENSOR_I2C_WRITE(0x97, 0x80);
	SENSOR_I2C_WRITE(0x17, 0x25);
	SENSOR_I2C_WRITE(0x18, 0xA2);
	SENSOR_I2C_WRITE(0x19, 0x01);
	SENSOR_I2C_WRITE(0x1a, 0xCA);
	SENSOR_I2C_WRITE(0x03, 0x0A);
	SENSOR_I2C_WRITE(0x32, 0x07);
	SENSOR_I2C_WRITE(0x98, 0x00);
	SENSOR_I2C_WRITE(0x99, 0x28);
	SENSOR_I2C_WRITE(0x9a, 0x00);
	SENSOR_I2C_WRITE(0x57, 0x00);
	SENSOR_I2C_WRITE(0x58, 0xB4);
	SENSOR_I2C_WRITE(0x59, 0xA0);
	SENSOR_I2C_WRITE(0x4c, 0x13);
	SENSOR_I2C_WRITE(0x4b, 0x36);
	SENSOR_I2C_WRITE(0x3d, 0x3c);
	SENSOR_I2C_WRITE(0x3e, 0x03);
	SENSOR_I2C_WRITE(0xbd, 0xA0);
	SENSOR_I2C_WRITE(0xbe, 0xb4);
	SENSOR_I2C_WRITE(0x37, 0x02);
	SENSOR_I2C_WRITE(0x60, 0x9d);

	//YAVG
	SENSOR_I2C_WRITE(0x4e, 0x55);
	SENSOR_I2C_WRITE(0x4f, 0x55);
	SENSOR_I2C_WRITE(0x50, 0x55);
	SENSOR_I2C_WRITE(0x51, 0x55);
	SENSOR_I2C_WRITE(0x24, 0x55);
	SENSOR_I2C_WRITE(0x25, 0x40);
	SENSOR_I2C_WRITE(0x26, 0xa1);

	//Clock
	SENSOR_I2C_WRITE(0x5c, 0x52);
	SENSOR_I2C_WRITE(0x5d, 0x00);
	SENSOR_I2C_WRITE(0x11, 0x01);
	SENSOR_I2C_WRITE(0x2a, 0x98);
	SENSOR_I2C_WRITE(0x2b, 0x06);
	SENSOR_I2C_WRITE(0x2d, 0x00);
	SENSOR_I2C_WRITE(0x2e, 0x00);

	//General
	SENSOR_I2C_WRITE(0x13, 0xA5);
	SENSOR_I2C_WRITE(0x14, 0x40);

	//Banding
	SENSOR_I2C_WRITE(0x4a, 0x00);
	SENSOR_I2C_WRITE(0x49, 0xce);
	SENSOR_I2C_WRITE(0x22, 0x03);
	SENSOR_I2C_WRITE(0x09, 0x00);

	//close AE_AWB
	SENSOR_I2C_WRITE(0x13, 0x80);
	SENSOR_I2C_WRITE(0x16, 0x00);
	SENSOR_I2C_WRITE(0x10, 0xf0);
	SENSOR_I2C_WRITE(0x00, 0x3f);
	SENSOR_I2C_WRITE(0x38, 0x00);
	SENSOR_I2C_WRITE(0x01, 0x40);
	SENSOR_I2C_WRITE(0x02, 0x40);
	SENSOR_I2C_WRITE(0x05, 0x40);
	SENSOR_I2C_WRITE(0x06, 0x00);
	SENSOR_I2C_WRITE(0x07, 0x00);

    //BLC
    SENSOR_I2C_WRITE(0x41, 0x84);
	return ;
}



/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 ov9712plus_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ov9712plus_reg_init;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = ov9712plus_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = ov9712plus_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ov9712plus_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ov9712plus_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ov9712plus_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = ov9712plus_get_sensor_max_resolution;

    return 0;
}


HI_S32 ov9712plus_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ov9712plus_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ov9712plus_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ov9712plus_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ov9712plus_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ov9712plus_gains_update;

    return 0;
}



HI_S32 ov9712plus_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ov9712plus_get_awb_default;

    return 0;
}



int ov9712plus_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ov9712plus_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(OV9712PLUS_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    ov9712plus_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, OV9712PLUS_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    ov9712plus_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, OV9712PLUS_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


int ov9712plus_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(OV9712PLUS_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, OV9712PLUS_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, OV9712PLUS_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

void OV9712PLUS_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write)
{
	
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	ov9712plus_reg_init();

	ov9712plus_sensor_register_callback();

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

#endif // __OV9712PLUS_CMOS_H_

