#if 0
#include "app_tfer.h"
#include "media_buf.h"
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <NkUtils/log.h>
#include <sdk_enc.h>
#include <rw_lock.h>
#include <rwlock.h>
#include <dirent.h>
#include <time.h>
#include <base/ja_process.h>

//#include <mediabuf_debug.h>
//#include <tfer.h>
//#include <thread.h>
#include "netsdk.h"

NK_TFer *g_pTFer = NULL;

#define JA_SUCCESS		(0)
#define	JA_FAIL			(-1)
/*
typedef struct _MEDIABUF_USER_ATTR
{
	int pool_ch;
	lpMEDIA_POOL_USER pool_user;
}stMEDIABUF_USER_ATTR, *lpMEDIABUF_USER_ATTR;

#define MEDIABUF_MAX_POOL_REF (64)
typedef struct MEDIA_BUF
{
	uint32_t n_pools;
	lpMEDIA_POOL pools[MEDIABUF_MAX_POOL_REF];
}stMEDIA_BUF, *lpMEDIA_BUF;

static stMEDIA_BUF _media_buf = {
	.n_pools = 0,
};

static lpMEDIA_BUF _p_media_buf = NULL;

const char *MEDIAPOOL_preferred_name(lpMEDIA_POOL const pool)
{
	return pool->preferred_name;
}

const char *MEDIAPOOL_alternate_name(lpMEDIA_POOL const pool)
{
	return pool->alternate_name;
}

static void user_operate_enter_critical(lpMEDIA_POOL const pool)
{
	pool->user_op_lock->wrlock(pool->user_op_lock);
}

static void user_operate_leave_critical(lpMEDIA_POOL const pool)
{
	pool->user_op_lock->unlock(pool->user_op_lock);
}

static stMEDIA_POOL_USER* user_find_vacancy(lpMEDIA_POOL const pool)
{
	int i = 0;
	for(i = 0; i < pool->user_available; ++i){
		lpMEDIA_POOL_USER const user = pool->users[i];
		if(NULL == user->pool){
			user->pool = (void*)pool;
			user->entry_rd = pool->entry_key_history[0];
			user->lock = NULL;
			return user;
		}
	}
	return NULL;
}

static int user_attached_amount(lpMEDIA_POOL const pool)
{
	int i = 0, user_attached = 0;
	for(i = 0; i < pool->user_available; ++i){
		lpMEDIA_POOL_USER const user = pool->users[i];
		if(NULL != user->pool){
			++user_attached;
		}
	}
	return user_attached;
}

lpMEDIA_POOL_USER MEDIAPOOL_user_attach(lpMEDIA_POOL const pool)
{
	user_operate_enter_critical(pool);
	stMEDIA_POOL_USER* const user = user_find_vacancy(pool);
	if(!user){
		MEDIABUF_TRACE("Media Pool(%s) is crowded(%d/%d)",
			pool->preferred_name, user_attached_amount(pool), pool->user_available);
	}else{
		user->timestamp = pool->timestamp;
	}
	if(pool->on_user_changed){
		pool->on_user_changed(pool->preferred_name, user_attached_amount(pool), pool->user_available);
	}
	user_operate_leave_critical(pool);
	return user;
}

int MEDIAPOOL_out_lock(lpMEDIA_POOL_USER user)
{
	lpMEDIA_POOL const pool = (lpMEDIA_POOL)user->pool;
	lpMEDIA_ENTRY const entry_out_p = pool->entries[user->entry_rd];
	LP_RW_LOCK const entry_out_lock = entry_out_p->rwlock;
//	if(0 == MediaPoolUser_sync(user)){
//		return MEDIAPOOL_out_lock(user);
//	}
	if(0 == RWLOCK_TRYRDLOCK(entry_out_lock)){
		// user must know what's the lock
		MEDIABUF_ASSERT(NULL == user->lock, "user->lock is using");
		user->lock = entry_out_lock;
		return 0;
	}
	return -1;
}

int MEDIABUF_out_lock(lpMEDIABUF_USER user)
{
	lpMEDIABUF_USER_ATTR const user_attr = (lpMEDIABUF_USER_ATTR)(user + 1);
	return MEDIAPOOL_out_lock(user_attr->pool_user);
}

int MEDIAPOOL_user_sync(lpMEDIA_POOL_USER const user)
{
	lpMEDIA_POOL const pool = user->pool;
	if(pool->timestamp > user->timestamp){
		user->timestamp = pool->timestamp;
		user->entry_rd = pool->entry_key_history[0];
		return 0;
	}
	return -1;
}

int MEDIAPOOL_out(lpMEDIA_POOL_USER user, void** ret_data_ptr, void* ret_data, size_t* ret_sz)
{
	int const entry_rd = user->entry_rd;
	lpMEDIA_POOL const pool = (lpMEDIA_POOL)user->pool;
	lpMEDIA_ENTRY const entry_out_p = pool->entries[entry_rd];

	if(user->entry_rd == pool->entry_in){
		//MEDIABUF_TRACE("read speed is higher %d/%d", user->entry_rd, pool->entry_in);
		return -1;
	}

	// update the next read pos
	user->entry_rd = entry_out_p->next;
	if(0 == entry_out_p->data_size){
		// data is not existed
		// an abnormal situation
		// the read speed is very slow to write speed of pool
//		MEDIABUF_TRACE("data out @ %d ptr=%p size=%d", entry_rd, entry_out_p->data, entry_out_p->
//data_size);
//		MEDIAPOOL_rw_status(pool);
		MEDIAPOOL_user_sync(user);
//		MEDIABUF_TRACE("user read entry = %d", user->entry_rd);
		return -1;
	}

	if(ret_data_ptr){ *ret_data_ptr = entry_out_p->data; }
	if(ret_data){ memcpy(ret_data, entry_out_p->data, entry_out_p->data_size); }
	if(ret_sz){ *ret_sz = entry_out_p->data_size; }

	if(pool->on_entry_out){
		pool->on_entry_out(pool->preferred_name, entry_out_p->data, entry_out_p->data_size, entry_out_p
->key_flag);
	}
	return 0;
}

int MEDIABUF_init()
{
	if(!_p_media_buf){
		int i = 0;
		// alloc the handle
		_p_media_buf = &_media_buf;
		// alloc all the pools
		_p_media_buf->n_pools = 0;
		// clear the handler
		for(i = 0; i < MEDIABUF_MAX_POOL_REF; ++i){
			_p_media_buf->pools[i] = NULL;
		}
		// success
		printf("media buf init success");
		return 0;
	}
	return -1;
}

static int MEDIABUF_lookup_byname(const char* name)
{
	int i = 0;
	// find following the preferred name
	for(i = 0; i < MEDIABUF_MAX_POOL_REF; ++i){
		if(_p_media_buf->pools[i]){
			const char* pool_name = MEDIAPOOL_preferred_name(_p_media_buf->pools[i]);
			if(0 == strcmp(pool_name, name) && strlen(pool_name) == strlen(name)){
				// bingo!!
				return i;
			}
		}
	}
	// find following the alternate name
	for(i = 0; i < MEDIABUF_MAX_POOL_REF; ++i){
		if(_p_media_buf->pools[i]){
			const char* pool_name = MEDIAPOOL_alternate_name(_p_media_buf->pools[i]);
			if(0 == strcmp(pool_name, name) && strlen(pool_name) == strlen(name)){
				// bingo!!
				return i;
			}
		}
	}
	return -1;
}

static lpMEDIABUF_USER MEDIABUF_attach(int id)
{
	if(id < _p_media_buf->n_pools){
		lpMEDIA_POOL_USER const pool_user = MEDIAPOOL_user_attach(_p_media_buf->pools[id]);
		if(pool_user){
			lpMEDIABUF_USER const user = calloc(sizeof(stMEDIABUF_USER) + sizeof(stMEDIABUF_USER_ATTR), 1);
			lpMEDIABUF_USER_ATTR const user_attr = (lpMEDIABUF_USER_ATTR)(user + 1);
			user->forbidden_zero = 0;
			user_attr->pool_ch = id;
			user_attr->pool_user = pool_user;
			return user;
		}
	}
	return NULL;	
}



static int MEDIABUF_out(lpMEDIABUF_USER user, void** ret_data_ptr, void* ret_data, size_t* ret_sz)
{
	lpMEDIABUF_USER_ATTR const user_attr = (lpMEDIABUF_USER_ATTR)(user + 1);
	return MEDIAPOOL_out(user_attr->pool_user, ret_data_ptr, ret_data, ret_sz);
}

void MEDIAPOOL_out_unlock(lpMEDIA_POOL_USER user)
{
	int ret = 0;
	LP_RW_LOCK const entry_out_lock = (LP_RW_LOCK)user->lock;
	MEDIABUF_ASSERT(entry_out_lock, "media pool out unlock failed");
	ret = entry_out_lock->unlock(entry_out_lock);
	MEDIABUF_ASSERT(0 == ret, "media out unlock failed!");
	user->lock = NULL; // clear the lock
}

void MEDIABUF_out_unlock(lpMEDIABUF_USER user)
{
	lpMEDIABUF_USER_ATTR const user_attr = (lpMEDIABUF_USER_ATTR)(user + 1);
	MEDIAPOOL_out_unlock(user_attr->pool_user);
}
*/

