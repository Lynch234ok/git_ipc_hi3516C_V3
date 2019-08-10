#include "hi_isp_cfg_def.h"
#include "hi_isp_cfg_parse.h"
#include "inifile.h"
#include "hi3516c.h"

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

		snprintf(tag_buf, sizeof(tag_buf), "MaxExptime_%d", i);
		if(default_str != handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf))){
			obj->MaxExptime[i]  = strtol(buf, NULL, 0);
			obj->MaxExptimeFlag = true;
		}else{
			obj->MaxExptimeFlag = false;
		}

		snprintf(tag_buf, sizeof(tag_buf), "LowLightMaxExptime_%d", i);
		if(default_str != handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf))){
			obj->LowLightMaxExptime[i]  = strtol(buf, NULL, 0);
			obj->LowLightExptimeFlag = true;
		}else{
			obj->LowLightExptimeFlag = false;
		}
		
		snprintf(tag_buf, sizeof(tag_buf), "StarMaxExptime_%d", i);
		if(default_str != handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf))){
			obj->StarMaxExptime[i] = strtol(buf, NULL, 0);
			obj->StarMaxExptimeFlag = true;
		}else{
			obj->StarMaxExptimeFlag = false;
		}
		
		snprintf(tag_buf, sizeof(tag_buf), "GainThreshold_%d", i);
		if(default_str != handle->read_text(handle, "AE", tag_buf, default_str, buf, sizeof(buf))){
			obj->GainThreshold[i]  = strtol(buf, NULL, 0);
			obj->GainThresholdFlag = true;
		}else{
			obj->GainThresholdFlag = false;
		}

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
	int ret = -1;

	char buf[1024] = {0};			// Buffer
	char tag_buf[32] = {0};		// Tag buffer

	for(i = 0; i < ARRAY_NUM(obj->Gamma); i++)
	{
//sharpen
		snprintf(tag_buf, sizeof(tag_buf), "SharpenD_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SharpenD[i], sizeof(obj->SharpenD[i]), ARRAY_NUM(obj->SharpenD[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SharpenUd_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SharpenUd[i], sizeof(obj->SharpenUd[i]), ARRAY_NUM(obj->SharpenUd[i])));

		snprintf(tag_buf, sizeof(tag_buf), "OverShoot_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->OverShoot[i], sizeof(obj->OverShoot[i]), ARRAY_NUM(obj->OverShoot[i])));

		snprintf(tag_buf, sizeof(tag_buf), "UnderShoot_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->UnderShoot[i], sizeof(obj->UnderShoot[i]), ARRAY_NUM(obj->UnderShoot[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TextureThr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TextureThr[i], sizeof(obj->TextureThr[i]), ARRAY_NUM(obj->TextureThr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "EdgeThr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->EdgeThr[i], sizeof(obj->EdgeThr[i]), ARRAY_NUM(obj->EdgeThr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SharpenEdge_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SharpenEdge[i], sizeof(obj->SharpenEdge[i]), ARRAY_NUM(obj->SharpenEdge[i])));

		snprintf(tag_buf, sizeof(tag_buf), "ShootSupStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->ShootSupStr[i], sizeof(obj->ShootSupStr[i]), ARRAY_NUM(obj->ShootSupStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "DetailCtrl_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->DetailCtrl[i], sizeof(obj->DetailCtrl[i]), ARRAY_NUM(obj->DetailCtrl[i])));

		snprintf(tag_buf, sizeof(tag_buf), "EdgeFiltStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->EdgeFiltStr[i], sizeof(obj->EdgeFiltStr[i]), ARRAY_NUM(obj->EdgeFiltStr[i])));
		snprintf(tag_buf, sizeof(tag_buf), "JagCtrl_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->JagCtrl[i], sizeof(obj->JagCtrl[i]), ARRAY_NUM(obj->JagCtrl[i])));
		snprintf(tag_buf, sizeof(tag_buf), "NoiseLumaCtrl_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->NoiseLumaCtrl[i], sizeof(obj->NoiseLumaCtrl[i]), ARRAY_NUM(obj->NoiseLumaCtrl[i])));
//NR
		snprintf(tag_buf, sizeof(tag_buf), "FineStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->FineStr[i], sizeof(obj->FineStr[i]), ARRAY_NUM(obj->FineStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "CoringWeight_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->CoringWeight[i], sizeof(obj->CoringWeight[i]), ARRAY_NUM(obj->CoringWeight[i])));

//Demosaic
		snprintf(tag_buf, sizeof(tag_buf), "EdgeSmoothThr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->EdgeSmoothThr[i], sizeof(obj->EdgeSmoothThr[i]), ARRAY_NUM(obj->EdgeSmoothThr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "EdgeSmoothSlope_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->EdgeSmoothSlope[i], sizeof(obj->EdgeSmoothSlope[i]), ARRAY_NUM(obj->EdgeSmoothSlope[i])));

		snprintf(tag_buf, sizeof(tag_buf), "AntiAliasThr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->AntiAliasThr[i], sizeof(obj->AntiAliasThr[i]), ARRAY_NUM(obj->AntiAliasThr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "AntiAliasSlope_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->AntiAliasSlope[i], sizeof(obj->AntiAliasSlope[i]), ARRAY_NUM(obj->AntiAliasSlope[i])));

		snprintf(tag_buf, sizeof(tag_buf), "NrCoarseStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->NrCoarseStr[i], sizeof(obj->NrCoarseStr[i]), ARRAY_NUM(obj->NrCoarseStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "DetailEnhanceStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->DetailEnhanceStr[i], sizeof(obj->DetailEnhanceStr[i]), ARRAY_NUM(obj->DetailEnhanceStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "NoiseSuppressStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->NoiseSuppressStr[i], sizeof(obj->NoiseSuppressStr[i]), ARRAY_NUM(obj->NoiseSuppressStr[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SharpenLumaStr_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SharpenLumaStr[i], sizeof(obj->SharpenLumaStr[i]), ARRAY_NUM(obj->SharpenLumaStr[i])));

//Gamma
		snprintf(tag_buf, sizeof(tag_buf), "Gamma_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->Gamma[i] = strtol(buf, NULL, 0);
//DRC
		snprintf(tag_buf, sizeof(tag_buf), "DrcThreshold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->DrcThreshold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "DrcAutoStrength_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->DrcAutoStrength[i], sizeof(obj->DrcAutoStrength[i]), ARRAY_NUM(obj->DrcAutoStrength[i])));

/////
		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateHighHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateHighHold[i] = strtol(buf, NULL, 0);

		snprintf(tag_buf, sizeof(tag_buf), "SlowFrameRateLowHold_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		obj->SlowFrameRateLowHold[i] = strtol(buf, NULL, 0);
		
//vppsV2
		snprintf(tag_buf, sizeof(tag_buf), "IES0_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->IES0[i], sizeof(obj->IES0[i]), ARRAY_NUM(obj->IES0[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SBS0_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SBS0[i], sizeof(obj->SBS0[i]), ARRAY_NUM(obj->SBS0[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SBS1_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SBS1[i], sizeof(obj->SBS1[i]), ARRAY_NUM(obj->SBS1[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SBS2_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SBS2[i], sizeof(obj->SBS2[i]), ARRAY_NUM(obj->SBS2[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SBS3_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SBS3[i], sizeof(obj->SBS3[i]), ARRAY_NUM(obj->SBS3[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SDS0_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SDS0[i], sizeof(obj->SDS0[i]), ARRAY_NUM(obj->SDS0[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SDS1_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SDS1[i], sizeof(obj->SDS1[i]), ARRAY_NUM(obj->SDS1[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SDS2_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SDS2[i], sizeof(obj->SDS2[i]), ARRAY_NUM(obj->SDS2[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SDS3_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SDS3[i], sizeof(obj->SDS3[i]), ARRAY_NUM(obj->SDS3[i])));

		snprintf(tag_buf, sizeof(tag_buf), "STH0_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->STH0[i], sizeof(obj->STH0[i]), ARRAY_NUM(obj->STH0[i])));

		snprintf(tag_buf, sizeof(tag_buf), "STH1_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->STH1[i], sizeof(obj->STH1[i]), ARRAY_NUM(obj->STH1[i])));

		snprintf(tag_buf, sizeof(tag_buf), "STH2_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->STH2[i], sizeof(obj->STH2[i]), ARRAY_NUM(obj->STH2[i])));

		snprintf(tag_buf, sizeof(tag_buf), "STH3_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->STH3[i], sizeof(obj->STH3[i]), ARRAY_NUM(obj->STH3[i])));

		snprintf(tag_buf, sizeof(tag_buf), "MDP_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->MDP[i], sizeof(obj->MDP[i]), ARRAY_NUM(obj->MDP[i])));

		snprintf(tag_buf, sizeof(tag_buf), "MATH1_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->MATH1[i], sizeof(obj->MATH1[i]), ARRAY_NUM(obj->MATH1[i])));

		snprintf(tag_buf, sizeof(tag_buf), "MATH2_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->MATH2[i], sizeof(obj->MATH2[i]), ARRAY_NUM(obj->MATH2[i])));

		snprintf(tag_buf, sizeof(tag_buf), "Pro3_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->Pro3[i], sizeof(obj->Pro3[i]), ARRAY_NUM(obj->Pro3[i])));

		snprintf(tag_buf, sizeof(tag_buf), "MDDZ1_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->MDDZ1[i], sizeof(obj->MDDZ1[i]), ARRAY_NUM(obj->MDDZ1[i])));

		snprintf(tag_buf, sizeof(tag_buf), "MDDZ2_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->MDDZ2[i], sizeof(obj->MDDZ2[i]), ARRAY_NUM(obj->MDDZ2[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TFS1_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TFS1[i], sizeof(obj->TFS1[i]), ARRAY_NUM(obj->TFS1[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TFS2_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TFS2[i], sizeof(obj->TFS2[i]), ARRAY_NUM(obj->TFS2[i])));

		snprintf(tag_buf, sizeof(tag_buf), "SFC_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->SFC[i], sizeof(obj->SFC[i]), ARRAY_NUM(obj->SFC[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TFC_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TFC[i], sizeof(obj->TFC[i]), ARRAY_NUM(obj->TFC[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TPC_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TPC[i], sizeof(obj->TPC[i]), ARRAY_NUM(obj->TPC[i])));

		snprintf(tag_buf, sizeof(tag_buf), "TRC_%d", i);
		ASSERT_POINT(default_str, handle->read_text(handle, "IMP", tag_buf, default_str, buf, sizeof(buf)));
		ASSERT_ZERO(str2array(buf, obj->TRC[i], sizeof(obj->TRC[i]), ARRAY_NUM(obj->TRC[i])));

	}


	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelStrength", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelStrength, sizeof(obj->DefectPixelStrength), ARRAY_NUM(obj->DefectPixelStrength)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "DefectPixelBlendRatio", default_str, buf, sizeof(buf)));
	ASSERT_ZERO(str2array(buf, obj->DefectPixelBlendRatio, sizeof(obj->DefectPixelBlendRatio), ARRAY_NUM(obj->DefectPixelBlendRatio)));

	ASSERT_POINT(default_str, handle->read_text(handle, "IMP", "AntiFlikeThreshold", default_str, buf, sizeof(buf)));
	obj->AntiFlikeThreshold = strtol(buf, NULL, 0);

	if(default_str != handle->read_text(handle, "IMP", "ColortoblackVal", default_str, buf, sizeof(buf))){
		ret = str2array(buf, obj->ColortoblackVal, sizeof(obj->ColortoblackVal), ARRAY_NUM(obj->ColortoblackVal));
	}
	if(0 != ret){
			unsigned int colortoblackval_array[9] = {10,20,30,40,100,200,300,400,500};
			memcpy(obj->ColortoblackVal,colortoblackval_array,sizeof(obj->ColortoblackVal));
		}
	ret = -1;

	///Daylight night  switch
	if(default_str != handle->read_text(handle, "IMP", "DaylightToNight", default_str, buf, sizeof(buf))){
		obj->DaylightToNight = strtol(buf, NULL, 0);
	}else{
		obj->DaylightToNight = 45;
	}
	if(default_str !=  handle->read_text(handle, "IMP", "NightToDaylight2", default_str, buf, sizeof(buf))){
		ret = str2array_float(buf, obj->NightToDaylight, sizeof(obj->NightToDaylight), ARRAY_NUM(obj->NightToDaylight));
	}
	if(0 != ret){
		float def_array[8] = {10,7,4,2,1,0.5,0.25,0.125};//deflaut array, night switch daylight
		memcpy(obj->NightToDaylight,def_array,sizeof(obj->NightToDaylight));
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

