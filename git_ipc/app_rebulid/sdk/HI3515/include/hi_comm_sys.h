/******************************************************************************
Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : hi3511_sys.h
Version       : Initial Draft
Author        : Hi3511 MPP Team
Created       : 2007/1/30
Last Modified :
Description   : Hi3511 chip specific configure data structure
Function List :
History       :
 1.Date        : 2007/1/30
   Author      : c42025
   Modification: Created file

 2.Date        : 2007/11/30
   Author      : c42025
   Modification: modify according review comments

 3.Date        : 2008/03/03
   Author      : c42025
   Modification: modify HI_TRACE_SYS

 4.Date        : 2008/03/05
   Author      : c42025
   Modification: modify 'HI_LOG_LEVEL_ERROR' to 'EN_ERR_LEVEL_ERROR'

******************************************************************************/
#ifndef __HI_COMM_SYS_H__
#define __HI_COMM_SYS_H__

#include "hi_type.h"
#include "hi_errno.h"
#include "hi_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define HI_TRACE_SYS(level, fmt...) HI_TRACE(level, HI_ID_SYS,##fmt)

typedef struct hiMPP_SYS_CONF_S
{
    /* stride of picture buffer must be aligned with this value.
     * you can choose a value from 1 to 1024,
     * and it except 1 must be multiple of 16.
     */
    HI_U32 u32AlignWidth;  

#define PMC_VI_0_BT601      0x01   /* bit 0~1: VI_0 conect with BT601 */
#define PMC_VI_2_BT601      0x04   /* bit 2~3: VI_1 conect with BT601 */
#define PMC_EN_VI_13        0x10   /* bit 4  : VI_1&VI_3 (or ETH) */

#define PMC_EN_SIO_0        0x0100 /* bit 9  : SIO_0 (or GPIO) */ 
#define PMC_SIO_0_CLK_R     0x0200 /* bit 10 : pSIO0XCK (or pSIO0RCK) */ 
    HI_U32 u32PinMuxCtrl;
}MPP_SYS_CONF_S;


#define HI_ERR_SYS_NULL_PTR  HI_DEF_ERR(HI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define HI_ERR_SYS_NOTREADY  HI_DEF_ERR(HI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define HI_ERR_SYS_NOT_PERM  HI_DEF_ERR(HI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define HI_ERR_SYS_ILLEGAL_PARAM HI_DEF_ERR(HI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define HI_ERR_SYS_BUSY      HI_DEF_ERR(HI_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __HI_COMM_SYS_H__ */

