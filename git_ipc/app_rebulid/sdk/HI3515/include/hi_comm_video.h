/******************************************************************************* 
  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
 File Name     : hi_comm_video.h
 Version       : Initial Draft
 Author        : c42025
 Created       : 2006/11/09
 Description   : 
 History       :
 1.Date        : 2006/11/03
   Author      : c42025
   Modification: Created file

 2.Date        : 2007/12/11
   Author      : c42025
   Modification: delelte all about digital watermark 

 3.Date        :   2008/10/31
   Author      :   z44949
   Modification:   Translate the chinese comment
******************************************************************************/

#ifndef __HI_COMM_VIDEO_H__
#define __HI_COMM_VIDEO_H__

#include "hi_type.h"
#include "hi_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef enum hiPIC_SIZE_E
{
    PIC_QCIF = 0,
    PIC_CIF,    
    PIC_2CIF,    
    PIC_HD1, 
    PIC_D1,
    
    PIC_QVGA,    /* 320 * 240 */
    PIC_VGA,     /* 640 * 480 */    
    PIC_XGA,     /* 1024 * 768 */   
    PIC_SXGA,    /* 1400 * 1050 */    
    PIC_UXGA,    /* 1600 * 1200 */    
    PIC_QXGA,    /* 2048 * 1536 */

    PIC_WVGA,    /* 854 * 480 */
    PIC_WSXGA,   /* 1680 * 1050 */      
    PIC_WUXGA,   /* 1920 * 1200 */
    PIC_WQXGA,   /* 2560 * 1600 */
    
    PIC_HD720,   /* 1280 * 720 */
    PIC_HD1080,  /* 1920 * 1080 */
    
    PIC_BUTT
}PIC_SIZE_E;

typedef enum hiVIDEO_NORM_E
{
	VIDEO_ENCODING_MODE_PAL=0,
	VIDEO_ENCODING_MODE_NTSC,
	VIDEO_ENCODING_MODE_AUTO,
	VIDEO_ENCODING_MODE_BUTT
} VIDEO_NORM_E;

typedef enum hiVIDEO_CONTROL_MODE_E
{
	VIDEO_CONTROL_MODE_SLAVER=0,
	VIDEO_CONTROL_MODE_MASTER,
	VIDEO_CONTROL_MODE_BUTT
}VIDEO_CONTROL_MODE_E;


/* we ONLY define picture format used, all unused will be deleted!*/
typedef enum hiPIXEL_FORMAT_E
{   
	PIXEL_FORMAT_RGB_1BPP = 0,
	PIXEL_FORMAT_RGB_2BPP,
	PIXEL_FORMAT_RGB_4BPP,
	PIXEL_FORMAT_RGB_8BPP,
	PIXEL_FORMAT_RGB_444,
	PIXEL_FORMAT_RGB_4444,
	PIXEL_FORMAT_RGB_555,
	PIXEL_FORMAT_RGB_565,
	PIXEL_FORMAT_RGB_1555,

	/*  9 reserved */
	PIXEL_FORMAT_RGB_888,
	PIXEL_FORMAT_RGB_8888,
	PIXEL_FORMAT_RGB_PLANAR_888,
	PIXEL_FORMAT_RGB_BAYER,

	PIXEL_FORMAT_YUV_A422,
	PIXEL_FORMAT_YUV_A444,

	PIXEL_FORMAT_YUV_PLANAR_422,
	PIXEL_FORMAT_YUV_PLANAR_420,
	PIXEL_FORMAT_YUV_PLANAR_444,

	PIXEL_FORMAT_YUV_SEMIPLANAR_422,
	PIXEL_FORMAT_YUV_SEMIPLANAR_420,
	PIXEL_FORMAT_YUV_SEMIPLANAR_444,

	PIXEL_FORMAT_UYVY_PACKAGE_422,
	PIXEL_FORMAT_YUYV_PACKAGE_422,
	PIXEL_FORMAT_VYUY_PACKAGE_422,
	PIXEL_FORMAT_YCbCr_PLANAR,

	PIXEL_FORMAT_BUTT   
} PIXEL_FORMAT_E;

typedef struct hiVIDEO_VBI_INFO_S
{
    HI_U32 au32Data[VIU_MAX_VBI_LEN];
    HI_U32 u32Len;
}VIDEO_VBI_INFO_S;

typedef enum hiVIDEO_FIELD_E
{
    VIDEO_FIELD_TOP         = 0x01,    /* even field */
    VIDEO_FIELD_BOTTOM      = 0x02,    /* odd field */
    VIDEO_FIELD_INTERLACED  = 0x03,    /* two interlaced fields */
    VIDEO_FIELD_FRAME       = 0x04,    /* frame */

    VIDEO_FIELD_BUTT
} VIDEO_FIELD_E;

typedef struct hiVIDEO_FRAME_S
{	
	HI_U32          u32Width;
	HI_U32          u32Height;    
	VIDEO_FIELD_E   u32Field;
    PIXEL_FORMAT_E  enPixelFormat;

	HI_U32          u32PhyAddr[3];
	HI_VOID         *pVirAddr[3];
	HI_U32          u32Stride[3];

	HI_U16          u16OffsetTop;	   /* top offset of show area */
	HI_U16          u16OffsetBottom;   /* bottom offset of show area */
	HI_U16          u16OffsetLeft;	   /* left offset of show area */
	HI_U16          u16OffsetRight;    /* right offset of show area */

	HI_U64          u64pts;
	HI_U32          u32TimeRef;

	HI_U32          u32PrivateData;
    VIDEO_VBI_INFO_S astVbiInfo[VIU_MAX_VBI_NUM];
}VIDEO_FRAME_S;

typedef struct hiVIDEO_FRAME_INFO_S
{
    VIDEO_FRAME_S stVFrame;
    HI_U32 u32PoolId;
}VIDEO_FRAME_INFO_S;

typedef struct hiBITMAP_S
{
    PIXEL_FORMAT_E enPixelFormat;  /* Bitmap's pixel format */

    HI_U32 u32Width;               /* Bitmap's width */
    HI_U32 u32Height;              /* Bitmap's height */
    HI_VOID *pData;                /* Address of Bitmap's data */
} BITMAP_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* _HI_COMM_VIDEO_H_ */ 
