
#if 0
#include "tfer_recorder.h" //<! 模块 TFerRecorder 定义文件。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <NkUtils/assert.h>
#include <NkUtils/macro.h>
#include <NkUtils/json.h>
#include <base/ja_process.h>
#ifdef _WIN32
#include "..\JaSDK_vs_pro\dirent.h"
#else
#include <dirent.h>
#endif
#include "tfer_indexer.h"


/**
 * TFerRecorder 模块私有句柄，句柄访问模块内部的私有成员。\n
 * 内存在 TFerRecorder 模块创建时统一分配。\n
 * 位置在句柄数据结构 @ref NK_TFerRecorder 上位，\n
 * 这样有效避免 @ref NK_TFerRecorder 内存空间被错误释放。\n
 * 如下图：\n
 *
 *  | NK_PrivatedTFerRecorder
 * \|/
 *  +------------------------+
 *  |          |             |
 *  |          |             |
 *  +------------------------+
 *            /|\
 *             | NK_TFerRecorder
 *
 */
typedef struct nkPrivatedTFerRecorder {

	/**
	 * 模块内存分配器。
	 */
	NK_Allocator *Alloctr;

	/**
	 * TFer 事件集。
	 */
	NK_TFerEventSet *EventSet;

	/**
	 * TFer 事件上下文。
	 */
	NK_PVoid evt_ctx;

	/**
	 * 录像属性。
	 */
	struct {
		NK_Char type[32];
		NK_Char fs_path[128];
		NK_Char record_path[128];
		NK_Char file_path[128];
	} Record;

	/**
	 * 录制媒体属性。
	 */
	struct {
		NK_TFerRecordAttr Video;
		NK_TFerRecordAttr Audio;
	} Attr;

	/**
	 * FLV 文件描述符。
	 */
	NK_FileFLV *FileFLV;

	/**
	 * 当前录像会话索引，录像过程中可能会产生多个索引。
	 */
	NK_JSON *Info;

} NK_PrivatedTFerRecorder;

/**
 * 通过模块公有句柄获取私有句柄。
 */
static inline NK_PrivatedTFerRecorder *
PRIVATED(NK_TFerRecorder *Public)
{
	/**
	 * 偏移到私有句柄。
	 */
	return (NK_PrivatedTFerRecorder *)Public - 1;
}

static NK_Size
getFreeSpace(NK_Char *fs_path)
{
	struct statvfs StatFS;
	NK_Int64 free_size = 0;

	/**
	 * 使用系统方法获取文件系统空间大小。
	 */
	if (statvfs(fs_path, &StatFS) < 0) {
		return 0;
	}

	free_size = StatFS.f_bavail;
	free_size *= StatFS.f_bsize;
	free_size /= 1024;
	free_size /= 1024; ///< 转换成 MB 单位。

	return (NK_Size)free_size;
}

static NK_Int
recylceOneDir(NK_Char *file_path)
{
	DIR *dirptr = NULL;
	struct dirent *entry;
	char path[60] = {0};
	char recycle_path[60] = {0};
	char cmd_text[80]={0};
	char right_path[50] = {0};

	sprintf(right_path, "%s/record", file_path);
	if((dirptr = opendir(right_path)) == NULL)
	{
		return -1;
	}
	else
	{
		time_t pre_time = 0;
		struct stat buf;
		int result=-1;

		while (entry = readdir(dirptr))
		{
			if(entry->d_name[0] == '.')
			{
				continue;
			}

			if(entry->d_name){
				memset(path, 0, 60);
				sprintf(path, "%s/%s", right_path, entry->d_name);
				//获取目录信息
				result = stat(path, &buf);
				if(result != 0){
					continue;
				}

				if(pre_time == 0){
					pre_time = buf.st_mtime;
					sprintf(recycle_path, "%s/%s", right_path, entry->d_name);
				}
				if(buf.st_mtime <= pre_time)
				{
					memset(recycle_path, 0, sizeof(recycle_path));
					sprintf(recycle_path, "%s/%s", right_path, entry->d_name);
					pre_time = buf.st_mtime;
				}

				if(NULL == path){
					sprintf(recycle_path, "%s/%s", right_path, entry->d_name);
				}
			}
		}
		closedir(dirptr);

		if(path){
			sprintf(cmd_text, "rm -rf %s", recycle_path);
			NK_SYSTEM(cmd_text);
		}
	}
	return 0;
}

/**
 * THIS 指针定义，\n
 * 定义以下为模块 API 接口实现。
 */
#define THIS NK_TFerRecorder * const Public


