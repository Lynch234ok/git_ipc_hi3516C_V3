#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"


static SENSOR_BG0701_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_BG0701_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__BG0701_CMOS_H_)
#define __BG0701_CMOS_H_

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

#define EXP_PRECISE  (16)
#define MCLK         (24000000)
#define ROWTIME      (0x408)
#define HSIZE        (1288)
#define VSIZE        (728)

#define FRAME_RATE   (20)

// Local define
#define BG_I2C_ADDR  (0x64)
#define BG070X_ID    (0x0703)

#define BG0701B_ID    (0x070107)
#define BG0701D_ID    (0x070109)
#define BG0701E_ID    (0x07010A)
#define BG0701F_ID    (0x07010B)
#define BG0703B_ID    (0x070703)
#define BG0703D_ID    (0x070707)

#define TROW         (ROWTIME*1000000/MCLK)
#define PORCH        (8)

#define VBLANK_Std   ((1000000/(FRAME_RATE*TROW)-(PORCH+VSIZE)))
#define VBLANK_5fps  ((1000000/(5*TROW)-(PORCH+VSIZE)))

#define MAXINT_Std   ((VSIZE+VBLANK_Std))
#define MAXINT_5fps  ((VSIZE+VBLANK_5fps))

//Minimum Gain for Full Swing in small FD (8bits fix point)
#define MIN_DGAIN (0x133)

//FD Gain BG070X (10bits fix point)
#define FD_Gain_0701 (4408)
#define FD_Gain_0703 (3480)

//FD Switch Threshold (When AGAIN > ?)
#define FD_TH  5460

//Limit AGain to 1-12X
#define VREFH_LIMIT_H 0x7F
#define VREFH_LIMIT_L 0x0C

// Typical BLCC Target
#define BLCC_TARGET_TYPICAL (24)

#define blcc_target_coef_0701b (48)
#define blcc_target_base_0701b (11)

#define blcc_target_coef_0701d (48)
#define blcc_target_base_0701d (11)

#define blcc_target_coef_0701e (58)
#define blcc_target_base_0701e (11)

#define blcc_target_coef_0701f (58)
#define blcc_target_base_0701f (11)

#define blcc_target_coef_0703b (91)
#define blcc_target_base_0703b (6)

#define blcc_target_coef_0703d (91)
#define blcc_target_base_0703d (6)

static const char INIT_PRINT[] = "Initialized ! Hello world !";

