#include "hi_isp_cfg_def.h"
#include "hi_isp_cfg_parse.h"
#include "inifile.h"
#include "hi3518e.h"

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


static int
str2array_float(INI_CSTR_t str, INI_BIN_t arg, size_t size, size_t nnum)
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
			((float *)arg)[i] = (float)strtof(buf, NULL);
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

		memset(buf, 0, sizeof(buf));
		snprintf(tag_buf, sizeof(tag_buf), "RedCastGain_%d", i);
		if(default_str != handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)))
		{
			ASSERT_ZERO(str2array(buf, obj->RedCastGain[i], sizeof(obj->RedCastGain[i]), ARRAY_NUM(obj->RedCastGain[i])));
		}
			
		memset(buf, 0, sizeof(buf));
		snprintf(tag_buf, sizeof(tag_buf), "GreenCastGain_%d", i);
		if(default_str != handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)))
		{
			ASSERT_ZERO(str2array(buf, obj->GreenCastGain[i], sizeof(obj->GreenCastGain[i]), ARRAY_NUM(obj->GreenCastGain[i])));
		}

		memset(buf,0, sizeof(buf));	
		snprintf(tag_buf, sizeof(tag_buf), "BlueCastGain_%d", i);
		if(default_str != handle->read_text(handle, "AWB", tag_buf, default_str, buf, sizeof(buf)))
		{
			ASSERT_ZERO(str2array(buf, obj->BlueCastGain[i], sizeof(obj->BlueCastGain[i]), ARRAY_NUM(obj->BlueCastGain[i])));
		}
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
	int ret = -1;

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

		snprintf(tag_buf, sizeof(tag_buf), "OverShoot_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->OverShoot[i], sizeof(obj->OverShoot[i]), ARRAY_NUM(obj->OverShoot[i])));

		snprintf(tag_buf, sizeof(tag_buf), "UnderShoot_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->UnderShoot[i], sizeof(obj->UnderShoot[i]), ARRAY_NUM(obj->UnderShoot[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TextureNoiseThd_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TextureNoiseThd[i], sizeof(obj->TextureNoiseThd[i]), ARRAY_NUM(obj->TextureNoiseThd[i])));

		snprintf(tag_buf, sizeof(tag_buf), "EdgeNoiseThd_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->EdgeNoiseThd[i], sizeof(obj->EdgeNoiseThd[i]), ARRAY_NUM(obj->EdgeNoiseThd[i])));

		snprintf(tag_buf, sizeof(tag_buf), "EnLowLumaShoot_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->EnLowLumaShoot[i], sizeof(obj->EnLowLumaShoot[i]), ARRAY_NUM(obj->EnLowLumaShoot[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SnrVarStrength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SnrVarStrength[i], sizeof(obj->SnrVarStrength[i]), ARRAY_NUM(obj->SnrVarStrength[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SnrThreshold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SnrThreshold[i], sizeof(obj->SnrThreshold[i]), ARRAY_NUM(obj->SnrThreshold[i])));

		snprintf(tag_buf, sizeof(tag_buf), "DemosaicUuSlope_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->DemosaicUuSlope[i], sizeof(obj->DemosaicUuSlope[i]), ARRAY_NUM(obj->DemosaicUuSlope[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Gamma_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->Gamma[i] = strtol(buf, NULL, 0);
//DRC
		snprintf(tag_buf, sizeof(tag_buf), "DrcThreshold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcThreshold[i] = strtol(buf, NULL, 0);

		

		memset(buf, 0, sizeof(buf));
		snprintf(tag_buf, sizeof(tag_buf), "DrcAutoStrength_new_%d", i);
		if(default_str != handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)))
		{
			ASSERT_ZERO(str2array(buf, obj->DrcAutoStrength_new[i], sizeof(obj->DrcAutoStrength_new[i]), ARRAY_NUM(obj->DrcAutoStrength_new[i])));
		}
		
			snprintf(tag_buf, sizeof(tag_buf), "DrcAutoStrength_%d", i);
			ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
			obj->DrcAutoStrength[i] = strtol(buf, NULL, 0);
		

		
/////
		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateHighHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateHighHold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateLowHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateLowHold[i] = strtol(buf, NULL, 0);
		
//vppsV1
		snprintf(tag_buf, sizeof(tag_buf), "YPKStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YPKStr[i], sizeof(obj->YPKStr[i]), ARRAY_NUM(obj->YPKStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YSFStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YSFStr[i], sizeof(obj->YSFStr[i]), ARRAY_NUM(obj->YSFStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YTFStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YTFStr[i], sizeof(obj->YTFStr[i]), ARRAY_NUM(obj->YTFStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TFStrMax_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TFStrMax[i], sizeof(obj->TFStrMax[i]), ARRAY_NUM(obj->TFStrMax[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TFStrMov_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TFStrMov[i], sizeof(obj->TFStrMov[i]), ARRAY_NUM(obj->TFStrMov[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YSmthStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YSmthStr[i], sizeof(obj->YSmthStr[i]), ARRAY_NUM(obj->YSmthStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YSmthRat_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YSmthRat[i], sizeof(obj->YSmthRat[i]), ARRAY_NUM(obj->YSmthRat[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YSFStrDlt_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YSFStrDlt[i], sizeof(obj->YSFStrDlt[i]), ARRAY_NUM(obj->YSFStrDlt[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YSFStrDl_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YSFStrDl[i], sizeof(obj->YSFStrDl[i]), ARRAY_NUM(obj->YSFStrDl[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YTFStrDlt_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YTFStrDlt[i], sizeof(obj->YTFStrDlt[i]), ARRAY_NUM(obj->YTFStrDlt[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YTFStrDl_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YTFStrDl[i], sizeof(obj->YTFStrDl[i]), ARRAY_NUM(obj->YTFStrDl[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YSFBriRat_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YSFBriRat[i], sizeof(obj->YSFBriRat[i]), ARRAY_NUM(obj->YSFBriRat[i])));

		snprintf(tag_buf, sizeof(tag_buf), "CSFStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->CSFStr[i], sizeof(obj->CSFStr[i]), ARRAY_NUM(obj->CSFStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "CTFstr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->CTFstr[i], sizeof(obj->CTFstr[i]), ARRAY_NUM(obj->CTFstr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "YTFMdWin_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->YTFMdWin[i], sizeof(obj->YTFMdWin[i]), ARRAY_NUM(obj->YTFMdWin[i])));
	
	}


	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelSlope", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelSlope, sizeof(obj->DefectPixelSlope), ARRAY_NUM(obj->DefectPixelSlope)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelBlendRatio", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelBlendRatio, sizeof(obj->DefectPixelBlendRatio), ARRAY_NUM(obj->DefectPixelBlendRatio)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "AntiFlikeThreshold", default_str, buf, sizeof(buf)));
	obj->AntiFlikeThreshold = strtol(buf, NULL, 0);

	//FOR IRCUT
	if(default_str != handle->read_text(handle, "IMP", "IrcutColorTemp", default_str, buf, sizeof(buf))){
		ret = str2array(buf, obj->IrcutColorTemp, sizeof(obj->IrcutColorTemp), ARRAY_NUM(obj->IrcutColorTemp));
	}
	if(0 != ret){
			unsigned int ircut_colortemp_array[2] = {2650, 3680};
			memcpy(obj->IrcutColorTemp,ircut_colortemp_array,sizeof(obj->IrcutColorTemp));
		}
	ret = -1;

	if(default_str != handle->read_text(handle, "IMP", "IrcutD2N", default_str, buf, sizeof(buf))){
		ret = str2array(buf, obj->IrcutDayToNight, sizeof(obj->IrcutDayToNight), ARRAY_NUM(obj->IrcutDayToNight));
	}
	if(0 != ret){
			unsigned int ircut_daytonight_array[2] = {29, 0};
			memcpy(obj->IrcutDayToNight,ircut_daytonight_array,sizeof(obj->IrcutDayToNight));
	}
	ret = -1;

	if(default_str != handle->read_text(handle, "IMP", "IrcutN2D", default_str, buf, sizeof(buf))){
		ret = str2array(buf, obj->IrcutNightToDay, sizeof(obj->IrcutNightToDay), ARRAY_NUM(obj->IrcutNightToDay));
	}
	if(0 != ret){
			unsigned int ircut_nighttoday_array[5] = {6, 2, 4, 4, 5};
			memcpy(obj->IrcutNightToDay,ircut_nighttoday_array,sizeof(obj->IrcutNightToDay));
	}
	ret = -1;

	if(default_str !=  handle->read_text(handle, "IMP", "IrcutRDG", default_str, buf, sizeof(buf))){
		ret = str2array_float(buf, obj->IrcutRDG, sizeof(obj->IrcutRDG), ARRAY_NUM(obj->IrcutRDG));
	}
	if(0 != ret){
		float ircut_rdg_array[5] = {0.936, 0.968, 0.980, 0.860, 0.970};
		memcpy(obj->IrcutRDG,ircut_rdg_array,sizeof(obj->IrcutRDG));
	}
	ret = -1;
	
	if(default_str !=  handle->read_text(handle, "IMP", "IrcutBDG", default_str, buf, sizeof(buf))){
		ret = str2array_float(buf, obj->IrcutBDG, sizeof(obj->IrcutBDG), ARRAY_NUM(obj->IrcutBDG));
	}
	if(0 != ret){
		float ircut_bdg_array[5] = {0.908, 0.965, 0.960, 0.890, 0.913};
		memcpy(obj->IrcutBDG,ircut_bdg_array,sizeof(obj->IrcutBDG));
	}

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
	if(NULL == _ispIniParse)
	{
		_ispIniParse = calloc(sizeof(StIspCfgIniParse), 1);
		ASSERT_NULL(_ispIniParse);
		_ispIniParse->ispCfgIniRead = _hi3516_isp_cfg_ini_read;
		_ispIniParse->ispCfgIniWrite = _hi3516_isp_cfg_ini_write;
	}
	return 0;
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

