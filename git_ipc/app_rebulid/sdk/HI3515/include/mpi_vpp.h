/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_vpp.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2007/10/27
  Description   : 
  History       :
  1.Date        : 2007/10/27
    Author      : l64467
    Modification: Created file

  1.Date        : 2007/03/05
    Author      : l64467
    Modification: modfiy for CR20080304018

******************************************************************************/
#ifndef __MPI_VPP_H__
#define __MPI_VPP_H__

#include "hi_common.h"
#include "hi_comm_vpp.h"
#include "hi_debug.h"
#include "hi_comm_vi.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */



HI_S32 HI_MPI_VPP_SetConf(VENC_GRP VeGroup, const VIDEO_PREPROC_CONF_S *pstConf);

HI_S32 HI_MPI_VPP_GetConf(VENC_GRP VeGroup, VIDEO_PREPROC_CONF_S *pstConf);

HI_S32 HI_MPI_VPP_CreateRegion(const REGION_ATTR_S *pstRegion, REGION_HANDLE *pHandle);

HI_S32 HI_MPI_VPP_DestroyRegion(REGION_HANDLE Handle);


HI_S32 HI_MPI_VPP_ControlRegion(REGION_HANDLE Handle,
    REGION_CRTL_CODE_E enCtrl, REGION_CTRL_PARAM_U *punParam);

HI_S32 HI_MPI_VPP_CreateScaleTask(const PIC_SCALE_TASK_S *pstTask, 
     VPP_SCALE_CONF_S *pstConf);

HI_S32 HI_MPI_VPP_WaitScaleTask(PIC_SCALE_TASK_S *pstTask, 
    HI_U32 u32BlockFlag, HI_U32 u32Timeout);

HI_S32 HI_MPI_VPP_SetDsuFiltParam(DSU_FILTER_PARAM_S *pstDsuFiltParam);

HI_S32 HI_MPI_VPP_GetDsuFiltParam(DSU_FILTER_PARAM_S *pstDsuFiltParam);

HI_S32 HI_MPI_VPP_GetFd();
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MPI_VPP_H__ */