#define SEQ_COMBO(x, y) (((x&0xFFFF)<<16)|(y&0xFFFF))
static const int sensor_init_seq[] =
{
  SEQ_COMBO(0xf0, 0x00),  //select page0
  //SEQ_COMBO(0x1c, 0x01),  //soft reset
  SEQ_COMBO(0x89, 0x21),  //internal vddpix off
  SEQ_COMBO(0xb9, 0x21),  //Manual Gain

  SEQ_COMBO(0x06, 0xFF&(HSIZE>>8)), 
  SEQ_COMBO(0x07, 0xFF&(HSIZE)),
  SEQ_COMBO(0x08, 0xFF&(VSIZE>>8)), 
  SEQ_COMBO(0x09, 0xFF&(VSIZE)),
  
  SEQ_COMBO(0x0e, 0xFF&(ROWTIME>>8)), 
  SEQ_COMBO(0x0f, 0xFF&(ROWTIME)),  //row time = 0x408/Fmclk = 1032/24MHz = 43 us

  SEQ_COMBO(0x14, 0x03),  //TXB ON *
  SEQ_COMBO(0x1E, 0x0f),  //VTH 3.8V please check the voltage
  //SEQ_COMBO(0x20, 0x02),  //mirror
  SEQ_COMBO(0x20, 0x01),  //mirror

  SEQ_COMBO(0x21, 0xFF&(VBLANK_Std>>8)),  
  SEQ_COMBO(0x22, 0xFF&(VBLANK_Std)),

  SEQ_COMBO(0x28, 0x00),  //RAMP1 ONLY
  SEQ_COMBO(0x29, 0x18),  //RSTB =1us
  SEQ_COMBO(0x2a, 0x18),  //TXB = 1us
  //SEQ_COMBO(0x2d, 0x01),  
  //SEQ_COMBO(0x2e, 0xB0),  //ibias_cnten_gap=17u
  SEQ_COMBO(0x2d, 0x00),  
  SEQ_COMBO(0x2e, 0x01),  //ibias_cnten_gap
  SEQ_COMBO(0x30, 0x18),  //rstb_cmprst_gap=1u
  SEQ_COMBO(0x34, 0x20),  //tx_ramp2=32 CLKIN cycle*

  SEQ_COMBO(0x38, 0x03), 
  SEQ_COMBO(0x39, 0xfd), 
  SEQ_COMBO(0x3a, 0x03), 
  SEQ_COMBO(0x3b, 0xfa), 

  SEQ_COMBO(0x50, 0x00), 
  SEQ_COMBO(0x53, 0x76), 
 // SEQ_COMBO(0x53, 0x4E), //20151110
  SEQ_COMBO(0x54, 0x03), 
  SEQ_COMBO(0x52, 0xdd), 
  SEQ_COMBO(0x60, 0x00),  //row refresh mode
  SEQ_COMBO(0x6d, 0x01),  //pll=288M pclk=72M  (when clkin=24M)
  SEQ_COMBO(0x64, 0x02),  
  SEQ_COMBO(0x65, 0x00),  //RAMP1 length=200
  SEQ_COMBO(0x67, 0x05), 
  SEQ_COMBO(0x68, 0xff),  //RAMP1 length=5ff
  SEQ_COMBO(0x87, 0xaf),  // votlgate of vbg-i
 // SEQ_COMBO(0x1d, 0x01),  //restart

  SEQ_COMBO(0xf0, 0x01), 
  SEQ_COMBO(0xc8, 0x04), 
  SEQ_COMBO(0xc7, 0x55),  // FD Gain = 1X 
  SEQ_COMBO(0xe0, 0x01), 
  SEQ_COMBO(0xe1, 0x04), 
  SEQ_COMBO(0xe2, 0x03), 
  SEQ_COMBO(0xe3, 0x02), 
  SEQ_COMBO(0xe4, 0x01), 
  SEQ_COMBO(0xe5, 0x01),  //vcm_comp =2.56V
  SEQ_COMBO(0xb4, 0x01),  //row noise remove on*
  SEQ_COMBO(0x20, 0x00), //blcc off
  SEQ_COMBO(0x31, 0x00), //blcc target upper high
  SEQ_COMBO(0x32, 0x38), 
  SEQ_COMBO(0x33, 0x00), //blcc target upper low
  SEQ_COMBO(0x34, 0x35), 
  SEQ_COMBO(0x35, 0x00), //blcc target lower high
  SEQ_COMBO(0x36, 0x33), 
  SEQ_COMBO(0x37, 0x00), //blcc target lower low
  SEQ_COMBO(0x38, 0x30), 
  SEQ_COMBO(0x39, 0x04), //frame count to ave
  
  SEQ_COMBO(0x3E, 0x07),
  SEQ_COMBO(0x3F, 0xff), // Upper Limit
  
  SEQ_COMBO(0x40, 0xff),
  SEQ_COMBO(0x41, 0xC0), // Lower Limit
  
  SEQ_COMBO(0x20, 0x00),  //blcc on

  SEQ_COMBO(0x4e, 0x00),  
  SEQ_COMBO(0x4f, 0x00),  //digital offset

  SEQ_COMBO(0xf1, 0x07),  //dpc on

  SEQ_COMBO(0xf0, 0x00), 
  SEQ_COMBO(0x7f, 0x00),  //cmp current
  SEQ_COMBO(0x81, 0x09),  //dot_en=1,vrst=vth,vtx=vth
  SEQ_COMBO(0x82, 0x11),  //bandgap current & ramp current
  SEQ_COMBO(0x83, 0x01),  //pixel current
  SEQ_COMBO(0x84, 0x07),  //check rst voltage 
  SEQ_COMBO(0x88, 0x05),  //pclk phase
  //SEQ_COMBO(0x88, 0x25),  //pclk phase
  SEQ_COMBO(0x8a, 0x01),  //pclk drv
  SEQ_COMBO(0x8c, 0x01),  //data drv
  SEQ_COMBO(0xb0, 0x01), 
  SEQ_COMBO(0xb1, 0x7f), 
  SEQ_COMBO(0xb2, 0x01), 
  SEQ_COMBO(0xb3, 0x7f),  //analog gain=1X
  SEQ_COMBO(0xb4, 0x11), 
  SEQ_COMBO(0xb5, 0x11), 
  SEQ_COMBO(0xb6, 0x11), 
  SEQ_COMBO(0xb7, 0x01), 
  SEQ_COMBO(0xb8, 0x00),  //digital gain=1X
  SEQ_COMBO(0xbf, 0x0c), 
  SEQ_COMBO(0x8e, 0x00),  //OEN
  SEQ_COMBO(0x8d, 0x00),  //OEN
  SEQ_COMBO(0x1d, 0x01), 
  SEQ_COMBO(0xFFFF, 0xFFFF),
  SEQ_COMBO(0xFFFF, 0xFFFF)
};

