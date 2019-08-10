/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_comm_vpp.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2007/10/25
  Description   : vpp public head file, use for kernel¡¢MPI¡¢API
  History       :
  1.Date        : 2007/10/25
    Author      : l64467
    Modification: Created file

  2.Date        : 2008/03/05
    Author      : l64467
    Modification: Modify for CR20080304018. Change the overlap zone from VI 
                  channel to GROUP channel.

  3.Date        : 2008/03/05
    Author      : l64467
    Modification: Modify for CR20080304018. Modify the structs defination of .
    			 PIC_SCALE_TASK_S and REGION_CTRL_PARAM_U.

  4.Date        : 2008/03/05
    Author      : l64467
    Modification: Modify for AE6D02984. Modify errcode.
  
  5.Date        : 2008/10/31
    Author      : z44949
    Modification: Modify for AE6D03709. And translate the file header.
                  Delete the errcode HI_ERR_VPP_SYS_NOTREADY.
                  
  6.Date        : 2008/11/18
    Author      : p00123320
    Modification: add some items in VPP_SCALE_MODE_E and VIDEO_PREPROC_CONF_S,
                  modify VPP_SCALE_FILTER_E to fit DSU.

  7.Date        : 2009/01/18
    Author      : z44949
    Modification: add VPP_CLIP_ATTR_S in VIDEO_PREPROC_CONF_S. Support to select
                  a part of picture to encode.
                  
  8.Date        : 2010/04/06
    Author      : p00123320
    Modification: Add a new region type that named ViOverlay.
******************************************************************************/


#ifndef __HI_COMM_VPP_H__
#define __HI_COMM_VPP_H__

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_errno.h"
#include "hi_defines.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef __KERNEL__ 
	#define HI_TRACE_VPP(level, fmt...)	\
	do{\
		if(level<=HI_DBG_ERR)\
		{\
			printf("[Func]:%s [Line]:%d [Info]:", __FUNCTION__, __LINE__);\
		    printf(fmt);\
		}\
	} while(0);

