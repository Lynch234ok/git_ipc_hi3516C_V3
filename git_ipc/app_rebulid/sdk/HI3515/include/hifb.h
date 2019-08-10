/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hifb.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/03/12
  Description   :
  History       :
  1.Date        : 2009/03/12
    Author      : H2 MPP TEAM
    Modification: Created file

******************************************************************************/

#ifndef __HIFB_H__
#define __HIFB_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#define IOC_TYPE_HIFB               'F'
#define FBIOGET_COLORKEY_HIFB       _IOR(IOC_TYPE_HIFB, 90, HIFB_COLORKEY_S)
#define FBIOPUT_COLORKEY_HIFB       _IOW(IOC_TYPE_HIFB, 91, HIFB_COLORKEY_S)
#define FBIOGET_ALPHA_HIFB          _IOR(IOC_TYPE_HIFB, 92, HIFB_ALPHA_S)
#define FBIOPUT_ALPHA_HIFB          _IOW(IOC_TYPE_HIFB, 93, HIFB_ALPHA_S)
#define FBIOGET_SCREEN_ORIGIN_HIFB  _IOR(IOC_TYPE_HIFB, 94, HIFB_POINT_S)
#define FBIOPUT_SCREEN_ORIGIN_HIFB  _IOW(IOC_TYPE_HIFB, 95, HIFB_POINT_S)
#define FBIOGET_DEFLICKER_HIFB      _IOR(IOC_TYPE_HIFB, 98, HIFB_DEFLICKER_S)
#define FBIOPUT_DEFLICKER_HIFB      _IOW(IOC_TYPE_HIFB, 99, HIFB_DEFLICKER_S)

#define FBIOGET_VBLANK_HIFB         _IO (IOC_TYPE_HIFB, 100)

#define FBIOPUT_SHOW_HIFB           _IOW(IOC_TYPE_HIFB, 101, HI_BOOL)
#define FBIOGET_SHOW_HIFB           _IOR(IOC_TYPE_HIFB, 102, HI_BOOL)

#define FBIOGET_CAPABILITY_HIFB     _IOR(IOC_TYPE_HIFB, 103, HIFB_CAPABILITY_S)
#define FBIOFLIP_SURFACE            _IOW(IOC_TYPE_HIFB, 104, HIFB_SURFACE_S)

#define FBIOPUT_CURSOR_HI3511       _IOW(IOC_TYPE_HIFB, 105, HI3511_CURSOR_S)
#define FBIOGET_CURSOR_HI3511       _IOR(IOC_TYPE_HIFB, 106, HI3511_CURSOR_S)


/*hifb extend*/
typedef struct
{
    HI_BOOL bKeyEnable;         /* colorkey enable flag */
    HI_BOOL bMaskEnable;        /* key mask enable flag */
    HI_U32 u32Key;              /* colorkey value, maybe contains alpha */
    HI_U8 u8RedMask;            /* red mask */
    HI_U8 u8GreenMask;          /* green mask */
    HI_U8 u8BlueMask;           /* blue mask */
    HI_U8 u8Reserved;
}HIFB_COLORKEY_S;

typedef struct
{
    HI_U32 u32PosX;         /* horizontal position */
    HI_U32 u32PosY;         /* vertical position */
}HIFB_POINT_S;

typedef struct hiHIFB_DEFLICKER_S
{
    HI_U32  u32HDfLevel;    /* horizontal deflicker level */
    HI_U32  u32VDfLevel;    /* vertical deflicker level */
    HI_U8   *pu8HDfCoef;    /* horizontal deflicker coefficient */
    HI_U8   *pu8VDfCoef;    /* vertical deflicker coefficient */
}HIFB_DEFLICKER_S;

typedef struct
{
    HI_BOOL bAlphaEnable;   /* alpha enable flag */
    HI_BOOL bAlphaChannel;  /* alpha channel enable flag */
    HI_U8 u8Alpha0;         /* alpha0 value, used in ARGB1555 */
    HI_U8 u8Alpha1;         /* alpha1 value, used in ARGB1555 */
    HI_U8 u8GlobalAlpha;    /* global alpha value */
    HI_U8 u8Reserved;
}HIFB_ALPHA_S;

typedef enum
{
    HIFB_FMT_1BPP = 0,      /* 1bpp */
    HIFB_FMT_2BPP,          /* 2bpp */
    HIFB_FMT_4BPP,          /* 4bpp */
    HIFB_FMT_8BPP,          /* 8bpp */
    HIFB_FMT_KRGB444,       /* RGB444 */
    HIFB_FMT_KRGB555,       /* RGB555 */
    HIFB_FMT_RGB565,        /* RGB565 */
    HIFB_FMT_ARGB4444,      /* RGB4444 */
    HIFB_FMT_ARGB1555,      /* RGB1555 */
    HIFB_FMT_KRGB888,       /* RGB888 */
    HIFB_FMT_ARGB8888,      /* RGB8888 */
    HIFB_FMT_BUTT
}HIFB_COLOR_FMT_E;

typedef struct
{
    HI_BOOL bKeyAlpha;              /* whether support colorkey alpha */
    HI_BOOL bGlobalAlpha;           /* whether support global alpha */
    HI_BOOL bCmap;                  /* whether support color map */
    HI_BOOL bColFmt[HIFB_FMT_BUTT]; /* support which color format */
    HI_U32  u32MaxWidth;            /* the max pixels per line */
    HI_U32  u32MaxHeight;           /* the max lines */
    HI_U32  u32MinWidth;            /* the min pixels per line */
    HI_U32  u32MinHeight;           /* the min lines */
    HI_U32  u32VDefLevel;           /* vertical deflicker level, 0 means vertical deflicker is unsupported */
    HI_U32  u32HDefLevel;           /* horizontal deflicker level, 0 means horizontal deflicker is unsupported */
}HIFB_CAPABILITY_S;

typedef struct
{
    HI_VOID* pvPhyAddr;         /* addr of the frame */
    HIFB_ALPHA_S stAlpha;       /* alpha properties */
    HIFB_COLORKEY_S stColorkey; /* colorkey properties */
}HIFB_SURFACE_S;

typedef enum
{
    HI3511_CURSOR_2COLOR = 0,
    HI3511_CURSOR_3COLOR,
    HI3511_CURSOR_4COLOR
}HI3511_CURSOR_E;

typedef struct
{
    HI_U8 u8Cr;
    HI_U8 u8Cb;
    HI_U8 u8Y;
    HI_U8 u8Reserved;
}HIFB_YCBCR_S;

typedef struct
{
    HI3511_CURSOR_E enCursor;
    HIFB_YCBCR_S stYCbCr[4];
}HI3511_CURSOR_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HIFB_H__ */