static const unsigned int sensor_i2c_addr=0x64;
static const unsigned int sensor_addr_byte=1;
static const unsigned int sensor_data_byte=1;
static unsigned int sensor_id=0x070109;

/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance                       */
/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c                                         */
#define CMOS_BG070x_ISP_WRITE_SENSOR_ENABLE (0)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/
static HI_U32 gu32FullLinesStd = (PORCH + VSIZE + VBLANK_Std)*EXP_PRECISE;
static HI_U8  gu8SensorMode = 0;
static HI_U32 gu32FullLines = (PORCH + VSIZE + VBLANK_Std)*EXP_PRECISE;
static HI_U32 FD_Gain;



static AWB_CCM_S g_stAwbCcm =
{
    4850,
    {   0x0187, 0x8056, 0x8031,
        0x8042, 0x017f, 0x803d,
        0x000a, 0x8116, 0x020b
    },
    3160,
    {   0x0178, 0x803e, 0x803a,
        0x8088, 0x01b3, 0x802b,
        0x8029, 0x8174, 0x029d
    },
#if 0
    2470,
    {   0x00dd, 0x0063, 0x8041,
        0x8085, 0x018a, 0x8005,
        0x8081, 0x8169, 0x02ea
    }
#else
    2470,
    {   0x0115, 0x001c, 0x8032,
        0x807c, 0x0168, 0x0013,
        0x8080, 0x820f, 0x038f
    }
#endif
};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

    /* saturation */
    {0x80,0x80,0x80,0x80,0x68,0x48,0x35,0x30}
};


static ISP_CMOS_AGC_TABLE_S g_stIspAgcTable =
{
    /* bvalid */
    1,    

    /* sharpen_alt_d */
    {0x8e,0x8b,0x88,0x83,0x7d,0x76,0x75,0x74},
        
    /* sharpen_alt_ud */
    {0x8f,0x89,0x7e,0x78,0x6f,0x44,0x40,0x35},
        
    /* snr_thresh *//*init data*/
    {0x19,0x1e,0x2d,0x32,0x39,0x3f,0x48,0x4b},
    //{0x08,0x0e,0x11,0x17,0x1c,0x23,0x28,0x41},
        
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

#if 0
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

#endif

static ISP_CMOS_GAMMA_S g_stIspGamma =
{
    /* bvalid */
    1,
    
    {0,54,106,158,209,259, 308, 356, 403, 450, 495, 540, 584, 628, 670, 713, 754, 795,
        835, 874, 913, 951, 989,1026,1062,1098,1133,1168,1203,1236,1270,1303, 1335,1367,
        1398,1429,1460,1490,1520,1549,1578,1607,1635,1663,1690,1717,1744,1770,1796,1822,
        1848,1873,1897,1922,1946,1970,1993,2017,2040,2062,2085,2107,2129,2150, 2172,2193,
        2214,2235,2255,2275,2295,2315,2335,2354,2373,2392,2411,2429,2447,2465, 2483,2501,
        2519,2536,2553,2570,2587,2603,2620,2636,2652,2668,2684,2700,2715,2731, 2746,2761,
        2776,2790,2805,2819,2834,2848,2862,2876,2890,2903,2917,2930,2944,2957, 2970,2983,
        2996,3008,3021,3033,3046,3058,3070,3082,3094,3106,3118,3129,3141,3152, 3164,3175,
        3186,3197,3208,3219,3230,3240,3251,3262,3272,3282,3293,3303,3313,3323, 3333,3343,
        3352,3362,3372,3381,3391,3400,3410,3419,3428,3437,3446,3455,3464,3473, 3482,3490,
        3499,3508,3516,3525,3533,3541,3550,3558,3566,3574,3582,3590,3598,3606, 3614,3621,
        3629,3637,3644,3652,3660,3667,3674,3682,3689,3696,3703,3711,3718,3725, 3732,3739,
        3746,3752,3759,3766,3773,3779,3786,3793,3799,3806,3812,3819,3825,3831, 3838,3844,
        3850,3856,3863,3869,3875,3881,3887,3893,3899,3905,3910,3916,3922,3928, 3933,3939,
        3945,3950,3956,3962,3967,3973,3978,3983,3989,3994,3999,4005,4010,4015, 4020,4026,
        4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095, 4095}
};

HI_U32 bg0701_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
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
    //memcpy(&pstDef->stShading, &g_stIspShading, sizeof(ISP_CMOS_SHADING_S));
    memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));

    return 0;
}

