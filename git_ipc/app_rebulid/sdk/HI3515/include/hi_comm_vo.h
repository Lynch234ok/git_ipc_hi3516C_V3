/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_comm_vo.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/03/18
  Description   :
  History       :
  1.Date        : 2009/03/18
    Author      : x00100808
    Modification: Created file

******************************************************************************/

#ifndef __HI_COMM_VO_H__
#define __HI_COMM_VO_H__

#include "hi_type.h"
#include "hi_common.h"
#include "hi_comm_video.h"

#define VO_DEF_CHN_BUF_LEN      8
#define VO_DEF_DISP_BUF_LEN		5
#define VO_DEF_VIRT_BUF_LEN		3

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum hiEN_VOU_ERR_CODE_E
{
    EN_ERR_VO_DEV_NOT_CONFIG	  = 0x40,
    EN_ERR_VO_DEV_NOT_ENABLE      = 0x41,
    EN_ERR_VO_DEV_NOT_DISABLE     = 0x42,
    EN_ERR_VO_DEV_HAS_BINDED      = 0x43,
    EN_ERR_VO_DEV_NOT_BINDED      = 0x44,

    ERR_VO_NOT_ENABLE             = 0x45,
    ERR_VO_NOT_DISABLE            = 0x46,
    ERR_VO_NOT_CONFIG             = 0x47,

    ERR_VO_CHN_NOT_DISABLE        = 0x48,
    ERR_VO_CHN_NOT_ENABLE         = 0x49,
    ERR_VO_CHN_NOT_CONFIG         = 0x4a,
    ERR_VO_CHN_NOT_ALLOC          = 0x4b,

    ERR_VO_CCD_INVALID_PAT        = 0x4c,
    ERR_VO_CCD_INVALID_POS        = 0x4d,

    ERR_VO_WAIT_TIMEOUT           = 0x4e,
    ERR_VO_INVALID_VFRAME         = 0x4f,
    ERR_VO_INVALID_RECT_PARA      = 0x50,
    ERR_VO_SETBEGIN_ALREADY       = 0x51,
    ERR_VO_SETBEGIN_NOTYET        = 0x52,
    ERR_VO_SETEND_ALREADY         = 0x53,
    ERR_VO_SETEND_NOTYET          = 0x54,

    ERR_VO_GRP_INVALID_ID         = 0x55,
    ERR_VO_GRP_NOT_CREATE         = 0x56,
    ERR_VO_GRP_HAS_CREATED        = 0x57,
    ERR_VO_GRP_NOT_DESTROY        = 0x58,
    ERR_VO_GRP_CHN_FULL           = 0x59,
    ERR_VO_GRP_CHN_EMPTY          = 0x5a,
    ERR_VO_GRP_CHN_NOT_EMPTY      = 0x5b,
    ERR_VO_GRP_INVALID_SYN_MODE   = 0x5c,
    ERR_VO_GRP_INVALID_BASE_PTS   = 0x5d,
    ERR_VO_GRP_NOT_START          = 0x5e,
    ERR_VO_GRP_NOT_STOP           = 0x5f,
    ERR_VO_GRP_INVALID_FRMRATE    = 0x60,
    ERR_VO_GRP_CHN_HAS_REG        = 0x61,
    ERR_VO_GRP_CHN_NOT_REG        = 0x62,
    ERR_VO_GRP_CHN_NOT_UNREG      = 0x63,
    ERR_VO_GRP_BASE_NOT_CFG       = 0x64,

    /* new added */
    EN_ERR_VO_DEV_HAS_ENABLED     = 0x65,

    ERR_VO_BUTT

}EN_VOU_ERR_CODE_E;

/* System define error code */
#define HI_ERR_VO_BUSY              	HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define HI_ERR_VO_NO_MEM                HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define HI_ERR_VO_NULL_PTR              HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define HI_ERR_VO_SYS_NOTREADY          HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define HI_ERR_VO_INVALID_DEVID         HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
#define HI_ERR_VO_INVALID_CHNID         HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define HI_ERR_VO_ILLEGAL_PARAM         HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define HI_ERR_VO_NOT_SURPPORT          HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SURPPORT)
#define HI_ERR_VO_NOT_PERMIT            HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)

