
#include "tfcard_export.h"
#include "tfcard_play.h"

typedef enum enTFPLAYSTATUS_
{
	TF_PLAY_START = (0x0),
	TF_PLAY_STOP,
	TF_PLAY_RUNING,
}enTFPLAYSTATUS;

typedef struct tagPLAYINFO {

	NK_Size nDayTime;
	NK_Int nStartTime;////用UTC时间
	NK_Int nEndTime;/////用UTC时间
	NK_Int nCurPlayTime;////UTC时间
	NK_Int nChn; /////用掩码处理
	NK_Int nType;/////录像回放类型(移动录像/定时录像/普通录像)

	enTFPLAYSTATUS enPlayStatus;////播放状态：开始播放/停止播放/正在播放
	NK_Int enMode;////按时间检索播放/按文件播放模式：TFPLAY_MODE_FILE/TFPLAY_MODE_TIME
	NK_Char strFile[64];////录像回放文件全路径

	/////////////////////////播放文件属性
	NK_Int 		 nFileSum;
	stTFPLAYFILE stFileAttr[20];
	NK_Int 	nCurNumber;////当前正在播放的序号文件
	/////////////////////////播放文件属性

	NK_PVoid hPlay;////TS 播放器句柄
	stFILEDESCIP stFileDesc;////录像文件描述
 }stPLAYINFO, *pstPLAYINFO;


 NK_Int SearchFile(NK_PVoid pContext, NK_Int bFileType, NK_Int nNumber,NK_Int nRecordTotal,NK_PVoid lParam)
 {
 	pstTFPLAYFILE pstFileAttr = (pstTFPLAYFILE)lParam;
  	pstPLAYINFO pstPlay = (pstPLAYINFO)pContext;
	if(NK_Nil == pstPlay || NK_Nil == pstFileAttr)
	{
		return NK_FALSE;
	}
	if(pstPlay->nFileSum >= 20 )
	{
		return NK_FALSE;
	}
	memset(&pstPlay->stFileAttr[pstPlay->nFileSum], 0 , sizeof(stTFPLAYFILE));
	memcpy(&pstPlay->stFileAttr[pstPlay->nFileSum], pstFileAttr, sizeof(stTFPLAYFILE));
	pstPlay->nFileSum ++;
	//printf("SearchFile nChn=%d,nNumber=%d,nFileSize=%d,strFile=%s \n", pstFileAttr->nChn,nNumber,pstFileAttr->nFileSize,pstFileAttr->strFile);
	return NK_TRUE;
 }

 NK_Boolean TFPLAY_HasFile(NK_PChar path)
 {
	 NK_Boolean hasFile = NK_FALSE;
	 if(NK_Nil == path || strlen(path) < 0)
	 {
		 return NK_FALSE;
	 }
	 hasFile = (-1 != access(path, F_OK));
	 return (hasFile ? NK_TRUE : NK_FALSE);
 }


HTFPLAY TFPLAY_Start(NK_Int nType, NK_PVoid lParam)
{
	NK_Int nPlayFile = 0;
	pstPLAYINFO pstPlay = NK_Nil;
	if(NK_Nil == lParam)
	{
		return NK_Nil;
	}
	pstPlay = (pstPLAYINFO)malloc(sizeof(stPLAYINFO));
	if(NK_Nil == pstPlay)
	{
		return NK_Nil;
	}
	memset(pstPlay, 0 , sizeof(stPLAYINFO));
	pstPlay->enMode = nType;

	switch(nType)
	{
		case TFPLAY_MODE_FILE:
			{
				NK_PChar pStrFile = (NK_PChar)lParam;
				if(NK_Nil == (pstPlay->hPlay = TSREAD_Open(pStrFile)))
				{
					goto ERROR;
				}

				TSREAD_GetDescrip(pstPlay->hPlay, (pstFILEDESCIP)&pstPlay->stFileDesc);
			}
			break;
		case TFPLAY_MODE_TIME:
			{

				pstTFPLAYSEARCH hPlayAttr = (pstTFPLAYSEARCH)lParam;
				if(NK_Nil == hPlayAttr)
				{
					goto ERROR;
				}
				pstPlay->nFileSum = 0;
				hPlayAttr->pUserCtx = pstPlay;
				if(NK_FALSE == TFPLAY_Search(hPlayAttr, SearchFile, 0))
				{
					goto ERROR;
				}
				pstPlay->nCurNumber = 0;
				////判断文件是否存在
				if(NK_FALSE == TFPLAY_HasFile(pstPlay->stFileAttr[pstPlay->nCurNumber].strFile))
				{
					goto ERROR;
				}
				pstPlay->hPlay = TSREAD_Open(pstPlay->stFileAttr[pstPlay->nCurNumber].strFile);
				if(NK_Nil == pstPlay->hPlay)
				{
					goto ERROR;
				}
				TSREAD_GetDescrip(pstPlay->hPlay, &pstPlay->stFileDesc);
				printf("w:%d,h:%d \n", pstPlay->stFileDesc.u32Width,pstPlay->stFileDesc.u32Height);
			}
			break;
		default:
			goto ERROR;
	}

	printf("start play ok! \n");
	return (HTFPLAY)pstPlay;

ERROR:

	if(NK_Nil != pstPlay->hPlay)
	{
		TSREAD_Close(pstPlay->hPlay);
		pstPlay->hPlay = NK_Nil;
	}
	if(NK_Nil != pstPlay)
	{
		free(pstPlay);
		pstPlay = NK_Nil;
	}
	//NK_ERROR("start play error! \n");
	return NK_Nil;
}

