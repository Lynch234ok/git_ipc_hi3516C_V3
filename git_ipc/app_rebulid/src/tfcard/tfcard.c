#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "macro.h"
#include <NkUtils/assert.h>
#include <base/ja_process.h>
#include <generic.h>

#include "tfcard.h"
#include "tfcard_record.h"
#include "tfcard_play.h"
#include "app_debug.h"
#include <sys/prctl.h>
#include "inifile.h"
#include "netsdk_def.h"
#include "../sound.h"
#include "base/cross_thread.h"



#define kNK_TFCARD_SOUND_FLAG "/tmp/TF_SOUND_FLAG"
#define DEBUG_INI_FILE_PATH "/media/tf/debug.ini"


typedef enum _Format_Status {
	emFormat_Status_none = 0,
	emFormat_Status_start_format,
	emFormat_Status_already_format,
}emFormat_Status;

typedef struct _TFCARD_ATTR
{
	pthread_t detector_pid;
	NK_Boolean detector_trigger;
	struct {
		emFormat_Status formatStatus;
		NK_Boolean mounted;
		NK_Char fs_path[32];
	}TFSlot;
	ST_TFCARD_FUNCTION EventSet;
	NK_Size max_buffer_size_kb;
}ST_TFCARD_ATTR, *LP_TFCARD_ATTR;


static LP_TFCARD_ATTR _tfcard = NULL;

// 此变量表示 tf 卡刚插入。用于播放 tf 异常语音
static NK_Boolean gs_tfcard_just_insert = NK_False;
static NK_Boolean gs_tfcard_check_turned_on = NK_False;


static NK_Boolean is_path_mounted(const char *path)
{
	const char *lineRet = NULL;
	const char *subStrRet = NULL;
	const char *proc_mounts = "/proc/mounts";
	FILE *fd_proc_mounts = NULL;
	char line[512];
	int line_n = sizeof(line);

	if (NULL == path) {
		printf("%s: mount dir can't be NULL\n", __FUNCTION__);
		return NK_False;
	}

	fd_proc_mounts = fopen(proc_mounts, "r");
	if (NULL == fd_proc_mounts) {
		printf("%s: Failed to open file: %s\n", __FUNCTION__, proc_mounts);
		return NK_False;
	}

	// 逐行查找 path
	while (1) {
		lineRet = fgets(line, line_n, fd_proc_mounts);
		if (NULL != lineRet) {
			subStrRet = strstr(lineRet, path);
			if (NULL != subStrRet) {
				break;
			}
		} else {
			break;
		}
	}

	fclose(fd_proc_mounts);

	// 找到 dir 说明已挂载，否则未挂载
	if (NULL == subStrRet) {
		return NK_False;
	} else {
		return NK_True;
	}
}

static NK_Void NK_TFCARD_check_debug_ini()
{
	if(IS_FILE_EXIST(DEBUG_INI_FILE_PATH)){
		lpINI_PARSER inf = NULL;
		inf = OpenIniFile(DEBUG_INI_FILE_PATH);
		int if_open_telnet = 0;
		int if_force_AP = 0;
		if_open_telnet = inf->read_int(inf, "wifi", "telnet", 0);
		if(if_open_telnet == 1){
			APP_TRACE("open telnet for debug");
			if(NK_PROCESS_find_by_name("telnetd") == 0){
				NK_SYSTEM("telnetd &");
			}
			else{
				APP_TRACE("telnetd is exist!");
			}
		}
		
		if_force_AP = inf->read_int(inf, "wifi", "forceToAP", 0);
		//force AP
		if(if_force_AP == 1){
			APP_TRACE("open AP to test ");
			ST_NSDK_NETWORK_INTERFACE wlan;
			NETSDK_conf_interface_get(4, &wlan);
			wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT;
			//NETSDK_tmp_interface_set(4, &wlan);
		}
		
	}
}

static void nk_tfcard_touch_file(char *filename)
{
	FILE *file;

	file = fopen(filename, "w");
	if (NULL == file) {
		APP_TRACE("%s: Can't create file: %s!!", __FUNCTION__, filename);
		return;
	}
	fclose(file);
}

