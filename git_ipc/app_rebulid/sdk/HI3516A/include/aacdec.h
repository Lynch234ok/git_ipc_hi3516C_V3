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
	ERR_AAC_NONE                          =   0,        /* ��������ȷ���޴��� */
	ERR_AAC_INDATA_UNDERFLOW              =  -1,        /* �����������ݲ������������޷�����1֡���� */
	ERR_AAC_NULL_POINTER                  =  -2,        /* ��⵽�Ƿ�������ָ�� */
	ERR_AAC_INVALID_ADTS_HEADER           =  -3,        /* �������������ADTSͷ��Ϣ�Ƿ� */ 
	ERR_AAC_INVALID_ADIF_HEADER           =  -4,        /* ��⵽����������ADIFͷ��Ϣ�Ƿ� */ 
	ERR_AAC_INVALID_FRAME                 =  -5,        /* ��⵽SetRawBlockParams����AACDecInfo��Ϣ�Ƿ� */        
	ERR_AAC_MPEG4_UNSUPPORTED             =  -6,        /* ��⵽��֧�ֵ�MPEG4 AAC������ʽ */
	ERR_AAC_CHANNEL_MAP                   =  -7,        /* ��⵽�Ƿ�����������Ϣ */
	ERR_AAC_SYNTAX_ELEMENT                =  -8,        /* ��⵽�������������� */
	ERR_AAC_DEQUANT                       =  -9,        /* ����������������ģ����� */
	ERR_AAC_STEREO_PROCESS                = -10,        /* ����������������ģ��ģ����� */
	ERR_AAC_PNS                           = -11,        /* ������PNS����ģ��ģ����� */
	ERR_AAC_SHORT_BLOCK_DEINT             = -12,        /* ������չ */
	ERR_AAC_TNS                           = -13,        /* ������TNS����ģ��ģ����� */
	ERR_AAC_IMDCT                         = -14,        /* ������IMDCT����ģ��ģ�����     */
	ERR_AAC_NCHANS_TOO_HIGH               = -15,        /* ��֧�Ŷ��������룬���֧������������ */
	ERR_AAC_SBR_INIT                      = -16,        /* ������SBR����ģ���ʼ������ */
	ERR_AAC_SBR_BITSTREAM                 = -17,        /* ��⵽SBR������Ϣ�Ƿ� */
	ERR_AAC_SBR_DATA                      = -18,        /* ������SBR���ݷǷ� */
	ERR_AAC_SBR_PCM_FORMAT                = -19,        /* ������SBR���ݵ�PCM_FORMAT��Ϣ�Ƿ� */
	ERR_AAC_SBR_NCHANS_TOO_HIGH           = -20,        /* ��֧��SBR���������룬���֧������������ */
	ERR_AAC_SBR_SINGLERATE_UNSUPPORTED    = -21,        /* SBR����Ƶ�ʷǷ� */
	ERR_AAC_RAWBLOCK_PARAMS               = -22,        /* ��⵽SetRawBlockParams��������Ƿ� */
	ERR_AAC_PS_INIT                       = -23,        /* ������PS����ģ���ʼ������ */       
	ERR_AAC_CH_MAPPING                    = -24,
	ERR_UNKNOWN               = -9999                   /* ������չ */
};

typedef struct _AACFrameInfo {
	int bitRate;
	int nChans;                      /* 1 �� 2 */
	int sampRateCore;                /* �ں˽����������ʣ�������Ϣ������Ϊ�������ʹ�� */
	int sampRateOut;                 /* ������������� */
	int bitsPerSample;               /* 16��Ŀǰ��֧��16����λ����� */
	int outputSamps;                 /* nChans*SamplePerFrame */ 
	int profile;                     /* profile,   ������Ϣ������Ϊ�������ʹ�� */
	int tnsUsed;                     /* TNS Tools��������Ϣ������Ϊ�������ʹ�� */
	int pnsUsed;                     /* PNS Tools��������Ϣ������Ϊ�������ʹ�� */
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