NK_Int TFPLAY_Stop(HTFPLAY hPlayBack)
{
	pstPLAYINFO hPlayAttr = (pstPLAYINFO)hPlayBack;
	if(NK_Nil == hPlayAttr)
	{
		return NK_FALSE;
	}
	if(NK_Nil != hPlayAttr->hPlay)
	{
		TSREAD_Close(hPlayAttr->hPlay);
		hPlayAttr->hPlay = NK_Nil;
	}
	free(hPlayAttr);
	hPlayAttr = NK_Nil;
	return NK_TRUE;
}


/*
*	1 如果播放单个文件，文件读帧直到读到文件结束
*   2 如果播放按时间播放：通过文件读帧，直到读到文件结束，检索下一个文件开始接着播放
*
*/
NK_Int TFPLAY_ReadFrame(HTFPLAY hPlayBack,pstTFMEDIAHEAD pFrameHead, NK_PVoid* pFrameData)
{
	NK_Int nRet = 0;
	NK_Int nFrameType = 0;
	NK_Char strFile[128] = {0};
	NK_UInt64 dayStartUtc = 0;
	pstPLAYINFO hPlayAttr = (pstPLAYINFO)hPlayBack;
	if(NK_Nil == hPlayAttr || NK_Nil == pFrameHead)
	{
		return EN_TFSDK_PLAY_READ_ERR_FAIL;
	}

	if((TFPLAY_MODE_FILE != hPlayAttr->enMode) && (TFPLAY_MODE_TIME != hPlayAttr->enMode))
	{
		return EN_TFSDK_PLAY_READ_ERR_MODE;
	}

	NK_Int nFormatStatus = TFCARDOPT_GetFormatStatus();////判断是否正在格式化
	if( TFCARD_FORMAT_READY == nFormatStatus ||
		TFCARD_FORMAT_RUNING == nFormatStatus)
	{
		return EN_TFSDK_PLAY_READ_ERR_FAIL;
	}
	if(TFPLAY_MODE_FILE == hPlayAttr->enMode)////按文件播放模式
	{
		if(0 > (nRet = TSREAD_Read(hPlayAttr->hPlay, pFrameData, &pFrameHead->dataSize,
				&nFrameType, (NK_UInt64*)&pFrameHead->coderStamp_us,
				&pFrameHead->isKeyFrame)))
		{
		
			*pFrameData = NULL;
			if(EN_TSREAD_ERR_MALLOC == nRet)
			{
				return EN_TFSDK_PLAY_READ_ERR_MALLOC;
			}
			if(EN_TSREAD_ERR_EOF == nRet)
			{
				return EN_TFSDK_PLAY_READ_ERR_EOF;
			}
			return EN_TFSDK_PLAY_READ_ERR_FAIL;
		}

		if(TS_STREAMTYPE_H264_VIDEO == nFrameType ) pFrameHead->codec = TFCARD_VCODEC_H264;
		if(TS_STREAMTYPE_H265_VIDEO == nFrameType ) pFrameHead->codec = TFCARD_VCODEC_H265;
		if(TS_STREAMTYPE_AAC_AUDIO == nFrameType ) pFrameHead->codec = TFCARD_ACODEC_AAC;

		pFrameHead->fps = hPlayAttr->stFileDesc.fFrameRate;
		pFrameHead->width = hPlayAttr->stFileDesc.u32Width;
		pFrameHead->height = hPlayAttr->stFileDesc.u32Height;

		return EN_TFSDK_PLAY_READ_ERR_SUCCESS;
	}

	if(TFPLAY_MODE_TIME == hPlayAttr->enMode)////按时间检索播放
	{
		////1 判断是否文件播放完成，准备播放下一个文件
		if(0 > (nRet = TSREAD_Read(hPlayAttr->hPlay, pFrameData,
									&pFrameHead->dataSize,
									&nFrameType,
									&pFrameHead->coderStamp_us,
									&pFrameHead->isKeyFrame)))
		{
			*pFrameData = NULL;
			if(EN_TSREAD_ERR_MALLOC == nRet)
			{
				return EN_TFSDK_PLAY_READ_ERR_MALLOC;
			}
			if(EN_TSREAD_ERR_EOF == nRet)
			{
				////寻找下一个文件
				NK_Int nCurPlayFile = hPlayAttr->nCurNumber + 1;
                if(nCurPlayFile >= 20)
                {
                    goto ERROR;
                }
				////判断文件是否存在
				if(NK_FALSE == TFPLAY_HasFile(hPlayAttr->stFileAttr[nCurPlayFile].strFile))
				{
					goto ERROR;
				}
				if(0 > TSREAD_Close(hPlayAttr->hPlay))
				{
					goto ERROR;
				}
				hPlayAttr->hPlay = TSREAD_Open(hPlayAttr->stFileAttr[nCurPlayFile].strFile);
				if(NK_Nil == hPlayAttr->hPlay)
				{
					goto ERROR;
				}
				hPlayAttr->nCurNumber ++;
				return EN_TFSDK_PLAY_READ_ERR_NEXT;
			}

			return EN_TFSDK_PLAY_READ_ERR_FAIL;
		}

		if(TS_STREAMTYPE_H264_VIDEO == nFrameType ) pFrameHead->codec = TFCARD_VCODEC_H264;
		if(TS_STREAMTYPE_H265_VIDEO == nFrameType ) pFrameHead->codec = TFCARD_VCODEC_H265;
		if(TS_STREAMTYPE_AAC_AUDIO == nFrameType ) pFrameHead->codec = TFCARD_ACODEC_AAC;
		pFrameHead->fps = hPlayAttr->stFileDesc.fFrameRate;
		pFrameHead->width = hPlayAttr->stFileDesc.u32Width;
		pFrameHead->height = hPlayAttr->stFileDesc.u32Height;
		dayStartUtc = (NK_UInt64)hPlayAttr->stFileAttr[hPlayAttr->nCurNumber].nStartUtc/(24*60*60)*(24*60*60);
		NK_UInt64 tmpUtc = 0;
		tmpUtc = (NK_UInt64)hPlayAttr->stFileAttr[hPlayAttr->nCurNumber].nStartUtc * 1000 + pFrameHead->coderStamp_us;
		pFrameHead->coderStamp_us = tmpUtc;
		pFrameHead->sysTime_us = pFrameHead->coderStamp_us;
		return EN_TFSDK_PLAY_READ_ERR_SUCCESS;
	}

ERROR:
	return EN_TFSDK_PLAY_READ_ERR_FAIL;
}