static inline NK_Int
SCRIPT(NK_PChar fmt, ...)
{
	NK_Char script[1024 * 4];
	va_list var;

	va_start(var, fmt);
	vsnprintf(script, sizeof(script), fmt, var);
	va_end(var);

	//return system(script);
	NK_SYSTEM(script);
	return 0;
}

static inline NK_SSize
SCRIPT2(NK_PChar result, NK_Size result_max, NK_PChar fmt, ...)
{
	NK_Char script[1024 * 4];
	va_list var;
	FILE *fID = NK_Nil;
	NK_SSize readn = -1;

	va_start(var, fmt);
	vsnprintf(script, sizeof(script), fmt, var);
	va_end(var);

	fID = popen(script, "r");
	if (!fID) {
		NK_Log()->error("Test: /bin/sh \"%s\" Error.", script);
		return -1;
	}

	readn = fread(result, 1, result_max, fID);
	pclose(fID);
	fID = NK_Nil;

	return readn;
}

static inline NK_Boolean
IS_FILE_EXIST(const char *filePath)
{
	return (-1 != access(filePath, F_OK));
}


static NK_Boolean
tf_onDetectTF(NK_PVoid ctx, NK_Int id)
{
	//return NK_True;
	return IS_FILE_EXIST("/dev/mmcblk0p1");
}

