#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_GC1004_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_GC1004_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__GC1004_CMOS_H_)
#define __GC1004_CMOS_H_

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

#define GC1004_ID 1004

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr = 0x78;
static unsigned int sensor_addr_byte = 1;
static unsigned int sensor_data_byte = 1;

static HI_U8 gu8SensorMode = 0;

static HI_U32 gu8Fps = 30;
static HI_U32 gu32FullLinesStd = 770;
static HI_U32 gu32FullLines = 770;



static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};


static AWB_CCM_S g_stAwbCcm =
{

/*	    4800,
        {  
            0x1be,0x80d2,0x14,
            0x807f,0x204,0x8085,
            0x8040,0x8119,0x259

        },
        3850,
        {   
            0x1bf,0x808a,0x8035,
            0x8076,0x1eb,0x8075,
            0x8052,0x8109,0x25b
        },
        2930,
        {   
            0x1a2,0x36,0x80d9,
            0x80b2,0x282,0x80d0,
            0x80e0,0x8169,0x369
        }


	4800,//10.24 室外版本
	 {	
		 0x24a,0x80B4,0x8096,
		 0x80b7,0x2d5,0x811e,
		 0x8040,0x817b,0x2bb

	 },
	 3850,
	 {	 
		 0x1e4,0x800d,0x80d7,
		 0x80a6,0x28a,0x80e4,
		 0x8070,0x81bb,0x32b
	 },
	 2300,
	 {	 
		 0xA2,0x37,0x80d9,
		 0x80d2,0x2c2,0x80f0,
		 0x80e0,0x8179,0x359
	 }	
	 */

	 4800,//10.27 室外版本
	 {	
		 0x23f,0x813c,0x8003,
		 0x8088,0x208,0x8080,
		 0x800a,0x81a7,0x2b1

	 },
	 3850,
	 {	 
		 0x25e,0x80cd,0x8091,
		 0x80ba,0x23b,0x8081,
		 0x8070,0x81bb,0x32b
	 },
	 2300,
	 {	 
		 0x1da,0x8087,0x8053,
		 0x80ca,0x27a,0x80b0,
		 0x806b,0x81de,0x349
	 }
		 

};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
	/* bvalid */
	1,

	/* saturation */
//	{0x80,0x80,0x80,0x80,0x68,0x48,0x35,0x30}
	
	{0x70,0x70,0x70,0x50,0x30,0x28,0x20,0x20}  //yang
 //   {0x90,0x88,0x80,0x80,0x70,0x60,0x50,0x40}
   
};


static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
	/* bvalid */
	1,
	

	//sharpen_alt_d
	//{0x50,0x4b,0x46,0x41,0x37,0x2c,0x1e,0xf},
//	{0x50,0x4a,0x44,0x3d,0x34,0x30,0x28,0x20},
	{0x99,0x90,0x80,0x70,0x60,0x58,0x48,0x30},
	//sharpen_alt_ud
//	{0x4b,0x46,0x41,0x3c,0x32,0x28,0x19,0xa},
//	{0x45,0x40,0x3a,0x35,0x30,0x30,0x30,0x30},
	{0x80,0x70,0x60,0x50,0x48,0x48,0x48,0x48},
	//snr_thresh
