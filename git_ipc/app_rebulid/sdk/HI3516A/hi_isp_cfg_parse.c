#include "hi_isp_cfg_def.h"
#include "hi_isp_cfg_parse.h"
#include "inifile.h"
#include "hi3516a.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ispCfgIniParse{
	int (*ispCfgIniRead)(const char *filepath, void *args, size_t size);
	int (*ispCfgIniWrite)(const char *filepath, void *args, size_t size);
}StIspCfgIniParse, *LpispCfgIniParse;



static LpispCfgIniParse _ispIniParse = NULL;



static const char *default_str = "NotFound";	// It is used for handle->read_text()
static const char separator = '|';		// Segmentation symbols



/**
 * Count number of array
 * @param[in]	str:			Src string
 * @param[in]	separator:		separator from Src string
 * @return				Return count number
 */
static size_t
count_separator(INI_CSTR_t str)
{
	size_t i = 0, sum = 0;
	for(i = 0; 0 != str[i]; i++)
	{
		if(separator == str[i])
		{
			sum++;
		}
	}
	return sum;
}



/**
 * Converts string to array
 * @param[in]	str:			Src string
 * @param[in]	arg:			Dest array
 * @param[in]	size:			Size of the dest array
 * @param[in]	nnum:			number of the dest array
 * @return	Return status
 * @retval		0:			Success
 * @retval		-1:			Failure
 */
static int
str2array(INI_CSTR_t str, INI_BIN_t arg, size_t size, size_t nnum)
{
	int i = 0;				// Temporary variable for for()

	char *offset = NULL;			// It is used for first separator's address on src string
	char buf[32] = {0};			// Temporary buffer for strtol()

	if(NULL == str || NULL == arg || 0 >= size || 0 >= nnum || size < nnum || nnum != count_separator(str))
	{
#ifdef INI_DEBUG_ECHO_OF_ALL
		ASSERT_ZERO(1);
#endif
		return -1;
	}

	// TODO: Cycle to extract data from the string
	for(i = 0; i < nnum; i++, str = offset + 1)
	{
		// TODO: Empty buffer
		bzero(buf, sizeof(buf));

		// TODO: Get substring
		offset = strchr(str, separator);	// Get first separator's address

		// TODO: return when error
		if(NULL == offset || offset < str)
		{
#ifdef INI_DEBUG_ECHO_OF_ALL
			ASSERT_ZERO(1);
#endif
			return -1;
		}

		// TODO: copy str to buffer
		strncpy(buf, str, offset - str);	// Copy valid data to buffer, size is offset - str

		// TODO: Converts substring to number
		switch(size / nnum)
		{
		case 1:
			((unsigned char *)arg)[i] = (unsigned char)strtol(buf, NULL, 0);
			continue;
		case 2:
			((unsigned short int *)arg)[i] = (unsigned short int)strtol(buf, NULL, 0);
			continue;
		case 4:
			((unsigned int *)arg)[i] = (unsigned int)strtol(buf, NULL, 0);
			continue;
		}
	}
	return 0;	// Return success to caller
}



/**
 * Parser AeCfgAttr
 * @param[in]	handle:		handle interface
 * @param[in]	obj:		Object of struct AeCfgAttr
 * @return	Return status
 * @retval		true:		Success
 * @retval		false:		Failure
 */
