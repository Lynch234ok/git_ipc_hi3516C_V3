/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mkp_vd.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/09/14
  Description   :
  History       :
  1.Date        : 2009/09/14
    Author      : x00100808
    Modification: Created file

******************************************************************************/

#ifndef  __MKP_VD_H__
#define  __MKP_VD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_errno.h"
#include "hi_common.h"
#include "hi_comm_vo.h"

typedef enum hiEN_VD_ERR_CODE_E
{
    EN_ERR_VD_DEV_NOT_CONFIG	  = 0x40,
    EN_ERR_VD_DEV_NOT_ENABLE      = 0x41,
    EN_ERR_VD_DEV_NOT_DISABLE     = 0x42,
    EN_ERR_VD_DEV_HAS_BINDED      = 0x43,
    EN_ERR_VD_DEV_NOT_BINDED      = 0x44,

	EN_ERR_GRAPHIC_NOT_DISABLE    = 0x45,

    ERR_VD_BUTT

}EN_VD_ERR_CODE_E;

/* System define error code */
#define HI_ERR_VD_NULL_PTR              HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define HI_ERR_VD_INVALID_DEVID         HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)
#define HI_ERR_VD_ILLEGAL_PARAM         HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define HI_ERR_VD_NOT_SURPPORT          HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SURPPORT)

/* device relative error code */
#define HI_ERR_VD_DEV_NOT_CONFIG        HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_CONFIG)
#define HI_ERR_VD_DEV_NOT_ENABLE        HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_ENABLE)
#define HI_ERR_VD_DEV_NOT_DISABLE       HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_DISABLE)
#define HI_ERR_VD_DEV_HAS_BINDED        HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_HAS_BINDED)
#define HI_ERR_VD_DEV_NOT_BINDED        HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_VO_DEV_NOT_BINDED)
#define HI_ERR_VD_GRAPHIC_NOT_DISABLE	HI_DEF_ERR(HI_ID_VD, EN_ERR_LEVEL_ERROR, EN_ERR_GRAPHIC_NOT_DISABLE)

#define IOC_TYPE_VD		'W'

typedef enum hiIOC_NR_VD_E
{
    /* device operation */
	IOC_NR_VD_SET_DEV_ID = 0,
    IOC_NR_VD_ENABLE_DEV,
    IOC_NR_VD_DISABLE_DEV,
	IOC_NR_VD_SET_DEV_ATTR,
	IOC_NR_VD_GET_DEV_ATTR,
	IOC_NR_VD_SET_GRAPHIC_BIND,
	IOC_NR_VD_GET_GRAPHIC_BIND,

    IOC_NR_VD_BUTT

} IOC_NR_VD_E;

/* the following IOCTL is used for general vo device operation */
#define VD_ENABLE_DEV        		         _IOW(IOC_TYPE_VD,   IOC_NR_VD_ENABLE_DEV, HI_S32)
#define VD_DISABLE_DEV                       _IOW(IOC_TYPE_VD,   IOC_NR_VD_DISABLE_DEV, HI_S32)
#define VD_SET_DEV_ID						 _IOW(IOC_TYPE_VD,   IOC_NR_VD_SET_DEV_ID, HI_S32)
#define VD_SET_DEV_ATTR                		 _IOW(IOC_TYPE_VD,   IOC_NR_VD_SET_DEV_ATTR, VD_PUB_ATTR_IOC_S)
#define VD_GET_DEV_ATTR                		 _IOWR(IOC_TYPE_VD,   IOC_NR_VD_GET_DEV_ATTR, VD_PUB_ATTR_IOC_S)
#define VD_SET_GRAPHIC_BIND                  _IOW(IOC_TYPE_VD,   IOC_NR_VD_SET_GRAPHIC_BIND, VD_BIND_S)
#define VD_GET_GRAPHIC_BIND                  _IOWR(IOC_TYPE_VD,   IOC_NR_VD_GET_GRAPHIC_BIND, VD_BIND_S)


typedef struct hiVD_BIND_S
{
    HI_S32  s32GraphicId;          /* which graphic layer to bind (0:G0,1:G1,2:G2,3:G3,4:HC)
									  note: jubt G0 and HC layer can be to bind*/
    VO_DEV  DevId;                 /* which device bind to(0:HD,1:AD,2:SD)
    								  note: just HD and AD Dev can be binded */
} VD_BIND_S;

typedef struct hiVD_PUB_ATTR_IOC_S
{
    VO_DEV  DevId;					/*vo dev id. range:[0~2]*/
    VO_PUB_ATTR_S stPubAttr;        /*pub attr*/
} VD_PUB_ATTR_IOC_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MKP_VD_H__ */

