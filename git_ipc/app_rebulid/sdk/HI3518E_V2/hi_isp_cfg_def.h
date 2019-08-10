
#ifndef HI_ISP_CFG_DEF_H_
#define HI_ISP_CFG_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>



//#define INI_DEBUG_ECHO_OF_ALL



//example:AeCompensation[0] for daylight; AeCompensation[1] for night; so as the others
typedef struct AeCfgAttr{
	uint8_t AeCompensation[2];
	uint32_t MaxAgainTarget[2];
	uint32_t MaxDgainTarget[2];
	uint32_t MaxISPDgainTarget[2];
	uint32_t MaxSystemGainTarget[2];
	uint8_t AeSpeed;
	int16_t AeTolerance;
	uint16_t AeHistslope[2][8];
	uint16_t AeHistOffset[2];	
	uint8_t AeHistweight[2][255];
} StAeCfgAttr,*LpAeCfgAttr;

//example:HighColorTemp[0] for outdoor; HighColorTemp[1] for indoor; so as the others
typedef struct AwbCfgAttr{
	uint16_t HighColorTemp[2];
	uint16_t HighCCM[2][9];
	uint16_t MidColorTemp[2];
	uint16_t MidCCM[2][9];
	uint16_t LowColorTemp[2];
	uint16_t LowCCM[2][9];
	uint16_t WbRefTemp[2];
	uint16_t StaticWB[2][4];
	int32_t AwbPara[2][6];
	uint8_t Saturation[2][16];
	uint16_t BlackLevel[2][4];
	uint32_t RedCastGain[2][8];
	uint32_t GreenCastGain[2][8];
	uint32_t BlueCastGain[2][8];
} StAwbCfgAttr, *LpAwbCfgAttr;

//example:SharpenAltD[0][] for daylight; SharpenAltD[1][] for night; so as the others
typedef struct ImpCfgAttr{
	uint8_t SharpenAltD[2][16];
	uint8_t SharpenAltUd[2][16];
	uint8_t OverShoot[2][16];
	uint8_t UnderShoot[2][16];
	uint8_t TextureNoiseThd[2][16];
	uint8_t EdgeNoiseThd[2][16];
	uint8_t EnLowLumaShoot[2][16];
	
	uint8_t SnrVarStrength[2][16];
	uint16_t SnrThreshold[2][16];
	uint16_t DemosaicUuSlope[2][4];
	uint8_t Gamma[2];
	uint16_t DrcThreshold[2];
	uint8_t DrcAutoStrength[2];
	uint8_t DrcAutoStrength_new[2][8];
	uint16_t DefectPixelSlope[16]; 
	uint16_t DefectPixelBlendRatio[16];	
	uint16_t AutoSlowFrameRate;
	uint8_t  src_framerate;
	uint8_t  flick_frequency;
	uint32_t SlowFrameRateHighHold[2];
	uint32_t SlowFrameRateLowHold[2];
	uint32_t AntiFlikeThreshold;

	uint32_t  YPKStr[2][10];	
	uint32_t  YSFStr[2][10];		/* [ 0 .. 200 ] */ 
	uint32_t  YTFStr[2][10];		/* [ 0 .. 128 ] */
	uint32_t  TFStrMax[2][10];		/* [ 0 .. 15 ] */
	uint32_t  TFStrMov[2][10];		/* [ 0 .. 31 ] */
	uint32_t  YSmthStr[2][10];		/* [ 0 .. 200 ] */
	uint32_t  YSmthRat[2][10];		/* [ 0 .. 32  ] */
	uint32_t  YSFStrDlt[2][10];		/* [ -128 .. 127 ] */ 
	uint32_t  YSFStrDl[2][10];		/* [ 0 .. 255 ] */ 
	uint32_t  YTFStrDlt[2][10];		/* [  -64 .. 63  ] */
	uint32_t  YTFStrDl[2][10];		/* [    0 .. 31  ] */
	uint32_t  YSFBriRat[2][10];		/* [ 0 .. 64 ] */
	uint32_t  CSFStr[2][10];		/* [ 0 ..  80 ] */
	uint32_t  CTFstr[2][10];		/* [ 0 ..  32 ] */
	uint32_t  YTFMdWin[2][10];      /* [ 0 .. 1 ] */

    uint32_t IrcutColorTemp[2];
    uint32_t IrcutDayToNight[2];
    uint32_t IrcutNightToDay[5];
    float    IrcutRDG[5];
    float    IrcutBDG[5];

}StImpCfgAttr,*LpImpCfgAttr;

typedef struct IspCfgAttr{
	StAeCfgAttr aeCfgAttr;
	StAwbCfgAttr awbCfgAttr;
	StImpCfgAttr impCfgAttr;
} StIspCfgAttr, *LpIspCfgAttr;


#ifndef ASSERT_NEW_POINT
#define ASSERT_NEW_POINT(ass_def, ass_var) \
	do { \
		if((void *)ass_def == (void *)ass_var) \
		{ \
			printf("[ \033[31;1m%80.80s \033[32;1m: %d\033[0m ] \033[35;1m no such para\033[0m\033[0m\n", __FILE__, __LINE__); \
			continue; \
		} \
	} while(0)
#endif




#ifdef INI_DEBUG_ECHO_OF_ALL