//	{0x10,0x14,0x1a,0x22,0x2a,0x37,0x46,0x50},
//	{0x10,0x14,0x19,0x22,0x2b,0x35,0x40,0x50},
	{0x10,0x14,0x19,0x22,0x2d,0x37,0x41,0x46 },
	
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
	{0, 27, 31, 33, 35, 36, 37, 38, 39, 40, 40, 41, 41, 42, 42, 43,
	43, 43, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47,
	47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49,
	49, 49, 50, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51,
	51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52, 52, 52, 52, 52, 52,
	52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
	53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
	54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55},

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
	0xcd,

	/*aa_slope*/
	0xbf,

	/*va_slope*/
	0xc1,

	/*uu_slope*/
	0xa0,

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
	/* bvalid */
	1,
	
	/*shading_center_r*/
	0x27a, 0x168,

	/*shading_center_g*/
	0x276, 0x16f,

	/*shading_center_b*/
	0x27a, 0x16c,

	/*shading_table_r*/
	{0x1000,0x1018,0x1028,0x103a,0x104c,0x105c,0x1072,0x1089,0x109e,0x10ba,0x10d5,0x10ef,
	0x110b,0x112b,0x114c,0x116d,0x118b,0x11ae,0x11d0,0x11f5,0x1218,0x123e,0x1260,0x1283,
	0x12aa,0x12cf,0x12f7,0x131b,0x1341,0x1369,0x138f,0x13b5,0x13db,0x1401,0x1423,0x1446,
	0x146d,0x148f,0x14b4,0x14d7,0x14fe,0x151e,0x153e,0x155a,0x1579,0x159a,0x15b7,0x15d3,
	0x15f4,0x1612,0x162e,0x164d,0x1663,0x167f,0x169a,0x16b1,0x16cb,0x16e4,0x16fc,0x170f,
	0x1727,0x173e,0x1753,0x176a,0x1783,0x1793,0x17a5,0x17b8,0x17c9,0x17da,0x17ec,0x17fe,
	0x180d,0x181a,0x182a,0x183b,0x184c,0x185a,0x1865,0x1876,0x1883,0x1890,0x189f,0x18a9,
	0x18b5,0x18c3,0x18cf,0x18d8,0x18e2,0x18e8,0x18ec,0x18f5,0x1901,0x190e,0x191f,0x1934,
	0x1946,0x1955,0x1968,0x197e,0x1993,0x19a4,0x19b5,0x19cc,0x19e1,0x19f5,0x1a06,0x1a16,
	0x1a2a,0x1a3d,0x1a4f,0x1a5d,0x1a6e,0x1a84,0x1a96,0x1aa7,0x1abb,0x1ad2,0x1ae5,0x1af9,
	0x1b0e,0x1b27,0x1b40,0x1b59,0x1b68,0x1b72,0x1b91,0x1bbf,0x1bf6},

	/*shading_table_g*/
	{0x1000,0x1013,0x1022,0x1033,0x1043,0x1054,0x1066,0x107b,0x108e,0x10a5,0x10bc,0x10d7,
	0x10f1,0x110c,0x112a,0x114b,0x116b,0x118c,0x11ae,0x11d0,0x11f3,0x1216,0x1238,0x125c,
	0x1282,0x12a9,0x12ce,0x12f3,0x131b,0x133f,0x1363,0x1386,0x13a9,0x13cc,0x13f0,0x1412,
	0x1434,0x1457,0x1479,0x149c,0x14bc,0x14da,0x14fa,0x1519,0x1537,0x1557,0x1575,0x1590,
	0x15ac,0x15c8,0x15e1,0x15fd,0x1617,0x162f,0x1648,0x165f,0x1673,0x168b,0x16a1,0x16b5,
	0x16c9,0x16db,0x16ee,0x1702,0x1714,0x1726,0x1736,0x1744,0x1752,0x1760,0x176c,0x1778,
	0x1785,0x1794,0x179e,0x17a8,0x17b5,0x17c1,0x17ca,0x17d5,0x17e1,0x17e9,0x17ef,0x17f3,
	0x17f9,0x17ff,0x1800,0x1801,0x1803,0x1806,0x180d,0x181c,0x182f,0x183e,0x184a,0x1854,
	0x185e,0x186a,0x1878,0x188b,0x189b,0x18a8,0x18b5,0x18c2,0x18d0,0x18e0,0x18ef,0x18fc,
	0x1907,0x1910,0x191a,0x1927,0x1936,0x1941,0x194a,0x1958,0x1965,0x1971,0x197e,0x1989,
	0x199c,0x19aa,0x19b7,0x19bd,0x19cc,0x19cc,0x19ce,0x19f6,0x1a0a},

	/*shading_table_b*/
	{0x1000,0x1012,0x101c,0x1025,0x102c,0x1031,0x103b,0x1045,0x104f,0x1059,0x1060,0x1072,
	0x107f,0x108d,0x109f,0x10b5,0x10c9,0x10dc,0x10ee,0x1109,0x111c,0x1137,0x1150,0x1167,
	0x1183,0x119e,0x11bb,0x11d5,0x11f4,0x120c,0x1228,0x1242,0x125a,0x1276,0x1292,0x12ac,
	0x12c7,0x12e0,0x12f9,0x1313,0x132d,0x1346,0x135d,0x1372,0x138a,0x13a4,0x13bb,0x13cf,
	0x13e5,0x13fb,0x1410,0x1427,0x143c,0x1450,0x1462,0x1475,0x1488,0x1496,0x14a5,0x14b7,
	0x14ca,0x14d7,0x14e5,0x14f4,0x1503,0x1511,0x151d,0x1529,0x1532,0x153c,0x1548,0x1552,
	0x155b,0x1566,0x156c,0x1573,0x157d,0x1587,0x158d,0x1593,0x159b,0x15a3,0x15a9,0x15ac,
	0x15b1,0x15b4,0x15b5,0x15b5,0x15b6,0x15b5,0x15b5,0x15ba,0x15c4,0x15d3,0x15e2,0x15ef,
	0x15f9,0x1604,0x1611,0x161e,0x1626,0x1632,0x1641,0x164c,0x165b,0x1667,0x1672,0x1679,
	0x1681,0x168e,0x1699,0x16a0,0x16aa,0x16bc,0x16ce,0x16dc,0x16e9,0x16f1,0x16f9,0x170c,
	0x1722,0x1735,0x173d,0x1739,0x1739,0x173c,0x173c,0x1731,0x1724},

	/*shading_off_center_r_g_b*/
	0xf57, 0xf0e, 0xf42,

	/*shading_table_nobe_number*/
	129
};

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
	/* bvalid */
	1,
	
