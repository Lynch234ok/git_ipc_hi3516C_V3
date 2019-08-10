
#include "media_buf.h"
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/prctl.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <NkUtils/log.h>
#include <NkUtils/assert.h>
#include <sdk_enc.h>
#include <rw_lock.h>
#include <rwlock.h>
#include <dirent.h>
#include <time.h>
#include <base/ja_process.h>
#include <app_debug.h>
#include "generic.h"
#include "netsdk.h"
#include <mp4mux/hi_type.h>
#include "log.h"
#include <stdbool.h>
#include <sdk/sdk_enc.h>
#include <mp4mux/hi_mp4_muxer.h>
#include <app_tfcard.h>
#include "bsp/keytime.h"
#include "../sound.h"
#include "../netsdk_def.h"
#include "../netsdk.h"
#include "../bsp/jagpio/bsp_jagpio.h"
#include "ticker.h"
#include <pthread.h>
#include "global_runtime.h"
#include "sensor.h"
#include "app_tfcard.h"
#include "app_gsensor.h"
#include "base/cross_thread.h"

#define TFCARD_FS_PATH "/media/tf"
#define TFCARD_FDISK_PATH "/dev/mmcblk0"
#define TFCARD_DEV_PATH "/dev/mmcblk0p1"

typedef struct _record{
	pthread_t record_pid;
	pthread_t delay_pid;
	NK_Int delay_sec;
	NK_Boolean trigger;
}stAPP_RECORD,*lpAPP_RECORD;

static lpAPP_RECORD _app_record = NULL;
static bool gs_inited = false;


#define  RECORD_MOTION_TIME_S  10


static pthread_t tfcard_check_pid = (pthread_t)NULL;
static void* TFCARD_record_check(void *arg);
static int tfcard_start_record(char *record_type);


// get current monotonic ms. p_cur_ms can't be NULL
static int tfcard_get_cur_mono_ms(int64_t *p_cur_ms)
{
    int ret;
    struct timespec tp;

    ret = clock_gettime(CLOCK_MONOTONIC, &tp);
    if (ret != 0) {
        APP_TRACE("%s: clock_gettime failed! ret: %d, errno: %d",
                  __FUNCTION__,
                  ret,
                  errno);
        return -1;
    } else {
        *p_cur_ms = ((int64_t)tp.tv_sec) * 1000 + ((int64_t)tp.tv_nsec) / 1000000;
        return 0;
    }
}

/**
 *	¸ù¾ÝÃû×Ö²éÕÒ½ø³ÌPID, ¿ÉÓÃÓÚÅÐ¶Ï½ø³ÌÊÇ·ñ´æÔÚ.
 *  	×¢Òâ : µ±ÏµÍ³ÖÐ´æÔÚÒ»Ä£Ò»ÑùÇÒ³¤¶ÈÏàÍ¬µÄ½ø³ÌÃûÊ±,²»ÊÊÓÃ´Ëº¯Êý.(fixme)
 *
 *	param : pName		Ðè²éÑ¯µÄ½ø³ÌÃû
 * 	return : ³É¹¦Ê±·µ»Ø½ø³Ì¶ÔÓ¦Pid(´óÓÚ0), Ê§°ÜÊ±·µ»Ø -1
 */
static NK_Int get_pid_by_name(NK_PChar pName)
{
	DIR *dir;
	struct dirent *d;
	NK_Int pid = 0;
	char *s = NULL;
	NK_Size pnLen = 0;

	if(!pName){
		NK_Log()->error("pName is NULL!");
		return NK_False;
	}
	pnLen = strlen(pName);

	/*open the /proc directory*/
	if(NULL == (dir = opendir("/proc"))){
		NK_Log()->error("open /proc error");
		return NK_False;
	}

	/* Walk through the directory. */
	while ((d = readdir(dir)) != NULL) {

		char exe [PATH_MAX+1];
        char path[PATH_MAX+1];
        int len;
        int namelen;

        /* See if this is a process */
        if ((pid = atoi(d->d_name)) == 0){
			continue;
        }

		snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);
        if ((len = readlink(exe, path, PATH_MAX)) < 0){
			continue;
		}
        path[len] = '\0';

        /* Find ProcName */
        s = strrchr(path, '/');
        if(s == NULL) continue;
        s++;

        /* we don't need small name len */
        namelen = strlen(s);
        if(namelen < pnLen){
			continue;
        }
        if(!strncmp(pName, s, pnLen)) {
        /* to avoid subname like search proc tao but proc taolinke matched */
			if(s[pnLen] == ' ' || s[pnLen] == '\0') {
				closedir(dir);
				return pid;
			}
		}
	}

	closedir(dir);
	return -1;
}

#if 0
void *tfcard_repair_mobile_mp4_thr(void *arg)
{
    static bool is_me_running = false;

    int ret;
    DIR *p_dir = NULL;
    struct dirent *entry;
    size_t entry_name_len;
    char *tmp_ext = ".mp4.tmp";
    char mp4_tmp_name[256];
    char mp4_name[256];

    const size_t tmp_ext_len = strlen(tmp_ext);
    const size_t mp4_tmp_name_sz = sizeof(mp4_tmp_name);
    const size_t mp4_name_sz = sizeof(mp4_name);

    prctl(PR_SET_NAME,"tfcard_repair_mobile_mp4_thr");

    // Only doing one repairing task at one time.
    if (is_me_running) {
        return NULL;
    }
    is_me_running = true;


    p_dir = opendir(CLIP_PATH);
    if(NULL == p_dir) {

        APP_TRACE("Failed to open mobile record dir. Path: %s", CLIP_PATH);
        goto THR_END;

    } else {

        while(1) {

            entry = readdir(p_dir);
            if (NULL == entry) {
                break;
            }

            entry_name_len = strlen(entry->d_name);

            // Search for and repair *.mp4.tmp files.
            if((0 == strcmp(entry->d_name,"."))
               || (0 == strcmp(entry->d_name,".."))) {

                // . and ..
                continue;

            } else if (entry_name_len <= tmp_ext_len) {

                // fils who's name too short
                continue;

            } else {

                // .mp4.tmp files
                if (0 == strcmp(entry->d_name + (entry_name_len - tmp_ext_len), tmp_ext)) {

                    ret = snprintf(mp4_tmp_name, mp4_tmp_name_sz, "%s/%s", CLIP_PATH, entry->d_name);
                    if (ret < 0) {

                        APP_TRACE("%s: snprintf Failed.", __FUNCTION__);
                        continue;

                    } else if (ret >= mp4_tmp_name_sz) {

                        APP_TRACE("%s: Mp4 temp file path-name is too long. Max allowed: %lu. path-name: %s/%s",
                                  __FUNCTION__,
                                  mp4_name_sz-1,
                                  CLIP_PATH,
                                  entry->d_name);
                        continue;

                    } else {
                        ;
                    }

                    APP_TRACE("Repairing mp4 temp file: %s", mp4_tmp_name);

                    ret = MP4_CheckFileIntegrity(mp4_tmp_name);
                    APP_TRACE("MP4_CheckFileIntegrity, ret: 0x%X", ret);

                    ret = MP4_ReparieFile(mp4_tmp_name);
                    if (ERR_MP4_OK != ret) {

                        APP_TRACE("Failed to repair mp4 temp file, ret: 0x%X", ret);

                    } else {

                        // ÐÞ¸´³É¹¦£¬¸ÄÃûµ½ .mp4
                        ret = snprintf(mp4_name, mp4_name_sz, "%s", mp4_tmp_name);
                        if (ret < 0) {

                            APP_TRACE("%s: snprintf Failed.", __FUNCTION__);

                        } else if (ret >= mp4_name_sz) {

                            APP_TRACE("%s: Mp4 temp file path-name is too long. Max allowed: %lu. path-name: %s",
                                      __FUNCTION__,
                                      mp4_name_sz-1,
                                      mp4_tmp_name);

                        } else {

                            // .mp4.tmp --> .mp4'\0'tmp
                            mp4_name[entry_name_len - 4] = '\0';
                            if (0 != rename(mp4_tmp_name, mp4_name)) {
                                const int errno_tmp = errno;
                                APP_TRACE("%s: rename temp mp4 file failed, %s -> %s. errno: %d",
                                          __FUNCTION__,
                                          mp4_tmp_name, mp4_name,
                                          errno_tmp);
                            }
                        }
                    }

                } else {

                    // other files
                    continue;
                }
            }
        }

        closedir(p_dir);
    }


THR_END:

    is_me_running = false;
}

