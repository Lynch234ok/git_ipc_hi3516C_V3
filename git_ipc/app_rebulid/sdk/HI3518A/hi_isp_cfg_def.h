
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
	uint8_t AeExpStep;
	int16_t AeExpTolerance;
	uint16_t AeHistslope[2][8];
	uint16_t AeHistOffset[2];
	uint8_t AeHistweight[2][255];
}StAeCfgAttr,*LpAeCfgAttr;

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
	uint8_t Saturation[2][8];
	uint16_t BlackLevel[2][4];
}StAwbCfgAttr, *LpAwbCfgAttr;

//example:SharpenAltD[0][] for daylight; SharpenAltD[1][] for night; so as the others
typedef struct ImpCfgAttr{
	uint8_t SharpenAltD[2][8];
	uint8_t SharpenAltUd[2][8];
	uint8_t SnrThresh[2][8];
	uint8_t DemosaicUuSlope[2][4];
	uint8_t Gamma[2];
	uint8_t SfStrength[2][8];
	uint16_t DrcThreshold[2];
	uint8_t DrcSlope[2];
	uint16_t DefectPixelSlope[4];
	uint16_t DefectPixelThresh[4];
	uint32_t ChnSp[2][4];
	uint16_t AutoSlowFrameRate;
	uint32_t SlowFrameRateHigherHold[2];
	uint32_t SlowFrameRateHighHold[2];
	uint32_t SlowFrameRateLowHold[2];
	uint32_t AntiFlikeThreshold;
}StImpCfgAttr,*LpImpCfgAttr;

typedef struct UserAttr{
	uint8_t DrcStrength;
	uint8_t SfNRStrength;
}StUserAttr, *LpUserAttr;

typedef struct IspCfgAttr{
	StAeCfgAttr aeCfgAttr;
	StAwbCfgAttr awbCfgAttr;
	StImpCfgAttr impCfgAttr;
	StUserAttr userAttr;
}StIspCfgAttr, *LpIspCfgAttr;

#ifndef ASSERT_POINT
#define ASSERT_POINT(ass_def, ass_var) \
	do { \
		if((void *)ass_def == (void *)ass_var) \
		{ \
			printf("\n\033[1;31m*** ERROR ***\033[0m\t\033[1;32m%-80s\033[0m\t\033[1;31m%-40s\033[0m\tLine->%4d\n", __FILE__, __func__, __LINE__); \
			return -1; \
		} \
	} while(0)
#endif


#ifndef ASSERT_ZERO
#define ASSERT_ZERO(ass_var) \
	do { \
		if(0 != ass_var) \
		{ \
			printf("\n\033[1;31m*** ERROR ***\033[0m\t\033[1;32m%-80s\033[0m\t\033[1;31m%-40s\033[0m\tLine->%4d\n", __FILE__, __func__, __LINE__); \
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
#define	ARRAY_NUM(_array_nums)	sizeof(_array_nums) / sizeof (_array_nums[0])
#endif

#ifdef __cplusplus
};
#endif
#endif //HI_ISP_CFG_DEF_H_