/*	{0	,27  ,60  ,100 ,140 ,178 ,216 ,242 ,276 ,312 ,346 ,380 ,412 ,444 ,476 ,508,
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
*/
	{
			0x0,0x17,0x3b,0x69,0x9e,0xd8,0x114,0x14f,0x187,0x1bc,0x1ed,0x21c,0x247,0x270,0x296,0x2ba,
			0x2dc,0x2fc,0x31b,0x338,0x353,0x36d,0x387,0x3a0,0x3b8,0x3cf,0x3e7,0x3fe,0x415,0x42c,0x442,0x459,
			0x46f,0x485,0x49b,0x4b0,0x4c6,0x4db,0x4f0,0x505,0x519,0x52e,0x542,0x556,0x56a,0x57e,0x592,0x5a5,
			0x5b8,0x5cb,0x5de,0x5f1,0x603,0x616,0x628,0x63a,0x64c,0x65e,0x66f,0x681,0x692,0x6a3,0x6b4,0x6c5,
			0x6d6,0x6e6,0x6f6,0x707,0x717,0x727,0x737,0x746,0x756,0x765,0x775,0x784,0x793,0x7a2,0x7b0,0x7bf,
			0x7ce,0x7dc,0x7ea,0x7f9,0x807,0x815,0x822,0x830,0x83e,0x84b,0x859,0x866,0x873,0x881,0x88e,0x89a,
			0x8a7,0x8b4,0x8c1,0x8cd,0x8da,0x8e6,0x8f2,0x8ff,0x90b,0x917,0x923,0x92f,0x93b,0x946,0x952,0x95e,
			0x969,0x975,0x980,0x98c,0x997,0x9a2,0x9ad,0x9b8,0x9c4,0x9cf,0x9da,0x9e4,0x9ef,0x9fa,0xa05,0xa10,
			0xa1a,0xa25,0xa30,0xa3a,0xa45,0xa4f,0xa5a,0xa64,0xa6f,0xa79,0xa83,0xa8e,0xa98,0xaa2,0xaad,0xab7,
			0xac1,0xacc,0xad6,0xae0,0xaea,0xaf4,0xaff,0xb09,0xb13,0xb1d,0xb27,0xb32,0xb3c,0xb46,0xb50,0xb5a,
			0xb65,0xb6f,0xb79,0xb84,0xb8e,0xb98,0xba3,0xbad,0xbb7,0xbc2,0xbcc,0xbd7,0xbe1,0xbec,0xbf6,0xc01,
			0xc0c,0xc16,0xc21,0xc2c,0xc37,0xc41,0xc4c,0xc57,0xc62,0xc6d,0xc79,0xc84,0xc8f,0xc9a,0xca6,0xcb1,
			0xcbc,0xcc8,0xcd4,0xcdf,0xceb,0xcf7,0xd03,0xd0f,0xd1b,0xd27,0xd33,0xd3f,0xd4c,0xd58,0xd65,0xd72,
			0xd7e,0xd8b,0xd98,0xda5,0xdb2,0xdbf,0xdcd,0xdda,0xde8,0xdf5,0xe03,0xe11,0xe1f,0xe2d,0xe3b,0xe49,
			0xe58,0xe66,0xe75,0xe84,0xe93,0xea2,0xeb1,0xec0,0xecf,0xedf,0xeef,0xefe,0xf0e,0xf1e,0xf2e,0xf3e,
			0xf4d,0xf5d,0xf6c,0xf7a,0xf89,0xf96,0xfa4,0xfb0,0xfbd,0xfc8,0xfd3,0xfdc,0xfe5,0xfed,0xff4,0xffa,0xfff
		}

};




