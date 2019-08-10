
#include "tfcard_export.h"





#define TFCARD_MAX_FILE_SIZE (16*1024) /////16M
#define TFCARD_MAX_DURA_SIZE (3*60) /////180 Sec

#define PACK_SYNC_SIZE (512*1024)

#define TFCARD_MAX_MOTION_TIME (30)  //移动录像30秒
typedef struct tagTFRECORDATTR {
	NK_Int nChnId;////录像对应通道号ID, 从1开始
	enTFRECSTATUS enRecStatus;////录像状态

	NK_Int nRecType;////录像类型用宏定义，表示当前的录像类型，单类型，状态变量
	NK_Int bManual; //是否开启手动录像
	NK_Int bOverload;////是否启用自动覆盖功能[0,1]
	NK_Int nMaxFreeSpace;////tfcard最大剩余空间单位是M,条件判断启动是否bOverload
	NK_Int nMaxFileSize;////打包文件最大容量单位是(K)
	NK_Int nPackType;////打包类型,后缀名.h264/.mp4
    NK_UInt32 nMaxDurTime; ///打包文件最大时间
    NK_UInt64 nbgnTimeStamp; ///打包文件开始时间戳，用来对比文件录像持续时间
    NK_UInt32 nMaxSyncSize;   // When to FSync or Sync;
    NK_UInt32 nSegByteSize;   // Bytes Counting in One Sync Loop;

	NK_PVoid hMediaReader;////media buffer 读写器
	HTFTHREAD hThread; ////启动录像线程

	struct tm tmStart;////当前创建文件时间
	NK_Int nUseSec;////录像使用时间以秒为单位
	time_t nStart_tv; ////开始创建文件获取时间秒数

	NK_Char strFile[64];////打开文件全路径

	stRECSCHEDULE stSchedule;////计划录像时间布防
	stTFRECEVENT stRecEvent; ////录像获取码流事件
 }stTFRECORDATTR, *pstTFRECORDATTR;

NK_Int Tfcard_CheckRecType(NK_Int nRecType)
{
	if( (EN_RECORD_TYPE_TIMER & nRecType)  ||
		(EN_RECORD_TYPE_MOTION & nRecType) ||
		(EN_RECORD_TYPE_ALARM & nRecType) ||
		(EN_RECORD_TYPE_MANUAL & nRecType) )
	{
		return NK_TRUE;
	}
	return NK_FALSE;
}
NK_Int Tfcard_GetRecType(NK_Int enRecType)
{
	if((EN_RECORD_TYPE_MANUAL & enRecType))////手动录像[R]
	{
		return 'R';
	}
	if((EN_RECORD_TYPE_ALARM & enRecType))////报警录像[A]
	{
		return 'A';
	}
	if((EN_RECORD_TYPE_MOTION & enRecType))////移动录像[M]
	{
		return 'M';
	}
	if((EN_RECORD_TYPE_TIMER & enRecType))////定时录像[T]
	{
		return 'T';
	}
	return 'U';
}

////录像回放文件全路径:/mnt/rec/20180419/01/0000-0100/HHMMSS-00005-T.TS
NK_Int TFcard_GetFileStart(pstTFRECORDATTR pRecAttr, NK_PChar pstrFilePath, NK_PChar pstrFileName)
{
	NK_Char strDayDir[64] = {0};
	NK_Char strTime[64] = {0};
    NK_Char strHourDir[64] = {0};
	NK_Int  nType = 0;
	if(NK_Nil == pRecAttr || NK_Nil == pstrFilePath || NK_Nil == pstrFileName)
	{
		return NK_FALSE;
	}
	pRecAttr->nUseSec = 0;
	//获取日期目录
	struct timespec start_tv;
	clock_gettime(CLOCK_MONOTONIC, &start_tv);
	pRecAttr->nStart_tv =start_tv.tv_sec;
	////获取本地时间
	time_t nTime_tv = time(NULL); 
	localtime_r(&nTime_tv, &pRecAttr->tmStart);
	snprintf(strDayDir, sizeof(strDayDir), "%04d%02d%02d", (1900+pRecAttr->tmStart.tm_year),(1 + pRecAttr->tmStart.tm_mon), pRecAttr->tmStart.tm_mday);
    snprintf(strHourDir, sizeof(strHourDir), "%02d00-%02d00", pRecAttr->tmStart.tm_hour, pRecAttr->tmStart.tm_hour + 1);

	//获取日期目录
	snprintf(pstrFilePath,sizeof(strDayDir), "%s/%s/%02d/%s/%s", TFCARDOPT_GetMountPath(), VIDEO_PATH, pRecAttr->nChnId, strDayDir,strHourDir);

	snprintf(strTime,sizeof(strTime), "%02d%02d%02d",pRecAttr->tmStart.tm_hour, pRecAttr->tmStart.tm_min, pRecAttr->tmStart.tm_sec);
	snprintf(pstrFileName,sizeof(strTime),"%s-%05d-%c.TS",strTime,pRecAttr->nUseSec,Tfcard_GetRecType(pRecAttr->nRecType));

	return NK_TRUE;
}

