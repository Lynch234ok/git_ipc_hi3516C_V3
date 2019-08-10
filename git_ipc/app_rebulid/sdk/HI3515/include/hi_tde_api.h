/*****************************************************************************
*             Copyright 2006 - 2050, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hi_api_tde.h
* Description:TDE2 API define
*
* History:
* Version   Date          Author        DefectNum       Description
*
*****************************************************************************/

#ifndef _HI_API_TDE2_H_
#define _HI_API_TDE2_H_

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#include "hi_type.h"
#include "hi_tde_type.h"

#define HI_TDE_Open HI_TDE2_Open
#define HI_TDE_Close HI_TDE2_Close
#define HI_TDE_BeginJob HI_TDE2_BeginJob

/****************************************************************************/
/*                             TDE2 API define                               */
/****************************************************************************/

/*****************************************************************************
* Function:      HI_TDE2_Open
* Description:   ��TDE2�豸
* Input:         ��
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_Open(HI_VOID);

/*****************************************************************************
* Function:      HI_TDE2_Close
* Description:   �ر�TDE2�豸
* Input:         ��
* Output:        ��
* Return:        ��
* Others:        ��
*****************************************************************************/
HI_VOID     HI_TDE2_Close(HI_VOID);

/*****************************************************************************
* Function:      HI_TDE2_BeginJob
* Description:   ���TDE2 ������
* Input:         ��
* Output:        ��
* Return:        tde��� / ������
* Others:        ��
*****************************************************************************/
TDE_HANDLE  HI_TDE2_BeginJob(HI_VOID);

