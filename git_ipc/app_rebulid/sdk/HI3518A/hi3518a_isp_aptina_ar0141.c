#include "sdk/sdk_debug.h"
#include "hi3518a.h"
#include "hi3518a_isp_sensor.h"

static SENSOR_APTINA_AR0141_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_APTINA_AR0141_DO_I2CWR sensor_i2c_write = NULL;

#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : -1)

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : -1)

#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__AR0141_CMOS_H_)
#define __AR0141_CMOS_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hi_comm_sns.h"
#include "hi_sns_ctrl.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"



//#include "ar0141_sensor_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define AR0141_ID 0141

/*set Frame End Update Mode 2 with HI_MPI_ISP_SetAEAttr and set this value 1 to avoid flicker in antiflicker mode */
/*when use Frame End Update Mode 2, the speed of i2c will affect whole system's performance                       */
/*increase I2C_DFT_RATE in Hii2c.c to 400000 to increase the speed of i2c                                         */
#define CMOS_AR0141_ISP_WRITE_SENSOR_ENABLE (1)
/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr = 0x30;
static unsigned int sensor_addr_byte = 0x2;
static unsigned int sensor_data_byte = 0x2;


//extern void sensor_init_wdr();

static HI_U8 gu8SensorMode = 0;
static HI_U32 gu32FullLinesStd = 750;

#if CMOS_AR0141_ISP_WRITE_SENSOR_ENABLE
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
#endif

#define AGAIN_TABLE_NUM 78



static int ar0141_sensor_rom_30_lin[] = {
//[Linear (Parallel) 720p30 74.25MHz PCLK 24MHz MCLK]

//Reset
0x301A0001, 	// RESET_REGISTER
0x301A10D8, 	// RESET_REGISTER
//Delay=200

0xFFFE00C8,   //delay 200ms

//LOAD=sequencer_ers_0828_AR0140.i
//LOAD= AR0140 Rev3 Optimized settings
//Delay=100

//[sequencer_ers_0828_AR0140.i]
0x30888000, 
0x30864558,
0x30866E9B,
0x30864A31,
0x30864342,
0x30868E03,
0x30862714,
0x30864578,
0x30867B3D,
0x3086FF3D,
0x3086FF3D,
0x3086EA27,
0x3086043D,
0x30861027,
0x30860527,
0x30861535,
0x30862705,
0x30863D10,
0x30864558,
0x30862704,
0x30862714,
0x30863DFF,
0x30863DFF,
0x30863DEA,
0x30862704,
0x30866227,
0x3086288E,
0x30860036,
0x30862708,
0x30863D64,
0x30867A3D,
0x30860444,
0x30862C4B,
0x30868F01,
0x30864372,
0x3086719F,
0x30864643,
0x3086166F,
0x30869F92,
0x30861244,
0x30861646,
0x30864316,
0x30869326,
0x30860426,
0x3086848E,
0x30860327,
0x3086FC5C,
0x30860D57,
0x30865417,
0x30860955,
0x30865649,
0x30865F53,
0x30860553,
0x30860728,
0x30866C4C,
0x30860928,
0x30862C72,
0x3086A37C,
0x30869728,
0x3086A879,
0x30866026,
0x30869C5C,
0x30861B45,
0x30864845,
0x30860845,
0x30868826,
0x3086BE8E,
0x30860127,
0x3086F817,
0x30860227,
0x3086FA17,
0x3086095C,
0x30860B17,
0x30861026,
0x3086BA5C,
0x30860317,
0x30861026,
0x3086B217,
0x3086065F,
0x30862888,
0x30869060,
0x308627F2,
0x30861710,
0x308626A2,
0x308626A3,
0x30865F4D,
0x30862808,
0x30861A27,
0x3086FA84,
0x308669A0,
0x3086785D,
0x30862888,
0x30868710,
0x30868C82,
0x30868926,
0x3086B217,
0x3086036B,
0x30869C60,
0x30869417,
0x30862926,
0x30868345,
0x3086A817,
0x30860727,
0x3086FB17,
0x30862945,
0x30868820,
0x30861708,
0x308627FA,
0x30865D87,
0x3086108C,
0x30868289,
0x3086170E,
0x30864826,
0x30869A28,
0x3086884C,
0x30860B79,
0x30861730,
0x30862692,
0x30861709,
0x30869160,
0x308627F2,
0x30861710,
0x30862682,
0x30862683,
0x30865F4D,
0x30862808,
0x30861A27,
0x3086FA84,
0x308669A1,
0x3086785D,
0x30862888,
0x30868710,
0x30868C80,
0x30868A26,
0x30869217,
0x3086036B,
0x30869D95,
0x30862603,
0x30865C01,
0x30864558,
0x30868E00,
0x30862798,
0x3086170A,
0x30864A0A,
0x30864316,
0x30860B43,
0x30865B43,
0x30861659,
0x30864316,
0x30868E03,
0x3086279C,
0x30864578,
0x30861707,
0x3086279D,
0x30861722,
0x30865D87,
0x30861028,
0x30860853,
0x30860D8C,
0x3086808A,
0x30864558,
0x30861708,
0x30868E01,
0x30862798,
0x30868E00,
0x308676A2,
0x308677A2,
0x30864644,
0x30861616,
0x3086967A,
0x30862644,
0x30865C05,
0x30861244,
0x30864B71,
0x3086759E,
0x30868B86,
0x3086184A,
0x30860343,
0x30861606,
0x30864316,
0x30860743,
0x30861604,
0x30864316,
0x30865843,
0x3086165A,
0x30864316,
0x30864558,
0x30868E03,
0x3086279C,
0x30864578,
0x30867B17,
0x30860727,
0x30869D17,
0x30862245,
0x30865822,
0x30861710,
0x30868E01,
0x30862798,
0x30868E00,
0x30861710,
0x30861244,
0x30864B8D,
0x3086602C,
0x30862C2C,
0x30862C00,

//[==== Optimized and Sequencer settings (DO NOT CHANGE) ====]
//[AR0140 Rev3 Optimized Settings]
0x30440400, //Manufacturer-specific
0x3052A134, //Manufacturer-specific
0x3092010F, //Manufacturer-specific
0x30FE0080, //Manufacturer-specific
0x3ECE40FF, //Manufacturer-specific
0x3ED0FF40, //Manufacturer-specific
0x3ED2A906, //Manufacturer-specific
0x3ED4001F, //Manufacturer-specific
0x3ED6638F, //Manufacturer-specific
0x3ED8CC99, //Manufacturer-specific
0x3EDA0888, //Manufacturer-specific
0x3EDE8878, //Manufacturer-specific
0x3EE07744, //Manufacturer-specific
0x3EE24463, //Manufacturer-specific
0x3EE4AAE0, //Manufacturer-specific
0x3EE61400, //Manufacturer-specific
0x3EEAA4FF, //Manufacturer-specific
0x3EEC80F0, //Manufacturer-specific
0x3EEE0000, //Manufacturer-specific
0x31E01701, //Manufacturer-specific


0xFFFE0064,	//delay 100ms

//PLL_configuration_Parallel
0x302A0006, 	// VT_PIX_CLK_DIV
0x302C0001, 	// VT_SYS_CLK_DIV
0x302E0004, 	// PRE_PLL_CLK_DIV
0x30300042, 	// PLL_MULTIPLIER
0x3036000C, 	// OP_PIX_CLK_DIV
0x30380001, 	// OP_SYS_CLK_DIV

//720P30fps_configuration
0x30040012, 	// X_ADDR_START
0x30020040, 	// Y_ADDR_START
0x30080519, 	// X_ADDR_END
0x30060317, 	// Y_ADDR_END
0x300A02EE, 	// FRAME_LENGTH_LINES
0x300C0CE4, 	// LINE_LENGTH_PCK
0x3012002D, 	// COARSE_INTEGRATION_TIME
0x30A20001, 	// X_ODD_INC
0x30A60001, 	// Y_ODD_INC
0x30400000, 	// READ_MODE

//Linear Mode Setup
0x30820009,     //Linear mode
0x318C0000, 	// Motion Compensation Off
0x32000000, 	// ADACD Disabled
0x31D00000, 	// COMPANDING disabled

0x30B00000, //DIGITAL_TEST
0x30BA012C, //DIGITAL_CTRL
0x31AC0C0C, //DATA_FORMAT_BITS: 12bit
0x31AE0301, //SERAIL_FORAMT: parallel infterface

0x30641882, 	// no statistic data output    

//0x306Ef010,   // current-drive set
0x306EE810,   // current-drive set
    
0x301A10DC, //Start streaming

0xFFFF0000  //end of configuration
};

