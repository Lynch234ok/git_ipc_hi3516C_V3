/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_comm_venc.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2006/11/24
  Last Modified :
  Description   : common struct definition for VENC
  Function List :
  History       :
  1.Date        : 2006/11/24
    Author      : c42025
    Modification: Created file
  2.Date        : 2008/07/25
    Author      : l64467
    Modification: delete the structe which is not need
                  and rework notation
  3.Date        : 2009/05/11
    Author      : c55300
    Modification: modify bCBR to RC_MODE_E in VENC_ATTR_H264_E.
                  add minutes for ABR; add QpI/QpP for FIXQP.
******************************************************************************/

#ifndef __HI_COMM_VENC_H__
#define __HI_COMM_VENC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "hi_type.h"
#include "hi_common.h"
#include "hi_errno.h"
#include "hi_comm_video.h"
#include "hi_comm_rc.h"

/* invlalid device ID */
#define HI_ERR_VENC_INVALID_DEVID     HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
/* invlalid channel ID */
#define HI_ERR_VENC_INVALID_CHNID     HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define HI_ERR_VENC_ILLEGAL_PARAM     HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/* channel exists */
#define HI_ERR_VENC_EXIST             HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/* channel exists */
#define HI_ERR_VENC_UNEXIST             HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
/* using a NULL point */
#define HI_ERR_VENC_NULL_PTR          HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configing attribute */
#define HI_ERR_VENC_NOT_CONFIG        HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define HI_ERR_VENC_NOT_SUPPORT      HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change stati attribute */
#define HI_ERR_VENC_NOT_PERM          HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define HI_ERR_VENC_NOMEM             HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
/* failure caused by malloc buffer */
#define HI_ERR_VENC_NOBUF             HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
/* no data in buffer */
#define HI_ERR_VENC_BUF_EMPTY         HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
/* no buffer for new data */
#define HI_ERR_VENC_BUF_FULL          HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
/* system is not ready,had not initialed or loaded*/
#define HI_ERR_VENC_SYS_NOTREADY      HI_DEF_ERR(HI_ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)

/*main stream*/
#define TYPE_MAIN_STREAM  (1)

/*minor stream*/
#define TYPE_MINOR_STREAM (0)





/*the nalu type of H264E*/
typedef enum hiH264E_NALU_TYPE_E
{
     H264E_NALU_PSLICE = 1, /*PSLICE types*/
     H264E_NALU_ISLICE = 5, /*ISLICE types*/
     H264E_NALU_SEI    = 6, /*SEI types*/
     H264E_NALU_SPS    = 7, /*SPS types*/
     H264E_NALU_PPS    = 8, /*PPS types*/
     H264E_NALU_BUTT        
} H264E_NALU_TYPE_E;

/*the reference type of H264E slice*/
typedef enum hiH264E_REFSLICE_TYPE_E
{
     H264E_REFSLICE_FOR_1X = 1, /*Reference slice for H264E_REF_MODE_1X*/
     H264E_REFSLICE_FOR_2X = 2, /*Reference slice for H264E_REF_MODE_2X*/
     H264E_REFSLICE_FOR_4X = 5, /*Reference slice for H264E_REF_MODE_4X*/
     H264E_REFSLICE_FOR_BUTT    /* slice not for reference*/          
} H264E_REFSLICE_TYPE_E;

/*the pack type of JPEGE*/
typedef enum hiJPEGE_PACK_TYPE_E
{
     JPEGE_PACK_ECS = 5, /*ECS types*/
     JPEGE_PACK_APP = 6, /*APP types*/
     JPEGE_PACK_VDO = 7, /*VDO types*/
     JPEGE_PACK_PIC = 8, /*PIC types*/
     JPEGE_PACK_BUTT
} JPEGE_PACK_TYPE_E;


