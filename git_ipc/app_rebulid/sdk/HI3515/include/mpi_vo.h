/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_vo.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/01/12
  Description   : Vou API
  History       :
  1.Date        : 2009/01/21
    Author      : x00100808
    Modification: Created file

******************************************************************************/

#ifndef __MPI_VO_H__
#define __MPI_VO_H__

#include "hi_comm_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */


/* Device Settings */

HI_S32 HI_MPI_VO_Enable (VO_DEV VoDev);
HI_S32 HI_MPI_VO_Disable(VO_DEV VoDev);

HI_S32 HI_MPI_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr);
HI_S32 HI_MPI_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);

HI_S32 HI_MPI_VO_CloseFd(HI_VOID);


/* Video Settings */

HI_S32 HI_MPI_VO_EnableVideoLayer (VO_DEV VoDev);
HI_S32 HI_MPI_VO_DisableVideoLayer(VO_DEV VoDev);

HI_S32 HI_MPI_VO_SetVideoLayerAttr(VO_DEV VoDev, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);
HI_S32 HI_MPI_VO_GetVideoLayerAttr(VO_DEV VoDev, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);

HI_S32 HI_MPI_VO_SetDisplayRect(VO_DEV VoDev, const RECT_S *pstRect);
HI_S32 HI_MPI_VO_GetDisplayRect(VO_DEV VoDev, RECT_S *pstRect);

HI_S32 HI_MPI_VO_SetValidImgRect(VO_DEV VoDev, const RECT_S *pstRect);
HI_S32 HI_MPI_VO_GetValidImgRect(VO_DEV VoDev, RECT_S *pstRect);


/* General Operation of Channel */

HI_S32 HI_MPI_VO_EnableChn (VO_DEV VoDev, VO_CHN VoChn);
HI_S32 HI_MPI_VO_DisableChn(VO_DEV VoDev, VO_CHN VoChn);

HI_S32 HI_MPI_VO_SetChnAttr(VO_DEV VoDev, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr);
HI_S32 HI_MPI_VO_GetChnAttr(VO_DEV VoDev, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr);

HI_S32 HI_MPI_VO_SetChnField(VO_DEV VoDev, VO_CHN VoChn, const VO_DISPLAY_FIELD_E enField);
HI_S32 HI_MPI_VO_GetChnField(VO_DEV VoDev, VO_CHN VoChn, VO_DISPLAY_FIELD_E *pField);

HI_S32 HI_MPI_VO_SetChnFrameRate(VO_DEV VoDev, VO_CHN VoChn, HI_S32 s32ChnFrmRate);
HI_S32 HI_MPI_VO_GetChnFrameRate(VO_DEV VoDev, VO_CHN VoChn, HI_S32 *ps32ChnFrmRate);

HI_S32 HI_MPI_VO_GetChnFrame    (VO_DEV VoDev, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstFrame);
HI_S32 HI_MPI_VO_ReleaseChnFrame(VO_DEV VoDev, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstFrame);

HI_S32 HI_MPI_VO_ChnPause (VO_DEV VoDev, VO_CHN VoChn);
HI_S32 HI_MPI_VO_ChnResume(VO_DEV VoDev, VO_CHN VoChn);
HI_S32 HI_MPI_VO_ChnStep  (VO_DEV VoDev, VO_CHN VoChn);

HI_S32 HI_MPI_VO_ChnShow(VO_DEV VoDev, VO_CHN VoChn);
HI_S32 HI_MPI_VO_ChnHide(VO_DEV VoDev, VO_CHN VoChn);

HI_S32 HI_MPI_VO_SetZoomInWindow(VO_DEV VoDev, VO_CHN VoChn, const VO_ZOOM_ATTR_S *pstZoomAttr);
HI_S32 HI_MPI_VO_GetZoomInWindow(VO_DEV VoDev, VO_CHN VoChn, VO_ZOOM_ATTR_S *pstZoomAttr);

HI_S32 HI_MPI_VO_GetChnPts   (VO_DEV VoDev, VO_CHN VoChn, HI_U64 *pu64ChnPts);
HI_S32 HI_MPI_VO_QueryChnStat(VO_DEV VoDev, VO_CHN VoChn, VO_QUERY_STATUS_S *pstStatus);

HI_S32 HI_MPI_VO_SendFrame(VO_DEV VoDev, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVFrame);

HI_S32 HI_MPI_VO_ClearChnBuffer(VO_DEV VoDev, VO_CHN VoChn, HI_BOOL bClrAll);

HI_S32 HI_MPI_VO_SetAttrBegin(VO_DEV VoDev);
HI_S32 HI_MPI_VO_SetAttrEnd  (VO_DEV VoDev);

HI_S32 HI_MPI_VO_SetChnFilter(VO_DEV VoDev, VO_CHN VoChn, const VO_CHN_FILTER_S *pstFilter);
HI_S32 HI_MPI_VO_GetChnFilter(VO_DEV VoDev, VO_CHN VoChn, VO_CHN_FILTER_S *pstFilter);

