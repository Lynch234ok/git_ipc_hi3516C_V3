/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_priv_avenc.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2006/04/05
  Description   : The private head file of AVENC
  History       :
  1.Date        : 2006/04/05
    Author      : Z44949
    Modification: Created file

******************************************************************************/

#ifndef __HI_COMM_AVENC_H__
#define __HI_COMM_AVENC_H__

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "hi_debug.h"
#include "hi_common.h"
#include "hi_comm_aio.h"
#include "hi_comm_venc.h"
#include "hi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


/*--------------------------Error coder defination---------------------------*/
//#define HI_ID_AVENC    1
/*  Invalid channel */
#define HI_ERR_AVENC_INVALID_CHN      HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 1)

/* The VENC channel is in use */
#define HI_ERR_AVENC_VCHN_USING      HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 2)

/* The AENC channel is in use */
#define HI_ERR_AVENC_ACHN_USING      HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 3)

/* The pointer may be NULL */
#define HI_ERR_AVENC_INVALID_PTR     HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 4)

/* The AVENC channel not be created */
#define HI_ERR_AVENC_CHN_NOT_CREATE  HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 5)

/* AVENC channel is runing */
#define HI_ERR_AVENC_CHN_NOT_STOP    HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 6)

/* The AVENC has been started */
#define HI_ERR_AVENC_CHN_RESTART     HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 7)

/* The AENC channel not exist */
#define HI_ERR_AVENC_AUDIO_NOT_EXIST HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 8)

/* The VENC channel not exist */
#define HI_ERR_AVENC_VIDEO_NOT_EXIST HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 9)

/* The AENC channel has been used by others */
#define HI_ERR_AVENC_AUDIO_USEDBYOTHER HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 10)

/* The VENC channel has been used by others */
#define HI_ERR_AVENC_VIDEO_USEDBYOTHER HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 11)

/* The AVENC channel has not been started */
#define HI_ERR_AVENC_CHN_NOT_START   HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 12)

/* Some stream buffer may be hold by user. */
#define HI_ERR_AVENC_BUFFER_NOTFREE  HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 13)

/* Invalid operate object */
#define HI_ERR_AVENC_INVALID_OBJECT  HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 14)

/* Not stream available */
#define HI_ERR_AVENC_NO_STREAM       HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 15)

/* The AENC channel attribute is noblock */
#define HI_ERR_AVENC_AUDIONONBLOCK   HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 16)

/* The water level is invalid */
#define HI_ERR_AVENC_INVALID_WATERLEVEL HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 17)

/* No stream available, and wait time has been out */
#define HI_ERR_AVENC_TIMEOUT         HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 18)

/* Invalid audio buffer unit size  */
#define HI_ERR_AVENC_INVALID_AUSIZE  HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 19)

#define HI_ERR_AVENC_INVALID_PARA    HI_DEF_ERR(HI_ID_AVENC, EN_ERR_LEVEL_ERROR, 20)


/* The list node struct */
typedef struct hiAVENC_LIST_S
{
    HI_VOID               *pData;  /* The data pointer        */
    struct hiAVENC_LIST_S *pNext;  /* The list next pointer   */
} AVENC_LIST_S;

/* AVENC stream frame defination */
typedef struct hiAVENC_STREAM_S
{
    AVENC_LIST_S *pAudioListHead;     /* The audio list head        */
    AVENC_LIST_S *pVideoListHead;     /* The video list head        */
    HI_U32        u32Seq;             /* The AVENC stream sequence. */
} AVENC_STREAM_S;

/* Define the object which can be used for AVENC */
typedef enum hiAVENC_OPERATE_OBJECT_E
{
    AVENC_OPERATION_BOTH =0,  /* Operate audio and video both */
    AVENC_OPERATION_AUDIO=1,  /* Operate audio only           */ 
    AVENC_OPERATION_VIDEO=2,  /* Operate video only           */
    AVENC_OPERATION_BUTT
}AVENC_OPERATE_OBJECT_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __HI_PRIV_AVENC_H__ */