HI_U32 bg0701_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    pstBlackLevel->au16BlackLevel[0] = 24;
    pstBlackLevel->au16BlackLevel[1] = 21;
    pstBlackLevel->au16BlackLevel[2] = 24;
    pstBlackLevel->au16BlackLevel[3] = 21;
    printf("cmos_get_isp_black_level : Black Level\n");
    return 0;    
}


HI_VOID bg0701_set_pixel_detect(HI_BOOL bEnable)
{
    HI_U8 page;
    uint16_t temp=0;
    SENSOR_I2C_READ(0xF0,&temp);
	page=0xFF&temp;
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(0xF0, 0x00);    // Page 0
        SENSOR_I2C_WRITE(0x21, 0xFF&(VBLANK_5fps>>8)); // 5fps
        SENSOR_I2C_WRITE(0x22, 0xFF&(VBLANK_5fps));    // Vblank
        SENSOR_I2C_WRITE(0x0C, 0xFF&(MAXINT_5fps>>8));
        SENSOR_I2C_WRITE(0x0D, 0xFF&(MAXINT_5fps));    //max exposure lines
        SENSOR_I2C_WRITE(0x26, 0x00);
        SENSOR_I2C_WRITE(0x27, 0x00);    
        SENSOR_I2C_WRITE(0xF0, 0x01);    // Page 1
        SENSOR_I2C_WRITE(0xC7, 0x55);    // FD = 1X
        SENSOR_I2C_WRITE(0xF0, 0x00);    // Page 0
        SENSOR_I2C_WRITE(0x1d, 0x02);    // Restart
        SENSOR_I2C_WRITE(0xB1, 0x6A);    // AGain = 1.2X
        SENSOR_I2C_WRITE(0xB7, 0x01);    // DGain = 1X
        SENSOR_I2C_WRITE(0xB8, 0x00);    //
        //printf("cmos_set_pixel_detect : 5fps\n");
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(0xF0, 0x00);    // Page 0
        SENSOR_I2C_WRITE(0x21, 0xFF&(VBLANK_Std>>8)); // fps
        SENSOR_I2C_WRITE(0x22, 0xFF&(VBLANK_Std));    // Vblank
        SENSOR_I2C_WRITE(0x0C, 0xFF&(MAXINT_Std>>8));
        SENSOR_I2C_WRITE(0x0D, 0xFF&(MAXINT_Std));    //max exposure lines
        SENSOR_I2C_WRITE(0x1d, 0x02);         
        //printf("cmos_set_pixel_detect : default fps\n");
    }
    SENSOR_I2C_WRITE(0xF0, page);

    return;
}

