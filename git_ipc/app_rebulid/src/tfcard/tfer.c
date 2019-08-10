#if 0
#include <NkEmbedded/tfer.h> //<! ģ�� TFer �����ļ���
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <NkUtils/assert.h>
#include <NkUtils/macro.h>
#include <NkUtils/json.h>
#include <NkEmbedded/thread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _WIN32
#include "..\JaSDK_vs_pro\dirent.h"
#else
#include <dirent.h>
#endif
#include "tfer_indexer.h"
#include "tfer_recorder.h"
#include "tfer_player.h"

#define FILE_EXIST(__file_path) (-1 != access(__file_path))


/**
 * TFer ģ��˽�о�����������ģ���ڲ���˽�г�Ա��\n
 * �ڴ��� TFer ģ�鴴��ʱͳһ���䡣\n
 * λ���ھ�����ݽṹ @ref NK_TFer ��λ��\n
 * ������Ч���� @ref NK_TFer �ڴ�ռ䱻�����ͷš�\n
 * ����ͼ��\n
 *
 *  | NK_PrivatedTFer
 * \|/
 *  +------------------------+
 *  |          |             |
 *  |          |             |
 *  +------------------------+
 *            /|\
 *             | NK_TFer
 *
 */
typedef struct nkPrivatedTFer {

	/**
	 * ģ���ڴ��������
	 */
	NK_Allocator *Alloctr;


	/**
	 * �¼����ϡ�
	 */
	NK_TFerEventSet EventSet;

	/**
	 * �¼������ġ�
	 */
	NK_PVoid event_ctx;

	/**
	 * TF ������������
	 */
	NK_Size tf_slots;

	/**
	 * ��ǰ¼�����͡�
	 */
	NK_Char record_type[16];

	/**
	 * ��ǰ�������͡�
	 */
	NK_Char play_type[16];

	NK_JSON *PlayFileList;


	/**
	 * ��ǰ������ʼʱ�����
	 */
	NK_UTC1970 play_utc;

	/**
	 * TF ����ʶ��
	 */
	struct {

		NK_Boolean mounted;
		NK_Char fs_path[32];

	} TFSlot[8];

	/**
	 * ����߳̾����
	 */
	NK_Thread *Detector;

	/**
	 * ¼���߳̾����
	 */
	NK_Thread *Recorder;

	/**
	 * �����߳̾����
	 */
	NK_Thread *Player;

	/**
	 * 
	 */
	pthread_mutex_t Mutex;

} NK_PrivatedTFer;

/**
 * ͨ��ģ�鹫�о����ȡ˽�о����
 */
static inline NK_PrivatedTFer *
PRIVATED(NK_TFer *Public)
{
	/**
	 * ƫ�Ƶ�˽�о����
	 */
	return (NK_PrivatedTFer *)Public - 1;
}


/**
 * THIS ָ�붨�壬\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
 */
#define THIS NK_TFer * const Public

/**
 * ��ȡģ�����ơ�
 */
static NK_PChar
Object_name(NK_Object *Obj)
{
	return "TFer";
}

/**
 * ���Դ�ӡģ�顣
 */
static NK_Void
Object_dump(NK_Object *Obj)
{
}

/**
 * ģ��ӿ��̰߳�ȫ��ʶ��
 */
static NK_Boolean
Object_threadsafe(NK_Object *Obj)
{
	return NK_False;
}

/**
 * ģ���ͷš�
 */
static NK_Void
Object_free(NK_Object *Obj)
{
	NK_TFer_Free((NK_TFer **)(&Obj));
}


