/******************************************************************************
 
  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
 
 ******************************************************************************
  File Name     : mpi_vi.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/3/9
  Description   : 
  History       :
  1.Date        : 2009/3/9
    Author      : p00123320
    Modification: Created file
    
******************************************************************************/
#ifndef __MPI_VI_H__
#define __MPI_VI_H__

#include "hi_comm_vi.h"
/******************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */
/******************************************/


HI_S32 HI_MPI_VI_SetPubAttr(VI_DEV ViDevId,const VI_PUB_ATTR_S *pstPubAttr);
HI_S32 HI_MPI_VI_GetPubAttr(VI_DEV ViDevId,VI_PUB_ATTR_S *pstPubAttr);

HI_S32 HI_MPI_VI_SetChnAttr(VI_DEV ViDevId,VI_CHN ViChn,const VI_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VI_GetChnAttr(VI_DEV ViDevId,VI_CHN ViChn,VI_CHN_ATTR_S *pstAttr);

HI_S32 HI_MPI_VI_Enable(VI_DEV ViDevId);
HI_S32 HI_MPI_VI_Disable(VI_DEV ViDevId);

HI_S32 HI_MPI_VI_EnableChn(VI_DEV ViDevId,VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableChn(VI_DEV ViDevId,VI_CHN ViChn);

HI_S32 HI_MPI_VI_BindOutput(VI_DEV ViDevId,VI_CHN ViChn,VO_DEV VoDevId,VO_CHN VoChn);
HI_S32 HI_MPI_VI_UnBindOutput(VI_DEV ViDevId,VI_CHN ViChn,VO_DEV VoDev,VO_CHN VoChn);

HI_S32 HI_MPI_VI_GetFrame(VI_DEV ViDevId,VI_CHN ViChn,VIDEO_FRAME_INFO_S *pstFrame);
HI_S32 HI_MPI_VI_ReleaseFrame(VI_DEV ViDevId,VI_CHN ViChn,const VIDEO_FRAME_INFO_S *pstRawFrame);

HI_S32 HI_MPI_VI_GetChnLuma(VI_DEV ViDevId,VI_CHN ViChn,VI_CH_LUM_S *pstLuma);

HI_S32 HI_MPI_VI_GetFd(VI_DEV ViDevId,VI_CHN ViChn);


HI_S32 HI_MPI_VI_SetSrcFrameRate(VI_DEV ViDevId,VI_CHN ViChn,HI_U32 u32ViFramerate);
HI_S32 HI_MPI_VI_GetSrcFrameRate(VI_DEV ViDevId,VI_CHN ViChn,HI_U32 *pu32ViFramerate);
HI_S32 HI_MPI_VI_SetFrameRate(VI_DEV ViDevId,VI_CHN ViChn,HI_U32 u32ViFramerate);
HI_S32 HI_MPI_VI_GetFrameRate(VI_DEV ViDevId,VI_CHN ViChn,HI_U32 *pu32ViFramerate);

HI_S32 HI_MPI_VI_SetChnMinorAttr(VI_DEV ViDevId,VI_CHN ViChn,const VI_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VI_GetChnMinorAttr(VI_DEV ViDevId,VI_CHN ViChn,VI_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VI_ClearChnMinorAttr(VI_DEV ViDevId,VI_CHN ViChn);

HI_S32 HI_MPI_VI_SetUserPic(VI_DEV ViDevId,VI_CHN ViChn,VIDEO_FRAME_INFO_S *pstVFrame);
HI_S32 HI_MPI_VI_SetUserPicEx(VI_DEV ViDevId,VI_CHN ViChn, VI_USERPIC_ATTR_S *pstUsrPicAttr);
HI_S32 HI_MPI_VI_EnableUserPic(VI_DEV ViDevId,VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableUserPic(VI_DEV ViDevId,VI_CHN ViChn);

HI_S32 HI_MPI_VI_EnableLumstrh(VI_DEV ViDevId,VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableLumstrh(VI_DEV ViDevId,VI_CHN ViChn);


HI_S32 HI_MPI_VI_GetDbgInfo(VI_DEV ViDevId,VI_CHN ViChn,VI_DBG_INFO_S *pstDbgInfo);

HI_S32 HI_MPI_VI_SetVbiCfg(VI_DEV ViDevId, VI_CHN ViChn, HI_S32 s32VbiId, VI_VBI_ATTR_S *pVbiCfg);
HI_S32 HI_MPI_VI_GetVbiCfg(VI_DEV ViDevId, VI_CHN ViChn, HI_S32 s32VbiId, VI_VBI_ATTR_S *pVbiCfg);
HI_S32 HI_MPI_VI_EnableVbi(VI_DEV ViDevId, VI_CHN ViChn, HI_S32 s32VbiId);
HI_S32 HI_MPI_VI_DisableVbi(VI_DEV ViDevId, VI_CHN ViChn, HI_S32 s32VbiId);

HI_S32 HI_MPI_VI_EnableCascade(VI_DEV ViDevId, VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableCascade(VI_DEV ViDevId, VI_CHN ViChn);

HI_S32 HI_MPI_VI_SetLumaCoef(VI_DEV ViDevId, VI_CHN ViChn, VI_LUMA_COEF_S *pstLumaCoef);
HI_S32 HI_MPI_VI_SetChromaCoef(VI_DEV ViDevId, VI_CHN ViChn, VI_CHROMA_COEF_S *pstChromaCoef);


/* Setup CH ID of AD codec, only used in BT.656 interface */
HI_S32 HI_MPI_VI_SetAdChnId(VI_DEV ViDevId, VI_CHN ViChn, HI_U32 u32AdChnId);
HI_S32 HI_MPI_VI_GetAdChnId(VI_DEV ViDevId, VI_CHN ViChn, HI_U32 *pu32AdChnId);


HI_S32 HI_MPI_VI_GetSrcCfg(VI_DEV ViDevId, VI_CHN ViChn, VI_SRC_CFG_S *pstSrcCfg);
HI_S32 HI_MPI_VI_SetSrcCfg(VI_DEV ViDevId, VI_CHN ViChn, const VI_SRC_CFG_S *pstSrcCfg);



/******************************************/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
/******************************************/
#endif /*__MPI_VI_H__ */

