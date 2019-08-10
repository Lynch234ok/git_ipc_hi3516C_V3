/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name             :   hi_comm_dwm.h
  Description           :   

  Function List         :
  History               :
  1.Date                :   2007/11/24
    Author              :   z53517
    Modification        :

  2.Date                :   2008/10/31
    Author              :   z44949
    Modification        :   Translate the chinese comment

******************************************************************************/

#ifndef __HI_COMM_DWM_H__
#define __HI_COMM_DWM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define  DWM_KEY_LEN          8      /* The max length of secret key */
#define  DWM_CHAR_LEN        16      /* The max lenght of DWM        */
#define  DWM_MB_MAX_NUM      128     /* The max macro block numbers of DWM */

typedef enum hiDWM_DENSITY_E
{
    DWM_DENSITY_LOW = 0, /* DWM's density is low */
    DWM_DENSITY_MIDDLE,  /* DWM's density is middle */
    DWM_DENSITY_HIGH,    /* DWM's density is high */
    DWM_DENSITY_BUTT
}DWM_DENSITY_E;

typedef struct hiVENC_WM_ATTR_S
{
    HI_U8         au8Key[DWM_KEY_LEN];       /* The key string for DWM */
    HI_U8         au8User[DWM_CHAR_LEN];     /* The strings used for DWM */
    //HI_U16        au16Mbn[DWM_MB_MAX_NUM]; /* The index of macro */
    //DWM_DENSITY_E enDensity;               /* The density of DWM */
}VENC_WM_ATTR_S;

typedef struct hiVDEC_WM_ATTR_S
{
    HI_U8 		u8Key[DWM_KEY_LEN];         /* The key string for DWM */
    //HI_U16  	au16Mbn[DWM_MB_MAX_NUM];    /* The index of macro */
    //DWM_DENSITY_E enDensity;              /* The density of DWM */
}VDEC_WM_ATTR_S;

typedef struct hiVDEC_WATERNARK_S
{
    HI_U8*    		pu8Addr;  		/* The address of DWM data */
    HI_U32  		u32Len;  		/* The lenght of DWM data */
    HI_BOOL         bValid;
}VDEC_WATERMARK_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif

#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifdef __HI_COMM_DWM_H__ */