void tfcard_repair_mobile_mp4(void)
{
    int ret;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, tfcard_repair_mobile_mp4_thr, NULL);
    if (0 != ret) {
        APP_TRACE("%s: Failed to create tfcard_repair_mobile_mp4_thr thread.", __FUNCTION__);
    }

    return;
}
#endif

static NK_Boolean onExistTF(NK_Void)
{
	return IS_FILE_EXIST(TFCARD_FDISK_PATH);
}

static NK_Boolean onDetectTF(NK_Void)
{
	return IS_FILE_EXIST(TFCARD_DEV_PATH);
}

//ÅÐ¶ÏTF¿¨ÊÇ·ñÕæµÄ¿É¶ÁÐ´£¬·ÀÖ¹»µ¿¨£¬»òÕß¿ÉÄÜÐ¡¸ÅÂÊ³öÏÖÎÄ¼þÏµÍ³³ÉÎªÖ»¶Á
static NK_Boolean TF_Can_Read_Write(NK_UInt8 cnt){
	FILE *fp = NULL;
	char file_name[40],cmd_line[50],test_write[6] = "ABCDE",test_read[6];

	if(IS_FILE_EXIST(TFCARD_FS_PATH)){
		memset(file_name, 0, sizeof(file_name));
		memset(cmd_line, 0, sizeof(cmd_line));
		
		strcpy(file_name,TFCARD_FS_PATH);
		strcat(file_name,"/tf_rw.txt");
		snprintf(cmd_line, sizeof(cmd_line),"rm -f %s",file_name);
		
		do{
			if(IS_FILE_EXIST(file_name)){
				NK_SYSTEM(cmd_line);
			}else{
				if((fp = fopen(file_name, "w+")) == NULL){
					APP_TRACE("TF_Can_Read_Write open error!");
					char cmd_remount[50];
					memset(cmd_remount, 0, sizeof(cmd_remount));
					snprintf(cmd_remount, sizeof(cmd_remount),"mount -o remount,rw %s",TFCARD_FS_PATH);
					NK_SYSTEM(cmd_remount);
				}else{
					fwrite(test_write,sizeof(test_write),1,fp);
					sync();
					fclose(fp);
					fp = NULL;
					usleep(500000);
					if((fp = fopen(file_name, "r")) != NULL){
						memset(test_read, 0, sizeof(test_read));
						fread(test_read,sizeof(test_read),1,fp);
						fclose(fp);
						fp = NULL;
						if(strcmp(test_read, test_write) == 0){
							APP_TRACE("TF_Can_Read_Write OK!");
							NK_SYSTEM(cmd_line);
							break;
						}
					}else{
						APP_TRACE("TF_Can_Read_Write Error!");
						NK_SYSTEM(cmd_line);
					}
					
				}
			}
		}while(--cnt > 0);
		if(cnt){
			return true;
		}else{
			memset(cmd_line, 0, sizeof(cmd_line));
			snprintf(cmd_line, sizeof(cmd_line),"rm -f /dev/mmcblk*");
			NK_SYSTEM(cmd_line);
			return false;
		}
	}else{
		return false;
	}
}

static NK_Int onMountTF(NK_PChar mountPath)
{
	NK_Char cmdLine[64] = {0};
	NK_Int ret=0;

	if(!IS_FILE_EXIST(mountPath)){
		NK_Log()->info("mkdir dir before mount,path = %s",mountPath);
		snprintf(cmdLine, sizeof(cmdLine), "mkdir -p %s", mountPath);
		NK_SYSTEM(cmdLine);
	}

	if(0 != (ret = mount(TFCARD_DEV_PATH, mountPath, "vfat", 0, NULL)))
	{
		NK_Log()->error("mount TFcard error, ret : %d, error : %d, path : %s",ret,errno,mountPath);
		//if(EBUSY == errno){
			memset(cmdLine, 0, sizeof(cmdLine));
			snprintf(cmdLine, sizeof(cmdLine), "umount -l %s", mountPath);
			NK_SYSTEM(cmdLine);
			return -1;
		//}
		//APP_TRACE("mount TFcard error, ret : %d, error : %d, path : %s",ret,errno,mountPath);
		//return -1;
	}else{
		if(TF_Can_Read_Write(5)){
			NK_Log()->info("mount TFcard success, path : %s",mountPath);
			NK_SYSTEM("mount");
		}else{
			return -1;
		}
	}
	

	return 0;
}

static bool motion_recording_stop_flag = false;

static NK_Int onUmountTF(NK_PChar mountPath)
{
	//NK_Char cmdLine[32] = {0};

    system("sync");
    usleep(500000);

    /* Terminates motion record unconditionally. */
    motion_recording_stop_flag = true;

	#if 0
	snprintf(cmdLine, sizeof(cmdLine), "umount -l %s", mountPath);
	if(0 != system(cmdLine))
	#else
	if(0 != umount2(mountPath, MNT_DETACH ))
	#endif
	{
		NK_Log()->error("umount TFcard error,path : %s",mountPath);
		return -1;
	}
    usleep(500000);
	NK_Log()->info("umount TFcard success, path : %s",mountPath);
	NK_SYSTEM("mount");

	return 0;
}

static NK_Int onCleanTF(NK_PChar mountPath)
{
	NK_Char cmdLine[32] = {0};

	snprintf(cmdLine, sizeof(cmdLine), "rm -rf %s", mountPath);
	NK_SYSTEM(cmdLine);

	return 0;
}

static NK_Size onGetCapacity(NK_PChar mountPath)
{
	struct statvfs statFs;
	NK_Int64 capacity = 0;

	if(statvfs(mountPath, &statFs) < 0){
		NK_Log()->error("get Gapacity error");
		return 0;
	}

	capacity = statFs.f_blocks;
	capacity *= statFs.f_bsize;

	//×ª»»³É MB µ¥Î»
	capacity /= 1024;
	capacity /= 1024;
	NK_Log()->info("get Gapacity : %d MB",(NK_Size)capacity);

	return (NK_Size)capacity;
}

