/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_comm_vdec.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2006/12/15
  Description   : Common Def Of aio 
  History       :
  1.Date        : 2006/12/15
    Author      : z50825
    Modification: Created file
  2.Date        : 2007/5/10
    Author      : z50825
    Modification: add err code
  3.Date        : 2008/1/8
    Author      : l59217
    Modification: add "hi_comm_video.h" and "buf reserved length 64byte"macro
  4.Date        : 2008/3/4
    Author      : l59217
    Modification: (1)add VDEC_CAPABILITY_S for user learn decoder capabilitys
                  (2)define max nalu size 0x80000 for h264
                     define max ecs size 0x100000 for jpeg, mjpeg
  5.Date		: 2008/3/6
    Author		: l59217
    Modification: change "HI_LOG_LEVEL_ERROR" to "EN_ERR_LEVEL_ERROR"
  6.Date		: 2008/3/7
  	Author		: l59217
  	Modification: added "HI_ERR_VDEC_BUSY" errno
  7.Date        : 2009/10/12
    Author      : c55300
    Modification: VDEC_CHN_STAT_S add two var.
******************************************************************************/


#ifndef  __HI_COMM_VDEC_H__
#define  __HI_COMM_VDEC_H__
#include "hi_type.h"
#include "hi_common.h"
#include "hi_errno.h"
#include "hi_comm_dwm.h"
#include "hi_comm_video.h"
#include "hi_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/*vdec channel attr(user interface)*/

typedef enum hiH264D_MODE_E
{
    H264D_MODE_STREAM = 0,/*send by stream*/
    H264D_MODE_FRAME ,/*send by frame*/
    H264D_MODE_BUTT
}H264D_MODE_E;

typedef struct hiVDEC_CHN_ATTR_S
{
	PAYLOAD_TYPE_E enType;
	HI_U32   	u32BufSize		;		/*stream buf size(Byte)*/
	HI_VOID *pValue; 
}VDEC_CHN_ATTR_S;


typedef struct hiVDEC_ATTR_JPEG_S 
{ 
    HI_U32 		u32Priority		; 		/*priority*/
	HI_U32 		u32PicWidth		;		/*max pic width*/
	HI_U32 		u32PicHeight	;		/*max pic height*/
}VDEC_ATTR_JPEG_S,*PTR_VDEC_ATTR_JPEG_S;


typedef struct hiVDEC_ATTR_H264_S 
{ 
    HI_U32 		u32Priority		; 		/*priority*/
	HI_U32 		u32PicWidth		;		/*max pic width*/
	HI_U32 		u32PicHeight	;		/*max pic height*/
    HI_U32      u32RefFrameNum  ;       /*ref pic num*/   
	H264D_MODE_E     enMode;				/*send by stream or by frame*/
}VDEC_ATTR_H264_S,*PTR_VDEC_ATTR_H264_S;

typedef struct hiVDEC_STREAM_S 
{ 
    HI_U8* 	pu8Addr;					/*stream address*/
    HI_U32  u32Len;				/*stream len*/
    HI_U64  u64PTS;             /*time stamp*/
}VDEC_STREAM_S,*PTR_VDEC_STREAM_S; 

typedef struct hiVDEC_PRIDATA_S{
    HI_U8*    		pu8Addr;   		/*privite data address*/
    HI_U32  		u32Len;    		/*privite data len*/
    HI_BOOL         bValid;			/*is valid?*/
}VDEC_USERDATA_S, *PTR_VDEC_USERDATA_S;


typedef struct hiVDEC_FRAME_S
{
    VIDEO_FRAME_INFO_S    stVideoFrameInfo;                /*frame*/
    HI_BOOL          bValid;
}VDEC_FRAME_INFO_S;

typedef struct hiVDEC_DATA_S 
{
    VDEC_FRAME_INFO_S    stFrameInfo;
    VDEC_WATERMARK_S     stWaterMark;/*water mark*/
    VDEC_USERDATA_S      stUserData;
}VDEC_DATA_S;


typedef struct hiVDEC_CHN_STAT_S
{
	HI_U32 u32LeftStreamBytes;/*left stream bytes waiting for decode*/
	HI_U32 u32LeftStreamFrames;/*left frames waiting for decode,only valid for H264D_MODE_FRAME*/
	HI_U32 u32LeftPics;/*pics waiting for output*/
	HI_BOOL bStartRecvStream;/*had started recv stream?*/
	HI_U32 u32RecvStreamFrames;   /* how many frames of stream has been received. valid when send by frame.*/
	HI_U32 u32DecodeStreamFrames; /* how many frames of stream has been decoded. valid when send by frame.*/
}VDEC_CHN_STAT_S;

/* 
 * static parameter: must set after stop sending stream and all stream is decoded.
 * dynamic parameter: can be set at any time. 
 */
