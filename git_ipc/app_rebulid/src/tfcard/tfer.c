#if 0
#include <NkEmbedded/tfer.h> //<! 模块 TFer 定义文件。
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
 * TFer 模块私有句柄，句柄访问模块内部的私有成员。\n
 * 内存在 TFer 模块创建时统一分配。\n
 * 位置在句柄数据结构 @ref NK_TFer 上位，\n
 * 这样有效避免 @ref NK_TFer 内存空间被错误释放。\n
 * 如下图：\n
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
	 * 模块内存分配器。
	 */
	NK_Allocator *Alloctr;


	/**
	 * 事件集合。
	 */
	NK_TFerEventSet EventSet;

	/**
	 * 事件上下文。
	 */
	NK_PVoid event_ctx;

	/**
	 * TF 卡卡槽数量。
	 */
	NK_Size tf_slots;

	/**
	 * 当前录像类型。
	 */
	NK_Char record_type[16];

	/**
	 * 当前播放类型。
	 */
	NK_Char play_type[16];

	NK_JSON *PlayFileList;


	/**
	 * 当前播放起始时间戳。
	 */
	NK_UTC1970 play_utc;

	/**
	 * TF 卡标识。
	 */
	struct {

		NK_Boolean mounted;
		NK_Char fs_path[32];

	} TFSlot[8];

	/**
	 * 检测线程句柄。
	 */
	NK_Thread *Detector;

	/**
	 * 录像线程句柄。
	 */
	NK_Thread *Recorder;

	/**
	 * 播放线程句柄。
	 */
	NK_Thread *Player;

	/**
	 * 
	 */
	pthread_mutex_t Mutex;

} NK_PrivatedTFer;

/**
 * 通过模块公有句柄获取私有句柄。
 */
static inline NK_PrivatedTFer *
PRIVATED(NK_TFer *Public)
{
	/**
	 * 偏移到私有句柄。
	 */
	return (NK_PrivatedTFer *)Public - 1;
}


/**
 * THIS 指针定义，\n
 * 定义以下为模块 API 接口实现。
 */
#define THIS NK_TFer * const Public

/**
 * 获取模块名称。
 */
static NK_PChar
Object_name(NK_Object *Obj)
{
	return "TFer";
}

/**
 * 调试打印模块。
 */
static NK_Void
Object_dump(NK_Object *Obj)
{
}

/**
 * 模块接口线程安全标识。
 */
static NK_Boolean
Object_threadsafe(NK_Object *Obj)
{
	return NK_False;
}

/**
 * 模块释放。
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
		 * 当前 TF 卡上个周期已经连接但此时检测不到 TF 卡存在。
		 * 有可能 TF 卡已经被强行拆卸。
		 */
		if (detected && !Privated->TFSlot[0].mounted) {

			/**
			 * 检查到 TF 卡并且当前未挂载。
			 */
			if (0 == Privated->EventSet.onMountTF(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path)) {

				free_size = Privated->EventSet.onGetFreeSpace(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
				Privated->TFSlot[0].mounted = NK_True;
				NK_Log()->debug("TFer: Card Mounted( Path = \"%s\" Free = %uMB ).",
						Privated->TFSlot[0].fs_path, free_size);
			}
		}

		/**
		 * 当前 TF 卡未连接但是此时检测到 TF 卡存在，有可能用户刚插入 TF 卡。
		 */
		if (!detected && Privated->TFSlot[0].mounted) {

			/**
			 * 检查不到 TF 但当前已挂载了。
			 */
			Privated->EventSet.onUmountTF(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
			Privated->TFSlot[0].mounted = NK_False;
			NK_Log()->debug("TFer: Card Umounted( Path = \"%s\" ).", Privated->TFSlot[0].fs_path);
		}

		/// 每隔 1 秒探测一次 TF 卡插入状态。
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
			system(cmd_text);
			printf("cmd_text = %s\n", cmd_text);
		}
	}
	return 0;
}


