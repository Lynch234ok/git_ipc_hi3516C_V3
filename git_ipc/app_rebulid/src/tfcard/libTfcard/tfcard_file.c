
#include "tfcard_export.h"
#include <dirent.h>


/////秒转换成本地时间
NK_Void Second2time(NK_Size nlocalSec, struct tm* plocaltime)
{
	localtime_r((time_t *)(&nlocalSec), plocaltime);
}

NK_Int TFFILE_GetDayDir(NK_Size nSecTime,NK_PChar pstrDayDir)
{
	struct tm tv_start;
	if(NK_Nil == pstrDayDir)
	{
		return NK_FALSE;
	}
	memset(&tv_start, 0 , sizeof(struct tm));
	localtime_r((time_t *)(&nSecTime), &tv_start);
	sprintf(pstrDayDir, "%4d%2d%2d", (1900+tv_start.tm_year),(1 + tv_start.tm_mon), tv_start.tm_mday);
	return NK_TRUE;
}

NK_Int TFFILE_HasFile(NK_PChar path)
{
	NK_Int hasFile = NK_FALSE;
	if(NK_Nil == path || strlen(path) < 0)
	{
		return NK_FALSE;
	}
	hasFile = (-1 != access(path, F_OK));
	return (hasFile ? NK_TRUE : NK_FALSE);
}

NK_Size TFFILE_GetLocaltime()
{
	NK_Size utc;
	struct tm setTm;
	utc = time(NULL);
	localtime_r((time_t *)(&utc), &setTm);
	return (setTm.tm_hour * 3600 + setTm.tm_min * 60 + setTm.tm_sec);
}

static inline int REMOVE_FILE(const char *filePath)
{
	if(0 == unlink(filePath)){
		if(0 == remove(filePath)){
			return 0;
		}
	}
	return -1;
}

NK_Int TFFILE_RemoveFile(NK_PChar pFilePath)
{
	if(NK_Nil == pFilePath)
	{
		return NK_FALSE;
	}
	if(0 > access(pFilePath, F_OK))
	{
		//NK_ERROR("access file[%s] err=0x%x \n", pFilePath, errno);
		return NK_FALSE;
	}
	REMOVE_FILE(pFilePath);
	printf("[%s]unlink file %s\n",__func__,pFilePath);
	return NK_TRUE;
}

NK_Int TFFILE_RemoveDir(NK_PChar pDirPath)
{
	DIR *dirPtr = NULL;
	struct dirent *entry;
	NK_Char path[64] = {0};
	NK_Char fileName[32] = {0};
	if(NK_Nil == (dirPtr = opendir(pDirPath)))
	{
	//	NK_ERROR("open dir error,path : %s",pDirPath);
		return NK_FALSE;
	}
	time_t pre_time = 0;
	struct stat buf;
	int result=-1;

	while ((entry = readdir(dirPtr)) != NULL)
	{
		if((0 == strcmp(entry->d_name,".")) ||
			(0 == strcmp(entry->d_name,"..")))
		{
			//NK_INFO("entry->d_name=%s \n", entry->d_name);
			continue;
		}

		if(entry->d_name)
		{
			memset(path, 0, sizeof(path));
			sprintf(path, "%s/%s", pDirPath, entry->d_name);
			if(0 != (result = stat(path, &buf)))
			{
				continue;
			}
			if(S_ISDIR(buf.st_mode))
			{
				if(NK_FALSE == TFFILE_RemoveDir(path)) //如果是目录文件，递归删除
				{
					closedir(dirPtr);
					return NK_FALSE;
				}
				rmdir(path); ////删除空子目录文件
				continue;
			}
			if(S_ISREG(buf.st_mode))
			{
				unlink(path);////如果是普通文件，则unlink
				continue;
			}
			//NK_ERROR("rm_dir:st_mode %s error!",path);
		}
	}

	closedir(dirPtr);
	rmdir(pDirPath); //// 删除空目录
//	NK_INFO("remove [%s] ok!\n", pDirPath);
	return NK_TRUE;
}