HI_VOID bg0701_set_wdr_mode(HI_U8 u8Mode)
{
    switch(u8Mode)
    {
        //sensor mode 0
        case 0:
            gu8SensorMode = 0;
        break;
        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    
    return;
}

HI_U16 SEN_REG_RD_WORD(HI_U8 x,HI_U8 y)
{
	HI_U16 high,low;
	SENSOR_I2C_READ(x,&high);
	SENSOR_I2C_READ(y,&low);
	return (((0xFF&high)<<8)|(0xFF&low));
}

static signed short bg0701_blcc_update(HI_U16 darkrow_avg, HI_U16 reg_dgain)
{
  static signed short offset = 0;
  signed short output;
  signed short target;
  signed short target_upper_hi;
  signed short target_upper_lo;
  signed short target_lower_hi;
  signed short target_lower_lo;
  signed short diff;
  HI_U16 blcc_coef;
  HI_U16 blcc_base;
  
  if(sensor_id==BG0701B_ID)
  {
    blcc_coef = blcc_target_coef_0701b;
    blcc_base = blcc_target_base_0701b;
  }
  else if(sensor_id==BG0701D_ID)
  {
    blcc_coef = blcc_target_coef_0701d;
    blcc_base = blcc_target_base_0701d;
  }
  else if(sensor_id==BG0701E_ID)
  {
    blcc_coef = blcc_target_coef_0701e;
    blcc_base = blcc_target_base_0701e;
  }
  else if((sensor_id&0xFFFF00)==(BG0701F_ID&0xFFFF00))
  {
    blcc_coef = blcc_target_coef_0701f;
    blcc_base = blcc_target_base_0701f;
  }
  else if(sensor_id==BG0703B_ID)
  {
    blcc_coef = blcc_target_coef_0703b;
    blcc_base = blcc_target_base_0703b;
  }
  else if((sensor_id&0xFFFF00)==(BG0703D_ID&0xFFFF00))
  {
    blcc_coef = blcc_target_coef_0703d;
    blcc_base = blcc_target_base_0703d;
  }
  else
  {
    printf("cmos_blcc_update : Invalid Sensor (ID = 0x%06X) !\n", sensor_id);
    return 0;
  }
  
  target = ((BLCC_TARGET_TYPICAL<<8)/reg_dgain) + (((darkrow_avg*blcc_coef)>>10) + blcc_base);
  output = darkrow_avg + offset;
  
  target_upper_hi = target+8;
  target_upper_lo = target+4;
  target_lower_hi = target-4;
  target_lower_lo = target-8;
  
  if(output>target_upper_hi)
  {
    diff = -1 - ((output - target_upper_hi)>>1);
  }
  else if(output>target_upper_lo)
  {
    diff = -1;
  }
  else if(output<target_lower_lo)
  {
    diff = 1 + ((target_lower_lo - output)>>1);
  }
  else if(output<target_lower_hi)
  {
    diff = 1;
  }
  else
  {
    diff = 0;
  }
  
  //if(diff> 128) diff =  128;
  //if(diff<-256) diff = -256;
  
  offset += diff;
  
  if(offset>128) offset = 128;
  if(offset<-2048) offset = -2048;
  //printf("dgain=0x%04X dark=%d target=%d output=%d diff=%d offset=%d \n", reg_dgain, darkrow_avg, target, output, diff, offset);
  
  return offset;
}

static HI_U8 FD_State = 0;
static HI_VOID bg0701_again_calc_table(HI_U32 u32InTimes, AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
  //u32InTimes = realGain*1024
  HI_U8 vrefh;
  HI_U8 fd;
  
  if(HI_NULL == pstAeSnsGainInfo)
  {
    printf("null pointer when get ae sensor gain info  value!\n");
    return;
  }

  if (u32InTimes > FD_TH)
  {
    fd = 1;
  }
  else if(u32InTimes < FD_Gain)
  {
    fd = 0;
  }
  else
  {
    fd = FD_State;
  }
  
  if(fd)//again = 128/vrefh
    vrefh = 128*FD_Gain/u32InTimes - 1;
  else
    vrefh = (128<<10)/u32InTimes - 1;
  
  if(vrefh>VREFH_LIMIT_H) vrefh = VREFH_LIMIT_H;
  if(vrefh<VREFH_LIMIT_L) vrefh = VREFH_LIMIT_L;
  
  if(fd)
  {
    pstAeSnsGainInfo->u32SnsTimes = 128*FD_Gain/vrefh;
    pstAeSnsGainInfo->u32GainDb = 0x80|vrefh;
  }
  else
  {
    pstAeSnsGainInfo->u32SnsTimes = (128<<10)/vrefh;
    pstAeSnsGainInfo->u32GainDb = vrefh;
  }
  
  return;
}

static HI_VOID bg0701_dgain_calc_table(HI_U32 u32InTimes, AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
  //u32InTimes = realGain*1024
  
  if(HI_NULL == pstAeSnsGainInfo)
  {
    printf("null pointer when get ae sensor gain info  value!\n");
    return;
  }
  
  if(FD_State)
  {
    pstAeSnsGainInfo->u32SnsTimes = (0xFFFF&(u32InTimes>>2))<<2;
    pstAeSnsGainInfo->u32GainDb = u32InTimes>>2;
  }
  else
  {
    pstAeSnsGainInfo->u32SnsTimes = MIN_DGAIN<<2;
    pstAeSnsGainInfo->u32GainDb = MIN_DGAIN;
  }
  
  return;
}

static HI_S32 bg0701_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
  if (HI_NULL == pstAeSnsDft)
  {
    printf("null pointer when get ae default value!\n");
    return -1;
  }
  SENSOR_I2C_WRITE(0xF0, 0x00);  //select page0
 // sensor_id = (SENSOR_I2C_READ(0x00)<<16)|(SENSOR_I2C_READ(0x01)<<8)|(SENSOR_I2C_READ(0x45)<<0);
  usleep(200000);
  if(sensor_id==BG0701B_ID)
  {
    FD_Gain = FD_Gain_0701;
  }
  else if(sensor_id==BG0701D_ID)
  {
    FD_Gain = FD_Gain_0701;
  }
  else if(sensor_id==BG0701E_ID)
  {
    FD_Gain = FD_Gain_0701;
  }
  else if((sensor_id&0xFFFF00)==(BG0701F_ID&0xFFFF00))
  {
    FD_Gain = FD_Gain_0701;
  }
  else if(sensor_id==BG0703B_ID)
  {
    FD_Gain = FD_Gain_0703;
  }
  else if((sensor_id&0xFFFF00)==(BG0703D_ID&0xFFFF00))
  {
    FD_Gain = FD_Gain_0703;
  }
  else
  {
    printf("cmos_get_ae_default : Invalid Sensor (ID = 0x%06X) !\n", sensor_id);
    return -1;
  }

  gu32FullLinesStd = (PORCH + VSIZE + VBLANK_Std)*EXP_PRECISE;
  
  pstAeSnsDft->au8HistThresh[0] = 0x0d;
  pstAeSnsDft->au8HistThresh[1] = 0x28;
  pstAeSnsDft->au8HistThresh[2] = 0x60;
  pstAeSnsDft->au8HistThresh[3] = 0x80;
  
  pstAeSnsDft->u8AeCompensation = 0x40;
  
  pstAeSnsDft->u32LinesPer500ms = (500000/TROW)*EXP_PRECISE; // 500000us/43us;
  pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
  pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;

  pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
  pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
  pstAeSnsDft->u32MaxIntTime = MAXINT_Std*EXP_PRECISE;
  pstAeSnsDft->u32MinIntTime = EXP_PRECISE;
  pstAeSnsDft->u32MaxIntTimeTarget = 65535;
  pstAeSnsDft->u32MinIntTimeTarget = EXP_PRECISE;

  pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
  pstAeSnsDft->stAgainAccu.f32Accuracy = 256;
  pstAeSnsDft->u32MaxAgain = FD_Gain*128/VREFH_LIMIT_L;
  pstAeSnsDft->u32MinAgain = 1024*128/VREFH_LIMIT_H;
  pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
  pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

  pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
  pstAeSnsDft->stDgainAccu.f32Accuracy = 2<<8;
  pstAeSnsDft->u32MaxDgain = 3<<9;
  pstAeSnsDft->u32MinDgain = 1<<10;
  pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
  pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain; 

  pstAeSnsDft->u32ISPDgainShift = 8;
  pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
  pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

  return 0;
}

