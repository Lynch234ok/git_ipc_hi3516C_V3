/******************************************************************************
 Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : hi_defines.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2005/4/23
Last Modified :
Description   : The common data type defination
Function List :
History       :
******************************************************************************/
#ifndef __HI_DEFINES_H__
#define __HI_DEFINES_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HI3511_V100 0x35110100
#define HI3511_V110 0x35110110
#define HI3520_V100 0x35200100
#define HI3515_V100 0x35150100
#define HI3516_V100 0x35160100
#define HI3531_V100 0x35310100
#define HI3532_V100 0x35320100
#define HI3521_V100 0x35210100
#define HI3520A_V100 0x3520A100

#define HI35xx_Vxxx 0x35000000

#ifndef HICHIP
    #define HICHIP HI3521_V100
#endif

#if HICHIP==HI3521_V100
    #define CHIP_NAME    "Hi3521"
    #define MPP_VER_PRIX "_MPP_V"
#elif HICHIP==HI3520A_V100
    #define CHIP_NAME    "Hi3520A"
    #define MPP_VER_PRIX "_MPP_V"
#elif HICHIP==HI35xx_Vxxx
    #error HuHu, I am an dummy chip
#else
    #error HICHIP define may be error
#endif

#define LINE_LEN_BIT            5
#define LINE_LEN                (1<<LINE_LEN_BIT)
#define LINE_BASE_MASK          (~(LINE_LEN-1))
static inline void InvalidateDcache(unsigned long addr, unsigned long len)
{
    unsigned long end;
    //TODO: cache刷新需要重写
    return ;
    
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

    //TODO: cache刷新需要重写
    return ;

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

#define DEFAULT_ALIGN     16
#define MAX_MMZ_NAME_LEN 16

#define MAX_NODE_NUM        16

/* For VDA */
#define VDA_MAX_NODE_NUM        32
#define VDA_MAX_INTERNAL        256
#define VDA_CHN_NUM_MAX         32
#define VDA_MAX_WIDTH           960
#define VDA_MAX_HEIGHT          576


/* For VENC */
#define VENC_MAX_NAME_LEN  16
#define VENC_PIC_ALIGN     16
#define VENC_MAX_CHN_NUM  64
#define VENC_MAX_GRP_NUM  64
#define H264E_MAX_WIDTH   1920
#define H264E_MAX_HEIGHT  2048
#define H264E_MIN_WIDTH   160
#define H264E_MIN_HEIGHT  64
#define JPEGE_MAX_WIDTH   8192
#define JPEGE_MAX_HEIGHT  8192
#define JPEGE_MIN_WIDTH   64
#define JPEGE_MIN_HEIGHT  64
#define VENC_MAX_ROI_NUM  8               /* 最大支持8个ROI区域 */
#define H264E_MIN_HW_INDEX 0
#define H264E_MAX_HW_INDEX 5
#define H264E_MIN_VW_INDEX 0
#define H264E_MAX_VW_INDEX 2
#define MPEG4E_MAX_HW_INDEX 1
#define MPEG4E_MAX_VW_INDEX 0


/* For VDEC */
/* Hi3521 和 Hi3520A 开放同样的解码模块, 
   Hi3520A不支持场解码
   Hi3520A不支持Mpeg4解码
*/
/*hi3521 vdec通道范围[0,63],其中[0,31]通道为vdh解码，[32,63]通道为vedu解码，*/
#define VDEC_MAX_CHN_NUM    64
#define VDH_VEDU_CHN		32				/*VDH 与VEDU临界通道*/


#define VEDU_MAX_CHN_NUM	(VDEC_MAX_CHN_NUM-VDH_VEDU_CHN)
#define MAX_VDEC_CHN        VDEC_MAX_CHN_NUM
#define BUF_RESERVE_LENTH   64          /*reserve 64byte for hardware*/
#define VEDU_H264D_MAX_WIDTH 1920
#define VEDU_H264D_MAX_HEIGHT 2048
#define VEDU_H264D_MIN_WIDTH 64
#define VEDU_H264D_MIN_HEIGHT 64
#define H264D_MAX_WIDTH     4096
#define H264D_MAX_HEIGHT    4096
#define MAX_JPEG_TOTAL      (2048*1536)
#define ONE_SLICE_SIZE_MAX  0x80000
#define ONE_ECS_SIZE_MAX    0x100000
#define H264D_MAX_SLICENUM	135
#define H264D_SLICEMSG_SIZE	32
#define VEDU_H264D_ERRRATE	10
/* For VPP */
#define RGN_HANDLE_MAX         1024
#define OVERLAY_MAX_NUM        8
#define COVEREX_MAX_NUM      16
#define COVER_MAX_NUM        4
#define OVERLAYEX_MAX_NUM      16 


#if 0
#define MAX_COVER_NUM           4
#define MAX_VIOVERLAY_NUM       8
#define MAX_COVEREX_REGION_NUM  16
#define MAX_REGION_NUM          8
#define OVERLAY_START_X_ALIGN   8
#define OVERLAY_START_Y_ALIGN   2
#define MAX_VIOVERLAY_ALPHA     255
#endif


/* number of channle and device on video input unit of chip
 * Note! VIU_MAX_CHN_NUM is NOT equal to VIU_MAX_DEV_NUM
 * multiplied by VIU_MAX_CHN_NUM, because all VI devices
 * can't work at mode of 4 channles at the same time.
 */
#define VIU_MAX_DEV_NUM         4
#define VIU_MAX_WAY_NUM_PER_DEV 4
#define VIU_MAX_CHN_NUM_PER_DEV 4
#define VIU_MAX_CHN_NUM         16
#define VIU_MAX_PHYCHN_NUM      16
#define VIU_CHNID_DEV_FACTOR    2

/* 3521本不支持级联，这里宏定义这些信息，只是为了编译通过 */
#define VIU_MAX_CAS_CHN_NUM    2
#define VIU_SUB_CHN_START    16    /* 定义次通道起始宏*/
#define VIU_CAS_CHN_START    32    /* 定义级联通道起始宏*/


/* max number of VBI region*/
#define VIU_MAX_VBI_NUM         2
/* max length of one VBI region (by word)*/
#define VIU_MAX_VBI_LEN         8

#define VO_MIN_CHN_WIDTH    32      /* channel minimal width */
#define VO_MIN_CHN_HEIGHT   32      /* channel minimal height */

#define VO_MAX_ZOOM_RATIO   1000    /* max zoom ratio, 1000 means 100% scale */

#if 0
/* For VOU */
#define VO_MAX_DEV_NUM      6       /* we have three VO physical device(HD,AD,SD) and three virtual device(VD1,VD2,VD3) */
#define VO_MAX_PHY_DEV      3        /* max physical device number(HD,AD,SD) */
#define VO_MAX_CHN_NUM      64      /* max channel number of each device */
#define VO_SYNC_MAX_GRP     16      /* we limit total sync group as 16 on three device */
#define VO_SYNC_MAX_CHN     64      /* each sync group can accommodate 64 channels */
#define VO_MIN_TOLERATE     1       /* min play toleration 1ms */
#define VO_MAX_TOLERATE     100000  /* max play toleration 100s */
#define VO_MAX_SOLIDDRAW    128        /* max draw region number */
#define VO_MIN_DISP_BUF     5        /* min display buffer number */
#define VO_MAX_DISP_BUF     15        /* max display buffer number */
#define VO_MIN_VIRT_BUF     3        /* min virtual device buffer number */
#define VO_MAX_VIRT_BUF     15        /* max virtual device buffer number */

#define VIVO_CSCD_VBI_ID         0
#define VIVO_CSCD_VBI_X          0
#define VIVO_CSCD_VBI_Y          0
#define VIVO_CSCD_VBI_LEN        2
#define VIVO_CSCD_VBI_LOC        VI_VBI_LOCAL_ODD_FRONT
#define VIVO_CSCD_VBI_DATA_WORD  0
#define VIVO_CSCD_VBI_DATA_BIT   (0x01 << 31)
#endif
#define VO_MAX_DEV_NUM          7      /* max dev num */
#define VO_MAX_LAYER_NUM        7      /* max layer num */
#define VO_MAX_CHN_NUM          32      /* mac chn num */
#define VO_MAX_LAYER_IN_DEV     2       /* max layer num of each dev */

//#define VO_MAX_CAS_DEV_NUM      2       /* max cascade dev num*/
//#define VO_CAS_DEV_1            4       /* cascade display device 1 */
//#define VO_CAS_DEV_2            5       /* cascade display device 2 */
#define VO_CAS_MAX_PAT          128     /* cascade pattern max number */
#define VO_CAS_MAX_POS_32RGN    32      /* cascade position max number */
#define VO_CAS_MAX_POS_64RGN    64      /* cascade position max number */

#define VO_MAX_VIRT_DEV_NUM     4       /* max virtual dev num*/
#define VO_VIRT_DEV_0           3       /* virtual display device 1 */
#define VO_VIRT_DEV_1           4       /* virtual display device 2 */
#define VO_VIRT_DEV_2           5       /* virtual display device 3 */
#define VO_VIRT_DEV_3           6       /* virtual display device 4 */

#define VO_MAX_GFX_LAYER_PER_DEV 3
#define VOU_GRAPHICS_LAYER_NUM   4     

#define VO_MIN_TOLERATE         1       /* min play toleration 1ms */
#define VO_MAX_TOLERATE         100000  /* max play toleration 100s */

#define SIO_MAX_NUM          4
#define AIO_MAX_CHN_NUM      16
#define AENC_MAX_CHN_NUM     32
#define ADEC_MAX_CHN_NUM     32



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEFINES_H__ */