#else
#define HI_TRACE_VPP(level, fmt...)\
	do{\
		HI_TRACE(level, HI_ID_MD, "[%s]: %d,", __FUNCTION__, __LINE__);\
		HI_TRACE(level, HI_ID_MD, ##fmt);\
	} while(0);
#endif	

/*all cover region number*/
#define VI_COVER_REGION (VIU_MAX_DEV_NUM * VIU_MAX_CHN_NUM_PER_DEV *MAX_COVER_NUM)

/*all overlay region number*/
#define VENC_OVERLAY_REGION (VENC_MAX_CHN_NUM*MAX_OVERLAY_NUM)

/*all vi overlay region number*/
#define VI_OVERLAY_REGION (VIU_MAX_DEV_NUM * VIU_MAX_CHN_NUM_PER_DEV *MAX_VIOVERLAY_NUM)

/*all coverEx region number*/
#define VI_COVEREX_REGION (VIU_MAX_DEV_NUM * VIU_MAX_CHN_NUM_PER_DEV *MAX_COVEREX_REGION_NUM)

/*invalid frame rate */
#define GRP_INVALID_FRMRATE (-1UL)


/*choose using scale or denoise function*/
typedef enum hiVPP_SCALE_DENOISE_CHOICE_E
{
    VPP_SCALE,
    VPP_DENOISE,
    VPP_BUTT,
} VPP_SCALE_DENOISE_CHOICE_E; 

/* denoise grade*/
typedef enum hiVPP_DENOISE_E
{
    VPP_DENOISE_ONLYEDAGE = 0,		
    VPP_DENOISE_LOWNOISE,		
    VPP_DENOISE_MIDNOISE,		
    VPP_DENOISE_HIGHNOISE,		
    VPP_DENOISE_VERYHIGHNOISE,	
    VPP_DENOISE_BUTT,
} VPP_DENOISE_E;


/* enhance color */
typedef enum hiVPP_CE_E
{
    VPP_CE_DISABLE = 0,
	VPP_CE_ENABLE,
    VPP_CE_BUTT,
} VPP_CE_E;

/* strenthen contrast */
typedef enum hiVPP_LUMA_STR_E
{
    VPP_LUMA_STR_DISABLE = 0,
	VPP_LUMA_STR_ENABLE,
    VPP_LUMA_STR_BUTT,
} VPP_LUMA_STR_E;

/* scale mode of group pic (mode would be adjusted automatically)*/
typedef enum hiVPP_SCALE_MODE_E
{
	VPP_SCALE_MODE_USEBOTTOM = 0,   /* extract bottom field then scale (default value) */
	VPP_SCALE_MODE_USETOP,          /* extract top field then scale */
	VPP_SCALE_MODE_DIRECT,          /* scale derectly */
	VPP_SCALE_MODE_USEBOTTOM2,      /* extract 1/2 bottom field then scale*/
	VPP_SCALE_MODE_USETOP2,         /* extract 1/2 top field then scale*/
	VPP_SCALE_MODE_BUTT,
} VPP_SCALE_MODE_E;

#define VPP_CLIPATTR_NUM 2
typedef struct hiVPP_CLIP_ATTR_S
{
    /* The full size of video which may be come from VI */
    HI_U32 u32SrcWidth;
    HI_U32 u32SrcHeight;

    /* The clip region */
    HI_U32 u32ClipMode;  /* Can be VIDEO_FIELD_xxx which defined in VIDEO_FRAME_S*/
    RECT_S stClipRect;
} VPP_CLIP_ATTR_S;


/* configruation for video preprocesse */
typedef struct hiVIDEO_PREPROC_CONF_S
{
	HI_BOOL	bTemporalDenoise; /* temporal denoise */
    HI_S32 s32TfBase;         /* temporal denoise base value [0,255] */
    HI_S32 s32TfRate;         /* temporal denoise rate value [0,255] */ 
    HI_S32 s32ChrTFDelta;     /* temporal denoise TFDelta value [-127-127] */
    HI_S32 s32DieMode;        /* de-interlace mode, default value is 0. */
    
	HI_BOOL bColorToGrey;     /* convert color video to grey video */

    /* 
     * below frame rate used to control frame of Group channel before DSU and encoder,
     * only valid when size of main channle not same as VI pic, used to save DSU capability,
     * default value GRP_INVALID_FRMRATE means not contrl frame. 
    */
    HI_S32 s32SrcFrmRate;       /* src frame rate of group, should equal to VI */
    HI_S32 s32TarFrmRate;       /* target frame rate of group */

	VPP_SCALE_MODE_E   enScaleMode; /*scale mode*/	

	DSU_FILTER_PARAM_TYPE enFilterType; /*filter type*/
	DSU_HSCALE_FILTER_E enHFilter;   /*horizontal scale filter sequence number*/
	DSU_VSCALE_FILTER_E enVFilterL;   /*vertical scale filter for luminance sequence number*/
	DSU_VSCALE_FILTER_E enVFilterC;   /*vertical scale filter for chroma sequence number*/
	
    /*
    ** If only a part region of the video from vi need send to venc then set this.
    ** The size of video from vi may be change between D1 and CIF, so you can 
    ** prepare two sets of clip attribute. If the video changed, the SDK will
    ** select the proper attribute.
    */
	VPP_CLIP_ATTR_S    stClipAttr[VPP_CLIPATTR_NUM];
}VIDEO_PREPROC_CONF_S;


/*  configruation for video preprocesse */
typedef struct hiVPP_SCALE_CONF_S
{
	HI_BOOL	bTemporalDenoise; /* temporal denoise */
	HI_BOOL	bDeInterlace;     /* convert two filed pic to one frame */ 
	HI_BOOL bColorToGrey;     /* convert color video to grey video */
	
	VPP_SCALE_DENOISE_CHOICE_E     enChoice; /*choose using scale or denoise function*/
	VPP_CE_E                       enCE;     /* enhance color */
	VPP_LUMA_STR_E                 enLumaStr;/* strengthen contrast */
	DSU_HSCALE_FILTER_E             enHFilter; /* horizontal filter option */
	DSU_VSCALE_FILTER_E             enVFilterL; /* vertical filter for lunm */
	DSU_VSCALE_FILTER_E             enVFilterC; /* vertical filter of chroma*/
	VPP_DENOISE_E                  enSpatialDenoise; /* spatial denoise */
} VPP_SCALE_CONF_S;


typedef HI_U32 REGION_HANDLE;


/* type of video regions */
typedef enum hiREGION_TYPE_E
{
    COVER_REGION = 0,   /* video cover region */
    OVERLAY_REGION,     /* video overlay region */
    VIOVERLAY_REGION,   /* video soft overlay region*/
    COVEREX_REGION,     /* video extra cover region */
    REGION_BUTT
} REGION_TYPE_E;

/* attribute of cover region */
typedef struct hiCOVER_ATTR_S
{
    VI_DEV ViDevId;    /* VI device ID */
    VI_CHN ViChn;      /* VI channel ID */
    HI_BOOL bIsPublic; /* is a public region? */
        
    HI_U32 u32Layer;   /* cover region layer */
    RECT_S stRect;     /* position and size of region */
    HI_U32 u32Color;   /* color of the region, pixel format is RGB8888 */
} COVER_ATTR_S;

/* attribute of overlay region */
typedef struct hiOVERLAY_ATTR_S
{
    VENC_GRP VeGroup; /*venc group ID*/
    HI_BOOL bPublic;  /* is a public region? */      
    RECT_S stRect;    /* position and size of region */

	/*bitmap pixel format,now only support ARGB1555 or ARGB4444*/
    PIXEL_FORMAT_E enPixelFmt;

	/*
	* Transparence: The value 0-128 can be used
    * 0 means all OSD image will be invisible, the background will be show whole.
    * 128 means OSD image will cover the background whole
    */

	/* background an foreground transparence when pixel format is ARGB1555 
    * the pixel format is ARGB1555,when the alpha bit is 1 this alpha is value!
    */
    HI_U32 u32FgAlpha;

	/* background an foreground transparence when pixel format is ARGB1555 
    * the pixel format is ARGB1555,when the alpha bit is 0 this alpha is value!
    */
    HI_U32 u32BgAlpha;

    /* background color, pixel format depends on "enPixelFmt" */
    HI_U32 u32BgColor;
} OVERLAY_ATTR_S;

typedef struct hiVI_OVERLAY_ATTR_S
{
    VI_DEV          ViDevId;        
    VI_CHN          ViChn;
    HI_BOOL         bIsPublic;
    
    RECT_S          stRect;
    PIXEL_FORMAT_E  enPixelFmt;
    HI_U32          u32Layer;
    HI_U32          u32BgColor;
    
    HI_U8           u8GlobalAlpha;
    HI_BOOL         bAlphaExt1555; 
    HI_U8           u8Alpha0;
    HI_U8           u8Alpha1;
} VI_OVERLAY_ATTR_S;

typedef struct hiCOVEREX_ATTR_S
{
    VI_DEV ViDevId;    /* VI device ID */
    VI_CHN ViChn;      /* VI channel ID */
    HI_BOOL bIsPublic; /* is a public region? */

	VIDEO_FIELD_E enField;	/* cover tag to which field, top/bottom/interlaced/frame */
    HI_U32 u32Layer;   /* cover region layer */
    RECT_S stRect;     /* position and size of region */
    HI_U32 u32Color;   /* color of the region, pixel format is RGB8888 */

}COVEREX_ATTR_S;

typedef union hiREGION_ATTR_U
{
    COVER_ATTR_S        stCover;        /* attribute of cover region */
    OVERLAY_ATTR_S      stOverlay;      /* attribute of overlay region */
	VI_OVERLAY_ATTR_S   stViOverlay;    /* attribute of vi overlay region */
	COVEREX_ATTR_S      stCoverEx;      /* attribute of extra cover region */
} REGION_ATTR_U;

/* attribute of a region */
typedef struct hiREGION_ATTR_S
{
    REGION_TYPE_E enType;  /*region type */
    REGION_ATTR_U unAttr;  /*region attribute*/
} REGION_ATTR_S;

/* code definition for controlling regions */
typedef enum hiREGION_CRTL_CODE_E
{
    REGION_SHOW = 0,        /* show region */
    REGION_HIDE,            /* hide region */
	REGION_SET_POSTION,     /* set position of a region */
	REGION_SET_COLOR,       /* set color of a region */
	
    REGION_SET_LAYER,       /* set layer of cover region and soft overlay region*/
    REGION_SET_SIZE,        /* set size of cover region */

    REGION_SET_ALPHA0,      /* set background transparence of overlay region and for soft overlay region*/
    REGION_SET_ALPHA1,      /* set foreground transparence of overlay region*/
    REGION_SET_GLOBAL_ALPHA,/* set global transparence of vpp region */
 	REGION_SET_BITMAP,      /* fill bitmap into a overlay region */
 	
    REGION_UPD_BITMAP,      /* update bitmap of overlay region */
    
    REGION_GET_SIGNLE_ATTR, /* get attribute of a region */
    REGION_GET_ALL_COVER_ATTR,    /* get attribute of all cover regions */
    REGION_GET_ALL_OVERLAY_ATTR,  /* get attribute of all overlay regions */
} REGION_CRTL_CODE_E;

typedef struct hiCOVER_S
{
    HI_U32 u32CoverNum; /*cover region number*/
    REGION_HANDLE aCoverHandles[VI_COVER_REGION];/*cover region handle*/
    COVER_ATTR_S astAttr[VI_COVER_REGION];/*cover region attribute*/
} COVER_S;


typedef struct hiOVERLAY_S
{       
    HI_U32 u32OverlayNum;/*overlay region number*/
    REGION_HANDLE aOverlayHandles[VENC_OVERLAY_REGION];/*overlay region handle*/
    OVERLAY_ATTR_S astAttr[VENC_OVERLAY_REGION];/*overlay region handle*/
} OVERLAY_S;

typedef struct hiCOVEREX_S
{       
    HI_U32 u32CoverExNum;/*extra cover region number*/
    REGION_HANDLE aCoverExHandles[VI_COVEREX_REGION];/*extra cover region handle*/
    COVEREX_ATTR_S astAttr[VI_COVEREX_REGION];/*extra cover region handle*/
} COVEREX_S;

#define REGION_MAX_BMP_UPD_NUM 16

typedef struct hiREGION_BMP_UPD_S
{
    POINT_S             stPoint;
    BITMAP_S            stBmp;
    HI_U32              u32Stride;
} REGION_BMP_UPD_S;

typedef struct hiREGION_BMP_UPD_CFG_S
{
    HI_U32              u32BmpCnt;
    REGION_BMP_UPD_S    astBmpUpd[REGION_MAX_BMP_UPD_NUM];
} REGION_BMP_UPD_CFG_S;


typedef union hiREGION_CTRL_PARAMETER_U
{
    /* for REGION_SET_LAYER */
    HI_U32 u32Layer;

    /* for REGION_SET_ALPHA0,REGION_SET_ALPHA1 or REGION_SET_GLOBAL_ALPHA */
    HI_U32 u32Alpha;

    /* for REGION_SET_COLOR */
    HI_U32 u32Color;

    /* for REGION_SET_POSTION */
    POINT_S stPoint;

    /* for REGION_SET_SIZE */
    DIMENSION_S stDimension;

    /* for REGION_SET_BITMAP */
    BITMAP_S stBitmap;

    /* for REGION_UPD_BITMAP*/
    REGION_BMP_UPD_CFG_S stBmpUpdate;
    
    /* for REGION_GET_SIGNLE_ATTR */
    REGION_ATTR_S stRegionAttr;
    
    /* for REGION_GET_ALL_COVER_ATTR */
    COVER_S stCovers;

    /* for REGION_GET_ALL_OVERLAY_ATTR */
    OVERLAY_S stOverlays;
}REGION_CTRL_PARAM_U;

typedef struct hiPIC_SCALE_TASK_S
{
    HI_U32 u32TaskId;               /* used to identify a task */
    VIDEO_FRAME_INFO_S stSrcPic;    /* source picture */
    VIDEO_FRAME_INFO_S stDesPic;    /* destination picture */
} PIC_SCALE_TASK_S;


/* invlalid device ID */
#define HI_ERR_VPP_INVALID_DEVID     HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define HI_ERR_VPP_INVALID_CHNID     HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define HI_ERR_VPP_ILLEGAL_PARAM     HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/* channel exists */
#define HI_ERR_VPP_EXIST             HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/*UN exist*/
#define HI_ERR_VPP_UNEXIST           HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
/* using a NULL point */
#define HI_ERR_VPP_NULL_PTR          HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define HI_ERR_VPP_NOT_CONFIG        HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define HI_ERR_VPP_NOT_SURPPORT      HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SURPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define HI_ERR_VPP_NOT_PERM          HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define HI_ERR_VPP_NOMEM             HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
/* failure caused by malloc buffer */
#define HI_ERR_VPP_NOBUF             HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
/* no data in buffer */
#define HI_ERR_VPP_BUF_EMPTY         HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
/* no buffer for new data */
#define HI_ERR_VPP_BUF_FULL          HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
/* bad address, eg. used for copy_from_user & copy_to_user */
#define HI_ERR_VPP_BADADDR           HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_BADADDR)
/* resource is busy, eg. destroy a venc chn without unregistering it */
#define HI_ERR_VPP_BUSY              HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)

/* System is not ready,maybe not initialed or loaded.
 * Returning the error code when opening a device file failed.
 */
#define HI_ERR_VPP_NOTREADY          HI_DEF_ERR(HI_ID_VPP, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __HI_COMM_VPP_H__ */

