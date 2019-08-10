#include <sys/reboot.h>
#include "remote_upgrade.h"
#include "custom.h"
#include "version.h"
#include "generic.h"
#include "app_debug.h"
#include "tfcard.h"
#include "firmware.h"
#include "sound.h"
#include "bsp/watchdog.h"
#include "global_runtime.h"
#include "wpa_supplicant/include/wpa_status.h"
#include "app_cloud_rec.h"
#include "base/ja_process.h"

#define REMOTE_UPGRADE_FILE_DIR "/tmp/upgrade"
#define REMOTE_UPGRADE_RATE "/tmp/upgrade/rate"
#define REMOTE_UPGRADE_FIRMWARE_FILE_DIR "/tmp/firmware.rom"

static int status_trap(int status)
{
	switch(status){
		case OTA_STATUS_START:
			SearchFileAndPlay(SOUND_Upgrade, NK_True);
			sleep(1);
			break;
		case OTA_STATUS_SUCCESS:
			//SearchFileAndPlay(SOUND_Upgrade_completed, NK_True);
			GLOBAL_reboot_system();
			break;
		case OTA_STATUS_FAILED:
			//WPA_resetWifiConnectedFlag();
			//SearchFileAndPlay(SOUND_Firmware_update_failed, NK_True);
			REMOVE_FILE(REMOTE_UPGRADE_RESTART_FLAG_PATH);
			REMOVE_FILE(REMOTE_UPGRADE_FIRMWARE_FILE_DIR);
			//sleep(3);
			GLOBAL_reboot_system();
			break;
		default:
			APP_TRACE("Unknow status type!");
			break;
	}
	return 0;
}

static int ota_firmware_write(char *filename, void *pUpdataMem, int dataSize)
{
#if 0
	mkdir("/tmp/upgrade/dev", 0);
	TOUCH_FILE(REMOTE_UPGRADE_RATE);
	FIRMWARE_upgrade_parse_rom_from_file(filename, REMOTE_UPGRADE_FILE_DIR);
#else
    if(NULL == pUpdataMem || dataSize <= 0)
    {
        return -1;
    }

    FIRMWARE_upgrade_start2(true, true, 1, pUpdataMem, dataSize);
#endif
	return 0;
}

static int ota_firmware_get_rate()
{
	FILE* fp = fopen(REMOTE_UPGRADE_RATE, "r");
	int ret =0, rate_tmp = 0;
	char buf[64];
	if(fp){
		ret = fread(buf, 1, sizeof(buf), fp);
		if(ret>0){
			rate_tmp = atoi(buf);
		}
		fclose(fp);
	}
	return rate_tmp;
}
static int remote_upgrade_check_reboot(bool reboot)
{
	char cmd[128];
	if(reboot){
		snprintf(cmd, sizeof(cmd), "echo need_reboot > %s ", REMOTE_UPGRADE_RESTART_FLAG_PATH);
		NK_SYSTEM(cmd);
		WATCHDOG_disable();
		sleep(2);
		exit(0);
	}else{
		//has been restart app for upgrade
#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
        NK_TFRECORD_Stop(0);
#else
		#if defined(TFCARD)
			TFCARD_destroy();
		#endif
#endif
		#if defined(CLOUD_REC)
			NK_CLOUD_REC_Deinit();
		#endif
		REMOVE_FILE(REMOTE_UPGRADE_RESTART_FLAG_PATH);
	}
	return 0;
}



static bool app_ota_init()
{
	ST_REMOTE_UPGRADE_DEVINFO info;
	FILE *fd = NULL;
	bool need_reboot = true;
	char file_data[16], sn[32];
	ST_CUSTOM_SETTING custom;

	memset(file_data, 0, sizeof(file_data));
	memset(&info, 0, sizeof(ST_REMOTE_UPGRADE_DEVINFO));
    memset(sn, 0, sizeof(sn));

	if(0 == CUSTOM_get(&custom) && CUSTOM_check_string_valid(custom.model.oemNumber)){
		strcpy(info.OEMnum, custom.model.oemNumber);
	}else{
		strcpy(info.OEMnum, PRODUCT_MODEL);
	}

	strcpy(info.model, "IPCAM");
	sprintf(info.FWversion, "%d.%d.%d.0", SWVER_MAJ, SWVER_MIN, SWVER_REV);
	strcpy(info.FWMagic, "JUAN IPCAM FIRMWARE DESIGNED BY LAW");
	strcpy(info.firmware_path, REMOTE_UPGRADE_FIRMWARE_FILE_DIR);
    UC_SNumberGet(sn);
    strcpy(info.deviceSN, sn);
	if(IS_FILE_EXIST(REMOTE_UPGRADE_RESTART_FLAG_PATH)){
		need_reboot = false;
	}else{
		need_reboot = true;
	}
	info.function.ota_status_trap = status_trap;
	info.function.flash_sync  = ota_firmware_write;
	info.function.get_rate = ota_firmware_get_rate;
	info.function.env_prepare = remote_upgrade_check_reboot;
	REMOTE_UPGRADE_init(need_reboot, &info);

	return need_reboot;
}


bool IPCAM_network_init_for_upgrade()
{
	bool ret = true;
	ret =  app_ota_init();

	if(!ret){
		//start thread for remote upgrade
		FIRMWARE_init(getenv("FLASHMAP"), NULL);
		ST_CUSTOM_SETTING custom;
		if(0 == CUSTOM_get(&custom) && CUSTOM_check_string_valid(custom.model.oemNumber)){
			FIRMWARE_upgrade_set_oem_number(custom.model.oemNumber);
		}
		REMOTE_UPGRADE_start();
	}else{
		//run as default
	}
	return ret;
}