static int ar0141_sensor_rom_30_wdr[] = 
{
//[HDR (Parallel) 720p30 74.25MHz PCLK 24MHz MCLK]

//Reset
0x301A0001, 	// RESET_REGISTER
0x301A10D8, 	// RESET_REGISTER
//Delay=200
0xFFFE00C8,   //delay 200ms

//LOAD=sequencer_hidy_0828_AR0140.i
//LOAD= AR0140 Rev3 Optimized settings
//Delay=100

//[sequencer_hidy_0828_AR0140.i]
0x30888000, 
0x30864558,
0x30866E9B,
0x30864A31,
0x30864342,
0x30868E03,
0x30862714,
0x30864578,
0x30867B3D,
0x3086FF3D,
0x3086FF3D,
0x3086EA27,
0x3086043D,
0x30861027,
0x30860527,
0x30861535,
0x30862705,
0x30863D10,
0x30864558,
0x30862704,
0x30862714,
0x30863DFF,
0x30863DFF,
0x30863DEA,
0x30862704,
0x30866227,
0x3086288E,
0x30860036,
0x30862708,
0x30863D64,
0x30867A3D,
0x30860444,
0x30862C4B,
0x30868F00,
0x30864372,
0x3086719F,
0x30866343,
0x3086166F,
0x30869F92,
0x30861244,
0x30861663,
0x30864316,
0x30869326,
0x30860426,
0x3086848E,
0x30860327,
0x3086FC5C,
0x30860D57,
0x30865417,
0x30860955,
0x30865649,
0x30865F53,
0x30860553,
0x30860728,
0x30866C4C,
0x30860928,
0x30862C72,
0x3086AD7C,
0x3086A928,
0x3086A879,
0x30866026,
0x30869C5C,
0x30861B45,
0x30864845,
0x30860845,
0x30868826,
0x3086BE8E,
0x30860127,
0x3086F817,
0x30860227,
0x3086FA17,
0x3086095C,
0x30860B17,
0x30861026,
0x3086BA5C,
0x30860317,
0x30861026,
0x3086B217,
0x3086065F,
0x30862888,
0x30869060,
0x308627F2,
0x30861710,
0x308626A2,
0x308626A3,
0x30865F4D,
0x30862808,
0x30861927,
0x3086FA84,
0x308669A0,
0x3086785D,
0x30862888,
0x30868710,
0x30868C82,
0x30868926,
0x3086B217,
0x3086036B,
0x30869C60,
0x30869417,
0x30862926,
0x30868345,
0x3086A817,
0x30860727,
0x3086FB17,
0x30862945,
0x3086881F,
0x30861708,
0x308627FA,
0x30865D87,
0x3086108C,
0x30868289,
0x3086170E,
0x30864826,
0x30869A28,
0x3086884C,
0x30860B79,
0x30861730,
0x30862692,
0x30861709,
0x30869160,
0x308627F2,
0x30861710,
0x30862682,
0x30862683,
0x30865F4D,
0x30862808,
0x30861927,
0x3086FA84,
0x308669A1,
0x3086785D,
0x30862888,
0x30868710,
0x30868C80,
0x30868A26,
0x30869217,
0x3086036B,
0x30869D95,
0x30862603,
0x30865C01,
0x30864558,
0x30868E00,
0x30862798,
0x3086170A,
0x30864A65,
0x30864316,
0x30866643,
0x3086165B,
0x30864316,
0x30865943,
0x3086168E,
0x30860327,
0x30869C45,
0x30867817,
0x30860727,
0x30869D17,
0x3086225D,
0x30868710,
0x30862808,
0x3086530D,
0x30868C80,
0x30868A45,
0x30865823,
0x30861708,
0x30868E01,
0x30862798,
0x30868E00,
0x30862644,
0x30865C05,
0x30861244,
0x30864B71,
0x3086759E,
0x30868B85,
0x30860143,
0x30867271,
0x3086A346,
0x30864316,
0x30866FA3,
0x30869612,
0x30864416,
0x30864643,
0x30861697,
0x30862604,
0x30862684,
0x30868E03,
0x308627FC,
0x30865C0D,
0x30865754,
0x30861709,
0x30865556,
0x3086495F,
0x30865305,
0x30865307,
0x3086286C,
0x30864C09,
0x3086282C,
0x308672AE,
0x30867CAA,
0x308628A8,
0x30867960,
0x3086269C,
0x30865C1B,
0x30864548,
0x30864508,
0x30864588,
0x308626BE,
0x30868E01,
0x308627F8,
0x30861702,
0x308627FA,
0x30861709,
0x30865C0B,
0x30861710,
0x308626BA,
0x30865C03,
0x30861710,
0x308626B2,
0x30861706,
0x30865F28,
0x30868898,
0x30866027,
0x3086F217,
0x30861026,
0x3086A226,
0x3086A35F,
0x30864D28,
0x3086081A,
0x308627FA,
0x30868469,
0x3086A578,
0x30865D28,
0x30868887,
0x3086108C,
0x30868289,
0x308626B2,
0x30861703,
0x30866BA4,
0x30866099,
0x30861729,
0x30862683,
0x308645A8,
0x30861707,
0x308627FB,
0x30861729,
0x30864588,
0x30862017,
0x30860827,
0x3086FA5D,
0x30868710,
0x30868C82,
0x30868917,
0x30860E48,
0x3086269A,
0x30862888,
0x30864C0B,
0x30867917,
0x30863026,
0x30869217,
0x3086099A,
0x30866027,
0x3086F217,
0x30861026,
0x30868226,
0x3086835F,
0x30864D28,
0x3086081A,
0x308627FA,
0x30868469,
0x3086AB78,
0x30865D28,
0x30868887,
0x3086108C,
0x3086808A,
0x30862692,
0x30861703,
0x30866BA6,
0x3086A726,
0x3086035C,
0x30860145,
0x3086588E,
0x30860027,
0x30869817,
0x30860A4A,
0x30860A43,
0x3086160B,
0x3086438E,
0x30860327,
0x30869C45,
0x30867817,
0x30860727,
0x30869D17,
0x3086225D,
0x30868710,
0x30862808,
0x3086530D,
0x30868C80,
0x30868A45,
0x30865817,
0x3086088E,
0x30860127,
0x3086988E,
0x30860076,
0x3086AC77,
0x3086AC46,
0x30864416,
0x308616A8,
0x30867A26,
0x3086445C,
0x30860512,
0x3086444B,
0x30867175,
0x3086A24A,
0x30860343,
0x30861604,
0x30864316,
0x30865843,
0x3086165A,
0x30864316,
0x30860643,
0x30861607,
0x30864316,
0x30868E03,
0x3086279C,
0x30864578,
0x30867B17,
0x3086078B,
0x30868627,
0x30869D17,
0x30862345,
0x30865822,
0x30861708,
0x30868E01,
0x30862798,
0x30868E00,
0x30862644,
0x30865C05,
0x30861244,
0x30864B8D,
0x3086602C,
0x30862C2C,
0x30862C00,

//[==== Optimized and Sequencer settings (DO NOT CHANGE) ====]
//[AR0140 Rev3 Optimized Settings]
0x30440400, //Manufacturer-specific
0x3052A134, //Manufacturer-specific
0x3092010F, //Manufacturer-specific
0x30FE0080, //Manufacturer-specific
0x3ECE40FF, //Manufacturer-specific
0x3ED0FF40, //Manufacturer-specific
0x3ED2A906, //Manufacturer-specific
0x3ED4001F, //Manufacturer-specific
0x3ED6638F, //Manufacturer-specific
0x3ED8CC99, //Manufacturer-specific
0x3EDA0888, //Manufacturer-specific
0x3EDE8878, //Manufacturer-specific
0x3EE07744, //Manufacturer-specific
0x3EE24463, //Manufacturer-specific
0x3EE4AAE0, //Manufacturer-specific
0x3EE61400, //Manufacturer-specific
0x3EEAA4FF, //Manufacturer-specific
0x3EEC80F0, //Manufacturer-specific
0x3EEE0000, //Manufacturer-specific
0x31E01701, //Manufacturer-specific

0xFFFE0064, //delay 100ms


//PLL_configuration_Parallel
0x302A0006, 	// VT_PIX_CLK_DIV
0x302C0001, 	// VT_SYS_CLK_DIV
0x302E0004, 	// PRE_PLL_CLK_DIV
0x30300042, 	// PLL_MULTIPLIER
0x3036000C, 	// OP_PIX_CLK_DIV
0x30380001, 	// OP_SYS_CLK_DIV

//720P30fps_configuration
0x30040012, 	// X_ADDR_START
0x30020040, 	// Y_ADDR_START
0x30080519, 	// X_ADDR_END
0x30060317, 	// Y_ADDR_END
0x300A05D4, 	// FRAME_LENGTH_LINES
0x300C0672, 	// LINE_LENGTH_PCK
0x3012002D, 	// COARSE_INTEGRATION_TIME
0x30A20001, 	// X_ODD_INC
0x30A60001, 	// Y_ODD_INC
0x30400000, 	// READ_MODE

//Companding_enabled_16to12
0x31AC100C, 	// DATA_FORMAT_BITS
0x31D00001, 	// COMPANDING

//HDR Mode 16x Setup
0x305E0080, // global_gain
0x30820008,     //HDR mode
0x318CC000, 	// Motion Compensation On
0x320A0080, 	// ADACD_PEDESTAL
0x32060A06, 	// ADACD_NOISE_FLOOR1
0x32060A06, 	// ADACD_NOISE_FLOOR1
0x32081A12, 	// ADACD_NOISE_FLOOR2
0x32081A12, 	// ADACD_NOISE_FLOOR2
0x320200A0, 	// ADACD_NOISE_MODEL1
0x32000002, 	// ADACD_CONTROL
0x31AC100C, 	// DATA_FORMAT_BITS
0x31D00001, 	// COMPANDING
0x318A0E74, 	// HDR_MC_CTRL1
0x31920400, 	// HDR_MC_CTRL5
0x3198183C,   //Motion detect Q1 set to 60, Q2 set to 24
0x318E0800,   //Gain before DLO set to 1
0x31940BB8,   //T1 barrier set to 3000
0x31960E74,   //T2 barrier set to 3700

0x30B00000, //DIGITAL_TEST
0x30BA012C, //DIGITAL_CTRL
0x31AE0301, //SERAIL_FORAMT: parallel infterface

0x30641882, 	// no statistic data output    
0x306EF010,   // current-drive set

0x301A10DC, //Start streaming

0xFFFF0000  //end of configuration

};


static AWB_CCM_S g_stAwbCcm =
{
    4850,
    {
        0x0180,0x8063,0x801d,
     //   0x01a0,0x8083,0x801d,
        0x8033,0x0128,0x000a,
        0x0010,0x808a,0x0179
    },

    3160,
    {
        0x162,0x8036,0x802c,
        0x804c,0x133,0x0018,
        0x000f,0x80d1,0x1c1
    },

    2470,
    {
        0x0142,0x6,0x8049,
        0x802a,0x00ee,0x003b,
        0x8007,0x80fc,0x0203
    }
};