// todo: fuyang
/*the pack type of MPEG4*/
typedef enum hiMPEG4E_PACK_TYPE_E
{
     MPEG4E_PACK_VOP_P = 1,  /*P VOP packet types*/
     MPEG4E_PACK_VOP_I = 5,  /*I VOP packet types*/
     MPEG4E_PACK_VOS   = 6,  /*VOS types*/
     MPEG4E_PACK_VO    = 7,  /*VO types*/
     MPEG4E_PACK_VOL   = 8,  /*VOL types*/
     MPEG4E_PACK_GVOP  = 9   /*GROUP of vop types */
} MPEG4E_PACK_TYPE_E;

/*the data type of VENC*/
typedef union hiVENC_DATA_TYPE_U
{
    H264E_NALU_TYPE_E    enH264EType; /*H264E NALU types*/
    JPEGE_PACK_TYPE_E    enJPEGEType; /*JPEGE pack types*/
    MPEG4E_PACK_TYPE_E   enMPEG4EType;/*MPEG4E pack types*/
}VENC_DATA_TYPE_U;


typedef struct hiVENC_PACK_S
{
    HI_U32   u32PhyAddr[2];         /*the physics address of stream*/
    HI_U8   *pu8Addr[2];    		/*the virtual address of stream*/
    HI_U32   u32Len[2];             /*the length of stream*/
    
    HI_U64   u64PTS;                /*PTS*/
    HI_BOOL  bFieldEnd;             /*field end？*/
    HI_BOOL  bFrameEnd;             /*frame end？*/
	
    VENC_DATA_TYPE_U  DataType;     /*the type of stream*/
}VENC_PACK_S;

typedef enum hiH264E_REF_TYPE_E
{
    BASE_IDRSLICE = 0,                  //base层中的IDR帧
    BASE_PSLICE_REFBYBASE,              //base层中的P帧，用于base层中其他帧的参考
    BASE_PSLICE_REFBYENHANCE,           //base层中的P帧，用于enhance层中的帧的参考
    ENHANCE_PSLICE_REFBYENHANCE,        //enhance层中的P帧，用于enhance层中其他帧的参考
    ENHANCE_PSLICE_NOTFORREF,           //enhance层中的P帧，不用于参考
    ENHANCE_PSLICE_BUTT
} H264E_REF_TYPE_E;

typedef struct hiVENC_STREAM_INFO_H264_S
{    
    HI_U32 u32PicBytesNum;             /* the coded picture stream byte number */
    HI_U32 u32PSkipMbNum;              /* the skip macroblock num */
    HI_U32 u32IpcmMbNum;               /* the ipcm macroblock num */
    HI_U32 u32Inter16x8MbNum;          /* the inter16x8 macroblock num */
    HI_U32 u32Inter16x16MbNum;         /* the inter16x16 macroblock num */
    HI_U32 u32Inter8x16MbNum;          /* the inter8x16 macroblock num */
    HI_U32 u32Inter8x8MbNum;           /* the inter8x8 macroblock num */
    HI_U32 u32Intra16MbNum;            /* the intra16x16 macroblock num */
    HI_U32 u32Intra8MbNum;             /* the intra8x8 macroblock num */
    HI_U32 u32Intra4MbNum;             /* the inter4x4 macroblock num */
    
    H264E_REFSLICE_TYPE_E enRefSliceType;  /*the reference type of H264E slice*/
    H264E_REF_TYPE_E      enRefType;
    HI_U32 u32UpdateAttrCnt;
}VENC_STREAM_INFO_H264_S;


typedef struct hiVENC_STREAM_INFO_JPEG_S
{
    HI_U32 u32PicBytesNum;         /* the coded picture stream byte number */
    HI_U32 u32UpdateAttrCnt;
}VENC_STREAM_INFO_JPEG_S;

typedef struct hiVENC_STREAM_INFO_MPEG4_S
{
    HI_U32 u32PicBytesNum;        /* the coded picture stream byte number */
    HI_U32 u32UpdateAttrCnt;
}VENC_STREAM_INFO_MPEG4_S;


