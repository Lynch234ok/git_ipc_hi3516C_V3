#ifndef __RL_PACK_H__
#define __RL_PACK_H__

#include "packfile.h"

#define RL_PATH "/media/tf/rec"

typedef struct {
	unsigned char * FrameBuff;
	unsigned int    FrameSize;
	int             FrameType;

	unsigned int    FrameRate;
	unsigned int    BitRate;

	int             PicWSize;
	int             PicHSize;

	unsigned long long TimeStamp;
#if defined(HI3516E_V1)
	unsigned long long CodecTimeStamp;
#endif

    int             CodecType;
} RL_Frame_t;

#define MIN_DISK_SIZE_SPARE (256*1024*1024) //256MByte

#if defined(HI3516E_V1)
#define MAX_FILE_SIZE_LIMIT (24*1024*1024)  // 24MByte
#define MAX_TIME_DURT_LIMIT (2*60)         // 2Minutes
#else
#define MAX_FILE_SIZE_LIMIT (32*1024*1024)  //32MByte
#define MAX_TIME_DURT_LIMIT (5*60)         //5Minutes
#endif

#define MAX_PATH_SIZE       (128)

typedef struct {
	char BasePath[MAX_PATH_SIZE];

	unsigned int MaxDurTime; //Max Duration Time to Save into One File;
	unsigned int MaxSizFile; //Max File Size Limit to Finish Packing One File;
	unsigned int MinDskSpare; //Min Disk Spare Space in Bytes to Delete Old Records;
	
} RL_Env_Cxt;

typedef struct {
	char TmpFile[MAX_PATH_SIZE];
	char MidPath[MAX_PATH_SIZE];

	unsigned int CurTimeCount;
	unsigned int CurByteCount;
	int notice_to_pack;  //Used to notice to pack right now.
						 //1, pack for now; 0, not required to pack for now,
	char motion_or_time[2];
	unsigned long long LstTimeStamp;  //Lst AV Frame TimeStamp;
	unsigned long long BgnTimeStamp;  //First I Frame TimeStamp;
} RL_Sta_Cxt;

typedef struct {
	RL_Env_Cxt Env;
	RL_Sta_Cxt Sta;

	PackCxt_t * PackFile;

	int  NeedToSwitch;
} RL_Cxt_t;

int RL_Init(RL_Cxt_t * Cxt, char * BasePath);
int RL_Exit(RL_Cxt_t * Cxt);
int RL_Write(RL_Cxt_t * Cxt, RL_Frame_t * Frame);
int RL_DelOldestFiles(RL_Cxt_t * Cxt, int Num);

#endif //__RL_PACK_H__