static HI_U32 Again_table[9]=
{
	//1024,1434,1843,2662,3482,4813,7004,9626,13517
	 1024,1440,1840,2608,3488,4912,7008,9632,13520
};


static HI_VOID gc1004_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
	int i;

	if(HI_NULL == pstAeSnsGainInfo)
	{
		printf("null pointer when get ae sensor gain info  value!\n");
		return;
	}

	pstAeSnsGainInfo->u32GainDb = 0;
	pstAeSnsGainInfo->u32SnsTimes = 1024;
   
	if (u32InTimes >= Again_table[8])
	{
		 pstAeSnsGainInfo->u32SnsTimes = Again_table[8];
		 pstAeSnsGainInfo->u32GainDb = 8;
		 return ;
	}
	
	for(i = 1; i < 9; i++)
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


HI_U32 gc1004_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
	if (HI_NULL == pstDef)
	{
		printf("null pointer when get isp default value!\n");
		return -1;
	}

	memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
	
	pstDef->stComm.u8Rggb			= 0x0;		// 0:  RGrGbB
	pstDef->stComm.u8BalanceFe		= 0x1;

	pstDef->stDenoise.u8SinterThresh= 0x15;
	pstDef->stDenoise.u8NoiseProfile= 0x0;		//0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
	pstDef->stDenoise.u16Nr0		= 0x0;
	pstDef->stDenoise.u16Nr1		= 0x0;

	pstDef->stDrc.u8DrcBlack		= 0x00;
	pstDef->stDrc.u8DrcVs			= 0x04; 	// variance space
	pstDef->stDrc.u8DrcVi			= 0x08; 	// variance intensity
	pstDef->stDrc.u8DrcSm			= 0xa0; 	// slope max
	pstDef->stDrc.u16DrcWl			= 0x4ff;	// white level

	memcpy(&pstDef->stAgcTbl, &g_stIspAgcTable, sizeof(ISP_CMOS_AGC_TABLE_S));
	memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTable, sizeof(ISP_CMOS_NOISE_TABLE_S));
	memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
	memcpy(&pstDef->stShading, &g_stIspShading, sizeof(ISP_CMOS_SHADING_S));
	memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));

	return 0;
}

HI_U32 gc1004_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
	if (HI_NULL == pstBlackLevel)
	{
		printf("null pointer when get isp black level value!\n");
		return -1;
	}

	/* Don't need to update black level when iso change */
	pstBlackLevel->bUpdate = HI_FALSE;

	pstBlackLevel->au16BlackLevel[0] = 12;
	pstBlackLevel->au16BlackLevel[1] = 8;
	pstBlackLevel->au16BlackLevel[2] = 8;
	pstBlackLevel->au16BlackLevel[3] = 12;

	return 0;	 
}

HI_VOID gc1004_set_pixel_detect(HI_BOOL bEnable)
{

	if (bEnable) /* setup for ISP pixel calibration mode */
	{
		/* 5 fps */ 		// 5240line per frame; VB = 5240 - 720 = 4520
		SENSOR_I2C_WRITE(0x07, 0x11);		// vb[12:8]
		SENSOR_I2C_WRITE(0x08, 0xa8);		// vb[7:0]
		
		/* min gain */
		SENSOR_I2C_WRITE(0xb1, 0x01);		//pre-gain[9:6]
		SENSOR_I2C_WRITE(0xb2, 0x00);		//pre-gain[5:0]
		
		SENSOR_I2C_WRITE(0xb6, 0x00);			//analog gain

	   // Global digital gain: default is 1.25x
		
		/* max exposure time*/		//total 5240 lines;   max exposure line = 5240 -4
		SENSOR_I2C_WRITE(0x03, 0x14);			//exp_time[12:8]
		SENSOR_I2C_WRITE(0x04, 0x74);			//exp_time[7:0]
	}
	else /* setup for ISP 'normal mode' */		//30fps
	{
		SENSOR_I2C_WRITE(0x07, 0x00);
		SENSOR_I2C_WRITE(0x08, 0x0c);
	}

	return;
}

HI_VOID gc1004_set_wdr_mode(HI_U8 u8Mode)
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

