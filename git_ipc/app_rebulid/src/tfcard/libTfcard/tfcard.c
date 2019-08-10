
#include "tfcard_export.h"

NK_Int TFSDK_CARD_Init(pstTFPARAM ptfParam,pstTFSDKEVENT pstEvent)
{
	if(NK_Nil == ptfParam || NK_Nil == pstEvent)
	{
		return NK_FALSE;
	}
	return TFCARDOPT_Init(ptfParam, pstEvent);
}

NK_Int TFSDK_CARD_Exit()
{
	return TFCARDOPT_Exit();
}
NK_Int TFSDK_CARD_SetParam(enTFCARDOPT enType, NK_PVoid lParam, NK_PVoid wParam)
{
	switch(enType)
	{
		case EN_TFCARD_OPT_MOUNT:
			{
				return TFCARDOPT_Mounted();
			}
			break;
		case EN_TFCARD_OPT_UMOUNT:
			{
				return TFCARDOPT_Umounted();
			}
			break;
		case EN_TFCARD_OPT_FORMAT:
			{
				return TFCARDOPT_SetFormat(NK_TRUE);
			}
			break;
		default:
			return NK_FALSE;
	}
	return NK_TRUE;

}

NK_Size TFSDK_CARD_GetCapacity()
{
    return TFCARDOPT_GetCapacity();
}
NK_Size TFSDK_CARD_GetFreeSpace()
{
    return TFCARDOPT_GetFreespace();
}
NK_Boolean TFSDK_CARD_IsExist()
{
    return TFCARDOPT_Exist();
}
NK_Boolean TFSDK_CARD_IsMounted()
{
    return TFCARDOPT_IsMounted();
}
NK_Int TFSDK_CARD_Format()
{
    return TFCARDOPT_SetFormat(NK_TRUE);
}
NK_Int TFSDK_CARD_Repair()
{
    return TFCARDOPT_RepairTF();
}

enTFCORDSTATUS TFSDK_CARD_GetStatus()
{
	char tfcardStatus[32] = { 0 };
   	return TFCARDOPT_GetTfcardStatus(tfcardStatus);
}
enTFCORDSTATUS TFSDK_CARD_GetFormatStatus()
{
    switch(TFCARDOPT_GetFormatStatus())
	{
		case TFCARD_FORMAT_READY:
		case TFCARD_FORMAT_RUNING:
			return EN_TFCORD_STATUS_FORMATTING;
		case TFCARD_FORMAT_FINISH:
			return EN_TFCORD_STATUS_FORMATED;
		default:
			return EN_TFCORD_STATUS_NO_FORMAT;
	}
}

NK_Int TFSDK_CARD_GetTsRecPathOfHex(NK_PChar pReFile)
{
    return TFPLAY_get_ts_recpath_of_hex(TFCARD_TS_PATH, pReFile);
}


NK_PVoid TFSDK_RECORD_Start(pstTFRECPARAM pstRecParam, pstTFRECEVENT pstRecEvent)
{
	if(NK_Nil == pstRecParam || NK_Nil == pstRecEvent)
	{
		return NK_Nil;
	}
	return TFRECORD_Create(pstRecParam, pstRecEvent);
}
NK_Int TFSDK_RECORD_Stop(NK_PVoid hRecord)
{
	return TFRECORD_Destroy(hRecord);
}

NK_Int TFSDK_RECORD_SetParam(NK_PVoid hRecord, enTFRECOPT enOpt, NK_PVoid lParam, NK_PVoid wParam)
{
	return TFRECORD_SetParam(hRecord, enOpt, lParam);
}

NK_Int TFSDK_RECORD_RecordFile_Get(HTFRECORD hRecord,NK_PChar recordFile)
{
	return TFRECORD_RecordFile_Get(hRecord, recordFile);
}



NK_PVoid TFSDK_PLAY_Open(NK_Int bPlayFile, NK_PVoid lParam)
{
	if(NK_TRUE == bPlayFile) ///文件方式打开
	{
		NK_PChar pStrFile = (NK_PChar)lParam;
		if(NK_Nil == pStrFile)
		{
			return NK_Nil;
		}
		if(NK_FALSE == TFPLAY_HasFile(pStrFile))////判断文件是否存在
		{
			return NK_Nil;
		}
		return TFPLAY_Start(TFPLAY_MODE_FILE, pStrFile);
	}

	pstTFPLAYSEARCH pstPlayParam = (pstTFPLAYSEARCH)lParam;
	if(NK_Nil == pstPlayParam)
	{
		return NK_Nil;
	}
	return TFPLAY_Start(TFPLAY_MODE_TIME, pstPlayParam);
}

NK_Int TFSDK_PLAY_Close(NK_PVoid hPlay)
{
	return TFPLAY_Stop(hPlay);
}

NK_Int TFSDK_PLAY_Read(NK_PVoid hPlay, NK_Int nMode, pstTFMEDIAHEAD pFrameHead, NK_PVoid* pFrameData)
{
	return TFPLAY_ReadFrame(hPlay,pFrameHead, pFrameData);
}
NK_Int TFSDK_PLAY_Search(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB pfSearchCB, NK_Int maxLimitCnt)
{
	return TFPLAY_Search(pstPlaySearch, pfSearchCB, maxLimitCnt);
}
NK_Int TFSDK_PLAY_SearchEx(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB pfSearchCB)
{
	return TFPLAY_SearchEx(pstPlaySearch, pfSearchCB);
}

