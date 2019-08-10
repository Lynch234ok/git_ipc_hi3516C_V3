#ifndef __MPI_VI_H__
#define __MPI_VI_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "hi_comm_vi.h"

HI_S32 HI_MPI_VI_SetDevAttr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr);
HI_S32 HI_MPI_VI_GetDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr);

HI_S32 HI_MPI_VI_EnableDev(VI_DEV ViDev);
HI_S32 HI_MPI_VI_DisableDev(VI_DEV ViDev);

HI_S32 HI_MPI_VI_SetChnAttr(VI_CHN ViChn, const VI_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VI_GetChnAttr(VI_CHN ViChn, VI_CHN_ATTR_S *pstAttr);

/* The following 3 functions are only for vichn minor attributes */
HI_S32 HI_MPI_VI_SetChnMinorAttr(VI_CHN ViChn,const VI_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VI_GetChnMinorAttr(VI_CHN ViChn,VI_CHN_ATTR_S *pstAttr);
HI_S32 HI_MPI_VI_ClearChnMinorAttr(VI_CHN ViChn);

HI_S32 HI_MPI_VI_EnableChn(VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableChn(VI_CHN ViChn);

HI_S32 HI_MPI_VI_EnableChnInterrupt(VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableChnInterrupt(VI_CHN ViChn);

HI_S32 HI_MPI_VI_GetFrame(VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo);
HI_S32 HI_MPI_VI_GetFrameTimeOut(VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, HI_U32 u32MilliSec);
HI_S32 HI_MPI_VI_ReleaseFrame(VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo);
HI_S32 HI_MPI_VI_SetFrameDepth(VI_CHN ViChn, HI_U32 u32Depth);
HI_S32 HI_MPI_VI_GetFrameDepth(VI_CHN ViChn, HI_U32 *pu32Depth);
HI_S32 HI_MPI_VI_SetUserPic(VI_CHN ViChn, VI_USERPIC_ATTR_S *pstUsrPic);
HI_S32 HI_MPI_VI_EnableUserPic(VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableUserPic(VI_CHN ViChn);

/* These functions are used to start the cascade mode. VI cascade mode can work normally Only when they have been called */
HI_S32 HI_MPI_VI_EnableCascade(VI_DEV ViDev); 
HI_S32 HI_MPI_VI_DisableCascade(VI_DEV ViDev); 
HI_S32 HI_MPI_VI_EnableCascadeChn(VI_CHN ViChn);
HI_S32 HI_MPI_VI_DisableCascadeChn(VI_CHN ViChn);

/* Normally, these functions are not necessary in typical business */
HI_S32 HI_MPI_VI_ChnBind(VI_CHN ViChn, const VI_CHN_BIND_ATTR_S *pstChnBindAttr);
HI_S32 HI_MPI_VI_ChnUnBind(VI_CHN ViChn);
HI_S32 HI_MPI_VI_GetChnBind(VI_CHN ViChn, VI_CHN_BIND_ATTR_S *pstChnBindAttr);

HI_S32 HI_MPI_VI_SetDevAttrEx(VI_DEV ViDev, const VI_DEV_ATTR_EX_S *pstDevAttrEx);
HI_S32 HI_MPI_VI_GetDevAttrEx(VI_DEV ViDev, VI_DEV_ATTR_EX_S *pstDevAttrEx);

HI_S32 HI_MPI_VI_GetFd(VI_CHN ViChn);

HI_S32 HI_MPI_VI_Query(VI_CHN ViChn, VI_CHN_STAT_S *pstStat);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__MPI_VI_H__ */



