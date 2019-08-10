
#include "tfcard_export.h"
#include <mntent.h>

static pstTFCARDATTR g_pTfcardAttr;
static HTFTHREAD g_hThread;

typedef struct tagTFRECYCLEFILE {
	NK_Int nChn;
	NK_Char strFilePath[64];
	NK_Char strFileName[64];
}stTFRECYCLEFILE, *pstTFRECYCLEFILE;

NK_Boolean TFCARD_MountAttr(pstTFCARDATTR ptfAttr)
{
	 NK_PChar filename = "/proc/mounts";
	 FILE *mntfile;
	 struct mntent *mntent;
	 if(NK_Nil == ptfAttr)
	 {
		 return NK_FALSE;
	 }
	 mntfile = setmntent(filename, "r");
	 if (!mntfile)
	 {
		 printf("Failed to read mtab file, error [%s]\n", strerror(errno));
		 return NK_FALSE;
	 }
	while((mntent = getmntent(mntfile))!=NULL)
	{
		 if( 0 == strcmp(ptfAttr->TFSlot.dev_path, mntent->mnt_fsname) &&
			 0 == strcmp(ptfAttr->TFSlot.fs_path, mntent->mnt_dir)) ////判断是否已经挂载上的设备
		 {
			 //ptfAttr->TFSlot.mounted = NK_TRUE;
			 if( (0 != strcmp(ptfAttr->TFSlot.strType, mntent->mnt_type))
			        && (0 != strcmp("exfat", mntent->mnt_type)))////判断格式vfat
			 {
				 //ptfAttr->TFSlot.formating = NK_TRUE;
				 printf("tfcard need format flash \n");
			 }
			 if(0 != strcmp(ptfAttr->TFSlot.strOpts, mntent->mnt_opts))////判断文件是否可读可写
			 {

			 }
			 strncpy(ptfAttr->TFSlot.strOpts,mntent->mnt_opts,sizeof(ptfAttr->TFSlot.strOpts));
			 //anyka_print("%s, %s, %s, %s \n",mntent->mnt_dir,mntent->mnt_fsname,mntent->mnt_type,mntent->mnt_opts);
			 endmntent(mntfile);
			 return NK_TRUE;
		 }
	}
	endmntent(mntfile);
	//ptfAttr->TFSlot.mounted = NK_FALSE;
	return NK_FALSE;

}

NK_Boolean TFCARDOPT_IsMountEx(pstTFCARDATTR ptfAttr)
{
   return TFCARD_MountAttr(ptfAttr);
}

NK_Boolean TFCARDOPT_RwMountEx(pstTFCARDATTR ptfAttr)
{
	pstTFCARDATTR tfCardAttr = ptfAttr;
	if(TFCARD_MountAttr(tfCardAttr)){
		if(!strcmp("rw",tfCardAttr->TFSlot.strOpts)){
			return NK_TRUE;
		}
	}
   return NK_FALSE;
}


static NK_Int TFCARDOPT_GetChnOldDir(NK_Int nChnMax, NK_PChar pOldDir, NK_PChar pOldName)
{
	NK_Int nRet = 0;
	NK_Int nChn = 0;
	NK_Int bFist = NK_TRUE;
	NK_Char strDirPath[64] = {0};

	NK_Char strOldPath[64] = {0};
	NK_Char strOldName[64] = {0};
	NK_Char strTmpName[64] = {0};

	stTFRECYCLEFILE stOldFile;
	stTFRECYCLEFILE stTmpFile;
    static NK_Int startIndex = 0;
    NK_Int actualChn = 0;

	for(nChn = 0; nChn < nChnMax; nChn++)
	{
		memset(&stOldFile, 0 , sizeof(stTFRECYCLEFILE));
		memset(strDirPath, 0 , sizeof(strDirPath));
        actualChn = (nChn+startIndex)%nChnMax;
		snprintf(strDirPath, sizeof(strDirPath), "%s/%s/%02d", TFCARDOPT_GetMountPath(),VIDEO_PATH, actualChn); //tf卡文件夹通道下标从1开始
		if(NK_FALSE == TFFILE_GetOldDir(strDirPath, stOldFile.strFilePath,stOldFile.strFileName))
		{
			continue;
		}
		stOldFile.nChn = actualChn;

		////判断文件名是否最小，用冒泡法
		if(NK_TRUE == bFist)
		{
			memset(&stTmpFile, 0 , sizeof(stTFRECYCLEFILE));
			memcpy(&stTmpFile, &stOldFile, sizeof(stTFRECYCLEFILE));
			bFist = NK_FALSE;
			continue;
		}
		if(0 <= strcmp(stOldFile.strFileName,stTmpFile.strFileName))
		{
			continue;
		}
		//// stOldFile.strFileName < stTmpFile.strFileName
		memset(&stTmpFile, 0 , sizeof(stTFRECYCLEFILE));
		memcpy(&stTmpFile, &stOldFile, sizeof(stTFRECYCLEFILE));
	}

    startIndex++;
    if(startIndex>=nChnMax)
    {
        startIndex%=nChnMax;
    }

	/////
	snprintf(pOldDir, sizeof(stTmpFile.strFilePath),"%s", stTmpFile.strFilePath);
	snprintf(pOldName, sizeof(stTmpFile.strFileName),"%s", stTmpFile.strFileName);
	return NK_TRUE;
}


