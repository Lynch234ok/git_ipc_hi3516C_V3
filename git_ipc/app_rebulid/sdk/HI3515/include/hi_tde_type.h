/******************************************************************************

  Copyright (C), 2001-2011, Huawei Tech. Co., Ltd.

 ******************************************************************************
 File Name     : tde_type.h
Version       : Initial Draft
Author        : w54130
Created       : 2007/5/21
Last Modified :
Description   : TDE public type
Function List :
History       :
1.Date        : 2007/5/21
Author      : w54130
Modification: Created file

2.Date       :2008/2/17
Author       : w54130
Modification :Add TDE2_COLORSPACECONVERSION_MODE_E Macro

 ******************************************************************************/
#ifndef __TDE_TYPE_H__
#define __TDE_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_tde_errcode.h"

/****************************************************************************/
/*                             TDE2 types define                             */
/****************************************************************************/

typedef HI_S32 TDE_HANDLE;

typedef HI_VOID (* TDE_FUNC_CB) (HI_VOID *);

typedef HI_U32 (* TDE_CB_MALLOC) (HI_U32 size, HI_U32* pu32PrvData);

typedef HI_VOID (* TDE_CB_FREE) (HI_U32 phyAddr,HI_U32 u32PrvData);

/* 颜色格式 */
typedef enum hiTDE2_COLOR_FMT_E
{
    TDE2_COLOR_FMT_RGB444 = 0,
    TDE2_COLOR_FMT_RGB555,
    TDE2_COLOR_FMT_RGB565,
    TDE2_COLOR_FMT_RGB888,
    TDE2_COLOR_FMT_ARGB4444,
    TDE2_COLOR_FMT_ARGB1555,
    TDE2_COLOR_FMT_ARGB8565,
    TDE2_COLOR_FMT_ARGB8888,
    TDE2_COLOR_FMT_CLUT1,
    TDE2_COLOR_FMT_CLUT2,
    TDE2_COLOR_FMT_CLUT4,
    TDE2_COLOR_FMT_CLUT8,
    TDE2_COLOR_FMT_ACLUT44,
    TDE2_COLOR_FMT_ACLUT88,
    TDE2_COLOR_FMT_A1,
    TDE2_COLOR_FMT_A8,
    TDE2_COLOR_FMT_YCbCr888,
    TDE2_COLOR_FMT_AYCbCr8888,
    TDE2_COLOR_FMT_YCbCr422,
    TDE2_COLOR_FMT_byte,
    TDE2_COLOR_FMT_halfword,
    TDE2_COLOR_FMT_BUTT
} TDE2_COLOR_FMT_E;

typedef enum hiTDE2_MB_COLORFMT_E
{
    TDE2_MB_COLOR_FMT_JPG_YCbCr400MBP = 0,
    TDE2_MB_COLOR_FMT_JPG_YCbCr422MBHP,
    TDE2_MB_COLOR_FMT_JPG_YCbCr422MBVP,
    TDE2_MB_COLOR_FMT_MP1_YCbCr420MBP,
    TDE2_MB_COLOR_FMT_MP2_YCbCr420MBP,
    TDE2_MB_COLOR_FMT_MP2_YCbCr420MBI,
    TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP,
    TDE2_MB_COLOR_FMT_JPG_YCbCr444MBP,
    TDE2_MB_COLOR_FMT_BUTT
} TDE2_MB_COLOR_FMT_E;


typedef enum hiTDE2_COLORSPACE_CONV_MODE_E
{
    TDE2_ITU_R_BT601_IMAGE = 0,
    TDE2_ITU_R_BT709_IMAGE,
    TDE2_ITU_R_BT601_VIDEO,
    TDE2_ITU_R_BT709_VIDEO
}TDE2_COLORSPACE_CONV_MODE_E;

typedef enum
{
    TDE2_MBFILL_YC = 0, /* 亮度、色度块都填充 */
    TDE2_MBFILL_Y,      /* 只填充亮度块 */
    TDE2_MBFILL_C,      /* 只填充色度块 */
}TDE2_MBFILL_E;

