#if !defined(NK_TFER_H)
#define NK_TFER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <NkUtils/types.h>
#include <NkEmbedded/thread.h>

#define NK_TFCARD_VCODEC_H264 (96)
#define NK_TFCARD_VCODEC_H265 (97)

#define NK_TFCARD_ACODEC_G711A (8)
#define NK_TFCARD_ACODEC_G711U (9)
#define NK_TFCARD_ACODEC_AAC (10)
#define NK_TFCARD_CODEC_DATA (11)


// tf CARD status
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

/**
 * ¬ºœÒ÷° ÷°Õ∑Ω·ππ 
 */
typedef struct Record_Frame_Head {
	NK_Int codec;
	NK_UInt64 coderStamp_ms;
	NK_UInt64 sysTime_ms;
	NK_Size dataSize;
	
	union {
		/**
		 * “Ù∆µ Ù–‘°£
		 */
		struct {
			NK_Int sampleRate, sampleWidth, samplePacket;
			NK_Float compressionRatio;
		};
		/**
		 *  ”∆µ Ù–‘°£
		 */
		struct {
			NK_Int isKeyFrame, fps, width, height;
		};
	};
	NK_Int reserve;
}stRecord_Frame_Head,*lpRecord_Frame_Head;

/**
 * ¬ºœÒ÷° ÷°Ω·ππ 
 */ 
typedef struct TFCARD_Record_Frame_Buf {
	stRecord_Frame_Head head;
	NK_PByte data;
}stTFCARD_Record_Frame_Buf,*lpTFCARD_Record_Frame_Buf;

/**
 * ¬ºœÒŒƒº˛Œ≤≤øΩ·ππ
 */
#define TAIL_STRUCT_SIZE (2048)
#define IFRAME_MAX_CNT_SAVE (200) //¥À¥¶–Ë“™◊¢“‚»›¡ø≤ªø…≥¨π˝Œ≤≤øΩ·ππ◊‹¡ø
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
			NK_Size filesize;// ˝æ›≥§∂», ≤ª∞¸¿®Œ≤≤øΩ·ππ≥§∂»
		};
	};
}stFile_Tail,*lpFile_Tail;

/**
 * ¬ºœÒ¿˙ ∑¡–±ÌΩ·ππ
 */
typedef struct TFCARD_History_List {
	NK_Char recordType[32];
	NK_UTC1970 beginTm;
	NK_UTC1970 endTm;
}stTFCARD_History_List, *lpTFCARD_History_List;

/**
 * ºÏ≤‚ TF ŒÔ¿Ì…œ «∑Ò¥Ê‘⁄ ¬º˛°£
 */
typedef NK_Boolean (*fTFcardOnExistTF)();

/**
 * ºÏ≤‚ TF  «∑Ò¥Ê‘⁄ ¬º˛°£
 */
typedef NK_Boolean (*fTFcardOnDetectTF)();

/**
 * ◊∞‘ÿ TF ø®Œƒº˛œµÕ≥ƒø¬º
 */
typedef NK_Int (*fTFcardOnMountTF)(NK_PChar mount_path);

/**
 * –∂‘ÿ TF ø®Œƒº˛œµÕ≥ƒø¬º ¬º˛°£
 */
typedef NK_Int (*fTFcardOnUmountTF)(NK_PChar mount_path);

/**
 * «Â≥˝ TF ø®π“‘ÿƒø¬º ¬º˛
 */
typedef NK_Int (*fTFcardOnCleanTF)(NK_PChar mount_path);

/**
 * ªÒ»° TF ø®◊‹»›¡ø°£
 */
typedef NK_Size (*fTFcardOnGetCapacity)(NK_PChar mount_path);

/**
 * ªÒ»° TF ø®ø…”√¡ø°£
 */
typedef NK_Size (*fTFcardOnGetFreeSpace)(NK_PChar mount_path);


/**
 * TF ø®∏Ò ΩªØ ¬º˛ 
 */
typedef NK_Int (*fTFcardOnFormat)();

/**
 *  TFÊ†ºÂºèÂåñÂêéÁöÑÊìç‰Ωú
 * @param status  trueÊ†ºÂºèÂåñÊàêÂäü|falseÊ†ºÂºèÂåñÂ§±Ë¥•
 */
typedef NK_Int (*fTFcardOnAfterFormat)(NK_Boolean status);


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
    fTFcardOnAfterFormat OnAfterFormat;
}ST_TFCARD_FUNCTION, *LP_TFCARD_FUNCTION;

extern NK_Int NK_TFCARD_init(LP_TFCARD_FUNCTION init, NK_PChar fs_path, NK_Size max_buffer_size_kb);
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
extern NK_Int NK_TFCARD_play_read_frame(lpRecord_Frame_Head frameHead, NK_PByte data,NK_Size dataMaxSize);
extern NK_Int NK_TFCARD_play_stop();
extern NK_Int NK_TFCARD_get_history(NK_UTC1970 beginUtc, NK_UTC1970 endUtc,
									NK_PChar type,
									lpTFCARD_History_List historyList,
									NK_Int startIndex,
									NK_Int *historyCnt);
extern NK_Int NK_TFCARD_get_status(char *ret_status);
extern void NK_TFCARD_turn_on_status_check(void);

#ifdef __cplusplus
};
#endif
#endif /*NK_TFER_H*/
