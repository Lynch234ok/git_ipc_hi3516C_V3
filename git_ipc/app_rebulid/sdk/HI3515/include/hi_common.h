/******************************************************************************
Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : hi_common.h
Version       : Initial Draft
Author        : Hi3511 MPP Team
Created       : 2006/11/09
Last Modified :
Description   : The common defination
Function List :
History       :
 1.Date        : 2009/03/03
   Author      : z44949
   Modification: Created file
2.Date        :   2009/07/01
  Author      :   z44949
  Modification:   Move MPP_VER_PRIX to hi_defines.h
3.Date        :   2009/08/13
  Author      :   y45339
  Modification:   add some proc define
   
******************************************************************************/
#ifndef __HI_COMMON_H__
#define __HI_COMMON_H__

#include "hi_type.h"
#include "hi_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define HI3511_VER_V100 0x35110100
#define HI3511_VER_V110 0x35110110
#define HI3520_VER_V100 0x35200100
#define HI3515_VER_V100 0x35150100

#ifndef DCC_MODE
#define DCC_MODE 0
#endif

#ifndef VER_X
#define VER_X 1
#endif

#ifndef VER_Y
#define VER_Y 0
#endif

#ifndef VER_Z
#define VER_Z 0
#endif

#ifndef VER_P
#define VER_P 0
#endif

#ifdef HI_DEBUG
#define VER_D " Debug"
#else
#define VER_D " Release"
#endif

#define __MK_VERSION(x,y,z,p) #x"."#y"."#z"."#p
#define MK_VERSION(x,y,z,p) __MK_VERSION(x,y,z,p)
#define MPP_VERSION MPP_VER_PRIX MK_VERSION(VER_X,VER_Y,VER_Z,VER_P) VER_D

typedef struct hiMPP_VERSION_S
{
	HI_CHAR aVersion[64];
}MPP_VERSION_S;


#if defined(MMZ_A_NAME) || defined(MMZ_B_NAME)
    #error MMZ_A_NAME or MMZ_B_NAME has conflict
#endif

/* This is the default MMZ Name, memory will be alloced at 'anonymous' */
#define MMZ_A_NAME NULL 

/*
** Only hi3520 may have two memory now, 
** memory accessed by CPU should be alloced at 'ddr16'
*/
#ifndef DDR_NUM
    #define DDR_NUM 1
#endif

#if DDR_NUM==2
    #define MMZ_B_NAME "ddr16"
#else
    #define MMZ_B_NAME NULL
#endif

typedef HI_S32 AI_CHN;
typedef HI_S32 AO_CHN;
typedef HI_S32 AENC_CHN;
typedef HI_S32 ADEC_CHN;
typedef HI_S32 AUDIO_DEV;
typedef HI_S32 AVENC_CHN;

typedef HI_S32 VI_DEV;
typedef HI_S32 VI_CHN;
typedef HI_S32 VO_DEV;
typedef HI_S32 VO_CHN;
typedef HI_S32 VENC_CHN;
typedef HI_S32 VDEC_CHN;
typedef HI_S32 VENC_GRP;
typedef HI_S32 VO_GRP;

#define HI_INVALID_CHN (-1)
#define HI_INVALID_DEV (-1)

#define HI_IO_BLOCK   0
#define HI_IO_NOBLOCK 1

#define  HI_LITTLE_ENDIAN   1234       /* little endian */
#define  HI_BIG_ENDIAN      4321       /* big endian    */

typedef struct hiPOINT_S
{
    HI_S32 s32X;
    HI_S32 s32Y;
}POINT_S;

typedef struct hiDIMENSION_S
{
    HI_S32 s32Width;
    HI_S32 s32Height;
}DIMENSION_S;

typedef struct hiSIZE_S
{
    HI_U32 u32Width;
    HI_U32 u32Height;
} SIZE_S;

typedef struct hiRECT_S
{
    HI_S32 s32X;
    HI_S32 s32Y;
    HI_U32 u32Width;
    HI_U32 u32Height;
}RECT_S;