static HI_S32 gc1004_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	if (HI_NULL == pstAeSnsDft)
	{
		printf("null pointer when get ae default value!\n");
		return -1;
	}
	
	pstAeSnsDft->u32LinesPer500ms = 770*30/2;
	pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;

	gu32FullLinesStd = 770;

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
	pstAeSnsDft->u32MaxIntTime = 754;
	pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;


	pstAeSnsDft->u32MinIntTime = 2;
	
	pstAeSnsDft->au8HistThresh[0] = 0xd;
	pstAeSnsDft->au8HistThresh[1] = 0x28;
	pstAeSnsDft->au8HistThresh[2] = 0x60;
	pstAeSnsDft->au8HistThresh[3] = 0x80;
	
	pstAeSnsDft->u8AeCompensation = 0x40;
	
	pstAeSnsDft->u32MaxIntTimeTarget = 8191;		//max exposure time in sensor
	pstAeSnsDft->u32MinIntTimeTarget = 2;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;		//gain x
	pstAeSnsDft->stAgainAccu.f32Accuracy = 0.01;
	pstAeSnsDft->u32MaxAgain = 13520;  /* 1, 2, 4, ... 16 (0~24db, unit is 6db) */
	pstAeSnsDft->u32MinAgain = 1024; 
	pstAeSnsDft->u32MaxAgainTarget = 13520;
	pstAeSnsDft->u32MinAgainTarget = 1024;
	

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 0.015625;
	pstAeSnsDft->u32MaxDgain = 1023;  // 1.5x
	pstAeSnsDft->u32MinDgain = 64;
	pstAeSnsDft->u32MaxDgainTarget = 1023;	  
	pstAeSnsDft->u32MinDgainTarget = 64; 

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

	return 0;
}

static HI_S32 gc1004_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID gc1004_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	switch(u8Fps)
	{
	case 30:
		pstAeSnsDft->u32MaxIntTime = 754;		 
		gu32FullLinesStd = 770;
		gu8Fps = u8Fps;
		 pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * u8Fps / 2 - 240;//while antiflicker0/1 enable,shutter can update to max for 30fps        
		 SENSOR_I2C_WRITE(0x07, 0x00);
		SENSOR_I2C_WRITE(0x08, 0x0c);
		break;
	case 25:
		pstAeSnsDft->u32MaxIntTime = 908;		
		gu32FullLinesStd = 924;
		gu8Fps = u8Fps;
		pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * u8Fps / 2 - 32;//while antiflicker0/1 enable,shutter can update to max for 25fps        
		SENSOR_I2C_WRITE(0x07, 0x00);
		SENSOR_I2C_WRITE(0x08, 0xa6);
		break;
	default:
		break;
	}
	pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
	return;
}

static HI_VOID gc1004_slow_framerate_set(HI_U16 u16FullLines,
	AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	HI_U32 u32VblankingLines;

	u32VblankingLines = u16FullLines - 758;

	SENSOR_I2C_WRITE(0x08, u32VblankingLines & 0xff);		// VB[7:0]
	SENSOR_I2C_WRITE(0x07, (u32VblankingLines & 0x1f00) >> 8);

	pstAeSnsDft->u32MaxIntTime =  u16FullLines - 16;	
	return;
}

static HI_VOID gc1004_init_regs_info(HI_VOID)
{

	HI_S32 i;
	static HI_BOOL bInit = HI_FALSE;

	if (HI_FALSE == bInit)
	{
		g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
		g_stSnsRegsInfo.u32RegNum = 5;
		for (i=0; i<5; i++)
		{
			g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
			g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
			g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
		}

		g_stSnsRegsInfo.astI2cData[0].bDelayCfg = HI_FALSE;
		g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x03;		//exp_time[12:8]
		g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
		g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x04;		//exp_time[7:0] 
		g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;
		g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0xb6;		//Again
		g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_FALSE;
		g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0xb1;		//Pre-gain1  [9:6]
		g_stSnsRegsInfo.astI2cData[4].bDelayCfg = HI_FALSE;
		g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0xb2;		//pre-gain2  [5:0]

		g_stSnsRegsInfo.bDelayCfgIspDgain = HI_TRUE;

		bInit = HI_TRUE;
	}

	return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID gc1004_inttime_update(HI_U32 u32IntTime)
{
	gc1004_init_regs_info();
	g_stSnsRegsInfo.astI2cData[1].u32Data = u32IntTime & 0xFF;
	g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 8) & 0x1F;

	return;
}