/*
 * 用于保证 tf 卡异常语音只播放一次(在不重复插卡的情况下)
 */
static void nk_tfcard_play_exception_sound(void)
{
	if (!gs_tfcard_check_turned_on) {
        return;
	}

	if (gs_tfcard_just_insert) {

		SearchFileAndPlay(SOUND_SD_card_exception, NK_False);
        gs_tfcard_just_insert = NK_False;
		nk_tfcard_touch_file(kNK_TFCARD_SOUND_FLAG);

	} else if (!IS_FILE_EXIST(kNK_TFCARD_SOUND_FLAG)) {

		SearchFileAndPlay(SOUND_SD_card_exception, NK_False);
        nk_tfcard_touch_file(kNK_TFCARD_SOUND_FLAG);
    }
}

static NK_Void detector(void)
{
	NK_UInt16 count = 0;
	prctl(PR_SET_NAME, "tfcard_detector");

	NK_Boolean detected = NK_True;
	NK_Boolean tmp_detected = NK_True;
	NK_Boolean existed = NK_True;
	NK_Size free_size;
	NK_Int UmntErrNum = 0;
	NK_Int MntErrNum = 0;
	NK_Int UndetectedCnt = 0;

	while (_tfcard->detector_trigger) {

		tmp_detected = _tfcard->EventSet.OnDetectTF();
		if (tmp_detected) {
			if (!detected) {
				gs_tfcard_just_insert = NK_True;
				MntErrNum = 0;
			}
		}
		detected = tmp_detected;

		existed = _tfcard->EventSet.onExistTF();

		_tfcard->TFSlot.mounted = is_path_mounted(_tfcard->TFSlot.fs_path);

		/**
		 * 当前需要格式化TF卡
		 */
		if(existed && (emFormat_Status_start_format == _tfcard->TFSlot.formatStatus)){
			sleep(1);

            // umount
            if (_tfcard->TFSlot.mounted) {
                _tfcard->TFSlot.mounted = NK_False;
                if (0 == _tfcard->EventSet.OnUmountTF(_tfcard->TFSlot.fs_path)) {
                    UmntErrNum = 0;
                 } else {
                    UmntErrNum++;
                }
            } else {
                UmntErrNum = 0;
            }

            // format
            if ((0 == UmntErrNum) || (UmntErrNum > 2)) {
                if(0 == _tfcard->EventSet.OnFormat()){
                    sleep(3);
                    UmntErrNum = 0;
                    count = 0;
                    _tfcard->TFSlot.formatStatus = emFormat_Status_already_format;
                } else {
                    _tfcard->TFSlot.formatStatus = emFormat_Status_none;
                    _tfcard->EventSet.OnAfterFormat(false);
                }
            }
			continue;
		} else if (existed && (!detected)) {
            // 自动格式化插入的没有分区的TF卡. 12 * 0.5s = 6s 之后仍然检测不到分区就格式化
			UndetectedCnt++;
			if (UndetectedCnt >= 12) {
                APP_TRACE("%s: No partition detected, start to format tfcard!", __FUNCTION__);
                _tfcard->TFSlot.formatStatus = emFormat_Status_start_format;
                UndetectedCnt = 0;
            }
        } else {
            UndetectedCnt = 0;
        }

		/**
		 * 当前 TF 卡未连接但是此时检测到 TF 卡存在，有可能用户刚插入 TF 卡。
		 */
		if (detected && !_tfcard->TFSlot.mounted) {
			if (0 == _tfcard->EventSet.OnMountTF(_tfcard->TFSlot.fs_path)) {
				MntErrNum = 0;
				usleep(50000);
				free_size = _tfcard->EventSet.OnGetFreeSpace(_tfcard->TFSlot.fs_path);
				_tfcard->TFSlot.mounted = NK_True;
				if(_tfcard->TFSlot.formatStatus == emFormat_Status_already_format) {
					_tfcard->TFSlot.formatStatus = emFormat_Status_none;
                    _tfcard->EventSet.OnAfterFormat(true);
				}
				NK_TFCARD_check_debug_ini();
			} else {
				MntErrNum++;
				if (MntErrNum > 2) {
					nk_tfcard_play_exception_sound();
					MntErrNum = 0;
				}
			}
		}

		/**
		 * 当前 TF 卡上个周期已经连接但此时检测不到 TF 卡存在。
		 * 有可能 TF 卡已经被强行拆卸。
		 */
		if (!detected && _tfcard->TFSlot.mounted) {
			_tfcard->TFSlot.mounted = NK_False;//首先停止TF卡所有活动
			sleep(1);
			if(0 == _tfcard->EventSet.OnUmountTF(_tfcard->TFSlot.fs_path)) {
				_tfcard->EventSet.OnCleanTF(_tfcard->TFSlot.fs_path);
			}else{
				_tfcard->TFSlot.mounted = NK_True;
			}
		}

		/// 每隔 0.5 秒探测一次 TF 卡插入状态。
		usleep(500000);
	}
}


