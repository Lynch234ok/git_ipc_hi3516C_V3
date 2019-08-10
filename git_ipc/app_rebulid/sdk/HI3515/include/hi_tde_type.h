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

/* ��ɫ��ʽ */
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
    TDE2_MBFILL_YC = 0, /* ���ȡ�ɫ�ȿ鶼��� */
    TDE2_MBFILL_Y,      /* ֻ������ȿ� */
    TDE2_MBFILL_C,      /* ֻ���ɫ�ȿ� */
}TDE2_MBFILL_E;

/* ֡/������ģʽ */
typedef enum hiTDE_PIC_MODE_E
{
    TDE_FRAME_PIC_MODE = 0,     /* ֡����ģʽ */
    TDE_BOTTOM_FIELD_PIC_MODE,  /* �׳�����ģʽ */
    TDE_TOP_FIELD_PIC_MODE,     /* ��������ģʽ */
    TDE_TB_FIELD_PIC_MODE,      /* ���׳�����ģʽ */
    TDE_PIC_MODE_BUTT
} TDE_PIC_MODE_E;


/* �û����õ�λͼ��Ϣ�ṹ */
typedef struct hiTDE2_SURFACE_S
{
    /* λͼ�׵�ַ */
    HI_U32 u32PhyAddr;

    /* ��ɫ��ʽ */
    TDE2_COLOR_FMT_E enColorFmt;

    /* λͼ�߶� */
    HI_U32 u32Height;

    /* λͼ��� */
    HI_U32 u32Width;

    /* λͼ��� */
    HI_U32 u32Stride;

    /*Clut���ַ,������ɫ��չ����ɫУ��*/
    HI_U8* pu8ClutPhyAddr;

    /* Clut���Ƿ�λ��YCbCr�ռ� */
    HI_BOOL bYCbCrClut;

    /* λͼalpha���ֵΪ255����128 */
    HI_BOOL bAlphaMax255;

    /* Alpha0��Alpha1ֵ������ARGB1555��ʽ */
    HI_BOOL bAlphaExt1555; /* �Ƿ�ʹ��1555��Alpha��չ */
    HI_U8 u8Alpha0;
    HI_U8 u8Alpha1;

    TDE2_COLORSPACE_CONV_MODE_E enColorSpaceConv;  /* ��ɫ�ռ�ת������ */
   
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

    TDE2_COLORSPACE_CONV_MODE_E enColorSpaceConv;  /* ��ɫ�ռ�ת������ */
    TDE_PIC_MODE_E      enPicMode;    
} TDE2_MB_S;

typedef struct hiTDE2_RECT_S
{
    HI_S32 s32Xpos;
    HI_S32 s32Ypos;
    HI_U32 u32Width;
    HI_U32 u32Height;
} TDE2_RECT_S;

/* �߼����㷽ʽ */
typedef enum hiTDE2_ALUCMD_E
{
    TDE2_ALUCMD_NONE = 0, 
    TDE2_ALUCMD_BLEND,            /*Alpha���,Դ2û��Ԥ��ģʽ*/
    TDE2_ALUCMD_BLEND_PREMUL,     /*Alpha���,Դ2�Ѿ�Ԥ��ģʽ*/
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

/*Clip��������*/
typedef enum hiTDE2_CLIPMODE_E
{
    TDE2_CLIPMODE_NONE = 0,
    TDE2_CLIPMODE_INSIDE,
    TDE2_CLIPMODE_OUTSIDE,
    TDE2_CLIPMODE_BUTT
} TDE2_CLIPMODE_E;

/*����ʽ��������*/
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
    TDE2_COLORKEY_MODE_NONE = 0,     /* ����color key */
    TDE2_COLORKEY_MODE_FOREGROUND,   /* ��ǰ��λͼ����color key��˵��:������ɫ��չ����CLUTǰ��color key��
                                        ������ɫУ��:��CLUT����color key */
    TDE2_COLORKEY_MODE_BACKGROUND,    /* �Ա���λͼ����color key*/
    TDE2_COLORKEY_MODE_BUTT
} TDE2_COLORKEY_MODE_E;