static NK_Void
detector(NK_Thread *Thread, NK_Integer argc, NK_PVoid argv[])
{
	NK_TFer *Public = (NK_TFer *)(argv[0]);
	NK_PrivatedTFer *Privated = PRIVATED(Public);

	while (!Thread->terminate(Thread)) {

		NK_Boolean const detected = Privated->EventSet.onDetectTF(Privated->event_ctx, 0);
		NK_Size free_size = 0;

		/**
		 * ��ǰ TF ���ϸ������Ѿ����ӵ���ʱ��ⲻ�� TF �����ڡ�
		 * �п��� TF ���Ѿ���ǿ�в�ж��
		 */
		if (detected && !Privated->TFSlot[0].mounted) {

			/**
			 * ��鵽 TF �����ҵ�ǰδ���ء�
			 */
			if (0 == Privated->EventSet.onMountTF(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path)) {

				free_size = Privated->EventSet.onGetFreeSpace(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
				Privated->TFSlot[0].mounted = NK_True;
				NK_Log()->debug("TFer: Card Mounted( Path = \"%s\" Free = %uMB ).",
						Privated->TFSlot[0].fs_path, free_size);
			}
		}

		/**
		 * ��ǰ TF ��δ���ӵ��Ǵ�ʱ��⵽ TF �����ڣ��п����û��ղ��� TF ����
		 */
		if (!detected && Privated->TFSlot[0].mounted) {

			/**
			 * ��鲻�� TF ����ǰ�ѹ����ˡ�
			 */
			Privated->EventSet.onUmountTF(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
			Privated->TFSlot[0].mounted = NK_False;
			NK_Log()->debug("TFer: Card Umounted( Path = \"%s\" ).", Privated->TFSlot[0].fs_path);
		}

		/// ÿ�� 1 ��̽��һ�� TF ������״̬��
		Thread->suspend(Thread, 1, 0, 0);
	}
}

static NK_Int 
recorder_recylceOneFile(NK_Char *file_path)
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
			system(cmd_text);
			printf("cmd_text = %s\n", cmd_text);
		}
	}
	return 0;
}


/**
 * ��̨¼���̡߳�
 *
 */