typedef struct hiVENC_STREAM_S
{
    VENC_PACK_S         *pstPack;		/*stream pack attribute*/
    HI_U32              u32PackCount;   /*the pack number of one frame stream*/
    HI_U32              u32Seq;         /*the list number of stream*/

    union
    {
        VENC_STREAM_INFO_H264_S stH264Info;   /*the stream info of h264*/
        VENC_STREAM_INFO_JPEG_S stJpegInfo;   /*the stream info of jpeg*/
        VENC_STREAM_INFO_MPEG4_S stMpeg4Info; /*the stream info of mpeg4*/
    };
}VENC_STREAM_S;


typedef struct hiVENC_ATTR_H264_S
{
    HI_U32  u32MaxPicWidth;         /*maximum width of a picture to be encoded, in pixel*/
    HI_U32  u32MaxPicHeight;        /*maximum height of a picture to be encoded, in pixel*/

    HI_U32  u32BufSize;          /*stream buffer size*/
    HI_U32  u32Profile;          /*0: baseline; 1:MP; 2:HP [0,2]*/          
    HI_BOOL bByFrame;            /*get stream mode is slice mode or frame mode*/

    HI_S32  bField;              /* surpport frame code only for hi3516, bfield = HI_FALSE */
    HI_S32  bMainStream;         /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
    HI_U32  u32Priority;         /*channels precedence level. invalidate for hi3516*/
    HI_BOOL bVIField;			 /*the sign of the VI picture is field or frame. Invalidate for hi3531*/

    HI_U32  u32PicWidth;         /*width of a picture to be encoded, in pixel*/
    HI_U32  u32PicHeight;        /*height of a picture to be encoded, in pixel*/    
}VENC_ATTR_H264_S;


typedef struct hiVENC_ATTR_MJPEG_S
{
    HI_U32  u32MaxPicWidth;         /*maximum width of a picture to be encoded, in pixel*/
    HI_U32  u32MaxPicHeight;        /*maximum height of a picture to be encoded, in pixel*/
    HI_U32  u32BufSize;          /*stream buffer size*/
    HI_BOOL bByFrame;            /*get stream mode is field mode  or frame mode*/
    
    HI_BOOL bMainStream;         /*main stream or minor stream types?*/
    HI_BOOL bVIField;			 /*the sign of the VI picture is field or frame*/
    HI_U32  u32Priority;		 /*channels precedence level*/
    
    HI_U32  u32PicWidth;         /*width of a picture to be encoded, in pixel*/
    HI_U32  u32PicHeight;        /*height of a picture to be encoded, in pixel*/ 
    
}VENC_ATTR_MJPEG_S;


typedef struct hiVENC_ATTR_JPEG_S 
{
    HI_U32  u32MaxPicWidth;         /*maximum width of a picture to be encoded, in pixel*/
    HI_U32  u32MaxPicHeight;        /*maximum height of a picture to be encoded, in pixel*/
    HI_U32  u32BufSize;        /*stream buffer size*/
    HI_BOOL bByFrame;          /*get stream mode is field mode  or frame mode*/
    
    HI_BOOL bVIField;			 /*the sign of the VI picture is field or frame*/
    HI_U32  u32Priority;	   /*channels precedence level*/
    
    HI_U32  u32PicWidth;         /*width of a picture to be encoded, in pixel*/
    HI_U32  u32PicHeight;        /*height of a picture to be encoded, in pixel*/
    
}VENC_ATTR_JPEG_S;