/**
 * 写入之前调用，处理写入前操作。
 *
 */
static NK_Void
beforeWrite(THIS, NK_Boolean seekable)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);
	NK_UTC1970 begin_utc = 0;
	NK_Char begin_date[64], begin_time[64];
	NK_Char text[128];
	NK_SSize free_size=0;

	/**
	 * 创建本段录像索引。
	 */
	if (!Privated->Info) {

		begin_utc = time(NK_Nil);

		/**
		 * 创建当前段索引。
		 */
		Privated->Info = NK_JSON_CreateObject(Privated->Alloctr);

		NK_JSON_AddStringToObject(Privated->Info, "type", Privated->Record.type);
		NK_TFer_LocalTime(begin_utc, begin_date, begin_time);
		snprintf(text, sizeof(text), "%s %s", begin_date, begin_time);
		NK_JSON_AddStringToObject(Privated->Info, "begin_time", text);
		NK_JSON_AddNumberToObject(Privated->Info, "begin_utc", begin_utc);
		NK_JSON_AddStringToObject(Privated->Info, "end_time", text);
		NK_JSON_AddNumberToObject(Privated->Info, "end_utc", begin_utc);
		NK_JSON_AddItemToObject(Privated->Info, "File", NK_JSON_CreateArray(Privated->Alloctr));
		Privated->Info->Object.dump(&Privated->Info->Object);
	}

	if (!Privated->FileFLV) {

		begin_utc = time(NK_Nil);

		/**
		 * 创建录像目录。
		 */
		NK_TFer_LocalTime(begin_utc, begin_date, begin_time);
		snprintf(Privated->Record.record_path, sizeof(Privated->Record.record_path), "%s/record/%s/",
				Privated->Record.fs_path, begin_date);
		//NK_Log()->debug("TFer: Make Record DIR( Path = \"%s\" ).", Privated->Record.record_path);
		snprintf(text, sizeof(text), "mkdir -p \"%s\"", Privated->Record.record_path);
		NK_SYSTEM(text);
		//snprintf(Privated->Record.file_path, sizeof(Privated->Record.file_path), "%s/%s.flv", Privated->Record.record_path, begin_time);
		snprintf(Privated->Record.file_path, sizeof(Privated->Record.file_path), "%s%s.flv", Privated->Record.record_path, begin_time);

		//undercapacity, recylce one dir
		free_size = getFreeSpace(Privated->Record.fs_path);
		if(free_size <= 64){
			recylceOneDir(Privated->Record.fs_path);
			return;
		}

		/**
		 * 创建 FLV 文件句柄。
		 */
		Privated->FileFLV = NK_FileFLV_Create(Privated->Alloctr, Privated->Record.file_path);
		
		if(NULL == Privated || NULL == Privated->FileFLV)
		{
			printf("NK_FileFLV_Create\n");
			return ;
		}
		/**
		 * 设置视频属性，适配 FLV 文件属性。
		 */
		do {
			NK_FLVTagCodecID codec_id = NK_FLV_TAG_CODEC_ID_AVC;

			/**
			 * 适配 Codec ID。
			 */
			if (NK_TFER_VCODEC_H264 == Privated->Attr.Video.codec) {
				codec_id = NK_FLV_TAG_CODEC_ID_AVC;
			} else {
				codec_id = NK_FLV_TAG_CODEC_ID_AVC;
			}

			Privated->FileFLV->setVideoAttr(Privated->FileFLV,
					codec_id,
					Privated->Attr.Video.width, Privated->Attr.Video.height,
					Privated->Attr.Video.frame_rate, 0);
		} while (0);

		do {
			NK_FLVTagSoundFormat sound_format = NK_FLV_TAG_SOUND_FMT_G711_ALAW;

			/**
			 * 适配 Sound Format。
			 */
			if (NK_TFER_ACODEC_G711A == Privated->Attr.Audio.codec) {
				sound_format = NK_FLV_TAG_SOUND_FMT_G711_ALAW;
			} else if (NK_TFER_ACODEC_G711A == Privated->Attr.Audio.codec) {
				sound_format = NK_FLV_TAG_SOUND_FMT_G711_ULAW;
			} else {
				sound_format = NK_FLV_TAG_SOUND_FMT_G711_ALAW;
			}

			Privated->FileFLV->setAudioAttr(Privated->FileFLV,
					sound_format,
					Privated->Attr.Audio.stereo,
					Privated->Attr.Audio.sample_bitwidth,
					Privated->Attr.Audio.sample_rate);

		} while (0);
	}
}