static NK_Void
recorder(NK_Thread *Thread, NK_Integer argc, NK_PVoid argv[])
{
	NK_TFer *Public = argv[0];
	NK_PrivatedTFer *Privated = PRIVATED(Public);
	NK_PVoid recorder_ctx = argv[1];
	NK_PChar fs_path = NK_Nil;
	NK_TFerRecorder *TFerRecorder = NK_Nil;
	NK_SSize free_space = 0;

	/**
	 * ���ﲻ���˳��̣߳���������������й©��
	 */
	while (!Thread->terminate(Thread)) {

		/**
		 * �ж��Ƿ�ʵ��¼���¼���
		 */
		if (NK_Nil != Privated->EventSet.onRecord) {

			/**
			 * �ж� TF ���Ѿ����ء�
			 */
			if (Privated->TFSlot[0].mounted) {

				/**
				 * ���վ��ļ��ͷ��㹻�ռ��ڳ����˴�¼��
				 */
				free_space = Privated->EventSet.onGetFreeSpace(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
				while (!Thread->terminate(Thread) && free_space < 64) {
					/// ÿ�� TF ��ʣ������С��һ���ٽ�ֵʱ���Ի���һ��Сʱ��¼��
					/// �ظ��˶���һֱ��¼�����㹻�ռ�¼��Ϊֹ��
					if(-1 == recorder_recylceOneFile(Privated->TFSlot[0].fs_path)){
						printf("TFer: TFCARD no space\n");
						return;
					}
					free_space = Privated->EventSet.onGetFreeSpace(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
					Thread->suspend(Thread, 0, 5, 0);
				}

				/**
				 * ��ʼ¼�������
				 */
				fs_path = Privated->TFSlot[0].fs_path;

				/**
				 * ������ý��¼���ļ���
				 */
				TFerRecorder = NK_TFer_CreateRecorder(Privated->Alloctr, &Privated->EventSet, Privated->event_ctx,
						"default", fs_path, recorder_ctx);
				if (!TFerRecorder) {
					NK_Log()->error("TFer: Create Recorder( Path = \"%s\") Failed.", fs_path);
					return;
				}

				NK_Log()->debug("TFer: Start Record on File( Path = \"%s\" ).", fs_path);
				Privated->EventSet.onRecord(Privated->event_ctx, Thread, TFerRecorder, recorder_ctx);
				NK_Log()->debug("TFer: Stop Record on File( Path = \"%s\" ).", fs_path);

				/**
				 * �رն�ý��¼���ļ���
				 */
				NK_TFer_FreeRecorder(&TFerRecorder);

			} else {
				NK_Log()->error("TFer: TF Card NOT Mounted.");
				Thread->suspend(Thread, 1, 0, 0);
			}
		} else {
			NK_Log()->error("TFer: Record Event Active Failed.");
			Thread->suspend(Thread, 1, 0, 0);
		}
	}
}

/**
 * ��̨�����̡߳�
 *
 */
static NK_Void
player(NK_Thread *Thread, NK_Integer argc, NK_PVoid argv[])
{
	NK_TFer *Public = argv[0];
	NK_PrivatedTFer *Privated = PRIVATED(Public);
	NK_JSON *PlayFileList = (NK_JSON *)(argv[1]);
	NK_TFerPlayer *Player = NK_Nil;
	NK_PVoid player_ctx = argv[2];

	NK_EXPECT_VERBOSE(NK_Nil != Privated->EventSet.onPlay);

	Player = NK_TFer_CreatePlayer(Privated->Alloctr, &Privated->EventSet, Privated->event_ctx,
			Privated->play_utc, PlayFileList, player_ctx);
	
	NK_Log()->info("TFer: Start Play ( Type = \"%s\" UTC = %u ).", Privated->play_type, Privated->play_utc);
	printf("TFer: Start Play ( Type = \"%s\" UTC = %u ).\n", Privated->play_type, Privated->play_utc);

	Privated->EventSet.onPlay(Privated->event_ctx, Thread, Player, player_ctx);
	printf("TFer: Stop Play ( Type = \"%s\" UTC = %u ).\n", Privated->play_type, Privated->play_utc);
	NK_TFer_FreePlayer(&Player);
}

static NK_Int
TFer_record(THIS, NK_Boolean start, NK_PChar type, NK_PVoid recorder_ctx)
{
	NK_PrivatedTFer *Privated = PRIVATED(Public);
	NK_Int argc = 0;
	NK_PVoid argv[32];

	if (start && NK_Nil == Privated->Recorder) {

		if (!type) {
			type = "default";
		}

		snprintf(Privated->record_type, sizeof(Privated->record_type), "%s", type);

		/**
		 * ������̨����̡߳�
		 */
		argc = 0;
		argv[argc++] = (NK_PVoid)(Public);
		argv[argc++] = (NK_PVoid)(recorder_ctx);
		Privated->Recorder = NK_Thread_Create(Privated->Alloctr, NK_False, NK_False,
				recorder, argc, argv);

	}

	if (!start && NK_Nil != Privated->Recorder) {

		NK_Thread_Free(&Privated->Recorder);
		Privated->Recorder = NK_Nil;

	}

	return 0;
}

/**
 * ��ȡ��ʷ��Ϣ��
 */
static NK_Int
TFer_history(THIS, NK_UTC1970 utc, NK_PChar type, NK_TFerHistory *History, NK_Size *n)
{
	NK_PrivatedTFer *Privated = PRIVATED(Public);
	NK_Char str_date[32], str_time[32];
	NK_Char record_path[128];
	NK_JSON *Indexer = NK_Nil;
	NK_Int ret = -1;

	NK_TFer_LocalTime(utc, str_date, str_time);
	if (0 == strlen(Privated->TFSlot[0].fs_path) || NULL == Privated->TFSlot[0].fs_path) {
		Privated->EventSet.onMountTF(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
	}
	snprintf(record_path, sizeof(record_path),
			"%s/record/%s", Privated->TFSlot[0].fs_path, str_date);

	NK_Log()->debug("TFer: History Lookup( Path = \"%s\" ).", record_path);

	Indexer = NK_TFer_OpenIndexer(Privated->Alloctr, record_path);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Indexer, -1);

	ret = NK_TFer_History(Indexer, utc, type, History, n, NK_Nil);
	NK_TFer_CloseIndexer(Indexer);
	Indexer = NK_Nil;

	return ret;
}

/**
 * ����¼����/�رա�
 */
static NK_Int
TFer_play(THIS, NK_Boolean start, NK_PChar type, NK_UTC1970 utc, NK_PVoid player_ctx)
{
	NK_PrivatedTFer *Privated = PRIVATED(Public);
	NK_Char record_path[128];
	NK_Char str_date[32], str_time[32];
	NK_JSON *Indexer = NK_Nil;
	NK_Int argc = 0;
	NK_PVoid argv[32];
	NK_Int i = 0;


	if (0 != pthread_mutex_trylock(&Privated->Mutex)) {
		return -1;
	}

	/**
	 * Ĭ�ϲ������͡�
	 */
	if (!type) {
		type = "default";
	}

	/**
	 * �������š�
	 */
	if (start && NK_Nil == Privated->Player) {

		Privated->play_utc = utc;
		snprintf(Privated->play_type, sizeof(Privated->play_type), "%s", type);

		/**
		 * ���ͷ���һ�β����ļ��б�
		 */
		if (NK_Nil != Privated->PlayFileList) {
			NK_JSON_Free(&Privated->PlayFileList);
		}

		NK_TFer_LocalTime(utc, str_date, str_time);
		snprintf(record_path, sizeof(record_path),
				"%s/record/%s", Privated->TFSlot[0].fs_path, str_date);

		Indexer = NK_TFer_OpenIndexer(Privated->Alloctr, record_path);
		NK_EXPECT_VERBOSE_RETURN(NK_Nil != Indexer, -1);

		NK_TFer_History(Indexer, utc, type, NK_Nil, NK_Nil, &Privated->PlayFileList);
		NK_TFer_CloseIndexer(Indexer);
		Indexer = NK_Nil;

		/**
		 * �ɹ������ļ��б�
		 */
		if (NK_Nil != Privated->PlayFileList) {

			NK_Size const file_list_len = NK_JSON_ArraySize(Privated->PlayFileList);
			NK_Char abs_path[128];
			NK_PChar file_path = NK_Nil;
			NK_JSON *VarFilePath = NK_Nil;

			if (file_list_len > 0) {
				/**
				 * ���²����б�·���ɾ���·����
				 */
				 
				for (i = 0; i < file_list_len; ++i) {
					VarFilePath = NK_JSON_IndexOf(Privated->PlayFileList, i);
					file_path = NK_JSON_GetString(VarFilePath, NK_Nil);
					if (NK_Nil != file_path) {
						snprintf(abs_path, sizeof(abs_path), "%s/%s", record_path, file_path);
						NK_JSON_SetItemInArray(Privated->PlayFileList, i,
								NK_JSON_CreateString(NK_JSON_Allocator(Privated->PlayFileList), abs_path));
					}
				}

				/**
				 * ������̨����̡߳�
				 */
				argc = 0;
				argv[argc++] = (NK_PVoid)(Public);
				argv[argc++] = (NK_PVoid)(Privated->PlayFileList);
				argv[argc++] = (NK_PVoid)(player_ctx);
				Privated->Player = NK_Thread_Create(Privated->Alloctr, NK_False, NK_False,
						player, argc, argv);

				pthread_mutex_unlock(&Privated->Mutex);
				return 0;
			}
		}

		NK_Log()->warn("TFer: NO Record to Play.");
	}

	/**
	 * ֹͣ���š�
	 */
	if (!start && NK_Nil != Privated->Player) {

		NK_Thread_Free(&Privated->Player);
		Privated->Player = NK_Nil;

		/**
		 * �ͷŲ����ļ��б�
		 */
		if (NK_Nil != Privated->PlayFileList) {
			NK_JSON_Free(&Privated->PlayFileList);
		}

		pthread_mutex_unlock(&Privated->Mutex);
		return 0;
	}


	pthread_mutex_unlock(&Privated->Mutex);

	return -1;
}


/**
 * THIS ָ�붨������\n
 * ��������Ϊģ�� API �ӿ�ʵ�֡�
 */
#undef THIS

NK_TFer *
NK_TFer_Create2(NK_Allocator *Alloctr, NK_Size slots, NK_TFerEventSet *EventSet, NK_PVoid event_ctx)
{
	NK_PrivatedTFer *Privated = NK_Nil;
	NK_TFer *Public = NK_Nil;
//	NK_Int i = 0;
	NK_Int argc = 0;
	NK_PVoid argv[32];

	/**
	 * �����Ϸ��Լ�顣
	 */
	NK_EXPECT_VERBOSE_RETURN(slots > 0, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(slots <= 8, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet->onDetectTF, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet->onMountTF, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet->onUmountTF, NK_Nil);

	/**
	 * ����ڴ��������
	 */
	if (!Alloctr) {
		Alloctr = NK_MemAlloc_OS();
	}
	//NK_EXPECT_VERBOSE_RETURN(NK_Nil != Alloctr, NK_Nil);

	/**
	 * ��ʼ�������
	 * ���о�����ڴ�ռ����˽�о���ĸ�λ��
	 * ��Ч��ֹģ�鹫�о�����ⲿ�������ͷš�
	 */
	Privated = Alloctr->alloc(Alloctr, sizeof(NK_PrivatedTFer) + sizeof(NK_TFer));
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Privated, NK_Nil);
	Public = (NK_TFer *)(Privated + 1);

	/**
	 * ��ʼ��˽�г�Ա��
	 */
	Privated->Alloctr = Alloctr;
	Privated->Detector = NK_Nil;
	Privated->Recorder = NK_Nil;
	Privated->Player = NK_Nil;

	pthread_mutex_init(&Privated->Mutex, NK_Nil);

	/**
	 * ��¼�¼�����
	 */
	memcpy(&Privated->EventSet, EventSet, sizeof(Privated->EventSet));
	Privated->event_ctx = event_ctx;

	/**
	 * ��ʼ�����г�Ա��
	 */
	Public->Object.name = Object_name;
	Public->Object.dump = Object_dump;
	Public->Object.threadsafe = Object_threadsafe;
	Public->Object.free = Object_free;

	Public->record = TFer_record;
	Public->history = TFer_history;
	Public->play = TFer_play;

	/**
	 * ������̨����̡߳�
	 */
	argc = 0;
	argv[argc++] = (NK_PVoid)(Public);
	Privated->Detector = NK_Thread_Create(Alloctr, NK_False, NK_False,
			detector, argc, argv);

	/**
	 * ���ع��о����
	 * ˽�о��������ⲿ�����߲��ɼ������ֹ�ⲿ�������д��
	 */
	return Public;
}

NK_Int
NK_TFer_Free(NK_TFer **TFer_r)
{
	NK_TFer *Public = NK_Nil;
	NK_PrivatedTFer *Privated = NK_Nil;
	NK_Allocator *Alloctr = NK_Nil;

	/**
	 * �ͷ���Ч������ԡ�
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFer_r, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFer_r[0], -1);

	/**
	 * ��ȡ˽�о����
	 */
	Public = TFer_r[0];
	Privated = PRIVATED(Public);
	TFer_r[0] = NK_Nil; ///< ����ǰ����վ����ֹ�ϲ��û����ͷŹ�����������á�
	Alloctr = Privated->Alloctr;

	/**
	 * ֹͣ¼��
	 */
	Public->record(Public, NK_False, NK_Nil, NK_Nil);

	/**
	 * ֹͣ�طš�
	 */
	Public->play(Public, NK_False, NK_Nil, 0, NK_Nil);

	/**
	 * �ͷż�����̡߳�
	 */
	NK_Thread_Free(&Privated->Detector);

	pthread_mutex_destroy(&Privated->Mutex);
	

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
