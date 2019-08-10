/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_avenc.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2006/01/19
  Description   : 
  History       :
  1.Date        : 2006/01/19
    Author      : Z44949
    Modification: Created file

******************************************************************************/

#ifndef __MPI_AVENC_H__
#define __MPI_AVENC_H__

#include "hi_errno.h"
#include "hi_comm_avenc.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


/* Create AVENC channel */
HI_S32 HI_MPI_AVENC_CreatCH(VENC_CHN VChnId,AENC_CHN AChnId,AVENC_CHN *pAVChnId);


/* Destroy the AVENC channel */
HI_S32 HI_MPI_AVENC_DestroyCH(AVENC_CHN AVChnId, HI_BOOL bFlag);

/* Start the AVENC channel */
HI_S32 HI_MPI_AVENC_StartCH(AVENC_CHN AVChnId, 
                        AVENC_OPERATE_OBJECT_E enObject,
                        HI_S32 s32Priority);

/* Stop the AVENC channel */
HI_S32 HI_MPI_AVENC_StopCH(AVENC_CHN ChanID, AVENC_OPERATE_OBJECT_E enObject);


/*
** Get the AVENC stream. The audio and video is synchronization 
** The sample follow shows how to access the first audio data and video data 
**   AENC_STREAM_S *pAudioData;
**   pAudioData = (AENC_STREAM_S *)(pStream->pAudioListHead->pData);
**   
**   VENC_STREAM_S *pVideoData;
**   pVideoData = (VENC_STREAM_S *)(pStream->pVideoListHead->pData);
*/
HI_S32 HI_MPI_AVENC_GetStream(AVENC_CHN ChanID, AVENC_STREAM_S *pStream,HI_U32 u32Block);

/* After the stream is used, user application must release the stream. */
HI_S32 HI_MPI_AVENC_ReleaseStream(AVENC_CHN ChanID, AVENC_STREAM_S *pStream);

#if 0
/* Define the function type of audio encoder function */
typedef HI_S32 (*AENC_GetStream_Fun)(AUDIO_DEV AiDev, AI_CHN AiChn, 
    AENC_CHN AencChn, AUDIO_STREAM_S *pstStream);
#else
/* Define the function type of audio encoder function */
typedef HI_S32 (*AENC_GetStream_Fun)(AENC_CHN AencChn, AUDIO_STREAM_S *pstStream, HI_U32 u32BlockFlag);

#endif

/* Register the audio encoder funtion */
HI_S32 HI_MPI_AVENC_RegisterFun(AVENC_CHN AVChnId, AENC_GetStream_Fun pFun);

#define AVENC_MAX_OVERFLOWLEVEL (~0UL)>>1
/*
** Set the water level of queue over flow
** WARNING: You should be careful to set this property
*/
HI_S32 HI_MPI_AVENC_SetOverFlowLevel(AVENC_CHN ChanId,
                        AVENC_OPERATE_OBJECT_E enObject,
                        HI_S32 s32Value);

/* Get the water level of queue over flow */
HI_S32 HI_MPI_AVENC_GetOverFlowLevel(AVENC_CHN ChanId,
                        AVENC_OPERATE_OBJECT_E enObject,
                        HI_S32 *pValue);

/* Set the buffer unit size, only audio buffer can be setted now. */
HI_S32 HI_MPI_AVENC_SetUnitBufSize(AVENC_OPERATE_OBJECT_E enObject, HI_S32 s32Value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __HI_AVENC_H__ */