enRECORDTYPE TFPLAY_GetFileMode(NK_Int nModeFile)
{
	if('T' == nModeFile)
	{
		return EN_RECORD_TYPE_TIMER;
	}
	if('M' == nModeFile)
	{
		return EN_RECORD_TYPE_MOTION;
	}
	if('A' == nModeFile)
	{
		return EN_RECORD_TYPE_ALARM;
	}
	if('R' == nModeFile)
	{
		return EN_RECORD_TYPE_MANUAL;
	}
	if('U' == nModeFile)
	{
		return EN_RECORD_TYPE_NONE;
	}
	return EN_RECORD_TYPE_NONE;
}

////sorting 按录像名字大小 顺序排列
NK_Int TFPLAY_Search(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB fFunch,NK_Int maxLimitCnt)
{
	NK_Char strDayDir[16] = {0};
	NK_Char strnChnDir[4] = {0};
	NK_Char strHourDir[16] = {0};
	NK_Char strSearchDir[64] = {0};
	NK_Char strFileName[64] = {0};
	stTFPLAYFILE stFileAttr;
	stTFTIME stStartTime;
	NK_Size nStartTime = 0;
	NK_Size nEndTime = 0;
	NK_Int nHour = 0;

	if(NK_Nil == pstPlaySearch || NK_Nil == fFunch)
	{
		return NK_FALSE;
	}

	if(pstPlaySearch->nStartUtc > pstPlaySearch->nEndUtc)
	{

//		NK_INFO("nStartUtc=%lld,nEndUtc=%lld \n", pstPlaySearch->nStartUtc, pstPlaySearch->nEndUtc);
		return NK_FALSE;
	}

	struct tm tv_start = {0};
	struct tm tv_end = {0};
	NK_Size nSecDayStart = 0; ////当天秒数:UTC
	NK_Size nSecDayEnd = 0;////当天秒数:UTC
	NK_Size nSecDayFile = 0;////当天秒数:UTC

	gmtime_r((time_t *)(&pstPlaySearch->nStartUtc),&tv_start);
	gmtime_r((time_t *)(&pstPlaySearch->nEndUtc),&tv_end);

	////目前只做当天检索
	if(tv_start.tm_mday != tv_end.tm_mday)
	{
	
		nSecDayEnd = 86400;
        tv_end.tm_hour = 23;
		printf("tv_start.tm_mday=%d,tv_end.tm_mday=%d \n",tv_start.tm_mday,tv_end.tm_mday);
		//return NK_FALSE;
	}else{
		nSecDayEnd = tv_end.tm_hour * 3600 + tv_end.tm_min * 60 + tv_end.tm_sec;

	}

	nSecDayStart = tv_start.tm_hour * 3600 + tv_start.tm_min * 60 + tv_start.tm_sec;

	if(nSecDayStart > nSecDayEnd)
	{
		return NK_FALSE;
	}


	if(0 == tv_end.tm_hour)
	{
		tv_end.tm_hour = 23;
	}

    for(nHour = tv_start.tm_hour; nHour <= tv_end.tm_hour; nHour++)
    {

    	////1 根据nStartTime生成YYYYMMDD目录字符串
    	sprintf(strDayDir, "%04d%02d%02d", (tv_start.tm_year + 1900),(tv_start.tm_mon + 1), tv_start.tm_mday);
    	//sprintf(strDayDir, "%04d%02d%02d", pstPlaySearch->stDate.nYear,pstPlaySearch->stDate.nMouth,pstPlaySearch->stDate.nDay);
    	sprintf(strHourDir, "%02d00-%02d00", nHour, nHour+1);

    	////2 生成通道目录(/mnt/video/20180505/01/0100-0200)
    	sprintf(strSearchDir,"%s/%s/%02d/%s/%s",TFCARDOPT_GetMountPath(), VIDEO_PATH, pstPlaySearch->nChn, strDayDir,strHourDir);
    	////3 判断文件目录是否存
    	if(NK_FALSE == TFPLAY_HasFile(strSearchDir))
    	{
    		continue;
    		//return NK_FALSE;
    	}
    	struct dirent **entry_list;
    	NK_Int count;
    	struct stat buf;
    	NK_Int result = 0;
    	NK_Int i = 0;
    	NK_Int nNumber = 0;
    	NK_Int nFileMode = 0;

    	NK_Char strRmFile[128] = {0};
    	count = scandir(strSearchDir, &entry_list, 0, alphasort);
    	if (count < 0)
    	{
    		printf("scandir err[%d] ERR!\n", count);
    		return NK_FALSE;
    	}

    	printf("scandir count[%d] ok!\n", count);

        //限制处理结果的最大条目数，如果maxLimitCnt为0，则不限制
        if(maxLimitCnt>0 && maxLimitCnt<count)
        {
            count = maxLimitCnt;
        }
    	for (i = 0; i < count; i++)
    	{
            struct dirent *entry;
            entry = entry_list[i];
    		memset(&buf, 0 , sizeof(struct stat));
    		memset(strRmFile, 0 , sizeof(strRmFile));
    		memset(strFileName , 0 , sizeof(strFileName));
    		sprintf(strRmFile, "%s/%s",strSearchDir,entry->d_name);
    		sprintf(strFileName, "%s",entry->d_name );
    		if(0 != (result = stat(strRmFile,&buf)))
    		{
           		free(entry);
    			continue;
    		}
    		if((0 == strcmp(entry->d_name,".")) || (0 == strcmp(entry->d_name,"..")))
    		{
    			free(entry);
    			continue;
    		}

    		if(S_ISREG(buf.st_mode)) ////文件类型
    		{
    			////4 分析文件名字填充结构体
    			memset(&stFileAttr, 0 , sizeof(stTFPLAYFILE));
    			memset(&stStartTime, 0 , sizeof(stTFTIME));
    			if(NK_FALSE == TFFILE_PareFileName( entry->d_name,
    												&nSecDayFile,
    												(NK_Size*)&stFileAttr.nUsedTime,
    												&nFileMode))
    			{
    			
    	       		free(entry);
    				continue;
    			}

    			////5 过滤文件名
    			////判断文件是否指定设定的时间
    			stFileAttr.nFileType = TFPLAY_GetFileMode(nFileMode);
                if((nSecDayStart <= nSecDayFile && nSecDayFile <= nSecDayEnd) ||
    				((nSecDayStart <= (nSecDayFile + stFileAttr.nUsedTime)) && ((nSecDayFile+stFileAttr.nUsedTime) <= nSecDayEnd )))
    			{
    				////判断文件类型是否一致
    				if((pstPlaySearch->nType & stFileAttr.nFileType))
    				{
    					stFileAttr.nChn = pstPlaySearch->nChn;
    					stFileAttr.nFileSize = buf.st_size;

    					struct tm tv = {0};
    					tv.tm_year = tv_start.tm_year;
    					tv.tm_mon = tv_start.tm_mon;
    					tv.tm_mday = tv_start.tm_mday;

    					stFileAttr.nStartUtc =  timegm(&tv) + nSecDayFile;
    					stFileAttr.nEndUtc = stFileAttr.nStartUtc + stFileAttr.nUsedTime;
    					sprintf(stFileAttr.strFile, "%s/%s", strSearchDir,strFileName);
    					if(NK_Nil != fFunch)
    					{
    						fFunch(pstPlaySearch->pUserCtx, NK_TRUE, nNumber,count, &stFileAttr);
    					}					
    					nNumber ++;
    				}
    			}
    				
    			free(entry);
    			continue;
    		}
    		if(S_ISDIR(buf.st_mode)) ////文件类型是
    		{
    			printf("%s is dir \n", strRmFile);
    		}
            free(entry);
        }
        free(entry_list);
    }
	return NK_TRUE;
}