/* 帧/场处理模式 */
typedef enum hiTDE_PIC_MODE_E
{
    TDE_FRAME_PIC_MODE = 0,     /* 帧处理模式 */
    TDE_BOTTOM_FIELD_PIC_MODE,  /* 底场处理模式 */
    TDE_TOP_FIELD_PIC_MODE,     /* 顶场处理模式 */
    TDE_TB_FIELD_PIC_MODE,      /* 顶底场处理模式 */
    TDE_PIC_MODE_BUTT
} TDE_PIC_MODE_E;


/* 用户设置的位图信息结构 */
typedef struct hiTDE2_SURFACE_S
{
    /* 位图首地址 */
    HI_U32 u32PhyAddr;

    /* 颜色格式 */
    TDE2_COLOR_FMT_E enColorFmt;

    /* 位图高度 */
    HI_U32 u32Height;

    /* 位图宽度 */
    HI_U32 u32Width;

    /* 位图跨度 */
    HI_U32 u32Stride;

    /*Clut表地址,用作颜色扩展或颜色校正*/
    HI_U8* pu8ClutPhyAddr;

    /* Clut表是否位于YCbCr空间 */
    HI_BOOL bYCbCrClut;

    /* 位图alpha最大值为255还是128 */
    HI_BOOL bAlphaMax255;

    /* Alpha0、Alpha1值，用作ARGB1555格式 */
    HI_BOOL bAlphaExt1555; /* 是否使能1555的Alpha扩展 */
    HI_U8 u8Alpha0;
    HI_U8 u8Alpha1;

    TDE2_COLORSPACE_CONV_MODE_E enColorSpaceConv;  /* 颜色空间转换矩阵 */
   
} TDE2_SURFACE_S;

typedef struct hiTDE2_MB_S
{
    TDE2_MB_COLOR_FMT_E enMbFmt;
    HI_U32              u32YPhyAddr;
    HI_U32              u32YWidth;
    HI_U32              u32YHeight;
    HI_U32              u32YStride;
    HI_U32              u32CbCrPhyAddr;
    HI_U32              u32CbCrStride;

    TDE2_COLORSPACE_CONV_MODE_E enColorSpaceConv;  /* 颜色空间转换矩阵 */
    TDE_PIC_MODE_E      enPicMode;    
} TDE2_MB_S;

typedef struct hiTDE2_RECT_S
{
    HI_S32 s32Xpos;
    HI_S32 s32Ypos;
    HI_U32 u32Width;
    HI_U32 u32Height;
} TDE2_RECT_S;

/* 逻辑运算方式 */
typedef enum hiTDE2_ALUCMD_E
{
    TDE2_ALUCMD_NONE = 0, 
    TDE2_ALUCMD_BLEND,            /*Alpha混合,源2没有预乘模式*/
    TDE2_ALUCMD_BLEND_PREMUL,     /*Alpha混合,源2已经预乘模式*/
    TDE2_ALUCMD_ROP,
    TDE2_ALUCMD_BUTT
} TDE2_ALUCMD_E;

typedef enum hiTDE2_ROP_CODE_E
{
    TDE2_ROP_BLACK = 0,         /*Blackness*/
    TDE2_ROP_NOTMERGEPEN,   /*~(S2+S1)*/
    TDE2_ROP_MASKNOTPEN,    /*~S2&S1*/
    TDE2_ROP_NOTCOPYPEN,    /* ~S2*/
    TDE2_ROP_MASKPENNOT,    /* S2&~S1 */
    TDE2_ROP_NOT,           /* ~S1 */
    TDE2_ROP_XORPEN,        /* S2^S1 */
    TDE2_ROP_NOTMASKPEN,    /* ~(S2&S1) */
    TDE2_ROP_MASKPEN,       /* S2&S1 */
    TDE2_ROP_NOTXORPEN,     /* ~(S2^S1) */
    TDE2_ROP_NOP,           /* S1 */
    TDE2_ROP_MERGENOTPEN,   /* ~S2+S1 */
    TDE2_ROP_COPYPEN,       /* S2 */
    TDE2_ROP_MERGEPENNOT,   /* S2+~S1 */
    TDE2_ROP_MERGEPEN,      /* S2+S1 */
    TDE2_ROP_WHITE,         /* Whiteness */
    TDE2_ROP_BUTT
} TDE2_ROP_CODE_E;

