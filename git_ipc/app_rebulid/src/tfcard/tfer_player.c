#if 0
#include "tfer_player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <NkUtils/assert.h>
#include <NkUtils/macro.h>
#include <NkUtils/json.h>


/**
 * TFerPlayer 模块私有句柄，句柄访问模块内部的私有成员。\n
 * 内存在 TFerPlayer 模块创建时统一分配。\n
 * 位置在句柄数据结构 @ref NK_TFerPlayer 上位，\n
 * 这样有效避免 @ref NK_TFerPlayer 内存空间被错误释放。\n
 * 如下图：\n
 *
 *  | NK_PrivatedTFerPlayer
 * \|/
 *  +------------------------+
 *  |          |             |
 *  |          |             |
 *  +------------------------+
 *            /|\
 *             | NK_TFerPlayer
 *
 */
typedef struct nkPrivatedTFerPlayer {

	/**
	 * 模块内存分配器。
	 */
	NK_Allocator *Alloctr;

	/**
	 * TFer 事件集。
	 */
	NK_TFerEventSet *EventSet;

	/**
	 * TFer 事件集上下文。
	 */
	NK_PVoid evt_ctx;

	/**
	 * 播放起始时间戳。
	 */
	NK_UTC1970 play_utc;

	/**
	 * 播放文件列表。
	 */
	NK_JSON *PlayFileList;

	/**
	 * 播放列表长度。
	 */
	NK_Size play_file_list_len;

	/**
	 * 当前播放的文件列表位置。
	 */
	NK_Size current_play_file;

	/**
	 * 当前播放文件。
	 */
	NK_FileFLV *CurrentFileFLV;

	/**
	 * 当前播放文件时间戳（单位：毫秒）
	 */
	NK_UInt64 current_file_ts_ms;

//	/**
//	 * 录制媒体属性。
//	 */
//	struct {
//		NK_TFerRecordAttr Video;
//		NK_TFerRecordAttr Audio;
//	} Attr;

} NK_PrivatedTFerPlayer;

/**
 * 通过模块公有句柄获取私有句柄。
 */
static inline NK_PrivatedTFerPlayer *
PRIVATED(NK_TFerPlayer *Public)
{
	/**
	 * 偏移到私有句柄。
	 */
	return (NK_PrivatedTFerPlayer *)Public - 1;
}


/**
 * THIS 指针定义，\n
 * 定义以下为模块 API 接口实现。
 */
#define THIS NK_TFerPlayer * const Public

static NK_SSize
ReadFrameFromFLV(NK_FileFLV *FileFLV, NK_UInt32 *ts_ms, NK_TFerRecordAttr *Attr, NK_PByte data, NK_Size stack_len)
{
	NK_FLVTagHeadField HeadField;
	NK_SSize readn = 0;

	readn = FileFLV->readTag(FileFLV, &HeadField, data, stack_len);
	if (readn > 0) {
		/**
		 * 读到一帧数据。
		 */
		if (NK_FLV_TAG_TYPE_VIDEO == HeadField.type) {
			if (NK_FLV_TAG_CODEC_ID_AVC == HeadField.Video.codecID) {
				Attr->codec = NK_TFER_VCODEC_H264;
			}

			if (NK_FLV_TAG_AVC_SEQ_HEAD == HeadField.Video.avcPacketType) {
				NK_Log()->debug("TFer: Skip AVC Sequence Header.");
				return ReadFrameFromFLV(FileFLV, ts_ms, Attr, data, stack_len);
			}

			Attr->width = 1280; ///< 目前获取不了哦。
			Attr->height = 720; ///< 目前获取不了哦。
			Attr->frame_rate = 25;

		} else if (NK_FLV_TAG_TYPE_AUDIO == HeadField.type) {
			if (NK_FLV_TAG_SOUND_FMT_G711_ALAW == HeadField.Audio.soundFormat) {
				Attr->codec = NK_TFER_ACODEC_G711A;
			} else if (NK_FLV_TAG_SOUND_FMT_G711_ULAW == HeadField.Audio.soundFormat) {
				Attr->codec = NK_TFER_ACODEC_G711U;
			}

			switch(HeadField.Audio.soundRate) {
				default:
				case NK_FLV_TAG_SOUND_RATE_5P5KHZ: {
					Attr->sample_rate = 5500;
				}
				break;
				case NK_FLV_TAG_SOUND_RATE_11KHZ: {
					Attr->sample_rate = 11000;
				}
				break;
				case NK_FLV_TAG_SOUND_RATE_22KHZ: {
					Attr->sample_rate = 22000;
				}
				break;
				case NK_FLV_TAG_SOUND_RATE_44KHZ: {
					Attr->sample_rate = 44000;
				}
				break;
			}

			switch(HeadField.Audio.soundSize) {
				case NK_FLV_TAG_SOUND_SIZE_8BITS: {
					Attr->sample_bitwidth = 8;
				}
				break;
				default:
				case NK_FLV_TAG_SOUND_SIZE_16BITS: {
					Attr->sample_bitwidth = 16;
				}
				break;
			}

			switch(HeadField.Audio.soundType) {
				default:
				case NK_FLV_TAG_SOUND_TYPE_MONO: {
					Attr->stereo = NK_False;
				}
				break;
				case NK_FLV_TAG_SOUND_TYPE_STEREO: {
					Attr->stereo = NK_True;
				}
				break;
			}
		}

		*ts_ms = (HeadField.timestampEx << 24) + HeadField.timestamp;
		return readn;
	}

	return readn;
}


