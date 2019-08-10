
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <time.h>
#include <app_debug.h>

#include "rl_pack.h"
#include "base/ja_process.h"

#define PACK_FILE_EXT "mp4"
#define TEMP_FILE_EXT "tmp"


// Save it for switch between 'T' and 'M'
static char current_rec_type = '\0';

static int MakeFileNameByElmt(
	char * Buff, int Size,
	unsigned long long TimeStamp,
	int DurtTime,
	char Type,
	char * Fix,
	char * Ext)
{
	int StrSize;

	struct timeval tv;
	struct tm * p;

	tv.tv_sec = TimeStamp/1000;
	p = localtime((time_t *)&tv.tv_sec);

	StrSize = snprintf(Buff, Size, "%02d%02d%02d-%05d-%c%s.%s",
                       p->tm_hour, p->tm_min, p->tm_sec,
                       DurtTime, Type, Fix, Ext);

	return StrSize;
}

static int MakeDatePathByElmt(
	char * Buff, int Size,
	unsigned long long TimeStamp)
{
	int StrSize;

	struct timeval tv;
	struct tm * p;

	tv.tv_sec = TimeStamp/1000;
	p = localtime((time_t *)&tv.tv_sec);

	StrSize = snprintf(Buff, Size, "%04d%02d%02d",
		p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);

	return StrSize;
}

static int CheckDir(char * Path)
{
	struct stat path_stat;

	if (0 == stat(Path, &path_stat)) {
		if (0 != S_ISDIR(path_stat.st_mode)) {
			return 0;
		}
	}

	return -1;
}

int RL_PreCheck(RL_Cxt_t * Cxt, RL_Frame_t * Frame)
{
    if (1 == Cxt->Sta.notice_to_pack) {
        Cxt->NeedToSwitch = 1;
        Cxt->Sta.notice_to_pack = 0;
    }
}

int RL_Update(RL_Cxt_t * Cxt, RL_Frame_t * Frame)
{
	Cxt->Sta.CurByteCount += Frame->FrameSize;
	Cxt->Sta.CurTimeCount += abs(Frame->TimeStamp - Cxt->Sta.LstTimeStamp)/1000; //FIXME:

	if(Cxt->Sta.CurByteCount >= Cxt->Env.MaxSizFile) { //Toggle MaxSizFile
		Cxt->NeedToSwitch = 1;
	}
	if(Cxt->Sta.CurTimeCount >= Cxt->Env.MaxDurTime) { //Toggle MaxDurTime
		Cxt->NeedToSwitch = 1;
	}
	if(abs(Frame->TimeStamp - Cxt->Sta.BgnTimeStamp)/1000 >= Cxt->Env.MaxDurTime) { //Toggle MaxDurTime
		Cxt->NeedToSwitch = 1;
	}

	return 0;
}

int RL_CheckSwitch(RL_Cxt_t * Cxt)
{
	return Cxt->NeedToSwitch;
}

int RL_ResetSwitch(RL_Cxt_t * Cxt)
{
	return Cxt->NeedToSwitch = 0;
}

int RL_Rename(RL_Cxt_t * Cxt, char * CurFileName, char * TgtFileName)
{
	char CurPath[MAX_PATH_SIZE];
	char TgtPath[MAX_PATH_SIZE];

	snprintf(CurPath, sizeof(CurPath), "%s/%s/%s", Cxt->Env.BasePath, Cxt->Sta.MidPath, CurFileName);
	snprintf(TgtPath, sizeof(TgtPath), "%s/%s/%s", Cxt->Env.BasePath, Cxt->Sta.MidPath, TgtFileName);

	if(-1 != rename(CurPath, TgtPath)) {
		return -1;
	}

	return 0;
}