typedef enum hiTDE2_MIRROR_E
{
    TDE2_MIRROR_NONE = 0,
    TDE2_MIRROR_HORIZONTAL,
    TDE2_MIRROR_VERTICAL,
    TDE2_MIRROR_BOTH,
    TDE2_MIRROR_BUTT
} TDE2_MIRROR_E;

/*Clip操作类型*/
typedef enum hiTDE2_CLIPMODE_E
{
    TDE2_CLIPMODE_NONE = 0,
    TDE2_CLIPMODE_INSIDE,
    TDE2_CLIPMODE_OUTSIDE,
    TDE2_CLIPMODE_BUTT
} TDE2_CLIPMODE_E;

/*宏块格式缩放类型*/
typedef enum hiTDE2_MBRESIZE_E
{
    TDE2_MBRESIZE_NONE = 0,
    TDE2_MBRESIZE_QUALITY_LOW,
    TDE2_MBRESIZE_QUALITY_MIDDLE,
    TDE2_MBRESIZE_QUALITY_HIGH,
    TDE2_MBRESIZE_BUTT
} TDE2_MBRESIZE_E;

typedef struct hiTDE2_FILLCOLOR_S
{
    TDE2_COLOR_FMT_E enColorFmt;
    HI_U32           u32FillColor;
} TDE2_FILLCOLOR_S;

typedef enum hiTDE2_COLORKEY_MODE_E
{
    TDE2_COLORKEY_MODE_NONE = 0,     /* 不做color key */
    TDE2_COLORKEY_MODE_FOREGROUND,   /* 对前景位图进行color key，说明:对于颜色扩展，在CLUT前做color key；
                                        对于颜色校正:在CLUT后做color key */
    TDE2_COLORKEY_MODE_BACKGROUND,    /* 对背景位图进行color key*/
    TDE2_COLORKEY_MODE_BUTT
} TDE2_COLORKEY_MODE_E;

typedef struct hiTDE2_COLORKEY_COMP_S
{
    HI_U8 u8CompMin;           /* 分量最小值*/
    HI_U8 u8CompMax;           /* 分量最大值*/
    HI_U8 bCompOut;            /* 分量关键色在范围内/范围外*/
    HI_U8 bCompIgnore;         /* 分量是否忽略*/
} TDE2_COLORKEY_COMP_S;

typedef union hiTDE2_COLORKEY_U
{
    struct
    {
        TDE2_COLORKEY_COMP_S stAlpha;
        TDE2_COLORKEY_COMP_S stRed;
        TDE2_COLORKEY_COMP_S stGreen;
        TDE2_COLORKEY_COMP_S stBlue;
    } struCkARGB;
    struct
    {
        TDE2_COLORKEY_COMP_S stAlpha;
        TDE2_COLORKEY_COMP_S stY;
        TDE2_COLORKEY_COMP_S stCb;
        TDE2_COLORKEY_COMP_S stCr;
    } struCkYCbCr;
    struct
    {
        TDE2_COLORKEY_COMP_S stAlpha;
        TDE2_COLORKEY_COMP_S stClut;
    } struCkClut;
} TDE2_COLORKEY_U;

typedef enum hiTDE2_OUTALPHA_FROM_E
{
    TDE2_OUTALPHA_FROM_NORM = 0,    /* 来源于alpha blending的结果或者抗闪烁的结果 */
    TDE2_OUTALPHA_FROM_BACKGROUND,  /* 来源于背景位图 */
    TDE2_OUTALPHA_FROM_FOREGROUND,  /* 来源于前景位图 */
    TDE2_OUTALPHA_FROM_GLOBALALPHA, /* 来源于全局alpha */
    TDE2_OUTALPHA_FROM_BUTT
} TDE2_OUTALPHA_FROM_E;

typedef enum hiTDE2_FILTER_MODE_E
{
    TDE2_FILTER_MODE_COLOR = 0, /* 对颜色进行滤波 */
    TDE2_FILTER_MODE_ALPHA,     /* 对alpha通道滤波 */
    TDE2_FILTER_MODE_BOTH,      /* 对颜色和alpha通道同时滤波 */
    TDE2_FILTER_MODE_BUTT
} TDE2_FILTER_MODE_E;

