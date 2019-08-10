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
* Description:   打开TDE2设备
* Input:         无
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_Open(HI_VOID);

/*****************************************************************************
* Function:      HI_TDE2_Close
* Description:   关闭TDE2设备
* Input:         无
* Output:        无
* Return:        无
* Others:        无
*****************************************************************************/
HI_VOID     HI_TDE2_Close(HI_VOID);

/*****************************************************************************
* Function:      HI_TDE2_BeginJob
* Description:   获得TDE2 任务句柄
* Input:         无
* Output:        无
* Return:        tde句柄 / 错误码
* Others:        无
*****************************************************************************/
TDE_HANDLE  HI_TDE2_BeginJob(HI_VOID);

/*****************************************************************************
* Function:      HI_TDE2_EndJob
* Description:   提交TDE2任务
* Input:         s32Handle: 任务句柄
*                bSync:是否同步
*                bBlock: 是否阻塞
*                u32TimeOut: 超时时间值(10ms为单位)
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_EndJob(TDE_HANDLE s32Handle, HI_BOOL bSync, HI_BOOL bBlock, HI_U32 u32TimeOut);

/*****************************************************************************
* Function:      HI_TDE2_CancelJob
* Description:   删除创建的TDE2任务,只能在EndJob之前调用才会有效
* Input:         s32Handle: 任务句柄
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_CancelJob(TDE_HANDLE s32Handle);

/*****************************************************************************
* Function:      HI_TDE2_IsCompleted
* Description:   等待提交的TDE2操作是否完成
* Input:         s32Handle: 任务句柄
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_WaitForDone(TDE_HANDLE s32Handle);

/*****************************************************************************
* Function:      HI_TDE2_QuickCopy
* Description:   快速从源搬移到目的位图，没有任何功能操作，源和目的操作区域尺寸一样，象素格式一样
* Input:         s32Handle: 任务句柄
*                pSrc: 源位图信息结构
*                pstSrcRect: 源位图操作区域
*                pDst: 目标位图信息结构
*                pstDstRect: 目标位图操作区域
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_QuickCopy(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                              TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect);

/*****************************************************************************
* Function:      HI_TDE2_QuickFill
* Description:   快速将固定值填充到目标位图，
* Input:         s32Handle: 任务句柄
*                pDst: 目标位图信息结构
*                pstDstRect: 目标位图操作区域
*                u32FillData: 填充值信息，象素格式要和目标位图保持一致
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_QuickFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect,
                              HI_U32 u32FillData);

/*****************************************************************************
* Function:      HI_TDE2_QuickResize
* Description:   将源位图的大小缩放为目标位图指定的大小，源和目标可以为同一位图
* Input:         s32Handle: 任务句柄
*                pSrc: 源位图信息结构
*                pstSrcRect: 源位图操作区域
*                pDst: 目标位图信息结构
*                pstDstRect: 目标位图操作区域
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_QuickResize(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                                TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect);

/*****************************************************************************
* Function:      HI_TDE2_QuickFlicker
* Description:   将源位图进行抗闪烁，输出到目标位图，源和目标可以为同一位图
* Input:         s32Handle: 任务句柄
*                pSrc: 源位图信息结构
*                pstSrcRect: 源位图操作区域
*                pDst: 目标位图信息结构
*                pstDstRect: 目标位图操作区域
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_QuickDeflicker(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                                   TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect);

/*****************************************************************************
* Function:      HI_TDE2_Blit
* Description:   将pstBackGround与pstForeGround的位图进行运算将结果输出到pDst中，运算设置在pOpt
* Input:         s32Handle: 任务句柄
*                pstBackGround: 背景位图信息结构
*                pstBackGroundRect: 背景位图操作区域
*                pstForeGround: 前景位图信息结构
*                pstForeGroundRect: 前景位图操作区域
*                pstDst:  目标位图信息结构
*                pstDstRect:  目标位图操作区域
*                pOpt:  运算参数设置结构
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_Bitblit(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                            TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect, TDE2_SURFACE_S* pstDst,
                            TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt);

/*****************************************************************************
* Function:      HI_TDE2_SolidDraw
* Description:   将src1与src2的位图进行运算将结果输出到pDst中，运算设置在pOpt
*                如果src位图为宏块格式则只能支持单源操作,即只设置pSrc1或只设置pSrc2
* Input:         s32Handle: 任务句柄
*                pstForeGround: 前景源位图信息结构
*                pstForeGroundRect: 前景源位图操作区域
*                pstDst: 目的位图信息结构
*                pstDstRect: 目的位图操作区域
*                pstFillColor:  填充数据值
*                pstOpt:  运算参数设置结构
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_SolidDraw(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                              TDE2_SURFACE_S *pstDst,
                              TDE2_RECT_S  *pstDstRect, TDE2_FILLCOLOR_S *pstFillColor,
                              TDE2_OPT_S *pstOpt);

/*****************************************************************************
* Function:      HI_TDE2_MbBlit
* Description:   宏块处理接口
* Input:         s32Handle: 任务句柄
*                pstMB:  宏块位图信息结构
*                pstDst: 目的位图信息结构
*                pstDstRect:  目标位图操作区域
*                pstMbOpt:  运算参数设置结构
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_MbBlit(TDE_HANDLE s32Handle, TDE2_MB_S* pstMB, TDE2_RECT_S  *pstMbRect, TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                           TDE2_MBOPT_S* pstMbOpt);

/*****************************************************************************
* Function:      HI_TDE2_BitmapMaskRop
* Description:   首先将源2与Mask位图进行RopMask, 然后与源1与中间位图RopMask，
*                将结果输出到目的位图中。
* Input:         s32Handle: 任务句柄
*                pstBackGround: 背景位图信息结构
*                pstBackGroundRect: 背景位图操作区域
*                pstForeGround: 前景位图信息结构
*                pstForeGroundRect: 前景位图操作区域
*                pstMask: 作掩码运算的位图信息结构
*                pstMaskRect: 作掩码运算的位图操作区域
*                pstDst:  目标位图信息结构
*                pstDstRect:  目标位图操作区域
*                enRopCode: 前景和背景运算的ROP码
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_BitmapMaskRop(TDE_HANDLE s32Handle, 
                                  TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                                  TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                                  TDE2_SURFACE_S* pstMask, TDE2_RECT_S  *pstMaskRect, 
                                  TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                                  TDE2_ROP_CODE_E enRopCode_Color, TDE2_ROP_CODE_E enRopCode_Alpha);

/*****************************************************************************
* Function:      HI_TDE2_BitmapMaskBlend
* Description:   首先将前景与Mask位图进行BlendMask得到中间位图, 然后背景与中间位图Blend，
*                将结果输出到目的位图中。
* Input:         s32Handle: 任务句柄
*                pstBackGround: 背景位图信息结构
*                pstBackGroundRect: 背景位图操作区域
*                pstForeGround: 前景位图信息结构
*                pstForeGroundRect: 前景位图操作区域
*                pstMask: 作掩码运算的位图信息结构
*                pstMaskRect: 作掩码运算的位图操作区域
*                pstDst:  目标位图信息结构
*                pstDstRect:  目标位图操作区域
*                u8Alpha:  参与运算Alpha数值
* Output:        无
* Return:        成功 / 错误码
* Others:        无
*****************************************************************************/
HI_S32      HI_TDE2_BitmapMaskBlend(TDE_HANDLE s32Handle, 
                                    TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                                    TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                                    TDE2_SURFACE_S* pstMask, TDE2_RECT_S  *pstMaskRect,
                                    TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                                    HI_U8 u8Alpha, TDE2_ALUCMD_E enBlendMode);

/*****************************************************************************
* Function:      HI_TDE2_Osd2Mb
* Description:   将前景的光栅位图的指定区域叠加到背景宏块位图的指定区域上，并输出
*                叠加后的宏块位图。该接口也可作为光栅位图转换为宏块位图使用。
* Input:         s32Handle: 任务句柄
*                pstBackGround: 宏块背景位图信息结构
*                pstBackGroundRect: 宏块背景位图操作区域
*                pstForeGround: 前景位图信息结构
*                pstForeGroundRect: 前景位图操作区域
*                pstMbOut: 宏块目的位图信息结构
*                pstMbOutRect : 宏块目的位图操作区域
*                pOpt:  运算参数设置结构
* Output:        无
* Return:        成功 / 错误码
* Others:        无
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