static int is_mount()
{
	 FILE *tfd = fopen("/media/tf/record/ttt", "wb");
	 if(NULL == tfd)
	 {
		 return -1;
	 }
	 else
	 {
		  fclose(tfd);
		  tfd = NULL;
	 }
	 return 0;
}

static NK_Int
tf_onMountTF(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
	sprintf(fs_path, "/media/tf");
	if(!IS_FILE_EXIST(fs_path)){
		SCRIPT("mkdir -p %s", fs_path);
	}
	SCRIPT("fdisk -l /dev/mmcblk0p1");
	if(0 == mount("/dev/mmcblk0p1", fs_path, "vfat", 0, NULL)){
		/// 挂在成功。
		NK_Log()->info("Recorder: Mount Record Disk.");

		// success to mount
		SCRIPT("ls -l %s/*.REC", fs_path);
		SCRIPT("rm -f %s/*.REC", fs_path);
		if(!IS_FILE_EXIST("/media/tf/record")){
			SCRIPT("mkdir -p %s", "/media/tf/record");
		}
		SCRIPT("mount");
		if(0 != is_mount()){
			NK_SYSTEM("umount -l /media/tf");
			sleep(2);
			tf_onMountTF(NULL, 0, fs_path);
		}

		return 0;

	}

	if (EBUSY == errno) {
		if(0 != is_mount()){
			NK_SYSTEM("umount -l /media/tf");
			sleep(2);
			tf_onMountTF(NULL, 0, fs_path);
		}

		/// 上个运行周期已经挂载。
		NK_Log()->info("Recorder: TF Mounted.");
		return 0;

	} else {

	}

	//perror("mount");
	NK_Log()->error("Recorder: Mount Record Disk Failed (%d, %s).", errno, strerror(errno));

	return -1;
}

