/******************************************************************************

Copyright (C), 2004-2020, Hisilicon Tech. Co., Ltd.

******************************************************************************
File Name     : hi_comm_vi.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2009/3/9
Last Modified :
Description   : 
Function List :
History       :
1.Date        : 2009/3/9
  Author      : p00123320
  Modification: Created file

******************************************************************************/

#ifndef __HI_COMM_VI_H__
#define __HI_COMM_VI_H__

#include "hi_common.h"
#include "hi_errno.h"
#include "hi_comm_video.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */


typedef enum hiEN_VIU_ERR_CODE_E
{
    ERR_VI_FAILED_NOTENABLE = 64,   /* device or channel not enable*/
    ERR_VI_FAILED_NOTDISABLE,       /* device not disable*/
    ERR_VI_FAILED_CHNOTDISABLE,     /* channel not disable*/
	ERR_VI_CFG_TIMEOUT,             /* config timeout*/
	ERR_VI_NORM_UNMATCH,            /* video norm of ADC and VIU is unmatch*/     
} EN_VIU_ERR_CODE_E;

#define HI_ERR_VI_INVALID_PARA          HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM) 
#define HI_ERR_VI_INVALID_DEVID         HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
#define HI_ERR_VI_INVALID_CHNID         HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define HI_ERR_VI_INVALID_NULL_PTR      HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define HI_ERR_VI_FAILED_NOTCONFIG      HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
#define HI_ERR_VI_SYS_NOTREADY          HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define HI_ERR_VI_BUF_EMPTY          	HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
#define HI_ERR_VI_BUF_FULL          	HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
#define HI_ERR_VI_NOMEM         		HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define HI_ERR_VI_NOT_SURPPORT         	HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SURPPORT)
#define HI_ERR_VI_BUSY              	HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define HI_ERR_VI_NOT_PERM              HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)

#define HI_ERR_VI_FAILED_NOTENABLE      HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_NOTENABLE)/* 0xA0108040*/
#define HI_ERR_VI_FAILED_NOTDISABLE     HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_NOTDISABLE)/* 0xA0108041*/
#define HI_ERR_VI_FAILED_CHNOTDISABLE   HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, ERR_VI_FAILED_CHNOTDISABLE)/* 0xA0108042*/
#define HI_ERR_VI_CFG_TIMEOUT           HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, ERR_VI_CFG_TIMEOUT)/* 0xA0108043*/
#define HI_ERR_VI_NORM_UNMATCH          HI_DEF_ERR(HI_ID_VIU, EN_ERR_LEVEL_ERROR, ERR_VI_NORM_UNMATCH)/* 0xA0108044*/



#define VI_INVALID_FRMRATE  (-1UL)

#define VI_INVALID_CHN_ID   (-1UL)


/* captrue selection of video input */
typedef enum hiVI_CAPSEL_E
{
    VI_CAPSEL_TOP = 0,		/* top field */
    VI_CAPSEL_BOTTOM,		/* bottom field */
    VI_CAPSEL_BOTH,			/* top and bottom field */
    VI_CAPSEL_BUTT
} VI_CAPSEL_E;


/* interface mode of video input */
typedef enum hiVI_INPUT_MODE_E
{
    VI_MODE_BT656 = 0,		    /* ITU-R BT.656 YUV4:2:2 */
    VI_MODE_BT601,			    /* ITU-R BT.601 YUV4:2:2 */			
    VI_MODE_DIGITAL_CAMERA,	    /* digatal camera mode */
    VI_MODE_BT1120_PROGRESSIVE, /* BT.1120 progressive mode */
    VI_MODE_BT1120_INTERLACED,  /* BT.1120 interstage mode */
    VI_MODE_BT601_SEP,
    VI_MODE_DIGITAL_CAMERA_SEP,
    VI_MODE_BUTT
} VI_INPUT_MODE_E;


/* work mode of BT.656 interface */
typedef enum hiVI_WORK_MODE_E
{
    VI_WORK_MODE_1D1 = 0,       /* 27M 1D1 mode */
    VI_WORK_MODE_2D1,           /* 54M 2D1 mode */
    VI_WORK_MODE_4HALFD1,       /* 54M 4CIF mode */
    VI_WORK_MODE_4D1,           /* 108M 4D1 mode */
    VI_WORK_MODE_BUTT
} VI_WORK_MODE_E;


