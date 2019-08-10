#ifndef _IMP_AVD_API_H_
#define _IMP_AVD_API_H_

#include "imp_algo_type.h"
#include "imp_avd_para.h"
#ifdef __cplusplus
extern "C"
{
#endif
/**
* \defgroup AVD interface
* AVD interface
* @ingroup interface
* @author Impower-tech
* @version 5.1
* @data 2009-2013
*/


/*@{*/
/**
* Create AVD handle
* @param s32Width 		width
* @param s32Height  	height
* @return IMP_HANDLE  handle of algorithm
*/
IMP_EXPORTS IMP_HANDLE IMP_AVD_Create(IMP_S32 s32Width, IMP_S32 s32Height);




/**
* Release AVD handle
* @param hModule      handle of algorithm
*/
IMP_EXPORTS IMP_VOID IMP_AVD_Release(IMP_HANDLE hModule);




/**
* Configure AVD parameters
* @param hModule 				handle of algorithm
* @param pstAvdParaSrc 	AVD parameters 
*/
IMP_BOOL IMP_AVD_ConfigParameter(IMP_HANDLE hModule, IMP_AVD_PARA_S *pstAvdParaSrc);





/**
* Pocess AVD algorithm
* @param hModule      handle of algorithm
* @param pstImage     current frame
*/
IMP_BOOL IMP_AVD_Process(IMP_HANDLE hModule, IMAGE3_S *pstImage);




/**
* Get AVD result after process
* @param hModule 			handle of algorithm
* @param pstAvdResult result after process
*/
IMP_EXPORTS IMP_VOID IMP_AVD_GetResult(IMP_HANDLE hModule, AVD_RESULT_S *pstAvdResult);

/*@}*/




/*@{*/
/**
* Create YUV420 image
* @param pstImg 		pointer of image 
* @param s32W 			width of image
* @param s32H 			height of image 
* @param pMmgr 			pointer of memory manager
*/
IMP_EXPORTS IMP_VOID IMP_Image3Create(IMAGE3_S * pstImg, IMP_S32 s32W, IMP_S32 s32H, IMAGE_FORMAT_E enFormat, IMP_VOID * pMmgr);




/**
* Release YUV420 image
* @param pstImg 	pointer of image
* @param pMmgr    pointer of memory manager
*/
IMP_EXPORTS IMP_VOID IMP_Image3Destroy(IMAGE3_S * pstImg, IMP_VOID * pMmgr);




/**
* Clean content of YUV420 image
* @param pstImg  pointer of image
*/
IMP_EXPORTS IMP_VOID IMP_Image3Clear(IMAGE3_S * pstImg);




/**
* copy content of YUV420 image
* @param pstSrc    pointer of source image
* @param pstDst 	 pointer of destination image
*/
IMP_EXPORTS IMP_VOID IMP_Image3Clone(IMAGE3_S * pstSrc, IMAGE3_S * pstDst);
/*@}*/



#ifdef __cplusplus
}
#endif

#endif /*_IMP_AVD_API_H_*/

