#include "sdk/sdk_debug.h"
#include "hi3518e.h"
#include "hi3518e_isp_sensor.h"
#include "hi_isp_i2c.h"
#include "sdk/sdk_sys.h"


#define SENSOR_NAME "sc1235"
static SENSOR_DO_I2CRD sensor_i2c_read = NULL;
static SENSOR_DO_I2CWR sensor_i2c_write = NULL;


#define SENSOR_I2C_READ(_add, _ret_data) \
	(sensor_i2c_read ? sensor_i2c_read((_add), (_ret_data)) : _i2c_read((_add), (_ret_data), 0x60, 2, 1))

#define SENSOR_I2C_WRITE(_add, _data) \
	(sensor_i2c_write ? sensor_i2c_write((_add), (_data)) : _i2c_write((_add), (_data), 0x60, 2, 1))


#define SENSOR_SC1235_WIDTH (1280)
#define SENSOR_SC1235_HEIGHT  (960)
#define SC1235_CHECK_DATA_LSB	(0x12)
#define SC1235_CHECK_DATA_MSB	(0x35)//CHIP_ID address is 0x3107&0x3108


#define SENSOR_DELAY_MS(_ms) do{ usleep(1024 * (_ms)); } while(0)

#if !defined(__SC1235_CMOS_H_)
#define __SC1235_CMOS_H_

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


#define SC1235_ID 1235

#define CMOS_SC1235_ISP_WRITE_SENSOR_ENABLE (1)
#define FULL_LINES_MAX  (0xFFFF)


/* To change the mode of config. ifndef INIFILE_CONFIG_MODE, quick config mode.*/
/* else, cmos_cfg.ini file config mode*/
#ifdef INIFILE_CONFIG_MODE

static AE_SENSOR_DEFAULT_S  g_AeDft[];
static AWB_SENSOR_DEFAULT_S g_AwbDft[];
static ISP_CMOS_DEFAULT_S   g_IspDft[];
static HI_S32 Cmos_LoadINIPara(const HI_CHAR *pcName);
#else

#endif

/****************************************************************************
 * local variables                                                            *
 ****************************************************************************/

static const unsigned int sensor_i2c_addr=0x60;
static unsigned int sensor_addr_byte=2;
static unsigned int sensor_data_byte=1;

#define VMAX_ADDR_H              (0x320e)
#define VMAX_ADDR_L              (0x320f)

#define SENSOR_960P_30FPS_MODE  (1)

#define INCREASE_LINES (0) /* make real fps less than stand fps because NVR require*/
#define VMAX_960P30_LINEAR     (1000+INCREASE_LINES)
#define CMOS_SC1235_SLOW_FRAMERATE_MODE (0)


static HI_U8 gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
static WDR_MODE_E genSensorMode = WDR_MODE_NONE;

static HI_U32 gu32FullLinesStd = VMAX_960P30_LINEAR;
static HI_U32 gu32FullLines = VMAX_960P30_LINEAR;

static HI_BOOL bInit = HI_FALSE;
static HI_BOOL bSensorInit = HI_FALSE;
static ISP_SNS_REGS_INFO_S g_stSnsRegsInfo = {0};
static ISP_SNS_REGS_INFO_S g_stPreSnsRegsInfo = {0};
static HI_U8 gu8Fps = 25;


/* AE default parameter and function */
static HI_S32 sc1235_get_ae_default(AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    if (HI_NULL == pstAeSnsDft)
    {
        printf("null pointer when get ae default value!\n");
        return -1;
    }

    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*25/2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
    pstAeSnsDft->u32FlickerFreq = 0;
	pstAeSnsDft->u32FullLinesMax = FULL_LINES_MAX;
	pstAeSnsDft->u8AERunInterval  = 3;
    pstAeSnsDft->u32InitExposure  = 10;


    pstAeSnsDft->au8HistThresh[0] = 0xd;
    pstAeSnsDft->au8HistThresh[1] = 0x28;
    pstAeSnsDft->au8HistThresh[2] = 0x60;
    pstAeSnsDft->au8HistThresh[3] = 0x80;
            
    pstAeSnsDft->u8AeCompensation = 0x2b;

    pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
    pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
    pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
    pstAeSnsDft->u32MinIntTime = 1;
    pstAeSnsDft->u32MaxIntTimeTarget =65535;
    pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;

    pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stAgainAccu.f32Accuracy = 0.0625; 
    pstAeSnsDft->u32MaxAgain = 992;  //62±¶ 
    pstAeSnsDft->u32MinAgain = 16;
    pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
    pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

    pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_LINEAR;
    pstAeSnsDft->stDgainAccu.f32Accuracy = 0.0625;//invalidate
    pstAeSnsDft->u32MaxDgain = 16;  
    pstAeSnsDft->u32MinDgain = 16;
    pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
    pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain; 
	
    pstAeSnsDft->u32ISPDgainShift = 8;
    pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
    pstAeSnsDft->u32MaxISPDgainTarget = 4 << pstAeSnsDft->u32ISPDgainShift; 
	  
//    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd*30/2;

    return 0;
}

/* the function of sensor set fps */
static HI_VOID sc1235_fps_set(HI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
    HI_U32 u32VblankingLines = 0xFFFF;

    if ((f32Fps <= 30) && (f32Fps >= 0.5))
    {
        if(SENSOR_960P_30FPS_MODE == gu8SensorImageMode)
        {
            u32VblankingLines = VMAX_960P30_LINEAR * 25 / f32Fps;
        }
    }
    else
    {
        printf("Not support Fps: %f\n", f32Fps);
        return;
    }

   gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;

//      gu32FullLinesStd = VMAX_960P30_LINEAR;

#if CMOS_SC1235_ISP_WRITE_SENSOR_ENABLE
	g_stSnsRegsInfo.astI2cData[4].u32Data = 0x08;
	g_stSnsRegsInfo.astI2cData[5].u32Data = 0x70;
	g_stSnsRegsInfo.astI2cData[6].u32Data = (u32VblankingLines >> 8) & 0xFF ;
	g_stSnsRegsInfo.astI2cData[7].u32Data = u32VblankingLines & 0xFF;
//		g_stSnsRegsInfo.astI2cData[6].u32Data = 0x03;
//		g_stSnsRegsInfo.astI2cData[7].u32Data = 0xe8;
#endif

    pstAeSnsDft->f32Fps = f32Fps;
    pstAeSnsDft->u32MaxIntTime = u32VblankingLines - 4;
    gu32FullLinesStd = u32VblankingLines;
    gu8Fps = f32Fps;
    pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * gu8Fps / 2;
    pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;

/**
    if (f32Fps == 30)
    {
		
		gu32FullLinesStd = VMAX_960P30_LINEAR;
		
#if 1
		g_stSnsRegsInfo.astI2cData[4].u32Data = 0x07;
		g_stSnsRegsInfo.astI2cData[5].u32Data = 0x08;
		g_stSnsRegsInfo.astI2cData[6].u32Data = 0x03;
		g_stSnsRegsInfo.astI2cData[7].u32Data = 0xe8;
#else	
		g_stSnsRegsInfo.astI2cData[8].u32Data = 0x06;
		g_stSnsRegsInfo.astI2cData[9].u32Data = 0xd8;
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x01;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x00;
	//	g_stSnsRegsInfo.astI2cData[12].u32Data = 0x03;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0xe8;
#endif
		pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
		pstAeSnsDft->f32Fps = f32Fps;
		pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 25 / 2;
		pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
		gu32FullLines = gu32FullLinesStd;
		pstAeSnsDft->u32FullLines = gu32FullLines;

		u32VblankingLines = VMAX_960P30_LINEAR * 30 / f32Fps;

	}
	else if(f32Fps == 25)/**/
//	{
	//	gu32FullLinesStd = VMAX_960P30_LINEAR;
		
#if 0
		g_stSnsRegsInfo.astI2cData[4].u32Data = 0x08;
		g_stSnsRegsInfo.astI2cData[5].u32Data = 0x70;
		g_stSnsRegsInfo.astI2cData[6].u32Data = 0x03;
		g_stSnsRegsInfo.astI2cData[7].u32Data = 0xe8;
#endif

#if 0
		g_stSnsRegsInfo.astI2cData[8].u32Data = 0x08;
		g_stSnsRegsInfo.astI2cData[9].u32Data = 0x00;
		g_stSnsRegsInfo.astI2cData[10].u32Data = 0x01;
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x00;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x03;
		g_stSnsRegsInfo.astI2cData[13].u32Data = 0xe8;
#endif	
/**		pstAeSnsDft->u32MaxIntTime = gu32FullLinesStd - 4;
		pstAeSnsDft->f32Fps = f32Fps;
		pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * 25 / 2;
		pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
		gu32FullLines = gu32FullLinesStd;
		pstAeSnsDft->u32FullLines = gu32FullLines;
		
    }
    else
    {
        printf("Not support Fps: %f\n", f32Fps);
        return;
    }

	gu32FullLinesStd = gu32FullLinesStd > FULL_LINES_MAX ? FULL_LINES_MAX : gu32FullLinesStd;

	g_stSnsRegsInfo.astI2cData[4].u32Data = (u32VblankingLines >> 8) & 0xFF ;
	g_stSnsRegsInfo.astI2cData[5].u32Data = u32VblankingLines & 0xFF;

	
	
	gu8Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = gu32FullLinesStd * gu8Fps / 2;
	pstAeSnsDft->u32FullLinesStd = gu32FullLinesStd;
/**/

    return;
}


static HI_VOID sc1235_slow_framerate_set(HI_U32 u32FullLines,
    AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{

return;
    u32FullLines = (u32FullLines > 0xFFFF) ? 0xFFFF : u32FullLines;
    gu32FullLines = u32FullLines;

    u32FullLines = (u32FullLines > FULL_LINES_MAX) ? FULL_LINES_MAX : u32FullLines;

#if CMOS_SC1235_ISP_WRITE_SENSOR_ENABLE
	g_stSnsRegsInfo.astI2cData[4].u32Data = (u32FullLines >> 8) & 0xFF;
	g_stSnsRegsInfo.astI2cData[5].u32Data = u32FullLines & 0xFf;
#else
	SENSOR_I2C_WRITE(0x320e, ((u32FullLines >> 8) & 0xFF));
	SENSOR_I2C_WRITE(0x320f, u32FullLines & 0xFf);
#endif

    pstAeSnsDft->u32MaxIntTime = gu32FullLines - 4;
	
#if 1
	HI_U16 u16Temp = u32FullLines - 0x2e8;
	SENSOR_I2C_WRITE(0x3336, (u16Temp>>8)&0xff);
	SENSOR_I2C_WRITE(0x3337, u16Temp&0xff);

	SENSOR_I2C_WRITE(0x3338, (u32FullLines>>8)&0xff);
	SENSOR_I2C_WRITE(0x3339, u32FullLines&0xff);

	HI_U16 u16RegH = 0, u16RegL = 0;
	SENSOR_I2C_READ(0x320c,&u16RegH);
	SENSOR_I2C_READ(0x320d,&u16RegL);

	u16Temp = (u16RegH<<8) | u16RegL;
	u16Temp -= 0x30;
	
	SENSOR_I2C_WRITE(0x3320, (u16Temp>>8)&0xff);
	SENSOR_I2C_WRITE(0x3321, u16Temp&0xff);
	
#endif

    return;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static HI_VOID sc1235_inttime_update(HI_U32 u32IntTime)
{

	/*优化麻点问题*/
    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 4) & 0xFF;//bit[11:4]
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime<<4) & 0xF0;  //bit[3:0]

    g_stSnsRegsInfo.astI2cData[8].u32Data =  0x84;
    g_stSnsRegsInfo.astI2cData[9].u32Data =  0x04;

#if 0
if (u32OldIntTime != u32IntTime)
{
	printf("u32IntTime:%#x...........%#x........%#x..\n",u32IntTime, (u32IntTime >> 4) & 0xFF,(u32IntTime<<4) & 0xF0);
	u32OldIntTime = u32IntTime;
}
#endif
/**
#if CMOS_SC1235_ISP_WRITE_SENSOR_ENABLE
    g_stSnsRegsInfo.astI2cData[0].u32Data = (u32IntTime >> 4) & 0xFF; 
    g_stSnsRegsInfo.astI2cData[1].u32Data = (u32IntTime<<4) & 0xF0;
#else
	SENSOR_I2C_WRITE(0x3e01, ((u32IntTime >> 4) & 0xFF));
	SENSOR_I2C_WRITE(0x3e02, (u32IntTime<<4) & 0xF0);
#endif

#if 1 // solve the problem of  outdoor picture turning red  -20161010
		HI_U32 exp_line = u32IntTime;
		if(exp_line < 50){
			SENSOR_I2C_WRITE(0x3307,0x13);
		}
		else{
			SENSOR_I2C_WRITE(0x3307,0x03);
		}
#endif 
/**/
    return;
}



static  HI_U32   analog_gain_table[64] = 
{ 

    1024,	1085,	1146,	1208,	1269,	1321,	1382,	1444,	1505,	1566,	1628,	1690,	1751,	1802,	1864,	1925,                       
    2048,	2170,	2292,	2416,	2538,	2642,	2764,	2888,	3010,	3132,	3256,	3380,	3502,	3604,	3728,	3850,                       
    4096,	4340,	4584,	4832,	5076,	5284,	5528,	5776,	6020,	6264,	6512,	6760,	7004,	7208,	7456,	7700,                       
    8192,	8680,	9168,	9664,	10152,	10568,	11056,	11552,	12040,	12528,	13024,	13520,	14008,	14416,	14912,	15400
};