static NK_Size onGetFreeSpace(NK_PChar mountPath)
{
	struct statvfs statFs;
	NK_Int64 freeSpace = 0;

	if(statvfs(mountPath, &statFs) < 0){
		NK_Log()->error("get freeSpace error");
		return 0;
	}

	freeSpace = statFs.f_bavail;
	freeSpace *= statFs.f_bsize;

	//×ª»»³É MB µ¥Î»
	freeSpace /= 1024;
	freeSpace /= 1024;
//	NK_Log()->info("get freeSpace : %d MB",(NK_Size)freeSpace);

	return (NK_Size)freeSpace;
}

static int tfcard_dev_fdisk()
{
	const char *scriptPath = "/tmp/fdisk.script";
	char sysCommand[64];
	FILE* fID = NULL;
	const char scriptContent[] =
		"d\n"
		"1\n"
		"d\n"
		"2\n"
		"d\n"
		"3\n"
		"d\n"
		"4\n"
		"n\n"
		"p\n" // primary partition
		"1\n" // partition number
		"\n" // first cylinder
		"\n" // last cylinder
		"w\n"; // write

	fID = fopen(scriptPath, "w+b");
	if(NULL != fID){
		fwrite(scriptContent, 1, strlen(scriptContent), fID);
        fclose(fID);
		// fdisk
		snprintf(sysCommand, sizeof(sysCommand), "fdisk %s < %s", TFCARD_FDISK_PATH,scriptPath);
		NK_SYSTEM(sysCommand);
		REMOVE_FILE(scriptPath);

		return 0;
	}

	return -1;
}

static int tfcard_dev_mkfs()
{
	NK_Char cmdLine[64] = {0};

	snprintf(cmdLine, sizeof(cmdLine), "mkdosfs -F 32 %s", TFCARD_DEV_PATH);
	if(0 != system(cmdLine)){
		NK_Log()->error("***tfcard format error!!!");
		return -1;
	}
	NK_Log()->info("***tfcard format success!!!");

	return 0;
}

static int onFormat()
{
	if(0 != tfcard_dev_fdisk()){
		NK_Log()->error("*****tfcard fdisk error: dev = %s!!!!!", TFCARD_FDISK_PATH);
		return -1;
	}
	if( 0 != tfcard_dev_mkfs()){
		NK_Log()->error("*****tfcard mkfs error: dev = %s!!!!!", TFCARD_DEV_PATH);
		return -1;
	}

	return 0;
}

static int onAfterFormat(NK_Boolean status)
{
    // ä¸ç®¡æ ¼å¼åŒ–æ˜¯å¦æˆåŠŸ,éƒ½æŠŠæ ¼å¼åŒ–å‰åœæ­¢çš„å½•åƒçº¿ç¨‹é‡æ–°å¯åŠ¨
    TFCARD_start_record();

}

static NK_Size get_fit_dirty_ratio()
{
	struct sysinfo info = {0};
	NK_Int totalRam = 0;
	NK_Size ratio = 0;

	if(0 != sysinfo(&info)){
		NK_Log()->warn("get sysinfo error, so return fit dirty ratio : 3");
		return 3;
	}

	totalRam = (NK_Int)(info.totalram/1024/1024);//µÃµ½ÏµÍ³×ÜÄÚ´æ(µ¥Î»:M)
	if(totalRam == 0){
		NK_Log()->warn("get totalram error, so return fit dirty ratio : 3");
		return 3;
	}

	ratio = 300/totalRam;

	return (ratio == 0? 1 : ratio);
}

static NK_Void set_dirty_ratio()
{
	NK_Size ratio = 0;
	NK_Char cmdLine[64] = {0};

	ratio = get_fit_dirty_ratio();
	NK_Log()->info("get fit dirty ratio is : %d",ratio);
	//snprintf(cmdLine, sizeof(cmdLine), "echo %d > /proc/sys/vm/dirty_ratio", ratio);
	snprintf(cmdLine, sizeof(cmdLine), "echo 5 > /proc/sys/vm/dirty_ratio");

	NK_SYSTEM(cmdLine);
}

static NK_Size get_max_buffer_size_kb()
{
	struct sysinfo info = {0};
	NK_Int totalRam = 0;
	NK_Size sizeKb = 0;

	if(0 != sysinfo(&info)){
		NK_Log()->warn("get sysinfo error, so return default size : 384");
		sizeKb = 384;
	}

	totalRam = (NK_Int)(info.totalram/1024/1024);//µÃµ½ÏµÍ³×ÜÄÚ´æ(µ¥Î»:M)
	if(totalRam <= 64){
		NK_Log()->info("get totalram less than 64, totalram : %d, so return small size : 384",totalRam);
		sizeKb = 384;
	}else{
		NK_Log()->info("get totalram bigger than 64, totalram : %d, so return big size : 1024",totalRam);
		sizeKb = 1024;
	}

	return sizeKb;
}