NK_Int NK_TFCARD_init(LP_TFCARD_FUNCTION init, NK_PChar fs_path, NK_Size max_buffer_size_kb)
{
	int ret = 0;
	if(!_tfcard){
		_tfcard = calloc(sizeof(ST_TFCARD_ATTR), 1);
		_tfcard->EventSet.onExistTF = init->onExistTF;
		_tfcard->EventSet.OnDetectTF= init->OnDetectTF;
		_tfcard->EventSet.OnGetCapacity = init->OnGetCapacity;
		_tfcard->EventSet.OnGetFreeSpace = init->OnGetFreeSpace;
		_tfcard->EventSet.OnMountTF = init->OnMountTF;
		_tfcard->EventSet.OnUmountTF = init->OnUmountTF;
		_tfcard->EventSet.OnCleanTF = init->OnCleanTF;
		_tfcard->EventSet.OnFormat = init->OnFormat;
        _tfcard->EventSet.OnAfterFormat = init->OnAfterFormat;
		_tfcard->max_buffer_size_kb = max_buffer_size_kb;
		snprintf(_tfcard->TFSlot.fs_path, sizeof(_tfcard->TFSlot.fs_path), "%s", fs_path);
		_tfcard->TFSlot.mounted = NK_False;
		_tfcard->TFSlot.formatStatus = emFormat_Status_none;
		_tfcard->detector_trigger = NK_True;
		ret = JA_THREAD_init0(&_tfcard->detector_pid, detector, NULL, NULL, 0, NULL, 131072, 0);
		if(0 != ret){
			NK_Log()->error("thread create failed!");
		}
	}else{
		NK_Log()->warn("module has been inited!");
	}
	return 0;
}

NK_Int NK_TFCARD_destroy()
{

	if(_tfcard){
		_tfcard->detector_trigger = NK_False;
		pthread_join(_tfcard->detector_pid, NULL);
		_tfcard->TFSlot.mounted = NK_False;
		_tfcard->EventSet.OnUmountTF(_tfcard->TFSlot.fs_path);
		free(_tfcard);
	}
	NK_Log()->info("NK tfcard destroy success!");
	_tfcard = NULL;
	return 0;
}

NK_Boolean NK_TFCARD_exist()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL && _tfcard->EventSet.onExistTF != NULL),NK_False);
	return _tfcard->EventSet.onExistTF();
}

NK_Boolean NK_TFCARD_detect()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL && _tfcard->EventSet.OnDetectTF!= NULL),NK_False);
	return _tfcard->EventSet.OnDetectTF();
}

NK_Boolean NK_TFCARD_is_mounted()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL),NK_False);
	return _tfcard->TFSlot.mounted;
}

NK_Size NK_TFCARD_get_capacity()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL && _tfcard->EventSet.OnGetCapacity!= NULL),0);
	return (NK_Size)_tfcard->EventSet.OnGetCapacity(_tfcard->TFSlot.fs_path);
}

