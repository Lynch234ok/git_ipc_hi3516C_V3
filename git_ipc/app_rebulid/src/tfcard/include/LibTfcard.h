
#ifndef _LIB_TFCARD_H_
#define _LIB_TFCARD_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "tfcard_Types.h"
#include <NkUtils/types.h>


/////////////////////////////////////////////////////////////

/**
 * 录像帧 帧头结构
 */
#define TFCARD_VCODEC_H264 (96)
#define TFCARD_VCODEC_H265 (97)
#define TFCARD_ACODEC_G711A (8)
#define TFCARD_ACODEC_G711U (9)
#define TFCARD_ACODEC_AAC (10)
typedef struct tagTFMEDIAHEAD
{
	NK_Int codec;
	NK_UInt64 coderStamp_us;
	NK_UInt64 sysTime_us;
	NK_Size dataSize;
	union {
		/**
		 * 音频属性。
		 */
		struct {
			NK_Int sampleRate, sampleWidth, samplePacket;
			NK_Float compressionRatio;
		};
		/**
		 * 视频属性。
		 */
		struct {
			NK_Int isKeyFrame, fps, width, height;
		};
	};
	NK_Int reserve;
}stTFMEDIAHEAD,*pstTFMEDIAHEAD;

typedef struct tagTFSDKEVENT {
	//检测 TF 物理上是否存在事件
	NK_Int (*OnExistTF)();
 	//检测 TF 是否存在事件
	NK_Int (*OnDetectTF)();
 	//装载 TF 卡文件系统目录
	NK_Int (*OnMountTF)(NK_PChar mount_path);
 	//卸载 TF 卡文件系统目录事件
	NK_Int (*OnUmountTF)(NK_PChar mount_path);
	//清除 TF 卡挂载目录事件
	NK_Int (*OnCleanTF)(NK_PChar mount_path);
	//获取 TF 卡总容量
	NK_Size (*OnGetCapacity)(NK_PChar mount_path);
	//获取 TF 卡可用量
	NK_Size (*OnGetFreeSpace)(NK_PChar mount_path);
    //TF 卡格式化事件
	NK_Int (*OnFormat)();
    //TF 获取状态事件
	//NK_Int (*OnGetStauts)(NK_PChar strTfDev, NK_Int nTfStauts, NK_Int nTfError);

    // TF 获取覆盖策略
    NK_Boolean (*OnGetOverWrite)();
	//TF格式化完成重新启动TF录像接口
	NK_Int (*fTFcardOnAfterFormat)(NK_Boolean status);
	NK_Int (*fTFcardOnRemountRo)();
	NK_Int (*fTFcardOnRepairRo)();
	NK_Int (*fTFcardOnFsckRo)();
}stTFSDKEVENT, *pstTFSDKEVENT;

typedef struct tagTFPARAM {
	NK_Int bRecordOverLoad; //// 0:1
	NK_Char strDevpath[64];  ////设备路径
	NK_Char strMountPath[64];////挂载路径
}stTFPARAM, *pstTFPARAM;

typedef enum enTFCARDTYPE_
{
	EN_TFCARD_OPT_MOUNT = (0x1001),////TF mount操作
	EN_TFCARD_OPT_UMOUNT,////TF umount操作
	EN_TFCARD_OPT_FORMAT,////TF 格式化操作:对应结构体：NK_TRUE:NK_FALSE
}enTFCARDOPT;

typedef struct tagTFRECEVENT {
	////创建读取器
	NK_Int (*AddReader)(NK_Int nReaderChn, NK_Int enRecType, NK_PVoid *hReader);
	////删除读取器
	NK_Int (*DelReader)(NK_PVoid *hReader);
	////锁住读取器
	NK_Int (*LockReader)(NK_PVoid *hReader);
	////解锁读取器
	NK_Int (*UlockReader)(NK_PVoid *hReader);
	////读取缓冲区媒体帧（包括音频/视频/JPEG）
	NK_Int (*ReadMediaFrame)(NK_PVoid *hReader, pstTFMEDIAHEAD pMediaBuf, NK_PVoid* pFrameData);
        ///判断缓冲区是否有数据
    NK_Int (*IsEmpty)(NK_PVoid *hReader);
    ////同步读取器
    NK_Int (*SyncReader)(NK_PVoid *hReader, NK_Int sec);
    // 请求I帧
    NK_Int (*requestStreamKeyframe)(NK_Void);

/********************
* 报警处理事件
********************/
    NK_Boolean (*GetMdStatus)(NK_Int nRecChn);

    NK_Int (*SetRecStatus)(NK_Int nRecChn, NK_Int nRecType);
}stTFRECEVENT, *pstTFRECEVENT;