static AWB_AGC_TABLE_S g_stAwbAgcTableLin =
{
    /* bvalid */
    1,

    /* saturation */
  //  {0x80,0x7C,0x78,0x6E,0x64,0x50,0x46,0x3C}
	
    {0x80,0x80,0x78,0x78,0x78,0x70,0x66,0x3C}
};



static ISP_CMOS_AGC_TABLE_S g_stIspAgcTableLin =
{
    /* bvalid */
    1,

    /* sharpen_alt_d */
 //   {0xa2,0xa0,0x8a,0x70,0x60,0x58,0x50,0x48},
	{0xa2,0xa0,0x88,0x80,0x78,0x70,0x68,0x48},
        
    /* sharpen_alt_ud */
 //   {0xae,0xab,0x98,0x70,0x60,0x5a,0x50,0x48},
    {0xae,0xab,0x8f,0x78,0x74,0x70,0x68,0x48},
        
    /* snr_thresh */
  //  {0xc,0x10,0x28,0x3a,0x46,0x50,0x54,0x5A},
	{0xc,0x10,0x1a,0x20,0x30,0x34,0x40,0x50},
        
    /* demosaic_lum_thresh */
    {0x60,0x60,0x80,0x80,0x80,0x80,0x80,0x80},
        
    /* demosaic_np_offset */
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x37},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37}
};

static ISP_CMOS_AGC_TABLE_S g_stIspAgcTableWdr =
{
    /* bvalid */
    1,

    /* sharpen_alt_d */
    {0x50,0x50,0x50,0x50,0x4a,0x45,0x40,0x3a},
        
    /* sharpen_alt_ud */
    {0x90,0x90,0x90,0x90,0x88,0x80,0x78,0x70},
        
    /* snr_thresh */
    {0x05,0x05,0x05,0x05,0x08,0x0c,0x10,0x18},
        
    /* demosaic_lum_thresh */
    {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
        
    /* demosaic_np_offset */
    {0x0,0xa,0x12,0x1a,0x20,0x28,0x30,0x37},
        
    /* ge_strength */
    {0x55,0x55,0x55,0x55,0x55,0x55,0x37,0x37}
};


static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTableLin =
{
    /* bvalid */
    1,
    
      /* nosie_profile_weight_lut */
    /*
     {
      0,0,0,0,0,6,13,18,21,23,25,27,28,30,31,32,33,34,35,36,36,37,38,38,39,39,40,41,41,
	41,42,42,43,43,44,44,44,45,45,45,46,46,46,47,47,47,47,48,48,48,48,49,49,49,49,50,
	50,50,50,51,51,51,51,51,52,52,52,52,52,52,53,53,53,53,53,53,54,54,54,54,54,54,55,
	55,55,55,55,55,55,56,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,58,58,58,58,58,
	58,58,58,58,59,59,59,59,59,59,59,59,59,59,60,60,60,60 
     }, 
     */
      	{0, 0, 0, 0, 0 ,3, 12, 17, 20, 23, 25, 27, 28, 30 ,31 ,32, 33, 34, 35 ,35, 37 ,37, 38, 38, 39, 40, 40, 41, 41,
      	42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 46, 46, 46, 46, 47, 47, 47, 48 ,48, 48, 48 ,49, 49, 49, 49, 50, 50,
      	50, 50, 50, 51, 51, 51 ,51 ,52, 52, 52, 52 ,52, 52, 53, 53, 53, 53, 53, 53, 54 ,54, 54, 54, 54, 54 ,55, 55,
      55,55,55,55,55,56,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,58,58,58,58,58,58,
      58,58,58,59,59,59,59,59,59,59,59,59,59,60,60,60,60,60
      	},
    
     /* demosaic_weight_lut */
    /*
    {
       0,6,13,18,21,23,25,27,28,30,31,32,33,34,35,36,36,37,38,38,39,39,40,41,41,
	41,42,42,43,43,44,44,44,45,45,45,46,46,46,47,47,47,47,48,48,48,48,49,49,49,49,50,
	50,50,50,51,51,51,51,51,52,52,52,52,52,52,53,53,53,53,53,53,54,54,54,54,54,54,55,
	55,55,55,55,55,55,56,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,58,58,58,58,58,
	58,58,58,58,59,59,59,59,59,59,59,59,59,59,60,60,60,60, 60,60,60,60
    }
    */
	{
	3,12,17,20,23,25,27,28,30,31,32,33,34,35,35,37,37,38,38,39,40,40,41,41,42,42,43,43,
	43,44,44,44,45,45,46,46,46,46,47,47,47,48,48,48,48,49,49,49,49,50,50,50,50,50,
	51,51,51,51,52,52,52,52,52,52,53,53,53,53,53,53,54,54,54,54,54,54,55,55,55,55,
	55,55,55,56,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,58,58,58,58,58,58,58,
	58,58,59,59,59,59,59,59,59,59,59,59,60,60,60,60,60,60,60,60,60,60
   	}
      
};

#if 0
static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTableWdr =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut */
    {
        13,13,13,13,13,14,15,25,31,31,31,31,31,31,31,31,31,31,31,31,31,32,32,32,32,32,32,32,39,49,54,56,58,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60
    },

    /* demosaic_weight_lut */
    {
        13,13,13,13,13,14,15,25,31,31,31,31,31,31,31,31,31,31,31,31,31,32,32,32,32,32,32,32,39,49,54,56,58,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60
    }
};
#endif

static ISP_CMOS_NOISE_TABLE_S g_stIspNoiseTableWdr =
{
    /* bvalid */
    1,
    
    /* nosie_profile_weight_lut WDR */
    {
        13,13,13,13,13,14,15,25,31,31,31,31,31,31,31,31,31,31,31,31,31,32,32,32,
        32,32,32,32,39,49,54,56,58,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,
        59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,
        59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,60,60,60,60,60,60,60,60,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60
    },

    /* demosaic_weight_lut WDR */
    {
        13,13,13,13,13,14,15,25,31,31,31,31,31,31,31,31,31,31,31,31,31,32,32,32,
        32,32,32,32,39,49,54,56,58,59,59,59,59,59,59,59,59,59,59,59,59,59,59,
        59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,
        59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,60,60,60,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,
        60,60,60
    }
};


static ISP_CMOS_DEMOSAIC_S g_stIspDemosaicLin =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xB9,

    /*aa_slope*/
    0x9B,

    /*va_slope*/
    0x9E,

    /*uu_slope*/
    0xA2,

    /*sat_slope*/
    0x5D,

    /*ac_slope*/
    0xCF,

    /*vh_thresh*/
    0x0,

    /*aa_thresh*/
    0x0,

    /*va_thresh*/
    0x0,

    /*uu_thresh*/
    0x8,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaicWdr =
{
    /* bvalid */
    1,
    
    /*vh_slope*/
    0xA0,

    /*aa_slope*/
    0x6E,

    /*va_slope*/
    0x96,

    /*uu_slope*/
    0x78,

    /*sat_slope*/
    0x5D,

    /*ac_slope*/
    0xCF,

    /*vh_thresh*/
    0x78,

    /*aa_thresh*/
    0x73,

    /*va_thresh*/
    0x6E,

    /*uu_thresh*/
    0x67,

    /*sat_thresh*/
    0x171,

    /*ac_thresh*/
    0x1b3
};