typedef enum hiMOD_ID_E
{
	HI_ID_CMPI = 0,
	HI_ID_VB,
	HI_ID_SYS,
	HI_ID_VALG,

	HI_ID_CHNL = 4,
	HI_ID_GROUP,
	HI_ID_VENC,
	HI_ID_VPP,

	HI_ID_MD = 8,
	HI_ID_H264E,
	HI_ID_JPEGE,
	HI_ID_MPEG4E,

	HI_ID_VDEC = 12,
	HI_ID_H264D,
	HI_ID_JPEGD,
	HI_ID_VOU,

	HI_ID_VIU = 16,
	HI_ID_DSU,

	HI_ID_SIO = 20,
	HI_ID_AI,
	HI_ID_AO,
	HI_ID_AENC,
	HI_ID_ADEC,

	HI_ID_AVENC = 25,
	
    HI_ID_PCIV  = 26,
    HI_ID_PCIVFMW = 27,
    
    HI_ID_RSV1 = 28,
    HI_ID_RSV2 = 29,
    HI_ID_RSV3 = 30,
    HI_ID_DCCM,
    HI_ID_DCCS,
    HI_ID_PROC,
    HI_ID_LOG,
    HI_ID_MST_LOG,
	HI_ID_VD,
	
    HI_ID_BUTT,
} MOD_ID_E;

#define MPP_MOD_VIU    "vi"
#define MPP_MOD_VOU    "vo"
#define MPP_MOD_DSU    "dsu"

#define MPP_MOD_CHNL   "chnl"
#define MPP_MOD_VENC   "venc"
#define MPP_MOD_GRP    "grp"
#define MPP_MOD_MD     "md"
#define MPP_MOD_VPP    "vpp"

#define MPP_MOD_H264E  "h264e"
#define MPP_MOD_JPEGE  "jpege"
#define MPP_MOD_MPEG4E "mpeg4e"

#define MPP_MOD_VDEC   "vdec"
#define MPP_MOD_H264D  "h264d"
#define MPP_MOD_JPEGD  "jpegd"

#define MPP_MOD_AI     "ai"
#define MPP_MOD_AO     "ao"
#define MPP_MOD_AENC   "aenc"
#define MPP_MOD_ADEC   "adec"
#define MPP_MOD_SIO    "sio"

#define MPP_MOD_VB     "vb"
#define MPP_MOD_SYS    "sys"
#define MPP_MOD_CMPI   "cmpi"

#define MPP_MOD_PCIV      "pciv"
#define MPP_MOD_PCIVFMW   "pcivfmw"

#define MPP_MOD_PROC  "proc"
#define MPP_MOD_LOG    "log"
#define MPP_MOD_MST_LOG "mstlog"

#define MPP_MOD_DCCM "dccm"
#define MPP_MOD_DCCS "dccs"


/* We just coyp this value of payload type from RTP/RTSP definition */
typedef enum
{
	PT_PCMU = 0,
	PT_1016 = 1,
	PT_G721 = 2,
	PT_GSM = 3,
	PT_G723 = 4,
	PT_DVI4_8K = 5,
	PT_DVI4_16K = 6,
	PT_LPC = 7,
	PT_PCMA = 8,
	PT_G722 = 9,
	PT_S16BE_STEREO,
	PT_S16BE_MONO = 11,
	PT_QCELP = 12,
	PT_CN = 13,
	PT_MPEGAUDIO = 14,
	PT_G728 = 15,
	PT_DVI4_3 = 16,
	PT_DVI4_4 = 17,
	PT_G729 = 18,
	PT_G711A = 19,
	PT_G711U = 20,
	PT_G726 = 21,
	PT_G729A = 22,
	PT_LPCM = 23,
	PT_CelB = 25,
	PT_JPEG = 26,
	PT_CUSM = 27,
	PT_NV = 28,
	PT_PICW = 29,
	PT_CPV = 30,
	PT_H261 = 31,
	PT_MPEGVIDEO = 32,
	PT_MPEG2TS = 33,
	PT_H263 = 34,
	PT_SPEG = 35,
	PT_MPEG2VIDEO = 36,
	PT_AAC = 37,
	PT_WMA9STD = 38,
	PT_HEAAC = 39,
	PT_PCM_VOICE = 40,
	PT_PCM_AUDIO = 41,
	PT_AACLC = 42,
	PT_MP3 = 43,
	PT_ADPCMA = 49,
	PT_AEC = 50,
	PT_X_LD = 95,
	PT_H264 = 96,
	PT_D_GSM_HR = 200,
	PT_D_GSM_EFR = 201,
	PT_D_L8 = 202,
	PT_D_RED = 203,
	PT_D_VDVI = 204,
	PT_D_BT656 = 220,
	PT_D_H263_1998 = 221,
	PT_D_MP1S = 222,
	PT_D_MP2P = 223,
	PT_D_BMPEG = 224,
	PT_MP4VIDEO = 230,
	PT_MP4AUDIO = 237,
	PT_VC1 = 238,
	PT_JVC_ASF = 255,
	PT_D_AVI = 256,
	PT_MAX = 257,

	PT_AMR = 1001, /* add by mpp */
	PT_MJPEG = 1002,
}PAYLOAD_TYPE_E;

