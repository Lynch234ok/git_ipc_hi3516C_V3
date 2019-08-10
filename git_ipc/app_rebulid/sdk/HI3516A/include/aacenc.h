/*****************************************************************************
*             Copyright 2004 - 2050, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hi_spdif.h
* Description:
*
* History:
* Version   Date         Author     DefectNum    Description
* 0.01      2006-11-01   z40717    NULL         Create this file.
*
*****************************************************************************/

#ifndef _AACENC_H
#define _AACENC_H

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif     /* __cpluscplus */
#endif  /* __cpluscplus */

#include "hi_type.h"

/* here we distinguish between stereo and the mono only encoder */
#ifdef MONO_ONLY
 #define MAX_CHANNELS 1
#else
 #define MAX_CHANNELS 2
#endif

#define AACENC_BLOCKSIZE 1024   /*! encoder only takes BLOCKSIZE samples at a time */

typedef enum
{
    AU_QualityExcellent = 0,
    AU_QualityHigh   = 1,
    AU_QualityMedium = 2,
    AU_QualityLow = 3,
} AuQuality;

typedef enum
{
    AACLC = 0,                /* AAC LC */
    EAAC = 1,                 /* eAAC  (HEAAC or AAC+  or aacPlusV1) */
    EAACPLUS = 2,             /* eAAC+ (AAC++ or aacPlusV2) */
} AuEncoderFormat;

typedef  struct
{
    AuQuality       quality;
    AuEncoderFormat coderFormat;
    HI_S16          bitsPerSample;
    HI_S32          sampleRate;    /* audio file sample rate */
    HI_S32          bitRate;       /* encoder bit rate in bits/sec */
    HI_S16          nChannelsIn;   /* number of channels on input (1,2) */
    HI_S16          nChannelsOut;  /* number of channels on output (1,2) */
    HI_S16          bandWidth;     /* targeted audio bandwidth in Hz */
} AACENC_CONFIG;

typedef struct AAC_ENCODER AAC_ENCODER_S;

typedef struct hiAACENC_VERSION_S
{
    HI_U8 aVersion[64];
} AACENC_VERSION_S;

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
HI_S32	HI_AACENC_GetVersion(AACENC_VERSION_S *pVersion);

/*****************************************************************************
  Function:    AACInitDefaultConfig
  Description: gives reasonable default configuration
  Calls:
  Called By:
  Input:
  Output:      pstConfig  --pointer to an configuration information structure.
  Return:      HI_SUCCESS if success
  Others:
*****************************************************************************/
HI_S32	AACInitDefaultConfig(AACENC_CONFIG *pstConfig);

//HI_S32	AACInitGetDefaultConfig(AACENC_CONFIG *pstConfig, AuEncoderFormat enFormat);
//HI_S32	AACCheckConfig(AACENC_CONFIG *pstConfig);

/*****************************************************************************
  Function:    AACEncoderOpen
  Description: allocate and initialize a new encoder instance
  Calls:
  Called By:
  Input:       phAacPlusEnc   --pointer to an encoder handle, initialized on return
               pstConfig      --pre-initialized config struct
  Output:
  Return:      HI_SUCCESS if success
  Others:
*****************************************************************************/
HI_S32	AACEncoderOpen(AAC_ENCODER_S **phAacPlusEnc, AACENC_CONFIG *pstConfig);

/*****************************************************************************
  Function:    AACEncoderFrame
  Description: encoder input PCM samples into .aac formate bitstream
  Calls:
  Called By:
  Input:       hAacPlusEnc   --encoder handle
               ps16PcmBuf    --BLOCKSIZE*nChannels audio samples, interleaved
               pu8Outbuf      --pointer to output buffer
                               (must be 6144/8*MAX_CHANNELS bytes large)
               ps32NumOutBytes   --number of bytes in output buffer after processing
  Output:
  Return:      HI_SUCCESS if success
  Others:
*****************************************************************************/
HI_S32	AACEncoderFrame(AAC_ENCODER_S * hAacPlusEnc, HI_S16 *ps16PcmBuf,
                        HI_U8 *pu8Outbuf, HI_S32 *ps32NumOutBytes);

/*****************************************************************************
  Function:    AACEncoderClose
  Description: close encoder device
  Calls:
  Called By:
  Input:       hAacPlusEnc   --encoder handle
  Output:
  Return:
  Others:
*****************************************************************************/
HI_VOID AACEncoderClose (AAC_ENCODER_S * hAacPlusEnc);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif     /* __cpluscplus */
#endif  /* __cpluscplus */

#endif  /* _AACENC_H */