/* attribute of VI channel*/
typedef struct hiVI_CHN_ATTR_S
{
    RECT_S  		stCapRect;      	/* captrue region(heed meaning of width and height) */
    VI_CAPSEL_E 	enCapSel;   		/* select of filed or frame */ 
    HI_BOOL 		bDownScale;     	/* downscaling switch(1/2) in horizon */
    HI_BOOL 		bChromaResample;	/* chroma resample */
    HI_BOOL 		bHighPri;           /* support one chn high priority */		
    PIXEL_FORMAT_E 	enViPixFormat;      /* support sp420 and sp422 */
} VI_CHN_ATTR_S;


/* public attribute of VI device*/
typedef struct hiVI_PUB_ATTR_S
{
    VI_INPUT_MODE_E enInputMode;		/* video input mode */
    
    VI_WORK_MODE_E  enWorkMode; 		/* work mode(only for ITU-R.BT656) */ 
    VIDEO_NORM_E 	enViNorm; 			/* video input normal(only for BT.601) */
    HI_BOOL         bIsChromaChn;       /* whethe this device capture chroma data (only for BT.1120) */
    HI_BOOL         bChromaSwap;        /* whethe exchange U/V of chroma channle  (only for BT.1120) */
} VI_PUB_ATTR_S;

typedef enum hi_VI_USERPIC_MODE_E
{
    VI_USERPIC_MODE_PIC = 0,
    VI_USERPIC_MODE_BGC,                /* the user picture background color mode */
    VI_USERPIC_MODE_BUTT,    
} VI_USERPIC_MODE_E;

typedef struct hiVI_USERPIC_BGC_S
{
    HI_U32          u32BgColor;
} VI_USERPIC_BGC_S;

typedef struct hiVI_USERPIC_ATTR_S
{
    HI_BOOL                 bPub;
    VI_USERPIC_MODE_E       enUsrPicMode;
    union
    {
        VIDEO_FRAME_INFO_S  stUsrPicFrm;
        VI_USERPIC_BGC_S    stUsrPicBg;
    }unUsrPic;    
} VI_USERPIC_ATTR_S;

typedef struct hiVI_CH_LUM_S
{
    HI_U32 u32FramId;
    HI_U32 u32Lum;    
    HI_U64 u64Pts;
} VI_CH_LUM_S;


typedef struct hiVI_CHN_LUMSTRH_ATTR_S
{
	HI_U32 u32TargetK;
} VI_CHN_LUMSTRH_ATTR_S;


typedef enum hiVI_VBI_LOCAL_E
{
    VI_VBI_LOCAL_ODD_FRONT = 0,
    VI_VBI_LOCAL_ODD_END,
    VI_VBI_LOCAL_EVEN_FRONT,
    VI_VBI_LOCAL_EVEN_END,
    VI_VBI_LOCAL_BUTT
} VI_VBI_LOCAL_E;


typedef struct hiVI_VBI_ATTR_S
{
    VI_VBI_LOCAL_E enLocal; /* location of VBI */
    HI_S32 s32X;            /* horizontal of VBI */
    HI_S32 s32Y;            /* verticality of VBI */
    HI_U32 u32Len;          /* length of VBI data, by word(4 Bytes) */
} VI_VBI_ATTR_S;


typedef struct hiVI_DBG_INFO_S
{
    HI_BOOL bEnable;   /* Whether this channel is enabled */
    HI_U32 u32IntCnt;  /* The video frame interrupt count */
    HI_U32 u32LostInt; /* The interrupt is received but nobody care */
    HI_U32 u32VbFail;  /* Video buffer malloc failure */
    HI_U32 u32PicWidth;/* curren pic width */
    HI_U32 u32PicHeight;/* current pic height */
} VI_DBG_INFO_S;


/* Data formart of source picture */
typedef enum hiVI_SRC_FIELD_E
{
    VI_SRC_FIELD_AUTO           = 0,    /* Auto config by VI timing interface */
    VI_SRC_FIELD_INTERLACED,            /* Interlaced formart */
    VI_SRC_FIELD_FRAME,                 /* Progressive formart */
        
    VI_SRC_FIELD_BUTT
} VI_SRC_FIELD_E;

typedef struct hiVI_SRC_CFG_S
{
    VI_SRC_FIELD_E  enSrcField;     /* Field type of codec source */
    HI_BOOL         bEnNoIntDet;    /* Whether enable no interrupt detecton */
    
    HI_U32          u32Resev;
    
} VI_SRC_CFG_S;
 
typedef struct hiVI_LUMA_COEF_S
{
     HI_S32 as32Coef[4][2];
} VI_LUMA_COEF_S;

typedef struct hiVI_CHROMA_COEF_S
{
     HI_S32 as32Coef[2][2];
} VI_CHROMA_COEF_S;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef__HI_COMM_VIDEO_IN_H__ */


