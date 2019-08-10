/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : Hi_comm_md.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2006/01/19
  Description   : Hi_MotionDetect.c header file
  History       :
  1.Date        : 2006/01/19
    Author      : cchao
    Modification: Created file
  2.Date		: 2008/3/6
    Author		: l59217
    Modification : chang "HI_LOG_LEVEL_ERROR" to "EN_ERR_LEVEL_ERROR"
    			   delete all local MD debug definition
  3.Date		: 2008/3/7
    Author	    : l59217
    Modification : added HI_ERR_MD_BUSY  errno
  4.Date        : 2008/4/1
    Author      : l59217
    Modification: delete MD_RGN_MODE_S,MD_RGN_ATTR_S,MD_RGN_CFG_S,MD_RGN_DATA_S
                  modified MD_DATA_S, MD_CHN_ATTR_S

  5.Date        : 2008/4/25
    Author      : l64467
    Modification: trim struct,add comment

  6.Date        : 2008/10/17
    Author      : z50825
    Modification: modify print HI_TRACE_MD

  7.Date        : 2008/10/31
    Author      : z44949
    Modification: delete the HI_DEF_ERR_MD defination
                  #define HI_DEF_ERR_MD(errid) \
                       HI_DEF_ERR(HI_ID_MD,HI_LOG_LEVEL_ERROR,errid)  

******************************************************************************/
#ifndef __HI_COMM_MD_H__
#define __HI_COMM_MD_H__

#include "hi_type.h"
#include "hi_debug.h"
#include "hi_comm_video.h"
#include "hi_common.h"
#include "hi_errno.h"
#include "hi_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef __KERNEL__ 
	#define HI_TRACE_MD(level, fmt...)	\
	do{\
		if(level<=HI_DBG_ERR)\
		{\
			printf("[Func]:%s [Line]:%d [Info]:", __FUNCTION__, __LINE__);\
		    printf(fmt);\
		}\
	} while(0);

#else
	#define HI_TRACE_MD(level, fmt...)\
	do{\
		HI_TRACE(level, HI_ID_MD, "[%s]: %d,", __FUNCTION__, __LINE__);\
		HI_TRACE(level, HI_ID_MD, ##fmt);\
	} while(0);
#endif\


/* invlalid device ID */
#define HI_ERR_MD_INVALID_DEVID     HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define HI_ERR_MD_INVALID_CHNID     HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define HI_ERR_MD_ILLEGAL_PARAM     HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/* channel exists */
#define HI_ERR_MD_EXIST             HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/* using a NULL point */
#define HI_ERR_MD_NULL_PTR          HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define HI_ERR_MD_NOT_CONFIG        HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define HI_ERR_MD_NOT_SURPPORT      HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SURPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define HI_ERR_MD_NOT_PERM          HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
/* the channle is not existed  */
#define HI_ERR_MD_UNEXIST             HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
/* failure caused by malloc memory */
#define HI_ERR_MD_NOMEM             HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
/* failure caused by malloc buffer */
#define HI_ERR_MD_NOBUF             HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
/* no data in buffer */
#define HI_ERR_MD_BUF_EMPTY         HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
/* no buffer for new data */
#define HI_ERR_MD_BUF_FULL          HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
/* system is not ready,had not initialed or loaded*/
#define HI_ERR_MD_SYS_NOTREADY      HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
/* resource is busy, eg. destroy a venc chn without unregistering it */
#define HI_ERR_MD_BUSY      		HI_DEF_ERR(HI_ID_MD, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)


typedef struct hiMD_MB_MODE_S
{
    HI_BOOL bMBSADMode;    /*MB SAD mode*/
    HI_BOOL bMBMVMode;     /*MB MV mode*/
	HI_BOOL bMBALARMMode;  /*MB alarm mode*/
    HI_BOOL bMBPelNumMode; /*MB alarm pels number mode*/   
} MD_MB_MODE_S;

typedef enum hiMD_REF_MODE_E
{
    MD_REF_AUTO = 0,  /*reference pictrue auto mode*/
    MD_REF_USER,      /*reference pictrue user mode*/
    MD_REF_MODE_BUTT  /*reserve*/
} MD_REF_MODE_E;

typedef enum hiMD_REF_STATUS_E
{
    MD_REF_STATIC = 0,  /*reference pictrue static*/
    MD_REF_DYNAMIC,     /*reference pictrue dynamic*/
    MD_REF_STATUS_BUTT  /*reserve*/
} MD_REF_STATUS_E;

typedef struct hiMD_DLIGHT_S
{
    HI_BOOL bEnable;    /*delete light enable?*/
    HI_U8   u8DlBeta;   /*delete light beta*/
    HI_U8   u8DlAlpha;  /*delete light alpha*/
    HI_U16  Reserved;   /*reserve*/
} MD_DLIGHT_S;

typedef enum hiMD_SADBITS_E
{
    MD_SAD_8BIT = 0,  /*SAD precision 8bits*/
    MD_SAD_16BIT,     /*SAD precision 16bits*/
    MD_SAD_BUTT       /*reserve*/
} MD_SADBITS_E;


typedef struct hiMD_REF_ATTR_S
{
    MD_REF_MODE_E    enRefFrameMode; /*reference picture mode*/
    MD_REF_STATUS_E  enRefFrameStat; /*reference picture status*/
    VIDEO_FRAME_S    stUserRefFrame; /*reference pictrue info*/
} MD_REF_ATTR_S; 

typedef struct hiMD_CHN_ATTR_S
{
    MD_MB_MODE_S  stMBMode;       /*MD mode*/
    MD_SADBITS_E  enSADBits;      /*sad  precision*/
    MD_DLIGHT_S   stDlight;       /*delight configure*/
    HI_U8         u8MBPelALTh;    /*pels alarm threshold*/
    HI_U8         u8MBPerALNumTh; /*pels alarm number threshold*/
    HI_U16        u16MBALSADTh;   /*sad alarm threshold*/
    HI_U32        u32MDInternal;  /*internal*/
    HI_U32        u32MDBufNum;    /*Result buffer number*/  
} MD_CHN_ATTR_S;


typedef struct hiMD_MB_DATA_S
{
    HI_U32* pu32Addr;  /*mb data address*/
    HI_U32  u32Stride; /*stride*/
} MD_MB_DATA_S;

typedef struct hiMD_DATA_S
{
	/*NOTICE:don't change this address variable!!!!!!
	 *this address is not userful for the user!!!!!!
	 *while the user release MD data,it will be used by SDK kernel
	 */
    HI_U32*  pu32Addr; 
	
    HI_U16   u16MBWidth;    /*MD channle width in MB*/
    HI_U16   u16MBHeight;   /*MD channle height in MB*/
    HI_U64   u64Pts;        /*time*/

	MD_MB_MODE_S  stMBMode;        /*MD mode*/
    MD_MB_DATA_S  stMBSAD;    	   /*SAD result*/
    MD_MB_DATA_S  stMBMV;     	   /*MV result*/
	MD_MB_DATA_S  stMBAlarm;       /*MB alarm result*/
    MD_MB_DATA_S  stMBPelAlarmNum; /*pels alarm number result*/
} MD_DATA_S;
    

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __HI_COMM_MD_H__ */