NK_Int TFcard_GetFileEnd(pstTFRECORDATTR pRecAttr, NK_Int enRecType,NK_PChar pstrFilePath , NK_PChar pstrFileName)
{
	NK_Char strDayDir[64] = {0};
	NK_Char strTime[64] = {0};
    NK_Char strHourDir[64] = {0};
	NK_Int  nType = 0;
	if(NK_Nil == pRecAttr || NK_Nil == pstrFileName)
	{
		return NK_FALSE;
	}
	//获取录像使用时间
	struct timespec end_tv;
	clock_gettime(CLOCK_MONOTONIC, &end_tv);
	pRecAttr->nUseSec = end_tv.tv_sec - pRecAttr->nStart_tv;
	////文件路径
	snprintf(strDayDir, sizeof(strDayDir), "%04d%02d%02d", (1900+pRecAttr->tmStart.tm_year),(1 + pRecAttr->tmStart.tm_mon), pRecAttr->tmStart.tm_mday);
	snprintf(strHourDir, sizeof(strHourDir), "%02d00-%02d00", pRecAttr->tmStart.tm_hour, pRecAttr->tmStart.tm_hour + 1);
	snprintf(pstrFilePath, sizeof(strDayDir), "%s/%s/%02d/%s/%s", TFCARDOPT_GetMountPath(), VIDEO_PATH, pRecAttr->nChnId, strDayDir,strHourDir);

	//获取日期目录
	snprintf(strTime,sizeof(strTime),"%02d%02d%02d",pRecAttr->tmStart.tm_hour, pRecAttr->tmStart.tm_min, pRecAttr->tmStart.tm_sec);
	snprintf(pstrFileName,sizeof(strTime),"%s-%05d-%c.TS",strTime,pRecAttr->nUseSec,Tfcard_GetRecType(enRecType));
	return NK_TRUE;
}


NK_Int Tfcard_CreateReader(pstTFRECORDATTR pRecAttr)
{
	if( NK_Nil == pRecAttr ||
		NK_Nil == pRecAttr->stRecEvent.AddReader)
	{
		//NK_INFO("AddReader Nil error!");
		return NK_FALSE;
	}
	if(NK_FALSE == pRecAttr->stRecEvent.AddReader(pRecAttr->nChnId,
		pRecAttr->nRecType,&pRecAttr->hMediaReader))
	{
		//NK_INFO("AddReader FALSE error!");
		return NK_FALSE;
	}
	if(NK_Nil == pRecAttr->hMediaReader)
	{
		//NK_INFO("hMediaReader Nil error!");
		return NK_FALSE;
	}

	return NK_TRUE;
}
NK_Int Tfcard_DestroyReader(pstTFRECORDATTR pRecAttr)
{
	if(NK_Nil == pRecAttr || NK_Nil == pRecAttr->stRecEvent.DelReader)
	{
		return NK_FALSE;
	}
	if(NK_FALSE == pRecAttr->stRecEvent.DelReader(&pRecAttr->hMediaReader))
	{
		return NK_FALSE;
	}
	pRecAttr->hMediaReader = NK_Nil;
	return NK_TRUE;
}

NK_Int Tfcard_CheckReaderEmpty(pstTFRECORDATTR pRecAttr)
{
    if(NK_Nil == pRecAttr || NK_Nil == pRecAttr->stRecEvent.IsEmpty)
	{
		return NK_FALSE;
	}
	if(NK_FALSE == pRecAttr->stRecEvent.IsEmpty(&pRecAttr->hMediaReader))
	{
		return NK_FALSE;
	}
	return NK_TRUE;
}

NK_Int Tfcard_GetMdStatus(pstTFRECORDATTR pRecAttr)
{
    if(NK_Nil == pRecAttr || NK_Nil == pRecAttr->stRecEvent.GetMdStatus)
    {
        return NK_FALSE;
    }

    return pRecAttr->stRecEvent.GetMdStatus(pRecAttr->nChnId);
}

/*
* 日程计划检测
*/