NK_Int TFSDK_PLAY_Search_To_Struct(pstTFPLAYSEARCH pstPlaySearch)
{
	return TFSDK_PLAY_CreateHistory(pstPlaySearch);
}


#define PLAY_HISTORY_MAX (2000)
typedef struct tagPLAYHISTORY_
{
	NK_PVoid pUserCtx;////用户上下文
	NK_Int nListTotal;///列表最大值
	NK_Int nReadIndex; ////读到列表的那个位置
	NK_PVoid pListFile[PLAY_HISTORY_MAX]; ////指向历史列表
}stPLAYHISTORY, *pstPLAYHISTORY;

NK_Int HistorySearchFile(NK_PVoid pContext, NK_Int bFileType, NK_Int nNumber, NK_Int nTotal, NK_PVoid lParam)
 {
 	pstTFPLAYFILE pstFileAttr = (pstTFPLAYFILE)lParam;
	pstPLAYHISTORY pstHistory = (pstPLAYHISTORY)pContext;
	if(NK_Nil == pstHistory || NK_Nil == pstFileAttr)
	{
		return NK_FALSE;
	}
	pstTFPLAYFILE pstFile = (pstTFPLAYFILE)malloc(sizeof(stTFPLAYFILE));
	if(NK_Nil == pstFile)
	{
		return NK_FALSE;
	}
	memset(pstFile,0,sizeof(stTFPLAYFILE));
	memcpy(pstFile,pstFileAttr,sizeof(stTFPLAYFILE));
	if(pstHistory->nListTotal >= PLAY_HISTORY_MAX)
	{
		free(pstFile);
		return NK_FALSE;
	}
	if(NK_Nil != pstHistory->pListFile[pstHistory->nListTotal])
	{
		free(pstHistory->pListFile[pstHistory->nListTotal]);
		pstHistory->pListFile[pstHistory->nListTotal] = NK_Nil;
	}
	pstHistory->pListFile[pstHistory->nListTotal] = pstFile;
	
	//anyka_print("History[%d] nChn=%d,nNumber=%d,nFileSize=%d,strFile=%s \n", pstHistory->nListTotal, pstFile->nChn,nNumber,pstFile->nFileSize,pstFile->strFile);
	pstHistory->nListTotal ++;
	//anyka_print("History nChn=%d,nNumber=%d,nFileSize=%d,strFile=%s \n", pstFileAttr->nChn,nNumber,pstFileAttr->nFileSize,pstFileAttr->strFile);
	return NK_TRUE;
 }


NK_PVoid TFSDK_PLAY_CreateHistory(pstTFPLAYSEARCH pstPlaySearch)
{
	NK_Int i = 0;
	pstPLAYHISTORY pstHistory = (pstPLAYHISTORY) malloc(sizeof(stPLAYHISTORY));
	if(NK_Nil == pstPlaySearch || NK_Nil == pstHistory)
	{
		goto ERROR;
	}
	memset(pstHistory, 0 , sizeof(stPLAYHISTORY));
	pstHistory->nListTotal = 0;
	pstPlaySearch->pUserCtx = pstHistory;
	if(NK_FALSE == TFPLAY_Search(pstPlaySearch, HistorySearchFile, 10000))
	{
		goto ERROR;
	}

	return (NK_PVoid)pstHistory;

ERROR:

	if(NK_Nil != pstHistory)
	{
		free(pstHistory);
		pstHistory = NK_Nil;
	}
	return NK_Nil;
}
NK_Int TFSDK_PLAY_FreeHistory(NK_PVoid hHistory)
{
	pstPLAYHISTORY pstHistory = (pstPLAYHISTORY)hHistory;
	if(NK_Nil == pstHistory)
	{
		return NK_FALSE;
	}
	NK_Int i = 0;
	for(i=0;i<PLAY_HISTORY_MAX;i++)
	{
		if(NK_Nil != pstHistory->pListFile[i])
		{
			free(pstHistory->pListFile[i]);
			pstHistory->pListFile[i] = NK_Nil;
		}
	}
	free(pstHistory);
	return NK_TRUE;
}
NK_Int TFSDK_PLAY_GetHistoryLen(NK_PVoid hHistory)
{
	pstPLAYHISTORY pstHistory = (pstPLAYHISTORY)hHistory;
	if(NK_Nil == pstHistory)
	{
		return NK_FALSE;
	}
	return pstHistory->nListTotal;
}
NK_Int TFSDK_PLAY_GetHistoryFile(NK_PVoid hHistory, NK_Int nIndex, pstTFPLAYFILE *pstFile)
{
	pstPLAYHISTORY pstHistory = (pstPLAYHISTORY)hHistory;
	if(NK_Nil == pstHistory)
	{
		return NK_FALSE;
	}
	if(NK_Nil == pstHistory->pListFile[nIndex])
	{
		return NK_FALSE;
	}
	*pstFile = (pstTFPLAYFILE)pstHistory->pListFile[nIndex];
	return NK_TRUE;
}