static NK_Void
flushIndexer(THIS)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);
	NK_JSON *VarIndexer = NK_Nil;
	NK_JSON *VarInfo = NK_Nil;
	NK_JSON *VarFileArray = NK_Nil;
	NK_Size file_array_len = 0;
	NK_JSON *VarFileName = NK_Nil;
	NK_PChar last_name = NK_Nil;
	NK_PChar file_name = NK_Nil;

	VarIndexer = NK_TFer_OpenIndexer(Privated->Alloctr, Privated->Record.record_path);
	if (NK_Nil != VarIndexer) {

		/**
		 * 更新文件索引。
		 */
		file_name = strrchr(Privated->Record.file_path, '/');
		if (!file_name) {
			file_name = Privated->Record.file_path;
		} else {
			++file_name;
		}

		VarFileArray = NK_JSON_IndexOfName(Privated->Info, "File");
		file_array_len = NK_JSON_ArraySize(VarFileArray);
		if (0 == file_array_len) {
			NK_JSON_AddItemToArray(VarFileArray, NK_JSON_CreateString(Privated->Alloctr, file_name));
		} else {

			VarFileName = NK_JSON_IndexOf(VarFileArray, file_array_len - 1);
			last_name = NK_JSON_GetString(VarFileName, "");
			if (!NK_STRCMP(last_name, file_name)) {
				/**
				 * 如果最后索引文件与当前文件不一样须要更新文件。
				 */
				NK_JSON_AddItemToArray(VarFileArray, NK_JSON_CreateString(Privated->Alloctr, file_name));
			}
		}

		VarInfo = NK_JSON_Duplicate(Privated->Alloctr, Privated->Info);
		if (0 != NK_TFer_AddInfoToIndexer(VarIndexer, VarInfo)) {
			VarInfo->Object.free(&VarInfo->Object);
		}
		NK_TFer_FlushIndexer(VarIndexer, Privated->Record.record_path);

		/**
		 *limit maximum(10) files in a field.
		 */
		if(file_array_len >= 9){
			Privated->Info = NK_Nil;
		}
		/**
		 * 关闭句柄。
		 */
		NK_TFer_CloseIndexer(VarIndexer);
		VarIndexer = NK_Nil;
	}
}


/**
 *
 * @return		返回 True 代表此帧处于文件封包临界区，须要重写。
 */
static NK_Boolean
afterWrite(THIS, NK_Boolean seekable)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);
	NK_Char text[128];
	NK_Char end_date[32], end_time[32];
	NK_UTC1970 end_utc = 0;

	/**
	 * 遇到可检索作出相应操作。
	 */
	if (seekable) {

		/**
		 * 更新 Info 结束时间。
		 */
		end_utc = time(NK_Nil);
		NK_TFer_LocalTime(end_utc, end_date, end_time);
		snprintf(text, sizeof(text), "%s %s", end_date, end_time);
		NK_JSON_SetItemInObject(Privated->Info, "end_time", NK_JSON_CreateString(Privated->Alloctr, text));
		NK_JSON_SetItemInObject(Privated->Info, "end_utc", NK_JSON_CreateNumber(Privated->Alloctr, end_utc));
//		Privated->Info->Object.dump(&Privated->Info->Object);

		/**
		 * 考虑每 8M 文件打一个包。
		 */
		if (Privated->FileFLV->size(Privated->FileFLV) >= 1024 * 1024 * 8) {

			/**
			 * 关闭文件。
			 */
			NK_FileFLV_Free(&Privated->FileFLV);

			/**
			 * 更新索引。
			 */
			flushIndexer(Public);
			return NK_True;
		}
	}

	return NK_False;
}


static NK_Int
TFerRecorder_setVideoAttr(THIS, NK_TFerRecordAttr *Attr)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);

	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Attr, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_TFER_VCODEC_H264 == Attr->codec, -1);
//	NK_EXPECT_VERBOSE(width > 0);
//	NK_EXPECT_VERBOSE(height > 0);
//	NK_EXPECT_VERBOSE(frame_rate > 0);

	memcpy(&Privated->Attr.Video, Attr, sizeof(NK_TFerRecordAttr));
	return 0;
}


static NK_Int
TFerRecorder_setAudioAttr(THIS, NK_TFerRecordAttr *Attr)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);

	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Attr, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_TFER_ACODEC_G711A == Attr->codec || NK_TFER_ACODEC_G711U == Attr->codec, -1);