/*horizontal scale filter coefficient of dsu
which affect image quality of encoding and preview.

normally the filter can be set be DSU_HSCALE_FILTER_DEFAULT
which means sdk will choose filter automatically.Otherwise,
you can choose other filter

Notes:65M means 6.5*/
typedef enum hiTDE2_HFILT_COEF_E
{
	TDE2_HFILT_COEF_Default = 0,
	TDE2_HFILT_COEF_C_65M,	
	TDE2_HFILT_COEF_CG_56M,
	TDE2_HFILT_COEF_LC_45M,
	TDE2_HFILT_COEF_CG_3M,
	TDE2_HFILT_COEF_CG_2M,
	TDE2_HFILT_COEF_CG_1M,
	TDE2_HFILT_COEF_BUTT
}TDE2_HFILT_COEF_E;

/*vertical scale filter coefficient of dsu
which affect image quality of encoding and preview.

normally the filter can be set be DSU_VSCALE_FILTER_DEFAULT
which means sdk will choose filter automatically.Otherwise,
you can choose other filter

Notes:38M means 3.8*/
typedef enum hiTDE2_VFILT_COEF_E
{
	TDE2_VFILT_COEF_Default = 0,
	TDE2_VFILT_COEF_S_6M,    
	TDE2_VFILT_COEF_S_5M,    
	TDE2_VFILT_COEF_S_4M,		 
	TDE2_VFILT_COEF_S_38M,	 
	TDE2_VFILT_COEF_S_37M,	 
	TDE2_VFILT_COEF_S_36M,	 
	TDE2_VFILT_COEF_S_25M,	 
	TDE2_VFILT_COEF_S_2M,		 
	TDE2_VFILT_COEF_S_15M,	 
	TDE2_VFILT_COEF_S_12M,	 
	TDE2_VFILT_COEF_S_11M,	 
	TDE2_VFILT_COEF_S_1M,	
	TDE2_VFILT_COEF_BUTT
}TDE2_VFILT_COEF_E;

/*滤波系数类型
 每种类型的滤波系数都有6组水平和12组垂直系数
 不同类型的滤波系数滤波效果不同*/
typedef enum hiTDE2_FILT_COEF_TYPE_E
{
	TDE2_FILT_COEF_TYPE_NORM = 0,      /*通用滤波类型，适用于普通滤波任务*/
	TDE2_FILT_COEF_TYPE_EX,			   /*扩展滤波类型，由vi滤波系数与norm类系数卷积得来*/
	TDE2_FILT_COEF_TYPE_EX2,		   /*扩展滤波类型2*/
	TDE2_FILT_COEF_TYPE_USER1,          /*用户自定义滤波系数1*/
	TDE2_FILT_COEF_TYPE_USER2,          /*用户自定义滤波系数2*/
	TDE2_FILT_COEF_TYPE_BUTT
}TDE2_FILT_COEF_TYPE_E;

typedef struct hiTDE2_FILT_COEF_S
{
	TDE2_FILT_COEF_TYPE_E enFilterType; /*滤波系数类型*/
	TDE2_HFILT_COEF_E enHFilterL;     /*亮度的水平滤波系数*/
	TDE2_HFILT_COEF_E enHFilterC;	  /*色度的水平滤波系数*/
	TDE2_VFILT_COEF_E enVFilterL;     /*亮度的垂直滤波系数*/
	TDE2_VFILT_COEF_E enVFilterC;     /*色度的垂直滤波系数*/
}TDE2_FILT_COEF_S;

typedef struct hiTDE2_FILTER_PARAM_S
{
    TDE2_FILT_COEF_TYPE_E enFiltType;
    HI_U8 *pu8HParamTable;
    HI_U8 *pu8VParamTable;
}TDE2_FILTER_PARAM_S;

/*滤波扩展属性*/
typedef struct hiTDE2_FILT_ATTR_EX_S
{
	HI_BOOL bForceHFilt;		/*是否强制水平滤波*/
	HI_BOOL bForceVFilt;		/*是否强制垂直滤波*/
}TDE2_FILT_ATTR_EX_S;