HI_S32 HI_MPI_VO_SetPlayToleration(VO_DEV VoDev, HI_U32 u32Toleration);
HI_S32 HI_MPI_VO_GetPlayToleration(VO_DEV VoDev, HI_U32 *pu32Toleration);

HI_S32 HI_MPI_VO_SetChnSrcAttr(VO_DEV VoDev, VO_CHN VoChn, VO_SRC_ATTR_S *pstSrcAttr);

HI_S32 HI_MPI_VO_SetZoomInRatio(VO_DEV VoDev, VO_CHN VoChn, const VO_ZOOM_RATIO_S *pstZoomRatio);
HI_S32 HI_MPI_VO_GetZoomInRatio(VO_DEV VoDev, VO_CHN VoChn, VO_ZOOM_RATIO_S *pstZoomRatio);


/* Vbi Data Operation */

HI_S32 HI_MPI_VO_SetVbiInfo(VO_DEV VoDev, const VO_VBI_INFO_S *pstVbiInfo);
HI_S32 HI_MPI_VO_GetVbiInfo(VO_DEV VoDev, VO_VBI_INFO_S *pstVbiInfo);

HI_S32 HI_MPI_VO_ClrVbiInfo(VO_DEV VoDev);


/* Display Operation of Screen */

HI_S32 HI_MPI_VO_GetScreenFrame    (VO_DEV VoDev, VIDEO_FRAME_INFO_S *pstVFrame);
HI_S32 HI_MPI_VO_ReleaseScreenFrame(VO_DEV VoDev, VIDEO_FRAME_INFO_S *pstVFrame);

HI_S32 HI_MPI_VO_SetDevCSC(VO_DEV VoDev, const VO_CSC_S *pstPubCSC);
HI_S32 HI_MPI_VO_GetDevCSC(VO_DEV VoDev, VO_CSC_S *pstPubCSC);

HI_S32 HI_MPI_VO_SetScreenFilter(VO_DEV VoDev, const VO_SCREEN_FILTER_S *pstFilter);
HI_S32 HI_MPI_VO_GetScreenFilter(VO_DEV VoDev, VO_SCREEN_FILTER_S *pstFilter);

HI_S32 HI_MPI_VO_SetSolidDraw(VO_DEV VoDev, const VO_DRAW_CFG_S *pstDrawCfg);
HI_S32 HI_MPI_VO_GetSolidDraw(VO_DEV VoDev, VO_DRAW_CFG_S *pstDrawCfg);

HI_S32 HI_MPI_VO_SetDispBufLen(VO_DEV VoDev, HI_U32 u32BufLen);
HI_S32 HI_MPI_VO_GetDispBufLen(VO_DEV VoDev, HI_U32 *pu32BufLen);


/* Cascade Operation of Multi-Chip */

HI_S32 HI_MPI_VO_EnableCascade (HI_VOID);
HI_S32 HI_MPI_VO_DisableCascade(HI_VOID);

HI_S32 HI_MPI_VO_SetCascadeMode(HI_BOOL bSlave);
HI_S32 HI_MPI_VO_GetCascadeMode(HI_BOOL *pbSlave);

HI_S32 HI_MPI_VO_SetCascadePattern(HI_U32 u32Pattern);
HI_S32 HI_MPI_VO_GetCascadePattern(HI_U32 *pu32Pattern);

HI_S32 HI_MPI_VO_CascadePosBindChn  (HI_U32 u32Pos, VO_CHN VoChn);
HI_S32 HI_MPI_VO_CascadePosUnBindChn(HI_U32 u32Pos, VO_CHN VoChn);


/* Synchronization Operation of Mulit-Channel */

HI_S32 HI_MPI_VO_CreateSyncGroup (VO_GRP VoGroup, VO_DEV VoDev);
HI_S32 HI_MPI_VO_DestroySyncGroup(VO_GRP VoGroup);

HI_S32 HI_MPI_VO_RegisterSyncGroup  (VO_DEV VoDev, VO_CHN VoChn, VO_GRP VoGroup);
HI_S32 HI_MPI_VO_UnRegisterSyncGroup(VO_DEV VoDev, VO_CHN VoChn, VO_GRP VoGroup);

HI_S32 HI_MPI_VO_SyncGroupStart(VO_GRP VoGroup);
HI_S32 HI_MPI_VO_SyncGroupStop (VO_GRP VoGroup);

HI_S32 HI_MPI_VO_SetSyncGroupBase(VO_GRP VoGroup, HI_U64 u64BasePts);
HI_S32 HI_MPI_VO_GetSyncGroupBase(VO_GRP VoGroup, HI_U64 *pu64BasePts);

HI_S32 HI_MPI_VO_SetSyncGroupFrameRate(VO_GRP VoGroup, HI_S32 s32GrpFrmRate);
HI_S32 HI_MPI_VO_GetSyncGroupFrameRate(VO_GRP VoGroup, HI_S32 *ps32GrpFrmRate);

HI_S32 HI_MPI_VO_PauseSyncGroup (VO_GRP VoGroup);
HI_S32 HI_MPI_VO_ResumeSyncGroup(VO_GRP VoGroup);
HI_S32 HI_MPI_VO_StepSyncGroup  (VO_GRP VoGroup);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__MPI_VO_H__ */