/**
 * 检测当前TF卡的是否插入/是否挂载功能/是否格式化
 */
static NK_Void* tfcard_detector(void *lparam)
{
	HTFTHREAD phThread = (HTFTHREAD)lparam;
	NK_PVoid  pUserCtx = TFCARD_THR_GetUserCtx(phThread);
	NK_Int nCheckMount = 0;

	NK_Int nCheckDetect = 0;
	NK_Int bEnableMount = NK_FALSE;
	NK_Int bEnableUmount = NK_FALSE;
	NK_Int bDetect = NK_FALSE;
	NK_Int UmntErrNum = 0;
	NK_Int UndetectedCnt = 0;
	NK_Int repairCnt = 0;

	TFCARD_THR_SetName("tf_detector");
	while(TFCARD_THR_Triger(phThread))
	{
		////检测TF是否插入

		NK_Boolean const existed = TFCARDOPT_Exist();
		NK_Boolean const bDetected = TFCARDOPT_Detect();

		NK_Boolean const bmounted = TFCARDOPT_IsMountEx(g_pTfcardAttr);

        /**
         * 初始化TF卡的挂载状态
          */
        g_pTfcardAttr->TFSlot.mounted = bmounted;

		/**
		 * 当前需要格式化TF卡
		 */

		NK_Int nFormatStatus = TFCARDOPT_GetFormatStatus();
		if((NK_TRUE == existed && TFCARD_FORMAT_READY == nFormatStatus))
		{
			sleep(1);
			if( g_pTfcardAttr->TFSlot.mounted){
				if(NK_True ==  TFCARDOPT_Umounted()){
					UmntErrNum = 0;
				}else{
					UmntErrNum++;
				}
			}else{
                UmntErrNum = 0;
            }

			 if ((0 == UmntErrNum) || (UmntErrNum > 2)) 
			 {
				 if(0 == TFCARDOPT_FormatTF())////格式化TF
				 {
				     sleep(3);
                     UmntErrNum = 0;
					 g_pTfcardAttr->TFSlot.formating = TFCARD_FORMAT_FINISH;
				 }else{
					 g_pTfcardAttr->TFSlot.formating = TFCARD_FORMAT_INVALID;
				//	 g_pTfcardAttr->EventSet.fTFcardOnAfterFormat(NK_FALSE);

				 }
				 
				 g_pTfcardAttr->TFSlot.mounted = NK_FALSE;
			 }
			continue;
		}else if(existed && (!bDetected)){
		// 自动格式化插入的没有分区的TF卡. 3 * 3s = 9s 之后仍然检测不到分区就格式化
			printf("[%s:%d]No partition detected UndetectedCnt:%d\n",__func__,__LINE__,UndetectedCnt);
			UndetectedCnt++;
			if (UndetectedCnt >= 3) {
					printf("%s: No partition detected, start to format tfcard!\n", __FUNCTION__);
					g_pTfcardAttr->TFSlot.formating = TFCARD_FORMAT_READY;
					UndetectedCnt = 0;
			} 

		}

		/**
		 * 当前 TF 卡未连接但是此时检测到 TF 卡存在，有可能用户刚插入 TF 卡。
		 */
		if(NK_TRUE == bDetected && NK_FALSE == bmounted)
		{
			if(NK_TRUE == TFCARDOPT_Mounted()) ////挂载目录
			{
				g_pTfcardAttr->TFSlot.mounted = NK_TRUE;
				if( TFCARD_FORMAT_FINISH == g_pTfcardAttr->TFSlot.formating){
				//	g_pTfcardAttr->EventSet.fTFcardOnAfterFormat(NK_TRUE);
				}
			}
		}

		/**
		 * 当前 TF 卡上个周期已经连接但此时检测不到 TF 卡存在。
		 * 有可能 TF 卡已经被强行拆卸。
		 */

		if(NK_FALSE == bDetected && NK_TRUE == bmounted)
		{
			if(NK_TRUE == TFCARDOPT_Umounted()) ////挂载目录
			{
				g_pTfcardAttr->TFSlot.mounted = NK_FALSE;
			}
		}

		if(NK_TRUE == bDetected && NK_TRUE == bmounted && repairCnt < 5){	
			/***通过TF属性判断TF当前是否可读写****/

			if(!TFCARDOPT_RwMountEx(g_pTfcardAttr)){
				printf("[%s:%d]FAT-fs Filesystem has been set read-only\n",__func__,__LINE__);
				TFCARDOPT_RemountRo();
				sleep(2);
				/***确认remount后TF是否恢复可读写状态***/
				if(!TFCARDOPT_RwMountEx(g_pTfcardAttr)){
					TFCARDOPT_RepairRo();
				}
				repairCnt++;    //每自动修复一次则累加,超过五次后则认为TF卡异常，需要工具进行深度修复
			}
		}

		////******远程设置触发修复TF只读属性********////
		if(g_pTfcardAttr->TFSlot.repairing)
		{
			TFCARDOPT_FsckRo();
		}

        if(NK_TRUE == bDetected && NK_TRUE == bmounted)
        {
			NK_Int64 nCurFreespace = TFCARDOPT_GetFreespace();
			if((2 * TFCARD_MIN_FREE_SPACE) > nCurFreespace && NK_TRUE == TFCARDOPT_GetOverWrite())
			{
				NK_Int nChnMax = 1;//4;
				NK_Int nRmFileMax = 10;
				NK_Char strOldName[64] = {0};
				NK_Char strOldPath[64] = {0};
	            ////获取多通道最旧的文件目录
	            if(NK_FALSE == TFCARDOPT_GetChnOldDir(nChnMax, strOldPath, strOldName))
				{
					sleep(1);
					continue;
				}
				////删除最旧的文件
				if(NK_FALSE == TFFILE_RemoveOldfile(strOldPath, nRmFileMax))
				{
					sleep(1);
					continue;
				}
			}
		}
		sleep(3);
	}

	TFCARD_THR_Cancel(phThread);
	return NK_Nil;
}