typedef struct hiTDE2_OPT_S
{
    /*逻辑运算类型*/
    TDE2_ALUCMD_E enAluCmd;

    /*颜色空间ROP类型*/
    TDE2_ROP_CODE_E enRopCode_Color;

    /*Alpha的ROP类型*/
    TDE2_ROP_CODE_E enRopCode_Alpha;

    /*color key方式*/
    TDE2_COLORKEY_MODE_E enColorKeyMode;

    /*color key设置值*/
    TDE2_COLORKEY_U unColorKeyValue;

    /*区域内作clip还是区域外作clip*/
    TDE2_CLIPMODE_E enClipMode;

    /*clip区域定义*/
    TDE2_RECT_S stClipRect;

    /*是否抗闪烁*/
    HI_BOOL bDeflicker;

    /*是否缩放*/
    HI_BOOL bResize;

    /* 缩放或deflicker时使用的滤波模式 */
    TDE2_FILTER_MODE_E enFilterMode;

    /*镜像类型*/
    TDE2_MIRROR_E enMirror;

    /*是否重新加载Clut表*/
    HI_BOOL bClutReload;

    /*全局Alpha值*/
    HI_U8   u8GlobalAlpha;

    /*输出alpha来源*/
    TDE2_OUTALPHA_FROM_E enOutAlphaFrom;
} TDE2_OPT_S;

typedef struct hiTDE2_MBOPT_S
{
    /*Clip模式选择*/
    TDE2_CLIPMODE_E enClipMode;

    /*clip区域定义*/
    TDE2_RECT_S stClipRect;

    /*是否抗闪烁*/
    HI_BOOL bDeflicker;

    /*缩放信息*/
    TDE2_MBRESIZE_E enResize;

    /*如果不设置Alpha,则默认输出最大Alpha值*/
    HI_BOOL bSetOutAlpha;
    HI_U8   u8OutAlpha;

	/*滤波系数选择*/
	TDE2_FILT_COEF_S stFiltCoef;

	/* 强制滤波选项*/
	HI_BOOL bForceHFilt;  	/*1:1搬移时是否强制水平滤波*/
	HI_BOOL bForceVFilt;	/*1:1搬移时是否强制垂直滤波*/

} TDE2_MBOPT_S;

typedef enum hiTDE_ROTATE_ANGLE_E
{
    TDE_ROTATE_CLOCKWISE_90 = 0,    /* 顺时针旋转90度 */
    TDE_ROTATE_CLOCKWISE_180,   /* 顺时针旋转180度 */
    TDE_ROTATE_CLOCKWISE_270,   /* 顺时针旋转270度 */
    TDE_ROTATE_BUTT
} TDE_ROTATE_ANGLE_E;


typedef struct hiTDE_BITBLIT_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_SURFACE_S stBackGround;
    TDE2_RECT_S    stBackGroundRect;
    TDE2_SURFACE_S stForeGround;
    TDE2_RECT_S    stForeGroundRect;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
    TDE2_OPT_S     stOpt;
    HI_U32         u32NullIndicator;
} TDE_BITBLIT_CMD_S;

typedef struct hiTDE_SOLIDDRAW_CMD_S
{
    TDE_HANDLE       s32Handle;
    TDE2_SURFACE_S   stForeGround;
    TDE2_RECT_S      stForeGroundRect;
    TDE2_SURFACE_S   stDst;
    TDE2_RECT_S      stDstRect;
    TDE2_FILLCOLOR_S stFillColor;
    TDE2_OPT_S       stOpt;
    HI_U32           u32NullIndicator;
} TDE_SOLIDDRAW_CMD_S;

typedef struct hiTDE_QUICKCOPY_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_SURFACE_S stSrc;
    TDE2_RECT_S    stSrcRect;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
} TDE_QUICKCOPY_CMD_S;

typedef struct hiTDE_QUICKFILL_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
    HI_U32         u32FillData;
} TDE_QUICKFILL_CMD_S;

typedef struct hiTDE_QUICKDEFLICKER_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_SURFACE_S stSrc;
    TDE2_RECT_S    stSrcRect;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
} TDE_QUICKDEFLICKER_CMD_S;

typedef struct hiTDE_QUICKRESIZE_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_SURFACE_S stSrc;
    TDE2_RECT_S    stSrcRect;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
} TDE_QUICKRESIZE_CMD_S;

typedef struct hiTDE_MBBITBLT_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_MB_S      stMB;
    TDE2_RECT_S    stMbRect;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
    TDE2_MBOPT_S   stMbOpt;
} TDE_MBBITBLT_CMD_S;