int RL_FinPack(RL_Cxt_t * Cxt)
{
	char TgtFile[MAX_PATH_SIZE] = {0};
	int continue_time_sec = 0;

	PackFileEnd(Cxt->PackFile);
	PackFileFree(Cxt->PackFile);

	if(Cxt->Sta.motion_or_time[0] == 'T'){
		continue_time_sec = ((Cxt->Sta.LstTimeStamp-Cxt->Sta.BgnTimeStamp)/1000) > 610 ? \
			610: (Cxt->Sta.LstTimeStamp-Cxt->Sta.BgnTimeStamp)/1000;
	}else{
		continue_time_sec = ((Cxt->Sta.LstTimeStamp-Cxt->Sta.BgnTimeStamp)/1000) > 310 ? \
			310: (Cxt->Sta.LstTimeStamp-Cxt->Sta.BgnTimeStamp)/1000;
	}
	
	MakeFileNameByElmt(TgtFile, sizeof(TgtFile), Cxt->Sta.BgnTimeStamp,
                       continue_time_sec, current_rec_type, "", PACK_FILE_EXT);
	RL_Rename(Cxt, Cxt->Sta.TmpFile, TgtFile); //Rename to Final Name Format
    Cxt->PackFile = NULL;

    current_rec_type = Cxt->Sta.motion_or_time[0];
    memset(&(Cxt->Sta), 0, sizeof(Cxt->Sta));
    Cxt->Sta.motion_or_time[0] = current_rec_type;

	return 0;
}

int RL_Init(RL_Cxt_t * Cxt, char * BasePath)
{
	memset(Cxt, 0, sizeof(*Cxt));

	Cxt->Env.MaxSizFile  = MAX_FILE_SIZE_LIMIT;
	Cxt->Env.MaxDurTime  = MAX_TIME_DURT_LIMIT;
	Cxt->Env.MinDskSpare = MIN_DISK_SIZE_SPARE;

	if(0 != CheckDir(BasePath)) { //BasePath
		if(0 != mkdir(BasePath,
					((S_IRUSR|S_IRGRP|S_IROTH) | (S_IWUSR|S_IWGRP|S_IWOTH)))) {
			return -1;
		}
	}

	snprintf(Cxt->Env.BasePath, sizeof(Cxt->Env.BasePath), "%s", BasePath);

	return 0;
}

int RL_Exit(RL_Cxt_t * Cxt)
{
	if(Cxt->PackFile) {
		RL_FinPack(Cxt);
	}

	return 0;
}

int RL_Write(RL_Cxt_t * Cxt, RL_Frame_t * Frame)
{
    RL_PreCheck(Cxt, Frame);

	if(RL_CheckSwitch(Cxt) && (Frame->FrameType == PACK_FRAME_TYPE_VIDEO_IFRAME)) { //to Create New File Once
		RL_ResetSwitch(Cxt);

        if(Cxt->PackFile) { //Finish Last File
            RL_FinPack(Cxt);
		}
	}

	if((NULL == Cxt->PackFile) && (Frame->FrameType == PACK_FRAME_TYPE_VIDEO_IFRAME)) { //Create New File
		char CurFile[MAX_PATH_SIZE] = {0};
		char CurPath[MAX_PATH_SIZE] = {0};
		char tmpPath[MAX_PATH_SIZE] = {0};
		char year[5] = {0};
        {
            current_rec_type = Cxt->Sta.motion_or_time[0];
			MakeDatePathByElmt(tmpPath, sizeof(tmpPath), Frame->TimeStamp);
			strncpy(year, tmpPath, 4);
			if(atoi(year) < 2016){
				APP_TRACE("video time cannot before 2016 year! ");
				return -1;
			}
			snprintf(CurPath, sizeof(CurPath), "%s/%s", Cxt->Env.BasePath, tmpPath);
			if(0 != CheckDir(CurPath)) { //CurPath
				if(0 != mkdir(CurPath,
							((S_IRUSR|S_IRGRP|S_IROTH) | (S_IWUSR|S_IWGRP|S_IWOTH)))) {
					return -1;
				}
			}

			snprintf(Cxt->Sta.MidPath, sizeof(Cxt->Sta.MidPath), "%s", tmpPath);
		}

		MakeFileNameByElmt(CurFile, sizeof(CurFile), Frame->TimeStamp, 0, current_rec_type, "", TEMP_FILE_EXT);
		snprintf(CurPath, sizeof(CurPath), "%s/%s/%s", Cxt->Env.BasePath, Cxt->Sta.MidPath, CurFile);
		Cxt->PackFile = PackFileNew(CurPath,
			Frame->PicWSize, Frame->PicHSize, Frame->BitRate, Frame->FrameRate, Frame->CodecType);
		if(Cxt->PackFile) {
			Cxt->Sta.BgnTimeStamp = Frame->TimeStamp;
			Cxt->Sta.LstTimeStamp = Frame->TimeStamp;

			snprintf(Cxt->Sta.TmpFile, sizeof(Cxt->Sta.TmpFile), "%s", CurFile);
		}
	}

	if(NULL == Cxt->PackFile) {
		return -1;
	}

	if(Cxt->PackFile) {
#if defined(HI3516E_V1)
        if(0 != PackFileWriteFrame(Cxt->PackFile,
            Frame->FrameBuff,
            Frame->FrameSize,
            Frame->CodecTimeStamp,
            Frame->FrameType))
#else
		if(0 != PackFileWriteFrame(Cxt->PackFile,
			Frame->FrameBuff,
			Frame->FrameSize,
			Frame->TimeStamp,
			Frame->FrameType))
#endif
		{
			return -1;
		}

		Cxt->Sta.LstTimeStamp = Frame->TimeStamp;
	}

	RL_Update(Cxt, Frame);

	return 0;
}

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