static  HI_U32   digital_gain_table[3] = 
{ 
    1024,2048,4096
};



static HI_VOID sc1235_again_calc_table(HI_U32 *pu32AgainLin, HI_U32 *pu32AgainDb)
{
    int i;

    if (*pu32AgainLin >= analog_gain_table[63])
    {
         *pu32AgainLin = analog_gain_table[63];
         *pu32AgainDb = 63;
         return ;
    }
    
    for (i = 1; i < 64; i++)
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

static HI_VOID sc1235_dgain_calc_table(HI_U32 *pu32DgainLin, HI_U32 *pu32DgainDb)
{
    int i;

    if (*pu32DgainLin >= digital_gain_table[2])
    {
         *pu32DgainLin = digital_gain_table[2];
         *pu32DgainDb = 2;
         return ;
    }
    
    for (i = 1; i < 3; i++)
    {
        if (*pu32DgainLin < digital_gain_table[i])
        {
            *pu32DgainLin = digital_gain_table[i - 1];
            *pu32DgainDb = i - 1;
            break;
        }
    }

    return;
}

static const HI_U16 sensor_gain_map[48] = {
	0x10, 0x11, 0x12,0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a,0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x30, 0x31, 0x32, 0x33,0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b,0x3c, 0x3d, 0x3e, 0x3f,
	0x70, 0x71, 0x72, 0x73,0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b,0x7c, 0x7d, 0x7e, 0x7f,
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
static HI_U32 sc1235_Again_limit(HI_U32 Again)
{
#define SENSOR_BLC_TOP_VALUE (0x58)
#define SENSOR_BLC_BOT_VALUE (0x45)
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

static HI_VOID sc1235_gains_update(HI_U32 u32Again, HI_U32 u32Dgain)
{
	HI_U32 u32gain = u32Again * u32Dgain;
	
	g_stSnsRegsInfo.astI2cData[2].u32Data =(u32gain >> 12);
    g_stSnsRegsInfo.astI2cData[3].u32Data =(u32gain >> 4) & 0xFF;
	
	
    g_stSnsRegsInfo.astI2cData[8].u32Data =  0x84;
    g_stSnsRegsInfo.astI2cData[9].u32Data =  0x04;
	
	g_stSnsRegsInfo.astI2cData[10].u32Data =  0x00;
    
	if(u32gain < 2*16*16){
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x84;
//		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x0c;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x05;
		g_stSnsRegsInfo.astI2cData[13].u32Data =  0x2f;
	}
	else if(u32gain < 4*16*16){
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x88;
//		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x07;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x1f;
g_stSnsRegsInfo.astI2cData[13].u32Data =  0x23;
	}
	else{
		g_stSnsRegsInfo.astI2cData[11].u32Data = 0x88;
//		g_stSnsRegsInfo.astI2cData[12].u32Data = 0x07;
		g_stSnsRegsInfo.astI2cData[12].u32Data = 0xff;
g_stSnsRegsInfo.astI2cData[13].u32Data =  0x43;
		
	}
	g_stSnsRegsInfo.astI2cData[14].u32Data =  0x30;

	//===============add logic===============
	if(g_stSnsRegsInfo.astI2cData[0].u32Data < 0x05)
	{
		g_stSnsRegsInfo.astI2cData[17].u32Data = 0x12;
	}
	else if(g_stSnsRegsInfo.astI2cData[0].u32Data > 0x0a)
	{
		g_stSnsRegsInfo.astI2cData[17].u32Data = 0x02;
	}

	//-----------dpc---logic-------
	if(u32gain < 10*16*16)
	{
	g_stSnsRegsInfo.astI2cData[15].u32Data =  0x04;
	g_stSnsRegsInfo.astI2cData[16].u32Data =  0x18;
	}
	else
	{
	g_stSnsRegsInfo.astI2cData[15].u32Data =  0x02;
	g_stSnsRegsInfo.astI2cData[16].u32Data =  0x08;
	}

    return;
}


HI_S32 sc1235_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_ae_default    = sc1235_get_ae_default;
    pstExpFuncs->pfn_cmos_fps_set           = sc1235_fps_set;
    pstExpFuncs->pfn_cmos_slow_framerate_set= sc1235_slow_framerate_set;    
    pstExpFuncs->pfn_cmos_inttime_update    = sc1235_inttime_update;
    pstExpFuncs->pfn_cmos_gains_update      = sc1235_gains_update;
    pstExpFuncs->pfn_cmos_again_calc_table  = NULL;//sc1235_again_calc_table;
    pstExpFuncs->pfn_cmos_dgain_calc_table  = NULL;//sc1235_dgain_calc_table;

    return 0;
}


/* AWB default parameter and function */
static AWB_CCM_S g_stAwbCcm =
{  
	4850,	 
	{		
		0x01B6,  0x80B4,  0x8002,
		0x805D,  0x01A2,  0x8045,		
		0x000A,  0x813F,  0x0235	
	},	

	3160,	 
	{		
		0x01B5,  0x8071,  0x8044,		
		0x808A,  0x01B9,  0x802F,		
		0x8003,  0x81B5,  0x02B8	
	},

	2470,	 
	{			 
		0x014F,  0x0091,  0x80E0,		 
		0x8095,  0x01E5,  0x8050,		
		0x803B,  0x81CF,  0x030A	
	} 

};

static AWB_AGC_TABLE_S g_stAwbAgcTable =
{
    /* bvalid */
    1,

	/* saturation */  
    /*1,  2,  4,  8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768*/
	{0x90,0x90,0x80,0x80,0x70,0x70,0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50}
};

static HI_S32 sc1235_get_awb_default(AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
    if (HI_NULL == pstAwbSnsDft)
    {
        printf("null pointer when get awb default value!\n");
        return -1;
    }

    memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

    pstAwbSnsDft->u16WbRefTemp = 4850;
    pstAwbSnsDft->au16GainOffset[0] = 0x18b;    
    pstAwbSnsDft->au16GainOffset[1] = 0x100;    
    pstAwbSnsDft->au16GainOffset[2] = 0x100;    
    pstAwbSnsDft->au16GainOffset[3] = 0x171;    
    pstAwbSnsDft->as32WbPara[0] = 104;    
    pstAwbSnsDft->as32WbPara[1] = 9;    
    pstAwbSnsDft->as32WbPara[2] = -143;    
    pstAwbSnsDft->as32WbPara[3] = 188843;    
    pstAwbSnsDft->as32WbPara[4] = 128;    
    pstAwbSnsDft->as32WbPara[5] = -140071;
    
    memcpy(&pstAwbSnsDft->stCcm, &g_stAwbCcm, sizeof(AWB_CCM_S));
    memcpy(&pstAwbSnsDft->stAgcTbl, &g_stAwbAgcTable, sizeof(AWB_AGC_TABLE_S));
    
    return 0;
}

HI_S32 sc1235_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
    memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

    pstExpFuncs->pfn_cmos_get_awb_default = sc1235_get_awb_default;

    return 0;
}

#define DMNR_CALIB_CARVE_NUM_SC1235 8

float g_coef_calib_sc1235[DMNR_CALIB_CARVE_NUM_SC1235][4] = 
{	  
	{100.000000f, 2.000000f, 0.036803f, 2.165183f, }, 	 
	{200.000000f, 2.301030f, 0.037471f, 2.296438f, },		
	{400.000000f, 2.602060f, 0.039678f, 2.043633f, },	   
	{800.000000f, 2.903090f, 0.042475f, 2.152369f, },	  
	{1592.000000f, 3.201943f, 0.048132f, 1.998313f, },	  
	{3231.000000f, 3.509337f, 0.054542f, 3.439073f, },	  
	{6532.000000f, 3.815046f, 0.068091f, 4.175384f, },	  
	{12477.000000f, 4.096110f, 0.084936f, 5.255723f, },  
};  



static ISP_NR_ISO_PARA_TABLE_S g_stNrIsoParaTab[HI_ISP_NR_ISO_LEVEL_MAX] = 
{
     //u16Threshold//u8varStrength//u8fixStrength//u8LowFreqSlope	
       {1500,       160,             256-256,            0 },  //100    //                      //                                                
       {1500,       120,             256-256,            0 },  //200    // ISO                  // ISO //u8LowFreqSlope
       {1500,       100,             256-256,            0 },  //400    //{400,  1200, 96,256}, //{400 , 0  }
       {1750,       80,              256-256,            8 },  //800    //{800,  1400, 80,256}, //{600 , 2  }
       {1500,       255,             256-256,            6 },  //1600   //{1600, 1200, 72,256}, //{800 , 8  }
       {1500,       255,             256-256,            0 },  //3200   //{3200, 1200, 64,256}, //{1000, 12 }
       {1375,       255,             256-256,            0 },  //6400   //{6400, 1100, 56,256}, //{1600, 6  }
       {1375,       255,             256-256,            0 },  //12800  //{12000,1100, 48,256}, //{2400, 0  }
       {1375,       255,             256-256,            0 },  //25600  //{36000,1100, 48,256}, //
       {1375,       255,             256-256,            0 },  //51200  //{64000,1100, 96,256}, //
       {1250,       255,             256-256,            0 },  //102400 //{82000,1000,240,256}, //
       {1250,       255,             256-256,            0 },  //204800 //                           //
       {1250,       255,             256-256,            0 },  //409600 //                           //
       {1250,       255,             256-256,            0 },  //819200 //                           //
       {1250,       255,             256-256,            0 },  //1638400//                           //
       {1250,       255,             256-256,            0 },  //3276800//                           //
};

static ISP_CMOS_DEMOSAIC_S g_stIspDemosaic =
{
	/*For Demosaic*/
	1, /*bEnable*/			
	50,/*u16VhLimit*/	
	40-24,/*u16VhOffset*/
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
    {0,	  0,   0,  0,   0,   0,   0,    0,    0,    1,    1,     1,     1,     1,     1,       1},/* enPixSel */
    {0xf0, 0x70, 0x68, 0x60, 0x50, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},/*SharpenUD*/ //3.23 wangji
    {0x80, 0x70, 0x68, 0x60, 0x50, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},/*SharpenD*/  //3.23 wangji    
    {0x10, 0x18, 0x20, 0x30, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},/*NoiseThd*/
    {0x18, 0x2f, 0x18, 0x09, 0x04, 0x07, 0x07, 0x07, 0x8c, 0x8c, 0x8c, 0x8c, 0x8c, 0x8c, 0x8c, 0x8c},/*EdgeThd2*/
  	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},/*overshootAmt*/ //3.23 wangji
  	{0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},/*undershootAmt*///3.23 wangji
};