NK_Int TFFILE_PareFileName(NK_PChar pstrFileName, NK_Size* nSecStart ,NK_Size* nUsedTime, NK_Int* nFileMode)
{
	////HHMMSS-00005-T.mp4 解释文件名字符串
	NK_PChar pStr = NULL;
	NK_Int i = 0;
	NK_Char chrMode = 0;
	NK_Char strtmp[4] = {0};
	if(NK_Nil == pstrFileName)
	{
		return NK_FALSE;
	}
	if(strlen("HHMMSS-00005-T.TS") != strlen(pstrFileName))
	{
		return NK_FALSE;
	}
	pStr = strtok(pstrFileName, "-");
	if(strlen("HHMMSS") != strlen(pStr))
	{
		return NK_FALSE;
	}
	NK_Int nSecDay = 0;
	memset(&strtmp, 0 , sizeof(strtmp));
	strtmp[0] = pStr[0];
	strtmp[1] = pStr[1];
	nSecDay += atoi(strtmp) * 60 * 60;

	memset(&strtmp, 0 , sizeof(strtmp));
	strtmp[0] = pStr[2];
	strtmp[1] = pStr[3];
	nSecDay += atoi(strtmp) * 60;

	memset(&strtmp, 0 , sizeof(strtmp));
	strtmp[0] = pStr[4];
	strtmp[1] = pStr[5];
	nSecDay += atoi(strtmp);
	while((pStr = strtok(NULL, "-")))
	{
		i++;
		if(1 == i) *nUsedTime = atoi(pStr);
		if(2 == i) chrMode = pStr[0]; /////只取T字符
	}
	*nFileMode = chrMode;
	*nSecStart = nSecDay;
	return NK_TRUE;
}



NK_Int TFFILE_MakeDir(NK_PChar sPathName)
{
	NK_Char DirName[256] = {0};
	NK_Int i, len;
	strcpy(DirName, sPathName);
	len = strlen(DirName);
	if('/' != DirName[len-1])
	{
	    strcat(DirName, "/");
	    len++;
	}

	for(i=1; i<len; i++)
	{
	    if('/' == DirName[i])
	    {
	        DirName[i] = '\0';
	        if(access(DirName, F_OK) != 0)
	        {
	            if(mkdir(DirName, 0777) == -1)
	            {
	             //   NK_ERROR("mkdir() failed!");
	                return NK_FALSE;
	            }
	        }
	        DirName[i] = '/';
	     }
	}
	return NK_TRUE;
}