NK_Int TFCARDOPT_Init(pstTFPARAM ptfParam, pstTFSDKEVENT pEventSet)
{
	if(NK_Nil == ptfParam || NK_Nil == pEventSet)
	{
		return NK_FALSE;
	}
	if(NK_Nil != g_pTfcardAttr)
	{
		return NK_TRUE;
	}
	if(NK_Nil == (g_pTfcardAttr = calloc(sizeof(stTFCARDATTR), 1)))
	{
		return NK_FALSE;
	}
	g_pTfcardAttr->EventSet.OnCleanTF = pEventSet->OnCleanTF;
	g_pTfcardAttr->EventSet.OnDetectTF = pEventSet->OnDetectTF;
	g_pTfcardAttr->EventSet.OnExistTF = pEventSet->OnExistTF;
	g_pTfcardAttr->EventSet.OnFormat = pEventSet->OnFormat;
	g_pTfcardAttr->EventSet.OnGetCapacity = pEventSet->OnGetCapacity;
	g_pTfcardAttr->EventSet.OnGetFreeSpace = pEventSet->OnGetFreeSpace;
	g_pTfcardAttr->EventSet.OnMountTF = pEventSet->OnMountTF;
	g_pTfcardAttr->EventSet.OnUmountTF = pEventSet->OnUmountTF;
	//g_pTfcardAttr->EventSet.OnGetStauts = pEventSet->OnGetStauts;
	g_pTfcardAttr->EventSet.OnGetOverWrite = pEventSet->OnGetOverWrite;
	g_pTfcardAttr->EventSet.fTFcardOnAfterFormat = pEventSet->fTFcardOnAfterFormat;
	g_pTfcardAttr->EventSet.fTFcardOnRemountRo = pEventSet->fTFcardOnRemountRo;
	g_pTfcardAttr->EventSet.fTFcardOnRepairRo  = pEventSet->fTFcardOnRepairRo;
	g_pTfcardAttr->EventSet.fTFcardOnFsckRo = pEventSet->fTFcardOnFsckRo;


	memset(&g_pTfcardAttr->TFSlot, 0 , sizeof(g_pTfcardAttr->TFSlot));
	sprintf(g_pTfcardAttr->TFSlot.strType, "%s", "vfat");
	sprintf(g_pTfcardAttr->TFSlot.strOpts, "%s", "rw");

	sprintf(g_pTfcardAttr->TFSlot.dev_path, "%s", ptfParam->strDevpath);
	sprintf(g_pTfcardAttr->TFSlot.fs_path, "%s", ptfParam->strMountPath);

	g_pTfcardAttr->TFSlot.mounted = NK_FALSE;
	g_pTfcardAttr->TFSlot.repairing = NK_FALSE;
	printf("dev_path= %s \n", g_pTfcardAttr->TFSlot.dev_path);
	printf("fs_path= %s \n", g_pTfcardAttr->TFSlot.fs_path);

	/////开启TF检测服务线程(是否热插扳，是否挂载，是否格式化)
	g_hThread = TFCARD_THR_CreateEx(tfcard_detector,NK_Nil);
	if(NK_Nil == g_hThread)
	{
		return NK_FALSE;
	}

	return NK_TRUE;
}
NK_Int TFCARDOPT_Exit()
{
	if(NK_Nil != g_hThread)
	{
		TFCARD_THR_Destory(g_hThread);
	}

	TFCARDOPT_Umounted();
	if(NK_Nil != g_pTfcardAttr)
	{
		free(g_pTfcardAttr);
		g_pTfcardAttr = NK_Nil;
	}
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
	return NK_TRUE;
}