/**
 * 后台录像线程。
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
	 * 这里不能退出线程，否则会出现描述符泄漏。
	 */
	while (!Thread->terminate(Thread)) {

		/**
		 * 判断是否实现录像事件。
		 */
		if (NK_Nil != Privated->EventSet.onRecord) {

			/**
			 * 判断 TF 卡已经挂载。
			 */
			if (Privated->TFSlot[0].mounted) {

				/**
				 * 回收旧文件释放足够空间腾出给此次录像。
				 */
				free_space = Privated->EventSet.onGetFreeSpace(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
				while (!Thread->terminate(Thread) && free_space < 64) {
					/// 每当 TF 卡剩余容量小于一个临界值时尝试回收一个小时的录像。
					/// 重复此动作一直到录像有足够空间录像为止。
					if(-1 == recorder_recylceOneFile(Privated->TFSlot[0].fs_path)){
						printf("TFer: TFCARD no space\n");
						return;
					}
					free_space = Privated->EventSet.onGetFreeSpace(Privated->event_ctx, 0, Privated->TFSlot[0].fs_path);
					Thread->suspend(Thread, 0, 5, 0);
				}

				/**
				 * 开始录像参数。
				 */
				fs_path = Privated->TFSlot[0].fs_path;

				/**
				 * 创建多媒体录像文件。
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
				 * 关闭多媒体录像文件。
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
 * 后台播放线程。
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
		 * 启动后台检测线程。
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
 * 获取历史信息。
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
 * 播放录像开启/关闭。
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
	 * 默认播放类型。
	 */
	if (!type) {
		type = "default";
	}

	/**
	 * 启动播放。
	 */
	if (start && NK_Nil == Privated->Player) {

		Privated->play_utc = utc;
		snprintf(Privated->play_type, sizeof(Privated->play_type), "%s", type);

		/**
		 * 先释放上一次播放文件列表。
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
		 * 成功搜索文件列表。
		 */
		if (NK_Nil != Privated->PlayFileList) {

			NK_Size const file_list_len = NK_JSON_ArraySize(Privated->PlayFileList);
			NK_Char abs_path[128];
			NK_PChar file_path = NK_Nil;
			NK_JSON *VarFilePath = NK_Nil;

			if (file_list_len > 0) {
				/**
				 * 更新播放列表路径成绝对路径。
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
				 * 启动后台检测线程。
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
	 * 停止播放。
	 */
	if (!start && NK_Nil != Privated->Player) {

		NK_Thread_Free(&Privated->Player);
		Privated->Player = NK_Nil;

		/**
		 * 释放播放文件列表。
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
 * THIS 指针定义解除，\n
 * 定义以上为模块 API 接口实现。
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
	 * 参数合法性检查。
	 */
	NK_EXPECT_VERBOSE_RETURN(slots > 0, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(slots <= 8, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet->onDetectTF, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet->onMountTF, NK_Nil);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != EventSet->onUmountTF, NK_Nil);

	/**
	 * 检查内存分配器。
	 */
	if (!Alloctr) {
		Alloctr = NK_MemAlloc_OS();
	}
	//NK_EXPECT_VERBOSE_RETURN(NK_Nil != Alloctr, NK_Nil);

	/**
	 * 初始化句柄。
	 * 公有句柄的内存空间仅靠私有句柄的高位。
	 * 有效防止模块公有句柄在外部被意外释放。
	 */
	Privated = Alloctr->alloc(Alloctr, sizeof(NK_PrivatedTFer) + sizeof(NK_TFer));
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != Privated, NK_Nil);
	Public = (NK_TFer *)(Privated + 1);

	/**
	 * 初始化私有成员。
	 */
	Privated->Alloctr = Alloctr;
	Privated->Detector = NK_Nil;
	Privated->Recorder = NK_Nil;
	Privated->Player = NK_Nil;

	pthread_mutex_init(&Privated->Mutex, NK_Nil);

	/**
	 * 记录事件集。
	 */
	memcpy(&Privated->EventSet, EventSet, sizeof(Privated->EventSet));
	Privated->event_ctx = event_ctx;

	/**
	 * 初始化公有成员。
	 */
	Public->Object.name = Object_name;
	Public->Object.dump = Object_dump;
	Public->Object.threadsafe = Object_threadsafe;
	Public->Object.free = Object_free;

	Public->record = TFer_record;
	Public->history = TFer_history;
	Public->play = TFer_play;

	/**
	 * 启动后台检测线程。
	 */
	argc = 0;
	argv[argc++] = (NK_PVoid)(Public);
	Privated->Detector = NK_Thread_Create(Alloctr, NK_False, NK_False,
			detector, argc, argv);

	/**
	 * 返回公有句柄。
	 * 私有句柄存放在外部调用者不可见区域防止外部被错误读写。
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
	 * 释放无效句柄断言。
	 */
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFer_r, -1);
	NK_EXPECT_VERBOSE_RETURN(NK_Nil != TFer_r[0], -1);

	/**
	 * 获取私有句柄。
	 */
	Public = TFer_r[0];
	Privated = PRIVATED(Public);
	TFer_r[0] = NK_Nil; ///< 操作前先清空句柄防止上层用户在释放过程中意外调用。
	Alloctr = Privated->Alloctr;

	/**
	 * 停止录像。
	 */
	Public->record(Public, NK_False, NK_Nil, NK_Nil);

	/**
	 * 停止回放。
	 */
	Public->play(Public, NK_False, NK_Nil, 0, NK_Nil);

	/**
	 * 释放检测器线程。
	 */
	NK_Thread_Free(&Privated->Detector);

	pthread_mutex_destroy(&Privated->Mutex);
	

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
