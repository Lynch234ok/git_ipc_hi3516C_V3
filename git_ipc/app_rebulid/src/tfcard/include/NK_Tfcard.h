

#ifndef _NK_TFCARD_H_
#define _NK_TFCARD_H_
#include "LibTfcard.h"

#ifdef __cplusplus
extern "C"{
#endif

#define TFCARD_MMCBLK0_PATH "/dev/mmcblk0"
#define TFCARD_MMCBLK0P1_PATH "/dev/mmcblk0p1"
#define TFCARD_MOUNT_PATH "/media/tf"


///////////////////////////////record
typedef enum tagTFRECORDTYPE_
{
	EN_NK_RECORD_TYPE_NONE   = (0),
	EN_NK_RECORD_TYPE_TIMER  = (1<<0),////定时录像[T]
	EN_NK_RECORD_TYPE_MOTION = (1<<1),////移动录像[M]
	EN_NK_RECORD_TYPE_ALARM  = (1<<2),////报警录像[A]
	EN_NK_RECORD_TYPE_MANUAL = (1<<3),////手动录像[R]
}enTFRECORDTYPE;

typedef enum enTFCMDTYPE_
{
	EN_TFCARD_RECORD_START = (0x10001),////TF打开录像文件:stTFPLAYATTR
	EN_TFCARD_RECORD_STOP,////TF关闭录像文件:stTFPLAYATTR
	EN_TFCARD_RECORD_TYPE,////TF录像类型:stTFPLAYATTR
	EN_TFCARD_RECORD_SCHEDULE,////TF录像日程安排:		stTIMESEGMENT

	EN_TFCARD_PLAY_SEARCH,////TF录像文件搜索
	EN_TFCARD_PLAY_START_FILE,////TF打开录像文件
	EN_TFCARD_PLAY_STOP_FILE,////TF打开录像文件
	EN_TFCARD_PLAY_START_TIME,////TF打开按条件查找的文件
	EN_TFCARD_PLAY_STOP_TIME,////TF打开按条件查找的文件

	EN_TFCARD_PLAY_READ_FILE,////TF打开按条件查找的文件
}enTFCMDTYPE;

typedef enum enNKTFCARDSTATUS_
{
	 EN_NK_TFCARD_STATUS_OK	= (0x0),////TF正常使用
	 EN_NK_TFCARD_STATUS_EXCEPTION,////TF检测没有插入
	 EN_NK_TFCARD_STATUS_NO_MOUNT,////TF没有挂载
	 EN_NK_TFCARD_STATUS_NO_TFCARD,////TF没有卡
	 EN_NK_TFCARD_STATUS_NO_FORMAT,////TF没有格式化
	 EN_NK_TFCORD_STATUS_NO_ABNORMAL,////TF异常  ，包括无分区/只读

	 EN_NK_TFCARD_STATUS_FORMATFAIL,////TF格式化失败
	 EN_NK_TFCARD_STATUS_FORMATTING,////TF正
	 EN_NK_TFCARD_STATUS_FORMATED,////TF格式化完成
	 EN_NK_TFCARD_STATUS_CNT,
}enNKTFCARDSTATUS;


NK_Int NK_TFCARD_Init(NK_PChar pstrDevpath, NK_PChar pstrMountPath);
NK_Int NK_TFCARD_Exit();
NK_Size NK_TFCARD_GetCapacity();
NK_Size NK_TFCARD_GetFreeSpace();
NK_Int NK_TFCARD_Format();
NK_Int NK_TFCARD_GetStatus();
NK_Int NK_TFCARD_GetTsRecPath_Of_Hex(NK_PChar pReFile);


NK_Int NK_TFRECORD_Start(NK_Int nChnId, NK_Int nRecType);
NK_Int NK_TFRECORD_Stop(NK_Int nChnId);
NK_Int NK_TFRCORD_SetParam();
NK_Boolean NK_TFRCORD_MdSetting();
NK_Int NK_TFRCORD_RecordFile_Get(NK_PChar pReFile);


NK_PVoid NK_TFPLAY_Open(NK_PChar pstrFile);
NK_PVoid NK_TFPLAY_OpenEx(NK_Int nChn,NK_UInt32 nStartUtc,	NK_UInt32 nEndUtc,	NK_Int enMode, NK_PVoid pUserCtx);
NK_Int NK_TFPLAY_Close(NK_PVoid hPlay);
NK_Int NK_TFPLAY_Read(NK_PVoid hPlay, NK_Int enMode, pstTFMEDIAHEAD pMediaAttr, NK_PVoid *pMediaAddr);
NK_Boolean NK_TFPLAY_ReadSeek(NK_PVoid hPlayBack, pstTFMEDIAHEAD pMediaAttr,NK_PVoid *pFrameData,NK_UInt32 nSeekUtc);

typedef struct tagNKTFPLAYFILE{
	NK_Int  nChn;////录像文件通道
	NK_UInt32 nStartUtc;////开始UTC时间
	NK_UInt32 nEndUtc;////结束UTC时间
	NK_UInt32 nUsedTime;///录像文件使用总时长(秒)
	NK_Int  nFileType;/////当前录像文件类型(移动/定时/手动)
	NK_Int	nFileEncode;////编码类型 H264/H265/JPEG
	NK_Int	nFileSize;////录像文件总大小
	NK_Char strFile[64];////文件名字:/mnt/video/01/20180505/HHMMSS_12345_T.mp4
}stNKTFPLAYFILE, *pstNKTFPLAYFILE;


typedef struct tagNKTFPLAYTIME{
	NK_Int  nChn;////录像文件通道
	NK_UInt32  nStartUtc;////世界时间码，此类新变量表示自公元 1970 年 1 月 1 日 GMT 时区经过的秒数
	NK_UInt32  nEndUtc;
	NK_UInt32  nUsedTime;///录像文件使用总时间
	NK_Int  nFileType;/////当前录像文件类型(移动/定时/手动)enRECORDTYPE
	NK_Int	nFileEncode;////编码类型 H264/H265/JPEG
}stNKTFPLAYTIME, *pstNKTFPLAYTIME;

typedef NK_Int (*fTFPLAYSEARCHCB)(NK_PVoid pUserCtx, NK_Int bFileType, NK_Int nNumber, NK_Int nRecordTotal, NK_PVoid lParam);
NK_Int NK_TFPLAY_SearchFile(NK_Int nChn,
								 NK_UInt32 nStartUtc,
								 NK_UInt32 nEndUtc,
								 NK_Int enMode,
								 NK_PVoid pUserCtx,
								 fTFPLAYSEARCHCB pfuch);

NK_Int NK_TFPLAY_SearchTime(NK_Int nChn,
								 NK_UInt32 nStartUtc,
								 NK_UInt32 nEndUtc,
								 NK_Int enMode,
								 NK_PVoid pUserCtx,
								 fTFPLAYSEARCHCB pfuch);

NK_PVoid NK_TFPLAY_CreateHistory(NK_Int nChn,
										   NK_UInt32 nStartUtc,
										   NK_UInt32 nEndUtc,
										   NK_Int enMode,
										   NK_PVoid pUserCtx);
NK_Int NK_TFPLAY_FreeHistory(NK_PVoid hHistory);

NK_Int NK_TFPLAY_GetHistoryLen(NK_PVoid hHistory);
NK_Int NK_TFPLAY_GetHistoryFile(NK_PVoid hHistory, NK_Int nIndex, NK_PVoid *pstFile);


#ifdef __cplusplus
}
#endif

#endif



