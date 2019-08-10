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
1.Date        :   2009/03/03
  Author      :   z44949
  Modification:   Create the file

2.Date        :   2009/07/01
  Author      :   z44949
  Modification:   Move MPP_VER_PRIX from hi_comm.h

******************************************************************************/
#ifndef __HI_DEFINES_H__
#define __HI_DEFINES_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifdef hi3515
#define MPP_VER_PRIX "Hi3515_MPP_V"
#else
#define MPP_VER_PRIX "Hi3520_MPP_V"
#endif

/* For MD */
#define MAX_NODE_NUM        16
#define MD_MAX_INTERNAL     256

/* For VENC */
#define VENC_MAX_CHN_NUM  64
#define VENC_MAX_GRP_NUM  64
#define H264E_MAX_WIDTH   2048
#define H264E_MAX_HEIGHT  1536
#define JPEGE_MAX_WIDTH   2048
#define JPEGE_MAX_HEIGHT  1536
#define JPEGE_MIN_WIDTH   80
#define JPEGE_MIN_HEIGHT  64

/* For VDEC */
#define VDEC_MAX_CHN_NUM    32
#define MAX_VDEC_CHN        VDEC_MAX_CHN_NUM
#define BUF_RESERVE_LENTH   64          /*reserve 64byte for hardware*/
#define H264D_MAX_WIDTH		4096
#define H264D_MAX_HEIGHT    4096
#define MAX_JPEG_TOTAL		(2048*1536)
#define ONE_SLICE_SIZE_MAX  0x80000
#define ONE_ECS_SIZE_MAX	0x100000

/* For VPP */
#define MAX_COVER_NUM           4
#define MAX_OVERLAY_NUM         4
#define MAX_VIOVERLAY_NUM       8
#define MAX_COVEREX_REGION_NUM  16
#define MAX_REGION_NUM          16
#define OVERLAY_START_X_ALIGN   8
#define OVERLAY_START_Y_ALIGN   2
#define MAX_VIOVERLAY_ALPHA     255


/* total number of video input channels including
 * VIU hardware channle and virtual channle.
 */
#define VI_MAX_CHN_NUM          32
#define VI_MAX_DEV_NUM          8

#ifdef hi3515
/* number of channle and device on video input unit of chip
 * Note! VIU_MAX_CHN_NUM is NOT equal to VIU_MAX_DEV_NUM
 * multiplied by VIU_MAX_CHN_NUM_PER_DEV, because all VI devices
 * can't work at mode of 4 channles at the same time.
 */
#define VIU_MAX_DEV_NUM         4
#define VIU_MAX_CHN_NUM_PER_DEV 4
#define VIU_MAX_CHN_NUM         8
#define VIU_CHNID_DEV_FACTOR    2
#else
/* number of channle and device on video input unit of chip
 * Note! VIU_MAX_CHN_NUM is NOT equal to VIU_MAX_DEV_NUM
 */
#define VIU_MAX_DEV_NUM         4
#define VIU_MAX_CHN_NUM_PER_DEV 4
#define VIU_MAX_CHN_NUM         16
#define VIU_CHNID_DEV_FACTOR    4
#endif

/* max number of VBI region*/
#define VIU_MAX_VBI_NUM         2
/* max length of one VBI region (by word)*/
#define VIU_MAX_VBI_LEN         8


/* For VOU */
#define VO_MAX_DEV_NUM      6       /* we have three VO physical device(HD,AD,SD) and three virtual device(VD1,VD2,VD3) */
#define VO_MAX_PHY_DEV 		3		/* max physical device number(HD,AD,SD) */
#define VO_MAX_CHN_NUM      64      /* max channel number of each device */
#define VO_CSCD_MAX_PAT     128     /* cascade pattern max number */
#define VO_CSCD_MAX_POS     32      /* cascade position max number */
#define VO_SYNC_MAX_GRP     16      /* we limit total sync group as 16 on three device */
#define VO_SYNC_MAX_CHN     64      /* each sync group can accommodate 64 channels */
#define VO_MIN_CHN_WIDTH    16      /* channel minimal width */
#define VO_MIN_CHN_HEIGHT   16      /* channel minimal height */
#define VO_MIN_TOLERATE     1       /* min play toleration 1ms */
#define VO_MAX_TOLERATE     100000  /* max play toleration 100s */
#define VO_MAX_SOLIDDRAW	128		/* max draw region number */
#define VO_MAX_ZOOM_RATIO	1000	/* max zoom ratio, 1000 means 100% scale */
#define VO_MIN_DISP_BUF		5		/* min display buffer number */
#define VO_MAX_DISP_BUF		15		/* max display buffer number */
#define VO_MIN_VIRT_BUF		3		/* min virtual device buffer number */
#define VO_MAX_VIRT_BUF		15		/* max virtual device buffer number */

#define VIVO_CSCD_VBI_ID         0
#define VIVO_CSCD_VBI_X          0
#define VIVO_CSCD_VBI_Y          0
#define VIVO_CSCD_VBI_LEN        2
#define VIVO_CSCD_VBI_LOC        VI_VBI_LOCAL_ODD_FRONT
#define VIVO_CSCD_VBI_DATA_WORD  0
#define VIVO_CSCD_VBI_DATA_BIT   (0x01 << 31)


#ifdef hi3515
#define SIO_MAX_NUM          2
#else
#define SIO_MAX_NUM          3
#endif


#define AIO_MAX_CHN_NUM      16

#define AENC_MAX_CHN_NUM     32
#define ADEC_MAX_CHN_NUM     32

#define AVENC_CHN_MAXNUM     32


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEFINES_H__ */