static ISP_CMOS_UVNR_S g_stIspUVNR = 
{
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


static ISP_LSC_CABLI_UNI_TABLE_S g_stCmosLscUniTable = 
{
    /* Mesh Grid Info: Width */
    {9, 13, 18, 26, 36, 50, 70, 98},
    /* Mesh Grid Info: Height */
    {7, 10, 14, 19, 27, 38, 52, 73},
};

static ISP_LSC_CABLI_TABLE_S g_stCmosLscTable[HI_ISP_LSC_LIGHT_NUM] = 
{
    /* Light Source 1 */
    {
		/* AWB RGain */
		256,
		/* AWB BGain */
		256,
		/* Channel R */
		{			  20729753, 20475625, 19803352, 18912098, 17664130, 16293035, 14891347, 13689246, 12930767, 13677134, 15061625, 16442373, 17863714, 19143313, 20010332, 20680624, 20976999, 			 
					  20599909, 20343134, 19660232, 18755427, 17544851, 16172300, 14800093, 13574635, 12832091, 13570074, 14934293, 16332060, 17733098, 18991039, 19860580, 20511904, 20767436,				
					  20208349, 19956673, 19307705, 18420453, 17268656, 15897458, 14532107, 13278929, 12594212, 13306470, 14670926, 16044244, 17446842, 18648007, 19527067, 20176807, 20415443,			   
					  19628823, 19424839, 18817960, 17948773, 16813365, 15488425, 14116657, 12866447, 12234518, 12940049, 14243565, 15616572, 17006071, 18153487, 19054249, 19657858, 19878162,			  
					  19026899, 18831565, 18228314, 17336468, 16302621, 14983217, 13628757, 12414377, 11802526, 12487373, 13785204, 15163644, 16458944, 17589755, 18460140, 19075529, 19338956, 			 
					  18220154, 18033335, 17457950, 16652292, 15559202, 14378170, 13037690, 11812883, 11227499, 11894581, 13202904, 14581651, 15810275, 16867914, 17688120, 18348049, 18636999,				
					  17494945, 17318947, 16741179, 15909061, 14913003, 13719674, 12434556, 11237647, 10622726, 11331487, 12630059, 13978024, 15190527, 16201673, 17023466, 17632860, 17849575,			   
					  16849529, 16650217, 16113673, 15301538, 14308232, 13143444, 11900151, 10682279, 10310739, 10793227, 12118061, 13408429, 14629479, 15623785, 16483734, 17088465, 17317738,			  
					  16620310, 16443411, 15884951, 15109372, 14099769, 12931184, 11696948, 10486282, 10000000, 10626951, 11942135, 13192907, 14393226, 15444710, 16248822, 16807886, 17016404, 			 
					  17302495, 17099403, 16523679, 15739863, 14725875, 13552493, 12247115, 11084615, 10616491, 11196226, 12545140, 13806650, 15006697, 16091126, 16923314, 17543906, 17799991,				
					  18351604, 18148048, 17544201, 16700873, 15636118, 14375234, 13051867, 11848321, 12354654, 11996946, 13301544, 14645437, 15938070, 16997859, 17895624, 18523943, 18754279,			   
					  19401906, 19159507, 18504006, 17599144, 16513779, 15172742, 13831907, 12583443, 12015125, 12726510, 14048514, 15496211, 16845061, 17965734, 18941529, 19664833, 19928045,			  
					  20454641, 20235980, 19530141, 18579702, 17407315, 16050275, 14608885, 13319160, 12728874, 13462519, 14802784, 16321315, 17721154, 18923915, 19968376, 20740234, 21034170, 			 
					  21183165, 20954217, 20252501, 19264651, 18047392, 16662679, 15185125, 13844479, 13253423, 14020006, 15380597, 16944396, 18413226, 19673076, 20704707, 21510025, 21831931,				
					  21800685, 21561304, 20780028, 19794323, 18600068, 17185540, 15679698, 14292137, 13697560, 14408074, 15858163, 17468312, 18963870, 20278596, 21337257, 22139223, 22468170,			   
					  22290030, 22017768, 21185319, 20217917, 19022383, 17547570, 16016128, 14630835, 14015254, 14771556, 16207470, 17869474, 19390856, 20714833, 21824340, 22615258, 22945972,			  
					  22479028, 22232858, 21442913, 20424568, 19187098, 17730382, 16160310, 14775285, 14156783, 14964882, 16399515, 18061157, 19583516, 20947561, 22069069, 22829027, 23144250, 		 
					  }, 		
		/* Channel Gr */
		{			  20796680, 20612029, 19952339, 18996307, 17762892, 16342160, 14927902, 13762776, 12962272, 13683597, 14973160, 16359638, 17781995, 19017821, 19916316, 20549305, 20844928, 			 
		              20661908, 20455017, 19788584, 18853641, 17644227, 16242224, 14806627, 13644336, 12855560, 13562906, 14860790, 16229247, 17649585, 18869366, 19733444, 20377515, 20658124,				
		              20299235, 20075265, 19424216, 18534460, 17334423, 15954364, 14535953, 13351420, 12607292, 13328247, 14596406, 15946149, 17339236, 18518789, 19346833, 19993358, 20268417,			   
		              19701031, 19527421, 18935280, 18047217, 16882649, 15520048, 14170434, 12921510, 12274525, 12964674, 14228201, 15563970, 16917385, 18024960, 18877852, 19482109, 19717121,			  
		              19036890, 18851572, 18318517, 17503063, 16307351, 15042910, 13693342, 12462943, 11866624, 12520869, 13751756, 15151588, 16418681, 17487872, 18289121, 18927075, 19190927, 			 
		              18223885, 18053038, 17530431, 16710645, 15633200, 14431487, 13103160, 11870129, 11296846, 11921027, 13210691, 14559311, 15762995, 16804518, 17576918, 18177853, 18439525,				
		              17535273, 17345724, 16807882, 15996989, 14998392, 13785021, 12478791, 11277204, 10681604, 11400713, 12670352, 14020565, 15168194, 16110563, 16927655, 17562579, 17827009,			   
		              16799138, 16648313, 16148574, 15361379, 14345322, 13179768, 11921535, 10712040, 10359450, 10822581, 12159804, 13437273, 14617555, 15554967, 16446412, 17045186, 17294947,			  
		              16600000, 16431034, 15919475, 15119301, 14103415, 12959827, 11716192, 10507596, 10000000, 10633937, 11952707, 13185357, 14354097, 15420470, 16173390, 16709906, 16922803, 			 
		              17249426, 17085693, 16534328, 15744799, 14713274, 13567752, 12236481, 11074703, 10629435, 11198537, 12554257, 13796070, 14957570, 16058010, 16863848, 17453821, 17722440,				
		              18323758, 18122442, 17524211, 16654882, 15591621, 14353333, 13027033, 11843041, 11348695, 11992220, 13286766, 14581000, 15858035, 16889517, 17728594, 18379827, 18639294,			   
		              19305609, 19103740, 18466382, 17558947, 16450673, 15116156, 13775170, 12574919, 12006416, 12706999, 13991412, 15412890, 16757970, 17841657, 18793714, 19475632, 19721032,			  
		              20355722, 20148117, 19460133, 18492232, 17354010, 15964139, 14550900, 13282609, 12689352, 13439806, 14735740, 16233710, 17620323, 18780869, 19779157, 20526251, 20810140, 			 
		              21052430, 20857414, 20164246, 19195412, 17957110, 16568152, 15091551, 13808600, 13210159, 13934212, 15262588, 16844563, 18293770, 19511790, 20547740, 21325737, 21635371,				
		              21671111, 21447888, 20735275, 19736640, 18483954, 17051302, 15524257, 14196684, 13628660, 14327017, 15706672, 17355684, 18818427, 20097209, 21201222, 21986964, 22301163,			   
		              22111523, 21875510, 21137881, 20119255, 18855806, 17406338, 15840383, 14470343, 13917120, 14628988, 16023380, 17684820, 19189123, 20503193, 21619648, 22438642, 22797299,			  
		              22286035, 22047524, 21290827, 20279949, 19009305, 17565804, 15986396, 14610066, 14040078, 14808862, 16182708, 17828775, 19326693, 20689403, 21846354, 22629824, 22987323, 		 
		              }, 		
		/* Channel Gb */
		{			  20848664, 20601082, 19935743, 18969947, 17814577, 16356322, 14963875, 13784156, 12987988, 13653247, 15005620, 16411670, 17819383, 19098601, 19968659, 20624467, 20871295, 			 
				      20702724, 20456702, 19784054, 18841986, 17662040, 16239407, 14842661, 13643902, 12872006, 13556110, 14888633, 16279228, 17685812, 18924051, 19822729, 20461442, 20708594,				
				      20326292, 20104187, 19442848, 18551123, 17349915, 15952983, 14567089, 13347908, 12630290, 13302716, 14611565, 15992587, 17358083, 18572899, 19455100, 20093337, 20362368,			   
				      19752205, 19560875, 18996375, 18109453, 16907483, 15537144, 14190696, 12900538, 12293299, 12940278, 14203673, 15596425, 16938403, 18115025, 18994000, 19601616, 19811871,			  
				      19091832, 18927919, 18380298, 17542592, 16325979, 15061834, 13696680, 12462971, 11855602, 12519550, 13777755, 15174684, 16435785, 17542024, 18375229, 18974829, 19213251, 			 
				      18311786, 18132065, 17574498, 16750829, 15677036, 14454916, 13122315, 11883966, 11290078, 11920412, 13232928, 14605748, 15854840, 16878664, 17667710, 18257378, 18525422,				
				      17605214, 17436709, 16888497, 16032863, 15026699, 13821683, 12524195, 11287762, 10690757, 11395853, 12688210, 14027153, 15226066, 16211884, 17017153, 17654445, 17888551,			   
				      16871727, 16716431, 16233680, 15413633, 14386301, 13229286, 11945724, 10724996, 10343002, 10838747, 12205346, 13479290, 14668286, 15658678, 16534937, 17160804, 17418194,			  
				      16720147, 16521270, 15968277, 15175472, 14166477, 12989321, 11762858, 10521768, 10000000, 10656556, 11981662, 13221203, 14397400, 15488760, 16270370, 16812530, 17039985, 			 
				      17374324, 17169649, 16564689, 15775768, 14787608, 13596297, 12265881, 11070773, 10624788, 11205856, 12596961, 13822841, 15046228, 16131242, 16962008, 17542105, 17793709,				
				      18358572, 18177196, 17600400, 16705731, 15644035, 14407751, 13049490, 11868293, 11368951, 12012209, 13331741, 14655917, 15934273, 16990999, 17855957, 18534825, 18832689,			   
				      19362648, 19131074, 18498485, 17653214, 16491208, 15152342, 13802443, 12584938, 11995685, 12733094, 14035889, 15480804, 16821691, 17928709, 18865622, 19597064, 19903717,			  
				      20398573, 20194688, 19513032, 18540617, 17347220, 15979431, 14543351, 13271985, 12699858, 13436414, 14759704, 16284394, 17712126, 18881005, 19896104, 20639656, 20948030, 			 
				      21180952, 20926329, 20194635, 19228720, 18014781, 16632525, 15101966, 13804349, 13209016, 13955038, 15294503, 16920818, 18375807, 19613562, 20718819, 21450205, 21701993,				
				      21714900, 21486676, 20770722, 19771899, 18539983, 17057896, 15520031, 14209174, 13616774, 14324113, 15729048, 17401513, 18919667, 20158187, 21314818, 22120684, 22430743,			   
				      22088645, 21861221, 21163423, 20157813, 18901492, 17426107, 15823657, 14488474, 13919998, 14635808, 16071914, 17767692, 19305359, 20600412, 21756764, 22561208, 22854907,			  
				      22260337, 22057020, 21345489, 20364225, 19058870, 17603663, 15976608, 14638064, 14066616, 14802422, 16221302, 17959955, 19470362, 20851927, 21987418, 22806249, 23090950, 		 
				      }, 		
	    /* Channel B */
		{			  20042148, 19779193, 19172969, 18270590, 17225046, 15925687, 14671152, 13596535, 12839040, 13594736, 14775057, 16027015, 17274753, 18316906, 19146706, 19755674, 19924063, 			 
				      19905624, 19638707, 19016726, 18159800, 17108956, 15817874, 14571261, 13480962, 12731174, 13474939, 14658634, 15905785, 17130716, 18169551, 19006956, 19607343, 19769610,				
				      19579325, 19315516, 18694234, 17875851, 16825870, 15581940, 14319935, 13213853, 12509106, 13208632, 14411382, 15638146, 16854233, 17899261, 18712136, 19275864, 19481788,			   
				      19086712, 18842130, 18266741, 17464437, 16381958, 15207200, 13959845, 12821686, 12190508, 12850784, 14060796, 15284058, 16433186, 17519629, 18301804, 18860445, 19117617,			  
				      18555216, 18323317, 17724094, 16948899, 15886569, 14740598, 13492336, 12386503, 11785610, 12435383, 13632274, 14876456, 15999798, 17024503, 17811247, 18364855, 18569745, 			 
				      17813019, 17586310, 16991558, 16220682, 15271818, 14203200, 12944067, 11835228, 11239478, 11855430, 13095824, 14317312, 15429867, 16459116, 17182539, 17705885, 17927016,				
				      17113335, 16885054, 16338500, 15557477, 14653602, 13574716, 12384126, 11281888, 10666091, 11336881, 12574698, 13765031, 14880205, 15885932, 16574247, 17090724, 17320530,			   
				      16433332, 16226231, 15700753, 15026916, 14096263, 12996269, 11845434, 10704070, 10335470, 10811618, 12067871, 13230487, 14407228, 15336181, 16046526, 16610164, 16854233,			  
				      16226231, 16029145, 15517931, 14803791, 13869826, 12797764, 11650731, 10498565, 10000000, 10623009, 11872704, 13056384, 14210438, 15160213, 15879796, 16348319, 16548249, 			 
				      16862581, 16664084, 16084292, 15352601, 14459522, 13360708, 12168161, 11035391, 10626202, 11206023, 12446316, 13673590, 14786750, 15755091, 16541393, 17065811, 17272602,				
				      17835462, 17598189, 16978509, 16189260, 15257332, 14148086, 12923869, 11781331, 12352118, 11957718, 13153872, 14474306, 15580103, 16570832, 17359650, 17930202, 18143947,			   
				      18649502, 18452363, 17868253, 17051024, 16069341, 14868284, 13632341, 12497049, 12000748, 12637268, 13871699, 15245846, 16424772, 17455482, 18268485, 18852564, 19066645,			  
				      19659580, 19436412, 18783645, 17892335, 16888086, 15637264, 14358469, 13162639, 12660622, 13339126, 14604124, 15999890, 17214627, 18315636, 19187222, 19828311, 20043378, 			 
				      20338020, 20092437, 19387588, 18510714, 17466691, 16221203, 14850141, 13649576, 13123946, 13825423, 15084798, 16520562, 17877406, 18979458, 19878670, 20562597, 20795384,				
				      20805910, 20543415, 19870556, 18961429, 17874124, 16626438, 15225851, 14015623, 13489614, 14186063, 15496438, 16976951, 18307781, 19472764, 20407503, 21054650, 21302293,			   
				      21229762, 20933528, 20235085, 19334494, 18158671, 16955010, 15514504, 14249226, 13746925, 14477667, 15794643, 17284923, 18658845, 19828665, 20807236, 21461627, 21765079,			  
				      21468020, 21134999, 20381794, 19542574, 18318901, 17116080, 15671887, 14373568, 13873502, 14638411, 15961509, 17461249, 18838804, 20014246, 21036931, 21681233, 22034720, 		 

		}, 	
	},

	/* Light Source 2 */
	{
		/* AWB RGain */
		256,
		/* AWB BGain */
		256,
		/* Channel R */
		{			  20729753, 20475625, 19803352, 18912098, 17664130, 16293035, 14891347, 13689246, 12930767, 13677134, 15061625, 16442373, 17863714, 19143313, 20010332, 20680624, 20976999, 			 
					  20599909, 20343134, 19660232, 18755427, 17544851, 16172300, 14800093, 13574635, 12832091, 13570074, 14934293, 16332060, 17733098, 18991039, 19860580, 20511904, 20767436,				
					  20208349, 19956673, 19307705, 18420453, 17268656, 15897458, 14532107, 13278929, 12594212, 13306470, 14670926, 16044244, 17446842, 18648007, 19527067, 20176807, 20415443,			   
					  19628823, 19424839, 18817960, 17948773, 16813365, 15488425, 14116657, 12866447, 12234518, 12940049, 14243565, 15616572, 17006071, 18153487, 19054249, 19657858, 19878162,			  
					  19026899, 18831565, 18228314, 17336468, 16302621, 14983217, 13628757, 12414377, 11802526, 12487373, 13785204, 15163644, 16458944, 17589755, 18460140, 19075529, 19338956, 			 
					  18220154, 18033335, 17457950, 16652292, 15559202, 14378170, 13037690, 11812883, 11227499, 11894581, 13202904, 14581651, 15810275, 16867914, 17688120, 18348049, 18636999,				
					  17494945, 17318947, 16741179, 15909061, 14913003, 13719674, 12434556, 11237647, 10622726, 11331487, 12630059, 13978024, 15190527, 16201673, 17023466, 17632860, 17849575,			   
					  16849529, 16650217, 16113673, 15301538, 14308232, 13143444, 11900151, 10682279, 10310739, 10793227, 12118061, 13408429, 14629479, 15623785, 16483734, 17088465, 17317738,			  
					  16620310, 16443411, 15884951, 15109372, 14099769, 12931184, 11696948, 10486282, 10000000, 10626951, 11942135, 13192907, 14393226, 15444710, 16248822, 16807886, 17016404, 			 
					  17302495, 17099403, 16523679, 15739863, 14725875, 13552493, 12247115, 11084615, 10616491, 11196226, 12545140, 13806650, 15006697, 16091126, 16923314, 17543906, 17799991,				
					  18351604, 18148048, 17544201, 16700873, 15636118, 14375234, 13051867, 11848321, 12354654, 11996946, 13301544, 14645437, 15938070, 16997859, 17895624, 18523943, 18754279,			   
					  19401906, 19159507, 18504006, 17599144, 16513779, 15172742, 13831907, 12583443, 12015125, 12726510, 14048514, 15496211, 16845061, 17965734, 18941529, 19664833, 19928045,			  
					  20454641, 20235980, 19530141, 18579702, 17407315, 16050275, 14608885, 13319160, 12728874, 13462519, 14802784, 16321315, 17721154, 18923915, 19968376, 20740234, 21034170, 			 
					  21183165, 20954217, 20252501, 19264651, 18047392, 16662679, 15185125, 13844479, 13253423, 14020006, 15380597, 16944396, 18413226, 19673076, 20704707, 21510025, 21831931,				
					  21800685, 21561304, 20780028, 19794323, 18600068, 17185540, 15679698, 14292137, 13697560, 14408074, 15858163, 17468312, 18963870, 20278596, 21337257, 22139223, 22468170,			   
					  22290030, 22017768, 21185319, 20217917, 19022383, 17547570, 16016128, 14630835, 14015254, 14771556, 16207470, 17869474, 19390856, 20714833, 21824340, 22615258, 22945972,			  
					  22479028, 22232858, 21442913, 20424568, 19187098, 17730382, 16160310, 14775285, 14156783, 14964882, 16399515, 18061157, 19583516, 20947561, 22069069, 22829027, 23144250, 		 
					  }, 		
		/* Channel Gr */
		{			  20796680, 20612029, 19952339, 18996307, 17762892, 16342160, 14927902, 13762776, 12962272, 13683597, 14973160, 16359638, 17781995, 19017821, 19916316, 20549305, 20844928, 			 
		              20661908, 20455017, 19788584, 18853641, 17644227, 16242224, 14806627, 13644336, 12855560, 13562906, 14860790, 16229247, 17649585, 18869366, 19733444, 20377515, 20658124,				
		              20299235, 20075265, 19424216, 18534460, 17334423, 15954364, 14535953, 13351420, 12607292, 13328247, 14596406, 15946149, 17339236, 18518789, 19346833, 19993358, 20268417,			   
		              19701031, 19527421, 18935280, 18047217, 16882649, 15520048, 14170434, 12921510, 12274525, 12964674, 14228201, 15563970, 16917385, 18024960, 18877852, 19482109, 19717121,			  
		              19036890, 18851572, 18318517, 17503063, 16307351, 15042910, 13693342, 12462943, 11866624, 12520869, 13751756, 15151588, 16418681, 17487872, 18289121, 18927075, 19190927, 			 
		              18223885, 18053038, 17530431, 16710645, 15633200, 14431487, 13103160, 11870129, 11296846, 11921027, 13210691, 14559311, 15762995, 16804518, 17576918, 18177853, 18439525,				
		              17535273, 17345724, 16807882, 15996989, 14998392, 13785021, 12478791, 11277204, 10681604, 11400713, 12670352, 14020565, 15168194, 16110563, 16927655, 17562579, 17827009,			   
		              16799138, 16648313, 16148574, 15361379, 14345322, 13179768, 11921535, 10712040, 10359450, 10822581, 12159804, 13437273, 14617555, 15554967, 16446412, 17045186, 17294947,			  
		              16600000, 16431034, 15919475, 15119301, 14103415, 12959827, 11716192, 10507596, 10000000, 10633937, 11952707, 13185357, 14354097, 15420470, 16173390, 16709906, 16922803, 			 
		              17249426, 17085693, 16534328, 15744799, 14713274, 13567752, 12236481, 11074703, 10629435, 11198537, 12554257, 13796070, 14957570, 16058010, 16863848, 17453821, 17722440,				
		              18323758, 18122442, 17524211, 16654882, 15591621, 14353333, 13027033, 11843041, 11348695, 11992220, 13286766, 14581000, 15858035, 16889517, 17728594, 18379827, 18639294,			   
		              19305609, 19103740, 18466382, 17558947, 16450673, 15116156, 13775170, 12574919, 12006416, 12706999, 13991412, 15412890, 16757970, 17841657, 18793714, 19475632, 19721032,			  
		              20355722, 20148117, 19460133, 18492232, 17354010, 15964139, 14550900, 13282609, 12689352, 13439806, 14735740, 16233710, 17620323, 18780869, 19779157, 20526251, 20810140, 			 
		              21052430, 20857414, 20164246, 19195412, 17957110, 16568152, 15091551, 13808600, 13210159, 13934212, 15262588, 16844563, 18293770, 19511790, 20547740, 21325737, 21635371,				
		              21671111, 21447888, 20735275, 19736640, 18483954, 17051302, 15524257, 14196684, 13628660, 14327017, 15706672, 17355684, 18818427, 20097209, 21201222, 21986964, 22301163,			   
		              22111523, 21875510, 21137881, 20119255, 18855806, 17406338, 15840383, 14470343, 13917120, 14628988, 16023380, 17684820, 19189123, 20503193, 21619648, 22438642, 22797299,			  
		              22286035, 22047524, 21290827, 20279949, 19009305, 17565804, 15986396, 14610066, 14040078, 14808862, 16182708, 17828775, 19326693, 20689403, 21846354, 22629824, 22987323, 		 
		              }, 		
		/* Channel Gb */
		{			  20848664, 20601082, 19935743, 18969947, 17814577, 16356322, 14963875, 13784156, 12987988, 13653247, 15005620, 16411670, 17819383, 19098601, 19968659, 20624467, 20871295, 			 
				      20702724, 20456702, 19784054, 18841986, 17662040, 16239407, 14842661, 13643902, 12872006, 13556110, 14888633, 16279228, 17685812, 18924051, 19822729, 20461442, 20708594,				
				      20326292, 20104187, 19442848, 18551123, 17349915, 15952983, 14567089, 13347908, 12630290, 13302716, 14611565, 15992587, 17358083, 18572899, 19455100, 20093337, 20362368,			   
				      19752205, 19560875, 18996375, 18109453, 16907483, 15537144, 14190696, 12900538, 12293299, 12940278, 14203673, 15596425, 16938403, 18115025, 18994000, 19601616, 19811871,			  
				      19091832, 18927919, 18380298, 17542592, 16325979, 15061834, 13696680, 12462971, 11855602, 12519550, 13777755, 15174684, 16435785, 17542024, 18375229, 18974829, 19213251, 			 
				      18311786, 18132065, 17574498, 16750829, 15677036, 14454916, 13122315, 11883966, 11290078, 11920412, 13232928, 14605748, 15854840, 16878664, 17667710, 18257378, 18525422,				
				      17605214, 17436709, 16888497, 16032863, 15026699, 13821683, 12524195, 11287762, 10690757, 11395853, 12688210, 14027153, 15226066, 16211884, 17017153, 17654445, 17888551,			   
				      16871727, 16716431, 16233680, 15413633, 14386301, 13229286, 11945724, 10724996, 10343002, 10838747, 12205346, 13479290, 14668286, 15658678, 16534937, 17160804, 17418194,			  
				      16720147, 16521270, 15968277, 15175472, 14166477, 12989321, 11762858, 10521768, 10000000, 10656556, 11981662, 13221203, 14397400, 15488760, 16270370, 16812530, 17039985, 			 
				      17374324, 17169649, 16564689, 15775768, 14787608, 13596297, 12265881, 11070773, 10624788, 11205856, 12596961, 13822841, 15046228, 16131242, 16962008, 17542105, 17793709,				
				      18358572, 18177196, 17600400, 16705731, 15644035, 14407751, 13049490, 11868293, 11368951, 12012209, 13331741, 14655917, 15934273, 16990999, 17855957, 18534825, 18832689,			   
				      19362648, 19131074, 18498485, 17653214, 16491208, 15152342, 13802443, 12584938, 11995685, 12733094, 14035889, 15480804, 16821691, 17928709, 18865622, 19597064, 19903717,			  
				      20398573, 20194688, 19513032, 18540617, 17347220, 15979431, 14543351, 13271985, 12699858, 13436414, 14759704, 16284394, 17712126, 18881005, 19896104, 20639656, 20948030, 			 
				      21180952, 20926329, 20194635, 19228720, 18014781, 16632525, 15101966, 13804349, 13209016, 13955038, 15294503, 16920818, 18375807, 19613562, 20718819, 21450205, 21701993,				
				      21714900, 21486676, 20770722, 19771899, 18539983, 17057896, 15520031, 14209174, 13616774, 14324113, 15729048, 17401513, 18919667, 20158187, 21314818, 22120684, 22430743,			   
				      22088645, 21861221, 21163423, 20157813, 18901492, 17426107, 15823657, 14488474, 13919998, 14635808, 16071914, 17767692, 19305359, 20600412, 21756764, 22561208, 22854907,			  
				      22260337, 22057020, 21345489, 20364225, 19058870, 17603663, 15976608, 14638064, 14066616, 14802422, 16221302, 17959955, 19470362, 20851927, 21987418, 22806249, 23090950, 		 
				      }, 		
	    /* Channel B */
		{			  20042148, 19779193, 19172969, 18270590, 17225046, 15925687, 14671152, 13596535, 12839040, 13594736, 14775057, 16027015, 17274753, 18316906, 19146706, 19755674, 19924063, 			 
				      19905624, 19638707, 19016726, 18159800, 17108956, 15817874, 14571261, 13480962, 12731174, 13474939, 14658634, 15905785, 17130716, 18169551, 19006956, 19607343, 19769610,				
				      19579325, 19315516, 18694234, 17875851, 16825870, 15581940, 14319935, 13213853, 12509106, 13208632, 14411382, 15638146, 16854233, 17899261, 18712136, 19275864, 19481788,			   
				      19086712, 18842130, 18266741, 17464437, 16381958, 15207200, 13959845, 12821686, 12190508, 12850784, 14060796, 15284058, 16433186, 17519629, 18301804, 18860445, 19117617,			  
				      18555216, 18323317, 17724094, 16948899, 15886569, 14740598, 13492336, 12386503, 11785610, 12435383, 13632274, 14876456, 15999798, 17024503, 17811247, 18364855, 18569745, 			 
				      17813019, 17586310, 16991558, 16220682, 15271818, 14203200, 12944067, 11835228, 11239478, 11855430, 13095824, 14317312, 15429867, 16459116, 17182539, 17705885, 17927016,				
				      17113335, 16885054, 16338500, 15557477, 14653602, 13574716, 12384126, 11281888, 10666091, 11336881, 12574698, 13765031, 14880205, 15885932, 16574247, 17090724, 17320530,			   
				      16433332, 16226231, 15700753, 15026916, 14096263, 12996269, 11845434, 10704070, 10335470, 10811618, 12067871, 13230487, 14407228, 15336181, 16046526, 16610164, 16854233,			  
				      16226231, 16029145, 15517931, 14803791, 13869826, 12797764, 11650731, 10498565, 10000000, 10623009, 11872704, 13056384, 14210438, 15160213, 15879796, 16348319, 16548249, 			 
				      16862581, 16664084, 16084292, 15352601, 14459522, 13360708, 12168161, 11035391, 10626202, 11206023, 12446316, 13673590, 14786750, 15755091, 16541393, 17065811, 17272602,				
				      17835462, 17598189, 16978509, 16189260, 15257332, 14148086, 12923869, 11781331, 12352118, 11957718, 13153872, 14474306, 15580103, 16570832, 17359650, 17930202, 18143947,			   
				      18649502, 18452363, 17868253, 17051024, 16069341, 14868284, 13632341, 12497049, 12000748, 12637268, 13871699, 15245846, 16424772, 17455482, 18268485, 18852564, 19066645,			  
				      19659580, 19436412, 18783645, 17892335, 16888086, 15637264, 14358469, 13162639, 12660622, 13339126, 14604124, 15999890, 17214627, 18315636, 19187222, 19828311, 20043378, 			 
				      20338020, 20092437, 19387588, 18510714, 17466691, 16221203, 14850141, 13649576, 13123946, 13825423, 15084798, 16520562, 17877406, 18979458, 19878670, 20562597, 20795384,				
				      20805910, 20543415, 19870556, 18961429, 17874124, 16626438, 15225851, 14015623, 13489614, 14186063, 15496438, 16976951, 18307781, 19472764, 20407503, 21054650, 21302293,			   
				      21229762, 20933528, 20235085, 19334494, 18158671, 16955010, 15514504, 14249226, 13746925, 14477667, 15794643, 17284923, 18658845, 19828665, 20807236, 21461627, 21765079,			  
				      21468020, 21134999, 20381794, 19542574, 18318901, 17116080, 15671887, 14373568, 13873502, 14638411, 15961509, 17461249, 18838804, 20014246, 21036931, 21681233, 22034720, 		 

		}, 	
	},


	/* Light Source 3 */
	{
		/* AWB RGain */
		256,
		/* AWB BGain */
		256,
		/* Channel R */
		{			  20729753, 20475625, 19803352, 18912098, 17664130, 16293035, 14891347, 13689246, 12930767, 13677134, 15061625, 16442373, 17863714, 19143313, 20010332, 20680624, 20976999, 			 
					  20599909, 20343134, 19660232, 18755427, 17544851, 16172300, 14800093, 13574635, 12832091, 13570074, 14934293, 16332060, 17733098, 18991039, 19860580, 20511904, 20767436,				
					  20208349, 19956673, 19307705, 18420453, 17268656, 15897458, 14532107, 13278929, 12594212, 13306470, 14670926, 16044244, 17446842, 18648007, 19527067, 20176807, 20415443,			   
					  19628823, 19424839, 18817960, 17948773, 16813365, 15488425, 14116657, 12866447, 12234518, 12940049, 14243565, 15616572, 17006071, 18153487, 19054249, 19657858, 19878162,			  
					  19026899, 18831565, 18228314, 17336468, 16302621, 14983217, 13628757, 12414377, 11802526, 12487373, 13785204, 15163644, 16458944, 17589755, 18460140, 19075529, 19338956, 			 
					  18220154, 18033335, 17457950, 16652292, 15559202, 14378170, 13037690, 11812883, 11227499, 11894581, 13202904, 14581651, 15810275, 16867914, 17688120, 18348049, 18636999,				
					  17494945, 17318947, 16741179, 15909061, 14913003, 13719674, 12434556, 11237647, 10622726, 11331487, 12630059, 13978024, 15190527, 16201673, 17023466, 17632860, 17849575,			   
					  16849529, 16650217, 16113673, 15301538, 14308232, 13143444, 11900151, 10682279, 10310739, 10793227, 12118061, 13408429, 14629479, 15623785, 16483734, 17088465, 17317738,			  
					  16620310, 16443411, 15884951, 15109372, 14099769, 12931184, 11696948, 10486282, 10000000, 10626951, 11942135, 13192907, 14393226, 15444710, 16248822, 16807886, 17016404, 			 
					  17302495, 17099403, 16523679, 15739863, 14725875, 13552493, 12247115, 11084615, 10616491, 11196226, 12545140, 13806650, 15006697, 16091126, 16923314, 17543906, 17799991,				
					  18351604, 18148048, 17544201, 16700873, 15636118, 14375234, 13051867, 11848321, 12354654, 11996946, 13301544, 14645437, 15938070, 16997859, 17895624, 18523943, 18754279,			   
					  19401906, 19159507, 18504006, 17599144, 16513779, 15172742, 13831907, 12583443, 12015125, 12726510, 14048514, 15496211, 16845061, 17965734, 18941529, 19664833, 19928045,			  
					  20454641, 20235980, 19530141, 18579702, 17407315, 16050275, 14608885, 13319160, 12728874, 13462519, 14802784, 16321315, 17721154, 18923915, 19968376, 20740234, 21034170, 			 
					  21183165, 20954217, 20252501, 19264651, 18047392, 16662679, 15185125, 13844479, 13253423, 14020006, 15380597, 16944396, 18413226, 19673076, 20704707, 21510025, 21831931,				
					  21800685, 21561304, 20780028, 19794323, 18600068, 17185540, 15679698, 14292137, 13697560, 14408074, 15858163, 17468312, 18963870, 20278596, 21337257, 22139223, 22468170,			   
					  22290030, 22017768, 21185319, 20217917, 19022383, 17547570, 16016128, 14630835, 14015254, 14771556, 16207470, 17869474, 19390856, 20714833, 21824340, 22615258, 22945972,			  
					  22479028, 22232858, 21442913, 20424568, 19187098, 17730382, 16160310, 14775285, 14156783, 14964882, 16399515, 18061157, 19583516, 20947561, 22069069, 22829027, 23144250, 		 
					  }, 		
		/* Channel Gr */
		{			  20796680, 20612029, 19952339, 18996307, 17762892, 16342160, 14927902, 13762776, 12962272, 13683597, 14973160, 16359638, 17781995, 19017821, 19916316, 20549305, 20844928, 			 
		              20661908, 20455017, 19788584, 18853641, 17644227, 16242224, 14806627, 13644336, 12855560, 13562906, 14860790, 16229247, 17649585, 18869366, 19733444, 20377515, 20658124,				
		              20299235, 20075265, 19424216, 18534460, 17334423, 15954364, 14535953, 13351420, 12607292, 13328247, 14596406, 15946149, 17339236, 18518789, 19346833, 19993358, 20268417,			   
		              19701031, 19527421, 18935280, 18047217, 16882649, 15520048, 14170434, 12921510, 12274525, 12964674, 14228201, 15563970, 16917385, 18024960, 18877852, 19482109, 19717121,			  
		              19036890, 18851572, 18318517, 17503063, 16307351, 15042910, 13693342, 12462943, 11866624, 12520869, 13751756, 15151588, 16418681, 17487872, 18289121, 18927075, 19190927, 			 
		              18223885, 18053038, 17530431, 16710645, 15633200, 14431487, 13103160, 11870129, 11296846, 11921027, 13210691, 14559311, 15762995, 16804518, 17576918, 18177853, 18439525,				
		              17535273, 17345724, 16807882, 15996989, 14998392, 13785021, 12478791, 11277204, 10681604, 11400713, 12670352, 14020565, 15168194, 16110563, 16927655, 17562579, 17827009,			   
		              16799138, 16648313, 16148574, 15361379, 14345322, 13179768, 11921535, 10712040, 10359450, 10822581, 12159804, 13437273, 14617555, 15554967, 16446412, 17045186, 17294947,			  
		              16600000, 16431034, 15919475, 15119301, 14103415, 12959827, 11716192, 10507596, 10000000, 10633937, 11952707, 13185357, 14354097, 15420470, 16173390, 16709906, 16922803, 			 
		              17249426, 17085693, 16534328, 15744799, 14713274, 13567752, 12236481, 11074703, 10629435, 11198537, 12554257, 13796070, 14957570, 16058010, 16863848, 17453821, 17722440,				
		              18323758, 18122442, 17524211, 16654882, 15591621, 14353333, 13027033, 11843041, 11348695, 11992220, 13286766, 14581000, 15858035, 16889517, 17728594, 18379827, 18639294,			   
		              19305609, 19103740, 18466382, 17558947, 16450673, 15116156, 13775170, 12574919, 12006416, 12706999, 13991412, 15412890, 16757970, 17841657, 18793714, 19475632, 19721032,			  
		              20355722, 20148117, 19460133, 18492232, 17354010, 15964139, 14550900, 13282609, 12689352, 13439806, 14735740, 16233710, 17620323, 18780869, 19779157, 20526251, 20810140, 			 
		              21052430, 20857414, 20164246, 19195412, 17957110, 16568152, 15091551, 13808600, 13210159, 13934212, 15262588, 16844563, 18293770, 19511790, 20547740, 21325737, 21635371,				
		              21671111, 21447888, 20735275, 19736640, 18483954, 17051302, 15524257, 14196684, 13628660, 14327017, 15706672, 17355684, 18818427, 20097209, 21201222, 21986964, 22301163,			   
		              22111523, 21875510, 21137881, 20119255, 18855806, 17406338, 15840383, 14470343, 13917120, 14628988, 16023380, 17684820, 19189123, 20503193, 21619648, 22438642, 22797299,			  
		              22286035, 22047524, 21290827, 20279949, 19009305, 17565804, 15986396, 14610066, 14040078, 14808862, 16182708, 17828775, 19326693, 20689403, 21846354, 22629824, 22987323, 		 
		              }, 		
		/* Channel Gb */
		{			  20848664, 20601082, 19935743, 18969947, 17814577, 16356322, 14963875, 13784156, 12987988, 13653247, 15005620, 16411670, 17819383, 19098601, 19968659, 20624467, 20871295, 			 
				      20702724, 20456702, 19784054, 18841986, 17662040, 16239407, 14842661, 13643902, 12872006, 13556110, 14888633, 16279228, 17685812, 18924051, 19822729, 20461442, 20708594,				
				      20326292, 20104187, 19442848, 18551123, 17349915, 15952983, 14567089, 13347908, 12630290, 13302716, 14611565, 15992587, 17358083, 18572899, 19455100, 20093337, 20362368,			   
				      19752205, 19560875, 18996375, 18109453, 16907483, 15537144, 14190696, 12900538, 12293299, 12940278, 14203673, 15596425, 16938403, 18115025, 18994000, 19601616, 19811871,			  
				      19091832, 18927919, 18380298, 17542592, 16325979, 15061834, 13696680, 12462971, 11855602, 12519550, 13777755, 15174684, 16435785, 17542024, 18375229, 18974829, 19213251, 			 
				      18311786, 18132065, 17574498, 16750829, 15677036, 14454916, 13122315, 11883966, 11290078, 11920412, 13232928, 14605748, 15854840, 16878664, 17667710, 18257378, 18525422,				
				      17605214, 17436709, 16888497, 16032863, 15026699, 13821683, 12524195, 11287762, 10690757, 11395853, 12688210, 14027153, 15226066, 16211884, 17017153, 17654445, 17888551,			   
				      16871727, 16716431, 16233680, 15413633, 14386301, 13229286, 11945724, 10724996, 10343002, 10838747, 12205346, 13479290, 14668286, 15658678, 16534937, 17160804, 17418194,			  
				      16720147, 16521270, 15968277, 15175472, 14166477, 12989321, 11762858, 10521768, 10000000, 10656556, 11981662, 13221203, 14397400, 15488760, 16270370, 16812530, 17039985, 			 
				      17374324, 17169649, 16564689, 15775768, 14787608, 13596297, 12265881, 11070773, 10624788, 11205856, 12596961, 13822841, 15046228, 16131242, 16962008, 17542105, 17793709,				
				      18358572, 18177196, 17600400, 16705731, 15644035, 14407751, 13049490, 11868293, 11368951, 12012209, 13331741, 14655917, 15934273, 16990999, 17855957, 18534825, 18832689,			   
				      19362648, 19131074, 18498485, 17653214, 16491208, 15152342, 13802443, 12584938, 11995685, 12733094, 14035889, 15480804, 16821691, 17928709, 18865622, 19597064, 19903717,			  
				      20398573, 20194688, 19513032, 18540617, 17347220, 15979431, 14543351, 13271985, 12699858, 13436414, 14759704, 16284394, 17712126, 18881005, 19896104, 20639656, 20948030, 			 
				      21180952, 20926329, 20194635, 19228720, 18014781, 16632525, 15101966, 13804349, 13209016, 13955038, 15294503, 16920818, 18375807, 19613562, 20718819, 21450205, 21701993,				
				      21714900, 21486676, 20770722, 19771899, 18539983, 17057896, 15520031, 14209174, 13616774, 14324113, 15729048, 17401513, 18919667, 20158187, 21314818, 22120684, 22430743,			   
				      22088645, 21861221, 21163423, 20157813, 18901492, 17426107, 15823657, 14488474, 13919998, 14635808, 16071914, 17767692, 19305359, 20600412, 21756764, 22561208, 22854907,			  
				      22260337, 22057020, 21345489, 20364225, 19058870, 17603663, 15976608, 14638064, 14066616, 14802422, 16221302, 17959955, 19470362, 20851927, 21987418, 22806249, 23090950, 		 
				      }, 		
	    /* Channel B */
		{			  20042148, 19779193, 19172969, 18270590, 17225046, 15925687, 14671152, 13596535, 12839040, 13594736, 14775057, 16027015, 17274753, 18316906, 19146706, 19755674, 19924063, 			 
				      19905624, 19638707, 19016726, 18159800, 17108956, 15817874, 14571261, 13480962, 12731174, 13474939, 14658634, 15905785, 17130716, 18169551, 19006956, 19607343, 19769610,				
				      19579325, 19315516, 18694234, 17875851, 16825870, 15581940, 14319935, 13213853, 12509106, 13208632, 14411382, 15638146, 16854233, 17899261, 18712136, 19275864, 19481788,			   
				      19086712, 18842130, 18266741, 17464437, 16381958, 15207200, 13959845, 12821686, 12190508, 12850784, 14060796, 15284058, 16433186, 17519629, 18301804, 18860445, 19117617,			  
				      18555216, 18323317, 17724094, 16948899, 15886569, 14740598, 13492336, 12386503, 11785610, 12435383, 13632274, 14876456, 15999798, 17024503, 17811247, 18364855, 18569745, 			 
				      17813019, 17586310, 16991558, 16220682, 15271818, 14203200, 12944067, 11835228, 11239478, 11855430, 13095824, 14317312, 15429867, 16459116, 17182539, 17705885, 17927016,				
				      17113335, 16885054, 16338500, 15557477, 14653602, 13574716, 12384126, 11281888, 10666091, 11336881, 12574698, 13765031, 14880205, 15885932, 16574247, 17090724, 17320530,			   
				      16433332, 16226231, 15700753, 15026916, 14096263, 12996269, 11845434, 10704070, 10335470, 10811618, 12067871, 13230487, 14407228, 15336181, 16046526, 16610164, 16854233,			  
				      16226231, 16029145, 15517931, 14803791, 13869826, 12797764, 11650731, 10498565, 10000000, 10623009, 11872704, 13056384, 14210438, 15160213, 15879796, 16348319, 16548249, 			 
				      16862581, 16664084, 16084292, 15352601, 14459522, 13360708, 12168161, 11035391, 10626202, 11206023, 12446316, 13673590, 14786750, 15755091, 16541393, 17065811, 17272602,				
				      17835462, 17598189, 16978509, 16189260, 15257332, 14148086, 12923869, 11781331, 12352118, 11957718, 13153872, 14474306, 15580103, 16570832, 17359650, 17930202, 18143947,			   
				      18649502, 18452363, 17868253, 17051024, 16069341, 14868284, 13632341, 12497049, 12000748, 12637268, 13871699, 15245846, 16424772, 17455482, 18268485, 18852564, 19066645,			  
				      19659580, 19436412, 18783645, 17892335, 16888086, 15637264, 14358469, 13162639, 12660622, 13339126, 14604124, 15999890, 17214627, 18315636, 19187222, 19828311, 20043378, 			 
				      20338020, 20092437, 19387588, 18510714, 17466691, 16221203, 14850141, 13649576, 13123946, 13825423, 15084798, 16520562, 17877406, 18979458, 19878670, 20562597, 20795384,				
				      20805910, 20543415, 19870556, 18961429, 17874124, 16626438, 15225851, 14015623, 13489614, 14186063, 15496438, 16976951, 18307781, 19472764, 20407503, 21054650, 21302293,			   
				      21229762, 20933528, 20235085, 19334494, 18158671, 16955010, 15514504, 14249226, 13746925, 14477667, 15794643, 17284923, 18658845, 19828665, 20807236, 21461627, 21765079,			  
				      21468020, 21134999, 20381794, 19542574, 18318901, 17116080, 15671887, 14373568, 13873502, 14638411, 15961509, 17461249, 18838804, 20014246, 21036931, 21681233, 22034720, 		 

		}, 	
	},

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

HI_U32 sc1235_get_isp_default(ISP_CMOS_DEFAULT_S *pstDef)
{   
    if (HI_NULL == pstDef)
    {
        printf("null pointer when get isp default value!\n");
        return -1;
    }

    memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

    
//    memcpy(&pstDef->stLsc.stLscUniParaTable, &g_stCmosLscUniTable, sizeof(ISP_LSC_CABLI_UNI_TABLE_S));
//    memcpy(&pstDef->stLsc.stLscParaTable[0], &g_stCmosLscTable[0], sizeof(ISP_LSC_CABLI_TABLE_S)*HI_ISP_LSC_LIGHT_NUM);  
    memcpy(&pstDef->stDrc, &g_stIspDrc, sizeof(ISP_CMOS_DRC_S));
    memcpy(&pstDef->stDemosaic, &g_stIspDemosaic, sizeof(ISP_CMOS_DEMOSAIC_S));
    memcpy(&pstDef->stGe, &g_stIspGe, sizeof(ISP_CMOS_GE_S));	

    pstDef->stNoiseTbl.stNrCaliPara.u8CalicoefRow = DMNR_CALIB_CARVE_NUM_SC1235;
    pstDef->stNoiseTbl.stNrCaliPara.pCalibcoef    = (HI_FLOAT (*)[4])g_coef_calib_sc1235;
//	memcpy(&pstDef->stNoiseTbl.stIsoParaTable[0], &g_stNrIsoParaTab[0],sizeof(ISP_NR_ISO_PARA_TABLE_S)*HI_ISP_NR_ISO_LEVEL_MAX);
	
    memcpy(&pstDef->stRgbSharpen, &g_stIspRgbSharpen, sizeof(ISP_CMOS_RGBSHARPEN_S));
	memcpy(&pstDef->stUvnr,       &g_stIspUVNR,       sizeof(ISP_CMOS_UVNR_S));
	memcpy(&pstDef->stDpc,       &g_stCmosDpc,       sizeof(ISP_CMOS_DPC_S));

    pstDef->stSensorMaxResolution.u32MaxWidth  = SENSOR_SC1235_WIDTH;
    pstDef->stSensorMaxResolution.u32MaxHeight = SENSOR_SC1235_HEIGHT;

    return 0;
}


HI_U32 sc1235_get_isp_black_level(ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel)
{
   // HI_S32  i;
    
    if (HI_NULL == pstBlackLevel)
    {
        printf("null pointer when get isp black level value!\n");
        return -1;
    }

    /* Don't need to update black level when iso change */
    pstBlackLevel->bUpdate = HI_FALSE;
          
    pstBlackLevel->au16BlackLevel[0] = 69;
    pstBlackLevel->au16BlackLevel[1] = 69;
    pstBlackLevel->au16BlackLevel[2] = 69;
    pstBlackLevel->au16BlackLevel[3] = 69;

    return 0;  
    
}

HI_VOID sc1235_set_pixel_detect(HI_BOOL bEnable)
{
	HI_U32 u32Lines = VMAX_960P30_LINEAR * 30 /5;
return;	
#if CMOS_SC1235_ISP_WRITE_SENSOR_ENABLE
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        /* 5 fps */
		SENSOR_I2C_WRITE(0x320e, (u32Lines >> 4) && 0xFF);
		SENSOR_I2C_WRITE(0x320f, ((u32Lines<<4)&&0xF0));
       
    }
    else /* setup for ISP 'normal mode' */
    { 
        SENSOR_I2C_WRITE(0x320e, (gu32FullLinesStd >> 8) && 0XFF);
        SENSOR_I2C_WRITE(0x320f, gu32FullLinesStd && 0xFF);
        
        bInit = HI_FALSE;
    }
#else
    if (bEnable) /* setup for ISP pixel calibration mode */
    {
        
		SENSOR_I2C_WRITE(0x3e01, (u32Lines >> 8) && 0xFF);
		SENSOR_I2C_WRITE(0x3e02, (u32Lines - 4) && 0xFF);
        
        /* min gain */
        SENSOR_I2C_WRITE(0x3e0e, 0x00);
		SENSOR_I2C_WRITE(0x3e0f, 0x00);

		/* 5 fps */
        SENSOR_I2C_WRITE(0x320e, (u32Lines >> 8) && 0xFF);
        SENSOR_I2C_WRITE(0x320f, u32Lines && 0xFF);
    }
    else /* setup for ISP 'normal mode' */
    { 
        SENSOR_I2C_WRITE(0x320e, (gu32FullLinesStd >> 8) && 0XFF);
        SENSOR_I2C_WRITE(0x320f, gu32FullLinesStd && 0xFF);
        
        bInit = HI_FALSE;
    }
#endif

    return;
}

HI_VOID sc1235_set_wdr_mode(HI_U8 u8Mode)
{
    bInit = HI_FALSE;
    
    switch(u8Mode)
    {
        case WDR_MODE_NONE:
            if (SENSOR_960P_30FPS_MODE == gu8SensorImageMode)
            {
                gu32FullLinesStd = VMAX_960P30_LINEAR;
            }
            genSensorMode = WDR_MODE_NONE;
            printf("linear mode\n");
        break;
        default:
            printf("NOT support this mode!\n");
            return;
        break;
    }
    return;
}

HI_U32 sc1235_get_sns_regs_info(ISP_SNS_REGS_INFO_S *pstSnsRegsInfo)
{

#if CMOS_SC1235_ISP_WRITE_SENSOR_ENABLE

    HI_S32 i;

    if (HI_FALSE == bInit)
    {
        g_stSnsRegsInfo.enSnsType = ISP_SNS_I2C_TYPE;
        g_stSnsRegsInfo.u8Cfg2ValidDelayMax = 2;		
        g_stSnsRegsInfo.u32RegNum = 18;
	
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)
        {	
            g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            g_stSnsRegsInfo.astI2cData[i].u8DevAddr = sensor_i2c_addr;
            g_stSnsRegsInfo.astI2cData[i].u32AddrByteNum = sensor_addr_byte;
            g_stSnsRegsInfo.astI2cData[i].u32DataByteNum = sensor_data_byte;
        }		
        g_stSnsRegsInfo.astI2cData[0].u8DelayFrmNum = HI_FALSE; 
        g_stSnsRegsInfo.astI2cData[0].u32RegAddr = 0x3e01;     //exp high  bit[7:0] 
        g_stSnsRegsInfo.astI2cData[1].u8DelayFrmNum = HI_FALSE; 
        g_stSnsRegsInfo.astI2cData[1].u32RegAddr = 0x3e02;     //exp low  bit[7:4] 
        g_stSnsRegsInfo.astI2cData[2].u8DelayFrmNum = HI_FALSE; 
        g_stSnsRegsInfo.astI2cData[2].u32RegAddr = 0x3e08;     //digita agin[6:5];    coarse analog again[4:2]
		g_stSnsRegsInfo.astI2cData[3].u8DelayFrmNum = HI_FALSE; 
        g_stSnsRegsInfo.astI2cData[3].u32RegAddr = 0x3e09;     //fine analog again[4:0]

        g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = HI_FALSE;        //FrameL 
        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x320c;
        g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = HI_FALSE;       
        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x320d;

        g_stSnsRegsInfo.astI2cData[6].u8DelayFrmNum = HI_FALSE;        //FrameH 
        g_stSnsRegsInfo.astI2cData[6].u32RegAddr = 0x320e;
        g_stSnsRegsInfo.astI2cData[7].u8DelayFrmNum = HI_FALSE;       
        g_stSnsRegsInfo.astI2cData[7].u32RegAddr = 0x320f;
        
        g_stSnsRegsInfo.astI2cData[8].u8DelayFrmNum = HI_FALSE;        //aec/agc timing 
        g_stSnsRegsInfo.astI2cData[8].u32RegAddr = 0x3903;
        g_stSnsRegsInfo.astI2cData[9].u8DelayFrmNum = HI_FALSE;       
        g_stSnsRegsInfo.astI2cData[9].u32RegAddr = 0x3903;
        g_stSnsRegsInfo.astI2cData[10].u8DelayFrmNum = HI_FALSE;       
        g_stSnsRegsInfo.astI2cData[10].u32RegAddr = 0x3812;
        g_stSnsRegsInfo.astI2cData[11].u8DelayFrmNum = HI_FALSE;       
        g_stSnsRegsInfo.astI2cData[11].u32RegAddr = 0x3631;
 //       g_stSnsRegsInfo.astI2cData[12].u8DelayFrmNum = HI_FALSE;        
 //       g_stSnsRegsInfo.astI2cData[12].u32RegAddr = 0x3639;
        g_stSnsRegsInfo.astI2cData[12].u8DelayFrmNum = HI_FALSE;       
        g_stSnsRegsInfo.astI2cData[12].u32RegAddr = 0x3301;
	g_stSnsRegsInfo.astI2cData[13].u8DelayFrmNum = HI_FALSE;       
	g_stSnsRegsInfo.astI2cData[13].u32RegAddr = 0x3633;

	g_stSnsRegsInfo.astI2cData[14].u8DelayFrmNum = HI_FALSE;       
	g_stSnsRegsInfo.astI2cData[14].u32RegAddr = 0x3812;

	g_stSnsRegsInfo.astI2cData[15].u8DelayFrmNum = HI_FALSE;       
	g_stSnsRegsInfo.astI2cData[15].u32RegAddr = 0x5781;

	g_stSnsRegsInfo.astI2cData[16].u8DelayFrmNum = HI_FALSE;       
	g_stSnsRegsInfo.astI2cData[16].u32RegAddr = 0x5785;

	g_stSnsRegsInfo.astI2cData[17].u8DelayFrmNum = HI_FALSE;       
	g_stSnsRegsInfo.astI2cData[17].u32RegAddr = 0x3314;

//		g_stSnsRegsInfo.astI2cData[4].u8DelayFrmNum = 0;
//        g_stSnsRegsInfo.astI2cData[4].u32RegAddr = 0x320e;     //TIMING_VTS  high bit[7:0] 
//		g_stSnsRegsInfo.astI2cData[5].u8DelayFrmNum = 0;
//        g_stSnsRegsInfo.astI2cData[5].u32RegAddr = 0x320f;     //TIMING_VTS  low bit[7:0] 

	
        bInit = HI_TRUE;
    }
    else    
    {        
        for (i=0; i<g_stSnsRegsInfo.u32RegNum; i++)        
        {            
            if (g_stSnsRegsInfo.astI2cData[i].u32Data == g_stPreSnsRegsInfo.astI2cData[i].u32Data)            
            {                
                g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_FALSE;
            }            
            else            
	        {
                g_stSnsRegsInfo.astI2cData[i].bUpdate = HI_TRUE;
            }        
        } 
	if( HI_TRUE == (g_stSnsRegsInfo.astI2cData[0].bUpdate ||  g_stSnsRegsInfo.astI2cData[1].bUpdate))
		{
			g_stSnsRegsInfo.astI2cData[8].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[9].bUpdate = HI_TRUE;
		}
		if( HI_TRUE == (g_stSnsRegsInfo.astI2cData[2].bUpdate ||  g_stSnsRegsInfo.astI2cData[3].bUpdate))
		{
			g_stSnsRegsInfo.astI2cData[8].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[9].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[10].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[11].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[12].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[13].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[14].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[15].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[16].bUpdate = HI_TRUE;
			g_stSnsRegsInfo.astI2cData[17].bUpdate = HI_TRUE;
		}   
    }

    if (HI_NULL == pstSnsRegsInfo)
    {
        printf("null pointer when get sns reg info!\n");
        return -1;
    }

    memcpy(pstSnsRegsInfo, &g_stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S)); 
    memcpy(&g_stPreSnsRegsInfo, &g_stSnsRegsInfo, sizeof(ISP_SNS_REGS_INFO_S)); 