////sorting 按录像创建时间排序
NK_Int TFPLAY_SearchEx(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB fFunch)
{
	NK_Char strDayDir[16] = {0};
	NK_Char strHourDir[16] = {0};
	NK_Char strnChnDir[4] = {0};
	NK_Char strSearchDir[64] = {0};
	NK_Char strFileName[64] = {0};

	stTFPLAYTIME stCurTimeAttr;
	stTFPLAYTIME stPreTimeAttr;
	NK_Int bFisrt = NK_TRUE;
    NK_Int nHour = 0;

	if(NK_Nil == pstPlaySearch || NK_Nil == fFunch)
	{
		return NK_FALSE;
	}
	if(pstPlaySearch->nStartUtc >= pstPlaySearch->nEndUtc)
	{
		return NK_FALSE;
	}

	struct tm tv_start = {0};
	struct tm tv_end = {0};
	NK_Size nSecDayStart = 0; ////当天秒数:UTC
	NK_Size nSecDayEnd = 0;////当天秒数:UTC
	NK_Size nSecDayFile = 0;////当天秒数:UTC
	NK_Size nSecUsed = 0;

	gmtime_r((time_t *)(&pstPlaySearch->nStartUtc),&tv_start);
	gmtime_r((time_t *)(&pstPlaySearch->nEndUtc),&tv_end);

	////目前只做当天检索
	if(tv_start.tm_mday != tv_end.tm_mday)
	{
		nSecDayEnd = 86400;
        tv_end.tm_hour = 23;
		//return NK_FALSE;
	}else{
		nSecDayEnd = tv_end.tm_hour * 3600 + tv_end.tm_min * 60 + tv_end.tm_sec;

	}
	nSecDayStart = tv_start.tm_hour * 3600 + tv_start.tm_min * 60 + tv_start.tm_sec;

	if(nSecDayStart >= nSecDayEnd)
	{
		return NK_FALSE;
	}

	if(0 == tv_end.tm_hour)
	{
		tv_end.tm_hour = 23;
	}
	
    for(nHour = tv_start.tm_hour; nHour <= tv_end.tm_hour; nHour++)
	{
        ////1 根据nStartTime生成YYYYMMDD目录字符串
        sprintf(strDayDir, "%04d%02d%02d", (tv_start.tm_year + 1900),(tv_start.tm_mon + 1), tv_start.tm_mday);
        sprintf(strHourDir, "%02d00-%02d00", nHour, nHour+1);

        ////2 生成通道目录(/mnt/video/01/20180505)
        sprintf(strSearchDir,"%s/%s/%02d/%s/%s/%s",TFCARDOPT_GetMountPath(), VIDEO_PATH, pstPlaySearch->nChn, strDayDir, strHourDir,strHourDir);

		//NK_INFO("strSearchDir=%s,%02d:%02d:%02d \n", strSearchDir,tv_start.tm_hour,tv_start.tm_min,tv_start.tm_sec);
		////3 判断文件目录是否存
		if(NK_FALSE == TFPLAY_HasFile(strSearchDir))
		{
			
			continue;
			//return NK_FALSE;
		}
		struct dirent **entry_list;
		NK_Int count;
		struct stat buf;
		NK_Int result = 0;
		NK_Int i = 0;
		NK_Int nNumber = 0;
		NK_Int nFileMode = 0;


		NK_Int nUsedTime = 0;////组全连续录像使用总时间

		NK_Char strRmFile[128] = {0};
		count = scandir(strSearchDir, &entry_list, 0, alphasort);
		if (count < 0)
		{
	//		NK_ERROR("scandir err[%d] ERR!\n", count);
			return NK_FALSE;
		}
		//NK_INFO("scandir count[%d] ok!\n", count);


		for (i = 0; i < count; i++)
		{
	        struct dirent *entry;
	        entry = entry_list[i];
			memset(&buf, 0 , sizeof(struct stat));
			memset(strRmFile, 0 , sizeof(strRmFile));
			memset(strFileName , 0 , sizeof(strFileName));
			sprintf(strRmFile, "%s/%s",strSearchDir,entry->d_name);
			sprintf(strFileName, "%s",entry->d_name );
			if(0 != (result = stat(strRmFile,&buf)))
			{
	       		free(entry);
				continue;
			}
			if((0 == strcmp(entry->d_name,".")) || (0 == strcmp(entry->d_name,"..")))
			{
				free(entry);
				continue;
			}
			if(S_ISREG(buf.st_mode)) ////文件类型
			{
				////4 分析文件名字填充结构体
				memset(&stCurTimeAttr, 0 , sizeof(pstTFPLAYTIME));
				if(NK_FALSE == TFFILE_PareFileName( entry->d_name,
													&nSecDayFile,
													&nSecUsed,
													&nFileMode))
				{
					free(entry);
					continue;
				}

				stCurTimeAttr.nFileType = TFPLAY_GetFileMode(nFileMode);
				////判断文件类型是否包括检查条件里
				if(!(pstPlaySearch->nType & stCurTimeAttr.nFileType))
				{
					free(entry);
					continue;
				}

				////判断文件时间是否在检查条件里
	            if( (nSecDayStart <= nSecDayFile && nSecDayFile < nSecDayEnd) ||
					(nSecDayStart < (nSecDayFile + nSecUsed) &&
					(nSecDayFile + nSecUsed)<= nSecDayEnd))
				{

					struct tm tv = {0};
					tv.tm_year = tv_start.tm_year;
					tv.tm_mon = tv_start.tm_mon;
					tv.tm_mday = tv_start.tm_mday;

					stCurTimeAttr.nUsedTime = nSecUsed;
					stCurTimeAttr.nChn = pstPlaySearch->nChn;
					stCurTimeAttr.nStartUtc =  timegm(&tv) + nSecDayFile;
					stCurTimeAttr.nEndUtc = stCurTimeAttr.nStartUtc + stCurTimeAttr.nUsedTime;

					if(NK_TRUE == bFisrt)
					{
						memset(&stPreTimeAttr, 0 , sizeof(stTFPLAYTIME));
						memcpy(&stPreTimeAttr, &stCurTimeAttr, sizeof(stTFPLAYTIME));
						nUsedTime = stCurTimeAttr.nUsedTime;
						bFisrt = NK_FALSE;

						free(entry);
						continue;
					}

					////录像类型不一样，提交前一个录像信息回调
					if(stPreTimeAttr.nFileType != stCurTimeAttr.nFileType)
					{
						//stPreTimeAttr.nUsedTime += stCurTimeAttr.nUsedTime;
						//stPreTimeAttr.nEndUtc = stPreTimeAttr.nStartUtc + stPreTimeAttr.nUsedTime;
						if(NK_Nil != fFunch)
						{
							stPreTimeAttr.nEndUtc = stPreTimeAttr.nStartUtc + nUsedTime;
							stPreTimeAttr.nUsedTime = nUsedTime;
							fFunch(pstPlaySearch->pUserCtx, NK_FALSE, nNumber,count, &stPreTimeAttr);
						}
						nNumber++;
#if 1
						//NK_INFO("zeng start:%lld,end:%lld,used:%lld",stPreTimeAttr.nStartUtc,
						//	stPreTimeAttr.nEndUtc,
						//	stPreTimeAttr.nUsedTime);
#endif
						memset(&stPreTimeAttr, 0 , sizeof(stTFPLAYTIME));
						memcpy(&stPreTimeAttr, &stCurTimeAttr, sizeof(stTFPLAYTIME));
						nUsedTime = stCurTimeAttr.nUsedTime;

						free(entry);
						continue;
					}




					////判断前后时间轴文件是否连续
					NK_Int nRetSub = (stCurTimeAttr.nStartUtc - stPreTimeAttr.nEndUtc);
					if(10 < nRetSub)
					{
						////不连续
						if(NK_Nil != fFunch)
						{
							stPreTimeAttr.nEndUtc = stPreTimeAttr.nStartUtc + nUsedTime;
							stPreTimeAttr.nUsedTime = nUsedTime;
						//	NK_INFO("cur_nStartUtc[%lld],per_nEndUtc[%lld] \n file=%s \n",
						//		stCurTimeAttr.nStartUtc,stPreTimeAttr.nEndUtc,strFileName);
							fFunch(pstPlaySearch->pUserCtx, NK_FALSE, nNumber,count, &stPreTimeAttr);

						}
						nNumber++;
						memset(&stPreTimeAttr, 0 , sizeof(stTFPLAYTIME));
						memcpy(&stPreTimeAttr, &stCurTimeAttr, sizeof(stTFPLAYTIME));
						nUsedTime = stCurTimeAttr.nUsedTime;

						free(entry);
						continue;
					}

					nUsedTime += stCurTimeAttr.nUsedTime;
					stPreTimeAttr.nEndUtc = stPreTimeAttr.nStartUtc + nUsedTime;
					//bEnd = NK_TRUE;

				}
			}
			if(S_ISDIR(buf.st_mode)) ////文件类型是
			{
		//		NK_INFO("%s is dir \n", strRmFile);
			}
	       free(entry);
	    }

		////假如有文件情况，防止最后一个漏掉
		if(NK_FALSE == bFisrt)
		{
			if(NK_Nil != fFunch)
			{
				stPreTimeAttr.nEndUtc = stPreTimeAttr.nStartUtc + nUsedTime;
				stPreTimeAttr.nUsedTime = nUsedTime;
				//NK_INFO("cur_nStartUtc[%lld],per_nEndUtc[%lld] \n file=%s \n",
				//	stCurTimeAttr.nStartUtc,stPreTimeAttr.nEndUtc,strFileName);
				fFunch(pstPlaySearch->pUserCtx, NK_FALSE, nNumber,count, &stPreTimeAttr);
			}
		}

	    free(entry_list);
	}
	return NK_TRUE;
}

