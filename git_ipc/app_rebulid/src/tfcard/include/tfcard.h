#ifndef NK_TFER_H
#define NK_TFER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <juan_types.h>

#define NK_TFCARD_VCODEC_H264 (96)
#define NK_TFCARD_VCODEC_H265 (97)

#define NK_TFCARD_ACODEC_G711A (8)
#define NK_TFCARD_ACODEC_G711U (9)
#if 0
#define sTFCARD_STATUS_OK "ok"
#define sTFCARD_STATUS_FORMATED "already_format"
#define sTFCARD_STATUS_NOT_FORMAT "no_format"
#define sTFCARD_STATUS_EXCEPTION "exception"
#define sTFCARD_STATUS_NO_TFCARD "no_tfcard"
#define sTFCARD_STATUS_FORMATTING "formatting"

enum{
	emTFCARD_STATUS_OK = 0,
	emTFCARD_STATUS_FORMATED,
	emTFCARD_STATUS_NOT_FORMAT,
	emTFCARD_STATUS_EXCEPTION,
	emTFCARD_STATUS_NO_TFCARD,
	emTFCARD_STATUS_FORMATTING,
	emTFCARD_STATUS_CNT,
};
#endif
/**
 * 录像文件时区转换
 * action : 设备无时区转换时需定义,例:东八区则定义为(+8)
 * 		   若设备已有时区转换,则定义为0.
 */
#define TIMEZONE_OF_LOCATION	(+8)

/**
 * 录像帧 帧头结构
 */
typedef struct Record_Frame_Head {
	NK_Int codec;
	NK_UInt64 coderStamp_ms;
	NK_UInt64 sysTime_ms;
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
}stRecord_Frame_Head,*lpRecord_Frame_Head;

/**
 * 录像帧 帧结构
 */
typedef struct TFCARD_Record_Frame_Buf {
	stRecord_Frame_Head head;
	NK_PByte data;
}stTFCARD_Record_Frame_Buf,*lpTFCARD_Record_Frame_Buf;

/**
 * 录像文件尾部结构
 */
#define TAIL_STRUCT_SIZE (2048)
#define IFRAME_MAX_CNT_SAVE (200) //此处需要注意容量不可超过尾部结构总量
typedef struct file_tail_t{
	union{
		struct {
			NK_Char buf[TAIL_STRUCT_SIZE];
		};
		struct {
			NK_Char record_type[32];
			NK_Int frameCnt;
			NK_Int iFrameCnt;
			NK_Size iFramePos[IFRAME_MAX_CNT_SAVE];
			NK_Int iFrameSec[IFRAME_MAX_CNT_SAVE];
			NK_Int beginSec;
			NK_Int endSec;
			NK_Size filesize;//数据长度, 不包括尾部结构长度
		};
	};
}stFile_Tail,*lpFile_Tail;

/**
 * 录像历史列表结构
 */
typedef struct TFCARD_History_List {
	NK_Char recordType[32];
	NK_UTC1970 beginTm;
	NK_UTC1970 endTm;
}stTFCARD_History_List, *lpTFCARD_History_List;


typedef NK_Boolean (*fTFcardOnExistTF)();
/**
 * 检测 TF 是否存在事件。
 */
typedef NK_Boolean (*fTFcardOnDetectTF)();

/**
 * 装载 TF 卡文件系统目录
 */
typedef NK_Int (*fTFcardOnMountTF)(NK_PChar mount_path);

/**
 * 卸载 TF 卡文件系统目录事件。
 */
typedef NK_Int (*fTFcardOnUmountTF)(NK_PChar mount_path);

/**
 * 清除 TF 卡挂载目录事件
 */
typedef NK_Int (*fTFcardOnCleanTF)(NK_PChar mount_path);

/**
 * 获取 TF 卡总容量。
 */
typedef NK_Size (*fTFcardOnGetCapacity)(NK_PChar mount_path);

/**
 * 获取 TF 卡可用量。
 */
typedef NK_Size (*fTFcardOnGetFreeSpace)(NK_PChar mount_path);


/**
 * TF 卡格式化事件
 */
typedef NK_Int (*fTFcardOnFormat)();


typedef struct tfcard_function {
	fTFcardOnExistTF onExistTF;
	fTFcardOnDetectTF OnDetectTF;
	fTFcardOnMountTF OnMountTF;
	fTFcardOnUmountTF OnUmountTF;
	fTFcardOnCleanTF OnCleanTF;
	fTFcardOnGetCapacity OnGetCapacity;
	fTFcardOnGetFreeSpace OnGetFreeSpace;
	//fTFcardOnRecord OnRecord;
	//fTFcardOnPlay OnPlay;
	fTFcardOnFormat OnFormat;
}ST_TFCARD_FUNCTION, *LP_TFCARD_FUNCTION;

extern NK_Int NK_TFCARD_init(LP_TFCARD_FUNCTION init, NK_PByte fs_path, NK_Size max_buffer_size_kb);
extern NK_Int NK_TFCARD_destroy();
extern NK_Boolean NK_TFCARD_exist();
extern NK_Boolean NK_TFCARD_detect();
extern NK_Boolean NK_TFCARD_is_mounted();
extern NK_Size NK_TFCARD_get_capacity();
extern NK_Size NK_TFCARD_get_freespace();
extern NK_Int NK_TFCARD_format();
extern NK_Int NK_TFCARD_record_start(NK_PChar record_type);
extern NK_Int NK_TFCARD_record_write_frame(lpRecord_Frame_Head frameHead, NK_PByte data);
extern NK_Int NK_TFCARD_record_stop();
extern NK_Int NK_TFCARD_play_start(NK_UTC1970 beginUtc,NK_PChar playType);
extern NK_Int NK_TFCARD_play_read_frame(lpRecord_Frame_Head frameHead, NK_PByte data, NK_Size dataMaxSize);
extern NK_Int NK_TFCARD_play_stop();
extern NK_Int NK_TFCARD_get_history(NK_UTC1970 beginUtc, NK_PChar type, lpTFCARD_History_List historyList, NK_Int startIndex, NK_Int *historyCnt);
extern NK_Int NK_TFCARD_get_timezone_sec();
extern NK_Void NK_TFCARD_localtime_r(time_t* utc, struct tm* result);
extern time_t NK_TFCARD_mktime(struct tm* result);
extern NK_Int NK_TFCARD_get_status(char *ret_status);


#ifdef __cplusplus
};
#endif
#endif /*NK_TFER_H*/