static ISP_CMOS_GAMMAFE_S g_stGammafe = 
{
    /* bvalid */
    1,

    { 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 78, 101, 120, 136, 150, 163, 175, 187, 197, 208, 217, 227, 235,
	244, 252, 260, 268, 276, 283, 290, 297, 304, 311, 317, 324, 330, 336, 342, 348, 354, 360, 365, 
	371, 376, 382, 387, 392, 398, 403, 408, 413, 418, 423, 427, 432, 437, 442, 446, 451, 455, 460, 
	464, 469, 477, 486, 494, 502, 511, 519, 526, 534, 542, 549, 557, 564, 571, 578, 585, 592, 599, 
	606, 613, 619, 626, 633, 639, 645, 652, 658, 664, 670, 677, 683, 689, 694, 700, 706, 712, 718, 
	723, 729, 735, 740, 746, 751, 757, 762, 767, 773, 778, 783, 789, 794, 799, 804, 809, 814, 819, 
	824, 829, 834, 839, 844, 849, 854, 858, 863, 868, 873, 877, 882, 887, 891, 896, 900, 905, 909, 
	914, 918, 923, 927, 932, 936, 940, 945, 949, 953, 958, 962, 966, 971, 975, 979, 983, 987, 991,
	996, 1000, 1004, 1067, 1127, 1184, 1238, 1290, 1340, 1388, 1435, 1480, 1524, 1566, 1607, 1648, 
	1687, 1726, 1763, 1800, 1836, 1872, 1906, 1941, 1974, 2007, 2040, 2072, 2103, 2134, 2165, 2195, 
	2224, 2254, 2283, 2311, 2340, 2367, 2395, 2422, 2449, 2476, 2502, 2528, 2554, 2580, 2605, 2630, 
	2655, 2680, 2704, 2728, 2752, 2776, 2800, 2823, 2846, 2869, 2892, 2937, 2981, 3025, 3068, 3111, 
	3153, 3194, 3235, 3275, 3315, 3354, 3393, 3432, 3470, 3508, 3545, 3582, 3618, 3654, 3690, 3726,
	3761, 3795, 3830, 3864, 3898, 3931, 3965, 3998, 4030, 4063, 4095, 4095, 4095, 4095, 4095, 4095,
	4095, 4095, 4095,
    }
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


static ISP_CMOS_GAMMA_S g_stGammaWdr =
{
    /* bvalid */
    1,
    
    {0, 0, 1, 2, 3, 5, 8, 10, 14, 17, 21, 26, 30, 36, 41, 47, 54, 61, 68, 75, 83, 92, 100, 109, 119, 129, 139,
    150, 161, 173, 184, 196, 209, 222, 235, 248, 262, 276, 290, 305, 320, 335, 351, 366, 382, 399, 415,
    433, 450, 467, 484, 502, 520, 539, 557, 576, 595, 614, 634, 653, 673, 693, 714, 734, 754, 775, 796, 
    816, 837, 858, 879, 901, 923, 944, 966, 988, 1010, 1032, 1054, 1076, 1098, 1120, 1142, 1165, 1188,
    1210, 1232, 1255, 1278, 1301, 1324, 1346, 1369, 1391, 1414, 1437, 1460, 1483, 1505, 1528, 1551, 1574, 
    1597, 1619, 1642, 1665, 1687, 1710, 1732, 1755, 1777, 1799, 1822, 1845, 1867, 1889, 1911, 1933, 1955, 
    1977, 1999, 2021, 2043, 2064, 2086, 2108, 2129, 2150, 2172, 2193, 2214, 2236, 2256, 2277, 2298, 2319, 
    2340, 2360, 2380, 2401, 2421, 2441, 2461, 2481, 2501, 2521, 2541, 2560, 2580, 2599, 2618, 2637, 2656, 
    2675, 2694, 2713, 2732, 2750, 2769, 2787, 2805, 2823, 2841, 2859, 2877, 2895, 2912, 2929, 2947, 2964, 
    2982, 2999, 3015, 3032, 3049, 3066, 3082, 3099, 3115, 3131, 3147, 3164, 3179, 3195, 3211, 3227, 3242, 
    3258, 3273, 3288, 3303, 3318, 3333, 3348, 3362, 3377, 3392, 3406, 3420, 3434, 3448, 3462, 3477, 3490,
    3504, 3517, 3531, 3544, 3558, 3571, 3584, 3597, 3611, 3623, 3636, 3649, 3662, 3674, 3686, 3698, 3711, 
    3723, 3736, 3748, 3759, 3771, 3783, 3795, 3806, 3818, 3829, 3841, 3852, 3863, 3874, 3885, 3896, 3907, 
    3918, 3929, 3939, 3949, 3961, 3971, 3981, 3991, 4001, 4012, 4022, 4032, 4042, 4051, 4061, 4071, 4081, 
    4090, 4095,}
};


static  HI_U32   au32Again_table[AGAIN_TABLE_NUM] = 
{ 
    1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,2048,2176,2304,
    2432,2560,2662,2828,2995,3161,3328,3494,3660,3827,3993,4160,4326,4492,4659,4825,4992,5158,5324,
    5657,5990,6323,6656,6988,7321,7654,7987,8320,8652,8985,9318,9651,9984,10316,10649,11315,11980,
    12646,13312,13977,14643,15308,15974,16640,17305,17971,18636,19302,19968,20633,21299,22630,23961,
    25292,26624,27955,29286,30617,31948
};


static HI_VOID ar0141_again_calc_table(HI_U32 u32InTimes,AE_SENSOR_GAININFO_S *pstAeSnsGainInfo)
{
    int i;

    if(HI_NULL == pstAeSnsGainInfo)
    {
        printf("null pointer when get ae sensor gain info  value!\n");
        return;
    }
 
    pstAeSnsGainInfo->u32GainDb = 0;
    pstAeSnsGainInfo->u32SnsTimes = 1024;
   
    if (u32InTimes >= au32Again_table[AGAIN_TABLE_NUM -1])
    {
         pstAeSnsGainInfo->u32SnsTimes = au32Again_table[AGAIN_TABLE_NUM -1];
         pstAeSnsGainInfo->u32GainDb = AGAIN_TABLE_NUM -1;
         return ;
    }
    
    for(i = 1; i < AGAIN_TABLE_NUM; i++)
    {
        if(u32InTimes < au32Again_table[i])
        {
            pstAeSnsGainInfo->u32SnsTimes = au32Again_table[i - 1];
            pstAeSnsGainInfo->u32GainDb = i - 1;
            break;
        }

    }
          
    return;

}


HI_U32 ar0141_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));
    
    switch (gu8SensorMode)
    {
        default:
        case 0:
            pstDef->stComm.u8Rggb           = 0x1;      // 1: grbg
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

            memcpy(&pstDef->stDemosaic, &g_stIspDemosaicLin, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTableLin, sizeof(ISP_CMOS_NOISE_TABLE_S));
            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTableLin, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stGamma, &g_stIspGamma, sizeof(ISP_CMOS_GAMMA_S));
        break;
        case 1:
            pstDef->stComm.u8Rggb           = 0x1;      // 1 :grbg 
            pstDef->stComm.u8BalanceFe      = 0x0;

            pstDef->stDenoise.u8SinterThresh= 0x9;
            pstDef->stDenoise.u8NoiseProfile= 0x0;      //0: use default profile table; 1: use calibrated profile lut, the setting for nr0 and nr1 must be correct.
            pstDef->stDenoise.u16Nr0        = 0x0;
            pstDef->stDenoise.u16Nr1        = 0x0;

            pstDef->stDrc.u8DrcBlack        = 0x00;
            pstDef->stDrc.u8DrcVs           = 0x04;     // variance space
            pstDef->stDrc.u8DrcVi           = 0x04;     // variance intensity
            pstDef->stDrc.u8DrcSm           = 0x3c;     // slope max
            pstDef->stDrc.u16DrcWl          = 0xFFF;    // white level

            memcpy(&pstDef->stDemosaic, &g_stIspDemosaicWdr, sizeof(ISP_CMOS_DEMOSAIC_S));
            memcpy(&pstDef->stNoiseTbl, &g_stIspNoiseTableWdr, sizeof(ISP_CMOS_NOISE_TABLE_S));
            memcpy(&pstDef->stGammafe, &g_stGammafe, sizeof(ISP_CMOS_GAMMAFE_S));
            memcpy(&pstDef->stAgcTbl, &g_stIspAgcTableWdr, sizeof(ISP_CMOS_AGC_TABLE_S));
            memcpy(&pstDef->stGamma, &g_stGammaWdr, sizeof(ISP_CMOS_GAMMA_S));
            
        break;
    }

    return 0;
}

HI_U32 ar0141_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
    HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;

    switch (gu8SensorMode)
    {
        default :
        case 0 :
			pstBlackLevel->au16BlackLevel[0] = 0xB3;
			pstBlackLevel->au16BlackLevel[1] = 0xAA;
			pstBlackLevel->au16BlackLevel[2] = 0xAA;
			pstBlackLevel->au16BlackLevel[3] = 0xB8;
			
     /*       for (i=0; i<4; i++)
            {
                pstBlackLevel->au16BlackLevel[i] = 0xAA;
            }
            */
            break;
        case 1 :
            for (i=0; i<4; i++)
            {
                pstBlackLevel->au16BlackLevel[i] = 0x0;
            }
            break;
    }

    return 0;    
}

HI_VOID ar0141_set_pixel_detect(HI_BOOL bEnable)
{
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        SENSOR_I2C_WRITE(0x300C, 0x4D58);    //5fps
        SENSOR_I2C_WRITE(0x3012, 0x2EC);    //max exposure lines
        SENSOR_I2C_WRITE(0x3060, 0x0000);    //AG, Context A

        //DG
     //  SENSOR_I2C_WRITE(0x3056, 0x0080);    // Gr
   //    SENSOR_I2C_WRITE(0x3058, 0x0080);    // Blue_gain
     //  SENSOR_I2C_WRITE(0x305A, 0x0080);    // reg_gain
     //  SENSOR_I2C_WRITE(0x305C, 0x0080);    // Gb
       SENSOR_I2C_WRITE(0x305E, 0x0080);    //Global_gain
    }
    else /* setup for ISP 'normal mode' */
    {
        SENSOR_I2C_WRITE(0x300C, 0xCE4);    //30fps
    }

    return;
}

