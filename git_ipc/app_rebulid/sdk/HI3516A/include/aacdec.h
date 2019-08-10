/*****************************************************************************
*             Copyright 2004 - 2050, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: aacdec.h
* Description:
*
* History:
* Version   Date         Author     DefectNum    Description
* 0.01      2006-11-01   z40717    NULL         Create this file.
*     
*****************************************************************************/

#ifndef _AACDEC_H
#define _AACDEC_H

#ifdef __cplusplus
    #if __cplusplus
    extern "C" {
    #endif  /* __cpluscplus */
#endif  /* __cpluscplus */

#include "hi_type.h"

/* according to spec (13818-7 section 8.2.2, 14496-3 section 4.5.3)
 * max size of input buffer = 
 *    6144 bits =  768 bytes per SCE or CCE-I
 *   12288 bits = 1536 bytes per CPE
 *       0 bits =    0 bytes per CCE-D (uses bits from the SCE/CPE/CCE-I it is coupled to)
 */
#ifndef AAC_MAX_NCHANS				/* if max channels isn't set in makefile, */
#define AAC_MAX_NCHANS		2		/* set to default max number of channels  */
#endif
#define AAC_MAX_NSAMPS		1024
#define AAC_MAINBUF_SIZE	(768 * AAC_MAX_NCHANS)

#define AAC_NUM_PROFILES	3
#define AAC_PROFILE_MP		0
#define AAC_PROFILE_LC		1
#define AAC_PROFILE_SSR		2

/* HISI_VOICE right code */
#ifndef HI_SUCCESS
#define HI_SUCCESS          0
#endif

enum {
	ERR_AAC_NONE                          =   0,        /* 解码器正确，无错误 */
	ERR_AAC_INDATA_UNDERFLOW              =  -1,        /* 输入码流数据不够，解码器无法解码1帧数据 */
	ERR_AAC_NULL_POINTER                  =  -2,        /* 检测到非法的输入指针 */
	ERR_AAC_INVALID_ADTS_HEADER           =  -3,        /* 检测输入码流的ADTS头信息非法 */ 
	ERR_AAC_INVALID_ADIF_HEADER           =  -4,        /* 检测到输入码流的ADIF头信息非法 */ 
	ERR_AAC_INVALID_FRAME                 =  -5,        /* 检测到SetRawBlockParams输入AACDecInfo信息非法 */        
	ERR_AAC_MPEG4_UNSUPPORTED             =  -6,        /* 检测到不支持的MPEG4 AAC码流格式 */
	ERR_AAC_CHANNEL_MAP                   =  -7,        /* 检测到非法解码声道信息 */
	ERR_AAC_SYNTAX_ELEMENT                =  -8,        /* 检测到输入码流包破损 */
	ERR_AAC_DEQUANT                       =  -9,        /* 解码器反量化处理模块出错 */
	ERR_AAC_STEREO_PROCESS                = -10,        /* 解码器立体声处理模块模块出错 */
	ERR_AAC_PNS                           = -11,        /* 解码器PNS处理模块模块出错 */
	ERR_AAC_SHORT_BLOCK_DEINT             = -12,        /* 保留扩展 */
	ERR_AAC_TNS                           = -13,        /* 解码器TNS处理模块模块出错 */
	ERR_AAC_IMDCT                         = -14,        /* 解码器IMDCT处理模块模块出错     */
	ERR_AAC_NCHANS_TOO_HIGH               = -15,        /* 不支撑多声道解码，最多支持两声道解码 */
	ERR_AAC_SBR_INIT                      = -16,        /* 解码器SBR处理模块初始化出错 */
	ERR_AAC_SBR_BITSTREAM                 = -17,        /* 检测到SBR码流信息非法 */
	ERR_AAC_SBR_DATA                      = -18,        /* 解码后的SBR数据非法 */
	ERR_AAC_SBR_PCM_FORMAT                = -19,        /* 解码后的SBR数据的PCM_FORMAT信息非法 */
	ERR_AAC_SBR_NCHANS_TOO_HIGH           = -20,        /* 不支撑SBR多声道解码，最多支持两声道解码 */
	ERR_AAC_SBR_SINGLERATE_UNSUPPORTED    = -21,        /* SBR采样频率非法 */
	ERR_AAC_RAWBLOCK_PARAMS               = -22,        /* 检测到SetRawBlockParams输入参数非法 */
	ERR_AAC_PS_INIT                       = -23,        /* 解码器PS处理模块初始化出错 */       
	ERR_AAC_CH_MAPPING                    = -24,
	ERR_UNKNOWN               = -9999                   /* 保留扩展 */
};

typedef struct _AACFrameInfo {
	int bitRate;
	int nChans;                      /* 1 或 2 */
	int sampRateCore;                /* 内核解码器采样率，码流信息，不作为输出控制使用 */
	int sampRateOut;                 /* 解码输出采样率 */
	int bitsPerSample;               /* 16，目前仅支持16比特位宽输出 */
	int outputSamps;                 /* nChans*SamplePerFrame */ 
	int profile;                     /* profile,   码流信息，不作为输出控制使用 */
	int tnsUsed;                     /* TNS Tools，码流信息，不作为输出控制使用 */
	int pnsUsed;                     /* PNS Tools，码流信息，不作为输出控制使用 */
} AACFrameInfo;

typedef void *HAACDecoder;

typedef struct hiAACDEC_VERSION_S
{
    HI_U8 aVersion[64];
}AACDEC_VERSION_S;

/*
*****************************************************************************
*                         ENCLARATION OF PROTOTYPES
*****************************************************************************
*/

/******************************************************************************
* Function:      HI_AMRNB_GetVersion
* Description:   Get version information 
* Calls: 
* Called By:     main
* Input:         pVersion       version describe struct 
* Output:        None
* Return:        int            HI_SUCCESS/HI_FAILURE
* Others:       
******************************************************************************/
HI_S32 HI_AACDEC_GetVersion(AACDEC_VERSION_S *pVersion);


/*
*****************************************************************************
*                         ENCLARATION OF PROTOTYPES
*****************************************************************************
*/

/*****************************************************************************
  Function:    AACInitDecoder
  Description: create and initial decoder device
  Calls: 
  Called By:
  Input:       
  Output:      
  Return:      HAACDecoder --AAC-Decoder handle
  Others:
*****************************************************************************/
HAACDecoder AACInitDecoder(HI_VOID);

/*****************************************************************************
  Function:    AACFreeDecoder
  Description: destroy AAC-Decoder, free the memory.
  Calls: 
  Called By:
  Input:       hAACDecoder --AAC-Decoder handle
  Output:      
  Return:      
  Others:
*****************************************************************************/
HI_VOID AACFreeDecoder(HAACDecoder hAACDecoder);

/*****************************************************************************
  Function:    AACSetRawMode
  Description: set RawMode before decode Raw Format aac bitstream
  Calls: 
  Called By:
  Input:       hAACDecoder   -- AAC-Decoder handle
  Input:       nChans        -- 1/2
  Input:       sampRate    
  Output:      
  Return:      SUCCESS/ERROR_CODE
  Others:
*****************************************************************************/
HI_S32  AACSetRawMode(HAACDecoder hAACDecoder, HI_S32 nChans, HI_S32 sampRate);

/*****************************************************************************
  Function:    AACDecodeFrame
  Description: look for valid AAC sync header.
  Calls: 
  Called By:
  Input:       hAACDecoder   --AAC-Decoder handle
               ppInbufPtr    --address of the pointer of start-point of the bitstream.
                           NOTE: bitstream should be in little endian format.
			   pBytesLeft --pointer to BytesLeft that indicates bitstream numbers at input buffer;
			               after FindSync, pInbufPtr pointer to current bitsrream header, BytesLeft
						   indicates the left bytes.

  Output:      updated bitstream buffer state(ppInbufPtr, pBytesLeft)
  
  Return:      <0    : ERR_AAC_INDATA_UNDERFLOW 
			   other : Success, return number bytes of current frame.
*****************************************************************************/
HI_S32 AACDecodeFindSyncHeader(HAACDecoder hAACDecoder, HI_U8 **ppInbufPtr, HI_S32 *pBytesLeft);

/*****************************************************************************
  Function:    AACDecodeFrame
  Description: decoding AAC frame and output 1024(LC) OR 2048(HEAAC/eAAC/eAAC+) 16bit PCM samples per channel.
  Calls: 
  Called By:
  Input:       hAACDecoder   --AAC-Decoder handle
               ppInbufPtr    --address of the pointer of start-point of the bitstream.
                           NOTE: bitstream should be in little endian format.
			   pBytesLeft --pointer to BytesLeft that indicates bitstream numbers at input buffer;
			               after decoding, pInbufPtr pointer to current bitsrream header, BytesLeft
						   indicates the left bytes.

               pOutPcm    --the address of the out pcm buffer,
						   pcm data in noninterlaced fotmat: L/L/L/... R/R/R/...
                           NOTE: 1 caller must be sure buffer size is GREAT ENOUGH.
  Output:      output pcm data and updated bitstream buffer state(ppInbufPtr, pBytesLeft)

  Return:      SUCCESS/ERROR_CODE
  Others:
*****************************************************************************/
HI_S32  AACDecodeFrame(HAACDecoder hAACDecoder, HI_U8 **ppInbufPtr, HI_S32 *pBytesLeft, HI_S16 *pOutPcm);

/*****************************************************************************
  Function:    AACGetLastFrameInfo
  Description: get the frame information
  Calls: 
  Called By:
  Input:       hAACDecoder    --AAC-Decoder handle
  Output:      aacFrameInfo   --frame information
  Return:      
  Others:
*****************************************************************************/
HI_VOID AACGetLastFrameInfo(HAACDecoder hAACDecoder, AACFrameInfo *aacFrameInfo);

/*****************************************************************************
  Function:    AACDecoderSetEosFlag
  Description: set eosflag
  Calls: 
  Called By:
  Input:       hAACDecoder    --AAC-Decoder handle
  Output:      s32Eosflag     --end flag
  Return:      
  Others:
*****************************************************************************/
HI_S32 AACDecoderSetEosFlag(HAACDecoder hAACDecoder, HI_S32 s32Eosflag);

/**************************************************************************************
  Function:    AACFlushCodec
 
  Description: flush internal codec state (after seeking, for example)
 
  Input:       hAACDecoder   --AAC-Decoder handle
 
  Output:      updated state variables in hAACDecoder
 
  Return:      SUCCESS/ERROR_CODE
**************************************************************************************/
HI_S32  AACFlushCodec(HAACDecoder hAACDecoder);



#ifdef __cplusplus
    #if __cplusplus
	}
    #endif  /* __cpluscplus */
#endif  /* __cpluscplus */

#endif	/* _AACDEC_H */