/* attribute of MPEG4*/
typedef struct  hiVENC_ATTR_MPEG4_S
{
    HI_U32  u32MaxPicWidth;         /*maximum width of a picture to be encoded, in pixel*/
    HI_U32  u32MaxPicHeight;        /*maximum height of a picture to be encoded, in pixel*/
    

    HI_U32  u32BufSize;           /*buffer size*/
    HI_BOOL bByFrame;             /*get stream mode is pack or frame*/

    HI_S32  bField;              /* surpport frame code only for hi3516, bfield = HI_FALSE */
    HI_S32  bMainStream;         /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
    HI_BOOL bVIField;			  /*the sign of the VI picture is field or frame*/    
    HI_U32  u32Priority;          /*channels precedence level*/

    HI_U32  u32PicWidth;         /*width of a picture to be encoded, in pixel*/
    HI_U32  u32PicHeight;        /*height of a picture to be encoded, in pixel*/
}VENC_ATTR_MPEG4_S;

typedef struct hiVENC_ATTR_S
{
    PAYLOAD_TYPE_E  enType;      /*the type of payload*/
    union
    {
        VENC_ATTR_H264_S  stAttrH264e;     /*attributes of h264*/
        VENC_ATTR_MJPEG_S stAttrMjpeg;     /*attributes of mjpeg*/
        VENC_ATTR_JPEG_S  stAttrJpeg;      /*attributes of jpeg*/
        VENC_ATTR_MPEG4_S stAttrMpeg4;     /*attributes of mpeg4*/
    };    
}VENC_ATTR_S;

typedef struct hiVENC_CHN_ATTR_S
{
    VENC_ATTR_S stVeAttr;    /*the attribute of video encoder*/
    VENC_RC_ATTR_S stRcAttr;    /*the attribute of rate  ctrl*/
}VENC_CHN_ATTR_S;

typedef struct hiVENC_CHN_STAT_S
{
    HI_BOOL bRegistered;         /*registered ?*/
    HI_U32 u32LeftPics;          /*left picture number */
    HI_U32 u32LeftStreamBytes;   /*left stream bytes*/
    HI_U32 u32LeftStreamFrames;  /*left stream frames*/
    HI_U32 u32CurPacks;          /*pack number of current frame*/
}VENC_CHN_STAT_S;

typedef struct hiVENC_H264_CAPABILITY_S
{
    HI_U8   u8Profile       ;   /*0:baseline 1:mainprofile  2:high profile*/
    HI_U8   u8Level         ;   /*eg: 22 denote level2.2*/
    HI_U8   u8BaseAttr      ;   /*bit0～bit5 denote MBAFF, PAFF，B SLICE，FMO， ASO，PARTITION  */
    HI_U8   u8ViFormat      ;   /*format， bit.0: PAL(25)，bit.1:NTSC(30)  */
    HI_U16  u16MaxWInMb     ;   /*the max width of input picture*/
    HI_U16  u16MaxHInMb     ;   /*the max height of input picture*/
    HI_U16  u16MaxCifNum    ;   /*the max coding capability */
    HI_U16  u16MaxBitrate   ;   /*the max bit rate output*/
    HI_U16  upperbandwidth  ;   /*upper band*/
    HI_U16  lowerbandwidth  ;   /*lower band*/
    HI_U8   palfps          ;   /*PAL: frame per second*/
    HI_U8   ntscfps         ;   /*NTSC: frame per second*/
}VENC_H264_CAPABILITY_S;

typedef struct hiVENC_JPEG_CAPABILITY_S
{
    HI_U8 u8Profile         ;   /*0:baseline 1:extened profile  2:loseless profile 3:hierarchical profile*/
    HI_U8 u8ViFormat        ;   /*format， bit.0: PAL(25)，bit.1:NTSC(30)  */
    HI_U16 u16MaxWInMb      ;   /*the max width of input picture*/
    HI_U16 u16MaxHInMb      ;   /*the max height of input picture*/
    HI_U16 u16MaxCifNum     ;   /*the max coding capability */
    HI_U16 u16MaxBitrate    ;   /*the max bit rate output*/
    HI_U16 upperbandwidth   ;   /*upper band*/
    HI_U16 lowerbandwidth   ;   /*lower band*/
    HI_U8 palfps            ;   /*PAL: frame per second*/
    HI_U8 ntscfps           ;   /*NTSC: frame per second*/
}VENC_JPEG_CAPABILITY_S;

