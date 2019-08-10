#include "hi_isp_cfg_def.h"
#include "hi_isp_cfg_parse.h"
#include "inifile.h"
#include "hi3518a.h"

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
		printf("\n\033[1;31m*** ERROR ***\033[0m\t\033[1;32m%-80s\033[0m\t\033[1;31m%-40s\033[0m\tLine->%4d\n", __FILE__, __func__, __LINE__);
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
			printf("\n\033[1;31m*** ERROR ***\033[0m\t\033[1;32m%-80s\033[0m\t\033[1;31m%-40s\033[0m\tLine->%4d\n", __FILE__, __func__, __LINE__);
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
	int i = 0,j = 0;	// Temporary variable for for()
	int column = 4*WEIGHT_ZONE_COLUMN;
	int ret = 0;
	 

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
		for(j=0; (j<WEIGHT_ZONE_ROW) && (ret ==0) ;j++){
			snprintf(tag_buf, sizeof(tag_buf), "AeHistweight_%d_%d", i,j);
			ret = default_str == handle->read_text(handle, "AE", tag_buf, default_str, buf+j*column, sizeof(buf)-j*column )? -1:0;
		}
		if(ret == 0){
			ASSERT_ZERO(str2array(buf, obj->AeHistweight[i], sizeof(obj->AeHistweight[i]), ARRAY_NUM(obj->AeHistweight[i])));
		}
		

	}

	ASSERT_POINT(default_str, handle->read_text(handle, "AE", "AeExpStep", default_str, buf, sizeof(buf)));
	obj->AeExpStep = strtol(buf, NULL, 0);

	ASSERT_POINT(default_str, handle->read_text(handle, "AE", "AeExpTolerance", default_str, buf, sizeof(buf)));
	obj->AeExpTolerance = strtol(buf, NULL, 0);

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

		snprintf(tag_buf, sizeof(tag_buf), "SnrThresh_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SnrThresh[i], sizeof(obj->SnrThresh[i]), ARRAY_NUM(obj->SnrThresh[i])));

		snprintf(tag_buf, sizeof(tag_buf), "DemosaicUuSlope_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->DemosaicUuSlope[i], sizeof(obj->DemosaicUuSlope[i]), ARRAY_NUM(obj->DemosaicUuSlope[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Gamma_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->Gamma[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "SfStrength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SfStrength[i], sizeof(obj->SfStrength[i]), ARRAY_NUM(obj->SfStrength[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SfStrength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SfStrength[i], sizeof(obj->SfStrength[i]), ARRAY_NUM(obj->SfStrength[i])));

		snprintf(tag_buf, sizeof(tag_buf), "DrcThreshold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcThreshold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "DrcSlope_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcSlope[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "ChnSp_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->ChnSp[i], sizeof(obj->ChnSp[i]), ARRAY_NUM(obj->ChnSp[i])));

		
		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateHigherHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateHigherHold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateHighHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateHighHold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateLowHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateLowHold[i] = strtol(buf, NULL, 0);
	}

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelSlope", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelSlope, sizeof(obj->DefectPixelSlope), ARRAY_NUM(obj->DefectPixelSlope)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelThresh", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelThresh, sizeof(obj->DefectPixelThresh), ARRAY_NUM(obj->DefectPixelThresh)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "AntiFlikeThreshold", default_str, buf, sizeof(buf)));
	obj->AntiFlikeThreshold = strtol(buf, NULL, 0);

	return 0;
}



#if defined(HI3518E) | defined(HI3518A) | defined(HI3518C) | defined(HI3516C)
static int _hi3518_isp_cfg_ini_read(const char *filepath, void *args, size_t size)
{
	int retval = -1;
	LpIspCfgAttr ispCfgAttr = (LpIspCfgAttr)args;

	if(sizeof(StIspCfgAttr) == size)
	{
		// TODO: ini parser

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

static int _hi3518_isp_cfg_ini_write(const char *filepath, void *args, size_t size)
{
	if(sizeof(StIspCfgAttr) == size)
	{
		// TODO: ini parser

		//lpINI_PARSER handle = NULL;
	}
	return -1;
}
#endif

int HI_ISP_cfg_ini_parse_init(void)
{
	if(NULL == _ispIniParse)
	{
#if defined(HI3518E) | defined(HI3518A) | defined(HI3518C) | defined(HI3516C)
		_ispIniParse = calloc(sizeof(StIspCfgIniParse), 1);
		ASSERT_NULL(_ispIniParse);
		_ispIniParse->ispCfgIniRead = _hi3518_isp_cfg_ini_read;
		_ispIniParse->ispCfgIniWrite = _hi3518_isp_cfg_ini_write;
#endif
	}
	return 0;
}

int HI_ISP_cfg_ini_parse_destroy()
{
	if(NULL != _ispIniParse)
	{
		free(_ispIniParse);
		_ispIniParse = NULL;
	}
	return 0;
}

//return 0:sucess  ;    -1:failed
int HI_ISP_cfg_ini_load(const char *filepath, void *args, size_t size)
{
	if(NULL != filepath && NULL != _ispIniParse && NULL != _ispIniParse->ispCfgIniRead)
	{
		return _ispIniParse->ispCfgIniRead(filepath, args, size);
	}
	return -1;
}

//return 0:sucess  ;    -1:failed
int HI_ISP_cfg_ini_save(const char *filepath, void* args, size_t size)
{
	if(filepath && _ispIniParse && _ispIniParse->ispCfgIniWrite)
	{
		return _ispIniParse->ispCfgIniWrite(filepath, args, size);
	}
	return -1;
}