////获取最旧的文件目录全路径
NK_Int  TFFILE_GetOldDir(NK_PChar pFilePath, NK_PChar pOldDirPath,NK_PChar pOldDirName)
{
	struct dirent **entry_list;
	NK_Int count;
	struct stat buf;
	NK_Int result = 0;
	NK_Int i = 0, j = 0;
	NK_Int nRmNumber = 0;

	NK_Char strRmFile[128] = {0};
	NK_Char strCmpPath[128] = {0};
	if(NK_Nil == pFilePath)
	{
		return NK_FALSE;
	}

	count = scandir(pFilePath, &entry_list, 0, alphasort);
	if (count < 0)
	{
//		NK_ERROR("scandir err[%d] ERR!\n", count);
		return NK_FALSE;
	}

    if((count == 2 && (0 == strcmp(entry_list[0]->d_name,".")) && (0 == strcmp(entry_list[1]->d_name,".."))) || count == 0)
    {
        if(NK_Nil != entry_list)
        {
            for(i = 0; i < count; i++)
            {
                free(entry_list[i]);
            }
            free(entry_list);
        }
        return NK_FALSE;
    }

	for (i = 0; i < count; i++)
	{
        struct dirent *entry;
        entry = entry_list[i];

		memset(strRmFile, 0 , sizeof(strRmFile));
		sprintf(strRmFile, "%s/%s",pFilePath,entry->d_name);
		memset(&buf, 0 , sizeof(struct stat));
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
		if(S_ISDIR(buf.st_mode)) ////文件类型是
		{
			sprintf(pOldDirPath,"%s",strRmFile);
			sprintf(pOldDirName, "%s", entry->d_name);
            for(j = i; j < count; j++)
            {
                free(entry_list[j]);
            }
			break;
		}
       free(entry);
    }
    free(entry_list);

    //检查时间分段文件夹
    count = scandir(pOldDirPath, &entry_list, 0, alphasort);
    if(count < 0)
    {
       return NK_TRUE;
    }

    if((count == 2 && (0 == strcmp(entry_list[0]->d_name,".")) && (0 == strcmp(entry_list[1]->d_name,".."))) || count == 0)
    {
        if(NK_Nil != entry_list)
        {
            for(i = 0; i < count; i++)
            {
                free(entry_list[i]);
            }
            free(entry_list);
        }
        return NK_TRUE;
    }

    for (i = 0; i < count; i++)
    {
        struct dirent *entry;
        entry = entry_list[i];

        memset(strRmFile, 0 , sizeof(strRmFile));
        sprintf(strRmFile, "%s/%s",pOldDirPath,entry->d_name);
        memset(&buf, 0 , sizeof(struct stat));
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
        if(S_ISDIR(buf.st_mode)) ////文件类型是
        {
            printf("get old hour:%s\n", strRmFile);
            sprintf(pOldDirPath,"%s",strRmFile);
            sprintf(pOldDirName, "%s/%s", strCmpPath, entry->d_name);
            for(j = i; j < count; j++)
            {
                free(entry_list[j]);
            }
            break;
        }
        free(entry);
    }
    free(entry_list);
	return NK_TRUE;
}


NK_Int  TFFILE_RemoveOldfile(NK_PChar pFilePath, NK_Int nTotal)
{
	struct dirent **entry_list;
	NK_Int count;
	struct stat buf;
	NK_Int result = 0;
	NK_Int i = 0, j = 0;
	NK_Int nRmNumber = 0;

	NK_Char strRmFile[128] = {0};
	if(NK_Nil == pFilePath)
	{
		return NK_FALSE;
	}

	count = scandir(pFilePath, &entry_list, 0, alphasort);
	if (count < 0)
	{
//		NK_ERROR("scandir err[%d] ERR!\n", count);
		return NK_FALSE;
	}

    if((count == 2 && (0 == strcmp(entry_list[0]->d_name,".")) && (0 == strcmp(entry_list[1]->d_name,".."))) || count == 0)
    {
        if(NK_Nil != entry_list)
        {
            for(i = 0; i < count; i++)
            {
                free(entry_list[i]);
            }
            free(entry_list);
        }
//        NK_INFO(" rm empty dir %s\n", pFilePath);
        if(rmdir(pFilePath) < 0)
        {
            return NK_FALSE;
        }
        
        return NK_TRUE;
    }

	for (i = 0; i < count; i++)
	{
        struct dirent *entry;
        entry = entry_list[i];

		memset(strRmFile, 0 , sizeof(strRmFile));
		sprintf(strRmFile, "%s/%s",pFilePath,entry->d_name);
		memset(&buf, 0 , sizeof(struct stat));
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
			if(nRmNumber < nTotal)
			{
			
				TFFILE_RemoveFile(strRmFile); ////删除文件
			}
			else
			{
                for(j = i; j < count; j++)
                {
                    free(entry_list[j]);
                }
				break;
			}

			nRmNumber ++;
//			NK_INFO(" rm %s is file \n", strRmFile);
		}
		if(S_ISDIR(buf.st_mode)) ////文件类型是
		{
//			NK_INFO("%s is dir \n", strRmFile);
		}
       free(entry);
    }
    free(entry_list);
    sync();
	return NK_TRUE;
}