static HI_VOID gc1004_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
	HI_U8 u8High, u8Low;


	u8High = (u32Dgain >>6) & 0x0f;    
	u8Low = (u32Dgain & 0x3f)<<2;

	
    HI_U32 u32NCurrent;
    //HI_U32 u32SCurrent;
    static HI_U32 u32Ratio_last = 32;
    static HI_U32 u32Ratio, u32Ratio_this;
	

	gc1004_init_regs_info();
	g_stSnsRegsInfo.astI2cData[2].u32Data = u32Again;
	
	g_stSnsRegsInfo.astI2cData[3].u32Data = u8High;
	g_stSnsRegsInfo.astI2cData[4].u32Data = u8Low;
	
	HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);

	    #if 1
	 uint16_t ret_data11 = 0;
	 uint16_t ret_data22 = 0;
	 SENSOR_I2C_READ(0x56,&ret_data11);
	 SENSOR_I2C_READ(0xd6,&ret_data22);
	 	 
	 ret_data22 = ret_data22<<8;
		 
	 u32NCurrent = ret_data11 | ret_data22;

	     
   // u32Ratio = (u32NCurrent * 10 + 11*u32SCurrent) * 32 / (21*u32NCurrent);     //k=1.3
   u32Ratio = (u32NCurrent * 10 + 11*170) * 32 / (21*u32NCurrent);     //k=1.1

   
//  printf("****%d**u32Ratio = 0x%x\n",__LINE__, u32Ratio);
   
    if(u32Ratio > 32)
            u32Ratio = 32;
    u32Ratio_this = (u32Ratio_last + u32Ratio) / 2;    //a + b = 10
    
    u32Ratio_this = u32Ratio_this;
    if(u32Ratio_this != u32Ratio_last)
    {		
		SENSOR_I2C_WRITE(0x66,u32Ratio_this);
    } 
    u32Ratio_last = u32Ratio_this;
    
#endif

	return;
}

static HI_S32 gc1004_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
	if (HI_NULL == pstAwbSnsDft)
	{
		printf("null pointer when get awb default value!\n");
		return -1;
	}

	memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
	
	pstAwbSnsDft->u16WbRefTemp = 4800;

/*	pstAwbSnsDft->au16GainOffset[0] = 0x12f;
	pstAwbSnsDft->au16GainOffset[1] = 0x100;
	pstAwbSnsDft->au16GainOffset[2] = 0x100;
	pstAwbSnsDft->au16GainOffset[3] = 0x124;

	pstAwbSnsDft->as32WbPara[0] = 78;
	pstAwbSnsDft->as32WbPara[1] = 112;
	pstAwbSnsDft->as32WbPara[2] = -64;
	pstAwbSnsDft->as32WbPara[3] = 184939;
	pstAwbSnsDft->as32WbPara[4] = 128;
	pstAwbSnsDft->as32WbPara[5] = -131329;
*/
//yang test
	pstAwbSnsDft->au16GainOffset[0] = 0x149;
	pstAwbSnsDft->au16GainOffset[1] = 0x100;
	pstAwbSnsDft->au16GainOffset[2] = 0x100;
	pstAwbSnsDft->au16GainOffset[3] = 0x12c;
/*
	pstAwbSnsDft->as32WbPara[0] = 115;//10.24 version
	pstAwbSnsDft->as32WbPara[1] = 5;
	pstAwbSnsDft->as32WbPara[2] = -135;
	pstAwbSnsDft->as32WbPara[3] = 258428;
	pstAwbSnsDft->as32WbPara[4] = 128;
	pstAwbSnsDft->as32WbPara[5] = -203009;
*/	
    pstAwbSnsDft->as32WbPara[0] = 125;//10.27 version
	pstAwbSnsDft->as32WbPara[1] = -12;
	pstAwbSnsDft->as32WbPara[2] = -142;
	pstAwbSnsDft->as32WbPara[3] = 223737;
	pstAwbSnsDft->as32WbPara[4] = 128;
	pstAwbSnsDft->as32WbPara[5] = -168965;

	memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
	memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
	
	return 0;
}

static HI_VOID gc1004_global_init()
{
	gu8SensorMode = 0;
}