/*****************************************************************************
* Function:      HI_TDE2_EndJob
* Description:   �ύTDE2����
* Input:         s32Handle: ������
*                bSync:�Ƿ�ͬ��
*                bBlock: �Ƿ�����
*                u32TimeOut: ��ʱʱ��ֵ(10msΪ��λ)
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_EndJob(TDE_HANDLE s32Handle, HI_BOOL bSync, HI_BOOL bBlock, HI_U32 u32TimeOut);

/*****************************************************************************
* Function:      HI_TDE2_CancelJob
* Description:   ɾ��������TDE2����,ֻ����EndJob֮ǰ���òŻ���Ч
* Input:         s32Handle: ������
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_CancelJob(TDE_HANDLE s32Handle);

/*****************************************************************************
* Function:      HI_TDE2_IsCompleted
* Description:   �ȴ��ύ��TDE2�����Ƿ����
* Input:         s32Handle: ������
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_WaitForDone(TDE_HANDLE s32Handle);

/*****************************************************************************
* Function:      HI_TDE2_QuickCopy
* Description:   ���ٴ�Դ���Ƶ�Ŀ��λͼ��û���κι��ܲ�����Դ��Ŀ�Ĳ�������ߴ�һ�������ظ�ʽһ��
* Input:         s32Handle: ������
*                pSrc: Դλͼ��Ϣ�ṹ
*                pstSrcRect: Դλͼ��������
*                pDst: Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect: Ŀ��λͼ��������
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_QuickCopy(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                              TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect);

/*****************************************************************************
* Function:      HI_TDE2_QuickFill
* Description:   ���ٽ��̶�ֵ��䵽Ŀ��λͼ��
* Input:         s32Handle: ������
*                pDst: Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect: Ŀ��λͼ��������
*                u32FillData: ���ֵ��Ϣ�����ظ�ʽҪ��Ŀ��λͼ����һ��
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_QuickFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect,
                              HI_U32 u32FillData);

/*****************************************************************************
* Function:      HI_TDE2_QuickResize
* Description:   ��Դλͼ�Ĵ�С����ΪĿ��λͼָ���Ĵ�С��Դ��Ŀ�����Ϊͬһλͼ
* Input:         s32Handle: ������
*                pSrc: Դλͼ��Ϣ�ṹ
*                pstSrcRect: Դλͼ��������
*                pDst: Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect: Ŀ��λͼ��������
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_QuickResize(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                                TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect);

/*****************************************************************************
* Function:      HI_TDE2_QuickFlicker
* Description:   ��Դλͼ���п���˸�������Ŀ��λͼ��Դ��Ŀ�����Ϊͬһλͼ
* Input:         s32Handle: ������
*                pSrc: Դλͼ��Ϣ�ṹ
*                pstSrcRect: Դλͼ��������
*                pDst: Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect: Ŀ��λͼ��������
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_QuickDeflicker(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                                   TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect);

/*****************************************************************************
* Function:      HI_TDE2_Blit
* Description:   ��pstBackGround��pstForeGround��λͼ�������㽫��������pDst�У�����������pOpt
* Input:         s32Handle: ������
*                pstBackGround: ����λͼ��Ϣ�ṹ
*                pstBackGroundRect: ����λͼ��������
*                pstForeGround: ǰ��λͼ��Ϣ�ṹ
*                pstForeGroundRect: ǰ��λͼ��������
*                pstDst:  Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect:  Ŀ��λͼ��������
*                pOpt:  ����������ýṹ
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_Bitblit(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                            TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect, TDE2_SURFACE_S* pstDst,
                            TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt);

/*****************************************************************************
* Function:      HI_TDE2_SolidDraw
* Description:   ��src1��src2��λͼ�������㽫��������pDst�У�����������pOpt
*                ���srcλͼΪ����ʽ��ֻ��֧�ֵ�Դ����,��ֻ����pSrc1��ֻ����pSrc2
* Input:         s32Handle: ������
*                pstForeGround: ǰ��Դλͼ��Ϣ�ṹ
*                pstForeGroundRect: ǰ��Դλͼ��������
*                pstDst: Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect: Ŀ��λͼ��������
*                pstFillColor:  �������ֵ
*                pstOpt:  ����������ýṹ
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_SolidDraw(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                              TDE2_SURFACE_S *pstDst,
                              TDE2_RECT_S  *pstDstRect, TDE2_FILLCOLOR_S *pstFillColor,
                              TDE2_OPT_S *pstOpt);

/*****************************************************************************
* Function:      HI_TDE2_MbBlit
* Description:   ��鴦��ӿ�
* Input:         s32Handle: ������
*                pstMB:  ���λͼ��Ϣ�ṹ
*                pstDst: Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect:  Ŀ��λͼ��������
*                pstMbOpt:  ����������ýṹ
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_MbBlit(TDE_HANDLE s32Handle, TDE2_MB_S* pstMB, TDE2_RECT_S  *pstMbRect, TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                           TDE2_MBOPT_S* pstMbOpt);

/*****************************************************************************
* Function:      HI_TDE2_BitmapMaskRop
* Description:   ���Ƚ�Դ2��Maskλͼ����RopMask, Ȼ����Դ1���м�λͼRopMask��
*                ����������Ŀ��λͼ�С�
* Input:         s32Handle: ������
*                pstBackGround: ����λͼ��Ϣ�ṹ
*                pstBackGroundRect: ����λͼ��������
*                pstForeGround: ǰ��λͼ��Ϣ�ṹ
*                pstForeGroundRect: ǰ��λͼ��������
*                pstMask: �����������λͼ��Ϣ�ṹ
*                pstMaskRect: �����������λͼ��������
*                pstDst:  Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect:  Ŀ��λͼ��������
*                enRopCode: ǰ���ͱ��������ROP��
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_BitmapMaskRop(TDE_HANDLE s32Handle, 
                                  TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                                  TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                                  TDE2_SURFACE_S* pstMask, TDE2_RECT_S  *pstMaskRect, 
                                  TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                                  TDE2_ROP_CODE_E enRopCode_Color, TDE2_ROP_CODE_E enRopCode_Alpha);

/*****************************************************************************
* Function:      HI_TDE2_BitmapMaskBlend
* Description:   ���Ƚ�ǰ����Maskλͼ����BlendMask�õ��м�λͼ, Ȼ�󱳾����м�λͼBlend��
*                ����������Ŀ��λͼ�С�
* Input:         s32Handle: ������
*                pstBackGround: ����λͼ��Ϣ�ṹ
*                pstBackGroundRect: ����λͼ��������
*                pstForeGround: ǰ��λͼ��Ϣ�ṹ
*                pstForeGroundRect: ǰ��λͼ��������
*                pstMask: �����������λͼ��Ϣ�ṹ
*                pstMaskRect: �����������λͼ��������
*                pstDst:  Ŀ��λͼ��Ϣ�ṹ
*                pstDstRect:  Ŀ��λͼ��������
*                u8Alpha:  ��������Alpha��ֵ
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32      HI_TDE2_BitmapMaskBlend(TDE_HANDLE s32Handle, 
                                    TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                                    TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                                    TDE2_SURFACE_S* pstMask, TDE2_RECT_S  *pstMaskRect,
                                    TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                                    HI_U8 u8Alpha, TDE2_ALUCMD_E enBlendMode);

/*****************************************************************************
* Function:      HI_TDE2_Osd2Mb
* Description:   ��ǰ���Ĺ�դλͼ��ָ��������ӵ��������λͼ��ָ�������ϣ������
*                ���Ӻ�ĺ��λͼ���ýӿ�Ҳ����Ϊ��դλͼת��Ϊ���λͼʹ�á�
* Input:         s32Handle: ������
*                pstBackGround: ��鱳��λͼ��Ϣ�ṹ
*                pstBackGroundRect: ��鱳��λͼ��������
*                pstForeGround: ǰ��λͼ��Ϣ�ṹ
*                pstForeGroundRect: ǰ��λͼ��������
*                pstMbOut: ���Ŀ��λͼ��Ϣ�ṹ
*                pstMbOutRect : ���Ŀ��λͼ��������
*                pOpt:  ����������ýṹ
* Output:        ��
* Return:        �ɹ� / ������
* Others:        ��
*****************************************************************************/
HI_S32  HI_TDE2_Osd2Mb(TDE_HANDLE s32Handle, TDE2_MB_S* pstBackGround, TDE2_RECT_S* pstBackGroundRect, 
                                TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S* pstForeGroundRect, 
                                TDE2_MB_S* pstMbOut, TDE2_RECT_S* pstMbOutRect, TDE2_OPT_S* pstOpt);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#endif  /* _HI_API_TDE2_H_ */
