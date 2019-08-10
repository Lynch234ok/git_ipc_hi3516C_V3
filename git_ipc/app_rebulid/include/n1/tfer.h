/**
 * TF 卡录像抽线模块。
 */


#include <NkUtils/object.h>
#include <NkUtils/allocator.h>
#include <NkEmbedded/thread.h>
#include "file_flv.h" ///< FLV 文件读写句柄。


#if !defined(NK_TFER_H_)
# define NK_TFER_H_
NK_CPP_EXTERN_BEGIN

/**
 * 版本号。
 */
#define NK_TFER_VERSION "0.1"


#define NK_TFER_VCODEC_H264 (96)

#define NK_TFER_ACODEC_G711A (8)
#define NK_TFER_ACODEC_G711U (9)

/**
 * TFer 录像参数定义。
 */
typedef struct Nk_TFerRecordAttr {

	NK_Int codec;

	union {
		/**
		 * 音频属性。
		 */
		struct {

			NK_Boolean stereo;
			NK_Size sample_bitwidth, sample_rate;

		};

		/**
		 * 视频属性。
		 */
		struct {

			NK_Size width, height;
			NK_Size frame_rate;

		};
	};

} NK_TFerRecordAttr;


typedef struct Nk_TFerRecorder {
#define NK_This struct Nk_TFerRecorder *const

	/**
	 * 设置录制的视频参数。
	 *
	 */
	NK_Int
	(*setVideoAttr)(NK_This, NK_TFerRecordAttr *Attr);

	/**
	 * 设置录制音频参数。
	 *
	 */
	NK_Int
	(*setAudioAttr)(NK_This, NK_TFerRecordAttr *Attr);

	/**
	 * 写入一个或者多个 H.264 NALU 数据。\n
	 * 通过此方法向 TFer 写入一个或者多个 H.264 NALU 数据。\n
	 * 调用此接口之前须要先通过 @ref NK_TFerRecorder::setVideo 设置成 H.264 的 codec。
	 *
	 * @param[in]		ts_ms			数据的时间戳（单位：毫秒）。
	 * @param[in]		seekable		可检索标识，此标识会影响到读取时候的检索策略，一本对应关键帧，当为 True 时，内部必须包含解码须要的 SPS 和 PPS 数据。
	 * @param[in]		data			一个或多个连续的 H.264 NALU 数据。
	 * @param[in]		len				一个或多个连续的 H.264 NALU 长度。
	 *
	 * @return		写入成功，返回写入数据的长度，写入失败返回 -1。
	 */
	NK_SSize
	(*writeH264)(NK_This, NK_UInt64 ts_ms, NK_Boolean seekable, NK_PVoid data, NK_Size len);

	NK_SSize
	(*writeG711)(NK_This, NK_UInt64 ts_ms, NK_PVoid data, NK_Size len);


#undef NK_This
} NK_TFerRecorder;


typedef struct Nk_TFerHistory {

	/**
	 * 历史录像的类型。
	 */
	NK_Char type[32];

	/**
	 * 历史录像的开始和结束时间。
	 */
	NK_UTC1970 begin_utc, end_utc;


	NK_Char begin_time[32], end_time[32];

} NK_TFerHistory;


/**
 * TFerPlayer 模块句柄。
 */
typedef struct Nk_TFerPlayer {
#define NK_This struct Nk_TFerPlayer *const

	/**
	 * 从回放器中读取一帧数据。
	 *
	 * @param Attr
	 * @param data
	 * @param data_size
	 * @return
	 */
	NK_SSize
	(*read)(NK_This, NK_TFerRecordAttr *Attr, NK_UInt64 *ts_ms, NK_PByte data, NK_Size stack_len);

	/**
	 * 定位到回放器数据的指定 UTC 时间数据上。
	 *
	 * @param[in]			utc				UTC 时间戳。
	 *
	 * @return		成功返回 0，失败返回 -1。
	 */
	NK_Int
	(*seek)(NK_This, NK_UTC1970 utc);


#undef NK_This
} NK_TFerPlayer;


/**
 * 检测 TF 是否存在事件。
 */
typedef NK_Boolean (*NK_TFerOnDetectTF)(NK_PVoid ctx, NK_Int id);

/**
 * 装载 TF 卡文件系统目录
 */
typedef NK_Int (*NK_TFerOnMountTF)(NK_PVoid ctx, NK_Int id, NK_PChar fs_path);

/**
 * 卸载 TF 卡文件系统目录事件。
 */
typedef NK_Int (*NK_TFerOnUmountTF)(NK_PVoid ctx, NK_Int id, NK_PChar fs_path);


/**
 * 获取 TF 卡总容量。
 */
typedef NK_Size (*NK_TFerOnGetCapacity)(NK_PVoid ctx, NK_Int id, NK_PChar fs_path);

/**
 * 获取 TF 卡可用量。
 */
typedef NK_Size (*NK_TFerOnGetFreeSpace)(NK_PVoid ctx, NK_Int id, NK_PChar fs_path);

/**
 * TF 卡录像事件。
 */
typedef NK_Int (*NK_TFerOnRecord)(NK_PVoid ctx, NK_Thread *Thread, NK_TFerRecorder *Recorder, NK_PVoid recorder_ctx);

/**
 * TF 卡播放事件。
 */
typedef NK_Int (*NK_TFerOnPlay)(NK_PVoid ctx, NK_Thread *Thread, NK_TFerPlayer *Player, NK_PVoid player_ctx);

/**
 * TFer 事件集合。
 *
 */