typedef struct hiVDEC_CHN_PARAM_S
{
    HI_U32 u32FrameSizeMax;   /* static parameter. 
                                      * range: [bufSize/2, bufSize*3/4] */
    HI_BOOL bDirectOut;       /* static parameter. Not support yet.
                                      * 1, output one picture directly after picture is decoded. 
                                      * 0, bumping output when DPB is full. Refer to H.264 for details of buming process.
                                      *  */
} VDEC_CHN_PARAM_S;

/* invlalid device ID */
#define HI_ERR_VDEC_INVALID_DEVID     HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define HI_ERR_VDEC_INVALID_CHNID     HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define HI_ERR_VDEC_ILLEGAL_PARAM     HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/* channel exists */
#define HI_ERR_VDEC_EXIST             HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/* using a NULL point */
#define HI_ERR_VDEC_NULL_PTR          HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define HI_ERR_VDEC_NOT_CONFIG        HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define HI_ERR_VDEC_NOT_SURPPORT      HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SURPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define HI_ERR_VDEC_NOT_PERM          HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
/* the channle is not existed  */
#define HI_ERR_VDEC_UNEXIST             HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
/* failure caused by malloc memory */
#define HI_ERR_VDEC_NOMEM             HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
/* failure caused by malloc buffer */
#define HI_ERR_VDEC_NOBUF             HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
/* no data in buffer */
#define HI_ERR_VDEC_BUF_EMPTY         HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
/* no buffer for new data */
#define HI_ERR_VDEC_BUF_FULL          HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
/* system is not ready,had not initialed or loaded*/
#define HI_ERR_VDEC_SYS_NOTREADY      HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
/*system busy*/
#define HI_ERR_VDEC_BUSY			  HI_DEF_ERR(HI_ID_VDEC, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)

typedef struct hiH264_VDEC_CAPABILITY_S
{
    HI_U8 profile           ;   /*0:baseline 1:mainprofile*/
    HI_U8 level             ;   /*4:4, where bit[4:7] represent the integer part, and bit[0:3] represent the fractional part.For example, 0x22 indicates level2.2.*/
    HI_U8 FMO               ;   /*Flexible macroblock ordering.0: Not supported.1: Supported.*/
    HI_U8 ASO               ;   /*Arbitrary Slice Ordering ,0: Not supported.1: Supported.*/
    HI_U8 MBAFF             ;   /*macroblock-adaptive frame/field,0: Not supported.1: Supported.*/
    HI_U8 PAFF              ;   /*picture-adaptive frame/field,0: Not supported.1: Supported.*/
    HI_U8 BSlice            ;   /*B slice,0: Not supported.1: Supported.*/
    HI_U8 subqcif           ;   /*SubQCIF picture,0: Not supported.1: Supported.*/
    HI_U8 qcif              ;   /*QCIF,0: Not supported.1: Supported.*/
    HI_U8 cif               ;   /*CIF,0: Not supported.1: Supported.*/
    HI_U8 fourcif           ;   /*4CIF,0: Not supported.1: Supported.*/
    HI_U8 sixteencif        ;   /*16CIF,0: Not supported.1: Supported.*/
    HI_U8 lostpacket        ;   /*Lost-packet compensation.,0: Not supported.1: Supported.*/
    HI_U16 upperbandwidth   ;   /*Upper limit of the bandwidth.It is measured in kbit/s.*/
    HI_U16 lowerbandwidth   ;   /*Lower limit of the bandwidth.It is measured in kbit/s.*/
    HI_U8 palfps            ;   /*Frame rate in the PAL standard.It is measured in f/s.*/
    HI_U8 ntscfps           ;   /*Frame rate in the NTSC standard.It is measured in f/s.*/
}H264_VDEC_CAPABILITY_S;

typedef struct hiJPEG_VDEC_CAPABILITY_S
{
    HI_U32  enProcess           ;   /* JPEG encoding process that is supported 0:baseline 1:extened profile  2:loseless profile 3:hierarchical profile*/ 
    HI_U32  u32ComponentNum     ;   /* number of components that is supported */
    HI_U32  u32QTNum            ;   /* number of quantization tables that is supported */
    HI_BOOL  bYUV420            ;   /* whether the sampling format 4:2:0 is supported */
    HI_BOOL  bYUV422            ;   /* whether the sampling format 4:2:2 is supported */
    HI_BOOL  bYUV444            ;   /* whether the sampling format 4:4:4 is supported */
    HI_U8   lostpacket          ;   /* lost-packet compensation 1: supported; 0: not supported */
    HI_U16  upperbandwidth      ;   /*upper limit of the bandwidth, measured in kbit/s*/
    HI_U16  lowerbandwidth      ;   /*lower limit of the bandwidth, measured in kbit/s*/
    HI_U8   palfps              ;   /* frame rate in the PAL standard, measured in f/s */
    HI_U8   ntscfps             ;   /* frame rate in the NTSC standard, measured in f/s */
}JPEG_VDEC_CAPABILITY_S;


typedef struct hiVDEC_CAPABILITY_S
{
    PAYLOAD_TYPE_E enType;
    HI_VOID *pCapability;
} VDEC_CAPABILITY_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __HI_COMM_VDEC_H__ */