/**
 *
 * @param file_name
 * @return
 */
static NK_UTC1970
ExtractTimestampFromFilename(NK_PChar file_path)
{
	NK_Int year, month, mday;
	NK_Int hour, min, sec;
	NK_Char ext[64];
	NK_PChar date_time = NK_Nil;
	struct tm GetTM;
	NK_UTC1970 file_utc = 0;

	date_time = strstr(file_path, "record/");
	if (NK_Nil != date_time) {
		date_time += strlen("record/");
		if (7 == sscanf(date_time, "%04d%02d%02d/%02d%02d%02d.%s",
				&year, &month, &mday, &hour, &min, &sec, ext)) {

			NK_Log()->debug("TFer: File UTC ( %04d%02d%02d-%02d%02d%02d ).",
					year, month, mday, hour, min, sec);

			NK_BZERO(&GetTM, sizeof(GetTM));
			GetTM.tm_year = year - 1900;
			GetTM.tm_mon = month - 1;
			GetTM.tm_mday = mday;
			GetTM.tm_hour = hour;
			GetTM.tm_min = min;
			GetTM.tm_sec = sec;
#ifdef _WIN32
			//*$*file_utc = timelocal(&GetTM);
#else
			file_utc = timelocal(&GetTM);
#endif
			NK_Log()->debug("TFer: File UTC %u.", file_utc);
			return file_utc;
		}
	}

	return 0;
}


static NK_Int
TFerPlayer_read(THIS, NK_TFerRecordAttr *Attr, NK_UInt64 *ts_ms, NK_PByte data, NK_Size stack_len)
{
	NK_PrivatedTFerPlayer *Privated = PRIVATED(Public);
	NK_JSON *VarFilePath = NK_Nil;
	NK_PChar file_path = NK_Nil;
	NK_UInt32 file_ts_ms = 0;
	NK_SSize readn = 0;

	NK_EXPECT_VERBOSE_RETURN(NK_Nil != ts_ms, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != data, -1);
	NK_EXPECT_VERBOSE_RETURN(stack_len > 0, -1);

	if (Privated->current_play_file == Privated->play_file_list_len) {
		NK_Log()->warn("TFer: Endof Play File List.");
		return 0;
	}

	/**
	 * 打开文件。
	 */
	if (!Privated->CurrentFileFLV) {
		VarFilePath = NK_JSON_IndexOf(Privated->PlayFileList, Privated->current_play_file);
		if (NK_Nil != VarFilePath) {
			file_path = NK_JSON_GetString(VarFilePath, NK_Nil);
			if (NK_Nil != file_path) {

				Privated->CurrentFileFLV = NK_FileFLV_Open(Privated->Alloctr, file_path);

				/**
				 * 打开文件失败，文件可能已经不存在了，尝试到下个周期打开文件列表中下一个文件。
				 */
				if (!Privated->CurrentFileFLV) {
					++Privated->current_play_file;
					Privated->current_play_file %= Privated->play_file_list_len;

					NK_Log()->error("TFer: File( Path = \"%s\") NOT Found.", file_path);
					return Public->read(Public, Attr, ts_ms, data, stack_len);
				}

				/**
				 * 计算一下该文件的时间戳。
				 */
				Privated->current_file_ts_ms = ExtractTimestampFromFilename(file_path);
				Privated->current_file_ts_ms *= 1000;

				NK_Log()->debug("TFer: Play File( Path = \"%s\" ).", file_path);
			}
		}
	}

	/**
	 * 从文件中读取一帧数据。
	 */
	if (NK_Nil != Privated->CurrentFileFLV) {

		readn = ReadFrameFromFLV(Privated->CurrentFileFLV, &file_ts_ms, Attr, data, stack_len);
//		if (readn < 0) {
//			NK_Log()->error("TFer: Read File Failed.");
//			return -1;
//		}

		if (readn == -1) {
			NK_Log()->error("TFer: Read File Failed.");
			return -1;
		}

		/// FIXME 这里逻辑有点问题。
		if (readn <= 0) {
		//if (0 == readn) {

			NK_Log()->debug("TFer: Read Endof File.");

			/**
			 * 读取失败了，切换到下一个文件。
			 */
			NK_FileFLV_Free(&Privated->CurrentFileFLV);
			++Privated->current_play_file;
			Privated->current_play_file %= Privated->play_file_list_len;

			return Public->read(Public, Attr, ts_ms, data, stack_len);
		}

		*ts_ms = Privated->current_file_ts_ms + file_ts_ms;
		return readn;
	}

	return -1;
}