NK_Int Tfcard_JudgeSchedule(pstTFRECORDATTR pRecAttr)
{
	if(NK_Nil == pRecAttr)
	{
		return NK_FALSE;
	}

	struct tm cur_tm;
	time_t utc = time(NULL);
    time_t curDaySec = 0;////当天时间秒数()
	localtime_r((time_t *)(&utc), &cur_tm);
	NK_Int nDay = cur_tm.tm_wday;
    curDaySec = ( 60 * 60 * cur_tm.tm_hour) + (60 * cur_tm.tm_min) + cur_tm.tm_sec;
    NK_Int i = 0;
    for(i = 0;
            i < sizeof(pRecAttr->stSchedule.stSectionSlot)
                / sizeof(pRecAttr->stSchedule.stSectionSlot[0]);
            i++)
    {
        if(pRecAttr->stSchedule.stSectionSlot[i].enable
            && (pRecAttr->stSchedule.stSectionSlot[i].nScheduleType&(1<<nDay)))
        {
            //判断当前时间是否满足
            if(pRecAttr->stSchedule.stSectionSlot[i].nStartTime<pRecAttr->stSchedule.stSectionSlot[i].nStopTime
                && pRecAttr->stSchedule.stSectionSlot[i].nStartTime<=curDaySec
                && pRecAttr->stSchedule.stSectionSlot[i].nStopTime>=curDaySec)
            {
                return NK_TRUE;
            }
        }
    }
#if 0
    for(i=0;i<8;i++)
    {
        //判断当前通道上下文是否满足

		if(pRecAttr->stSchedule.stSectionSlot[i].enable 
        //    && (pRecAttr->stSchedule.stSectionSlot[i].chnMask&(1<<(pRecAttr->nChnId-1))) 
        //   && (pRecAttr->stSchedule.stSectionSlot[i].nRecType&pRecAttr->nRecType)
            && (pRecAttr->stSchedule.stSectionSlot[i].nScheduleType&(1<<nDay)))
        {
            //判断当前时间是否满足
			if(pRecAttr->stSchedule.stSectionSlot[i].nStartTime<pRecAttr->stSchedule.stSectionSlot[i].nStopTime
                && pRecAttr->stSchedule.stSectionSlot[i].nStartTime<=curDaySec
                && pRecAttr->stSchedule.stSectionSlot[i].nStopTime>=curDaySec)
            {
                return NK_TRUE;
            }
        }
    }
#endif
    return NK_FALSE;
}