static NK_Int
tf_onUmountTF(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
//	if (0 == umount2(fs_path, MNT_DETACH)) { // like 'umount -l'
//	if (0 == umount2("/dev/mmcblk0p1", MNT_DETACH)) { // like 'umount -l'
//	if(0 == system("umount -l /dev/mmcblk0p1")){
		if(0 == system("umount -l /media/tf")){
		
		NK_Log()->info("Recorder: Unmount Record Disk.");
		SCRIPT("mount");
		//clear dir after umount success,aviod legacy
		NK_SYSTEM("rm -rf /media/tf");
 		
		return 0;
	}

	return -1;
}


static NK_Size
tf_onGetCapacity(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
	struct statvfs StatFS;
	NK_Int64 capacity = 0;

	//return 128;

	/**
	 * 使用系统方法获取文件系统空间大小。
	 */
	if (statvfs(fs_path, &StatFS) < 0) {
		return 0;
	}

	capacity = StatFS.f_blocks;
	capacity *= StatFS.f_bsize;
	/**
	 * 转换成 MB 单位。
	 */
	capacity /= 1024;
	capacity /= 1024;

	NK_Log()->error("Case: TF Capacity %dMB.", (NK_SSize)capacity);
	return (NK_Size)capacity;
}


static NK_Size
tf_onGetFreeSpace(NK_PVoid ctx, NK_Int id, NK_PChar fs_path)
{
	struct statvfs StatFS;
	NK_Int64 free_size = 0;

	//return 64;

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

	NK_Log()->debug("Case: TF Free Space %dMB.", (NK_SSize)free_size);
	return (NK_Size)free_size;
}
static NK_Int
_record_counter = 30;


