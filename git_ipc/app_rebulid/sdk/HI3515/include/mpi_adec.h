/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : ai.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/6/15
  Description   : 
  History       :
  1.Date        : 2009/6/19
    Author      : p00123320
    Modification: Created file    
******************************************************************************/


#ifndef _MPI_ADEC_H__
#define _MPI_ADEC_H__

#include "hi_common.h"
#include "hi_comm_aio.h"
#include "hi_comm_adec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */



HI_S32 HI_MPI_ADEC_CreateChn(ADEC_CHN AdChn, ADEC_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_ADEC_DestroyChn(ADEC_CHN AdChn);

HI_S32 HI_MPI_ADEC_SendStream(ADEC_CHN AdChn, 
        const AUDIO_STREAM_S *pstStream, HI_U32 u32BlockFlag);

HI_S32 HI_MPI_ADEC_ClearChnBuf(ADEC_CHN AdChn);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

