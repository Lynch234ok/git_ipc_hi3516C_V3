/**
 *
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>
#include "flv_def.h"

#ifndef NK_FLV_FILE_H_
#define NK_FLV_FILE_H_
NK_CPP_EXTERN_BEGIN


/**
 * FlvFile 模块句柄。
 */
typedef struct Nk_FileFLV {
#define NK_This struct Nk_FileFLV *const

	/**
	 * 基类模块。
	 */
	NK_Object Object;

	/**
	 * 获取 FLV 文件大小。
	 */
	NK_SSize
	(*size)(NK_This);

    /**
     * 获取文件头。
     */
    NK_Integer
    (*getFileHeadField)(NK_This, NK_FLVFileHeadField *HeadField);

    /**
     * 设置视频属性。
     *
     * @param[in]		width			视频宽度，传入 0 表示使用默认。
     * @param[in]		height			视频高度，传入 0 表示使用默认。
     * @param[in]		framerate		视频帧率，传入 0 表示使用默认。
     * @param[in]		bitrate			码率，传入 0 表示使用默认。
     *
     * @return		设置成功返回 0，否则返回 -1。
     */
    NK_Int
	(*setVideoAttr)(NK_This, NK_FLVTagCodecID codec_id, NK_Size width, NK_Size height, NK_Size framerate, NK_Size bitrate);

    /**
     * 设置音频属性。
     *
     * @return
     */
    NK_Int
	(*setAudioAttr)(NK_This, NK_FLVTagSoundFormat sound_fmt, NK_Boolean stereo, NK_Int32 bitwidth, NK_UInt32 samplerate);



    /**
	 * 写入一个 TAG 数据。
	 *
	 * @param[in]			ts_ms			当前 TAG 数据所在时间戳（单位：毫秒）。
	 * @param[in,out]		TagHeadField	FLV 文件的 TAG 头部数据结构，传入后模块将会对里面的 dateSize、timestamp 和 timestampEx 参数修正。
	 * @param[in]			data			FLV 文件的 TAG 数据。
	 * @param[in]			len				FLV 文件的 TAG 数据长度。
	 *
	 * @return				写入长度。
	 *
	 */
    NK_SSize
	(*writeTag)(NK_This, NK_UInt64 ts_ms, NK_FLVTagHeadField *TagHeadField, NK_PByte data, NK_Size len);

    /**
     * 写入同一时间戳的一个或者多个 AVC NALU 数据。\n
     * 当 @ref seekable 标识位有效的时候，数据必须包含所有编码器产生的 PPS 和 SPS NALU，\n
     * 在内部将会调用 @ref NK_FileFLV::writeTag 实现写入操作。
     *
     * @param[in]			ts_ms			当前数据的时间戳（单位：毫秒）。
     * @param[in]			seekable		可检索标识，与文件内部定位有关，一般对应关键帧数据。
     * @param[in]			data			写入的一个或者多个连续 NALU 数据。
     * @param[in]			len				数据长度。
     *
     *
     */
	NK_SSize
    (*writeH264)(NK_This, NK_UInt64 ts_ms, NK_Boolean seekable, NK_PByte data, NK_Size len);

	/**
	 * 写入 AAC Raw 数据。\n
	 * 在内部将会调用 NK_FileFLV::writeTag 实现写入操作。
	 *
	 * @param[in]			timestamp		该数据编码产生时的时间戳（单位：毫秒）。
	 * @param[in]			stereo			是否双声道标识，NK_True 表示双声道，否则为单声道。
	 * @param[in]			bitwidth		音频位宽，参考 @ref NK_FLVTagHeadField::Audio::soundSize。
	 * @param[in]			sample			音频采样率，参考 @ref NK_FLVTagHeadField::Audio::soundRate。
	 * @param[in]			raw				音频数据，需要携带 ADTS 信息。
	 * @param[in]			len				音频数据的长度（单位：字节）。
	 *
	 * @return				成功返回写入音频数据的长度，失败返回 -1。
	 */
	NK_SSize
	(*writeAACRaw)(NK_This, NK_UInt64 timestamp, NK_Boolean stereo, NK_Int32 bitwidth, NK_UInt32 sample, NK_PByte raw, NK_Size len);

	/**
	 *
	 * @param
	 * @param timestamp
	 * @param stereo
	 * @param bitwidth
	 * @param sample
	 * @param raw
	 * @param len
	 * @return
	 */
	NK_SSize
	(*writeG711a)(NK_This, NK_UInt64 ts_ms, NK_PByte data, NK_Size len);

    /**
     * 读取一个 TAG 数据。
     */
    NK_SSize
    (*readTag)(NK_This, NK_FLVTagHeadField *TagHeader, NK_PByte stack, NK_Size stack_len);


#undef NK_This
} NK_FileFLV;


/**
 * 创建 FlvFile 模块句柄。\n
 * 创建方式生成句柄只具备写功能。
 */
extern NK_FileFLV *
NK_FileFLV_Create(NK_Allocator *Alloctr, const NK_PChar file_path);

/**
 * 打开 FlvFile 文件并生成句柄。\n
 * 通过打开方式产生句柄之具备读功能。
 */
extern NK_FileFLV *
NK_FileFLV_Open(NK_Allocator *Alloctr, const NK_PChar file_path);

/**
 * 销毁 FlvFile 模块句柄。
 */
extern NK_Int
NK_FileFLV_Free(NK_FileFLV **FileFLV_r);


NK_CPP_EXTERN_END
#endif /* NK_FLV_FILE_H_ */