typedef struct tagTFRECPARAM {
	NK_Int nChnId;///
	NK_Int enRecType;////录像类型
 }stTFRECPARAM, *pstTFRECPARAM;

typedef struct tagSECTIONSLOT_
{
    NK_Int          enable;
    NK_Int          nScheduleType;////周日/周一/周二/周三/周四/周五/周六/
    NK_Int          nRecType;////按位表示，参照enRECORDTYPE
    NK_Int64        chnMask; //对应通道，按位表示
    NK_UInt32       nStartTime; //// 开始时间,记录当天的时分秒总秒数
    NK_UInt32       nStopTime;  //// 停止时间,记录当天的时分秒总秒数
} stSECTIONSLOT,*pstSECTIONSLOT;

typedef struct tagRECSCHEDULE_ ////日程安排
{
	stSECTIONSLOT stSectionSlot[8];//总共最多支持8段
}stRECSCHEDULE,*pstRECSCHEDULE;

 typedef enum enSCHDULETYPE_
 {
	 EN_SCHEDULE_TYPE_SUNDAY	 = (1<<0),////周日
	 EN_SCHEDULE_TYPE_MONDAY	 = (1<<1),////周一
	 EN_SCHEDULE_TYPE_TUESDAY	 = (1<<2),////周二
	 EN_SCHEDULE_TYPE_WEDNESDAY = (1<<3),////周三
	 EN_SCHEDULE_TYPE_THURSSDAY = (1<<4),////周四
	 EN_SCHEDULE_TYPE_FRIDAY	 = (1<<5),////周五
	 EN_SCHEDULE_TYPE_SATURDAY  = (1<<6),////周六
 }enSCHDULETYPE;

 typedef enum enTFRECOPT_
 {
	 EN_TFRECORD_OPT_RECTYPE = (0x1001), ////结构体：NK_Int
	 EN_TFRECORD_OPT_SCHEDULE,
	 EN_TFRECORD_OPT_RECSTART,
	 EN_TFRECORD_OPT_RECSTOP,
	 EN_TFRECORD_OPT_MANUALREC, ///结构体：NK_Int
 }enTFRECOPT;

typedef enum enRECORDTYPE_
{
	 EN_RECORD_TYPE_NONE	= (0x0),
	 EN_RECORD_TYPE_TIMER	= (1<<0),////定时录像[T]
	 EN_RECORD_TYPE_MOTION  = (1<<1),////移动录像[M]
	 EN_RECORD_TYPE_ALARM	= (1<<2),////报警录像[A]
	 EN_RECORD_TYPE_MANUAL  = (1<<3),////手动录像[R]
}enRECORDTYPE;

typedef struct tagTFDATE{
	NK_Int 	nYear;
	NK_Int 	nMouth;
	NK_Int 	nDay;
}stTFDATE,*pstTFDATE;

typedef struct tagTFTIME{
	NK_Int 	nHour;
	NK_Int 	nMinute;
	NK_Int 	nSecond;
}stTFTIME,*pstTFTIME;

typedef enum enTFCORDSTATUS_
{
	 EN_TFCORD_STATUS_OK	= (0x0),////TF正常使用
	 EN_TFCORD_STATUS_EXCEPTION,////TF检测没有插入
	 EN_TFCORD_STATUS_NO_MOUNT,////TF没有挂载
	 EN_TFCORD_STATUS_NO_TFCARD,////TF没有卡
	 EN_TFCORD_STATUS_NO_FORMAT,////TF没有格式化
	 EN_TFCORD_STATUS_NO_ABNORMAL,////TF异常  ，包括无分区/只读

	 EN_TFCORD_STATUS_FORMATFAIL,////TF格式化失败
	 EN_TFCORD_STATUS_FORMATTING,////TF正
	 EN_TFCORD_STATUS_FORMATED,////TF格式化完成
	 EN_TFCORD_STATUS_CNT,
}enTFCORDSTATUS;



NK_Int TFSDK_CARD_Init(pstTFPARAM ptfParam,pstTFSDKEVENT pstEvent);
NK_Int TFSDK_CARD_Exit();
NK_Int TFSDK_CARD_SetParam(enTFCARDOPT enType, NK_PVoid lParam, NK_PVoid wParam);
NK_Size TFSDK_CARD_GetCapacity();
NK_Size TFSDK_CARD_GetFreeSpace();
NK_Boolean TFSDK_CARD_IsExist();
NK_Boolean TFSDK_CARD_IsMounted();
NK_Int TFSDK_CARD_Format();
NK_Int TFSDK_CARD_Repair();
NK_Int TFSDK_CARD_GetTsRecPathOfHex(NK_PChar pReFile);

