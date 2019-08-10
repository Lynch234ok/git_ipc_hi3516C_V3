/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_vdec.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2007/01/18
  Description   : 
  History       :
  1.Date        : 2007/01/18
    Author      : z50825
    Modification: Created file
  2.Date        : 2008/1/8
    Author      : l59217
    Modification: Delete get wm attr interface
  3.Date        : 2008/3/4
    Author      : l59217
    Modification: modified HI_MPI_VDEC_GetCapability interface
  4.Date        : 2008/7/19
    Author      : c55300
    Modification: CR20080716007£¨HI_MPI_VDEC_ResetChn() is added.
******************************************************************************/
#ifndef  __MPI_VDEC_H__
#define  __MPI_VDEC_H__

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_vdec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


HI_S32 HI_MPI_VDEC_CreateChn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr, const VDEC_WM_ATTR_S *pstWm);
HI_S32 HI_MPI_VDEC_DestroyChn(VDEC_CHN VdChn);

HI_S32 HI_MPI_VDEC_BindOutput(VDEC_CHN VdChn,VO_DEV VoDev, VO_CHN VoChn);
HI_S32 HI_MPI_VDEC_UnbindOutput(VDEC_CHN VdChn);	
HI_S32 HI_MPI_VDEC_UnbindOutputChn(VDEC_CHN VdChn, VO_DEV VoDev, VO_CHN VoChn);

HI_S32 HI_MPI_VDEC_SetChnAttr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VDEC_GetChnAttr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VDEC_SendStream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, HI_U32 u32BlockFlag);

HI_S32 HI_MPI_VDEC_GetData(VDEC_CHN VdChn, VDEC_DATA_S *pstData, HI_U32 u32BlockFlag);
HI_S32 HI_MPI_VDEC_ReleaseData(VDEC_CHN VdChn, VDEC_DATA_S *pstData);

HI_S32 HI_MPI_VDEC_StartRecvStream(VDEC_CHN VdChn);
HI_S32 HI_MPI_VDEC_StopRecvStream(VDEC_CHN VdChn);
HI_S32 HI_MPI_VDEC_Query(VDEC_CHN VdChn,VDEC_CHN_STAT_S *pstStat);
HI_S32 HI_MPI_VDEC_GetCapability(VDEC_CAPABILITY_S *pstCap);  /*Ω‚¬Î¿‡–Õ*/

HI_S32 HI_MPI_VDEC_GetFd(VDEC_CHN VdChn);

HI_S32 HI_MPI_VDEC_ResetChn(VDEC_CHN VdChn);
HI_S32 HI_MPI_VDEC_QueryData(VDEC_CHN VdChn,HI_BOOL *pbIsData);

HI_S32 HI_MPI_VDEC_SetChnParam(VDEC_CHN VdChn, VDEC_CHN_PARAM_S* pstParam);
HI_S32 HI_MPI_VDEC_GetChnParam(VDEC_CHN VdChn, VDEC_CHN_PARAM_S* pstParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef  __MPI_VDEC_H__ */