/* device relative error code */
#define HI_ERR_VO_DEV_NOT_CONFIG        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_CONFIG)
#define HI_ERR_VO_DEV_NOT_ENABLE        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_ENABLE)
#define HI_ERR_VO_DEV_HAS_ENABLED       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_HAS_ENABLED)
#define HI_ERR_VO_DEV_NOT_DISABLE       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_DISABLE)
#define HI_ERR_VO_DEV_HAS_BINDED        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_HAS_BINDED)
#define HI_ERR_VO_DEV_NOT_BINDED        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_BINDED)

/* video relative error code */
#define HI_ERR_VO_VIDEO_NOT_ENABLE      HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_NOT_ENABLE)
#define HI_ERR_VO_VIDEO_NOT_DISABLE     HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_NOT_DISABLE)
#define HI_ERR_VO_VIDEO_NOT_CONFIG      HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_NOT_CONFIG)

/* channel relative error code */
#define HI_ERR_VO_CHN_NOT_DISABLE       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_CHN_NOT_DISABLE)
#define HI_ERR_VO_CHN_NOT_ENABLE        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_CHN_NOT_ENABLE)
#define HI_ERR_VO_CHN_NOT_CONFIG        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_CHN_NOT_CONFIG)
#define HI_ERR_VO_CHN_NOT_ALLOC         HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_CHN_NOT_ALLOC)

/* cascade relatvie error code */
#define HI_ERR_VO_INVALID_PATTERN       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_CCD_INVALID_PAT)
#define HI_ERR_VO_INVALID_POSITION      HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_CCD_INVALID_POS)

/* misc */
#define HI_ERR_VO_WAIT_TIMEOUT          HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_WAIT_TIMEOUT)
#define HI_ERR_VO_INVALID_VFRAME        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_INVALID_VFRAME)
#define HI_ERR_VO_INVALID_RECT_PARA     HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_INVALID_RECT_PARA)
#define HI_ERR_VO_SETBEGIN_ALREADY      HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_SETBEGIN_ALREADY)
#define HI_ERR_VO_SETBEGIN_NOTYET       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_SETBEGIN_NOTYET)
#define HI_ERR_VO_SETEND_ALREADY        HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_SETEND_ALREADY)
#define HI_ERR_VO_SETEND_NOTYET         HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_SETEND_NOTYET)

/* sync group relative error code */
#define HI_ERR_VO_GRP_INVALID_ID    	HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_INVALID_ID)
#define HI_ERR_VO_GRP_NOT_CREATE		HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_NOT_CREATE)
#define HI_ERR_VO_GRP_HAS_CREATED		HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_HAS_CREATED)
#define HI_ERR_VO_GRP_NOT_DESTROY       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_NOT_DESTROY)
#define HI_ERR_VO_GRP_CHN_FULL			HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_CHN_FULL)
#define HI_ERR_VO_GRP_CHN_EMPTY			HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_CHN_EMPTY)
#define HI_ERR_VO_GRP_CHN_NOT_EMPTY		HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_CHN_NOT_EMPTY)
#define HI_ERR_VO_GRP_INVALID_SYN_MODE	HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_INVALID_SYN_MODE)
#define HI_ERR_VO_GRP_INVALID_BASE_PTS	HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_INVALID_BASE_PTS)
#define HI_ERR_VO_GRP_NOT_START			HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_NOT_START)
#define HI_ERR_VO_GRP_NOT_STOP          HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_NOT_STOP)
#define HI_ERR_VO_GRP_INVALID_FRMRATE   HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_INVALID_FRMRATE)
#define HI_ERR_VO_GRP_CHN_HAS_REG       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_CHN_HAS_REG)
#define HI_ERR_VO_GRP_CHN_NOT_REG       HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_CHN_NOT_REG)
#define HI_ERR_VO_GRP_CHN_NOT_UNREG     HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_CHN_NOT_UNREG)
#define HI_ERR_VO_GRP_BASE_NOT_CFG      HI_DEF_ERR(HI_ID_VOU, EN_ERR_LEVEL_ERROR, ERR_VO_GRP_BASE_NOT_CFG)


typedef struct hiVO_CHN_ATTR_S
{
    HI_U32  u32Priority;                 /* video out overlay pri */
    RECT_S  stRect; 	                 /* rect of video out chn */
    HI_BOOL bZoomEnable;                 /* zoom or not */
    HI_BOOL bDeflicker;                  /* deflicker or not */

}VO_CHN_ATTR_S;