static int RL_GetOldestDir(RL_Cxt_t * Cxt, char * Path, int Size)
{
    struct dirent ** dirList;

    int  lstNum = 0;
	int  gotDir = 0;

    int ret;
    int i;

    lstNum = scandir(Cxt->Env.BasePath, &dirList, NULL, alphasort);
    if(lstNum < 0) {
        return -1;
    }

    gotDir = 0;
    for(i = 0; i < lstNum; i ++) {
		if(0 != gotDir) {
			free(dirList[i]);
			continue;
		}

        if((0 == strcmp(dirList[i]->d_name, "."))
        || (0 == strcmp(dirList[i]->d_name, ".."))) {
            free(dirList[i]);
            continue;
        }

        ret = snprintf(Path, Size, "%s", dirList[i]->d_name);
        if((ret > 0) && (ret < Size)) {
            gotDir = 1;
        }

        free(dirList[i]);
    }

    free(dirList);

    if(0 == gotDir) {
        return -1;
    }

	return 0;
}

// return -2,Cannot Found Any Directory in Cxt->Env.BasePath
int RL_DelOldestFiles(RL_Cxt_t * Cxt, int Num)
{
	char tmpPath[MAX_PATH_SIZE];
	char dirPath[MAX_PATH_SIZE];
	char cmdCall[MAX_PATH_SIZE];

	if(0 != RL_GetOldestDir(Cxt, tmpPath, sizeof(tmpPath))) {
		printf("\n--%s-Cannot Found Any Directory in %s--\n", __FUNCTION__, Cxt->Env.BasePath);
		return -2;
	}

	if(0 == strlen(tmpPath)) {
		printf("\n--%s-Directory Path Size 0--\n", __FUNCTION__);
		return -1;
	}

	snprintf(dirPath, sizeof(dirPath), "%s/%s", Cxt->Env.BasePath, tmpPath);
	if(0 != CheckDir(dirPath)) {
		snprintf(cmdCall, sizeof(cmdCall), "rm %s -rf", dirPath); //Delete Unknown Files
		NK_SYSTEM(cmdCall);
		printf("\n--%s-Non-Directory Deleted %s--\n", __FUNCTION__, dirPath);
		return -1;
	}

	{
		struct dirent ** fileList;
		struct stat path_stat;
		int  lstNum = 0;
		int  DelCount = 0;
		int  i;

		lstNum = scandir(dirPath, &fileList, NULL, alphasort);
		if(lstNum < 0) {
			printf("Failed to scandir %s. Errno: %d", dirPath, errno);
			return -1;
		}

		for(i = 0; i < lstNum; i ++) {
			if(DelCount >= Num) { //Delete Max Num Files in One Time
				free(fileList[i]);
				continue;
			}

			if((0 == strcmp(fileList[i]->d_name, "."))
			|| (0 == strcmp(fileList[i]->d_name, ".."))) {
				free(fileList[i]);
				continue;
			}

			snprintf(cmdCall, sizeof(cmdCall), "rm %s/%s -rf", dirPath, fileList[i]->d_name); //Delete Unknown Files
			NK_SYSTEM(cmdCall);
			printf("\n--%s-Non-Directory Deleted %s/%s--\n", __FUNCTION__, dirPath, fileList[i]->d_name);
			DelCount += 1;

			free(fileList[i]);
		}

        {
            int deleteNum;
            deleteNum = Num < 1 ? 1 : Num;

            // Delete directory if files less than deleteNum
            if (lstNum <= (2 + deleteNum)) {
                snprintf(cmdCall, sizeof(cmdCall), "rm %s -rf", dirPath);
                NK_SYSTEM(cmdCall);
            }
        }

		free(fileList);
	}

	return 0;
}