int TFPLAY_get_ts_recpath_of_hex(char *filepath, char *ts_file_dateinfo)
{
	// app query date
	struct dirent **namelist;
	static int firstflag = 1;
	long int day_offset = 0;
	unsigned long int	year_num = 0, month_num = 0, day_num = 0;
	char year[10] = {0}, month[10] = {0}, day[10] = {0}, *y = year, *m = month, *d = day;
	char yearAndmonth[10] = {0}, yearAndmonth_tmp[10] = {0};
	char OneMonthInfo[20] = {0};
	char ts_rec_filepath[256] = {0};
	int count = 0, i = 0;

	count = scandir(filepath, &namelist, NULL, alphasort);

	if (count < 0)
	{
		printf("---> scandir error\n");
		return -1;
	}
	else
	{
		//printf("count = %d\n\n", count); //返回找到文件的个数

		for(i = 0; i < count; i++)
		{
			//printf("%02d : %s\n", i, namelist[i]->d_name);

			if(1 < i) //过滤掉两个目录：.和..
			{
				// 对读取到的目录进行处理，拆分成：年、月、日
				snprintf(year,	4+1, "%s", namelist[i]->d_name);
				snprintf(month, 2+1, "%s", namelist[i]->d_name + 4);
				snprintf(day,	2+1, "%s", namelist[i]->d_name + 6);

				y = year;
				m = month;
				d = day;
				while(*(y++) == '0');
				while(*(m++) == '0');
				while(*(d++) == '0');

				year_num   = strtol(y-1, NULL, 0);
				month_num  = strtol(m-1, NULL, 0);
				day_offset = strtol(d-1, NULL, 0); // be used to offset

				//snprintf(year,	4+1, "%lX", year_num);
				//snprintf(month, 2+1, "%lX", month_num);

				//printf("Y = %s, M = %s, D = %s\n", year, month, day);

				// --------------------------------------------------------------------

				// 对年、月、日拼接成十六进制数据
				snprintf(yearAndmonth, sizeof(yearAndmonth), "%s-%s", year, month);

				if(1 == firstflag)
				{
					firstflag = 0;
					snprintf(yearAndmonth_tmp, sizeof(yearAndmonth_tmp), "%s-%s", year, month);
				}
				if(0 == strcmp(yearAndmonth, yearAndmonth_tmp))
				{
					if((0 < day_offset) && (32 > day_offset))
					{
						day_num = (0x01 << (day_offset-1)) | day_num;
					}
				}
				else
				{
					snprintf(day, 8+1, "%lX", day_num);
					if(0 == strlen(ts_rec_filepath))
					{
						snprintf(OneMonthInfo, sizeof(OneMonthInfo),"%s-%s|", yearAndmonth_tmp, day);
					}
					else
					{
						snprintf(OneMonthInfo, sizeof(OneMonthInfo),"%s-%s|", yearAndmonth_tmp, day);
					}

					snprintf(yearAndmonth_tmp, sizeof(yearAndmonth_tmp), "%s-%s", year, month); // update yearAndmonth_tmp

					//printf("%02d : OneMonthInfo = %s\n\n", i, OneMonthInfo);

					snprintf(ts_rec_filepath + strlen(ts_rec_filepath), sizeof(ts_rec_filepath), "%s", OneMonthInfo);

					day_num = ~0xFFFFFFFF & day_num;
					if((0 < day_offset) && (32 > day_offset))
					{
						day_num = (0x01 << (day_offset-1)) | day_num;
					}
				}

				if(i == (count-1))
				{
					snprintf(day, 8+1, "%lX", day_num);
					if(3 == count)
					{
						snprintf(OneMonthInfo, sizeof(OneMonthInfo),"%s-%s", yearAndmonth_tmp, day);
					}
					else
					{
						snprintf(OneMonthInfo, sizeof(OneMonthInfo),"%s-%s", yearAndmonth_tmp, day);
					}
					snprintf(yearAndmonth_tmp, sizeof(yearAndmonth_tmp), "%s-%s", year, month); // update yearAndmonth_tmp

					//printf("%02d : OneMonthInfo = %s\n\n", i, OneMonthInfo);

					snprintf(ts_rec_filepath + strlen(ts_rec_filepath), sizeof(ts_rec_filepath), "%s", OneMonthInfo);

					day_num = ~0xFFFFFFFF & day_num;
				}

				snprintf(yearAndmonth_tmp, sizeof(yearAndmonth_tmp), "%s-%s", year, month);
			}

			free(namelist[i]);
		}
		//printf("\nts_rec_filepath = %s\n\n", ts_rec_filepath);
		free(namelist);
	}

	if(2 == count)
	{
	    // strcpy(ts_file_dateinfo, "NULL");
        firstflag = 1;
        return 0;
	}
	else
	{
        strcpy(ts_file_dateinfo, ts_rec_filepath);
        firstflag = 1;
        return (count - 2);
	}
}