static HTSWRITE Record_Open(pstTFRECORDATTR pRecAttr,pstTFMEDIAHEAD pstMediaHead)
{
	NK_Char strFileName[64] = {0};
	NK_Char	strFilePath[64] = {0};
	NK_Char strFileRecord[128] = {0};
	int nVencType = EN_MPEG_TS_H264;
	if(NK_Nil == pRecAttr || NK_Nil == pstMediaHead)
	{
		return NK_Nil;
	}
	////判断是否I帧
	if((NK_FALSE == pstMediaHead->isKeyFrame)
	    ||(TFCARD_VCODEC_H264 != pstMediaHead->codec && TFCARD_VCODEC_H265 != pstMediaHead->codec))
	{
		return NK_Nil;
	}
	

	//4 获取当前文件名
	memset(&strFilePath, 0 , sizeof(strFilePath));
	memset(&strFileName, 0 , sizeof(strFileName));
	if(NK_FALSE == TFcard_GetFileStart(pRecAttr,strFilePath,strFileName))
	{
		printf("tfcard Get record path[%s] file[%s] error!\n",strFilePath,strFileName);
		goto ERROR;
	}
	//创建文件目录
	if(NK_FALSE == TFFILE_MakeDir(strFilePath))
	{
		printf("create dir[%s] err!\n", strFilePath);
		goto ERROR;
	}
	////创建录像文件
	sprintf(strFileRecord,"%s/%s", strFilePath, strFileName);
	if(TFCARD_VCODEC_H264 == pstMediaHead->codec)
	{
        nVencType = EN_MPEG_TS_H264;
	}
	else if(TFCARD_VCODEC_H265 == pstMediaHead->codec)
	{
        nVencType = EN_MPEG_TS_H265;
	}
	HTSWRITE hRecord = TSWRITE_Open(strFileRecord, nVencType);
	if(NK_Nil == hRecord)
	{
		printf("create record file[%s] err!\n", strFileRecord);
		goto ERROR;
	}
	//设置录像图标
    if(NK_Nil != pRecAttr->stRecEvent.SetRecStatus)
    {
	    pRecAttr->stRecEvent.SetRecStatus(pRecAttr->nChnId, pRecAttr->nRecType);
    }

	//设置录像Descrip
	int nBitRate = 2*1024*1024;
	TSWRITE_SetDescrip(hRecord, pstMediaHead->width, pstMediaHead->height, nBitRate, pstMediaHead->fps);

	memset(pRecAttr->strFile, 0 , sizeof(pRecAttr->strFile));
	snprintf(pRecAttr->strFile, sizeof(pRecAttr->strFile), "%s", strFileRecord);
    pRecAttr->nbgnTimeStamp = pstMediaHead->coderStamp_us;
	printf("create record file[%s] ok!\n", pRecAttr->strFile);
	return hRecord;

ERROR:
	return NK_Nil;
}
static NK_Int Record_Close(HTSWRITE hWrite, NK_Int nCurRecType, pstTFRECORDATTR pRecAttr)
{
	if(NK_Nil == hWrite || NK_Nil == pRecAttr)
	{
		return NK_FALSE;
	}
	////结束录像文件
	if(0 > TSWRITE_Close(hWrite))
	{
		//NK_ERROR("Record close err!\n");
		return NK_FALSE;
	}
    if(NK_Nil != pRecAttr->stRecEvent.SetRecStatus)
    {
        pRecAttr->stRecEvent.SetRecStatus(pRecAttr->nChnId, EN_RECORD_TYPE_NONE);
    }
	////录像文件重命名
	NK_Char strNewFilePath[128] = {0};
	NK_Char strNewFile[128] = {0};
	NK_Char strFilePath[128] = {0};

	if(NK_FALSE == TFcard_GetFileEnd(pRecAttr, nCurRecType,strFilePath, strNewFile))
	{
		//NK_ERROR("Get file dir[%s],file[%s] err!",strFilePath, strNewFile);
		return NK_FALSE;
	}
	////�ж��Ƿ�����ļ�		
	////�ж��Ƿ��ļ�Ŀ¼
	if( NK_FALSE == TFFILE_HasFile(strFilePath) || 
		NK_FALSE == TFFILE_HasFile(pRecAttr->strFile) )
	{
		return NK_FALSE;
	}
	snprintf(strNewFilePath, sizeof(strNewFilePath), "%s/%s", strFilePath, strNewFile);
	//sprintf(strNewFilePath, "%s/%s", strFilePath, strNewFile);
    if (rename(pRecAttr->strFile, strNewFilePath) == 0)
    {
       // NK_INFO("rename %s to %s \n", pRecAttr->strFile, strNewFilePath);
    }

	return NK_TRUE;
}
static NK_Int Record_Write(HTSWRITE hWrite,pstTFRECORDATTR pRecAttr, pstTFMEDIAHEAD pstMediaHead, NK_PVoid pFrameData)
{
	enTSENCTYPE entype;
	NK_Int nRet = 0;
	if(	NK_Nil == hWrite 		||
		NK_Nil == pstMediaHead	||
		NK_Nil == pRecAttr		||
		NK_Nil == pFrameData )
	{
		return EN_TSWRITE_ERR_FAILT;
	}
		
	switch(pstMediaHead->codec)
	{
		case TFCARD_VCODEC_H264:
			{
				nRet = TSWRITE_Write(hWrite, EN_MPEG_TS_H264, pFrameData,
					pstMediaHead->dataSize,
					pstMediaHead->coderStamp_us,
					pstMediaHead->isKeyFrame);
				if(nRet < 0)
				{
					printf("[%s:%d]write video date %d,isKeyFrame=%d,nRet=%d \n", 
						__func__,__LINE__,pstMediaHead->dataSize,pstMediaHead->isKeyFrame, nRet);
				}
			}
			break;
		case TFCARD_VCODEC_H265:
			{
				//entype = (TFCARD_VCODEC_H264 == pstMediaHead->codec) ? EN_MPEG_TS_H264 : EN_MPEG_TS_H265;
				nRet = TSWRITE_Write(hWrite, EN_MPEG_TS_H265, pFrameData,
					pstMediaHead->dataSize,
					pstMediaHead->coderStamp_us,
					pstMediaHead->isKeyFrame);
				if(nRet < 0)
				{
					printf("[%s:%d]write video data failed: dataSize=%d,isKeyFrame=%d,nRet=%d \n",
						__func__,__LINE__,pstMediaHead->dataSize,pstMediaHead->isKeyFrame, nRet);
				}
			}
			break;
		case TFCARD_ACODEC_AAC:
			{
				nRet = TSWRITE_Write(hWrite, EN_MPEG_TS_AAC,
						pFrameData,
						pstMediaHead->dataSize,
						pstMediaHead->coderStamp_us,
						pstMediaHead->isKeyFrame);
				if(nRet < 0)
				{
					printf("[%s:%d]write aac date failed: dataSize=%d,isKeyFrame=%d,nRet=%d \n",
						__func__,__LINE__,pstMediaHead->dataSize,pstMediaHead->isKeyFrame, nRet);
				}
			}
			break;
        case TFCARD_ACODEC_G711A:
        case TFCARD_ACODEC_G711U:
			{
                enTSENCTYPE tsEncType = EN_MPEG_TS_INVAILD;
                if(TFCARD_ACODEC_AAC == pstMediaHead->codec)
                {
                    tsEncType = EN_MPEG_TS_AAC;
                }
                else if(TFCARD_ACODEC_G711A == pstMediaHead->codec)
                {
                    tsEncType = EN_MPEG_TS_G711A;
                }
                else if(TFCARD_ACODEC_G711U == pstMediaHead->codec)
                {
                    tsEncType = EN_MPEG_TS_G711U;
                }
				nRet = TSWRITE_Write(hWrite, tsEncType,
						pFrameData,
						pstMediaHead->dataSize,
						pstMediaHead->coderStamp_us,
						pstMediaHead->isKeyFrame);
				if(nRet < 0)
				{
					printf("[%s:%d]write aac date failed: dataSize=%d,isKeyFrame=%d,nRet=%d \n",
						__func__,__LINE__,pstMediaHead->dataSize,pstMediaHead->isKeyFrame, nRet);
				}
			}
			break;
		default:
			return EN_TSWRITE_ERR_FAILT;
	}

	return nRet;
}