typedef enum hiVOU_WHO_SENDPIC_E
{
    VOU_WHO_SENDPIC_VIU     = 0,
    VOU_WHO_SENDPIC_VDEC    = 1,
    VOU_WHO_SENDPIC_PCIV    = 2,
    VOU_WHO_SENDPIC_VPP     = 3,
    VOU_WHO_SENDPIC_USR     = 4,
    VOU_WHO_SENDPIC_BUTT
} VOU_WHO_SENDPIC_E;

/*horizontal scale filter coefficient of dsu
which affect image quality of encoding and preview.

normally the filter can be set be DSU_HSCALE_FILTER_DEFAULT
which means sdk will choose filter automatically.Otherwise,
you can choose other filter

Notes:65M means 6.5*/
typedef enum hiDSU_HSCALE_FILTER_E
{
	DSU_HSCALE_FILTER_DEFAULT = 0,
	DSU_HSCALE_FILTER_C_65M,	
	DSU_HSCALE_FILTER_CG_56M,
	DSU_HSCALE_FILTER_LC_45M,
	DSU_HSCALE_FILTER_CG_3M,
	DSU_HSCALE_FILTER_CG_2M,
	DSU_HSCALE_FILTER_CG_1M,
	DSU_HSCALE_FILTER_BUTT
}DSU_HSCALE_FILTER_E;


/*vertical scale filter coefficient of dsu
which affect image quality of encoding and preview.

normally the filter can be set be DSU_VSCALE_FILTER_DEFAULT
which means sdk will choose filter automatically.Otherwise,
you can choose other filter

Notes:38M means 3.8*/
typedef enum hiDSU_VSCALE_FILTER_E
{
	DSU_VSCALE_FILTER_DEFAULT = 0,
	DSU_VSCALE_FILTER_S_6M,    
	DSU_VSCALE_FILTER_S_5M,    
	DSU_VSCALE_FILTER_S_4M,		 
	DSU_VSCALE_FILTER_S_38M,	 
	DSU_VSCALE_FILTER_S_37M,	 
	DSU_VSCALE_FILTER_S_36M,	 
	DSU_VSCALE_FILTER_S_25M,	 
	DSU_VSCALE_FILTER_S_2M,		 
	DSU_VSCALE_FILTER_S_15M,	 
	DSU_VSCALE_FILTER_S_12M,	 
	DSU_VSCALE_FILTER_S_11M,	 
	DSU_VSCALE_FILTER_S_1M,		 
	DSU_VSCALE_FILTER_BUTT
}DSU_VSCALE_FILTER_E;

/*DSU filter param type*/
typedef enum hiDSU_FILTER_PARAM_TYPE
{
	FILTER_PARAM_TYPE_NORM = 0,   /*普通功能的滤波系数类型*/
	FILTER_PARAM_TYPE_EX,		 /*扩展的滤波系数类型:针对vi混合输入的主图像缩放*/
	FILTER_PARAM_TYPE_EX2,        /*扩展滤波系数类型2*/
	FILTER_PARAM_TYPE_USER1,      /*用户自定义滤波系数1*/
	FILTER_PARAM_TYPE_USER2,      /*用户自定义滤波系数2*/
	FILTER_PARAM_TYPE_BUTT
}DSU_FILTER_PARAM_TYPE;

#define DSU_HFILTER_PARAM_NUM   792 /*水平滤波参数个数*/
#define DSU_VFILTER_PARAM_NUM   480 /*垂直滤波参数个数*/

typedef struct hiDSU_FILTER_PARAM_S 
{ 
     DSU_FILTER_PARAM_TYPE enFiltType; 
     HI_U8 au8HParamTable[DSU_HFILTER_PARAM_NUM]; 
     HI_U8 au8VParamTable[DSU_VFILTER_PARAM_NUM]; 
}DSU_FILTER_PARAM_S; 


static inline HI_VOID HI_MemSet32(HI_VOID *pDest,
    HI_U32 u32Value, HI_U32 u32Cnt)
{
    HI_U32 i, *p = (HI_U32 *)pDest;
    for (i = 0; i < u32Cnt; i++)
    {
        *p++ = u32Value;
    }
}