#ifndef ECHO_HEX_DEC
#define ECHO_HEX_DEC(ECHO_HEX_DEC_STR, ECHO_HEX_DEC_VAL)		printf("\033[32;1m%-25.25s\t\033[31;1m0x%02X \033[32;1m(%d)\033[0m\n\n", ECHO_HEX_DEC_STR, ECHO_HEX_DEC_VAL, ECHO_HEX_DEC_VAL)
#endif

#ifndef ECHO_FOR_LOOP_0
#define ECHO_FOR_LOOP_0(ECHO_HEX_DEC_STR_0, ECHO_HEX_DEC_VAL_i, ECHO_HEX_DEC_VAL) \
	do { \
		for(ECHO_HEX_DEC_VAL_i = 0; ARRAY_NUM(ECHO_HEX_DEC_VAL) > ECHO_HEX_DEC_VAL_i; ECHO_HEX_DEC_VAL_i++) \
		{ \
			printf("\033[32;1m%-25.25s[%d]\t\033[31;1m0x%02X \033[32;1m(%d)\033[0m\n", ECHO_HEX_DEC_STR_0, ECHO_HEX_DEC_VAL_i, ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i], ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i]); \
		} \
		printf("\n"); \
	} while(0)
#endif

#ifndef ECHO_FOR_LOOP_1
#define ECHO_FOR_LOOP_1(ECHO_HEX_DEC_STR_0, ECHO_HEX_DEC_VAL_i, ECHO_HEX_DEC_VAL) \
	do { \
		printf("\033[32;1m%-25.25s\033[0m", ECHO_HEX_DEC_STR_0); \
		for(ECHO_HEX_DEC_VAL_i = 0; ARRAY_NUM(ECHO_HEX_DEC_VAL) > ECHO_HEX_DEC_VAL_i; ECHO_HEX_DEC_VAL_i++) \
		{ \
			if(0 < ECHO_HEX_DEC_VAL_i && 0 == ECHO_HEX_DEC_VAL_i % 9) \
			{ \
				printf("\n\t\t\t"); \
			} \
			printf("\t\033[31;1m0x%02X \033[32;1m(%d)\033[0m", ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i], ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i]); \
		} \
		printf("\n\n"); \
	} while(0)
#endif

#ifndef ECHO_FOR_LOOP_2
#define ECHO_FOR_LOOP_2(ECHO_HEX_DEC_STR_0, ECHO_HEX_DEC_VAL_i, ECHO_HEX_DEC_VAL_j, ECHO_HEX_DEC_VAL) \
	do { \
		for(ECHO_HEX_DEC_VAL_i = 0; ARRAY_NUM(ECHO_HEX_DEC_VAL) > ECHO_HEX_DEC_VAL_i; ECHO_HEX_DEC_VAL_i++) \
		{ \
			printf("\033[32;1m%-25.25s[%d]\033[0m", ECHO_HEX_DEC_STR_0, ECHO_HEX_DEC_VAL_i); \
			for(ECHO_HEX_DEC_VAL_j = 0; ARRAY_NUM(ECHO_HEX_DEC_VAL[0]) > ECHO_HEX_DEC_VAL_j; ECHO_HEX_DEC_VAL_j++) \
			{ \
				if(0 < ECHO_HEX_DEC_VAL_j && 0 == ECHO_HEX_DEC_VAL_j % 9) \
				{ \
					printf("\n\t\t\t"); \
				} \
				printf("\t\033[31;1m0x%02X \033[32;1m(%d)\033[0m", ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i][ECHO_HEX_DEC_VAL_j], ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i][ECHO_HEX_DEC_VAL_j]); \
			} \
			printf("\n"); \
		} \
		printf("\n"); \
	} while(0)
#endif

#endif // #ifdef INI_DEBUG_ECHO_OF_ALL



#ifndef ASSERT_POINT
#define ASSERT_POINT(ass_def, ass_var) \
	do { \
		if((void *)ass_def == (void *)ass_var) \
		{ \
			printf("[ \033[31;1m%80.80s \033[32;1m: %d\033[0m ] \033[35;1mFAILED in function \033[33;1m%s\033[0m\033[0m\n", __FILE__, __LINE__, __func__); \
			return -1; \
		} \
	} while(0)
#endif

#ifndef ASSERT_ZERO
#define ASSERT_ZERO(ass_var) \
	do { \
		if(0 != ass_var) \
		{ \
			printf("[ \033[31;1m%80.80s \033[32;1m: %d\033[0m ] \033[35;1mFAILED in function \033[33;1m%s\033[0m\033[0m\n", __FILE__, __LINE__, __func__); \
			return -1; \
		} \
	} while(0)
#endif

#ifndef ASSERT_NULL
#define ASSERT_NULL(ass_var) \
	do { \
		if(NULL == ass_var) \
		{ \
			return -1; \
		} \
	} while(0)
#endif

#ifndef LINE_COUNT
#define	LINE_COUNT(_y_max,_y_min,_x_max,_x_min,_cur_x)					(int)(_y_min+(float)(_y_max-_y_min)*(float)(_cur_x-_x_min)/(_x_max-_x_min))
#endif


#ifndef ARRAY_NUM
#define	ARRAY_NUM(_array_nums)					(sizeof(_array_nums) / sizeof(_array_nums[0]))
#endif



#ifdef __cplusplus
};
#endif


#endif //#ifndef HI_ISP_CFG_DEF_H_