HI_VOID ar0141_set_wdr_mode(HI_U8 u8Mode)
{
    switch(u8Mode)
    {
        //720P30 linear
        case 0:
            gu8SensorMode = 0;
            printf("linear mode\n");

            /* program sensor to linear mode */
            //ar0141_reg_prog(ar0141_sensor_rom_30_lin);


        break;

        //720P30 wdr
        case 1:
            gu8SensorMode = 1;
            printf("wdr mode\n");

            /* program sensor to wdr mode */
            //ar0141_reg_prog(ar0141_sensor_rom_30_wdr);
	//	sensor_init_wdr();

        break;

        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    
    return;
}

static HI_S32 ar0141_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    gu32FullLinesStd = 750;
    pstAeSnsDft->u32LinesPer500ms = 750*30/2;
    pstAeSnsDft->u32FlickerFreq = 0;//60*256;//50*256;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0625;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0078125;
    pstAeSnsDft->u32ISPDgainShift = 8;
    switch(gu8SensorMode)
    {
        default:
        case 0: //linear mode
            pstAeSnsDft->au8HistThresh[0] = 0xd;
            pstAeSnsDft->au8HistThresh[1] = 0x28;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 0x40;
            
            pstAeSnsDft->u32MaxIntTime = 748;
            pstAeSnsDft->u32MinIntTime = 2;
            pstAeSnsDft->u32MaxIntTimeTarget = 65535;
            pstAeSnsDft->u32MinIntTimeTarget = 2;
            
			//pstAeSnsDft->u32MaxAgain = 15974;
			pstAeSnsDft->u32MaxAgain = 12288;			//  max Again: 12x15974
            pstAeSnsDft->u32MinAgain = 1024;
        	pstAeSnsDft->u32MaxAgainTarget = 12288;
			//pstAeSnsDft->u32MaxAgainTarget = 15974;
            pstAeSnsDft->u32MinAgainTarget = 1024;
            
            pstAeSnsDft->u32MaxDgain = 2047;  /* 8 / 0.03125 = 256 */
            pstAeSnsDft->u32MinDgain = 128;
            pstAeSnsDft->u32MaxDgainTarget = 2047;
            pstAeSnsDft->u32MinDgainTarget = 128;
            
            pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
        break;
        case 1: //WDR mode
            pstAeSnsDft->au8HistThresh[0] = 0x20;
            pstAeSnsDft->au8HistThresh[1] = 0x40;
            pstAeSnsDft->au8HistThresh[2] = 0x60;
            pstAeSnsDft->au8HistThresh[3] = 0x80;
            
            pstAeSnsDft->u8AeCompensation = 22;

#if 0
            pstAeSnsDft->u32MaxIntTime = 675;//1440;
            pstAeSnsDft->u32MinIntTime = 128;//8;
            pstAeSnsDft->u32MaxIntTimeTarget = 675;//1440;  /* for short exposure, Exposure ratio = 16X */
            pstAeSnsDft->u32MinIntTimeTarget = 128;//8;
#endif
            pstAeSnsDft->u32MaxIntTime = 698;  //750 - 52 = 698
            pstAeSnsDft->u32MinIntTime = 8;
            pstAeSnsDft->u32MaxIntTimeTarget = 698;  /* for short exposure, Exposure ratio = 16X */
            pstAeSnsDft->u32MinIntTimeTarget = 8;

           pstAeSnsDft->u32MaxAgain = 12288;			
         // pstAeSnsDft->u32MaxAgain = 1024;			//  max Again: 12x
            pstAeSnsDft->u32MinAgain = 1024;
           pstAeSnsDft->u32MaxAgainTarget = 12288;	//  max Again target: 12x
          //  pstAeSnsDft->u32MaxAgainTarget = 1024;
            pstAeSnsDft->u32MinAgainTarget = 1024;
            
            pstAeSnsDft->u32MaxDgain = 2047;  /* 8 / 0.03125 = 256 */
            pstAeSnsDft->u32MinDgain = 128;
            pstAeSnsDft->u32MaxDgainTarget = 2047;
            pstAeSnsDft->u32MinDgainTarget = 128;
            
         //   pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift;
            pstAeSnsDft->u32MaxISPDgainTarget = 32 << pstAeSnsDft->u32ISPDgainShift;
        break;
    }

    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;

    return 0;
}

static HI_S32 ar0141_get_sensor_max_resolution(ISP_CMOS_SENSOR_MAX_RESOLUTION *pstSensorMaxResolution)
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
static HI_VOID ar0141_fps_set(HI_U8 u8Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    switch(u8Fps)
    {
        case 30:
            gu32FullLinesStd = 750;
            pstAeSnsDft->u32MaxIntTime = 748;
            pstAeSnsDft->u32LinesPer500ms = 750 * 30 / 2;
            SENSOR_I2C_WRITE(0x300C, 0xCE4);
        break;
        case 25:
            gu32FullLinesStd = 750;
            pstAeSnsDft->u32MaxIntTime = 748;
            pstAeSnsDft->u32LinesPer500ms = 750 * 25 / 2;
            SENSOR_I2C_WRITE(0x300C, 0xF78);
        break;        
        default:
        break;
    }

    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    return;
}

static HI_VOID ar0141_slow_framerate_set(HI_U16 u16FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
  
    SENSOR_I2C_WRITE(0x300A, u16FullLines);

    pstAeSnsDft->u32MaxIntTime = u16FullLines - 2;
    
    return;
}

static HI_VOID ar0141_init_regs_info(HI_VOID)
{
#if CMOS_AR0141_ISP_WRITE_SENSOR_ENABLE
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
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3012;		//exposure time
        g_stSnsRegsInfo.astI2cData[1].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3060;		//Again
        g_stSnsRegsInfo.astI2cData[2].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x305E;      //Dgain
         g_stSnsRegsInfo.astI2cData[3].bDelayCfg = HI_FALSE;
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3100;      //DCGgain       
        g_stSnsRegsInfo.bDelayCfgIspDgain = HI_TRUE;

        bInit = HI_TRUE;
    }
#endif
    return;
}


/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID ar0141_inttime_update(HI_U32 u32IntTime)
{

//printf("u32IntTime = %d\n",u32IntTime);

  // u32IntTime = (u32IntTime  + 8)/16 *16;


#if CMOS_AR0141_ISP_WRITE_SENSOR_ENABLE
    ar0141_init_regs_info();
    g_stSnsRegsInfo.astI2cData[0].u32Data = u32IntTime;
#else
    SENSOR_I2C_WRITE(0x3012, u32IntTime);
#endif
    return;
}

static HI_VOID ar0141_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{

   HI_U32 u32DCGgain = 0;
//printf(" u32Again ([6:4]:[3:0])= 0x%x,u32Dgain ([10:7]:[6:0])=0x%x\n\n",u32Again,u32Dgain);

  if(u32Again < 21)
    u32DCGgain = 0;
  else
    {
    u32DCGgain = 4;
    u32Again = u32Again - 21;
    }


#if CMOS_AR0141_ISP_WRITE_SENSOR_ENABLE
    ar0141_init_regs_info();
    g_stSnsRegsInfo.astI2cData[1].u32Data = u32Again;
    g_stSnsRegsInfo.astI2cData[2].u32Data = u32Dgain;
    g_stSnsRegsInfo.astI2cData[3].u32Data = u32DCGgain;
    
    HI_MPI_ISP_SnsRegsCfg(&g_stSnsRegsInfo);
#else
    SENSOR_I2C_WRITE(0x3060, u32Again);
    SENSOR_I2C_WRITE(0x3100, u32DCGgain);
    SENSOR_I2C_WRITE(0x305E, u32Dgain);
#endif

    return;
}

static HI_S32 ar0141_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4850;

    pstAwbSnsDft->au16GainOffset[0] = 0x0187;
    pstAwbSnsDft->au16GainOffset[1] = 0x0100;
    pstAwbSnsDft->au16GainOffset[2] = 0x0100;
    pstAwbSnsDft->au16GainOffset[3] = 0x01A7;

    pstAwbSnsDft->as32WbPara[0] = 86;
    pstAwbSnsDft->as32WbPara[1] = -9;
    pstAwbSnsDft->as32WbPara[2] = -179;
    pstAwbSnsDft->as32WbPara[3] = 223078;
    pstAwbSnsDft->as32WbPara[4] = 128;
    pstAwbSnsDft->as32WbPara[5] = -174506;

    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));

    
    switch (gu8SensorMode)
    {
        default:
        case 0:
            memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTableLin, sizeof(AWB_AGC_TABLE_S));
        break;
        case 1:

        break;
    }
    
    return 0;
}

HI_VOID ar0141_global_init()
{

   gu8SensorMode = 0;
   
}
static void ar0141_delay_ms(int ms) { 
    usleep(ms*1000);
}




HI_VOID ar0141_reg_prog(int* rom)
{  
	int i = 0;
    while (1) {
        int lookup = rom[i++];
        int addr = (lookup >> 16) & 0xFFFF;
        int data = lookup & 0xFFFF;
        if (addr == 0xFFFE) {
            ar0141_delay_ms(data);
        } else if (addr == 0xFFFF) {
            return;
        } else {
			SENSOR_I2C_WRITE(addr, data);
        }
    }
}


HI_VOID ar0141_reg_init()
{
	ar0141_reg_prog(ar0141_sensor_rom_30_lin);

}