#define RGB(r,g,b)   (((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))
#define RGB_RED(c)   ((c & 0xff0000) >> 16)
#define RGB_GREEN(c) ((c & 0xff00) >> 8)
#define RGB_BLUE(c)  (c & 0xff)

#define YUV(y,u,v)   (((y & 0xff) << 16) | ((u & 0xff) << 8) | (v & 0xff))
#define YUV_Y(c)     ((c & 0xff0000) >> 16)
#define YUV_U(c)     ((c & 0xff00) >> 8)
#define YUV_V(c)     (c & 0xff)

static inline HI_VOID Rgb2Yc(HI_U8 r, HI_U8 g, HI_U8 b, HI_U8 * py, HI_U8 * pcb, HI_U8 * pcr)
{
    /* Y */
    *py = (HI_U8)(((r*66+g*129+b*25) >> 8) + 16);

    /* Cb */
    *pcb = (HI_U8)((((b*112-r*38)-g*74) >> 8) + 128);

    /* Cr */
    *pcr = (HI_U8)((((r*112-g*94)-b*18) >> 8) + 128);
}

static inline HI_U32 Rgb2Yuv(HI_U32 u32Rgb)
{
    HI_U8 r,g,b;
    HI_U8 y,u,v;

    r = RGB_RED(u32Rgb);
    g = RGB_GREEN(u32Rgb);
    b = RGB_BLUE(u32Rgb);

    /* Y */
    y = (HI_U8)((r*66+g*129+b*25)/256 + 16);

    /* Cb */
    u = (HI_U8)(((b*112-r*38)-g*74)/256 + 128);

    /* Cr */
    v = (HI_U8)(((r*112-g*94)-b*18)/256 + 128);

    return YUV(y,u,v);
}

static inline HI_VOID GetYCFromRGB(HI_U32 rgb, HI_U32 * pY, HI_U32 * pC)
{
    HI_U8 r, g, b;
    HI_U8 y, cb, cr;
    HI_U32 color_y, color_c, tmp;
    r = (HI_U8)((rgb>>16)&0xff);
    g = (HI_U8)((rgb>>8)&0xff);
    b = (HI_U8)(rgb&0xff);

    Rgb2Yc(r, g, b, &y, &cb, &cr);
    tmp = y;
    tmp &= 0xff;
    color_y = (tmp<<24) + (tmp<<16) + (tmp<<8) + tmp;

    tmp = cb;
    color_c = (tmp<<24) + (tmp<<8);
    tmp = cr;
    color_c = color_c + (tmp<<16) + tmp;

    *pY = color_y;
    *pC = color_c;
}
#define LINE_LEN_BIT            5
#define LINE_LEN                (1<<LINE_LEN_BIT)
#define LINE_BASE_MASK          (~(LINE_LEN-1))
static inline void InvalidateDcache(unsigned long addr, unsigned long len)
{
    unsigned long end;

    addr &= LINE_BASE_MASK;
    len >>= LINE_LEN_BIT;
    end   = addr + len*LINE_LEN;

    while(addr != end)
    {
        asm("mcr p15, 0, %0, c7, c6, 1"::"r"(addr));
        addr += LINE_LEN;
    }
    return;
}

static inline  void FlushDcache(unsigned long addr, unsigned long len)
{
    unsigned long end;

    addr &= LINE_BASE_MASK;
    len >>= LINE_LEN_BIT;
    end   = addr + len*LINE_LEN;

    while(addr != end)
    {
        asm("mcr p15, 0, %0, c7, c10, 1"::"r"(addr));
        addr += LINE_LEN;
    }
    return;
}


#ifdef __KERNEL__
#include <asm/system.h>
#include <asm/io.h>

static inline void HI_RegSetBit(unsigned long value, unsigned long offset,
    unsigned long addr)
{
	unsigned long t, mask;

	mask = 1 << offset;
	t = readl(addr);
	t &= ~mask;
	t |= (value << offset) & mask;
	writel(t, addr);
}

static inline void HI_RegWrite32(unsigned long value, unsigned long mask,
    unsigned long addr)
{
	unsigned long t;

	t = readl(addr);
	t &= ~mask;
	t |= value & mask;
	writel(t, addr);
}

#endif /* __KERNEL__ */


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* _HI_COMMON_H_ */