static HI_S32 bg0701_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID bg0701_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
  HI_U32 u32VblankingLines;
  HI_U8  page ;
  uint16_t temp=0;
  SENSOR_I2C_READ(0xF0,&temp);
  page=0xFF&temp;
  u32VblankingLines = 1000000/(u8Fps*TROW)-(PORCH+VSIZE);
  if(u32VblankingLines>0xFFFF)u32VblankingLines=0xFFFF;
  if(u32VblankingLines<0x0024)u32VblankingLines=0x0024;
  
  SENSOR_I2C_WRITE(0xF0, 0x00);    
  SENSOR_I2C_WRITE(0x21, 0xFF&(u32VblankingLines>>8));    
  SENSOR_I2C_WRITE(0x22, 0xFF&(u32VblankingLines));
  SENSOR_I2C_WRITE(0x1D, 0x02); 
  SENSOR_I2C_WRITE(0xF0, page); 

  gu32FullLinesStd = (PORCH + VSIZE + u32VblankingLines)*EXP_PRECISE;
  pstAeSnsDft->u32MaxIntTime = (VSIZE + u32VblankingLines)*EXP_PRECISE;
  pstAeSnsDft->u32LinesPer500ms = (500000/TROW)*EXP_PRECISE; // 500000us/43us;
  pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
  //printf("fps = %d\n", u8Fps);
  return;
}