void gc1004_reg_init()
{
//SYS
	SENSOR_I2C_WRITE(0xfe,0x80);
	SENSOR_I2C_WRITE(0xfe,0x80);
	SENSOR_I2C_WRITE(0xfe,0x80);
	SENSOR_I2C_WRITE(0xf2,0x0f);
	SENSOR_I2C_WRITE(0xf6,0x00);
	SENSOR_I2C_WRITE(0xfc,0xc6);
	SENSOR_I2C_WRITE(0xf7,0xb9);
	SENSOR_I2C_WRITE(0xf8,0x03);
	SENSOR_I2C_WRITE(0xf9,0x2e);
	SENSOR_I2C_WRITE(0xfa,0x00);
	SENSOR_I2C_WRITE(0xfe,0x00);





	//ANALOG & CISCTL
	SENSOR_I2C_WRITE(0x03,0x02);
	SENSOR_I2C_WRITE(0x04,0xfa);
	SENSOR_I2C_WRITE(0x05,0x01);
//	SENSOR_I2C_WRITE(0x06,0x77);	
	SENSOR_I2C_WRITE(0x06,0x6b);
	SENSOR_I2C_WRITE(0x07,0x00);
	SENSOR_I2C_WRITE(0x08,0x0c);
	SENSOR_I2C_WRITE(0x0d,0x02);
	SENSOR_I2C_WRITE(0x0e,0xe6);
	SENSOR_I2C_WRITE(0x0f,0x05);
	SENSOR_I2C_WRITE(0x10,0x10);
	SENSOR_I2C_WRITE(0x11,0x00);
//	SENSOR_I2C_WRITE(0x12,0x0c);
	SENSOR_I2C_WRITE(0x12,0x18);
	SENSOR_I2C_WRITE(0x17,0x14);
	SENSOR_I2C_WRITE(0x18,0x0a);
	SENSOR_I2C_WRITE(0x19,0x06);
	SENSOR_I2C_WRITE(0x1a,0x09);
	SENSOR_I2C_WRITE(0x1b,0x4f);
	SENSOR_I2C_WRITE(0x1c,0x21);
	SENSOR_I2C_WRITE(0x1d,0xe0);
	SENSOR_I2C_WRITE(0x1e,0xfc);
	SENSOR_I2C_WRITE(0x1f,0x08);
	SENSOR_I2C_WRITE(0x20,0xa9);
//	SENSOR_I2C_WRITE(0x21,0x6f);
	SENSOR_I2C_WRITE(0x21,0x2f);
	SENSOR_I2C_WRITE(0x22,0xb0);
	SENSOR_I2C_WRITE(0x23,0x32);
	SENSOR_I2C_WRITE(0x24,0x2f);
	SENSOR_I2C_WRITE(0x2a,0x00);
	SENSOR_I2C_WRITE(0x2c,0xb0);//0xc0 20140801
	SENSOR_I2C_WRITE(0x2d,0x0f);
	SENSOR_I2C_WRITE(0x2e,0xf0);
	SENSOR_I2C_WRITE(0x2f,0x1f);
	SENSOR_I2C_WRITE(0x25,0xc0);
	SENSOR_I2C_WRITE(0x36,0x0b);
	SENSOR_I2C_WRITE(0x37,0x13);
	SENSOR_I2C_WRITE(0x38,0x1b);




	//ISP
	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0x8a,0x00);
	SENSOR_I2C_WRITE(0x8c,0x02);
	SENSOR_I2C_WRITE(0x8e,0x02);
	SENSOR_I2C_WRITE(0x90,0x01);
	SENSOR_I2C_WRITE(0x94,0x02);
	SENSOR_I2C_WRITE(0x95,0x02);
	SENSOR_I2C_WRITE(0x96,0xd0);
	SENSOR_I2C_WRITE(0x97,0x05);
	SENSOR_I2C_WRITE(0x98,0x00);



	//MIPI
	SENSOR_I2C_WRITE(0xfe,0x03);
	SENSOR_I2C_WRITE(0x01,0x00);
	SENSOR_I2C_WRITE(0x02,0x00);
	SENSOR_I2C_WRITE(0x03,0x00);
	SENSOR_I2C_WRITE(0x06,0x00);
	SENSOR_I2C_WRITE(0x10,0x00);
	SENSOR_I2C_WRITE(0x15,0x00);



	//BLK
	SENSOR_I2C_WRITE(0xfe,0x00);
	SENSOR_I2C_WRITE(0x18,0x02);
	SENSOR_I2C_WRITE(0x1a,0x01);
	SENSOR_I2C_WRITE(0x40,0x23);
	SENSOR_I2C_WRITE(0x5e,0x00);
	SENSOR_I2C_WRITE(0x66,0x20);





	//Dark ...
	SENSOR_I2C_WRITE(0xfe,0x02);
	SENSOR_I2C_WRITE(0x40,0x37);
	SENSOR_I2C_WRITE(0x49,0x23);
	SENSOR_I2C_WRITE(0xa4,0x00);
	SENSOR_I2C_WRITE(0xfe,0x00);




	//Gain
	SENSOR_I2C_WRITE(0xb0,0x50);
	SENSOR_I2C_WRITE(0xb3,0x40);
	SENSOR_I2C_WRITE(0xb4,0x40);
	SENSOR_I2C_WRITE(0xb5,0x40);
	SENSOR_I2C_WRITE(0xfe,0x00);

	printf("GalaxyCore GC1004 sensor 720p 30fps init success!\n");
	return ;




}