static NK_Int tf_onRecord(NK_PVoid ctx, NK_Thread *Thread, NK_TFerRecorder *Writer, NK_PVoid recorder_ctx)
{
	int mediaBufID = 0;
	lpMEDIABUF_USER mediaBufUser = NULL;
	NK_TFerRecordAttr RecordAttr;
	int count = 0;
			
	mediaBufID = MEDIABUF_lookup_byname("ch0_0.264"); // FIXME:
	if (-1 == mediaBufID)
	{
		printf("Mediabuf Lookup mediaBufID Error!!!\n");
		return -1;
	}

	mediaBufUser = MEDIABUF_attach(mediaBufID);
	if (NULL == mediaBufUser)
	{
		printf("MediaBufUser Attach Error!!!\n");
		return -1;
	}
	while (!Thread->terminate(Thread)) {

		NK_Boolean is_keyframe = NK_False;
		NK_Boolean has_read = NK_False;
		static NK_PByte h264_nalu = NK_Nil;
		NK_SSize h264_nalu_size = 0;
		NK_UInt64 ts_ms = 0;

		if (0 == MEDIABUF_out_lock(mediaBufUser)) {

			static lpSDK_ENC_BUF_ATTR attr = NULL;
			size_t outDataLen = 0;
			if (0 == MEDIABUF_out(mediaBufUser, (void *)&attr, NULL, &outDataLen)) {

				if(attr->type == kSDK_ENC_BUF_DATA_H264)// || attr->type == kSDK_ENC_BUF_DATA_H265)
				{
					h264_nalu_size = attr->data_sz;
					h264_nalu = (void *)(attr + 1);

					is_keyframe = attr->h264.keyframe;

					ts_ms = attr->timestamp_us / 1000;
					has_read = NK_True;

					RecordAttr.codec = NK_TFER_VCODEC_H264;
					RecordAttr.width = attr->h264.width;
					RecordAttr.height = attr->h264.height;
					RecordAttr.frame_rate = attr->h264.fps;
 					
					Writer->setVideoAttr(Writer, &RecordAttr);
					if(outDataLen > h264_nalu_size && h264_nalu_size > 0){
						int ret = -1;
						ret= Writer->writeH264(Writer, ts_ms, is_keyframe, h264_nalu, h264_nalu_size);
						if(ret == -1)
						{
							count ++;
							printf("write error\n");
						}
						if(count >= 2)
						{
							printf("write errorwrite errorwrite errorwrite error\n");
							MEDIABUF_out_unlock(mediaBufUser);
							_record_counter= 0;
							break;
						}
					}
					//printf("ts_ms = %lld, attr->h264.width = %d, attr->h264.height = %d, attr->h264.fps = %d, is_keyframe=%d\n", ts_ms, attr->h264.width, attr->h264.height, attr->h264.fps, is_keyframe);
				}
				else if(attr->type == kSDK_ENC_BUF_DATA_G711A)
				{
					NK_PByte g711au_data = (void *)(attr + 1);
					NK_SSize g711au_size = attr->data_sz;
					NK_UInt64 timet = attr->timestamp_us / 1000; 

					RecordAttr.codec = NK_TFER_ACODEC_G711A;
					RecordAttr.stereo = NK_FALSE;
					RecordAttr.sample_bitwidth = attr->g711a.sample_width;
					RecordAttr.sample_rate = attr->g711a.sample_rate; 
					has_read = NK_True;
					Writer->setAudioAttr(Writer, &RecordAttr);
					if (g711au_size > 0) {
						int ret = Writer->writeG711(Writer, timet, g711au_data, g711au_size);
						if(ret == -1)
						{
							count ++;
							printf("write error\n");
						}
						if(count >= 2)
						{
							printf("write errorwrite errorwrite errorwrite error\n");
							MEDIABUF_out_unlock(mediaBufUser);
							_record_counter= 0;
							break;
						}
					}
				}
				/*
				else if(attr->type == kSDK_ENC_BUF_DATA_G711U)
				{
					NK_PByte g711au_data = (void *)(attr + 1);
					NK_SSize g711au_size = attr->data_sz;
					NK_UInt64 timet = attr->timestamp_us / 1000;

					RecordAttr.codec = NK_TFER_ACODEC_G711U;
					RecordAttr.stereo = NK_FALSE;
					RecordAttr.sample_bitwidth = attr->g711a.sample_width;
					RecordAttr.sample_rate = attr->g711a.sample_rate; 
					has_read = NK_True;
					Writer->setAudioAttr(Writer, &RecordAttr);
					if (g711au_size > 0) {
						Writer->writeG711(Writer, timet, g711au_data, g711au_size);
					}
					printf("G711UG711UG711UG711UG711UG711UG711UG711UG711U\n");
				}*/

			}
			
			MEDIABUF_out_unlock(mediaBufUser);
		}
		
		if (!has_read) {
			usleep(10000);
		}
	}

	/**
	 * 释放用户。
	 */
	MEDIABUF_detach(mediaBufUser);
	mediaBufUser = NK_Nil;

	return 0;
}

typedef struct Recframe{
	stSDK_ENC_BUF_ATTR *rec_attr;
	void *raw_buff;	
    size_t frame_maxlen;
	int flag;
	int recType;
	int utc_t;
}Recframe_t;