#endif
    return 0;
}

static HI_S32 sc1235_set_image_mode(ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
    HI_U8 u8SensorImageMode = gu8SensorImageMode;

    bInit = HI_FALSE;

    if (HI_NULL == pstSensorImageMode )
    {
        printf("null pointer when set image mode\n");
        return -1;
    }

    if ((pstSensorImageMode->u16Width <= 1280) && (pstSensorImageMode->u16Height <= 960))
    {
        if (WDR_MODE_NONE == genSensorMode)
        {
            if (pstSensorImageMode->f32Fps <= 30)
            {
                u8SensorImageMode = SENSOR_960P_30FPS_MODE;
            }
            else
            {
                printf("Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n", 
                    pstSensorImageMode->u16Width, 
                    pstSensorImageMode->u16Height,
                    pstSensorImageMode->f32Fps,
                    genSensorMode);

                return -1;
            }
        }
        else
        {
            printf("Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n", 
                pstSensorImageMode->u16Width, 
                pstSensorImageMode->u16Height,
                pstSensorImageMode->f32Fps,
                genSensorMode);

            return -1;
        }
    }
    else
    {
        printf("Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n", 
            pstSensorImageMode->u16Width, 
            pstSensorImageMode->u16Height,
            pstSensorImageMode->f32Fps,
            genSensorMode);

        return -1;
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

HI_VOID sc1235_global_init()
{   
    gu8SensorImageMode = SENSOR_960P_30FPS_MODE;
    genSensorMode = WDR_MODE_NONE;
    gu32FullLinesStd = VMAX_960P30_LINEAR; 
    gu32FullLines = VMAX_960P30_LINEAR;
    bInit = HI_FALSE;
    bSensorInit = HI_FALSE; 

    memset(&g_stSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
    memset(&g_stPreSnsRegsInfo, 0, sizeof(ISP_SNS_REGS_INFO_S));
}

static void sc1235_reg_init()
{
/**/
//----------DOTHINKEY REGISTER SETTING----------------
SENSOR_I2C_WRITE(0x0103,0x01);
SENSOR_I2C_WRITE(0x0100,0x00);

SENSOR_I2C_WRITE(0x3621,0x28);
SENSOR_I2C_WRITE(0x3622,0x02);

SENSOR_I2C_WRITE(0x3633,0xb5);
SENSOR_I2C_WRITE(0x3634,0xa4);
SENSOR_I2C_WRITE(0x3635,0xa0); //bypass txvdd to AVDD
SENSOR_I2C_WRITE(0x3305,0x00);
SENSOR_I2C_WRITE(0x3306,0x60);

SENSOR_I2C_WRITE(0x3638,0xb8);
SENSOR_I2C_WRITE(0x3639,0x0c);
SENSOR_I2C_WRITE(0x363a,0x1f);
SENSOR_I2C_WRITE(0x363b,0x09); //hvdd
SENSOR_I2C_WRITE(0x363c,0x05); //NVDD -0.8V

SENSOR_I2C_WRITE(0x3e08,0x7f);
SENSOR_I2C_WRITE(0x3e09,0x1f);


SENSOR_I2C_WRITE(0x33b5,0x10);
SENSOR_I2C_WRITE(0x4500,0x59);
SENSOR_I2C_WRITE(0x3211,0x08);

SENSOR_I2C_WRITE(0x335c,0x57); //[3:0] falling edge of stg1 samp LSB
SENSOR_I2C_WRITE(0x333a,0xaa); //[7:4] falling edge of stg1 samp MSB

SENSOR_I2C_WRITE(0x3d08,0x01);//0x01
SENSOR_I2C_WRITE(0x3621,0x28);


SENSOR_I2C_WRITE(0x330b,0xd6);
           
//low power
SENSOR_I2C_WRITE(0x3303,0x28);
SENSOR_I2C_WRITE(0x3309,0x28);
SENSOR_I2C_WRITE(0x333a,0x0a);
SENSOR_I2C_WRITE(0x3633,0x33);
SENSOR_I2C_WRITE(0x3634,0x42);

//fullwell blksun
SENSOR_I2C_WRITE(0x3622,0x06);
SENSOR_I2C_WRITE(0x3637,0x0f);
SENSOR_I2C_WRITE(0x3908,0x11);
                           
SENSOR_I2C_WRITE(0x337f,0x03); //[1:0] 11:new auto precharge  330e in 3372    [7:6] 11: close div_rst 00:open div_rst
SENSOR_I2C_WRITE(0x3368,0x03);
SENSOR_I2C_WRITE(0x3369,0x00);
SENSOR_I2C_WRITE(0x336a,0x00);
SENSOR_I2C_WRITE(0x336b,0x00);
SENSOR_I2C_WRITE(0x3367,0x08);
SENSOR_I2C_WRITE(0x330e,0x30);
                           
SENSOR_I2C_WRITE(0x3366,0x7c); // div_rst gap


//ae
SENSOR_I2C_WRITE(0x3e08,0x03);
SENSOR_I2C_WRITE(0x3e09,0x10);
SENSOR_I2C_WRITE(0x3638,0x38); //ramp_gen by sc
SENSOR_I2C_WRITE(0x3636,0x25);
SENSOR_I2C_WRITE(0x3625,0x01);

//25fps  59mW
SENSOR_I2C_WRITE(0x320c,0x08); 
SENSOR_I2C_WRITE(0x320d,0x70); 

//30fps  63mW
//SENSOR_I2C_WRITE(0x320c,0x07); 
//SENSOR_I2C_WRITE(0x320d,0x08); 


//0526
//dithring 
SENSOR_I2C_WRITE(0x391e,0x00);  //[2] 1:enable

SENSOR_I2C_WRITE(0x330b,0xb8); 

//0531  

SENSOR_I2C_WRITE(0x3034,0x05);
SENSOR_I2C_WRITE(0x3301,0x0f);
SENSOR_I2C_WRITE(0x3306,0x78);
SENSOR_I2C_WRITE(0x3638,0x28);
SENSOR_I2C_WRITE(0x330a,0x01);
SENSOR_I2C_WRITE(0x330b,0x80);
SENSOR_I2C_WRITE(0x3633,0x22);
SENSOR_I2C_WRITE(0x3634,0x21);
           
           
SENSOR_I2C_WRITE(0x3630,0xc8);
SENSOR_I2C_WRITE(0x3631,0x86);
           
SENSOR_I2C_WRITE(0x3e01,0x3e);
           
SENSOR_I2C_WRITE(0x3364,0x05);// [2] 1: write at sampling ending
           
           
           
////0531B  
SENSOR_I2C_WRITE(0x3630,0xd8);
SENSOR_I2C_WRITE(0x363c,0x06);
SENSOR_I2C_WRITE(0x3637,0x0e);
SENSOR_I2C_WRITE(0x330b,0x90);


//0602
SENSOR_I2C_WRITE(0x3638,0x2f);
SENSOR_I2C_WRITE(0x3306,0xb8);
SENSOR_I2C_WRITE(0x330b,0xa8);
SENSOR_I2C_WRITE(0x3309,0x38);
SENSOR_I2C_WRITE(0x331f,0x1d);
SENSOR_I2C_WRITE(0x3321,0x1f);

//blc
//SENSOR_I2C_WRITE(0x3905,0xda);

//612 prnu
SENSOR_I2C_WRITE(0x335e,0x01);
SENSOR_I2C_WRITE(0x335f,0x03);
SENSOR_I2C_WRITE(0x337c,0x04);
SENSOR_I2C_WRITE(0x337d,0x06);
SENSOR_I2C_WRITE(0x33a0,0x05);
SENSOR_I2C_WRITE(0x3301,0x05);
SENSOR_I2C_WRITE(0x3302,0xff);
SENSOR_I2C_WRITE(0x3633,0x2f);


//0612
SENSOR_I2C_WRITE(0x3306,0x58);
SENSOR_I2C_WRITE(0x330b,0x6c);
SENSOR_I2C_WRITE(0x3630,0xa8);
SENSOR_I2C_WRITE(0x3622,0x02);
                            
                            
SENSOR_I2C_WRITE(0x3638,0x0f);
SENSOR_I2C_WRITE(0x3306,0x68);
SENSOR_I2C_WRITE(0x366e,0x08);  // ofs auto en [3]
SENSOR_I2C_WRITE(0x366f,0x2f);  // ofs+finegain  real ofs in 0x3687[4:0]

//ana dithering
//SENSOR_I2C_WRITE(0x3364,0x15); //[4] auto en
SENSOR_I2C_WRITE(0x3e23,0x07); //gain set
SENSOR_I2C_WRITE(0x3e24,0x10);
SENSOR_I2C_WRITE(0x331d,0x0a); //value
//SENSOR_I2C_WRITE(0x3301,0xff);


//SENSOR_I2C_WRITE(0x337f,0x23); //[5] st2 adjsut en
SENSOR_I2C_WRITE(0x333b,0x00); //[3:0] msb
SENSOR_I2C_WRITE(0x3357,0x5a); //[3:0] lsb


//smear
SENSOR_I2C_WRITE(0x3309,0xa8);
SENSOR_I2C_WRITE(0x331f,0x8d);
SENSOR_I2C_WRITE(0x3321,0x8f);


//0613
SENSOR_I2C_WRITE(0x3631,0x84);
//SENSOR_I2C_WRITE(0x363c,0x26); //bypass npump
SENSOR_I2C_WRITE(0x3038,0xff);  //light
SENSOR_I2C_WRITE(0x3213,0x00);


//0615
SENSOR_I2C_WRITE(0x391b,0x4d); //high temp

//0622
//atuo logic

//SENSOR_I2C_WRITE(0x3670,0x08); //[3]:3633 logic ctrl  real value in 3682
//SENSOR_I2C_WRITE(0x367e,0x07);  //gain0
//SENSOR_I2C_WRITE(0x367f,0x0f);  //gain1
//SENSOR_I2C_WRITE(0x3677,0x2f);  //<gain0
//SENSOR_I2C_WRITE(0x3678,0x23);  //gain0 - gain1
//SENSOR_I2C_WRITE(0x3679,0x23);  //>gain1


SENSOR_I2C_WRITE(0x337f,0x03); //new auto precharge  330e in 3372
SENSOR_I2C_WRITE(0x3368,0x02);
SENSOR_I2C_WRITE(0x3369,0x00);
SENSOR_I2C_WRITE(0x336a,0x00);
SENSOR_I2C_WRITE(0x336b,0x00);
SENSOR_I2C_WRITE(0x3367,0x08);
SENSOR_I2C_WRITE(0x330e,0x30);
          
SENSOR_I2C_WRITE(0x3e03,0x03);     //add for gain  choose       
SENSOR_I2C_WRITE(0x3e08,0x00);          
//0626
SENSOR_I2C_WRITE(0x3213,0x02); //flip
//0721
SENSOR_I2C_WRITE(0x3802,0x01);
SENSOR_I2C_WRITE(0x3235,0x03);
SENSOR_I2C_WRITE(0x3236,0xe6);

//SENSOR_I2C_WRITE(0x3679,0x43);
SENSOR_I2C_WRITE(0x3f00,0x07);
SENSOR_I2C_WRITE(0x3f04,0x08);
SENSOR_I2C_WRITE(0x3f05,0x4c);

//manual DPC
SENSOR_I2C_WRITE(0x5780,0xff);
SENSOR_I2C_WRITE(0x5781,0x04);
SENSOR_I2C_WRITE(0x5785,0x18);
//0823
SENSOR_I2C_WRITE(0x3622,0x06);
SENSOR_I2C_WRITE(0x3630,0x28);

SENSOR_I2C_WRITE(0x3313,0x05);
//0830
SENSOR_I2C_WRITE(0x3639,0x09);

SENSOR_I2C_WRITE(0x0100,0x01);

/**
SENSOR_I2C_WRITE(0x0100,0x00);
SENSOR_I2C_WRITE(0x3621,0x28);
SENSOR_I2C_WRITE(0x3622,0x02);
SENSOR_I2C_WRITE(0x3633,0xb5);
SENSOR_I2C_WRITE(0x3634,0xa4);
SENSOR_I2C_WRITE(0x3635,0xa0);
//bypass txvdd to AVDD
SENSOR_I2C_WRITE(0x3305,0x00);
SENSOR_I2C_WRITE(0x3306,0x60);
SENSOR_I2C_WRITE(0x3638,0xb8);
SENSOR_I2C_WRITE(0x3639,0x0c);
SENSOR_I2C_WRITE(0x363a,0x1f);
SENSOR_I2C_WRITE(0x363b,0x09);
//hvdd
SENSOR_I2C_WRITE(0x363c,0x05);
//NVDD -0.8V
SENSOR_I2C_WRITE(0x3e08,0x7f);
SENSOR_I2C_WRITE(0x3e09,0x1f);
SENSOR_I2C_WRITE(0x33b5,0x10);
SENSOR_I2C_WRITE(0x4500,0x59);
SENSOR_I2C_WRITE(0x3211,0x08);
SENSOR_I2C_WRITE(0x335c,0x57);
//[3:0] falling edge of stg1 samp LSB
SENSOR_I2C_WRITE(0x333a,0xaa);
//[7:4] falling edge of stg1 samp MSB
SENSOR_I2C_WRITE(0x3d08,0x00);
SENSOR_I2C_WRITE(0x3621,0x28);
SENSOR_I2C_WRITE(0x330b,0xd6);
//low power
SENSOR_I2C_WRITE(0x3303,0x28);
SENSOR_I2C_WRITE(0x3309,0x28);
SENSOR_I2C_WRITE(0x333a,0x0a);
SENSOR_I2C_WRITE(0x3633,0x33);
SENSOR_I2C_WRITE(0x3634,0x42);
//fullwell blksun
SENSOR_I2C_WRITE(0x3622,0x06);
SENSOR_I2C_WRITE(0x3637,0x0f);
SENSOR_I2C_WRITE(0x3908,0x11);
SENSOR_I2C_WRITE(0x337f,0x03);
//[1:0] 11:new auto precharge  330e in 3372    [7:6] 11: close div_rst 00:open div_rst
SENSOR_I2C_WRITE(0x3368,0x03);
SENSOR_I2C_WRITE(0x3369,0x00);
SENSOR_I2C_WRITE(0x336a,0x00);
SENSOR_I2C_WRITE(0x336b,0x00);
SENSOR_I2C_WRITE(0x3367,0x08);
SENSOR_I2C_WRITE(0x330e,0x30);
SENSOR_I2C_WRITE(0x3366,0x7c);
// div_rst gap
//ae
SENSOR_I2C_WRITE(0x3e08,0x03);
SENSOR_I2C_WRITE(0x3e09,0x10);
SENSOR_I2C_WRITE(0x3638,0x38);
//ramp_gen by sc
SENSOR_I2C_WRITE(0x3636,0x25);
SENSOR_I2C_WRITE(0x3625,0x01);
//25fps  59mW
SENSOR_I2C_WRITE(0x320c,0x08);
SENSOR_I2C_WRITE(0x320d,0x70);
//30fps  63mW
//0x320c,0x07);
//0x320d,0x08);
//0526
//dithring
SENSOR_I2C_WRITE(0x391e,0x00);
//[2] 1:enable
SENSOR_I2C_WRITE(0x330b,0xb8);
//0531
SENSOR_I2C_WRITE(0x3034,0x05);
SENSOR_I2C_WRITE(0x3301,0x0f);
SENSOR_I2C_WRITE(0x3306,0x78);
SENSOR_I2C_WRITE(0x3638,0x28);
SENSOR_I2C_WRITE(0x330a,0x01);
SENSOR_I2C_WRITE(0x330b,0x80);
SENSOR_I2C_WRITE(0x3633,0x22);
SENSOR_I2C_WRITE(0x3634,0x21);
SENSOR_I2C_WRITE(0x3630,0xc8);
SENSOR_I2C_WRITE(0x3631,0x86);
SENSOR_I2C_WRITE(0x3e01,0x3e);
SENSOR_I2C_WRITE(0x3364,0x05);
// [2] 1: write at sampling ending
//
//0531B
SENSOR_I2C_WRITE(0x3630,0xd8);
SENSOR_I2C_WRITE(0x363c,0x06);
SENSOR_I2C_WRITE(0x3637,0x0e);
SENSOR_I2C_WRITE(0x330b,0x90);
//0602
SENSOR_I2C_WRITE(0x3638,0x2f);
SENSOR_I2C_WRITE(0x3306,0xb8);
SENSOR_I2C_WRITE(0x330b,0xa8);
SENSOR_I2C_WRITE(0x3309,0x38);
SENSOR_I2C_WRITE(0x331f,0x1d);
SENSOR_I2C_WRITE(0x3321,0x1f);
//blc
//0x3905,0xda);
//612 prnu
SENSOR_I2C_WRITE(0x335e,0x01);
SENSOR_I2C_WRITE(0x335f,0x03);
SENSOR_I2C_WRITE(0x337c,0x04);
SENSOR_I2C_WRITE(0x337d,0x06);
SENSOR_I2C_WRITE(0x33a0,0x05);
SENSOR_I2C_WRITE(0x3301,0x05);
SENSOR_I2C_WRITE(0x3302,0xff);
SENSOR_I2C_WRITE(0x3633,0x2f);
//0612
SENSOR_I2C_WRITE(0x3306,0x58);
SENSOR_I2C_WRITE(0x330b,0x6c);
SENSOR_I2C_WRITE(0x3630,0xa8);
SENSOR_I2C_WRITE(0x3622,0x02);
SENSOR_I2C_WRITE(0x3638,0x0f);
SENSOR_I2C_WRITE(0x3306,0x68);
SENSOR_I2C_WRITE(0x366e,0x08);
// ofs auto en [3]
SENSOR_I2C_WRITE(0x366f,0x2f);
// ofs+finegain  real ofs in 0x3687[4:0]
//ana dithering
//0x3364,0x15);
//[4] auto en
SENSOR_I2C_WRITE(0x3e23,0x07);
//gain set
SENSOR_I2C_WRITE(0x3e24,0x10);
SENSOR_I2C_WRITE(0x331d,0x0a);
//value
//0x3301,0xff);
//0x337f,0x23);
//[5] st2 adjsut en
SENSOR_I2C_WRITE(0x333b,0x00);
//[3:0] msb
SENSOR_I2C_WRITE(0x3357,0x5a);
//[3:0] lsb
//smear
SENSOR_I2C_WRITE(0x3309,0x68);
SENSOR_I2C_WRITE(0x331f,0x4d);
SENSOR_I2C_WRITE(0x3321,0x4f);
//0613
SENSOR_I2C_WRITE(0x3631,0x84);
//0x363c,0x26);
//bypass npump
SENSOR_I2C_WRITE(0x3038,0xff);
//light
SENSOR_I2C_WRITE(0x3213,0x00);
//0615
SENSOR_I2C_WRITE(0x391b,0x4d);
//high temp
//atuo logic
SENSOR_I2C_WRITE(0x3670,0x08);
//[3]:3633 logic ctrl  real value in 3682
SENSOR_I2C_WRITE(0x367e,0x07);
//gain0
SENSOR_I2C_WRITE(0x367f,0x0f);
//gain1
SENSOR_I2C_WRITE(0x3677,0x2f);
//<gain0
SENSOR_I2C_WRITE(0x3678,0x22);
//gain0 - gain1
SENSOR_I2C_WRITE(0x3679,0x22);
//>gain1
SENSOR_I2C_WRITE(0x337f,0x03);
//new auto precharge  330e in 3372
SENSOR_I2C_WRITE(0x3368,0x02);
SENSOR_I2C_WRITE(0x3369,0x00);
SENSOR_I2C_WRITE(0x336a,0x00);
SENSOR_I2C_WRITE(0x336b,0x00);
SENSOR_I2C_WRITE(0x3367,0x08);
SENSOR_I2C_WRITE(0x330e,0x30);
SENSOR_I2C_WRITE(0x3d08,0x03);

SENSOR_I2C_WRITE(0x3e03,0x03);
SENSOR_I2C_WRITE(0x3213,0x02);
SENSOR_I2C_WRITE(0x0100,0x01);

/**/
    bSensorInit = HI_TRUE;
    printf("======================================================================\n");
    printf("===SmartSens sc1235 sensor 960P30fps(Parallel port) init success!=====\n");
    printf("======================================================================\n");

    return;
}


HI_S32 sc1235_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
    memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	g_stSnsRegsInfo.astI2cData[0].u32Data = 0x00;
	g_stSnsRegsInfo.astI2cData[1].u32Data = 0x10;
	g_stSnsRegsInfo.astI2cData[2].u32Data = 0x00;
	g_stSnsRegsInfo.astI2cData[3].u32Data = 0x10;
	g_stSnsRegsInfo.astI2cData[4].u32Data = 0x08;
	g_stSnsRegsInfo.astI2cData[5].u32Data = 0x70;
	g_stSnsRegsInfo.astI2cData[6].u32Data = 0x03;
	g_stSnsRegsInfo.astI2cData[7].u32Data = 0xe8;
	g_stSnsRegsInfo.astI2cData[8].u32Data =  0x84;
	g_stSnsRegsInfo.astI2cData[9].u32Data =  0x04;
	g_stSnsRegsInfo.astI2cData[10].u32Data =  0x00;
	g_stSnsRegsInfo.astI2cData[11].u32Data = 0x84;
//	g_stSnsRegsInfo.astI2cData[12].u32Data = 0x0c;
	g_stSnsRegsInfo.astI2cData[12].u32Data = 0x05;
	g_stSnsRegsInfo.astI2cData[13].u32Data =  0x42;
	g_stSnsRegsInfo.astI2cData[14].u32Data =  0x30;
	g_stSnsRegsInfo.astI2cData[15].u32Data =  0x04;
	g_stSnsRegsInfo.astI2cData[16].u32Data =  0x18;
	g_stSnsRegsInfo.astI2cData[17].u32Data =  0x02;

    pstSensorExpFunc->pfn_cmos_sensor_init = sc1235_reg_init;
   // pstSensorExpFunc->pfn_cmos_sensor_exit = sensor_exit;
    pstSensorExpFunc->pfn_cmos_sensor_global_init = sc1235_global_init;
    pstSensorExpFunc->pfn_cmos_set_image_mode = sc1235_set_image_mode;
    pstSensorExpFunc->pfn_cmos_set_wdr_mode = sc1235_set_wdr_mode;
    
    pstSensorExpFunc->pfn_cmos_get_isp_default = sc1235_get_isp_default;
    pstSensorExpFunc->pfn_cmos_get_isp_black_level = sc1235_get_isp_black_level;
    pstSensorExpFunc->pfn_cmos_set_pixel_detect = sc1235_set_pixel_detect;
    pstSensorExpFunc->pfn_cmos_get_sns_reg_info = sc1235_get_sns_regs_info;

    return 0;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/

int sc1235_register_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    ISP_SENSOR_REGISTER_S stIspRegister;
    AE_SENSOR_REGISTER_S  stAeRegister;
    AWB_SENSOR_REGISTER_S stAwbRegister;

    sc1235_init_sensor_exp_function(&stIspRegister.stSnsExp);
    s32Ret = HI_MPI_ISP_SensorRegCallBack(IspDev, SC1235_ID, &stIspRegister);
    if (s32Ret)
    {
        printf("sensor register callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    sc1235_init_ae_exp_function(&stAeRegister.stSnsExp);
    s32Ret = HI_MPI_AE_SensorRegCallBack(IspDev, &stLib, SC1235_ID, &stAeRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    sc1235_init_awb_exp_function(&stAwbRegister.stSnsExp);
    s32Ret = HI_MPI_AWB_SensorRegCallBack(IspDev, &stLib, SC1235_ID, &stAwbRegister);
    if (s32Ret)
    {
        printf("sensor register callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}

int sc1235_unregister_callback(void)
{
    ISP_DEV IspDev = 0;
    HI_S32 s32Ret;
    ALG_LIB_S stLib;

    s32Ret = HI_MPI_ISP_SensorUnRegCallBack(IspDev, SC1235_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function failed!\n");
        return s32Ret;
    }
    
    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, sizeof(HI_AE_LIB_NAME));
    s32Ret = HI_MPI_AE_SensorUnRegCallBack(IspDev, &stLib, SC1235_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }

    stLib.s32Id = 0;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, sizeof(HI_AWB_LIB_NAME));
    s32Ret = HI_MPI_AWB_SensorUnRegCallBack(IspDev, &stLib, SC1235_ID);
    if (s32Ret)
    {
        printf("sensor unregister callback function to ae lib failed!\n");
        return s32Ret;
    }
    
    return 0;
}


void SmartSens_SC1235_init(SENSOR_DO_I2CRD do_i2c_read, SENSOR_DO_I2CWR do_i2c_write)//ISP_AF_REGISTER_S *pAfRegister
{
	//SENSOR_EXP_FUNC_S sensor_exp_func;

	// init i2c buf
	sensor_i2c_read = do_i2c_read;
	sensor_i2c_write = do_i2c_write;

//	ar0130_reg_init();

	sc1235_register_callback();
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
	printf("SmartSens SC1235 sensor 1080P30fps init success!\n");
		
}


	
int SC1235_get_resolution(uint32_t *ret_width, uint32_t *ret_height)
{
	if(ret_width && ret_height){
		*ret_width = SENSOR_SC1235_WIDTH;
		*ret_height = SENSOR_SC1235_HEIGHT;
		return 0;
	}
	return -1;
}

bool SENSOR_SC1235_probe()
{
	uint16_t ret_data1 = 0;
	uint16_t ret_data2 = 0;

	SENSOR_I2C_READ(0x3107, &ret_data1);
	SENSOR_I2C_READ(0x3108, &ret_data2);
	
	if(ret_data1 == SC1235_CHECK_DATA_LSB && ret_data2 == SC1235_CHECK_DATA_MSB){
		//sensor power down GPIO7_4  --- high
		sdk_sys->write_reg(0x200f00f0, 0x1); 
		sdk_sys->write_reg(0x201b0400, 0x90); 
		sdk_sys->write_reg(0x201b0040, 0x00); 
		usleep(100000) ;
		sdk_sys->write_reg(0x201b0040, 0x10); 
		
		
		//set sensor CLK
		sdk_sys->write_reg(0x200f0040, 0x2); 
		sdk_sys->write_reg(0x200f0044, 0x2);
		
		sdk_sys->write_reg(0x2003002c, 0xb4001);	 // sensor unreset, clk 27MHz, VI 99MHz
		
		return true;
	}
	return false;
}

int SC1235_get_sensor_name(char *sensor_name)
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

#endif /* __SOISC1235_CMOS_H_ */



