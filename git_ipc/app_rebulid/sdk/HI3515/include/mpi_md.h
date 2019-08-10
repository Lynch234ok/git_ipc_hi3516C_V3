/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_md.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2007/03/27
  Description   : 
  History       :
  1.Date        : 2007/03/27
    Author      : l64467
    Modification: Created file

******************************************************************************/
#ifndef __MPI_MD_H__
#define __MPI_MD_H__

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_md.h"

#ifdef __cplusplus
    #if __cplusplus
    extern "C"{
    #endif
#endif /* End of #ifdef __cplusplus */

HI_S32 HI_MPI_MD_EnableChn(VENC_CHN VeChn);

HI_S32 HI_MPI_MD_DisableChn(VENC_CHN VeChn);

HI_S32 HI_MPI_MD_SetChnAttr(VENC_CHN VeChn, const MD_CHN_ATTR_S *pstAttr);

HI_S32 HI_MPI_MD_GetChnAttr(VENC_CHN VeChn, MD_CHN_ATTR_S *pstAttr);

HI_S32 HI_MPI_MD_SetRefFrame(VENC_CHN VeChn, const MD_REF_ATTR_S *pstAttr);

HI_S32 HI_MPI_MD_GetRefFrame(VENC_CHN VeChn, MD_REF_ATTR_S *pstAttr);

HI_S32 HI_MPI_MD_GetData(VENC_CHN VeChn, MD_DATA_S *pstMdData, HI_U32 u32BlockFlag);

HI_S32 HI_MPI_MD_ReleaseData(VENC_CHN VeChn, const MD_DATA_S* pstMdData);

HI_S32 HI_MPI_MD_GetFd(VENC_CHN VeChn);

#ifdef __cplusplus
    #if __cplusplus
    }
    #endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __MPI_MD_H__ */