NK_Boolean TFCARDOPT_Exist()
{
	if( NK_Nil == g_pTfcardAttr ||
		NK_Nil == g_pTfcardAttr->EventSet.OnExistTF)
	{
		return NK_FALSE;
	}
	return g_pTfcardAttr->EventSet.OnExistTF();
}

NK_Boolean TFCARDOPT_Detect()
{
	if( NK_Nil == g_pTfcardAttr ||
		NK_Nil == g_pTfcardAttr->EventSet.OnDetectTF)
	{
		return NK_FALSE;
	}
	return g_pTfcardAttr->EventSet.OnDetectTF();
}

NK_Boolean TFCARDOPT_IsMounted()
{
	if(NK_Nil == g_pTfcardAttr)
	{
		return NK_FALSE;
	}
	return g_pTfcardAttr->TFSlot.mounted;
}

/**TF属性，直接在tfcard_opt.c实现**/
NK_Boolean TFCARDOPT_RwMounted()
{
	return TFCARDOPT_RwMountEx(g_pTfcardAttr);
}


NK_Boolean TFCARDOPT_Mounted()
{
	if( NK_Nil == g_pTfcardAttr || NK_Nil == g_pTfcardAttr->EventSet.OnMountTF)
	{
		return NK_FALSE;
	}
//	NK_INFO("mount fs=%s \n",g_pTfcardAttr->TFSlot.fs_path);
	return g_pTfcardAttr->EventSet.OnMountTF(g_pTfcardAttr->TFSlot.fs_path);
}
NK_Boolean TFCARDOPT_Umounted()
{
	if( NK_Nil == g_pTfcardAttr || NK_Nil == g_pTfcardAttr->EventSet.OnUmountTF)
	{
		return NK_FALSE;
	}

//	NK_INFO("umount fs=%s \n",g_pTfcardAttr->TFSlot.fs_path);
	return g_pTfcardAttr->EventSet.OnUmountTF(g_pTfcardAttr->TFSlot.fs_path);
}


NK_Boolean TFCARDOPT_CleanTF()
{
	if( NK_Nil == g_pTfcardAttr || NK_Nil == g_pTfcardAttr->EventSet.OnCleanTF)
	{
		return NK_FALSE;
	}
	return g_pTfcardAttr->EventSet.OnCleanTF(g_pTfcardAttr->TFSlot.fs_path);
}


NK_Size TFCARDOPT_GetCapacity()
{
	if( NK_Nil == g_pTfcardAttr || NK_Nil == g_pTfcardAttr->EventSet.OnGetCapacity)
	{
		return NK_FALSE;
	}
	return g_pTfcardAttr->EventSet.OnGetCapacity(g_pTfcardAttr->TFSlot.fs_path);
}

NK_Size TFCARDOPT_GetFreespace()
{
	if( NK_Nil == g_pTfcardAttr ||	NK_Nil == g_pTfcardAttr->EventSet.OnGetFreeSpace)
	{
		return NK_FALSE;
	}
	return g_pTfcardAttr->EventSet.OnGetFreeSpace(g_pTfcardAttr->TFSlot.fs_path);
}

NK_Int TFCARDOPT_FormatTF()
{	
	if( NK_Nil == g_pTfcardAttr ||	NK_Nil == g_pTfcardAttr->EventSet.OnFormat)
	{
		return NK_FALSE;
	}
	g_pTfcardAttr->TFSlot.formating = TFCARD_FORMAT_RUNING;
	return g_pTfcardAttr->EventSet.OnFormat();
}

