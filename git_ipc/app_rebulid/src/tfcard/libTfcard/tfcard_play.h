#ifndef _TFCARD_RECORD_H_
#define _TFCARD_RECORD_H_

#ifdef __cplusplus
extern "C"{
#endif

#define TFPLAY_MODE_FILE	(0x1)
#define TFPLAY_MODE_TIME	(0x2)
#define TFCARD_TS_PATH "/media/tf/video/00"

NK_Boolean TFPLAY_HasFile(NK_PChar path);
HTFPLAY TFPLAY_Start(NK_Int nType, NK_PVoid lParam);
NK_Int TFPLAY_Stop(HTFPLAY hPlayBack);
NK_Int TFPLAY_ReadFrame(HTFPLAY hPlayBack,pstTFMEDIAHEAD pFrameHead, NK_PVoid* pFrameData);
NK_Int TFPLAY_Search(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB fFunch, NK_Int maxLimitCnt);
NK_Int TFPLAY_SearchEx(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB fFunch);

#ifdef __cplusplus
 }
#endif

#endif