typedef struct hiTDE_ENDJOB_CMD_S
{
    TDE_HANDLE s32Handle;
    HI_BOOL    bSync;
    HI_BOOL    bBlock;
    HI_U32     u32TimeOut;
} TDE_ENDJOB_CMD_S;

typedef struct hiTDE_BITMAP_MASKROP_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_SURFACE_S stBackGround;
    TDE2_RECT_S    stBackGroundRect;
    TDE2_SURFACE_S stForeGround;
    TDE2_RECT_S    stForeGroundRect;
    TDE2_SURFACE_S stMask;
    TDE2_RECT_S    stMaskRect;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
    TDE2_ROP_CODE_E enRopCode_Color;
    TDE2_ROP_CODE_E enRopCode_Alpha;
} TDE_BITMAP_MASKROP_CMD_S;

typedef struct hiTDE_BITMAP_MASKBLEND_CMD_S
{
    TDE_HANDLE     s32Handle;
    TDE2_SURFACE_S stBackGround;
    TDE2_RECT_S    stBackGroundRect;
    TDE2_SURFACE_S stForeGround;
    TDE2_RECT_S    stForeGroundRect;
    TDE2_SURFACE_S stMask;
    TDE2_RECT_S    stMaskRect;
    TDE2_SURFACE_S stDst;
    TDE2_RECT_S    stDstRect;
    HI_U8   u8Alpha;
    TDE2_ALUCMD_E enBlendMode;
}TDE_BITMAP_MASKBLEND_CMD_S;


typedef enum 
{
    TDE2_MBRESIZE_HIGH = 0,  /* 输出为光栅时使用，宏块不需要 */
    TDE2_MBRESIZE_MIDDLE,    /* 输出为光栅时使用，宏块不需要 */
    TDE2_OSD2MB_OUT42X,      /* 输出为YCbCr444时不需要中间内存 */
    TDE2_MASK_ROP,
    TDE2_MASK_BLEND,
    TDE2_CMD_BUTT
}TDE2_CMD_E;


typedef struct hiTDE_MB2MB_CMD_S
{
    TDE_HANDLE s32Handle;
    TDE2_MB_S stMBIn;
    TDE2_RECT_S stInRect;
    TDE2_MB_S stMBOut;
    TDE2_RECT_S stOutRect;
    TDE2_MBOPT_S stMbOpt;
}TDE_MB2MB_CMD_S;

typedef struct hiTDE_OSD2MB_CMD_S
{
    TDE_HANDLE s32Handle;
    TDE2_MB_S stBackGround;
    TDE2_RECT_S stBackGroundRect;
    TDE2_SURFACE_S  stForeGround;
    TDE2_RECT_S stForeGroundRect;
    TDE2_MB_S stMBOut;
    TDE2_RECT_S stOutRect;
    TDE2_OPT_S stOpt;
}TDE_OSD2MB_CMD_S;

typedef struct hiTDE_MBFILL_CMD_S
{
    TDE_HANDLE s32Handle;
    TDE2_MB_S stMb;
    HI_U32 u32YFill;
    HI_U32 u32CFill;
    TDE2_MBFILL_E eMbFill;
}TDE_MBFILL_CMD_S;

/*tde任务使用的中间临时buff分配方式*/
typedef enum hiTDE_TEMPBUFF_ALLOC_TYPE
{
	TEMPBUFF_ALLOC_AUTO = 0,      /*内部自动分配*/
	TEMPBUFF_ALLOC_BYUSER,		  /*调用者分配*/
	TEMPBUFF_ALLOC_BUTT
}TDE_TEMPBUFF_ALLOC_TYPE;

/*beginJob 属性*/
typedef struct hiTDE_JOB_ATTR
{
	HI_BOOL bUseTpl;			 /*该job是否使用模版方式生成*/
	TDE_TEMPBUFF_ALLOC_TYPE enTempBuffType; /*该job用到的临时buff分配方式*/
	HI_U32 u32TempBuffSize;      /*临时buff大小，enTempBuffType为TEMPBUFF_ALLOC_BUTT有效*/
}TDE_JOB_ATTR;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __TDE_TYPE_H__ */