static HI_VOID bg0701_slow_framerate_set(HI_U16 u16FullLines, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 u32VblankingLines;
    HI_U8  page;
	uint16_t temp=0;
    SENSOR_I2C_READ(0xF0,&temp);
	page=0xFF&temp;
    u32VblankingLines = u16FullLines/EXP_PRECISE - (PORCH+VSIZE);      // - vsize -8
    if(u32VblankingLines>0xFFFF)u32VblankingLines=0xFFFF;
    if(u32VblankingLines<0x0024)u32VblankingLines=0x0024;

    SENSOR_I2C_WRITE(0xF0, 0x00);    
    SENSOR_I2C_WRITE(0x21,  (u32VblankingLines >> 8) & 0xff);    // high byte address
    SENSOR_I2C_WRITE(0x22, u32VblankingLines & 0xff);    //low byte address
    SENSOR_I2C_WRITE(0x1D, 0x02); 
    SENSOR_I2C_WRITE(0xF0, page); 
    
    gu32FullLines = (PORCH + VSIZE + u32VblankingLines)*EXP_PRECISE;
    pstAeSnsDft->u32MaxIntTime = (VSIZE + u32VblankingLines)*EXP_PRECISE;
    //printf("FullLines = %d\n", gu32FullLines);
    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID bg0701_inttime_update(HI_U32 u32IntTime)
{
  static HI_U32 u32IntTime_old = 0;
  static HI_U32 line_old = 0;
  static HI_U32 pixel_old = 0;
  HI_U32 line;
  HI_U32 pixel;
  HI_U8  update_int = 0;
  HI_U8  update_line = 0;
  HI_U8  update_pixel = 0;
  
  if(u32IntTime != u32IntTime_old)
  {
    update_int = 1;
    u32IntTime_old = u32IntTime;
  }
  
  if(update_int)
  {
    update_int = 0;
    if(u32IntTime<(16*EXP_PRECISE))
    {
      line = u32IntTime/EXP_PRECISE;
      pixel = (ROWTIME*((EXP_PRECISE-1)&u32IntTime))/EXP_PRECISE;
    }
    else
    {
      line = (u32IntTime+(EXP_PRECISE>>1))/EXP_PRECISE;
      pixel = 0;
    }
    if(line != line_old)
    {
      update_line = 1;
      line_old = line;
    }
    if(pixel != pixel_old)
    {
      update_pixel = 1;
      pixel_old = pixel;
    }
    if(update_line||update_pixel)
    {
      HI_U8  page ;
	  uint16_t temp=0;
	  SENSOR_I2C_READ(0xF0,&temp);
	  page=0xFF&temp;

      if(page)
	  SENSOR_I2C_WRITE(0xF0, 0x00);
      if(update_line)
      {
        SENSOR_I2C_WRITE(0x0C, (line>>8) & 0xFF);
        SENSOR_I2C_WRITE(0x0D, (line) & 0xFF);
        //printf("cmos_inttime_update : line = %d\n", line);
      }
      if(update_pixel)
      {
        SENSOR_I2C_WRITE(0x26, (pixel>>8) & 0xFF);
        SENSOR_I2C_WRITE(0x27, (pixel) & 0xFF);
        //printf("cmos_inttime_update : pix = %d\n", pixel);
      }
      SENSOR_I2C_WRITE(0x1D, 0x02);
      if(page)SENSOR_I2C_WRITE(0xF0, page);
      //printf("cmos_inttime_update : TEXP = 0x%04X\n", u32IntTime);
      //printf("cmos_inttime_update : line = %d pix = %d\n", line, pixel);
    }
  }
  return;
}


static HI_VOID bg0701_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
  static HI_U8  reg_fdsel_1d = 0x55;
  HI_U8  update_fdsel = 0;
  HI_U16 darkrow_ave;
  HI_U8  page = 0;
  HI_U16 reg_offset;
  HI_U8  reg_fdsel;
  HI_U8  reg_vrefh;
  HI_U32 reg_dgain;
  uint16_t temp=0;
  
  reg_fdsel = (0x80 & u32Again) ? 0xAA : 0x55;
  FD_State = (0x80 & u32Again) ? 1 : 0;
  
  reg_vrefh = 0x7F & u32Again;
  if(reg_vrefh>VREFH_LIMIT_H) reg_vrefh = VREFH_LIMIT_H;
  if(reg_vrefh<VREFH_LIMIT_L) reg_vrefh = VREFH_LIMIT_L;
  
  reg_dgain = u32Dgain;
  if(reg_dgain>0xFFFF)reg_dgain=0xFFFF;
  
  if(reg_fdsel!=reg_fdsel_1d)
  {
    update_fdsel = 1;
    reg_fdsel_1d = reg_fdsel;
  }
  

   SENSOR_I2C_READ(0xF0,&temp);
   page=0xFF&temp;
  
  SENSOR_I2C_WRITE(0xF0, 0x01); // Page 01
  
  darkrow_ave = SEN_REG_RD_WORD(0x44, 0x45); // before DGAIN
  if(darkrow_ave>=(1<<11))darkrow_ave = 0; // Signed 1+11 bit
  reg_offset = bg0701_blcc_update(darkrow_ave, reg_dgain);
  SENSOR_I2C_WRITE(0x4E, 0xFF&(reg_offset>>8));
  SENSOR_I2C_WRITE(0x4F, 0xFF&(reg_offset));
  
  if(update_fdsel) SENSOR_I2C_WRITE(0xC7, reg_fdsel);
  
  SENSOR_I2C_WRITE(0xF0, 0x00); // Page 00
  
  if(update_fdsel)
  {
    update_fdsel = 0;
    SENSOR_I2C_WRITE(0x1D, 0x02);
    //printf("cmos_gains_update: FD Switched\n");
  }
  
  SENSOR_I2C_WRITE(0xB1, reg_vrefh);
  SENSOR_I2C_WRITE(0xB7, 0xFF&(reg_dgain>>8));
  SENSOR_I2C_WRITE(0xB8, 0xFF&(reg_dgain));
  
  SENSOR_I2C_WRITE(0xF0, page);

  return;
}