static NK_Int CheckRecordMode(pstTFRECORDATTR pRecAttr)
{
	if(NK_Nil == pRecAttr)
	{
		return NK_FALSE;
	}
	////检查是否有手动录像
	if(NK_TRUE == pRecAttr->bManual)
	{
		pRecAttr->nRecType = EN_RECORD_TYPE_MANUAL;
		return NK_TRUE;
	}
	////检查移动录像
	if(Tfcard_GetMdStatus(pRecAttr))
	{
	    pRecAttr->nRecType = EN_RECORD_TYPE_MOTION;
	   // if(NK_TRUE == Tfcard_JudgeSchedule(pRecAttr))
	    //{
	        return NK_TRUE;
	    //}
	}

	////检查定时录像
	pRecAttr->nRecType = EN_RECORD_TYPE_TIMER;
	if(NK_TRUE == Tfcard_JudgeSchedule(pRecAttr))
	{
		return NK_TRUE;
	}
	return NK_FALSE;
}
static void* Tfcard_RecLoop(void * lparam)
{
	stTFMEDIAHEAD stMediaHead;
	NK_PVoid pFrameData = NK_Nil;

	NK_Size nFileSize = 0; ////K为单位
	NK_Int	nRet = 0;
	NK_Int 	bClose = NK_FALSE;////是否关闭当前录像
	NK_Int  bFirst = NK_TRUE;
	NK_Int  SyncReader = NK_FALSE;  // 同步请求I帧标志

	HTSWRITE hRecWrite = NK_Nil;
	NK_Int nCurRecType = 0;

	NK_Size nCurFreespace = 0;
	HTFTHREAD phThread = (HTFTHREAD)lparam;
	NK_PVoid  pUserCtx = TFCARD_THR_GetUserCtx(phThread);
	pstTFRECORDATTR pRecAttr = (pstTFRECORDATTR)pUserCtx;
	if(NK_Nil == phThread || NK_Nil == pRecAttr)
	{
		goto ERROR;
	}
	////创建读取器
	if(NK_FALSE == Tfcard_CreateReader(pRecAttr))
	{
		goto ERROR;
	}
	////检查读取器事件是否有效
	if( NK_Nil == pRecAttr->hMediaReader ||
		NK_Nil == pRecAttr->stRecEvent.UlockReader ||
		NK_Nil == pRecAttr->stRecEvent.LockReader ||
		NK_Nil == pRecAttr->stRecEvent.ReadMediaFrame)
	{
		goto ERROR;
	}

	TFCARD_THR_SetName("tf_record");
	sleep(3);
	while(TFCARD_THR_Triger(phThread))
	{
		////判断是否暂停
		if(EN_TFRECORD_STOP == pRecAttr->enRecStatus)
		{
            goto CLOSE_RECORD;
		}
        ////判断是否正在格式化
        NK_Int nFormatStatus = TFCARDOPT_GetFormatStatus();
		if( TFCARD_FORMAT_READY == nFormatStatus ||
			TFCARD_FORMAT_RUNING == nFormatStatus)
		{
            goto CLOSE_RECORD;
		}
			
		////检测TF物理上是否存在事件
		if(NK_FALSE == TFCARDOPT_Detect())
		{
            goto CLOSE_RECORD;
		}
		////检查TFCARD挂载状态和TF卡是否可读写
		
		if(NK_FALSE == TFCARDOPT_IsMounted()
		    || NK_FALSE == TFCARDOPT_RwMounted())
		{
            goto CLOSE_RECORD;
		}
		////检查TFCARD空间是否足够
		nCurFreespace = TFCARDOPT_GetFreespace();
		if( pRecAttr->nMaxFreeSpace > nCurFreespace ||
			TFCARD_MIN_FREE_SPACE > nCurFreespace)
		{
            //空间不够不进行写文件
            goto CLOSE_RECORD;
		}
		//检查录像模式并且是否在计划中当
		
		if(NK_FALSE == CheckRecordMode(pRecAttr))
		{
			goto CLOSE_RECORD;
		}
		
        //判断缓存区是否有数据
        if(NK_TRUE == Tfcard_CheckReaderEmpty(pRecAttr))
        {
		//	usleep(20*1000);
		//	continue;
        }
		////销住锁存读取器
		if(NK_FALSE == pRecAttr->stRecEvent.LockReader(&pRecAttr->hMediaReader))
		{
			//NK_INFO("stRecEvent LockReader error! \n");
			usleep(100*1000);
			continue;
		}
		////获取编码读取流
		memset(&stMediaHead, 0 , sizeof(stTFMEDIAHEAD));
		if(NK_FALSE == pRecAttr->stRecEvent.ReadMediaFrame(
								&pRecAttr->hMediaReader,
								&stMediaHead,
								&pFrameData))
		{
			pRecAttr->stRecEvent.UlockReader(&pRecAttr->hMediaReader);
			usleep(100*1000);
			continue;
		}
		if(NK_Nil == pFrameData)
		{
			printf("[%s:%d]rec read MediaBuffer FrameData failed! \n",__func__,__LINE__);
			pRecAttr->stRecEvent.UlockReader(&pRecAttr->hMediaReader);
			usleep(100*1000);
			continue;
		}
		
		////判断开启录像文件
		if(NK_TRUE == bFirst)
		{
            if(NK_TRUE == SyncReader)
            {
                if(NK_Nil != pRecAttr->stRecEvent.requestStreamKeyframe)
                {
                    pRecAttr->stRecEvent.requestStreamKeyframe();
                }
                pRecAttr->stRecEvent.UlockReader(&pRecAttr->hMediaReader);
                SyncReader = NK_FALSE;
                usleep(100*1000);
                continue;
            }

			TFCARD_THR_Lock(phThread);
			hRecWrite = Record_Open(pRecAttr,&stMediaHead);
			TFCARD_THR_UnLock(phThread);
			
			if(NK_Nil == hRecWrite)
			{
				printf("[%s:%d]Record Open file failed! \n",__func__,__LINE__);
				pRecAttr->stRecEvent.UlockReader(&pRecAttr->hMediaReader);
				usleep(100*1000);
				continue;
			}
			////当前打开录像类型
			nCurRecType = pRecAttr->nRecType;
			bFirst = NK_FALSE;
			nFileSize = 0;

		////如果报警/移动录像:按时间关闭录像
		}
		////如果录像类型改变，需要关闭当前录像
		if(nCurRecType != pRecAttr->nRecType)
		{
			bClose = NK_TRUE;
		}

		////录像文件大小控制/关闭当前录像
		if( pRecAttr->nMaxFileSize < (nFileSize / 1024) || NK_TRUE == bClose
		        || (abs(stMediaHead.coderStamp_us - pRecAttr->nbgnTimeStamp) / 1000 >= pRecAttr->nMaxDurTime))
		{
			if(NK_TRUE == stMediaHead.isKeyFrame) ////是否为关键帧
			{
				////关闭录像文件
				if(NK_Nil != hRecWrite)
				{
				
					TFCARD_THR_Lock(phThread);
					Record_Close(hRecWrite, nCurRecType, pRecAttr);
					TFCARD_THR_UnLock(phThread);

					hRecWrite = NK_Nil;
					bClose = NK_FALSE;
				}
				////启新录像文件
				if(NK_Nil == hRecWrite)
				{
					TFCARD_THR_Lock(phThread);
					hRecWrite = Record_Open(pRecAttr, &stMediaHead);
					TFCARD_THR_UnLock(phThread);
					
					if(NK_Nil == hRecWrite)
					{
						//NK_INFO("Record_Open err!");
						pRecAttr->stRecEvent.UlockReader(&pRecAttr->hMediaReader);
						usleep(100*1000);
						continue;
					}
					////保存当前录像类型
					nCurRecType = pRecAttr->nRecType;
					nFileSize = 0;
				}
			}
		}
		////累加帧大小
		nFileSize += stMediaHead.dataSize;

		////写帧,如果是文件失败，那需要先关闭文件，再打开写文件
		if(NK_Nil != hRecWrite)
		{
			TFCARD_THR_Lock(phThread);
			nRet = Record_Write(hRecWrite,pRecAttr, &stMediaHead, pFrameData);
			TFCARD_THR_UnLock(phThread);
			if(EN_TSWRITE_ERR_FILE == nRet)////写文件错误，需要关闭视频
			{
				////关闭录像文件
				if(NK_Nil != hRecWrite)
				{
					TFCARD_THR_Lock(phThread);
					Record_Close(hRecWrite, nCurRecType, pRecAttr);
					TFCARD_THR_UnLock(phThread);
					
					hRecWrite = NK_Nil;
					SyncReader = NK_TRUE;
					bFirst = NK_TRUE; ////开启第一帧录像标识
				}
			}
		}
		////解除锁存读取器
		pRecAttr->stRecEvent.UlockReader(&pRecAttr->hMediaReader);

        pRecAttr->nSegByteSize += stMediaHead.dataSize;
        if(pRecAttr->nSegByteSize >= pRecAttr->nMaxSyncSize) // Reach MaxSyncSize to Fsync
        {
            pRecAttr->nSegByteSize = 0;
            TSWRITE_Sync(hRecWrite);
        }

        continue;
		
CLOSE_RECORD:
        if(NK_Nil != hRecWrite)
        {
        
			TFCARD_THR_Lock(phThread);
            Record_Close(hRecWrite, nCurRecType, pRecAttr);
			TFCARD_THR_UnLock(phThread);
			
            hRecWrite = NK_Nil;
            SyncReader = NK_TRUE;
            bFirst = NK_TRUE; ////开启第一帧录像标识
        }
        sleep(1);
        continue;
#if 0
		memset(&strFilePath, 0 , sizeof(strFilePath));
		memset(&strFileName, 0 , sizeof(strFileName));
		if(NK_FALSE == TFcard_GetFileStart(pRecAttr,strFilePath, strFileName))
		{
			NK_INFO("tfcard Get record path[%s] file[%s] error!\n",strFilePath,strFileName);
			goto WHILE_END;
		}
		NK_INFO("record file path[%s],file[%s] \n", strFilePath,strFileName );
		if(NK_FALSE == Tfcard_RecordFile(pRecAttr, strFilePath,strFileName))
		{
			NK_INFO("Tfcard WriteFile name[%s/%s] error!\n",strFilePath,strFileName);
			goto WHILE_END;
		}
#endif
	}

ERROR:


	////销毁读取器
	if(NK_Nil != pRecAttr->hMediaReader)
	{
		Tfcard_DestroyReader(pRecAttr);
	}
	TFCARD_THR_Cancel(phThread);
	return NK_Nil;
}