NK_Size NK_TFCARD_get_freespace()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL && _tfcard->EventSet.OnGetFreeSpace!= NULL),0);
	return (NK_Size)_tfcard->EventSet.OnGetFreeSpace(_tfcard->TFSlot.fs_path);
}

NK_Int NK_TFCARD_format()
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL && _tfcard->EventSet.OnFormat!= NULL),-1);
	_tfcard->TFSlot.formatStatus = emFormat_Status_start_format;

	return 0;
}

NK_Int NK_TFCARD_record_start(NK_PChar record_type)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((record_type != NULL && _tfcard != NULL),-1);
	return TFcard_Record_init(record_type,_tfcard->EventSet.OnGetFreeSpace, _tfcard->TFSlot.fs_path, _tfcard->max_buffer_size_kb);
}

NK_Int NK_TFCARD_record_write_frame(lpRecord_Frame_Head frameHead,NK_PByte data)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL), -1);
	NK_EXPECT_VERBOSE_RETURN_VAL((frameHead != NULL && data != NULL),-1);
	return TFcard_Record_write_frame(frameHead, data);
}

NK_Int NK_TFCARD_record_stop()
{
	return TFcard_Record_stop();
}

NK_Int NK_TFCARD_play_start(NK_UTC1970 beginUtc,NK_PChar playType)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((_tfcard != NULL),-1);
	return TFCARD_Play_start(beginUtc, playType, _tfcard->TFSlot.fs_path);
}

NK_Int NK_TFCARD_play_read_frame(lpRecord_Frame_Head frameHead, NK_PByte data, NK_Size dataMaxSize)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((frameHead != NULL && data != NULL && dataMaxSize > 0),-1);
	return TFcard_Play_read_frame(frameHead, data, dataMaxSize);
}

NK_Int NK_TFCARD_play_stop()
{
	return TFCARD_Play_stop();
}

NK_Int NK_TFCARD_get_history(NK_UTC1970 beginUtc, NK_UTC1970 endUtc,
							 NK_PChar type,
							 lpTFCARD_History_List historyList,
							 NK_Int startIndex,
							 NK_Int *historyCnt)
{
	NK_EXPECT_VERBOSE_RETURN_VAL((historyList != NULL && _tfcard != NULL),-1);
	return TFCARD_get_history(beginUtc, endUtc, type, historyList, startIndex, historyCnt,_tfcard->TFSlot.fs_path);
}

NK_Int NK_TFCARD_get_status(char *ret_status)
{
	int ret = emTFCARD_STATUS_NO_TFCARD;
	NK_EXPECT_VERBOSE_RETURN_VAL((ret_status != NULL),-1);

	if(NK_TFCARD_exist()){
		if(emFormat_Status_start_format == _tfcard->TFSlot.formatStatus){
			sprintf(ret_status,sTFCARD_STATUS_FORMATTING);
			ret = emTFCARD_STATUS_FORMATTING;
		}else if(NK_TFCARD_detect()){
			if(NK_TFCARD_is_mounted()){
				sprintf(ret_status, sTFCARD_STATUS_OK);
				ret = emTFCARD_STATUS_OK;
			}else if(emFormat_Status_already_format == _tfcard->TFSlot.formatStatus){
				sprintf(ret_status, sTFCARD_STATUS_FORMATED);
				ret = emTFCARD_STATUS_FORMATED;
			}else{
				sprintf(ret_status, sTFCARD_STATUS_EXCEPTION);
				ret = emTFCARD_STATUS_EXCEPTION;
			}
		}else{
			sprintf(ret_status, sTFCARD_STATUS_NOT_FORMAT);
			ret = emTFCARD_STATUS_NOT_FORMAT;
		}
	}else{
		sprintf(ret_status, sTFCARD_STATUS_NO_TFCARD);
		ret = emTFCARD_STATUS_NO_TFCARD;
	}

	return ret;
}

void NK_TFCARD_turn_on_status_check(void)
{
	gs_tfcard_check_turned_on = NK_True;
}