#define RECORD_SEND_ERROR_MAX (3)
static NK_PVoid TFCARD_record(NK_PVoid argv)
{
	NK_PChar record_type = (NK_PChar)argv;
	NK_Int errorCnt = 0;
	NK_Int mediaBufId = 0;
	lpMEDIABUF_USER mediaBufUser = NULL;

	static stRecord_Frame_Head frameHead = {0};
	static NK_PByte frameData = NULL;
	static lpSDK_ENC_BUF_ATTR attr = NULL;
	NK_Size outSize = 0;

	pthread_detach(pthread_self());
	//»ñÈ¡mideaÓÃ»§
	if(-1 == (mediaBufId = MEDIABUF_lookup_byname("ch0_0.264"))){
		NK_Log()->error("lookup mediaBufId error");
		goto STH_ERR;
	}
	if(NULL == (mediaBufUser = MEDIABUF_attach(mediaBufId))){
		NK_Log()->error("mediaBufUser attach error");
		goto STH_ERR;
	}

	//Â¼Ïñ¿ªÊ¼
	if(0 != NK_TFCARD_record_start(record_type)){
		errorCnt = RECORD_SEND_ERROR_MAX;
	}
	usleep(500000);

#ifdef LED_CTRL
        initLedContrl(LED_REC_ID, true, LED_MIN_MODE);
#endif

	while(NULL != _app_record && _app_record->trigger && errorCnt < RECORD_SEND_ERROR_MAX)
	{
		NK_Boolean hasFrame = NK_False;

		if(0 == MEDIABUF_out_lock(mediaBufUser)){
			/*static stRecord_Frame_Head frameHead = {0};
			static NK_PByte frameData = NULL;
			lpSDK_ENC_BUF_ATTR attr = NULL;
			NK_Size outSize = 0;*/

			if(0 == MEDIABUF_out(mediaBufUser, (NK_PVoid)&attr, NULL, &outSize)) {
				if((attr->data_sz > 0) && (outSize > attr->data_sz)) {

					if(attr->type == kSDK_ENC_BUF_DATA_H264) {
						frameHead.codec = NK_TFCARD_VCODEC_H264;
						frameHead.coderStamp_ms = attr->timestamp_us / 1000;
						frameHead.sysTime_ms = attr->time_us / 1000;
						frameHead.dataSize = attr->data_sz;
						frameHead.isKeyFrame = attr->h264.keyframe;
						frameHead.fps = attr->h264.fps;
						frameHead.width = attr->h264.width;
						frameHead.height = attr->h264.height;
						frameData = (NK_PVoid)(attr+1);
						hasFrame = NK_True;
					}
					else if(attr->type == kSDK_ENC_BUF_DATA_H265) {
						frameHead.codec = NK_TFCARD_VCODEC_H265;
						frameHead.coderStamp_ms = attr->timestamp_us / 1000;
						frameHead.sysTime_ms = attr->time_us / 1000;
						frameHead.dataSize = attr->data_sz;
						frameHead.isKeyFrame = attr->h265.keyframe;
						frameHead.fps = attr->h265.fps;
						frameHead.width = attr->h265.width;
						frameHead.height = attr->h265.height;
						frameData = (NK_PVoid)(attr+1);
						hasFrame = NK_True;
					}
                    else if(attr->type == kSDK_ENC_BUF_DATA_AAC) {
                        frameHead.codec = NK_TFCARD_ACODEC_AAC;
                        frameHead.coderStamp_ms = attr->timestamp_us / 1000;
                        frameHead.sysTime_ms = attr->time_us / 1000;
                        frameHead.dataSize = attr->data_sz;
                        frameHead.sampleRate = attr->g711a.sample_rate;
                        frameHead.sampleWidth = attr->g711a.sample_width;
                        frameHead.samplePacket = attr->g711a.packet;
                        frameHead.compressionRatio = attr->g711a.compression_ratio;
                        frameData = (NK_PVoid)(attr+1);
                        hasFrame = NK_True;
                    }
					else if(attr->type == kSDK_ENC_BUF_DATA_G711A) {
						frameHead.codec = NK_TFCARD_ACODEC_G711A;
						frameHead.coderStamp_ms = attr->timestamp_us / 1000;
						frameHead.sysTime_ms = attr->time_us / 1000;
						frameHead.dataSize = attr->data_sz;
						frameHead.sampleRate = attr->g711a.sample_rate;
						frameHead.sampleWidth = attr->g711a.sample_width;
						frameHead.samplePacket = attr->g711a.packet;
						frameHead.compressionRatio = attr->g711a.compression_ratio;
						frameData = (NK_PVoid)(attr+1);
						hasFrame = NK_True;
					}
					//Ð´ÈëÖ¡Êý¾Ý
					if(hasFrame){
						if(0 != NK_TFCARD_record_write_frame(&frameHead,frameData)){
							errorCnt++;
							NK_Log()->warn("write frame error, errorCnt : %d !!!",errorCnt);
						}
					}
				}
			}
			MEDIABUF_out_unlock(mediaBufUser);
		}
		if(!hasFrame){
			usleep(10000);
		}
	}

#ifdef LED_CTRL
        initLedContrl(LED_REC_ID, true, LED_DARK_MODE);
#endif

	//Â¼Ïñ½áÊø
	NK_TFCARD_record_stop();

	//Çå³ýÐÅºÅÁ¿,ÍË³öÏß³Ì
STH_ERR:
	if(mediaBufUser){
		MEDIABUF_detach(mediaBufUser);
		mediaBufUser = NULL;
	}
	if(_app_record)
	{
		free(_app_record);
		_app_record = NULL;
	}
	pthread_exit(NULL);
}

static NK_PVoid record_delay(NK_PVoid arg)
{
	pthread_detach(pthread_self());
	while(NULL != _app_record && _app_record->delay_sec > 0){
		if(!NK_TFCARD_is_mounted()){
			_app_record->delay_sec = 0;
		}else{
			--_app_record->delay_sec;
			sleep(1);
		}
	}

	if(_app_record){
		free(_app_record);
		_app_record = NULL;
	}
	pthread_exit(NULL);
}

NK_Int TFCARD_record_start(NK_PChar record_type)
{
	NK_PChar argv = NULL;

	//Ä£¿éÎ´³õÊ¼»¯
	if(!gs_inited){
		return -1;
	}

	//µ± TF ¿¨Ã»ÓÐ¹ÒÔØµÄÊ±ºò,²»½øÐÐÂ¼Ïñ
	if(!NK_TFCARD_is_mounted()){
		//APP_TRACE("TFCARD not mount,record start error");
		return -1;
	}
	//µ±Ê±¼äÎª¹«Ôª2016ÄêÇ°,²»½øÐÐÂ¼Ïñ
	if(time(NULL) < 1451577600){
		if(_app_record){
			_app_record->delay_sec = 0;
		}
		return -1;
	}
	if(!_app_record){
		_app_record = calloc(sizeof(stAPP_RECORD),1);
		_app_record->delay_sec = 30;
		_app_record->trigger = NK_TRUE;
		pthread_create(&_app_record->delay_pid, NULL, record_delay, NULL);
		argv = record_type;
		pthread_create(&_app_record->record_pid, NULL, TFCARD_record, (NK_PVoid)argv);
	}else{
		_app_record->delay_sec = 30;
	}

	return 0;
}

