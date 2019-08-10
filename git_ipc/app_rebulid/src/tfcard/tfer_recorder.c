
#if 0
#include "tfer_recorder.h" //<! ģ�� TFerRecorder �����ļ���
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
 * TFerRecorder ģ��˽�о�����������ģ���ڲ���˽�г�Ա��\n
 * �ڴ��� TFerRecorder ģ�鴴��ʱͳһ���䡣\n
 * λ���ھ�����ݽṹ @ref NK_TFerRecorder ��λ��\n
 * ������Ч���� @ref NK_TFerRecorder �ڴ�ռ䱻�����ͷš�\n
 * ����ͼ��\n
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
	 * ģ���ڴ��������
	 */
	NK_Allocator *Alloctr;

	/**
	 * TFer �¼�����
	 */
	NK_TFerEventSet *EventSet;

	/**
	 * TFer �¼������ġ�
	 */
	NK_PVoid evt_ctx;

	/**
	 * ¼�����ԡ�
	 */
	struct {
		NK_Char type[32];
		NK_Char fs_path[128];
		NK_Char record_path[128];
		NK_Char file_path[128];
	} Record;

	/**
	 * ¼��ý�����ԡ�
	 */
	struct {
		NK_TFerRecordAttr Video;
		NK_TFerRecordAttr Audio;
	} Attr;

	/**
	 * FLV �ļ���������
	 */
	NK_FileFLV *FileFLV;

	/**
	 * ��ǰ¼��Ự������¼������п��ܻ�������������
	 */
	NK_JSON *Info;

} NK_PrivatedTFerRecorder;

/**
 * ͨ��ģ�鹫�о����ȡ˽�о����
 */
static inline NK_PrivatedTFerRecorder *
PRIVATED(NK_TFerRecorder *Public)
{
	/**
	 * ƫ�Ƶ�˽�о����
	 */
	return (NK_PrivatedTFerRecorder *)Public - 1;
}

static NK_Size
getFreeSpace(NK_Char *fs_path)
{
	struct statvfs StatFS;
	NK_Int64 free_size = 0;

	/**
	 * ʹ��ϵͳ������ȡ�ļ�ϵͳ�ռ��С��
	 */
	if (statvfs(fs_path, &StatFS) < 0) {
		return 0;
	}

	free_size = StatFS.f_bavail;
	free_size *= StatFS.f_bsize;
	free_size /= 1024;
	free_size /= 1024; ///< ת���� MB ��λ��

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
				//��ȡĿ¼��Ϣ
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
 * THIS ָ�붨�壬\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
 */
#define THIS NK_TFerRecorder * const Public


/**
 * д��֮ǰ���ã�����д��ǰ������
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
	 * ��������¼��������
	 */
	if (!Privated->Info) {

		begin_utc = time(NK_Nil);

		/**
		 * ������ǰ��������
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
		 * ����¼��Ŀ¼��
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
		 * ���� FLV �ļ������
		 */
		Privated->FileFLV = NK_FileFLV_Create(Privated->Alloctr, Privated->Record.file_path);
		
		if(NULL == Privated || NULL == Privated->FileFLV)
		{
			printf("NK_FileFLV_Create\n");
			return ;
		}
		/**
		 * ������Ƶ���ԣ����� FLV �ļ����ԡ�
		 */
		do {
			NK_FLVTagCodecID codec_id = NK_FLV_TAG_CODEC_ID_AVC;

			/**
			 * ���� Codec ID��
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
			 * ���� Sound Format��
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
		 * �����ļ�������
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
				 * �����������ļ��뵱ǰ�ļ���һ����Ҫ�����ļ���
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
		 * �رվ����
		 */
		NK_TFer_CloseIndexer(VarIndexer);
		VarIndexer = NK_Nil;
	}
}


/**
 *
 * @return		���� True �����֡�����ļ�����ٽ�������Ҫ��д��
 */
static NK_Boolean
afterWrite(THIS, NK_Boolean seekable)
{
	NK_PrivatedTFerRecorder *Privated = PRIVATED(Public);
	NK_Char text[128];
	NK_Char end_date[32], end_time[32];
	NK_UTC1970 end_utc = 0;

	/**
	 * �����ɼ���������Ӧ������
	 */
	if (seekable) {

		/**
		 * ���� Info ����ʱ�䡣
		 */
		end_utc = time(NK_Nil);
		NK_TFer_LocalTime(end_utc, end_date, end_time);
		snprintf(text, sizeof(text), "%s %s", end_date, end_time);
		NK_JSON_SetItemInObject(Privated->Info, "end_time", NK_JSON_CreateString(Privated->Alloctr, text));
		NK_JSON_SetItemInObject(Privated->Info, "end_utc", NK_JSON_CreateNumber(Privated->Alloctr, end_utc));
//		Privated->Info->Object.dump(&Privated->Info->Object);

		/**
		 * ����ÿ 8M �ļ���һ������
		 */
		if (Privated->FileFLV->size(Privated->FileFLV) >= 1024 * 1024 * 8) {

			/**
			 * �ر��ļ���
			 */
			NK_FileFLV_Free(&Privated->FileFLV);

			/**
			 * ����������
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
 * THIS ָ�붨������\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
 */
#undef THIS


NK_TFerRecorder *
NK_TFer_CreateRecorder(NK_Allocator *Alloctr, NK_TFerEventSet *EventSet, NK_PVoid evt_ctx,
		NK_PChar type, NK_PChar fs_path, NK_PVoid recorder_ctx)
{
	NK_PrivatedTFerRecorder *Privated = NK_Nil;
	NK_TFerRecorder *Public = NK_Nil;

	/**
	 * ����ڴ��������
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Alloctr, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != fs_path, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(strlen(fs_path) > 0, NK_Nil);

	/**
	 * ��ʼ�������
	 * ���о�����ڴ�ռ����˽�о���ĸ�λ��
	 * ��Ч��ֹģ�鹫�о�����ⲿ�������ͷš�
	 */
	Privated = Alloctr->alloc(Alloctr, sizeof(NK_PrivatedTFerRecorder) + sizeof(NK_TFerRecorder));
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Privated, NK_Nil);
	Public = (NK_TFerRecorder *)(Privated + 1);

	/**
	 * ��ʼ��˽�г�Ա��
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
	 * ��ʼ�����г�Ա��
	 */
	Public->setVideoAttr = TFerRecorder_setVideoAttr;
	Public->setAudioAttr = TFerRecorder_setAudioAttr;
	Public->writeH264 = TFerRecorder_writeH264;
	Public->writeG711 = TFerRecorder_writeG711;

	/**
	 * ���ع��о����
	 * ˽�о��������ⲿ�����߲��ɼ������ֹ�ⲿ�������д��
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
	 * �ͷ���Ч������ԡ�
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerRecorder_r, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFerRecorder_r[0], -1);

	/**
	 * ��ȡ˽�о����
	 */
	Public = TFerRecorder_r[0];
	Privated = PRIVATED(Public);
	TFerRecorder_r[0] = NK_Nil; ///< ����ǰ����վ����ֹ�ϲ��û����ͷŹ�����������á�
	Alloctr = Privated->Alloctr;

	/**
	 * �������ݿ⵽�ļ�ϵͳ��
	 */
	flushIndexer(Public);

	/**
	 * �ر��ļ���
	 */
	if (NK_Nil != Privated->FileFLV) {
		NK_FileFLV_Free(&Privated->FileFLV);
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