NK_Int TFCARDOPT_RepairTF()
{	
	if( NK_Nil == g_pTfcardAttr ||	NK_Nil == g_pTfcardAttr->EventSet.fTFcardOnFsckRo)
	{
		return NK_FALSE;
	}
	g_pTfcardAttr->TFSlot.repairing = TFCARD_REPAIR_RUNING;
	return g_pTfcardAttr->EventSet.fTFcardOnFsckRo();
}

NK_Int TFCARDOPT_GetFormatStatus()
{
	if( NK_Nil == g_pTfcardAttr )
	{
		return NK_FALSE;
	}
	return g_pTfcardAttr->TFSlot.formating;
}

////***get TF status***////
NK_Int TFCARDOPT_GetTfcardStatus( char *ret_status)
{
	if(ret_status == NULL){
		printf("tf card status NULL\n");
		return -1;
	}
	int ret = EN_TFCORD_STATUS_NO_TFCARD;
	if(TFCARDOPT_Exist()){
		if(TFCARD_FORMAT_READY == g_pTfcardAttr->TFSlot.formating 
		 ||TFCARD_FORMAT_RUNING == g_pTfcardAttr->TFSlot.formating ){
			sprintf(ret_status,sTFCARD_STATUS_FORMATTING);
			ret = EN_TFCORD_STATUS_FORMATTING;
		 }else if(TFCARDOPT_Detect()){
			if(TFCARDOPT_IsMounted()){

				if(!TFCARDOPT_RwMounted()){     //需要TF修复，说明TF属性是只读
					sprintf(ret_status, sTFCARD_STATUS_ABNORMAL);
					ret = EN_TFCORD_STATUS_NO_ABNORMAL;

				}else{
					sprintf(ret_status, sTFCARD_STATUS_OK);
					ret = EN_TFCORD_STATUS_OK;
				}

			}else if(TFCARD_FORMAT_FINISH == g_pTfcardAttr->TFSlot.formating){
				sprintf(ret_status, sTFCARD_STATUS_FORMATED);
				ret = EN_TFCORD_STATUS_FORMATED;
			}else{
				sprintf(ret_status, sTFCARD_STATUS_EXCEPTION);
				ret = EN_TFCORD_STATUS_NO_TFCARD;
			}
		 }else{
				sprintf(ret_status, sTFCARD_STATUS_NOT_FORMAT);
				ret = EN_TFCORD_STATUS_NO_FORMAT;
		 }
		 
	}else{
		sprintf(ret_status, sTFCARD_STATUS_NO_TFCARD);
		ret = EN_TFCORD_STATUS_NO_TFCARD;

	}
	return ret;

}


NK_PChar TFCARDOPT_GetMountPath()
{
	if( NK_Nil == g_pTfcardAttr)
	{
		return NK_Nil;
	}
	return g_pTfcardAttr->TFSlot.fs_path;
}

NK_Int TFCARDOPT_SetFormat(NK_Int bFormat)
{
	if( NK_Nil == g_pTfcardAttr )
	{
		return NK_FALSE;
	}
	g_pTfcardAttr->TFSlot.formating = (NK_TRUE == bFormat) ? TFCARD_FORMAT_READY : TFCARD_FORMAT_INVALID;
	return NK_TRUE;
}

NK_Boolean TFCARDOPT_GetOverWrite()
{
    if( NK_Nil == g_pTfcardAttr ||  NK_Nil == g_pTfcardAttr->EventSet.OnGetOverWrite)
        {
            return NK_FALSE;
        }
        return g_pTfcardAttr->EventSet.OnGetOverWrite();

}

NK_Int TFCARDOPT_RemountRo()
{
    if( NK_Nil == g_pTfcardAttr ||  NK_Nil == g_pTfcardAttr->EventSet.fTFcardOnRemountRo)
        {
            return NK_FALSE;
        }
        return g_pTfcardAttr->EventSet.fTFcardOnRemountRo();

}

NK_Int TFCARDOPT_RepairRo()
{
    if( NK_Nil == g_pTfcardAttr ||  NK_Nil == g_pTfcardAttr->EventSet.fTFcardOnRepairRo)
        {
            return NK_FALSE;
        }
        return g_pTfcardAttr->EventSet.fTFcardOnRepairRo();
}

NK_Int TFCARDOPT_FsckRo()
{
    if( NK_Nil == g_pTfcardAttr ||  NK_Nil == g_pTfcardAttr->EventSet.fTFcardOnFsckRo)
        {
            return NK_FALSE;
        }
        return g_pTfcardAttr->EventSet.fTFcardOnFsckRo();

}


