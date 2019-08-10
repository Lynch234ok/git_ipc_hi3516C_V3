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
 * TFerPlayer ģ��˽�о�����������ģ���ڲ���˽�г�Ա��\n
 * �ڴ��� TFerPlayer ģ�鴴��ʱͳһ���䡣\n
 * λ���ھ�����ݽṹ @ref NK_TFerPlayer ��λ��\n
 * ������Ч���� @ref NK_TFerPlayer �ڴ�ռ䱻�����ͷš�\n
 * ����ͼ��\n
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
	 * ģ���ڴ��������
	 */
	NK_Allocator *Alloctr;

	/**
	 * TFer �¼�����
	 */
	NK_TFerEventSet *EventSet;

	/**
	 * TFer �¼��������ġ�
	 */
	NK_PVoid evt_ctx;

	/**
	 * ������ʼʱ�����
	 */
	NK_UTC1970 play_utc;

	/**
	 * �����ļ��б�
	 */
	NK_JSON *PlayFileList;

	/**
	 * �����б��ȡ�
	 */
	NK_Size play_file_list_len;

	/**
	 * ��ǰ���ŵ��ļ��б�λ�á�
	 */
	NK_Size current_play_file;

	/**
	 * ��ǰ�����ļ���
	 */
	NK_FileFLV *CurrentFileFLV;

	/**
	 * ��ǰ�����ļ�ʱ�������λ�����룩
	 */
	NK_UInt64 current_file_ts_ms;

//	/**
//	 * ¼��ý�����ԡ�
//	 */
//	struct {
//		NK_TFerRecordAttr Video;
//		NK_TFerRecordAttr Audio;
//	} Attr;

} NK_PrivatedTFerPlayer;

/**
 * ͨ��ģ�鹫�о����ȡ˽�о����
 */
static inline NK_PrivatedTFerPlayer *
PRIVATED(NK_TFerPlayer *Public)
{
	/**
	 * ƫ�Ƶ�˽�о����
	 */
	return (NK_PrivatedTFerPlayer *)Public - 1;
}


/**
 * THIS ָ�붨�壬\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
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
		 * ����һ֡���ݡ�
		 */
		if (NK_FLV_TAG_TYPE_VIDEO == HeadField.type) {
			if (NK_FLV_TAG_CODEC_ID_AVC == HeadField.Video.codecID) {
				Attr->codec = NK_TFER_VCODEC_H264;
			}

			if (NK_FLV_TAG_AVC_SEQ_HEAD == HeadField.Video.avcPacketType) {
				NK_Log()->debug("TFer: Skip AVC Sequence Header.");
				return ReadFrameFromFLV(FileFLV, ts_ms, Attr, data, stack_len);
			}

			Attr->width = 1280; ///< Ŀǰ��ȡ����Ŷ��
			Attr->height = 720; ///< Ŀǰ��ȡ����Ŷ��
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
	 * ���ļ���
	 */
	if (!Privated->CurrentFileFLV) {
		VarFilePath = NK_JSON_IndexOf(Privated->PlayFileList, Privated->current_play_file);
		if (NK_Nil != VarFilePath) {
			file_path = NK_JSON_GetString(VarFilePath, NK_Nil);
			if (NK_Nil != file_path) {

				Privated->CurrentFileFLV = NK_FileFLV_Open(Privated->Alloctr, file_path);

				/**
				 * ���ļ�ʧ�ܣ��ļ������Ѿ��������ˣ����Ե��¸����ڴ��ļ��б�����һ���ļ���
				 */
				if (!Privated->CurrentFileFLV) {
					++Privated->current_play_file;
					Privated->current_play_file %= Privated->play_file_list_len;

					NK_Log()->error("TFer: File( Path = \"%s\") NOT Found.", file_path);
					return Public->read(Public, Attr, ts_ms, data, stack_len);
				}

				/**
				 * ����һ�¸��ļ���ʱ�����
				 */
				Privated->current_file_ts_ms = ExtractTimestampFromFilename(file_path);
				Privated->current_file_ts_ms *= 1000;

				NK_Log()->debug("TFer: Play File( Path = \"%s\" ).", file_path);
			}
		}
	}

	/**
	 * ���ļ��ж�ȡһ֡���ݡ�
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

		/// FIXME �����߼��е����⡣
		if (readn <= 0) {
		//if (0 == readn) {

			NK_Log()->debug("TFer: Read Endof File.");

			/**
			 * ��ȡʧ���ˣ��л�����һ���ļ���
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
 * THIS ָ�붨������\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
 */
#undef THIS

NK_TFerPlayer *
NK_TFer_CreatePlayer(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_UTC1970 play_utc, NK_JSON *PlayFileList, NK_PVoid player_ctx)
{
	NK_PrivatedTFerPlayer *Privated = NK_Nil;
	NK_TFerPlayer *Public = NK_Nil;

	/**
	 * ����ڴ��������
	 */
	if (!Alloctr) {
		Alloctr = NK_MemAlloc_OS();
	}

	/**
	 * ��ʼ�������
	 * ���о�����ڴ�ռ����˽�о���ĸ�λ��
	 * ��Ч��ֹģ�鹫�о�����ⲿ�������ͷš�
	 */
	Privated = Alloctr->alloc(Alloctr, sizeof(NK_PrivatedTFerPlayer) + sizeof(NK_TFerPlayer));
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Privated, NK_Nil);
	Public = (NK_TFerPlayer *)(Privated + 1);

	/**
	 * ��ʼ��˽�г�Ա��
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
	 * ��ʼ�����г�Ա��
	 */
	Public->read = TFerPlayer_read;
	Public->seek = TFerPlayer_seek;

//	NK_Log()->debug("Privated->PlayFileList = %p", Privated->PlayFileList);
//	Privated->PlayFileList->Object.dump(&Privated->PlayFileList->Object);

	/**
	 * ���ع��о����
	 * ˽�о��������ⲿ�����߲��ɼ������ֹ�ⲿ�������д��
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
	 * �ͷ���Ч������ԡ�
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerPlayer_r, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerPlayer_r[0], -1);

	/**
	 * ��ȡ˽�о����
	 */
	Public = TFerPlayer_r[0];
	Privated = PRIVATED(Public);
	TFerPlayer_r[0] = NK_Nil; ///< ����ǰ����վ����ֹ�ϲ��û����ͷŹ�����������á�
	Alloctr = Privated->Alloctr;

	/**
	 * �رյ�ý�岥���ļ���
	 */
	if (NK_Nil != Privated->CurrentFileFLV) {
		NK_FileFLV_Free(&Privated->CurrentFileFLV);
	}

	/**
	 * �ͷ�ģ������
	 */

	Alloctr->freep(Alloctr, Privated);

	/**
	 * �ͷųɹ���
	 */
	return 0;
}

#endif