static NK_Int
TFerPlayer_seek(THIS, NK_UTC1970 utc)
{
//	NK_PrivatedTFerPlayer *Privated = PRIVATED(Public);

	NK_Log()->error("TFer: My Brother, This Function NOT Support.");
	return -1;
}


/**
 * THIS 指针定义解除，\n
 * 定义以上为模块 API 接口实现。
 */
#undef THIS

NK_TFerPlayer *
NK_TFer_CreatePlayer(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_UTC1970 play_utc, NK_JSON *PlayFileList, NK_PVoid player_ctx)
{
	NK_PrivatedTFerPlayer *Privated = NK_Nil;
	NK_TFerPlayer *Public = NK_Nil;

	/**
	 * 检查内存分配器。
	 */
	if (!Alloctr) {
		Alloctr = NK_MemAlloc_OS();
	}

	/**
	 * 初始化句柄。
	 * 公有句柄的内存空间仅靠私有句柄的高位。
	 * 有效防止模块公有句柄在外部被意外释放。
	 */
	Privated = Alloctr->alloc(Alloctr, sizeof(NK_PrivatedTFerPlayer) + sizeof(NK_TFerPlayer));
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Privated, NK_Nil);
	Public = (NK_TFerPlayer *)(Privated + 1);

	/**
	 * 初始化私有成员。
	 */
	Privated->Alloctr = Alloctr;
	Privated->EventSet = EventSet;
	Privated->evt_ctx = evt_ctx;
	Privated->play_utc = play_utc;
	Privated->PlayFileList = PlayFileList;
	Privated->play_file_list_len = NK_JSON_ArraySize(Privated->PlayFileList);
	Privated->current_play_file = 0;
	Privated->CurrentFileFLV = NK_Nil;

	/**
	 * 初始化公有成员。
	 */
	Public->read = TFerPlayer_read;
	Public->seek = TFerPlayer_seek;

//	NK_Log()->debug("Privated->PlayFileList = %p", Privated->PlayFileList);
//	Privated->PlayFileList->Object.dump(&Privated->PlayFileList->Object);

	/**
	 * 返回公有句柄。
	 * 私有句柄存放在外部调用者不可见区域防止外部被错误读写。
	 */
	return Public;
}

NK_Int
NK_TFer_FreePlayer(NK_TFerPlayer **TFerPlayer_r)
{
	NK_TFerPlayer *Public = NK_Nil;
	NK_PrivatedTFerPlayer *Privated = NK_Nil;
	NK_Allocator *Alloctr = NK_Nil;

	/**
	 * 释放无效句柄断言。
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerPlayer_r, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerPlayer_r[0], -1);

	/**
	 * 获取私有句柄。
	 */
	Public = TFerPlayer_r[0];
	Privated = PRIVATED(Public);
	TFerPlayer_r[0] = NK_Nil; ///< 操作前先清空句柄防止上层用户在释放过程中意外调用。
	Alloctr = Privated->Alloctr;

	/**
	 * 关闭的媒体播放文件。
	 */
	if (NK_Nil != Privated->CurrentFileFLV) {
		NK_FileFLV_Free(&Privated->CurrentFileFLV);
	}

	/**
	 * 释放模块句柄。
	 */

	Alloctr->freep(Alloctr, Privated);

	/**
	 * 释放成功。
	 */
	return 0;
}

#endif