typedef struct Nk_TFerEventSet
{
	/**
	 * 检测 TF 是否存在事件。\n
	 * 模块创建以后会定时调用此接口去检测 TF 卡是否插入。\n
	 * 调用者需要实现所在平台检测 TF 卡存在的方法，\n
	 * 并返回相对应的结果给模块。\n
	 * 当此接口返回 True 表明通知模块已经检测到 TF 卡，\n
	 * 之后模块会触发 @ref TNK_TFStorageEvent::onInstallTF 事件去连接 TF 卡。
	 *
	 * @param[in]		ctx			事件触发用户上下文，在模块创建时由用户传入。
	 *
	 * @return	检测到 TF 卡时返回 True，否则返回 False。
	 */
	NK_TFerOnDetectTF onDetectTF;

	/**
	 * 装载 TF 卡文件系统目录。\n
	 * 模块会在 @ref TNK_TFStorageEventSet::onDetect 事件调用确定 TF 卡插入的时候主动触发此事件。\n
	 * 用户根据具体平台实现 TF 卡插入后加载的各种工作，如挂载文件系统、音频提示音播报提示插入成功等。\n
	 * 成功之后，模块开始在该卡槽上的 TF 卡进行读写。
	 *
	 * @param[in]		ctx			事件触发用户上下文，在模块创建时由用户传入。
	 * @param[in]		id			TF 卡对应的序列号。
	 * @param[out]		fs_path		文件系统目录，TF 卡插入后模块会自动挂在文件系统到该路径下，此处通知模块录像所在路径。
	 *
	 *
	 * @return	连接成功返回 0，失败返回 -1。
	 */
	NK_TFerOnMountTF onMountTF;

	/**
	 * 卸载 TF 卡文件系统目录事件。\n
	 * 模块会在 @ref TNK_TFStorageEventSet::onDetectTF 事件调用确定 TF 卡拔出的时候主动触发此事件。\n
	 * 用户根据具体平台实现 TF 卡拔出后卸载的各种工作，如强行停止录像，卸载文件系统、音频提示音播报拔出成功等。\n
	 * 断开之后，模块将不在该卡槽上的 TF 卡进行读写。
	 *
	 * @param[in]		ctx			事件触发用户上下文，在模块创建时由用户传入。
	 * @param[in]		id			TF 卡对应的序列号。
	 * @param[in]		fs_path		模块创建时传入的映射文件路径，TF 卡插入后模块会自动挂在文件系统到该路径下。
	 *
	 *
	 * @return		断开成功返回 0，失败返回 -1。
	 */
	NK_TFerOnUmountTF onUmountTF;


	/**
	 * 获取 TF 卡总容量。\n
	 *
	 * @param[in]		ctx			事件触发用户上下文，在模块创建时由用户传入。
	 * @param[in]		id			TF 卡对应的序列号。
	 * @param[in]		fs_path		模块创建时传入的映射文件路径，TF 卡插入后模块会自动挂在文件系统到该路径下。
	 *
	 * @return	获取成功返回容量大小（单位：兆字节），失败返回 0。
	 */
	NK_TFerOnGetCapacity onGetCapacity;

	/**
	 * 获取 TF 卡可用量。\n
	 *
	 * @param[in]		ctx			事件触发用户上下文，在模块创建时由用户传入。
	 * @param[in]		id			TF 卡对应的序列号。
	 * @param[in]		fs_path		模块创建时传入的映射文件路径，TF 卡插入后模块会自动挂在文件系统到该路径下。
	 *
	 * @return	获取成功返回容量大小（单位：MBytes），失败返回 0。
	 */
	NK_TFerOnGetFreeSpace onGetFreeSpace;


	/**
	 * TF 用户录像。
	 */
	NK_TFerOnRecord onRecord;

	/**
	 * TF 用户播放。
	 */
	NK_TFerOnPlay onPlay;


} NK_TFerEventSet;


/**
 * TFer 模块句柄。
 */
typedef struct Nk_TFer {
#define NK_This struct Nk_TFer *const

	/**
	 * 模块基础接口。
	 */
	NK_Object Object;

	/**
	 * 录像开启/关闭。
	 */
	NK_Int
	(*record)(NK_This, NK_Boolean start, NK_PChar type, NK_PVoid recorder_ctx);

	/**
	 * 获取历史信息。
	 */
	NK_Int
	(*history)(NK_This, NK_UTC1970 utc, NK_PChar type, NK_TFerHistory *History, NK_Size *n);

	/**
	 * 播放录像开启/关闭。
	 */
	NK_Int
	(*play)(NK_This, NK_Boolean start, NK_PChar type, NK_UTC1970 utc, NK_PVoid player_ctx);


#undef NK_This
} NK_TFer;


/**
 * 创建 TFer 模块句柄。
 *
 * @param[in]		Alloctr			内存分配器。
 * @param[in]		slots			TF 卡卡槽数量，表示此模块可以同时支持的 TF 卡数量。
 * @param[in]		EventSet		TF 卡控制事件集。
 *
 * @return		模块句柄。
 */
extern NK_TFer *
NK_TFer_Create(NK_Allocator *Alloctr, NK_Size slots, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx);

/**
 * 销毁 TFer 模块句柄。
 *
 * @param[in]		TFer_r			模块句柄引用。
 *
 * @return 		释放成功返回 0，否则返回 -1。
 */
extern NK_Int
NK_TFer_Free(NK_TFer **TFer_r);



NK_CPP_EXTERN_END
#endif /* NK_TFER_H_ */