static bool
is_record_on_schedule()
{
    int beginSec, endSec;
    int i;
    unsigned int cur_weekday = 0;
	unsigned int cur_sec = 0;

	time_t cur_time;
	struct tm *p;

    ST_NSDK_SYSTEM_SETTING sinfo = {0};


    if (NULL == NETSDK_conf_system_get_setting_info(&sinfo)) {
        APP_TRACE("NETSDK_conf_system_get_setting_info Failed!");
        return true;
    }

    cur_time = time(NULL);
    if (cur_time < 0) {
        APP_TRACE("time() Failed! ret: %ld, errno: %d", cur_time, errno);
        return true;
    }

	p = localtime(&cur_time);
    if (NULL == p) {
        APP_TRACE("localtime() return NULL!");
        return true;
    }

	cur_weekday = 1 << p->tm_wday;
	cur_sec = p->tm_hour * 3600 + p->tm_min * 60 + p->tm_sec;

	for (i = 0
			; i < sizeof(sinfo.TFcard_Record.Schedule)
				/ sizeof(sinfo.TFcard_Record.Schedule[0]);
			++i)
	{

        if (!(sinfo.TFcard_Record.Schedule[i].enabled)) {
            continue;
        }

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

static bool timeRecEnabled()
{
    ST_NSDK_SYSTEM_SETTING sinfo = {0};
    if (NULL == NETSDK_conf_system_get_setting_info(&sinfo)) {
        APP_TRACE("NETSDK_conf_system_get_setting_info Failed!");
        return false;
    }

    return sinfo.timeRecordEnabled;

}

/*
 * Wait for tf card physical i/o finished.
 *
 * *Should call this Function after 'sync' command be executed.*
 *
 * ret: 0 OK; -1 No TF Card; -2 TF Card Status Unexpected
 */
static int tfcard_wait_phy_io(void)
{
    int ret = 0;

#if defined(HI3516D)

    uint32_t reg_addr = 0x206E0048;
    uint32_t reg_val;

    char str[32];
    int tfcard_status;

    while (1) {

        BSP_read_reg((unsigned)reg_addr, &reg_val);
        APP_TRACE("TF Card I/O Status Value: 0x%08X", reg_val);

        tfcard_status = NK_TFCARD_get_status(str);
        if (emTFCARD_STATUS_OK == tfcard_status) {
            if ((0x00006906 == reg_val) || (0x00006106 == reg_val)) {
                // 0x00006906 --> tf card write finished
                // 0x00006106 --> tf card idle
                ret = 0;
                break;
            } else {
                // tf card doing i/o
                ;
            }
        } else if (emTFCARD_STATUS_NO_TFCARD == tfcard_status) {
            // no tf card
            ret = -1;
            break;
        } else {
            // tf card status unexpected
            ret = -2;
            break;
        }

        usleep(300000);
    }

#endif

    return ret;
}

static NK_Void TFCARD_on_schedule_record(NK_Void)
{
	pthread_detach(pthread_self());
	while (1)
	{
		if(is_record_on_schedule() == true){
			TFCARD_record_start("schedule");
		}
		sleep(1);
	}
	pthread_exit(NULL);
}

static NK_Void timer_flush_log()
{
	NK_Log()->flush();
}

/* ¹¦ÄÜ:ÓÃÓÚ¼ì²âÊÇ·ñÔÚ½øÐÐÂ¼Ïñ */
int TFCARD_is_record_now()
{
    if(_app_record) {
        return 1;
    }
    else {
        return 0;
    }

    return 0;
}

#include "rl_pack.h"
TF_RL_Pack_t gRL;

static int tfcard_wait_mp4rec_thr_stop(size_t timeout_ms)
{
	size_t sleep_us = 50000;
	ssize_t total_sleep_us = 0;
	size_t timeout_us = 1000 * timeout_ms;

	while (0 != gRL.RunningFlag) {
		if(total_sleep_us > timeout_us){
			APP_TRACE("%s: Time out waiting mp4 rec thread to stop! timeout ms: %u",
					  __FUNCTION__, timeout_ms);
			return -1;
		}
		usleep(sleep_us);
		total_sleep_us += sleep_us;
	}

	return 0;
}

int TFCARD_init(int use_samba, char *path)
{
    ST_TFCARD_FUNCTION init;
	NK_Size maxBufferSizeKb = 0;

    if (0 != use_samba) {
        init.onExistTF = NULL;
        init.OnDetectTF = NULL;
        init.OnMountTF = NULL;
        init.OnUmountTF = NULL;
        init.OnFormat = NULL;
    } else {
        init.onExistTF = onExistTF;
        init.OnDetectTF = onDetectTF;
        init.OnMountTF = onMountTF;
        init.OnUmountTF = onUmountTF;
        init.OnFormat = onFormat;
    }
    init.OnCleanTF = onCleanTF;
	init.OnGetCapacity = onGetCapacity;
	init.OnGetFreeSpace = onGetFreeSpace;
    init.OnAfterFormat = onAfterFormat;
	maxBufferSizeKb = get_max_buffer_size_kb();

	if (0 != use_samba) {
		NK_TFCARD_init(&init, path, maxBufferSizeKb);
	} else {
		NK_TFCARD_init(&init, TFCARD_FS_PATH, maxBufferSizeKb);
	}
	set_dirty_ratio();

	TICKER_add_task(timer_flush_log, 120, false);

	memset(&gRL.st_TF_record_parameter, 0, sizeof(ST_TF_RECORD_PARAMETER));
	gRL.LockingFlag = 0;
	gRL.PthreadID = (pthread_t)NULL;
	gRL.RunningFlag = 0;
	gRL.Update = 0;

	gs_inited = true;

    return 0;
}



int TFCARD_start_record_thread()
{
	int ret = 0;

	if (!gs_inited) {
		APP_TRACE("tfcard module hasn't been inited!");
		return -1;
	}

	if (tfcard_check_pid) {
		APP_TRACE("tfcard thread already started!");
		return -1;
	}

	ret = pthread_create(&tfcard_check_pid, NULL, TFCARD_record_check, NULL);
	if(0 != ret){
		APP_TRACE("Failed to start record check thread!");
		tfcard_check_pid = (pthread_t)NULL;
		return -1;
	}

	return 0;
}


static int TFMp4Rec_Record(TF_RL_Pack_t * rl)
{
	TF_RL_Pack_t *tmpRL = (TF_RL_Pack_t *)rl;

	APP_TRACE("mp4 record start");

	if(NULL == tmpRL) {
		APP_TRACE("mp4 record start failed! tmpRL can't be NULL");
		return -1;
	}

	NK_Int errorCnt = 0;
	NK_Int mediaBufId = 0;
	lpMEDIABUF_USER mediaBufUser = NULL;
    bool errOccur = false;

	stRecord_Frame_Head frameHead = {0};
	//NK_PByte frameData = NULL;
	lpSDK_ENC_BUF_ATTR attr = NULL;
	NK_Size outSize = 0;
	
	RL_Cxt_t tmpRLCxt;

    char tmp_saved_rec_type = '\0';
	int tmp_saved_v_fps = -1;
	bool rec_need_new_pack = false;
#ifdef GSENSOR
    stGsensor_angles gsensorAngles;
    unsigned char *gsensorAnglesFrame = NULL;
#endif

#if defined(HI3516E_V1)
    unsigned long long initUsPts = 0;
#endif

    if(0 != RL_Init(&tmpRLCxt, RL_PATH)) {
		APP_TRACE("RL_Init() Failed!!!");
		return -1;
	}


	tmpRLCxt.Sta.notice_to_pack = 0;
	tmpRLCxt.Sta.motion_or_time[0] = gRL.st_TF_record_parameter.record_type;
	tmpRLCxt.Sta.motion_or_time[1] = '\0';

	//APP_TRACE("tmpRLCxt.Sta.motion_or_time = %s ",tmpRLCxt.Sta.motion_or_time);
		
	
	//»ñÈ¡mideaÓÃ»§
	if(-1 == (mediaBufId = MEDIABUF_lookup_byname("ch0_0.264"))){
		APP_TRACE("lookup mediaBufId error!");
        errOccur = true;
		goto STH_ERR;
	}
	if(NULL == (mediaBufUser = MEDIABUF_attach(mediaBufId))){
		APP_TRACE("mediaBufUser attach error!");
        errOccur = true;
		goto STH_ERR;
	}

#if defined(HI3516E_V1)
	SDK_ENC_get_enc_pts(0, 0, &initUsPts);
#endif

#ifdef LED_CTRL
    initLedContrl(LED_REC_ID, true, LED_MIN_MODE);
#endif

#define TFMP4_RECORD_ERR_MAX (8)
	while(tmpRL->LockingFlag && (errorCnt < TFMP4_RECORD_ERR_MAX)) {
		NK_Boolean hasFrame = NK_False;
		int tmpByte         = 0;

		if(!NK_TFCARD_is_mounted()) { //TF Card not Exist;
			//FIX
            errOccur = true;
			break;
		}

		tmpByte = NK_TFCARD_get_freespace();
		if(tmpByte < 256) { //Min 256MByte Spare Size On Disk;
			/* ÏÈÉ¾³ý¾É¸ñÊ½Â¼Ïñ */
			int ret = 1;
			if(GLOBAL_isOldTypeRecord() == true) {
				RL_Cxt_t tmpRLCxtOld;  // Ö»ÊÇÓÃÓÚ´«¾ÉÂ¼Ïñ±£´æÄ¿Â¼
				char cmd[32];
				snprintf(tmpRLCxtOld.Env.BasePath, sizeof(tmpRLCxtOld.Env.BasePath), "%s", TFCARD_OLD_RECORD_PATH);
				ret = RL_DelOldestFiles(&tmpRLCxtOld, 20); // Ò»´ÎÉ¾³ý20¸ö¾ÉÂ¼ÎÄ¼þ
				if(ret == -2) {  // TFCARD_OLD_RECORD_PATHÃ»ÓÐ×ÓÄ¿Â¼
					snprintf(cmd, sizeof(cmd), "rm %s -rf", TFCARD_OLD_RECORD_PATH);
					NK_SYSTEM(cmd);
					GLOBAL_setOldTypeRecordFlag();  // ¸üÐÂ×´Ì¬
					continue;
				}
				else if(ret == -1) {
					APP_TRACE("Failed to delete old files!");
					errOccur = true;
					break;
				}
			}
			else {
#if defined(HI3516E_V1)
                // to Delete A Least 10 Files;
                if (0 != RL_DelOldestFiles(&tmpRLCxt, 10)){
#else
				// to Delete A Least 3 Files;
				if (0 != RL_DelOldestFiles(&tmpRLCxt, 3)){
#endif
					APP_TRACE("Failed to delete old files!");
					errOccur = true;
					break;
				}
			}

		}

		if(0 == MEDIABUF_out_lock(mediaBufUser)) {
			if(0 == MEDIABUF_out(mediaBufUser, (NK_PVoid)&attr, NULL, &outSize)) {
#if defined(HI3516E_V1)
                if(attr->timestamp_us < initUsPts)
                {
                    SDK_ENC_request_stream_keyframe(0, 0);
                    MEDIABUF_out_unlock(mediaBufUser);
                    continue;
                }
#endif
				if((attr->data_sz > 0) && (outSize > attr->data_sz)) {
					int FrameType = PACK_FRAME_TYPE_NONE;
					int CodecType = PACK_CODEC_TYPE_H264;
					if(attr->type == kSDK_ENC_BUF_DATA_H264) {
						frameHead.codec = NK_TFCARD_VCODEC_H264;
						frameHead.coderStamp_ms = attr->timestamp_us / 1000;
						frameHead.sysTime_ms = attr->time_us / 1000;
						frameHead.dataSize = attr->data_sz;
						frameHead.isKeyFrame = attr->h264.keyframe;
						frameHead.fps = SENSOR_GET_CUR_FPS();
						frameHead.width = attr->h264.width;
						frameHead.height = attr->h264.height;
						//frameData = (NK_PVoid)(attr+1);
						hasFrame = NK_True;
						FrameType = attr->h264.keyframe ? PACK_FRAME_TYPE_VIDEO_IFRAME : PACK_FRAME_TYPE_VIDEO_PFRAME;
						CodecType = PACK_CODEC_TYPE_H264;
					}
					else if(attr->type == kSDK_ENC_BUF_DATA_H265) {
						frameHead.codec = NK_TFCARD_VCODEC_H265;
						frameHead.coderStamp_ms = attr->timestamp_us / 1000;
						frameHead.sysTime_ms = attr->time_us / 1000;
						frameHead.dataSize = attr->data_sz;
						frameHead.isKeyFrame = attr->h265.keyframe;
						frameHead.fps = SENSOR_GET_CUR_FPS();
						frameHead.width = attr->h265.width;
						frameHead.height = attr->h265.height;
						//frameData = (NK_PVoid)(attr+1);
						hasFrame = NK_True;
						FrameType = attr->h265.keyframe ? PACK_FRAME_TYPE_VIDEO_IFRAME : PACK_FRAME_TYPE_VIDEO_PFRAME;
						CodecType = PACK_CODEC_TYPE_H265;
					}
					else if(attr->type == kSDK_ENC_BUF_DATA_AAC) {
						frameHead.codec = NK_TFCARD_ACODEC_AAC;
						frameHead.coderStamp_ms = attr->timestamp_us / 1000;
						frameHead.sysTime_ms = attr->time_us / 1000;
						frameHead.dataSize = attr->data_sz;
						frameHead.sampleRate = attr->g711a.sample_rate;
						frameHead.sampleWidth = attr->g711a.sample_width;
						frameHead.samplePacket = attr->g711a.packet;
						frameHead.compressionRatio = attr->g711a.compression_ratio;
						//frameData = (NK_PVoid)(attr+1);
						hasFrame = NK_True;
						FrameType = PACK_FRAME_TYPE_AUDIO;
					}

					//Ð´ÈëÖ¡Êý¾Ý
					if(hasFrame) {
						RL_Frame_t tmpFrame;
						memset(&tmpFrame, 0, sizeof(tmpFrame));
#if !defined(HI3516E_V1)
						// ÊÓÆµÖ¡ÂÊ»òÂ¼ÏñÀàÐÍ·¢Éú±ä»¯Ê±£¬Â¼ÏñÐèÒªÖØÐÂ´ò°ü¡£Â¼ÏñÀàÐÍ±ä»¯ÐèÒªÇëÇóÒ»´Î I Ö¡
						if (PACK_FRAME_TYPE_VIDEO_PFRAME == FrameType
							|| PACK_FRAME_TYPE_VIDEO_IFRAME == FrameType) {
							if (tmp_saved_v_fps <= 0) {
								tmp_saved_v_fps = frameHead.fps;
							} else if (tmp_saved_v_fps != frameHead.fps) {
								rec_need_new_pack = true;
								tmp_saved_v_fps = frameHead.fps;
							}
						}
#endif

						tmpRLCxt.Sta.motion_or_time[0] = gRL.st_TF_record_parameter.record_type;
						if ('\0' == tmp_saved_rec_type) {
                            tmp_saved_rec_type = tmpRLCxt.Sta.motion_or_time[0];
                        } else if (tmp_saved_rec_type != tmpRLCxt.Sta.motion_or_time[0]) {
                            rec_need_new_pack = true;
                            tmp_saved_rec_type = tmpRLCxt.Sta.motion_or_time[0];
                        }

                        if (rec_need_new_pack) {
                            SDK_ENC_request_stream_keyframe(0, 0);
                            tmpRLCxt.Sta.notice_to_pack = 1;
                            rec_need_new_pack = false;
                        }

						tmpFrame.FrameBuff = (NK_PVoid)(attr+1);
						tmpFrame.FrameSize = attr->data_sz;
						tmpFrame.FrameType = FrameType;
						tmpFrame.FrameRate = frameHead.fps;
						tmpFrame.BitRate   = 4000*1024;
						tmpFrame.PicWSize  = 1920;
						tmpFrame.PicHSize  = 1080;
						tmpFrame.TimeStamp = frameHead.sysTime_ms;
#if defined(HI3516E_V1)
						tmpFrame.CodecTimeStamp = frameHead.coderStamp_ms;
#endif
						tmpFrame.CodecType = CodecType;  // Ä¿Ç°Ö»´¦ÀíÁËÊÓÆµ±àÂë
						if(0 == RL_Write(&tmpRLCxt, &tmpFrame)) {
							errorCnt  = 0;
#ifdef GSENSOR
                            if(APP_GSENSOR_is_support()) {
                                if((FrameType == PACK_FRAME_TYPE_VIDEO_IFRAME) || (FrameType == PACK_FRAME_TYPE_VIDEO_PFRAME)) {                                    
                                    static int tmpCount = 0;
                                    if(tmpCount < 15) {  // å¤§æ¦‚ä¸€ç§’ä¸€å¸§Gsensor oob
                                        tmpCount++;
                                        MEDIABUF_out_unlock(mediaBufUser);
                                        continue;
                                    }
                                    tmpCount = 0;
                                    tmpFrame.FrameType = PACK_FRAME_TYPE_DATA;
                                    tmpFrame.TimeStamp = frameHead.sysTime_ms;
                                
                                    if(NULL == gsensorAnglesFrame) {
                                        gsensorAnglesFrame = alloca(sizeof(ST_GSENSOR_ANGLES) + sizeof(ST_GSENSOR_FRAME));
                                    }

                                    if(NULL != gsensorAnglesFrame) {
                                        if(0 == APP_GSENSOR_get_angles_frame((void *)gsensorAnglesFrame, &tmpFrame.FrameSize)) {
                                            tmpFrame.FrameBuff = gsensorAnglesFrame;
                                            RL_Write(&tmpRLCxt, &tmpFrame);
                                        }
                                    }
                                }
                            }
#endif
						}
						else {
							//FIX
							//Ä¿Ç°½öÌ½²âÊÓÆµÖ¡Ð´Èë´íÎó
							if((attr->type == kSDK_ENC_BUF_DATA_H264)||(attr->type == kSDK_ENC_BUF_DATA_H265)){
								errorCnt += 1;
								APP_TRACE("write frame error, errorCnt : %d !!!", errorCnt);
								if(errorCnt >= TFMP4_RECORD_ERR_MAX){
                                    errOccur = true;
									TF_Can_Read_Write(5);
								}
							}
						}
					}
				}
			}
			MEDIABUF_out_unlock(mediaBufUser);
		}
		if(!hasFrame){
			usleep(10000);
		}
	}

	//FIX
	RL_Exit(&tmpRLCxt);
	//Çå³ýÐÅºÅÁ¿,ÍË³öÏß³Ì

STH_ERR:
	if(mediaBufUser){
		MEDIABUF_detach(mediaBufUser);
		mediaBufUser = NULL;
	}

	APP_TRACE("mp4 record end");

    if (errOccur) {
        return -1;
    } else {
	    return 0;
    }
}


//¼ì²éÂ¼ÏñÊÇ·ñÆô¶¯
static void* TFCARD_record_check(void *arg)
{
	int64_t motion_cur_ms = 0;
	int64_t motion_end_ms = 0;
    // ÓÃÕâ¸ö±ê¼Ç±ÜÃâÃ¿¸öÑ­»·¶¼È¥»ñÈ¡Ê±¼ä
    bool in_motion = false;

	APP_TRACE("Record check thread started.");

	prctl(PR_SET_NAME, "tf record check");

//	sleep(30); //µÈ´ý sdk³õÊ¼»¯Íê³É

#ifdef TF_RECORD_AUTO_TEST
	while((gRL.Update == 0) && (st_test_tf_autio_record.is_record_autio_test_run == 0))
#else
	while(gRL.Update == 0)
#endif
	{
        //¶ÁÈ¡Â¼ÏñÉèÖÃÀàÐÍ  timing/motion
        if (gRL.st_TF_record_parameter.motion_check == 1) {
            gRL.st_TF_record_parameter.motion_check = 0;

#if defined(PIR_ALARM)
            // PIR
            NETSDK_conf_system_get_record_info(&gRL.st_TF_record_parameter.recManager);
            if(gRL.st_TF_record_parameter.recManager.useIOAlarm == false || \
                HICHIP_get_io_alarm_status())
            {
#endif
                tfcard_get_cur_mono_ms(&motion_cur_ms);
                motion_end_ms = motion_cur_ms + 1000 * RECORD_MOTION_TIME_S;
                in_motion = true;
#if defined(PIR_ALARM)
            }
#endif
        } else if (in_motion) {
            tfcard_get_cur_mono_ms(&motion_cur_ms);
        }

        // ÒÆ¶¯Õì²âÂ¼ÏñºÍ¶¨Ê±Â¼ÏñÍ¬Ê±Âú×ã£¬ÔòÈÏÎªÊÇÒÆ¶¯Õì²âÂ¼Ïñ
        if (in_motion && motion_cur_ms < motion_end_ms) {

            gRL.st_TF_record_parameter.is_on_schedule_time = 1;

#if !defined(DANA_P2P)
            /* FIXMEæš‚æ—¶å±è”½motionå½•åƒç±»åž‹ */
            //tfcard_start_record("motion");
            tfcard_start_record("time");
#else
            tfcard_start_record("motion");
#endif
        } else if (timeRecEnabled() && is_record_on_schedule()) {

            gRL.st_TF_record_parameter.is_on_schedule_time = 1;
            tfcard_start_record("time");
            in_motion = false;

        } else {

            gRL.st_TF_record_parameter.is_on_schedule_time = 0;
            gRL.LockingFlag = 0;
            in_motion = false;
        }

		usleep(500000);
	}

	APP_TRACE("Record check thread quit.");
	pthread_exit(NULL);
}




static void* TFMp4RecDaemon_Thread(void* arg)
{
	TF_RL_Pack_t *tmpRL = (TF_RL_Pack_t *)arg;
	char threadName[16] = {0};

	tmpRL->RunningFlag = 1;
	APP_TRACE("Mp4 rec daemon thread started.");

	snprintf(threadName, sizeof(threadName), "TFREC_%c", tmpRL->st_TF_record_parameter.record_type);
	prctl(PR_SET_NAME, (unsigned long)threadName);

	pthread_detach(pthread_self());

	if(NULL == tmpRL) {
		goto THREAD_EXIT;
	}
	while(tmpRL->LockingFlag) {
		int RLRet;
		//µ± TF ¿¨Ã»ÓÐ¹ÒÔØµÄÊ±ºò,²»½øÐÐÂ¼Ïñ
		if(!NK_TFCARD_is_mounted()){
			//APP_TRACE("TFCARD not mounted");
#ifdef LED_CTRL
            initLedContrl(LED_REC_ID, true, LED_DARK_MODE);
#endif
            usleep(500000);
			continue;
		}
		
		//µ±Ê±¼äÎª¹«Ôª2016ÄêÇ°,²»½øÐÐÂ¼Ïñ
		if(time(NULL) < 1451577600){
#ifdef LED_CTRL
            initLedContrl(LED_REC_ID, true, LED_DARK_MODE);
#endif
            usleep(500000);
			//APP_TRACE(" *********** time can not before year,please set system time!*************** ");
			continue;
		}
		
		RLRet = TFMp4Rec_Record(tmpRL);
		if(0 != RLRet) {
#ifdef LED_CTRL
            initLedContrl(LED_REC_ID, true, LED_DARK_MODE);
#endif
            APP_TRACE("TFMp4Rec_Record failed, ret: %d", RLRet);
            sleep(2);
		}
	}

    system("sync");
    tfcard_wait_phy_io();

#ifdef LED_CTRL
    initLedContrl(LED_REC_ID, true, LED_DARK_MODE);
#endif

THREAD_EXIT:

	tmpRL->RunningFlag = 0;
	tmpRL->LockingFlag = 0;
	APP_TRACE("Mp4 rec daemon thread quit.");
	pthread_exit(NULL);
}


#ifdef TF_RECORD_AUTO_TEST

static NK_Void* TF_record_autio_test(NK_Void){
	NK_UInt8 ret = 0,record_data = 0;
	
	gRL.LockingFlag = 0;
	gRL.RunningFlag = 0;
	
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "TF_record_autio_test");
	while(st_test_tf_autio_record.is_record_autio_test_run){
		if(NK_TFCARD_is_mounted()){
			//Â¼Ïñ²âÊÔÎÄ¼þ
			if(record_data = (test_tf_record_auto_run(200))){
				if((gRL.LockingFlag == 0) && (gRL.RunningFlag == 0)){
					gRL.LockingFlag = 1;
					gRL.RunningFlag = 0;
					//APP_TRACE(" MP4 Recording start ");
					if(record_data == 2){
						gRL.st_TF_record_parameter.record_type = 'M';
						
					}else{
						gRL.st_TF_record_parameter.record_type = 'T';
					}
					gRL.st_TF_record_parameter.sec_delay = atoi(st_tf_record_setting.rec_lenght);
					ret = pthread_create(&gRL.PthreadID, NULL, TFMp4RecDaemon_Thread, &gRL);
					if(0 != ret){
						NK_Log()->error("Failed to start thread tfcard_rename_start_rec_thr", TFCARD_DEV_PATH);
					}
					
				}else{
					//APP_TRACE("MP4 Recording ....... ");
				}
			}else{
				if(gRL.st_TF_record_parameter.record_type == 'T'){
					gRL.LockingFlag = 0;
				}
				//APP_TRACE("MP4 Recording stop");
			}
			usleep(200000);
		}else{
			//APP_TRACE(" NO TF card !");
			sleep(1);
		}
	}
	
	APP_TRACE(" TF_record_autio_test thread exit ! ");
	pthread_exit(NULL);
}


static NK_Void  TF_record_autio_init(void){
	int ret;

	if(st_test_tf_autio_record.is_power_run == 0 ){
		APP_TRACE("  test_auto_record init! ");
		st_test_tf_autio_record.is_power_run = 1;
		if(test_tf_record_auto_init()){
			APP_TRACE("  test auto record start!");
			st_test_tf_autio_record.is_record_autio_test_run = 1;
			st_test_tf_autio_record.freespace_printf_num = 0;
			ret = pthread_create(&st_test_tf_autio_record.tf_record_autio_test_pid, NULL, TF_record_autio_test, NULL);
			if(0 != ret){
				NK_Log()->error("Failed to start thread TF_record_autio_test !");
			}
		}
	}
}


#endif

void TFCARD_destroy()
{
	gs_inited = false;

	TFCARD_stop_record();

	NK_TFCARD_destroy();
	APP_TRACE("TFCARD destory success!");
	NK_Log()->flush();
}


int tfcard_start_record(char *record_type)
{
	int ret;

	if(gRL.Update == 1){
		//APP_TRACE("start record is stop, becase update! ");
		return -1;
	}
#ifdef TF_RECORD_AUTO_TEST
	TF_record_autio_init();
	//Èç¹û°´ÕÕTF¿¨ÅäÖÃÎÄ¼þ½øÐÐÂ¼Ïñ£¬Ôò²»½øÐÐÊ±¼äºÍÒÆ¶¯Â¼Ïñ
	if(st_test_tf_autio_record.is_record_autio_test_run){
		//APP_TRACE(" is_record_autio_test_run ! ");
		return -1;
	}
	
#endif
	//²»Âú×ãÂ¼ÏñÊ±¼ä
	if(gRL.st_TF_record_parameter.is_on_schedule_time ==0 ){
		//APP_TRACE("is not on record scheld time !");
		return -1;
	}
	if((gRL.LockingFlag == 0) && (gRL.RunningFlag == 0)){
		//Â¼ÏñÏß³ÌÃ»ÓÐÔËÐÐ£¬Æô¶¯Â¼ÏñÏß³Ì
		gRL.LockingFlag = 1;
		gRL.RunningFlag = 0;

		//Â¼Ïñ½öÓÐÒÆ¶¯ºÍ¶¨Ê±
		if (strstr(record_type,"motion")) {
//			APP_TRACE(" motion record !");
			gRL.st_TF_record_parameter.record_type = 'M';
		}else{
//			APP_TRACE(" time record !");
			gRL.st_TF_record_parameter.record_type = 'T';
		}
		ret = JA_THREAD_init0(&gRL.PthreadID, TFMp4RecDaemon_Thread, &gRL, NULL, 0, NULL, 131072, 0);
		if(0 != ret){
			APP_TRACE("Failed to start mp4 rec daemon thread");
			gRL.LockingFlag = 0;
			return -1;
		}
	}else{
		//Â¼ÏñÏß³ÌÒÑ¾­ÔÚÔËÐÐ£¬²»ÐèÒªÆô¶¯
		if(strstr(record_type,"motion")){
//			APP_TRACE(" motion record !");
			gRL.st_TF_record_parameter.record_type = 'M';
		}else{
//			APP_TRACE(" time record !");
			gRL.st_TF_record_parameter.record_type = 'T';
		}
		return 0;
	}
	
	return 0;
}




int TFCARD_stop_record(void)
{
	int ret;
	
	gRL.Update = 1;
	gRL.LockingFlag = 0;

    if (tfcard_check_pid) {
        ret = pthread_join(tfcard_check_pid, NULL);
        if (0 != ret) {
			APP_TRACE("Failed to join TFCARD_record_check thread!");
        }
        tfcard_check_pid = (pthread_t)NULL;
    }

	if (0 != tfcard_wait_mp4rec_thr_stop(10000)) {
		APP_TRACE("TFCARD record stop fail!");
		return -1;
	}

	APP_TRACE("TFCARD record stop success!");

    return 0;
}

int TFCARD_start_record(void)
{
    int ret;

    if (!gs_inited) {
        APP_TRACE("tfcard module hasn't been inited!");
        return -1;
    }

    if (tfcard_check_pid) {
        APP_TRACE("tfcard thread already started!");
        return -1;
    }

    gRL.RunningFlag = 0;
    gRL.Update = 0;
    gRL.LockingFlag = 0;

    ret = JA_THREAD_init0(&tfcard_check_pid, TFCARD_record_check, NULL, NULL, 0, NULL, 131072, 0);
    if(0 != ret) {
        APP_TRACE("Failed to start record check thread!");
        tfcard_check_pid = (pthread_t)NULL;
        gRL.Update = 1;
        gRL.LockingFlag = 0;
        return -1;
    }

    return 0;

}