typedef struct hiVENC_MPEG4_CAPABILITY_S
{
    HI_BOOL IVOP_En;          /* I-VOP Enable */
    HI_BOOL PVOP_En;          /* P-VOP Enable */
    HI_BOOL BVOP_En;          /* B-VOP Enable*/
    HI_BOOL DCPred_En;        /* DC Prediction Enable*/
    HI_BOOL ACPred_En;        /*AC Prediction Enable*/
    HI_BOOL M4V_En;           /*M4V Enable*/
    HI_BOOL Resyn_En;         /*GOB Resynchronization Enable*/
    HI_BOOL DataPart_En;      /*Data Partitioning Enable*/
    HI_BOOL RVLC_En;          /*Reversible VLC Enable*/
    HI_BOOL ShortHead_En;     /*Short Header Enable*/
    HI_BOOL Interlace_En;     /*Interlace Enable*/
    HI_BOOL GMC_En;           /*Global Motion Compensation Enable*/
    HI_BOOL QMC_En;           /*Quarter-pel Motion Compensation Enable*/
    HI_BOOL HalfPel_En;       /* I-VOP Enable*/
    HI_BOOL DBlock_En;        /*Deblocking Enable*/
    HI_BOOL DRing_En;         /*Deringing Enable*/

//    M4QType QuantType;        /*Method 1/Method 2 Quantisation*/
    HI_U32  SearchWin;        /*Search Window*/
}VENC_MPEG4_CAPABILITY_S;







typedef struct hiVENC_CAPABILITY_S
{
    PAYLOAD_TYPE_E enType; /*the type of payload*/
    union
    {
        VENC_H264_CAPABILITY_S  stH264Cap;
        VENC_JPEG_CAPABILITY_S  stJpegCap;
        VENC_MPEG4_CAPABILITY_S stMpegCap;
    };
}VENC_CAPABILITY_S;    

typedef struct hiVENC_PARAM_H264_SLICE_SPLIT_S
{
    HI_BOOL bSplitEnable;      /*slice split enable, HI_TRUE:enable, HI_FALSE:diable, default value:HI_FALSE*/
    HI_U32  u32SplitMode;      /*0:bit number, 1:mb line number, >=2:no meaning*/
    HI_U32  u32SliceSize;      /*when the splitmode is 0, this value presents the bitsnum of one slice average, 
                                 when the splitmode is 1, this value presents the mb num of one slice*/
} VENC_PARAM_H264_SLICE_SPLIT_S;

typedef struct hiVENC_PARAM_H264_INTER_PRED_S
{
    /* search window */
    HI_U32 u32HWSize;    /* size of horizontal search window.
                                * default value: differ with the picture size
                                * 0000: [-16, +15]
                                * 0001: [-32, +31]
                                * 0010: [-48, +47]
                                * 0011: [-64, +63]
                                * 0100: [-80, +79]
                                * 0101: [-96, +95]
                                * 0110: [-112, +111]
                                * 0111: [-128, +127]
                                * 1000: [-144, +143]
                                * 1001: [-160, +159]
                                * >1001: no meaning. 
                                */
    HI_U32 u32VWSize;    /* size of vertical search window.
                                * default value: differ with the picture size
                                * 000: [-16, +15]
                                * 001: [-32, +31]
                                * 010: [-48, +47]
                                * 011: [-64, +63]
                                * 100: [-80, +79]
                                * >100: no meaning. 
                                */
    /* inter pred, one of the following 4 kinds of inter prediction mode must be enable */
    HI_BOOL bInter16x16PredEn;   /*default: HI_TRUE, enable 16x16 prediction*/
    HI_BOOL bInter16x8PredEn;  /*default: HI_TRUE*/
    HI_BOOL bInter8x16PredEn;  /*default: HI_TRUE*/
    HI_BOOL bInter8x8PredEn;   /*default: HI_TRUE*/
    HI_BOOL bExtedgeEn;        /*default: HI_TRUE*/
} VENC_PARAM_H264_INTER_PRED_S;