typedef enum hiVO_SCREEN_HFILTER_E
{
	VO_SCREEN_HFILTER_DEF	= 0,
	VO_SCREEN_HFILTER_8M,
	VO_SCREEN_HFILTER_6M,
	VO_SCREEN_HFILTER_5M,
	VO_SCREEN_HFILTER_4M,
	VO_SCREEN_HFILTER_3M,
	VO_SCREEN_HFILTER_2M,
	VO_SCREEN_HFILTER_BUTT
    
} VO_SCREEN_HFILTER_E;

typedef enum hiVO_SCREEN_VFILTER_E
{
	VO_SCREEN_VFILTER_DEF	= 0,
	VO_SCREEN_VFILTER_8M,
	VO_SCREEN_VFILTER_6M,
	VO_SCREEN_VFILTER_5M,
	VO_SCREEN_VFILTER_4M,
	VO_SCREEN_VFILTER_3M,
	VO_SCREEN_VFILTER_2M,	
	VO_SCREEN_VFILTER_BUTT
    
} VO_SCREEN_VFILTER_E;

typedef struct hiVO_SCALE_FILTER_S
{
    VO_SCREEN_HFILTER_E enHFilter;
    VO_SCREEN_VFILTER_E enVFilter;
    
} VO_SCREEN_FILTER_S;

typedef struct hiVO_CHN_FILTER_S
{
    DSU_FILTER_PARAM_TYPE    enFiltType;     /* filter type*/
    DSU_HSCALE_FILTER_E      enHFilter;      /* horizontal filter sequence number*/
    DSU_VSCALE_FILTER_E      enVFilterL;     /* vertical filter of luminance sequence number*/
    DSU_VSCALE_FILTER_E      enVFilterC;     /* vertical filter of chroma sequence number*/

}VO_CHN_FILTER_S;

typedef enum hiVO_DISPLAY_FIELD_E
{
  VO_FIELD_TOP,		            /* top field*/
  VO_FIELD_BOTTOM,	            /* bottom field*/
  VO_FIELD_BOTH,	            /* top and bottom field*/
  VO_FIELD_BUTT

} VO_DISPLAY_FIELD_E;

typedef struct hiVO_QUERY_STATUS_S
{
    HI_U32 u32ChnBufUsed;       /* channel buffer that been occupied */

} VO_QUERY_STATUS_S;

typedef struct hiVO_SRC_ATTR_S
{
    HI_BOOL bInterlaced;		/* interlaced source */
    
} VO_SRC_ATTR_S;


/*****************************************************************************
 * 3520 ADDed
 *****************************************************************************/
typedef enum hiVO_INTF_TYPE_E
{
    VO_INTF_CVBS     = 0,                  /* for SD or AD */
    VO_INTF_BT656    = 1,                  /* for AD only */
    VO_INTF_VGA      = 2,                  /* for HD or AD */
    VO_INTF_YPBPR    = 3,                  /* for HD only */
    VO_INTF_BT1120   = 4,                  /* for HD only */
    VO_INTF_LCD      = 5,                  /* for HD only */
    VO_INTF_BUTT

} VO_INTF_TYPE_E;

typedef enum hiVO_INTF_SYNC_E
{
    VO_OUTPUT_PAL           = 0,
    VO_OUTPUT_NTSC          = 1,

    VO_OUTPUT_720P60        = 2,
    VO_OUTPUT_1080I50       = 3,
    VO_OUTPUT_1080I60       = 4,
    VO_OUTPUT_1080P25       = 5,
    VO_OUTPUT_1080P30       = 6,

    VO_OUTPUT_800x600_60    = 7,			/* VESA 800 x 600 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1024x768_60   = 8,			/* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1280x1024_60  = 9,			/* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1366x768_60   = 10,			/* VESA 1366 x 768 at 60 Hz (non-interlaced) */
    VO_OUTPUT_1440x900_60   = 11,			/* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */

    VO_OUTPUT_800x600_75    = 12,			/* VESA 800 x 600 at 75 Hz (non-interlaced) */
    VO_OUTPUT_1024x768_75   = 13,			/* VESA 1024 x 768 at 75 Hz (non-interlaced) */

    VO_OUTPUT_USER			= 14,
    VO_OUTPUT_BUTT

} VO_INTF_SYNC_E;

