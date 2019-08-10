#ifndef _IMP_AVD_PARA_H_
#define _IMP_AVD_PARA_H_

#include "imp_algo_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
* \defgroup AVD interface
* AVD interface
* @ingroup interface
* @author Impower-tech
* @version 5.1
* @data 2009-2013
*/


//parameter set of scene change diagnosis 
typedef struct impAVD_CHANGE_PARA_S
{
	IMP_U32 u32Enable;			//detection switch.		0:disable . 1:enable . Default:1
	IMP_U32 u32AlarmLevel;		//sensitivity level.	range:1~5(more sensitivity with larger number). Default:3
}IMP_AVD_CHANGE_PARA_S;


//parameter set of low bright abnormal diagnosis 
typedef struct impAVD_BRIGHT_LOW_PARA_S
{
	IMP_U32 u32Enable;			//detection switch.		0:disable . 1:enable . Default:1
	IMP_U32 u32AlarmLevel;		//sensitivity level.	range:1~5(more sensitivity with larger number). Default:3
	IMP_U32 u32AlarmTime;		//time of alarm waiting.   range:0~20,unit:frame. Default:5
}IMP_AVD_BRIGHT_LOW_PARA_S;

//parameter set of high bright abnormal diagnosis 
typedef struct impAVD_BRIGHT_HIGH_PARA_S
{
	IMP_U32 u32Enable;			//detection switch.		0:disable . 1:enable . Default:1
	IMP_U32 u32AlarmLevel;		//sensitivity level.	range:1~5(more sensitivity with larger number). Default:3
	IMP_U32 u32AlarmTime;		//time of alarm waiting.   range:0~20,unit:frame. Default:5
}IMP_AVD_BRIGHT_HIGH_PARA_S;

//parameter set of clarity diagnosis 
typedef struct impAVD_CLARITY_PARA_S
{
	IMP_U32 u32Enable;			//detection switch.		0:disable . 1:enable . Default:1
	IMP_U32 u32AlarmLevel;		//sensitivity level.	range:1~5(more sensitivity with larger number). Default:3
	IMP_U32 u32AlarmTime;		//time of alarm waiting.   range:0~20,unit:frame. Default:5
}IMP_AVD_CLARITY_PARA_S;


//parameter set of color abnormal diagnosis
typedef struct impAVD_COLOR_PARA_S
{
	IMP_U32 u32Enable;			//detection switch.		0:disable . 1:enable . Default:1
	IMP_U32 u32AlarmLevel;		//sensitivity level.	range:1~5(more sensitivity with larger number). Default:3
	IMP_U32 u32AlarmTime;		//time of alarm waiting.   range:0~20,unit:frame. Default:5
	
}IMP_AVD_COLOR_PARA_S;


//parameter set of interfere diagnosis
typedef struct impAVD_INTERFERE_PARA_S
{
	IMP_U32 u32Enable;			//detection switch.		0:disable . 1:enable . Default:1
	IMP_U32 u32AlarmLevel;		//sensitivity level.	range:1~5(more sensitivity with larger number). Default:3
	IMP_U32 u32AlarmTime;		//time of alarm waiting.   range:0~20,unit:frame. Default:5
}IMP_AVD_INTERFERE_PARA_S;




//parameter set of all AVD modules
typedef struct impAVD_PARA_S
{
	IMP_AVD_CHANGE_PARA_S stChangePara;					//parameter set of scene change diagnosis 
	IMP_AVD_BRIGHT_LOW_PARA_S stBrightLowPara;			//parameter set of low bright abnormal diagnosis 
	IMP_AVD_BRIGHT_HIGH_PARA_S stBrightHighPara;		//parameter set of high bright abnormal diagnosis 
	IMP_AVD_CLARITY_PARA_S stClarityPara;				//parameter set of clarity diagnosis 
	IMP_AVD_COLOR_PARA_S stColorPara;					//parameter set of color abnormal diagnosis 
	IMP_AVD_INTERFERE_PARA_S stInterferePara;			//parameter set of interfere diagnosis 
}IMP_AVD_PARA_S;

/**************************************************************************************************/
/**************************************************************************************************/



// AVD modules analysis value
typedef struct impAVD_Analysis_DATA_S
{
	IMP_S32 s32SceneChg;            //value of scene change
	IMP_S32 s32ClarityValue;        //value of clarity
	IMP_S32 s32BrightHValue;        //value of high bright
	IMP_S32 s32BrightLValue;        //value of low	bright
	IMP_S32 s32ColorValue;	        //value of color abnormal
	IMP_S32 s32InterfereValue;      //value of interfere
}AVD_ANALYSIS_DATA_S;

// AVD process result
typedef struct impAVD_RESULT_S
{
	AVD_ANALYSIS_DATA_S stAnalysisData;	//data of AVD analysis result
	EVT_SET_S	stEventSet;				//data of AVD events
}AVD_RESULT_S;


#ifdef __cplusplus
}
#endif

#endif