/****************************************************************************
 * callback structure														*
 ****************************************************************************/
HI_S32 gc1004_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = gc1004_reg_init;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = gc1004_global_init;
	pstSensorExpFunc->pfn_cmos_get_isp_default = gc1004_get_isp_default;
	pstSensorExpFunc->pfn_cmos_get_isp_black_level = gc1004_get_isp_black_level;
	pstSensorExpFunc->pfn_cmos_set_pixel_detect = gc1004_set_pixel_detect;
	pstSensorExpFunc->pfn_cmos_set_wdr_mode = gc1004_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = gc1004_get_sensor_max_resolution;

	return 0;
}

HI_S32 gc1004_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_ae_default	= gc1004_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set			= gc1004_fps_set;
	pstExpFuncs->pfn_cmos_slow_framerate_set= gc1004_slow_framerate_set;	  
	pstExpFuncs->pfn_cmos_inttime_update	= gc1004_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update		= gc1004_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table	= gc1004_again_calc_table;

	return 0;
}

HI_S32 gc1004_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_awb_default = gc1004_get_awb_default;

	return 0;
}

int gc1004_sensor_register_callback(void)
{
	HI_S32 s32Ret;
	ALG_LIB_S stLib;
	ISP_SENSOR_REGISTER_S stIspRegister;
	AE_SENSOR_REGISTER_S  stAeRegister;
	AWB_SENSOR_REGISTER_S stAwbRegister;

	gc1004_init_sensor_exp_function(&stIspRegister.stSnsExp);
	s32Ret = HI_MPI_ISP_SensorRegCallBack(GC1004_ID, &stIspRegister);
	if (s32Ret)
	{
		printf("sensor register callback function failed!\n");
		return s32Ret;
	}
	
	stLib.s32Id = 0;
	strcpy(stLib.acLibName, HI_AE_LIB_NAME);
	gc1004_init_ae_exp_function(&stAeRegister.stSnsExp);
	s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, GC1004_ID, &stAeRegister);
	if (s32Ret)
	{
		printf("sensor register callback function to ae lib failed!\n");
		return s32Ret;
	}

	stLib.s32Id = 0;
	strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
	gc1004_init_awb_exp_function(&stAwbRegister.stSnsExp);
	s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, GC1004_ID, &stAwbRegister);
	if (s32Ret)
	{
		printf("sensor register callback function to ae lib failed!\n");
		return s32Ret;
	}
	
	return 0;
}

int gc1004_sensor_unregister_callback(void)
{
	HI_S32 s32Ret;
	ALG_LIB_S stLib;

	s32Ret = HI_MPI_ISP_SensorUnRegCallBack(GC1004_ID);
	if (s32Ret)
	{
		printf("sensor unregister callback function failed!\n");
		return s32Ret;
	}
	
	stLib.s32Id = 0;
	strcpy(stLib.acLibName, HI_AE_LIB_NAME);
	s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, GC1004_ID);
	if (s32Ret)
	{
		printf("sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	stLib.s32Id = 0;
	strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
	s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, GC1004_ID);
	if (s32Ret)
	{
		printf("sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}
	
	return 0;
}


void GC1004_init(SENSOR_GC1004_DO_I2CRD do_i2c_read, SENSOR_GC1004_DO_I2CWR do_i2c_write)
{
	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	gc1004_reg_init();

	gc1004_sensor_register_callback();

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
	printf("GC1004 sensor 720P30fps init success!\n");

return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif // __GC1004_CMOS_H_

