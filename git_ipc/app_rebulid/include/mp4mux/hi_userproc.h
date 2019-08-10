/******************************************************************************

  Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : hi_userproc.h
  Version       : Initial Draft
  Author        :
  Created       : 2017/03/21
  Description   : Support user proc function.
  History       :
  1.Date        : 2017/03/21
    Modification: Created file


******************************************************************************/

#ifndef __HI_USERPROC_H__
#define __HI_USERPROC_H__

/******************************* Include Files *******************************/

/* add include here */
#include "hi_type.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
#define HI_UPROC_ERRNO_BASE 0x2B00
/** Null pointer*/
#define HI_UPROC_ERR_NULL_PTR            (HI_UPROC_ERRNO_BASE + 1)

#define HI_UPROC_MW_DIR_NAME            "middleware"

/*************************** Structure Definition ****************************/
/********************** Global Variable declaration **************************/

/** Module ID flags */
typedef enum hiUPROC_MOD_ID_E
{
    /**< user definition. */ /**< CNcomment: Ϊ�����������Զ����� */
    HI_UPROC_ID_DTCF   = 0xC0,
    HI_UPROC_ID_RECORD,
    HI_UPROC_ID_MUXTER,
    HI_UPROC_ID_DEMUXTER,
    HI_UPROC_ID_DCF,

    HI_UPROC_ID_BUTT        = 0xFF
} HI_UPROC_MOD_ID_E;


/** Defines user mode proc show buffer */
/**CNcomment: �û�̬PROC buffer���� */
typedef struct hiUPROC_SHOW_BUFFER_S
{
    HI_U8* pu8Buf;                  /**<Buffer address*/  /**<CNcomment: Buffer��ַ */
    HI_U32 u32Size;                 /**<Buffer size*/     /**<CNcomment: Buffer��С */
    HI_U32 u32Offset;               /**<Offset*/          /**<CNcomment: ��ӡƫ�Ƶ�ַ */
}HI_UPROC_SHOW_BUFFER_S;

/** UProc show function */
/**CNcomment: UProc��Ϣ��ʾ�ص����� */
typedef HI_S32 (* HI_UPROC_SHOW_FN)(HI_UPROC_SHOW_BUFFER_S * pstBuf, HI_VOID *pPrivData);

/** UProc command function */
/**CNcomment: UProc���ƻص����� */
typedef HI_S32 (* HI_UPROC_CMD_FN)(HI_UPROC_SHOW_BUFFER_S * pstBuf, HI_U32 u32Argc, HI_U8 *pu8Argv[], HI_VOID *pPrivData);

/** Defines user mode proc entry */
/**CNcomment: �û�̬PROC��ڶ��� */
typedef struct hiUPROC_ENTRY_S
{
    HI_CHAR *pszEntryName;          /**<Entry name*/            /**<CNcomment: ����ļ��� */
    HI_CHAR *pszDirectory;          /**<Directory name. If null, the entry will be added to /proc/hisi directory*/
                                    /**<CNcomment: Ŀ¼�������Ϊ�գ���������/proc/hisiĿ¼�� */
    HI_UPROC_SHOW_FN pfnShowProc;    /**<UProc show function*/    /**<CNcomment: UProc��Ϣ��ʾ�ص����� */
    HI_UPROC_CMD_FN pfnCmdProc;      /**<UProc command function*/ /**<CNcomment: UProc���ƻص����� */
    HI_VOID *pPrivData;             /**<Private data*/          /**<CNcomment: Buffer��ַ */
}HI_UPROC_ENTRY_S;

/******************************* API declaration *****************************/
HI_VOID HI_UPROC_Enable(HI_BOOL bEnable);
HI_S32 HI_UPROC_Init(HI_VOID);
HI_S32 HI_UPROC_DeInit(HI_VOID);
HI_S32 HI_UPROC_AddEntry(HI_U32 u32ModuleID, const HI_UPROC_ENTRY_S* pstEntry);
HI_S32 HI_UPROC_RemoveEntry(HI_U32 u32ModuleID, const HI_UPROC_ENTRY_S* pstEntry);
HI_S32 HI_UPROC_Printf(HI_UPROC_SHOW_BUFFER_S *pstBuf, const HI_CHAR *pFmt, ...);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __HI_USERPROC_H__ */

