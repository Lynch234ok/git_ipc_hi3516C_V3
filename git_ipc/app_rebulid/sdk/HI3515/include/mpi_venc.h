/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_venc.h
  Version       : Initial Draft
  Author        : Hisilicon Hi3511 MPP Team
  Created       : 2006/11/22
  Last Modified :
  Description   : mpi functions declaration
  Function List :
  History       :
  1.Date        : 2006/11/22
    Author      : z50929
    Modification: Create
  2.Date        : 2007/11/22
    Author      : z50825
    Modification: Create
  3.Date        : 2008/7/18
    Author      : l64467
    Modification: Add interface for CR20080716031
    			  HI_MPI_VENC_SetMaxStreamCnt;
				  HI_MPI_VENC_GetMaxStreamCnt;
******************************************************************************/
#ifndef __MPI_VENC_H__
#define __MPI_VENC_H__

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_venc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


HI_S32 HI_MPI_VENC_CreateGroup(VENC_GRP VeGroup);
HI_S32 HI_MPI_VENC_DestroyGroup(VENC_GRP VeGroup);

HI_S32 HI_MPI_VENC_BindInput(VENC_GRP VeGroup, VI_DEV ViDevId, VI_CHN ViChn);
HI_S32 HI_MPI_VENC_UnbindInput(VENC_GRP VeGroup);

HI_S32 HI_MPI_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr, const VENC_WM_ATTR_S *pstWm);
HI_S32 HI_MPI_VENC_DestroyChn(VENC_CHN VeChn);

HI_S32 HI_MPI_VENC_RegisterChn(VENC_GRP VeGroup,VENC_CHN VeChn );
HI_S32 HI_MPI_VENC_UnRegisterChn(VENC_CHN VeChn);

HI_S32 HI_MPI_VENC_StartRecvPic(VENC_CHN VeChn);
HI_S32 HI_MPI_VENC_StopRecvPic(VENC_CHN VeChn);
HI_S32 HI_MPI_VENC_Query(VENC_CHN VeChn,VENC_CHN_STAT_S *pstStat);

HI_S32 HI_MPI_VENC_SetChnAttr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VENC_GetChnAttr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstAttr);

HI_S32 HI_MPI_VENC_SetWmAttr(VENC_CHN VeChn, const VENC_WM_ATTR_S *pstWm);
HI_S32 HI_MPI_VENC_GetWmAttr(VENC_CHN VeChn, VENC_WM_ATTR_S *pstWm);

HI_S32 HI_MPI_VENC_EnableWm(VENC_CHN VeChn);
HI_S32 HI_MPI_VENC_DisableWm(VENC_CHN VeChn);

HI_S32 HI_MPI_VENC_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, HI_U32 u32BlockFlag);
HI_S32 HI_MPI_VENC_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream);

HI_S32 HI_MPI_VENC_RequestIDR(VENC_CHN VeChn);
HI_S32 HI_MPI_VENC_InsertUserData(VENC_CHN VeChn, HI_U8 *pu8Data, HI_U32 u32Len);

HI_S32 HI_MPI_VENC_SendFrame(VI_DEV ViDevId, VI_CHN ViChn, HI_U32 u32PoolId, VIDEO_FRAME_S *pstFrame, HI_BOOL bIDR);
HI_S32 HI_MPI_VENC_GetCapability(VENC_CAPABILITY_S *pstCap);  

HI_S32 HI_MPI_VENC_SetMaxStreamCnt(VENC_CHN VeChn,HI_U32 u32MaxStrmCnt);
HI_S32 HI_MPI_VENC_GetMaxStreamCnt(VENC_CHN VeChn,HI_U32 *pu32MaxStrmCnt);

HI_S32 HI_MPI_VENC_SetH264eRcPara(VENC_CHN VeChn, VENC_ATTR_H264_RC_S *pstAttr);
HI_S32 HI_MPI_VENC_GetH264eRcPara(VENC_CHN VeChn, VENC_ATTR_H264_RC_S *pstAttr);

HI_S32 HI_MPI_VENC_GetFd(VENC_CHN VeChn);
HI_S32 HI_MPI_GRP_GetFd(VENC_CHN VeChn);

HI_S32 HI_MPI_VENC_CfgMestPara(VENC_CHN VeChn, VENC_ATTR_MEPARA_S * pstParam);
HI_S32 HI_MPI_VENC_GetMestPara(VENC_CHN VeChn, VENC_ATTR_MEPARA_S * pstParam);

HI_S32 HI_MPI_VENC_SetH264eNaluPara( VENC_CHN VeChn, VENC_ATTR_H264_NALU_S *pstH264eNalu);
HI_S32 HI_MPI_VENC_GetH264eNaluPara( VENC_CHN VeChn, VENC_ATTR_H264_NALU_S *pstH264eNalu);

HI_S32 HI_MPI_VENC_SetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E enRefMode);
HI_S32 HI_MPI_VENC_GetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E* penRefMode);

HI_S32 HI_MPI_VENC_SetParamSet(VENC_CHN VeChn, VENC_PARAM_SET_S * pstParamSet);
HI_S32 HI_MPI_VENC_GetParamSet(VENC_CHN VeChn, VENC_PARAM_SET_S * pstParamSet);

HI_S32 HI_MPI_VENC_GetMeParam(VENC_CHN VeChn, VENC_ME_PARAM_S* pstMeParam);
HI_S32 HI_MPI_VENC_SetMeParam(VENC_CHN VeChn, VENC_ME_PARAM_S* pstMeParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MPI_VENC_H__ */