typedef struct hiTDE2_COLORKEY_COMP_S
{
    HI_U8 u8CompMin;           /* ������Сֵ*/
    HI_U8 u8CompMax;           /* �������ֵ*/
    HI_U8 bCompOut;            /* �����ؼ�ɫ�ڷ�Χ��/��Χ��*/
    HI_U8 bCompIgnore;         /* �����Ƿ����*/
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
    TDE2_OUTALPHA_FROM_NORM = 0,    /* ��Դ��alpha blending�Ľ�����߿���˸�Ľ�� */
    TDE2_OUTALPHA_FROM_BACKGROUND,  /* ��Դ�ڱ���λͼ */
    TDE2_OUTALPHA_FROM_FOREGROUND,  /* ��Դ��ǰ��λͼ */
    TDE2_OUTALPHA_FROM_GLOBALALPHA, /* ��Դ��ȫ��alpha */
    TDE2_OUTALPHA_FROM_BUTT
} TDE2_OUTALPHA_FROM_E;

typedef enum hiTDE2_FILTER_MODE_E
{
    TDE2_FILTER_MODE_COLOR = 0, /* ����ɫ�����˲� */
    TDE2_FILTER_MODE_ALPHA,     /* ��alphaͨ���˲� */
    TDE2_FILTER_MODE_BOTH,      /* ����ɫ��alphaͨ��ͬʱ�˲� */
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

/*�˲�ϵ������
 ÿ�����͵��˲�ϵ������6��ˮƽ��12�鴹ֱϵ��
 ��ͬ���͵��˲�ϵ���˲�Ч����ͬ*/
typedef enum hiTDE2_FILT_COEF_TYPE_E
{
	TDE2_FILT_COEF_TYPE_NORM = 0,      /*ͨ���˲����ͣ���������ͨ�˲�����*/
	TDE2_FILT_COEF_TYPE_EX,			   /*��չ�˲����ͣ���vi�˲�ϵ����norm��ϵ���������*/
	TDE2_FILT_COEF_TYPE_EX2,		   /*��չ�˲�����2*/
	TDE2_FILT_COEF_TYPE_USER1,          /*�û��Զ����˲�ϵ��1*/
	TDE2_FILT_COEF_TYPE_USER2,          /*�û��Զ����˲�ϵ��2*/
	TDE2_FILT_COEF_TYPE_BUTT
}TDE2_FILT_COEF_TYPE_E;

typedef struct hiTDE2_FILT_COEF_S
{
	TDE2_FILT_COEF_TYPE_E enFilterType; /*�˲�ϵ������*/
	TDE2_HFILT_COEF_E enHFilterL;     /*���ȵ�ˮƽ�˲�ϵ��*/
	TDE2_HFILT_COEF_E enHFilterC;	  /*ɫ�ȵ�ˮƽ�˲�ϵ��*/
	TDE2_VFILT_COEF_E enVFilterL;     /*���ȵĴ�ֱ�˲�ϵ��*/
	TDE2_VFILT_COEF_E enVFilterC;     /*ɫ�ȵĴ�ֱ�˲�ϵ��*/
}TDE2_FILT_COEF_S;

typedef struct hiTDE2_FILTER_PARAM_S
{
    TDE2_FILT_COEF_TYPE_E enFiltType;
    HI_U8 *pu8HParamTable;
    HI_U8 *pu8VParamTable;
}TDE2_FILTER_PARAM_S;

/*�˲���չ����*/
typedef struct hiTDE2_FILT_ATTR_EX_S
{
	HI_BOOL bForceHFilt;		/*�Ƿ�ǿ��ˮƽ�˲�*/
	HI_BOOL bForceVFilt;		/*�Ƿ�ǿ�ƴ�ֱ�˲�*/
}TDE2_FILT_ATTR_EX_S;

typedef struct hiTDE2_OPT_S
{
    /*�߼���������*/
    TDE2_ALUCMD_E enAluCmd;

    /*��ɫ�ռ�ROP����*/
    TDE2_ROP_CODE_E enRopCode_Color;

    /*Alpha��ROP����*/
    TDE2_ROP_CODE_E enRopCode_Alpha;

    /*color key��ʽ*/
    TDE2_COLORKEY_MODE_E enColorKeyMode;

    /*color key����ֵ*/
    TDE2_COLORKEY_U unColorKeyValue;

    /*��������clip������������clip*/
    TDE2_CLIPMODE_E enClipMode;

    /*clip������*/
    TDE2_RECT_S stClipRect;

    /*�Ƿ���˸*/
    HI_BOOL bDeflicker;

    /*�Ƿ�����*/
    HI_BOOL bResize;

    /* ���Ż�deflickerʱʹ�õ��˲�ģʽ */
    TDE2_FILTER_MODE_E enFilterMode;

    /*��������*/
    TDE2_MIRROR_E enMirror;

    /*�Ƿ����¼���Clut��*/
    HI_BOOL bClutReload;

    /*ȫ��Alphaֵ*/
    HI_U8   u8GlobalAlpha;

    /*���alpha��Դ*/
    TDE2_OUTALPHA_FROM_E enOutAlphaFrom;
} TDE2_OPT_S;

typedef struct hiTDE2_MBOPT_S
{
    /*Clipģʽѡ��*/
    TDE2_CLIPMODE_E enClipMode;

    /*clip������*/
    TDE2_RECT_S stClipRect;

    /*�Ƿ���˸*/
    HI_BOOL bDeflicker;

    /*������Ϣ*/
    TDE2_MBRESIZE_E enResize;

    /*���������Alpha,��Ĭ��������Alphaֵ*/
    HI_BOOL bSetOutAlpha;
    HI_U8   u8OutAlpha;

	/*�˲�ϵ��ѡ��*/
	TDE2_FILT_COEF_S stFiltCoef;

	/* ǿ���˲�ѡ��*/
	HI_BOOL bForceHFilt;  	/*1:1����ʱ�Ƿ�ǿ��ˮƽ�˲�*/
	HI_BOOL bForceVFilt;	/*1:1����ʱ�Ƿ�ǿ�ƴ�ֱ�˲�*/

} TDE2_MBOPT_S;

typedef enum hiTDE_ROTATE_ANGLE_E
{
    TDE_ROTATE_CLOCKWISE_90 = 0,    /* ˳ʱ����ת90�� */
    TDE_ROTATE_CLOCKWISE_180,   /* ˳ʱ����ת180�� */
    TDE_ROTATE_CLOCKWISE_270,   /* ˳ʱ����ת270�� */
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
    TDE2_MBRESIZE_HIGH = 0,  /* ���Ϊ��դʱʹ�ã���鲻��Ҫ */
    TDE2_MBRESIZE_MIDDLE,    /* ���Ϊ��դʱʹ�ã���鲻��Ҫ */
    TDE2_OSD2MB_OUT42X,      /* ���ΪYCbCr444ʱ����Ҫ�м��ڴ� */
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

/*tde����ʹ�õ��м���ʱbuff���䷽ʽ*/
typedef enum hiTDE_TEMPBUFF_ALLOC_TYPE
{
	TEMPBUFF_ALLOC_AUTO = 0,      /*�ڲ��Զ�����*/
	TEMPBUFF_ALLOC_BYUSER,		  /*�����߷���*/
	TEMPBUFF_ALLOC_BUTT
}TDE_TEMPBUFF_ALLOC_TYPE;

/*beginJob ����*/
typedef struct hiTDE_JOB_ATTR
{
	HI_BOOL bUseTpl;			 /*��job�Ƿ�ʹ��ģ�淽ʽ����*/
	TDE_TEMPBUFF_ALLOC_TYPE enTempBuffType; /*��job�õ�����ʱbuff���䷽ʽ*/
	HI_U32 u32TempBuffSize;      /*��ʱbuff��С��enTempBuffTypeΪTEMPBUFF_ALLOC_BUTT��Ч*/
}TDE_JOB_ATTR;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __TDE_TYPE_H__ */