typedef struct hiVENC_PARAM_H264_INTRA_PRED_S
{
    /* intra pred, one of following 2 kinds of intra pred mode must be enable */                            
    HI_BOOL bIntra16x16PredEn;          /*default: HI_TRUE, enable 16x16 intra prediction*/           
    HI_BOOL bIntraNxNPredEn;            /*default: HI_TRUE, enable 4x4 and/or 8x8 prediction
                                                Notes: this value must work with IntraTransMode*/
    HI_U32 constrained_intra_pred_flag;
    HI_BOOL bIpcmEn;
}VENC_PARAM_H264_INTRA_PRED_S;

typedef struct hiVENC_PARAM_H264E_TRANS_S
{
    HI_U32 u32IntraTransMode;           /* 0: trans4x4, trans8x8; 1: trans4x4, 2: trans8x8 */
    HI_U32 u32InterTransMode;           /* 0: trans4x4, trans8x8; 1: trans4x4, 2: trans8x8 */
    
    HI_BOOL bScalingListValid;          /* default: HI_FALSE */
                                        /* Notes: Enable IntraScalingList8X8 and InterScalingList8X8 while bScalingListValid is HI_TRUE */
    HI_U8  InterScalingList8X8[64];
    HI_U8  IntraScalingList8X8[64];
    
    HI_S32 chroma_qp_index_offset;            /* [-12,12],default value: 0*/
}VENC_PARAM_H264_TRANS_S;

typedef struct hiVENC_PARAM_H264E_ENTROPY_S
{
    HI_U32 u32EntropyEncModeI;                   /* 0:cavlc, 1:cabac */
    HI_U32 u32EntropyEncModeP;                   /* 0:cavlc, 1:cabac */
    HI_U32 cabac_stuff_en;                       /* default: 0*/
    HI_U32 Cabac_init_idc;                       /* 0~2 */
}VENC_PARAM_H264_ENTROPY_S;

typedef struct hiVENC_PARAM_H264_POC_S
{
    HI_U32 pic_order_cnt_type;          /* default value: 2. {0,1,2} */

}VENC_PARAM_H264_POC_S;

typedef struct hiVENC_PARAM_H264E_DBLK_S
{
    HI_U32 disable_deblocking_filter_idc;   /* default value: 0. {0,1,2} */
    HI_S32 slice_alpha_c0_offset_div2;      /* default value: 5. [-6,+6] */
    HI_S32 slice_beta_offset_div2;          /* default value: 5. [-6,+6] */
}VENC_PARAM_H264_DBLK_S;

typedef struct hiVENC_PARAM_H264E_VUI_S
{
    HI_S32 timing_info_present_flag;    /* default value: 0. If 1, timing info belows will be encoded into vui. {0,1} */
    HI_S32 num_units_in_tick;           /* default value: n/a. > 0. */
    HI_S32 time_scale;                  /* default value: n/a. > 0. */
    HI_S32 fixed_frame_rate_flag;       /* default value: n/a. {0,1} */
}VENC_PARAM_H264_VUI_S;

typedef struct hiVENC_PARAM_JPEG_S
{
    HI_U32 u32Qfactor;          /*image quality :[1,99]*/

    HI_U8  u8YQt[64];          /* y qt value */
    HI_U8  u8CbQt[64];         /* cb qt value */
    HI_U8  u8CrQt[64];         /* cr qt value */

    HI_U32 u32MCUPerECS;        /*default value: 0, MCU number of one ECS*/
} VENC_PARAM_JPEG_S;

