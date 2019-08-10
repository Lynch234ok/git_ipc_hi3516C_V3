
#ifndef HI_ISP_CFG_DEF_H_
#define HI_ISP_CFG_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>
#include <stdbool.h>


//#define INI_DEBUG_ECHO_OF_ALL



//example:AeCompensation[0] for daylight; AeCompensation[1] for night; so as the others
typedef struct AeCfgAttr{
	uint32_t MaxExptime[2];
	bool MaxExptimeFlag;
	uint32_t LowLightMaxExptime[2];
	bool LowLightExptimeFlag;
	bool LowLightModeEnable;
	uint32_t StarMaxExptime[2];
	bool StarMaxExptimeFlag;
	bool StarLightModeEnable;
	uint32_t GainThreshold[2];
	bool GainThresholdFlag;
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
} StAwbCfgAttr, *LpAwbCfgAttr;

//example:SharpenAltD[0][] for daylight; SharpenAltD[1][] for night; so as the others
typedef struct ImpCfgAttr{
//sharpen
	uint8_t SharpenD[2][16];
	uint8_t SharpenUd[2][16];
	uint8_t OverShoot[2][16];
	uint8_t UnderShoot[2][16];
	uint8_t EdgeThr[2][16];
	uint8_t TextureThr[2][16];
	uint8_t SharpenEdge[2][16];
	uint8_t ShootSupStr[2][16];
	uint8_t DetailCtrl[2][16];	

	uint8_t EdgeFiltStr[2][16];
	uint8_t JagCtrl[2][16];
	uint8_t NoiseLumaCtrl[2][16];
//NR	

	uint8_t FineStr[2][16];
	uint16_t CoringWeight[2][16];
	
//Demosaic	
	uint16_t EdgeSmoothThr[2][16];
	uint16_t EdgeSmoothSlope[2][16];
	uint16_t AntiAliasThr[2][16];
	uint16_t AntiAliasSlope[2][16];
	uint16_t NrCoarseStr[2][16];
	uint8_t  DetailEnhanceStr[2][16];	
	uint16_t NoiseSuppressStr[2][16];
	uint16_t SharpenLumaStr[2][16];	
//	
	uint8_t Gamma[2];
	uint16_t DrcThreshold[2];	
	uint8_t DrcAutoStrength[2][16];
	uint16_t DefectPixelStrength[16]; 
	uint16_t DefectPixelBlendRatio[16];	
	uint16_t AutoSlowFrameRate;
	uint8_t  src_framerate;
	uint8_t  flick_frequency;
	uint32_t SlowFrameRateHighHold[2];
	uint32_t SlowFrameRateLowHold[2];
	uint32_t AntiFlikeThreshold;

	uint8_t  IES0[2][10];
	uint8_t  SBS0[2][10];
	uint8_t  SBS1[2][10];
	uint8_t  SBS2[2][10];
	uint8_t  SBS3[2][10];
	uint8_t  SDS0[2][10];
	uint8_t  SDS1[2][10];
	uint8_t  SDS2[2][10];
	uint8_t  SDS3[2][10];
	uint8_t  STH0[2][10];
	uint8_t  STH1[2][10];
	uint8_t  STH2[2][10];
	uint8_t  STH3[2][10];

	uint8_t  MDP[2][10];
	uint8_t  MATH1[2][10];
	uint8_t  MATH2[2][10];
	uint8_t  Pro3[2][10];
	uint8_t  MDDZ1[2][10];
	uint8_t  MDDZ2[2][10];
	uint8_t  TFS1[2][10];
	uint8_t  TFS2[2][10];

	uint8_t  SFC[2][10];
	uint8_t  TFC[2][10];
	uint8_t  TPC[2][10];
	uint8_t  TRC[2][10];

	uint32_t DaylightToNight;
	float    NightToDaylight[8];
	uint32_t ColortoblackVal[9];
}StImpCfgAttr,*LpImpCfgAttr;

typedef struct IspCfgAttr{
	StAeCfgAttr aeCfgAttr;
	StAwbCfgAttr awbCfgAttr;
	StImpCfgAttr impCfgAttr;
} StIspCfgAttr, *LpIspCfgAttr;



#ifdef INI_DEBUG_ECHO_OF_ALL

#ifndef ECHO_HEX_DEC
#define ECHO_HEX_DEC(ECHO_HEX_DEC_STR, ECHO_HEX_DEC_VAL)		printf("\033[32;1m%-25.25s\t\033[31;1m0x%02X \033[32;1m(%d)\033[0m\n\n", ECHO_HEX_DEC_STR, ECHO_HEX_DEC_VAL, ECHO_HEX_DEC_VAL)
#endif

#ifndef ECHO_FOR_LOOP_0
#define ECHO_FOR_LOOP_0(ECHO_HEX_DEC_STR_0, ECHO_HEX_DEC_VAL_i, ECHO_HEX_DEC_VAL) \
	do { \
		for(ECHO_HEX_DEC_VAL_i = 0; ARRAY_NUM(ECHO_HEX_DEC_VAL) > ECHO_HEX_DEC_VAL_i; ECHO_HEX_DEC_VAL_i++) \
		{ \
			printf("\033[32;1m%-25.25s[%d]\t\033[31;1m0x%02X \033[32;1m(%u)\033[0m\n", ECHO_HEX_DEC_STR_0, ECHO_HEX_DEC_VAL_i, ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i], ECHO_HEX_DEC_VAL[ECHO_HEX_DEC_VAL_i]); \
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

#ifndef ARRAY_NUM
#define	ARRAY_NUM(_array_nums)					(sizeof(_array_nums) / sizeof(_array_nums[0]))
#endif

#ifndef LINE_COUNT
#define	LINE_COUNT(_y_max,_y_min,_x_max,_x_min,_cur_x)	(int)(_y_min+(float)(_y_max-_y_min)*(float)(_cur_x-_x_min)/(_x_max-_x_min))
#endif


#ifdef __cplusplus
};
#endif


#endif //#ifndef HI_ISP_CFG_DEF_H_