static HI_S32 bg0701_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));
    
    pstAwbSnsDft->u16WbRefTemp = 4850;

    pstAwbSnsDft->au16GainOffset[0] = 0x0193;
    pstAwbSnsDft->au16GainOffset[1] = 0x100;
    pstAwbSnsDft->au16GainOffset[2] = 0x100;
    pstAwbSnsDft->au16GainOffset[3] = 0x0149;

    pstAwbSnsDft->as32WbPara[0] = 115;
    pstAwbSnsDft->as32WbPara[1] = 1;
    pstAwbSnsDft->as32WbPara[2] = -140;
    pstAwbSnsDft->as32WbPara[3] = 202060;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -152069;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_VOID bg0701_global_init()
{

   gu8SensorMode = 0;
   
}



void bg0701_prog(int* rom) 
{
    int i = 0;
    while (1)
    {
        int u32Lookup = rom[i++];
        int addr = (u32Lookup >> 16) & 0xFFFF;
        int data = u32Lookup & 0xFFFF;
        if (addr == 0xFFFE)
        {
            SENSOR_DELAY_MS(data);
        }
        else if (addr == 0xFFFF)
        {
            return;
        }
        else
        {
            SENSOR_I2C_WRITE(addr, data);
        }
    }
}

void bg0701_reg_init()
{
  if(sensor_id==BG0701B_ID)
  {
  }
  else if(sensor_id==BG0701D_ID)
  {
  }
  else if((sensor_id&0xFFFF00)==(BG0701F_ID&0xFFFF00))
  {
  }
  else if(sensor_id==BG0701E_ID)
  {
  }
  else if(sensor_id==BG0703B_ID)
  {
  }
  else if((sensor_id&0xFFFF00)==(BG0703D_ID&0xFFFF00))
  {
  }
  else
  {
    printf("sensor_init : Invalid Sensor (ID = 0x%06X) !\n", sensor_id);
    return;
  }
  
  bg0701_prog((int *)sensor_init_seq);
  
  printf("sensor_init : BG Sensor (ID = 0x%06X) %s\n", sensor_id, INIT_PRINT);
  return ;
}


/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 bg0701_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = bg0701_reg_init;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = bg0701_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = bg0701_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = bg0701_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = bg0701_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = bg0701_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = bg0701_get_sensor_max_resolution;

    return 0;
}

HI_S32 bg0701_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default     = bg0701_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set            = bg0701_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set = bg0701_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update     = bg0701_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update       = bg0701_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table   = bg0701_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table   = bg0701_dgain_calc_table;
    
    return 0;
}

HI_S32 bg0701_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = bg0701_get_awb_default;

    return 0;
}

int bg0701_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    bg0701_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(BG070X_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    bg0701_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, BG070X_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    bg0701_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, BG070X_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int bg0701_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(BG070X_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, BG070X_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, BG070X_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

void BG0701_init(SENSOR_BG0701_DO_I2CRD do_i2c_read, SENSOR_BG0701_DO_I2CWR do_i2c_write)
{  

	
	//printf("*****%s****",__FUNCTION__);
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	bg0701_reg_init();

	bg0701_register_callback();

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


#endif // __BG0701_CMOS_H_