static NK_Int 
tf_onPlay(NK_PVoid ctx, NK_Thread *Thread, NK_TFerPlayer *Player, NK_PVoid player_ctx)
{
	if (NULL == player_ctx) {
		printf("The parameter of player_ctx can not be NULL !!!");
		return -1;
	}

	NK_TFerRecordAttr RecordAttr;
	NK_SSize len = 0;

	ST_NSDK_SYSTEM_TIME systime;
	NETSDK_conf_system_get_time(&systime);
	int timezone_second = ((abs(systime.greenwichMeanTime) / 100) * (3600)
							+(abs(systime.greenwichMeanTime) % 100) * 60)
							* (systime.greenwichMeanTime > 0 ? 1 : -1);
	Recframe_t* pRecframe = (Recframe_t*)player_ctx;

	while (!Thread->terminate(Thread)) {
		NK_UInt64 ts_ms = 0;
		
		if(pRecframe->flag == 0)
		{
			len = Player->read(Player, &RecordAttr, &ts_ms, (NK_PByte)pRecframe->raw_buff, pRecframe->frame_maxlen);
			if(0 == len || len < 0){
				printf("__FUNCTION__:%s, LINE:%d, player_ctx=%p, len:%d,pRecframe->frame_maxlen:%d\n", __FUNCTION__, __LINE__, player_ctx, len, pRecframe->frame_maxlen);
				pRecframe->rec_attr = NULL;
				pRecframe->raw_buff = NULL;
				pRecframe->flag = -1;
				//return -1;
			}
			else
			{
				pRecframe->rec_attr->data_sz = len;
				pRecframe->rec_attr->timestamp_us = ts_ms +timezone_second*1000;//ts_ms*1000;

				if (RecordAttr.codec == NK_TFER_VCODEC_H264) {

					pRecframe->rec_attr->type = kSDK_ENC_BUF_DATA_H264;
					if(((*((NK_PByte)(pRecframe->raw_buff) + 4)) & 0x1f) == 1)
					{
						pRecframe->rec_attr->h264.keyframe = 0;
					}
					else
					{
						pRecframe->rec_attr->h264.keyframe = 1;
					}
					pRecframe->rec_attr->h264.fps = RecordAttr.frame_rate;
					pRecframe->rec_attr->h264.width = RecordAttr.width;
					pRecframe->rec_attr->h264.height = RecordAttr.height;
					pRecframe->flag = 1;
	//				printf("ts_ms=%lld, pRecframe->rec_attr->timestamp_us = %lld, pRecframe->rec_attr->h264.keyframe = %d\n", ts_ms, pRecframe->rec_attr->timestamp_us, pRecframe->rec_attr->h264.keyframe);
				}
				else if(RecordAttr.codec == NK_TFER_ACODEC_G711A)
				{
					pRecframe->rec_attr->type = kSDK_ENC_BUF_DATA_G711A;//kSDK_ENC_BUF_DATA_PCM;
					pRecframe->rec_attr->g711a.sample_rate = RecordAttr.sample_rate;
					pRecframe->rec_attr->g711a.sample_width = RecordAttr.sample_bitwidth;
					pRecframe->flag = 1;
				}
				else if(RecordAttr.codec == NK_TFER_ACODEC_G711U)
				{
					pRecframe->rec_attr->type = kSDK_ENC_BUF_DATA_G711U;//kSDK_ENC_BUF_DATA_PCM;
					pRecframe->rec_attr->g711a.sample_rate = RecordAttr.sample_rate;
					pRecframe->rec_attr->g711a.sample_width = RecordAttr.sample_bitwidth;
					pRecframe->flag = 1;
				}
				else
				{
					pRecframe->rec_attr->type = kSDK_ENC_BUF_DATA_PCM;//kSDK_ENC_BUF_DATA_PCM;
					pRecframe->rec_attr->g711a.sample_rate = RecordAttr.sample_rate;
					pRecframe->rec_attr->g711a.sample_width = RecordAttr.sample_bitwidth;
					pRecframe->flag = 1;
				}
			}
		}
		else
		{
			usleep(20000);
		}
	}

	return 0;
}

static NK_Thread *
_RecordDelay = NK_Nil;

static NK_Void record_delay(NK_Thread *const Thread, NK_Int argc,
		NK_PVoid argv[])
{
	char *record_type = argv[0];
	while (NK_Nil != g_pTFer && _record_counter> 0) {
		Thread->suspend(Thread, 1, 0, 0);
		--_record_counter;
	}

	g_pTFer->record(g_pTFer, NK_False, record_type, NK_Nil);
	_RecordDelay = NK_Nil;
}