static int
parser_AeCfgAttr(lpINI_PARSER handle, LpAeCfgAttr obj)
{
	int i = 0,j = 0,ret = 0;				// Temporary variable for for()
	int column = 4*AE_ZONE_COLUMN;

	char buf[1024] = {0};			// Buffer
	char tag_buf[32] = {0};		// Tag buffer

	for(i = 0; i < ARRAY_NUM(obj->AeCompensation); i++)
	{
		snprintf(tag_buf, sizeof(tag_buf), "AeCompensation_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf)));
		obj->AeCompensation[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "MaxAgainTarget_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf)));
		obj->MaxAgainTarget[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "MaxDgainTarget_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf)));
		obj->MaxDgainTarget[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "MaxISPDgainTarget_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf)));
		obj->MaxISPDgainTarget[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "MaxSystemGainTarget_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf)));
		obj->MaxSystemGainTarget[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "AeHistslope_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->AeHistslope[i], sizeof(obj->AeHistslope[i]), ARRAY_NUM(obj->AeHistslope[i])));

		snprintf(tag_buf, sizeof(tag_buf), "AeHistOffset_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf)));
		obj->AeHistOffset[i] = strtol(buf, NULL, 0);

		memset(obj->AeHistweight[i],0,sizeof(obj->AeHistweight[i]));
		for(j=0; (j<AE_ZONE_ROW) && (ret ==0) ;j++){
			snprintf(tag_buf, sizeof(tag_buf), "AeHistweight_%d_%d", i,j);
			ret = default_str == handle->read_text(handle, "AE", tag_buf, default_str, buf+j*column, sizeof(buf)-j*column )? -1:0;
		}
		if(ret == 0){
			ASSERT_ZERO(str2array(buf, obj->AeHistweight[i], sizeof(obj->AeHistweight[i]), ARRAY_NUM(obj->AeHistweight[i])));
		}
	}

	ASSERT_POINT(default_str, handle->read_text(handle, "AE", "AeSpeed", default_str, buf, sizeof(buf)));
	obj->AeSpeed= strtol(buf, NULL, 0);

	ASSERT_POINT(default_str, handle->read_text(handle, "AE", "AeTolerance", default_str, buf, sizeof(buf)));
	obj->AeTolerance = strtol(buf, NULL, 0);

	return 0;
}



/**
 * Parser AwbCfgAttr
 * @param[in]	handle:		handle interface
 * @param[in]	obj:		Object of struct AwbCfgAttr
 * @return	Return status
 * @retval		true:		Success
 * @retval		false:		Failure
 */
static int
parser_AwbCfgAttr(lpINI_PARSER handle, LpAwbCfgAttr obj)
{
	int i = 0;				// Temporary variable for for()

	char buf[1024] = {0};			// Buffer
	char tag_buf[32] = {0};		// Tag buffer

	for(i = 0; i < ARRAY_NUM(obj->HighColorTemp); i++)
	{
		snprintf(tag_buf, sizeof(tag_buf), "HighColorTemp_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		obj->HighColorTemp[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "HighCCM_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->HighCCM[i], sizeof(obj->HighCCM[i]), ARRAY_NUM(obj->HighCCM[i])));

		snprintf(tag_buf, sizeof(tag_buf), "MidColorTemp_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		obj->MidColorTemp[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "MidCCM_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->MidCCM[i], sizeof(obj->MidCCM[i]), ARRAY_NUM(obj->MidCCM[i])));

		snprintf(tag_buf, sizeof(tag_buf), "LowColorTemp_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		obj->LowColorTemp[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "LowCCM_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->LowCCM[i], sizeof(obj->LowCCM[i]), ARRAY_NUM(obj->LowCCM[i])));

		snprintf(tag_buf, sizeof(tag_buf), "WbRefTemp_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		obj->WbRefTemp[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "GainOffset_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->StaticWB[i], sizeof(obj->StaticWB[i]), ARRAY_NUM(obj->StaticWB[i])));

		snprintf(tag_buf, sizeof(tag_buf), "AwbPara_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->AwbPara[i], sizeof(obj->AwbPara[i]), ARRAY_NUM(obj->AwbPara[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Saturation_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Saturation[i], sizeof(obj->Saturation[i]), ARRAY_NUM(obj->Saturation[i])));

		snprintf(tag_buf, sizeof(tag_buf), "BlackLevel_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->BlackLevel[i], sizeof(obj->BlackLevel[i]), ARRAY_NUM(obj->BlackLevel[i])));
	}

	return 0;
}



/**
 * Parser ImpCfgAttr
 * @param[in]	handle:		handle interface
 * @param[in]	obj:		Object of struct ImpCfgAttr
 * @return	Return status
 * @retval		true:		Success
 * @retval		false:		Failure
 */
static int
parser_ImpCfgAttr(lpINI_PARSER handle, LpImpCfgAttr obj)
{
	int i = 0;				// Temporary variable for for()

	char buf[1024] = {0};			// Buffer
	char tag_buf[32] = {0};		// Tag buffer

	for(i = 0; i < ARRAY_NUM(obj->Gamma); i++)
	{
		snprintf(tag_buf, sizeof(tag_buf), "SharpenAltD_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SharpenAltD[i], sizeof(obj->SharpenAltD[i]), ARRAY_NUM(obj->SharpenAltD[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SharpenAltUd_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SharpenAltUd[i], sizeof(obj->SharpenAltUd[i]), ARRAY_NUM(obj->SharpenAltUd[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SharpenRGB_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SharpenRGB[i], sizeof(obj->SharpenRGB[i]), ARRAY_NUM(obj->SharpenRGB[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SnrThresh_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SnrThresh[i], sizeof(obj->SnrThresh[i]), ARRAY_NUM(obj->SnrThresh[i])));

		snprintf(tag_buf, sizeof(tag_buf), "DemosaicUuSlope_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->DemosaicUuSlope[i], sizeof(obj->DemosaicUuSlope[i]), ARRAY_NUM(obj->DemosaicUuSlope[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Gamma_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->Gamma[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "GlobalStrength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->GlobalStrength[i], sizeof(obj->GlobalStrength[i]), ARRAY_NUM(obj->GlobalStrength[i])));
//DRC
		snprintf(tag_buf, sizeof(tag_buf), "DrcThreshold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcThreshold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "DrcAutoStrength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcAutoStrength[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "DrcSlopeMax_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcSlopeMax[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "DrcSlopeMin_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcSlopeMin[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "DrcWhiteLevel_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcWhiteLevel[i] = strtol(buf, NULL, 0);
/////
		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateHighHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateHighHold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateLowHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateLowHold[i] = strtol(buf, NULL, 0);
		
//vppsV2
		snprintf(tag_buf, sizeof(tag_buf), "Chroma_SF_Strength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Chroma_SF_Strength[i], sizeof(obj->Chroma_SF_Strength[i]), ARRAY_NUM(obj->Chroma_SF_Strength[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Chroma_TF_Strength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Chroma_TF_Strength[i], sizeof(obj->Chroma_TF_Strength[i]), ARRAY_NUM(obj->Chroma_TF_Strength[i])));

		snprintf(tag_buf, sizeof(tag_buf), "IE_Strength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->IE_Strength[i], sizeof(obj->IE_Strength[i]), ARRAY_NUM(obj->IE_Strength[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Luma_MotionThresh_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Luma_MotionThresh[i], sizeof(obj->Luma_MotionThresh[i]), ARRAY_NUM(obj->Luma_MotionThresh[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Luma_SF_MoveArea_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Luma_SF_MoveArea[i], sizeof(obj->Luma_SF_MoveArea[i]), ARRAY_NUM(obj->Luma_SF_MoveArea[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Luma_SF_StillArea_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Luma_SF_StillArea[i], sizeof(obj->Luma_SF_StillArea[i]), ARRAY_NUM(obj->Luma_SF_StillArea[i])));
		
		snprintf(tag_buf, sizeof(tag_buf), "Luma_TF_Strength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Luma_TF_Strength[i], sizeof(obj->Luma_TF_Strength[i]), ARRAY_NUM(obj->Luma_TF_Strength[i])));
		
		snprintf(tag_buf, sizeof(tag_buf), "DeSand_Strength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->DeSand_Strength[i], sizeof(obj->DeSand_Strength[i]), ARRAY_NUM(obj->DeSand_Strength[i])));
		
		
	}

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelSlope", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelSlope, sizeof(obj->DefectPixelSlope), ARRAY_NUM(obj->DefectPixelSlope)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelThresh", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelThresh, sizeof(obj->DefectPixelThresh), ARRAY_NUM(obj->DefectPixelThresh)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "AntiFlikeThreshold", default_str, buf, sizeof(buf)));
	obj->AntiFlikeThreshold = strtol(buf, NULL, 0);

	return 0;
}

static int
_hi3516_isp_cfg_ini_read(const char *filepath, void *args, size_t size)
{
	int retval = -1;
	LpIspCfgAttr ispCfgAttr = (LpIspCfgAttr)args;

	if(sizeof(StIspCfgAttr) == size)
	{
		lpINI_PARSER handle = NULL;
		handle = OpenIniFile(filepath);
		if(NULL != handle)
		{
			retval = 0 == parser_AeCfgAttr(handle, &(ispCfgAttr->aeCfgAttr)) && 0 == parser_AwbCfgAttr(handle, &(ispCfgAttr->awbCfgAttr)) && 0 == parser_ImpCfgAttr(handle, &(ispCfgAttr->impCfgAttr)) ? 0 : -1;
			CloseIniFile(handle);
			handle = NULL;
		}
	}
	return retval;
}

static int
_hi3516_isp_cfg_ini_write(const char *filepath, void *args, size_t size)
{
	int retval = -1;
	if(sizeof(StIspCfgAttr) == size)
	{
	}
	return retval;
}

int
HI_ISP_cfg_ini_parse_init(void)
{
#if defined(HI3518E) | defined(HI3518A) | defined(HI3518C) | defined(HI3516C) | defined(HI3516A) | defined(HI3516D)
	if(NULL == _ispIniParse)
	{
		_ispIniParse = calloc(sizeof(StIspCfgIniParse), 1);
		ASSERT_NULL(_ispIniParse);
		_ispIniParse->ispCfgIniRead = _hi3516_isp_cfg_ini_read;
		_ispIniParse->ispCfgIniWrite = _hi3516_isp_cfg_ini_write;
	}
	return 0;
#else
	return -1;
#endif
}

int
HI_ISP_cfg_ini_parse_destroy(void)
{
	if(NULL != _ispIniParse)
	{
		free(_ispIniParse);
		_ispIniParse = NULL;
	}
	return 0;
}

//return 0:sucess  ;    -1:failed
int
HI_ISP_cfg_ini_load(const char *filepath, void *args, size_t size)
{
	if(NULL != filepath && NULL != _ispIniParse && NULL != _ispIniParse->ispCfgIniRead)
	{
		return _ispIniParse->ispCfgIniRead(filepath, args, size);
	}
	return -1;
}

//return 0:sucess  ;    -1:failed
int
HI_ISP_cfg_ini_save(const char *filepath, void *args, size_t size)
{
	if(filepath && _ispIniParse && _ispIniParse->ispCfgIniWrite)
	{
		return _ispIniParse->ispCfgIniWrite(filepath, args, size);
	}
	return -1;
}