HI_VOID ar0141_sensor_init_wdr()
{
	//Reset
		SENSOR_I2C_WRITE( 0x301A, 0x0001); 	// RESET_REGISTER
		SENSOR_I2C_WRITE( 0x301A, 0x10D8); 	// RESET_REGISTER
		ar0141_delay_ms(200);
	
		//LOAD=sequencer_hidy_0828_AR0140.i
		//sensor_init_hidy();
	    SENSOR_I2C_WRITE(0x3088, 0x8000);
	    SENSOR_I2C_WRITE(0x3086, 0x4558);
	    SENSOR_I2C_WRITE(0x3086, 0x6E9B);
	    SENSOR_I2C_WRITE(0x3086, 0x4A31);
	    SENSOR_I2C_WRITE(0x3086, 0x4342);
	    SENSOR_I2C_WRITE(0x3086, 0x8E03);
	    SENSOR_I2C_WRITE(0x3086, 0x2714);
	    SENSOR_I2C_WRITE(0x3086, 0x4578);
	    SENSOR_I2C_WRITE(0x3086, 0x7B3D);
	    SENSOR_I2C_WRITE(0x3086, 0xFF3D);
	    SENSOR_I2C_WRITE(0x3086, 0xFF3D);
	    SENSOR_I2C_WRITE(0x3086, 0xEA27);
	    SENSOR_I2C_WRITE(0x3086, 0x043D);
	    SENSOR_I2C_WRITE(0x3086, 0x1027);
	    SENSOR_I2C_WRITE(0x3086, 0x0527);
	    SENSOR_I2C_WRITE(0x3086, 0x1535);
	    SENSOR_I2C_WRITE(0x3086, 0x2705);
	    SENSOR_I2C_WRITE(0x3086, 0x3D10);
	    SENSOR_I2C_WRITE(0x3086, 0x4558);
	    SENSOR_I2C_WRITE(0x3086, 0x2704);
	    SENSOR_I2C_WRITE(0x3086, 0x2714);
	    SENSOR_I2C_WRITE(0x3086, 0x3DFF);
	    SENSOR_I2C_WRITE(0x3086, 0x3DFF);
	    SENSOR_I2C_WRITE(0x3086, 0x3DEA);
	    SENSOR_I2C_WRITE(0x3086, 0x2704);
	    SENSOR_I2C_WRITE(0x3086, 0x6227);
	    SENSOR_I2C_WRITE(0x3086, 0x288E);
	    SENSOR_I2C_WRITE(0x3086, 0x0036);
	    SENSOR_I2C_WRITE(0x3086, 0x2708);
	    SENSOR_I2C_WRITE(0x3086, 0x3D64);
	    SENSOR_I2C_WRITE(0x3086, 0x7A3D);
	    SENSOR_I2C_WRITE(0x3086, 0x0444);
	    SENSOR_I2C_WRITE(0x3086, 0x2C4B);
	    SENSOR_I2C_WRITE(0x3086, 0x8F00);
	    SENSOR_I2C_WRITE(0x3086, 0x4372);
	    SENSOR_I2C_WRITE(0x3086, 0x719F);
	    SENSOR_I2C_WRITE(0x3086, 0x6343);
	    SENSOR_I2C_WRITE(0x3086, 0x166F);
	    SENSOR_I2C_WRITE(0x3086, 0x9F92);
	    SENSOR_I2C_WRITE(0x3086, 0x1244);
	    SENSOR_I2C_WRITE(0x3086, 0x1663);
	    SENSOR_I2C_WRITE(0x3086, 0x4316);
	    SENSOR_I2C_WRITE(0x3086, 0x9326);
	    SENSOR_I2C_WRITE(0x3086, 0x0426);
	    SENSOR_I2C_WRITE(0x3086, 0x848E);
	    SENSOR_I2C_WRITE(0x3086, 0x0327);
	    SENSOR_I2C_WRITE(0x3086, 0xFC5C);
	    SENSOR_I2C_WRITE(0x3086, 0x0D57);
	    SENSOR_I2C_WRITE(0x3086, 0x5417);
	    SENSOR_I2C_WRITE(0x3086, 0x0955);
	    SENSOR_I2C_WRITE(0x3086, 0x5649);
	    SENSOR_I2C_WRITE(0x3086, 0x5F53);
	    SENSOR_I2C_WRITE(0x3086, 0x0553);
	    SENSOR_I2C_WRITE(0x3086, 0x0728);
	    SENSOR_I2C_WRITE(0x3086, 0x6C4C);
	    SENSOR_I2C_WRITE(0x3086, 0x0928);
	    SENSOR_I2C_WRITE(0x3086, 0x2C72);
	    SENSOR_I2C_WRITE(0x3086, 0xAD7C);
	    SENSOR_I2C_WRITE(0x3086, 0xA928);
	    SENSOR_I2C_WRITE(0x3086, 0xA879);
	    SENSOR_I2C_WRITE(0x3086, 0x6026);
	    SENSOR_I2C_WRITE(0x3086, 0x9C5C);
	    SENSOR_I2C_WRITE(0x3086, 0x1B45);
	    SENSOR_I2C_WRITE(0x3086, 0x4845);
	    SENSOR_I2C_WRITE(0x3086, 0x0845);
	    SENSOR_I2C_WRITE(0x3086, 0x8826);
	    SENSOR_I2C_WRITE(0x3086, 0xBE8E);
	    SENSOR_I2C_WRITE(0x3086, 0x0127);
	    SENSOR_I2C_WRITE(0x3086, 0xF817);
	    SENSOR_I2C_WRITE(0x3086, 0x0227);
	    SENSOR_I2C_WRITE(0x3086, 0xFA17);
	    SENSOR_I2C_WRITE(0x3086, 0x095C);
	    SENSOR_I2C_WRITE(0x3086, 0x0B17);
	    SENSOR_I2C_WRITE(0x3086, 0x1026);
	    SENSOR_I2C_WRITE(0x3086, 0xBA5C);
	    SENSOR_I2C_WRITE(0x3086, 0x0317);
	    SENSOR_I2C_WRITE(0x3086, 0x1026);
	    SENSOR_I2C_WRITE(0x3086, 0xB217);
	    SENSOR_I2C_WRITE(0x3086, 0x065F);
	    SENSOR_I2C_WRITE(0x3086, 0x2888);
	    SENSOR_I2C_WRITE(0x3086, 0x9060);
	    SENSOR_I2C_WRITE(0x3086, 0x27F2);
	    SENSOR_I2C_WRITE(0x3086, 0x1710);
	    SENSOR_I2C_WRITE(0x3086, 0x26A2);
	    SENSOR_I2C_WRITE(0x3086, 0x26A3);
	    SENSOR_I2C_WRITE(0x3086, 0x5F4D);
	    SENSOR_I2C_WRITE(0x3086, 0x2808);
	    SENSOR_I2C_WRITE(0x3086, 0x1927);
	    SENSOR_I2C_WRITE(0x3086, 0xFA84);
	    SENSOR_I2C_WRITE(0x3086, 0x69A0);
	    SENSOR_I2C_WRITE(0x3086, 0x785D);
	    SENSOR_I2C_WRITE(0x3086, 0x2888);
	    SENSOR_I2C_WRITE(0x3086, 0x8710);
	    SENSOR_I2C_WRITE(0x3086, 0x8C82);
	    SENSOR_I2C_WRITE(0x3086, 0x8926);
	    SENSOR_I2C_WRITE(0x3086, 0xB217);
	    SENSOR_I2C_WRITE(0x3086, 0x036B);
	    SENSOR_I2C_WRITE(0x3086, 0x9C60);
	    SENSOR_I2C_WRITE(0x3086, 0x9417);
	    SENSOR_I2C_WRITE(0x3086, 0x2926);
	    SENSOR_I2C_WRITE(0x3086, 0x8345);
	    SENSOR_I2C_WRITE(0x3086, 0xA817);
	    SENSOR_I2C_WRITE(0x3086, 0x0727);
	    SENSOR_I2C_WRITE(0x3086, 0xFB17);
	    SENSOR_I2C_WRITE(0x3086, 0x2945);
	    SENSOR_I2C_WRITE(0x3086, 0x881F);
	    SENSOR_I2C_WRITE(0x3086, 0x1708);
	    SENSOR_I2C_WRITE(0x3086, 0x27FA);
	    SENSOR_I2C_WRITE(0x3086, 0x5D87);
	    SENSOR_I2C_WRITE(0x3086, 0x108C);
	    SENSOR_I2C_WRITE(0x3086, 0x8289);
	    SENSOR_I2C_WRITE(0x3086, 0x170E);
	    SENSOR_I2C_WRITE(0x3086, 0x4826);
	    SENSOR_I2C_WRITE(0x3086, 0x9A28);
	    SENSOR_I2C_WRITE(0x3086, 0x884C);
	    SENSOR_I2C_WRITE(0x3086, 0x0B79);
	    SENSOR_I2C_WRITE(0x3086, 0x1730);
	    SENSOR_I2C_WRITE(0x3086, 0x2692);
	    SENSOR_I2C_WRITE(0x3086, 0x1709);
	    SENSOR_I2C_WRITE(0x3086, 0x9160);
	    SENSOR_I2C_WRITE(0x3086, 0x27F2);
	    SENSOR_I2C_WRITE(0x3086, 0x1710);
	    SENSOR_I2C_WRITE(0x3086, 0x2682);
	    SENSOR_I2C_WRITE(0x3086, 0x2683);
	    SENSOR_I2C_WRITE(0x3086, 0x5F4D);
	    SENSOR_I2C_WRITE(0x3086, 0x2808);
	    SENSOR_I2C_WRITE(0x3086, 0x1927);
	    SENSOR_I2C_WRITE(0x3086, 0xFA84);
	    SENSOR_I2C_WRITE(0x3086, 0x69A1);
	    SENSOR_I2C_WRITE(0x3086, 0x785D);
	    SENSOR_I2C_WRITE(0x3086, 0x2888);
	    SENSOR_I2C_WRITE(0x3086, 0x8710);
	    SENSOR_I2C_WRITE(0x3086, 0x8C80);
	    SENSOR_I2C_WRITE(0x3086, 0x8A26);
	    SENSOR_I2C_WRITE(0x3086, 0x9217);
	    SENSOR_I2C_WRITE(0x3086, 0x036B);
	    SENSOR_I2C_WRITE(0x3086, 0x9D95);
	    SENSOR_I2C_WRITE(0x3086, 0x2603);
	    SENSOR_I2C_WRITE(0x3086, 0x5C01);
	    SENSOR_I2C_WRITE(0x3086, 0x4558);
	    SENSOR_I2C_WRITE(0x3086, 0x8E00);
	    SENSOR_I2C_WRITE(0x3086, 0x2798);
	    SENSOR_I2C_WRITE(0x3086, 0x170A);
	    SENSOR_I2C_WRITE(0x3086, 0x4A65);
	    SENSOR_I2C_WRITE(0x3086, 0x4316);
	    SENSOR_I2C_WRITE(0x3086, 0x6643);
	    SENSOR_I2C_WRITE(0x3086, 0x165B);
	    SENSOR_I2C_WRITE(0x3086, 0x4316);
	    SENSOR_I2C_WRITE(0x3086, 0x5943);
	    SENSOR_I2C_WRITE(0x3086, 0x168E);
	    SENSOR_I2C_WRITE(0x3086, 0x0327);
	    SENSOR_I2C_WRITE(0x3086, 0x9C45);
	    SENSOR_I2C_WRITE(0x3086, 0x7817);
	    SENSOR_I2C_WRITE(0x3086, 0x0727);
	    SENSOR_I2C_WRITE(0x3086, 0x9D17);
	    SENSOR_I2C_WRITE(0x3086, 0x225D);
	    SENSOR_I2C_WRITE(0x3086, 0x8710);
	    SENSOR_I2C_WRITE(0x3086, 0x2808);
	    SENSOR_I2C_WRITE(0x3086, 0x530D);
	    SENSOR_I2C_WRITE(0x3086, 0x8C80);
	    SENSOR_I2C_WRITE(0x3086, 0x8A45);
	    SENSOR_I2C_WRITE(0x3086, 0x5823);
	    SENSOR_I2C_WRITE(0x3086, 0x1708);
	    SENSOR_I2C_WRITE(0x3086, 0x8E01);
	    SENSOR_I2C_WRITE(0x3086, 0x2798);
	    SENSOR_I2C_WRITE(0x3086, 0x8E00);
	    SENSOR_I2C_WRITE(0x3086, 0x2644);
	    SENSOR_I2C_WRITE(0x3086, 0x5C05);
	    SENSOR_I2C_WRITE(0x3086, 0x1244);
	    SENSOR_I2C_WRITE(0x3086, 0x4B71);
	    SENSOR_I2C_WRITE(0x3086, 0x759E);
	    SENSOR_I2C_WRITE(0x3086, 0x8B85);
	    SENSOR_I2C_WRITE(0x3086, 0x0143);
	    SENSOR_I2C_WRITE(0x3086, 0x7271);
	    SENSOR_I2C_WRITE(0x3086, 0xA346);
	    SENSOR_I2C_WRITE(0x3086, 0x4316);
	    SENSOR_I2C_WRITE(0x3086, 0x6FA3);
	    SENSOR_I2C_WRITE(0x3086, 0x9612);
	    SENSOR_I2C_WRITE(0x3086, 0x4416);
	    SENSOR_I2C_WRITE(0x3086, 0x4643);
	    SENSOR_I2C_WRITE(0x3086, 0x1697);
	    SENSOR_I2C_WRITE(0x3086, 0x2604);
	    SENSOR_I2C_WRITE(0x3086, 0x2684);
	    SENSOR_I2C_WRITE(0x3086, 0x8E03);
	    SENSOR_I2C_WRITE(0x3086, 0x27FC);
	    SENSOR_I2C_WRITE(0x3086, 0x5C0D);
	    SENSOR_I2C_WRITE(0x3086, 0x5754);
	    SENSOR_I2C_WRITE(0x3086, 0x1709);
	    SENSOR_I2C_WRITE(0x3086, 0x5556);
	    SENSOR_I2C_WRITE(0x3086, 0x495F);
	    SENSOR_I2C_WRITE(0x3086, 0x5305);
	    SENSOR_I2C_WRITE(0x3086, 0x5307);
	    SENSOR_I2C_WRITE(0x3086, 0x286C);
	    SENSOR_I2C_WRITE(0x3086, 0x4C09);
	    SENSOR_I2C_WRITE(0x3086, 0x282C);
	    SENSOR_I2C_WRITE(0x3086, 0x72AE);
	    SENSOR_I2C_WRITE(0x3086, 0x7CAA);
	    SENSOR_I2C_WRITE(0x3086, 0x28A8);
	    SENSOR_I2C_WRITE(0x3086, 0x7960);
	    SENSOR_I2C_WRITE(0x3086, 0x269C);
	    SENSOR_I2C_WRITE(0x3086, 0x5C1B);
	    SENSOR_I2C_WRITE(0x3086, 0x4548);
	    SENSOR_I2C_WRITE(0x3086, 0x4508);
	    SENSOR_I2C_WRITE(0x3086, 0x4588);
	    SENSOR_I2C_WRITE(0x3086, 0x26BE);
	    SENSOR_I2C_WRITE(0x3086, 0x8E01);
	    SENSOR_I2C_WRITE(0x3086, 0x27F8);
	    SENSOR_I2C_WRITE(0x3086, 0x1702);
	    SENSOR_I2C_WRITE(0x3086, 0x27FA);
	    SENSOR_I2C_WRITE(0x3086, 0x1709);
	    SENSOR_I2C_WRITE(0x3086, 0x5C0B);
	    SENSOR_I2C_WRITE(0x3086, 0x1710);
	    SENSOR_I2C_WRITE(0x3086, 0x26BA);
	    SENSOR_I2C_WRITE(0x3086, 0x5C03);
	    SENSOR_I2C_WRITE(0x3086, 0x1710);
	    SENSOR_I2C_WRITE(0x3086, 0x26B2);
	    SENSOR_I2C_WRITE(0x3086, 0x1706);
	    SENSOR_I2C_WRITE(0x3086, 0x5F28);
	    SENSOR_I2C_WRITE(0x3086, 0x8898);
	    SENSOR_I2C_WRITE(0x3086, 0x6027);
	    SENSOR_I2C_WRITE(0x3086, 0xF217);
	    SENSOR_I2C_WRITE(0x3086, 0x1026);
	    SENSOR_I2C_WRITE(0x3086, 0xA226);
	    SENSOR_I2C_WRITE(0x3086, 0xA35F);
	    SENSOR_I2C_WRITE(0x3086, 0x4D28);
	    SENSOR_I2C_WRITE(0x3086, 0x081A);
	    SENSOR_I2C_WRITE(0x3086, 0x27FA);
	    SENSOR_I2C_WRITE(0x3086, 0x8469);
	    SENSOR_I2C_WRITE(0x3086, 0xA578);
	    SENSOR_I2C_WRITE(0x3086, 0x5D28);
	    SENSOR_I2C_WRITE(0x3086, 0x8887);
	    SENSOR_I2C_WRITE(0x3086, 0x108C);
	    SENSOR_I2C_WRITE(0x3086, 0x8289);
	    SENSOR_I2C_WRITE(0x3086, 0x26B2);
	    SENSOR_I2C_WRITE(0x3086, 0x1703);
	    SENSOR_I2C_WRITE(0x3086, 0x6BA4);
	    SENSOR_I2C_WRITE(0x3086, 0x6099);
	    SENSOR_I2C_WRITE(0x3086, 0x1729);
	    SENSOR_I2C_WRITE(0x3086, 0x2683);
	    SENSOR_I2C_WRITE(0x3086, 0x45A8);
	    SENSOR_I2C_WRITE(0x3086, 0x1707);
	    SENSOR_I2C_WRITE(0x3086, 0x27FB);
	    SENSOR_I2C_WRITE(0x3086, 0x1729);
	    SENSOR_I2C_WRITE(0x3086, 0x4588);
	    SENSOR_I2C_WRITE(0x3086, 0x2017);
	    SENSOR_I2C_WRITE(0x3086, 0x0827);
	    SENSOR_I2C_WRITE(0x3086, 0xFA5D);
	    SENSOR_I2C_WRITE(0x3086, 0x8710);
	    SENSOR_I2C_WRITE(0x3086, 0x8C82);
	    SENSOR_I2C_WRITE(0x3086, 0x8917);
	    SENSOR_I2C_WRITE(0x3086, 0x0E48);
	    SENSOR_I2C_WRITE(0x3086, 0x269A);
	    SENSOR_I2C_WRITE(0x3086, 0x2888);
	    SENSOR_I2C_WRITE(0x3086, 0x4C0B);
	    SENSOR_I2C_WRITE(0x3086, 0x7917);
	    SENSOR_I2C_WRITE(0x3086, 0x3026);
	    SENSOR_I2C_WRITE(0x3086, 0x9217);
	    SENSOR_I2C_WRITE(0x3086, 0x099A);
	    SENSOR_I2C_WRITE(0x3086, 0x6027);
	    SENSOR_I2C_WRITE(0x3086, 0xF217);
	    SENSOR_I2C_WRITE(0x3086, 0x1026);
	    SENSOR_I2C_WRITE(0x3086, 0x8226);
	    SENSOR_I2C_WRITE(0x3086, 0x835F);
	    SENSOR_I2C_WRITE(0x3086, 0x4D28);
	    SENSOR_I2C_WRITE(0x3086, 0x081A);
	    SENSOR_I2C_WRITE(0x3086, 0x27FA);
	    SENSOR_I2C_WRITE(0x3086, 0x8469);
	    SENSOR_I2C_WRITE(0x3086, 0xAB78);
	    SENSOR_I2C_WRITE(0x3086, 0x5D28);
	    SENSOR_I2C_WRITE(0x3086, 0x8887);
	    SENSOR_I2C_WRITE(0x3086, 0x108C);
	    SENSOR_I2C_WRITE(0x3086, 0x808A);
	    SENSOR_I2C_WRITE(0x3086, 0x2692);
	    SENSOR_I2C_WRITE(0x3086, 0x1703);
	    SENSOR_I2C_WRITE(0x3086, 0x6BA6);
	    SENSOR_I2C_WRITE(0x3086, 0xA726);
	    SENSOR_I2C_WRITE(0x3086, 0x035C);
	    SENSOR_I2C_WRITE(0x3086, 0x0145);
	    SENSOR_I2C_WRITE(0x3086, 0x588E);
	    SENSOR_I2C_WRITE(0x3086, 0x0027);
	    SENSOR_I2C_WRITE(0x3086, 0x9817);
	    SENSOR_I2C_WRITE(0x3086, 0x0A4A);
	    SENSOR_I2C_WRITE(0x3086, 0x0A43);
	    SENSOR_I2C_WRITE(0x3086, 0x160B);
	    SENSOR_I2C_WRITE(0x3086, 0x438E);
	    SENSOR_I2C_WRITE(0x3086, 0x0327);
	    SENSOR_I2C_WRITE(0x3086, 0x9C45);
	    SENSOR_I2C_WRITE(0x3086, 0x7817);
	    SENSOR_I2C_WRITE(0x3086, 0x0727);
	    SENSOR_I2C_WRITE(0x3086, 0x9D17);
	    SENSOR_I2C_WRITE(0x3086, 0x225D);
	    SENSOR_I2C_WRITE(0x3086, 0x8710);
	    SENSOR_I2C_WRITE(0x3086, 0x2808);
	    SENSOR_I2C_WRITE(0x3086, 0x530D);
	    SENSOR_I2C_WRITE(0x3086, 0x8C80);
	    SENSOR_I2C_WRITE(0x3086, 0x8A45);
	    SENSOR_I2C_WRITE(0x3086, 0x5817);
	    SENSOR_I2C_WRITE(0x3086, 0x088E);
	    SENSOR_I2C_WRITE(0x3086, 0x0127);
	    SENSOR_I2C_WRITE(0x3086, 0x988E);
	    SENSOR_I2C_WRITE(0x3086, 0x0076);
	    SENSOR_I2C_WRITE(0x3086, 0xAC77);
	    SENSOR_I2C_WRITE(0x3086, 0xAC46);
	    SENSOR_I2C_WRITE(0x3086, 0x4416);
	    SENSOR_I2C_WRITE(0x3086, 0x16A8);
	    SENSOR_I2C_WRITE(0x3086, 0x7A26);
	    SENSOR_I2C_WRITE(0x3086, 0x445C);
	    SENSOR_I2C_WRITE(0x3086, 0x0512);
	    SENSOR_I2C_WRITE(0x3086, 0x444B);
	    SENSOR_I2C_WRITE(0x3086, 0x7175);
	    SENSOR_I2C_WRITE(0x3086, 0xA24A);
	    SENSOR_I2C_WRITE(0x3086, 0x0343);
	    SENSOR_I2C_WRITE(0x3086, 0x1604);
	    SENSOR_I2C_WRITE(0x3086, 0x4316);
	    SENSOR_I2C_WRITE(0x3086, 0x5843);
	    SENSOR_I2C_WRITE(0x3086, 0x165A);
	    SENSOR_I2C_WRITE(0x3086, 0x4316);
	    SENSOR_I2C_WRITE(0x3086, 0x0643);
	    SENSOR_I2C_WRITE(0x3086, 0x1607);
	    SENSOR_I2C_WRITE(0x3086, 0x4316);
	    SENSOR_I2C_WRITE(0x3086, 0x8E03);
	    SENSOR_I2C_WRITE(0x3086, 0x279C);
	    SENSOR_I2C_WRITE(0x3086, 0x4578);
	    SENSOR_I2C_WRITE(0x3086, 0x7B17);
	    SENSOR_I2C_WRITE(0x3086, 0x078B);
	    SENSOR_I2C_WRITE(0x3086, 0x8627);
	    SENSOR_I2C_WRITE(0x3086, 0x9D17);
	    SENSOR_I2C_WRITE(0x3086, 0x2345);
	    SENSOR_I2C_WRITE(0x3086, 0x5822);
	    SENSOR_I2C_WRITE(0x3086, 0x1708);
	    SENSOR_I2C_WRITE(0x3086, 0x8E01);
	    SENSOR_I2C_WRITE(0x3086, 0x2798);
	    SENSOR_I2C_WRITE(0x3086, 0x8E00);
	    SENSOR_I2C_WRITE(0x3086, 0x2644);
	    SENSOR_I2C_WRITE(0x3086, 0x5C05);
	    SENSOR_I2C_WRITE(0x3086, 0x1244);
	    SENSOR_I2C_WRITE(0x3086, 0x4B8D);
	    SENSOR_I2C_WRITE(0x3086, 0x602C);
	    SENSOR_I2C_WRITE(0x3086, 0x2C2C);
	    SENSOR_I2C_WRITE(0x3086, 0x2C00);
		//LOAD= AR0140 Rev3 Optimized settings
	//	sensor_init_optimized_settings();
		
		SENSOR_I2C_WRITE(0x3044, 0x0400); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3052, 0xA134); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3092, 0x010F); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x30FE, 0x0080); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3ECE, 0x40FF); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3ED0, 0xFF40); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3ED2, 0xA906); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3ED4, 0x001F); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3ED6, 0x638F); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3ED8, 0xCC99); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EDA, 0x0888); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EDE, 0x8878); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EE0, 0x7744); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EE2, 0x4463); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EE4, 0xAAE0); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EE6, 0x1400); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EEA, 0xA4FF); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EEC, 0x80F0); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x3EEE, 0x0000); //Manufacturer-specific
		SENSOR_I2C_WRITE(0x31E0, 0x1701); //Manufacturer-specific


		ar0141_delay_ms(100);
	
		//PLL_configuration_Parallel
		SENSOR_I2C_WRITE( 0x302A, 0x0006); // VT_PIX_CLK_DIV
		SENSOR_I2C_WRITE( 0x302C, 0x0001); // VT_SYS_CLK_DIV
		SENSOR_I2C_WRITE( 0x302E, 0x0004); 	// PRE_PLL_CLK_DIV
		SENSOR_I2C_WRITE( 0x3030, 0x0042); 	// PLL_MULTIPLIER
		SENSOR_I2C_WRITE( 0x3036, 0x000C); 	// OP_PIX_CLK_DIV
		SENSOR_I2C_WRITE( 0x3038, 0x0001); 	// OP_SYS_CLK_DIV
	
		//720P30fps_configuration
		SENSOR_I2C_WRITE( 0x3004, 0x0012); 	// X_ADDR_START
		SENSOR_I2C_WRITE( 0x3002, 0x0040); 	// Y_ADDR_START
		SENSOR_I2C_WRITE( 0x3008, 0x0519); 	// X_ADDR_END
		SENSOR_I2C_WRITE( 0x3006, 0x0317); 	// Y_ADDR_END
		SENSOR_I2C_WRITE( 0x300A, 0x05D4); 	// FRAME_LENGTH_LINES
		SENSOR_I2C_WRITE( 0x300C, 0x0672); 	// LINE_LENGTH_PCK
		SENSOR_I2C_WRITE( 0x3012, 0x002D); 	// COARSE_INTEGRATION_TIME
		SENSOR_I2C_WRITE( 0x30A2, 0x0001); 	// X_ODD_INC
		SENSOR_I2C_WRITE( 0x30A6, 0x0001); 	// Y_ODD_INC
		SENSOR_I2C_WRITE( 0x3040, 0x0000); 	// READ_MODE
	
		//Companding_enabled_16to12
		SENSOR_I2C_WRITE( 0x31AC, 0x100C); 	// DATA_FORMAT_BITS
		SENSOR_I2C_WRITE( 0x31D0, 0x0001); 	// COMPANDING
	
		//HDR Mode 16x Setup
		SENSOR_I2C_WRITE( 0x305E, 0x0080); 	// global_gain
		SENSOR_I2C_WRITE( 0x3082, 0x0008);    //HDR mode
		SENSOR_I2C_WRITE( 0x318C, 0xC000); 	// Motion Compensation On
		SENSOR_I2C_WRITE( 0x320A, 0x0080); 	// ADACD_PEDESTAL
		SENSOR_I2C_WRITE( 0x3206, 0x0A06); 	// ADACD_NOISE_FLOOR1
		SENSOR_I2C_WRITE( 0x3206, 0x0A06); 	// ADACD_NOISE_FLOOR1
		SENSOR_I2C_WRITE( 0x3208, 0x1A12); 	// ADACD_NOISE_FLOOR2
		SENSOR_I2C_WRITE( 0x3208, 0x1A12); 	// ADACD_NOISE_FLOOR2
		SENSOR_I2C_WRITE( 0x3202, 0x00A0); 	// ADACD_NOISE_MODEL1
		SENSOR_I2C_WRITE( 0x3200, 0x0002); 	// ADACD_CONTROL
		SENSOR_I2C_WRITE( 0x31AC, 0x100C); 	// DATA_FORMAT_BITS
		SENSOR_I2C_WRITE( 0x31D0, 0x0001); 	// COMPANDING
		SENSOR_I2C_WRITE( 0x318A, 0x0E74); 	// HDR_MC_CTRL1
		SENSOR_I2C_WRITE( 0x3192, 0x0400); 	// HDR_MC_CTRL5
		SENSOR_I2C_WRITE(0x3198, 0x183C);	 //Motion detect Q1 set to 60, Q2 set to 24
		SENSOR_I2C_WRITE(0x318E, 0x0800);	 //Gain before DLO set to 1
		SENSOR_I2C_WRITE(0x3194, 0x0BB8);	 //T1 barrier set to 3000
		SENSOR_I2C_WRITE(0x3196, 0x0E74);	//T2 barrier set to 3700
	
		SENSOR_I2C_WRITE( 0x30B0, 0x0000); //DIGITAL_TEST
		SENSOR_I2C_WRITE( 0x30BA, 0x012C); //DIGITAL_CTRL
		SENSOR_I2C_WRITE( 0x31AE, 0x0301); //SERAIL_FORAMT: parallel infterface
	
		SENSOR_I2C_WRITE( 0x306E, 0xE810); //
		
		SENSOR_I2C_WRITE( 0x3064, 0x1882); //
		
		
		SENSOR_I2C_WRITE( 0x301A, 0x10DC); //Start streaming
		printf(">>>>>>>>>>>>>>>>>init wdr success!\n");
	

	
}