///////////////////////////////////////////////////////////////////
HTFRECORD TFRECORD_Create(pstTFRECPARAM pstRecParam, pstTFRECEVENT pstRecEvent)
{
	if(NK_Nil == pstRecParam || NK_Nil == pstRecEvent)
	{
		return NK_Nil;
	}
	pstTFRECORDATTR pTfRecAttr = (pstTFRECORDATTR)malloc(sizeof(stTFRECORDATTR));
	if(NK_Nil == pTfRecAttr)
	{
		goto ERROR;
	}
	memset(pTfRecAttr, 0, sizeof(stTFRECORDATTR));

	pTfRecAttr->nMaxFileSize = TFCARD_MAX_FILE_SIZE;
	pTfRecAttr->nMaxDurTime = TFCARD_MAX_DURA_SIZE;
	pTfRecAttr->nMaxSyncSize = PACK_SYNC_SIZE;
	pTfRecAttr->nSegByteSize = 0;
	pTfRecAttr->nChnId = pstRecParam->nChnId;
	pTfRecAttr->nRecType = pstRecParam->enRecType;

	pTfRecAttr->stRecEvent.AddReader = pstRecEvent->AddReader;
	pTfRecAttr->stRecEvent.DelReader = pstRecEvent->DelReader;
	pTfRecAttr->stRecEvent.LockReader = pstRecEvent->LockReader;
	pTfRecAttr->stRecEvent.UlockReader = pstRecEvent->UlockReader;
	pTfRecAttr->stRecEvent.ReadMediaFrame = pstRecEvent->ReadMediaFrame;
    pTfRecAttr->stRecEvent.IsEmpty = pstRecEvent->IsEmpty;
    pTfRecAttr->stRecEvent.SyncReader = pstRecEvent->SyncReader;
    pTfRecAttr->stRecEvent.requestStreamKeyframe = pstRecEvent->requestStreamKeyframe;
    pTfRecAttr->stRecEvent.GetMdStatus = pstRecEvent->GetMdStatus;
    pTfRecAttr->stRecEvent.SetRecStatus = pstRecEvent->SetRecStatus;

	HTFTHREAD hThread = TFCARD_THR_CreateEx(Tfcard_RecLoop,pTfRecAttr);
	if(NK_Nil == hThread)
	{
		goto ERROR;
	}

    //验证这种方式是否可以
	TFCARD_THR_Lock(hThread);
	pTfRecAttr->hThread = hThread;
	TFCARD_THR_UnLock(hThread);

//	NK_TRACE("TFRECORD_Create ok!\n");
	return (HTFRECORD)pTfRecAttr;
ERROR:

	if(pTfRecAttr->hThread)
	{
		TFCARD_THR_Destory(pTfRecAttr->hThread);
	}
	if(pTfRecAttr)
	{
		free(pTfRecAttr);
	}

	printf("TFRECORD_Create error!\n");
	return NK_Nil;
}
NK_Int TFRECORD_Destroy(HTFRECORD hRecord)
{
	pstTFRECORDATTR pTfRecAttr  = (pstTFRECORDATTR) hRecord;
	if(NK_Nil == pTfRecAttr)
	{
		return NK_FALSE;
	}
	if(pTfRecAttr->hThread)
	{
		TFCARD_THR_Destory(pTfRecAttr->hThread);
	}

	free(pTfRecAttr);
	pTfRecAttr = NK_Nil;
//	NK_TRACE("TFRECORD_Destroy ok!\n");
	return NK_TRUE;
}
NK_Int TFRECORD_SetParam(HTFRECORD hRecord, enTFRECOPT emType, NK_PVoid lParam)
{
	pstTFRECORDATTR pTfRecAttr	= (pstTFRECORDATTR)hRecord;
	if(NK_Nil == pTfRecAttr || NK_Nil == pTfRecAttr->hThread)
	{
		return NK_FALSE;
	}
	switch(emType)
	{
		case EN_TFRECORD_OPT_RECTYPE:
			{
				NK_Int nRecType = *(NK_Int*)lParam;
				TFCARD_THR_Lock(pTfRecAttr->hThread);
				pTfRecAttr->nRecType = nRecType;
				TFCARD_THR_UnLock(pTfRecAttr->hThread);
			}
			break;
		case EN_TFRECORD_OPT_SCHEDULE:
			{
				pstRECSCHEDULE pstSchedule = (pstRECSCHEDULE)lParam;
				if(NK_Nil == pstSchedule)
				{
					return NK_FALSE;
				}
				TFCARD_THR_Lock(pTfRecAttr->hThread);
				memcpy(&pTfRecAttr->stSchedule,pstSchedule,sizeof(stRECSCHEDULE));
				TFCARD_THR_UnLock(pTfRecAttr->hThread);

			}
			break;
		case EN_TFRECORD_OPT_RECSTART:
			{
				TFCARD_THR_Lock(pTfRecAttr->hThread);
				pTfRecAttr->enRecStatus = EN_TFRECORD_START;
				TFCARD_THR_UnLock(pTfRecAttr->hThread);
			}
			break;
		case EN_TFRECORD_OPT_RECSTOP:
			{
				TFCARD_THR_Lock(pTfRecAttr->hThread);
				pTfRecAttr->enRecStatus = EN_TFRECORD_STOP;
				TFCARD_THR_UnLock(pTfRecAttr->hThread);
			}
			break;
        case EN_TFRECORD_OPT_MANUALREC:
            {
                printf("@@@ manual rec  %d\n", *(NK_Int*)lParam);
                TFCARD_THR_Lock(pTfRecAttr->hThread);
                pTfRecAttr->bManual = *(NK_Int*)lParam;
                TFCARD_THR_UnLock(pTfRecAttr->hThread);
            }
            break;
		default:
			return NK_FALSE;
	}
	return NK_TRUE;
}

TFRECORD_RecordFile_Get(HTFRECORD hRecord, NK_PChar recordFile)
{
	pstTFRECORDATTR pTfRecAttr = (pstTFRECORDATTR)hRecord;
	if(NK_Nil == pTfRecAttr)
	{
		return NK_FALSE;
	}
	strncpy(recordFile,pTfRecAttr->strFile,strlen(pTfRecAttr->strFile));
}