//	NK_EXPECT_VERBOSE(!stereo);
//	NK_EXPECT_VERBOSE(0 == (sample_bitwidth % 8));
//	NK_EXPECT_VERBOSE(sample_rate > 0);

	memcpy(&Privated->Attr.Audio, Attr, sizeof(NK_TFerRecordAttr));
	return 0;
}


static NK_SSize
TFerRecorder_writeH264(THIS, NK_UInt64 ts_ms, NK_Boolean seekable, NK_PVoid data, NK_Size len)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);
	NK_SSize writen = 0;

	beforeWrite(Public, seekable);
	if(NULL == Privated || NULL == Privated->FileFLV)
	{
		printf("beforeWrite\n");
		return -1;
	}
	writen = Privated->FileFLV->writeH264(Privated->FileFLV, ts_ms, seekable, data, len);
	afterWrite(Public, seekable);

	if (writen == len) {
		return len;
	}

	return -1;
}


static NK_SSize
TFerRecorder_writeG711(THIS, NK_UInt64 ts_ms, NK_PVoid data, NK_Size len)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);
	NK_SSize writen = 0;

	beforeWrite(Public, NK_False);
	if(NULL == Privated || NULL == Privated->FileFLV)
	{
		printf("TFerRecorder_writeG711-beforeWrite\n");
		return -1;
	}

	writen = Privated->FileFLV->writeG711a(Privated->FileFLV, ts_ms, data, len);
	afterWrite(Public, NK_False);

	if (writen == len) {
		return len;
	}

	return -1;
}


/**
 * THIS 指针定义解除，\n
 * 定义以上为模块 API 接口实现。
 */
#undef THIS


NK_TFerRecorder *
NK_TFer_CreateRecorder(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_PChar type, NK_PChar fs_path, NK_PVoid recorder_ctx)
{
	NK_PrivatedTFerRecorder *Privated = NK_Nil;
	NK_TFerRecorder *Public = NK_Nil;

	/**
	 * 检查内存分配器。
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Alloctr, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != fs_path, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(strlen(fs_path) > 0, NK_Nil);

	/**
	 * 初始化句柄。
	 * 公有句柄的内存空间仅靠私有句柄的高位。
	 * 有效防止模块公有句柄在外部被意外释放。
	 */
	Privated = Alloctr->alloc(Alloctr, sizeof(NK_PrivatedTFerRecorder) + sizeof(NK_TFerRecorder));
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Privated, NK_Nil);
	Public = (NK_TFerRecorder *)(Privated + 1);

	/**
	 * 初始化私有成员。
	 */
	Privated->Alloctr = Alloctr;
	Privated->EventSet = EventSet;
	Privated->evt_ctx = evt_ctx;
	NK_BZERO(&Privated->Record, sizeof(Privated->Record));
	snprintf(Privated->Record.type, sizeof(Privated->Record.type), "%s", NK_Nil != type ? type : "default");
	snprintf(Privated->Record.fs_path, sizeof(Privated->Record.fs_path), "%s", fs_path);
	NK_BZERO(&Privated->Attr, sizeof(Privated->Attr));
	Privated->FileFLV = NK_Nil;
	Privated->Info = NK_Nil;


	/**
	 * 初始化公有成员。
	 */
	Public->setVideoAttr = TFerRecorder_setVideoAttr;
	Public->setAudioAttr = TFerRecorder_setAudioAttr;
	Public->writeH264 = TFerRecorder_writeH264;
	Public->writeG711 = TFerRecorder_writeG711;

	/**
	 * 返回公有句柄。
	 * 私有句柄存放在外部调用者不可见区域防止外部被错误读写。
	 */
	return Public;
}

NK_Int
NK_TFer_FreeRecorder(NK_TFerRecorder **TFerRecorder_r)
{
	NK_TFerRecorder *Public = NK_Nil;
	NK_PrivatedTFerRecorder *Privated = NK_Nil;
	NK_Allocator *Alloctr = NK_Nil;

	/**
	 * 释放无效句柄断言。
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerRecorder_r, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerRecorder_r[0], -1);

	/**
	 * 获取私有句柄。
	 */
	Public = TFerRecorder_r[0];
	Privated = PRIVATED(Public);
	TFerRecorder_r[0] = NK_Nil; ///< 操作前先清空句柄防止上层用户在释放过程中意外调用。
	Alloctr = Privated->Alloctr;

	/**
	 * 更新数据库到文件系统。
	 */
	flushIndexer(Public);

	/**
	 * 关闭文件。
	 */
	if (NK_Nil != Privated->FileFLV) {
		NK_FileFLV_Free(&Privated->FileFLV);
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