enTFCORDSTATUS TFSDK_CARD_GetStatus();




typedef void* HTFRECORD;

HTFRECORD TFSDK_RECORD_Start(pstTFRECPARAM pstRecParam, pstTFRECEVENT pstRecEvent);
NK_Int TFSDK_RECORD_Stop(HTFRECORD hRecord);
NK_Int TFSDK_RECORD_SetParam(HTFRECORD hRecord, enTFRECOPT enOpt, NK_PVoid lParam, NK_PVoid wParam);
NK_Int TFSDK_RECORD_RecordFile_Get(HTFRECORD hRecord,NK_PChar recordFile);

typedef void* HTFPLAY;

typedef enum tagTFSDKPLAYERR_
{
	EN_TFSDK_PLAY_READ_ERR_SUCCESS 	= (0),
	EN_TFSDK_PLAY_READ_ERR_FAIL = (-1),
	EN_TFSDK_PLAY_READ_ERR_NEXT = (-2),///准备读下一文件
	EN_TFSDK_PLAY_READ_ERR_FILE = (-3),///读取文件失败
	EN_TFSDK_PLAY_READ_ERR_MODE = (-4),//读文件模式失败
	EN_TFSDK_PLAY_READ_ERR_EOF	= (-5),///读到文件结尾
	EN_TFSDK_PLAY_READ_ERR_MALLOC = (-6),///创建内存失败
}enTFSDKPLAYERR;

typedef struct tagTFPLAYSEARCH_
{
	NK_Int  nChn; /////
	NK_UInt32 nStartUtc;////查找当天开始时间
	NK_UInt32 nEndUtc;////查找当天结束时间
	NK_Int  nType;/////用掩码处理
	NK_PVoid pUserCtx;////用户上下文
}stTFPLAYSEARCH, *pstTFPLAYSEARCH;

typedef struct tagTFPLAYFILE{
	NK_Int  nChn;////录像文件通道
	NK_UInt32 nStartUtc;
	NK_UInt32 nEndUtc;
	NK_UInt32 nUsedTime;///录像文件使用总时间
	NK_Int  nFileType;/////当前录像文件类型(移动/定时/手动)enRECORDTYPE
	NK_Int	nFileEncode;////编码类型 H264/H265/JPEG
	NK_Int	nFileSize;////录像文件总大小
	NK_Char strFile[64];////文件名字 /mnt/video/00/20180505/HHMMSS_12345_T.mp4
}stTFPLAYFILE, *pstTFPLAYFILE;

typedef struct tagTFPLAYTIME{
	NK_Int  nChn;////录像文件通道
	NK_UInt32  nStartUtc;////世界时间码，此类新变量表示自公元 1970 年 1 月 1 日 GMT 时区经过的秒数
	NK_UInt32  nEndUtc;
	NK_UInt32  nUsedTime;///录像文件使用总时间
	NK_Int  nFileType;/////当前录像文件类型(移动/定时/手动)enRECORDTYPE
	NK_Int	nFileEncode;////编码类型 H264/H265/JPEG
}stTFPLAYTIME, *pstTFPLAYTIME;

HTFPLAY TFSDK_PLAY_Open(NK_Int bPlayFile, NK_PVoid lParam);
NK_Int TFSDK_PLAY_Close(HTFPLAY hPlay);
NK_Int TFSDK_PLAY_Read(HTFPLAY hPlay, NK_Int nMode, pstTFMEDIAHEAD pFrameHead, NK_PVoid* pFrameData);

typedef NK_Int (*fPLAYSEARCHCB)(NK_PVoid pUserCtx, NK_Int bFileType, NK_Int nNumber,NK_Int nRecordTotal, NK_PVoid lParam);
NK_Int TFSDK_PLAY_Search(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB pfSearchCB, NK_Int maxLimitCnt);
NK_Int TFSDK_PLAY_SearchEx(pstTFPLAYSEARCH pstPlaySearch, fPLAYSEARCHCB pfSearchCB);

NK_PVoid TFSDK_PLAY_CreateHistory(pstTFPLAYSEARCH pstPlaySearch);
NK_Int TFSDK_PLAY_FreeHistory(NK_PVoid hHistory);
NK_Int TFSDK_PLAY_GetHistoryLen(NK_PVoid hHistory);
NK_Int TFSDK_PLAY_GetHistoryFile(NK_PVoid hHistory, NK_Int nIndex, pstTFPLAYFILE *pstFile);


#ifdef __cplusplus
}
#endif

#endif