NK_Void
TFER_start_record(const char *record_type)
{
	NK_Int argc = 0;
	NK_PVoid argv[32];

	if (!g_pTFer){
		return;
	}
	//靠靠靠靠2010򡂡靠靠�
	if(time(NULL) < 1262278861)
	{
		return;
	}

	if(!tf_onDetectTF(NULL, 0))
	{
		_record_counter= 0;
		return;
	}
	else
	{
		if(!IS_FILE_EXIST("/media/tf/record/"))
		{
			char file_path[20];
			_record_counter= 0;
			tf_onMountTF(NULL, 0, file_path);
			return;
		}
		else
		{
		   if(0 != is_mount())
		   {
			   char file_path[20];
			   _record_counter= 0;
			   tf_onMountTF(NULL, 0, file_path);
			   return;
		   }
		}
	}

	_record_counter= 30;

	if (!_RecordDelay) {
		argc = 0;
		argv[argc++] = (NK_PVoid)(record_type);
		//_record_counter= 30;
		g_pTFer->record(g_pTFer, NK_True, record_type, NK_Nil);
		_RecordDelay = NK_Thread_Create(NK_MemAlloc_OS(), NK_True, NK_True,
			record_delay, argc, argv);
	}
}

static bool
is_record_on_schedule()
{
	ST_NSDK_SYSTEM_SETTING sinfo={0};
	NETSDK_conf_system_get_setting_info(&sinfo);

	unsigned int cur_weekday=0;
	unsigned int cur_sec=0;

	time_t cur_time;
	struct tm *p;
	time(&cur_time);
	p=localtime(&cur_time);//get localtime

	cur_weekday = 1 << p->tm_wday;
	cur_sec = p->tm_hour * 3600 + p->tm_min * 60 + p->tm_sec;

	int i;
	for (i = 0
			; i < sizeof(sinfo.TFcard_Record.Schedule)
				/ sizeof(sinfo.TFcard_Record.Schedule[0]);
			++i)
	{
		int beginSec, endSec;
		beginSec = sinfo.TFcard_Record.Schedule[i].BeginTime.hour * 3600
				+ sinfo.TFcard_Record.Schedule[i].BeginTime.min * 60
				+ sinfo.TFcard_Record.Schedule[i].BeginTime.sec;
		endSec = sinfo.TFcard_Record.Schedule[i].EndTime.hour * 3600
				+ sinfo.TFcard_Record.Schedule[i].EndTime.min * 60
				+ sinfo.TFcard_Record.Schedule[i].EndTime.sec;

		if((sinfo.TFcard_Record.Schedule[i].weekday) & cur_weekday)
		{
			if(cur_sec >= beginSec && cur_sec <= endSec){
				return true;
			}
		}
	}

	return false;
}


static NK_Thread *
_ScheduleRecord = NK_Nil;

static NK_Void
TFER_on_schedule_record(NK_Thread *Thread, NK_Integer argc, NK_PVoid argv[])
{
	while (!Thread->terminate(Thread))
	{
		if(is_record_on_schedule()){
			TFER_start_record("schedule");
		}

		Thread->suspend(Thread, 1, 0, 0);
	}
}

static int tfer_Create(NK_TFer **ppTFer)
{
	//NK_Boolean record = NK_False;
	if(NULL == ppTFer)
	{
		printf("tfer create error !!! : the pointer of tfer is NULL.\n");
		return JA_FAIL;
	}

	NK_TFerEventSet TFerEventSet;
	TFerEventSet.onDetectTF = tf_onDetectTF;
	TFerEventSet.onMountTF = tf_onMountTF;
	TFerEventSet.onUmountTF = tf_onUmountTF;
	TFerEventSet.onGetCapacity = tf_onGetCapacity;
	TFerEventSet.onGetFreeSpace = tf_onGetFreeSpace;
	TFerEventSet.onRecord = tf_onRecord;
	TFerEventSet.onPlay = tf_onPlay;

	NK_Log()->setTerminalLevel(NK_LOG_LV_ALERT);
	*ppTFer = NK_TFer_Create2(NK_Nil, 1, &TFerEventSet, NK_Nil);

	/*startup schedule record pthread background*/
	_ScheduleRecord = NK_Thread_Create(NK_MemAlloc_OS(), NK_False, NK_False,
			TFER_on_schedule_record, 0, NULL);

	return JA_SUCCESS;
}

void TFER_destroy(void)
{
	NK_TFer_Free(&g_pTFer);
}

NK_TFer *TFER_init(void)
{
	int reVal;

	reVal = tfer_Create(&g_pTFer);
	if(JA_SUCCESS == reVal)
	{
		return g_pTFer;
	}

	return NULL;
}

#endif