typedef struct tagVO_SYNC_INFO_S
{
    HI_BOOL  bSynm;     /* sync mode(0:timing,as BT.656; 1:signal,as LCD) */
    HI_BOOL  bIop;      /* interlaced or progressive display(0:i; 1:p) */
    HI_U8    u8Intfb;   /* interlace bit width while output */

    HI_U16   u16Vact ;  /* vertical active area */
    HI_U16   u16Vbb;    /* vertical back blank porch */
    HI_U16   u16Vfb;    /* vertical front blank porch */

    HI_U16   u16Hact;   /* herizontal active area */
    HI_U16   u16Hbb;    /* herizontal back blank porch */
    HI_U16   u16Hfb;    /* herizontal front blank porch */

    HI_U16   u16Bvact;  /* bottom vertical active area */
    HI_U16   u16Bvbb;   /* bottom vertical back blank porch */
    HI_U16   u16Bvfb;   /* bottom vertical front blank porch */

    HI_U16   u16Hpw;    /* horizontal pulse width */
    HI_U16   u16Vpw;    /* vertical pulse width */

    HI_BOOL  bIdv;      /* inverse data valid of output */
    HI_BOOL  bIhs;      /* inverse horizontal synch signal */
    HI_BOOL  bIvs;      /* inverse vertical synch signal */

} VO_SYNC_INFO_S;

typedef struct hiVO_PUB_ATTR_S
{
    HI_U32           u32BgColor;            /* background color of device [RGB] */
    VO_INTF_TYPE_E   enIntfType;            /* vo inteface type */
    VO_INTF_SYNC_E   enIntfSync;            /* vo interface synchronization */
    VO_SYNC_INFO_S   stSyncInfo;            /* video synchronizing information */

} VO_PUB_ATTR_S;

#define VO_DEFAULT_CHN      -1          /* use vo buffer as pip buffer */

typedef struct hiVO_VIDEO_LAYER_ATTR_S
{
    RECT_S stDispRect;                  /* display window */
    SIZE_S stImageSize;                 /* display resolution */
    HI_U32 u32DispFrmRt;                /* display frame rate */
    PIXEL_FORMAT_E enPixFormat;         /* support sp420 and sp422 */

    HI_S32 s32PiPChn;                   /* (1)VO_DEFAULT_CHN: default value */
                                        /* (2)[0~31]: use the channel buffer as pip buffer */
} VO_VIDEO_LAYER_ATTR_S;

typedef struct hiVO_VBI_INFO_S
{
    HI_U32 u32X;
    HI_U32 u32Y;
    HI_U8  u8VbiInfo[128];
    HI_U32 u32InfoLen;

} VO_VBI_INFO_S;

typedef struct hiVO_ZOOM_ATTR_S
{
    RECT_S stZoomRect;
    VIDEO_FIELD_E  enField;

} VO_ZOOM_ATTR_S;

typedef enum hiVO_CSC_E
{
    VO_CSC_LUMA     = 0,        /* luminance:   [0 ~ 100] */
    VO_CSC_CONTR    = 1,        /* contrast :   [0 ~ 100] */
    VO_CSC_HUE      = 2,        /* hue      :   [0 ~ 100] */
    VO_CSC_SATU     = 3,        /* satuature:   [0 ~ 100] */
    VO_CSC_BUTT

} VO_CSC_E;

typedef struct hiVO_CSC_S
{
    VO_CSC_E enCSCType;
    HI_U32   u32Value;

} VO_CSC_S;

typedef struct hiVO_DRAW_ATTR_S
{
    RECT_S stDrawRect;
    HI_U32 u32Color;
    
} VO_DRAW_ATTR_S;

typedef struct hiVO_DRAW_CFG_S
{
	HI_U32 u32DrawCount;
	VO_DRAW_ATTR_S stDrawAttr[VO_MAX_SOLIDDRAW];
    
} VO_DRAW_CFG_S;

typedef struct hiVO_ZOOM_RATIO_S
{
    HI_U32 u32XRatio;
    HI_U32 u32YRatio;
    HI_U32 u32WRatio;
    HI_U32 u32HRatio;
    
} VO_ZOOM_RATIO_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_COMM_VO_H__ */

