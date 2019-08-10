#include "app_debug.h"
#include "sound.h"
#include "app_tfcard.h"
#include "bsp/keytime.h"
#include "bsp/watchdog.h"
#include "ticker.h"
#include "app_overlay.h"
#include "wifi/ja_wifi_seek.h"
#include "wpa_supplicant/include/wpa_status.h"
#include "firmware.h"
#include "version.h"
#include <sys/prctl.h>
#include "base/ja_process.h"
#include "generic.h"
#include "global_runtime.h"

/*
* @brief  检查固件版本号，是否在升级允许范围内。
*         可以通过参数downgrade标志进行强制降级
* @param firmware_file  升级固件绝对路径文件名
* @param downgrade      强制降级标志
* @return 成功  0
*         失败    -1
*/
static int app_upgrade_check_version(const char *firmware_file, bool downgrade)
{
    int ret = 0;
    FwHeader_t *header = NULL;
    FILE *fp = NULL;

    fp = fopen(firmware_file, "r+");
    if(fp != NULL) {

        header = calloc(sizeof(FwHeader_t), 1);
        if(NULL == header)
        {
            fclose(fp);
            return -1;
        }

        ret = fread(header, 1, sizeof(FwHeader_t), fp);
        fclose(fp);
        fp = NULL;
        if(ret == sizeof(FwHeader_t)) {
            APP_TRACE("upgrade (%d.%d.%d.%s => %d.%d.%d.%s)", SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT, header->version.major, header->version.minor, header->version.revision, header->version.extend);
            if(header->version.major != SWVER_MAJ || header->version.minor != SWVER_MIN || header->version.revision != SWVER_REV || strcmp(header->version.extend, SWVER_EXT) != 0) {
                if(FIRMWARE_upgrade_import_check(false, firmware_file)) {

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }

                    return 0;
                }
                else {
                    APP_TRACE("check file failed!");
                }
            }
        }
        else {
            APP_TRACE("read header failed!(%d != %d)", ret, sizeof(FwHeader_t));
        }
    }

    if(NULL != header)
    {
        free(header);
        header = NULL;
    }

    return -1;
}

/*
* @brief  升级前，释放资源
*
*/
static void app_upgrade_before()
{
    APP_TRACE("start upgrade!");
    SearchFileAndPlay(SOUND_Upgrade, NK_True);

    GLOBAL_cpuDetectDestroy();

#if defined(WIFI)
    NK_WIFI_adapter_monitor_thread_stop();
#if defined(DANA_P2P)
    APP_WIFI_Wifi_Exit();
#endif
#endif

#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
    NK_TFRECORD_Stop(0);
#else
#if defined(TFCARD)
    TFCARD_stop_record();
#endif
#endif
    KeyTime_destroy();
    WATCHDOG_disable();
    TICKER_destroy();
    APP_OVERLAY_destroy();

}

/*
* @brief  升级后的处理 播报语音，更改升级固件名字，进行重启
* @param upgrade_status  升级返回的状态
* @param firmware_file   升级固件绝对路径文件名
*/
static void app_upgrade_after(int upgrade_status, const char *firmware_file)
{
    char text[128];
    if(FIRMWARE_UPGRADE_ERROR_NONE == upgrade_status) {
    APP_TRACE("UPGRADE errno:%d", upgrade_status);
        //SearchFileAndPlay(SOUND_Upgrade_completed, NK_True);
    }
    else {
        SearchFileAndPlay(SOUND_Firmware_update_failed, NK_True);
    }

    if(!IS_FILE_EXIST(FIRMWARE_NOT_MV_FILE)) {
        sprintf(text, "mv %s %s.used", firmware_file, firmware_file);
        NK_SYSTEM(text);
        sleep(5);
    }

    GLOBAL_reboot_system();

}

/*
* @brief  固件升级
* @param firmware_file  升级固件绝对路径文件名
* @param downgrade      强制降级标志
* @return 	升级的状态
*           FIRMWARE_UPGRADE_ERROR_NONE,
*	        FIRMWARE_UPGRADE_ERROR_MISMATCH_SOC,
*	        FIRMWARE_UPGRADE_ERROR_MISMATCH_MD5
*/
static int app_upgrade_firmware(const char *firmware_file, bool downgrade)
{
    int rate = 0, err_num = 0, ret;

    FIRMWARE_import_release_memery();
    ret = FIRMWARE_upgrade_start(false, downgrade, firmware_file);
    if(ret < 0) {
        APP_TRACE("start FIRMWARE_upgrade thread failed, exit");
        FIRMWARE_import_recover_memery();
        initLedContrl(DEF_LED_ID,true,LED_DARK_MODE);
        exit(0);
    }
    do {
        err_num = FIRMWARE_upgrade_get_errno();
        rate = FIRMWARE_upgrade_get_rate();
        APP_TRACE("upgrade_rate = %d", rate);
        sleep(1);
    }while(100 > rate && FIRMWARE_UPGRADE_ERROR_NONE == err_num);
    FIRMWARE_import_recover_memery();

    return err_num;

}

/*
* @brief  固件升级处理, 目前处理逻辑是假如开始了升级进程，不管最终是否升级成功，都会重启
* @param firmware_file  升级固件绝对路径文件名
* @param downgrade      强制降级标志
* @return 成功  0
*         失败    -1
*/
static int app_upgrade(const char *firmware_file, bool downgrade)
{
	int ret = 0, upgrade_ret = 0;
    ret = app_upgrade_check_version(firmware_file, downgrade);  // 先判断版本
    if(ret == 0) {
        app_upgrade_before();

        upgrade_ret = app_upgrade_firmware(firmware_file, downgrade);

        app_upgrade_after(upgrade_ret, firmware_file);
    }
    else {
        APP_TRACE("upgrade failed");
        return -1;
    }

    return 0;

}

/*
* @brief  固件升级处理, 目前处理逻辑是假如开始了升级进程，不管最终是否升级成功，都会重启
* @param firmware_file  升级固件绝对路径文件名
* @param downgrade      强制降级标志
* @return 成功  0
*         失败    -1
*/
int APP_UPGRADE_start(const char *firmware_file, bool downgrade)
{
    return app_upgrade(firmware_file, downgrade);

}