typedef struct hiVENC_PARAM_MJPEG_S
{
    HI_U8 u8YQt[64];          /* y qt value */
    HI_U8 u8CbQt[64];         /* cb qt value */
    HI_U8 u8CrQt[64];         /* cr qt value */

    HI_U32  u32MCUPerECS;      /*default value: 0, MCU number of one ECS*/
} VENC_PARAM_MJPEG_S;

typedef struct hiVENC_PARAM_MPEG4_S
{
    HI_BOOL bSplitEnable;      /*slice split enable, HI_TRUE:enable, HI_FALSE:diable, default value:HI_FALSE*/
    HI_U32  u32SplitMode;      /*0:bit number, 1:mb line number, >=2:no meaning*/
    HI_U32  u32PacketSize;      /*when the splitmode is 0, this value presents the bitsnum of one packet average, 
                                 when the splitmode is 1, this value presents the mb num of one packet*/

    /* search window */
    HI_U32 u32HWSize;    /* size of horizontal search window.
                                * default value: differ with the picture size
                                * 0000: [-16, +15]
                                * 0001: [-32, +31]
                                */
    HI_U32 u32VWSize;    /* size of vertical search window.
                                * default value: differ with the picture size
                                * 000: [-16, +15]
                                * >000: no meaning. 
                                */

} VENC_PARAM_MPEG4_S;


typedef struct hiCROP_INFO_S
{
    HI_BOOL         bCrop;
    HI_U16          u16OffsetTop;	   /* top offset of show area */
    HI_U16          u16OffsetBottom;   /* bottom offset of show area */
    HI_U16          u16OffsetLeft;	   /* left offset of show area */
    HI_U16          u16OffsetRight;    /* right offset of show area */
    HI_U32          u32AlignHeight;
    HI_U32          u32AlignWidth;
    HI_U32          u32PicHeight;
    HI_U32          u32PicWidth;
}CROP_INFO_S;

/* ROI struct */
typedef struct hiVENC_ROI_CFG_S
{
    HI_U32      u32Index;                    /* Index of an ROI. The system supports indexes ranging from 0 to 7 */
    HI_BOOL   bEnable;                     /* Whether to enable this ROI */
    HI_BOOL   bAbsQp;                      /* QP mode of an ROI.HI_FALSE: relative QP.HI_TURE: absolute QP.*/
    HI_S32     s32Qp;                       /* QP value. */
    RECT_S    stRect;                      /* Region of an ROI*/
}VENC_ROI_CFG_S;

typedef struct hiGROUP_COLOR2GREY_S
{
    HI_BOOL  bColor2Grey;                      /* Whether to enable Color2Grey.*/
}GROUP_COLOR2GREY_S;

typedef struct hiGROUP_COLOR2GREY_CONF_S
{
    HI_BOOL		bEnable;
    HI_U32  	u32MaxWidth;                   
    HI_U32 		u32MaxHeight; 
}GROUP_COLOR2GREY_CONF_S;


typedef struct hiGROUP_CLIP_CFG_S
{
    HI_BOOL bEnable;    /* Clip region enable */
    RECT_S  stRect;     /* Clip region, note: s32X must be multi of 16 */
}GROUP_CLIP_CFG_S;


typedef struct hiGROUP_FRAME_RATE_S
{
    HI_S32  s32ViFrmRate;          /* Input frame rate of a  group*/
    HI_S32  s32VpssFrmRate;        /* Output frame rate of a channel group */
} GROUP_FRAME_RATE_S;

typedef struct hiVENC_ATTR_H264_REF_PARAM_S
{
    HI_U32       u32Base;
    HI_U32       u32Enhance;
    HI_BOOL      bEnablePred;
} VENC_ATTR_H264_REF_PARAM_S;

typedef enum hiVENC_ATTR_H264_REF_MODE_E
{
    H264E_REF_MODE_1X = 1,
    H264E_REF_MODE_2X = 2,
    H264E_REF_MODE_4X = 5,
    H264E_REF_MODE_BUTT,
}VENC_ATTR_H264_REF_MODE_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __HI_COMM_VENC_H__ */