/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
HI_S32 ar0141_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

    pstSensorExpFunc->pfn_cmos_sensor_init = ar0141_reg_init;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = ar0141_global_init;
    pstSensorExpFunc->pfn_cmos_get_isp_default = ar0141_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = ar0141_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = ar0141_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = ar0141_set_wdr_mode;
    pstSensorExpFunc->pfn_cmos_get_sensor_max_resolution = ar0141_get_sensor_max_resolution;
    
    return 0;
}

HI_S32 ar0141_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = ar0141_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = ar0141_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= ar0141_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = ar0141_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = ar0141_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = ar0141_again_calc_table;
    

    return 0;
}

HI_S32 ar0141_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = ar0141_get_awb_default;

    return 0;
}

int ar0141_sensor_register_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    ar0141_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(AR0141_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    ar0141_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(&stLib, AR0141_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    ar0141_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(&stLib, AR0141_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int ar0141_sensor_unregister_callback(void)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(AR0141_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(&stLib, AR0141_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(&stLib, AR0141_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}



void APTINA_AR0141_init(SENSOR_APTINA_AR0141_DO_I2CRD do_i2c_read,SENSOR_APTINA_AR0141_DO_I2CWR do_i2c_write)
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

	ar0141_reg_init();
//	ar0141_sensor_init_wdr();

	ar0141_sensor_register_callback();

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
	printf("Aptina AR0141 sensor 720P30fps init success!\n");
	
	return 0;


}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // __AR0141_CMOS_H_





















































