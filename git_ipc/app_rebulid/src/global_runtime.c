
#include "global_runtime.h"
#include <sys/sysinfo.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <stdlib.h>
#include "generic.h"
#include "crc.h"
#include <string.h>
#include <unistd.h>
#include <bsp/keytime.h>
#include "mutex_f.h"
#include "securedat.h"
#include <sys/prctl.h>
#include "base/ja_process.h"
#include "app_cloud_rec.h"
#include "app_motor.h"

#define RECOVER_SYSTIME_FILE_PATH "/media/conf/recover_systime"
#define RECOVER_SYSTIME_FILE_TMP_PATH "/media/conf/recover_systime_tmp"

char g_esee_id[32] = {""};
bool g_authorized = false;
uint32_t g_hardware_version = 0x00010000;
int g_encryp_chip_type = 0;
uint32_t g_encryp_chip_odm1 = 0;
uint32_t g_encryp_chip_odm2 = 0;

LP_MUTEX g_event_mutex_lock = NULL;
int g_on_call_num = 0;
int g_on_pb_num = 0;
int g_sensor_type;

// lock and counter for live
static pthread_mutex_t gs_live_lock = PTHREAD_MUTEX_INITIALIZER;
static int gs_live_cnt = 0;

static bool oldTypeRecord = false;

int GLOBAL_event_lock_init()
{
	 g_event_mutex_lock = MUTEX_create();
	 return g_event_mutex_lock ? 0 : -1;
}

int GLOBAL_event_lock_deinit()
{
	if(g_event_mutex_lock){
		MUTEX_release(g_event_mutex_lock);
	}
	return 0;
}

int GLOBAL_event_lock()
{

	if(g_event_mutex_lock){
		return g_event_mutex_lock->lock(g_event_mutex_lock) == 0;
	}
	return 0;
}

int GLOBAL_event_unlock()
{
	if(g_event_mutex_lock){
		return g_event_mutex_lock->unlock(g_event_mutex_lock) == 0;
	}
	return 0;
}

int GLOBAL_enter_twowaytalk()
{
	int ret = -1;
	if(GLOBAL_event_lock()){
		if(g_on_call_num < G_NK_MAX_TWOWAYTALK_NUM){
			g_on_call_num ++;
			ret = 0;
		}
		GLOBAL_event_unlock();
		return ret;
	}
	return ret;
}

int GLOBAL_leave_twowaytalk()
{
	int ret = -1;
	if(GLOBAL_event_lock()){
		if(g_on_call_num > 0){
			g_on_call_num --;
			ret = 0;
		}
		GLOBAL_event_unlock();

		return ret;
	}
	return ret;
}

int GLOBAL_enter_playback()
{
	int ret = -1;
	if(GLOBAL_event_lock()){
#if defined(HI3516E_V1)
        if(g_on_pb_num < 1)
        {
#else
		if(g_on_pb_num < G_NK_MAX_PLAYBACK_NUM){
#endif
			g_on_pb_num ++;
			ret = 0;
		}
		GLOBAL_event_unlock();
		return ret;
	}
	return ret;
}

int GLOBAL_leave_playback()
{
	int ret = -1;
	if(GLOBAL_event_lock()){
		if(g_on_pb_num > 0){
			g_on_pb_num --;
			ret = 0;
		}
		GLOBAL_event_unlock();

		return ret;
	}
	return ret;
}

void GLOBAL_remove_conf_file()
{
	NK_SYSTEM("rm -rf /media/conf/netsdk");
	NK_SYSTEM("rm -rf /media/conf/main_resolution");
	NK_SYSTEM("rm -rf /media/conf/multi_conf_mode");
}

uint32_t GLOBAL_get_sys_mem_mb()
{
	struct sysinfo info = {0};
	uint32_t totalRam = 0;

	if(0 != sysinfo(&info)){
		totalRam = 40;
	}

	totalRam = (uint32_t)(info.totalram/1024/1024);//得到系统总内存(单位:M)

	printf("----system memery size:%dMbytes----\n", totalRam);

	return totalRam;
}

uint32_t GLOBAL_reboot_system()
{
#if defined(WIFI)
	APP_WIFI_exit_wifi();
#endif
#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
    NK_TFRECORD_Stop(0);
    NK_TFCARD_Exit();
#else
#if defined(TFCARD)
	TFCARD_destroy();
#endif
#endif
	reboot(RB_AUTOBOOT);
}

/*
功能:从一个文件中读取struct timeval格式时间,并且做8位crc校验，然后设置系统时间
成功返回0，失败返回-1
*/
int global_setSystime(const char *file)
{
	FILE *fp = NULL;
	char readData[8];
	unsigned char crc1, crc2;
	int ret = 0;
    struct timeval tv;

	fp = fopen(file, "r");  // FIXME此处没有加锁操作
	if (NULL == fp) {
		printf("[%s:%s]Open file %s error !\n", __FUNCTION__, __FILE__, file);
		return -1;
	}

	fread(readData, 1, sizeof(readData), fp);
	fread(&tv, 1, sizeof(struct timeval), fp);
    fclose(fp);
	crc1 = atoi(readData);
	crc2 = CRC_getByteCRC((char *)&tv, sizeof(struct timeval));
	if(crc1 == crc2) {
		settimeofday(&tv, NULL);
		printf("%s recover system time %ld\n", __FUNCTION__, tv.tv_sec);
		ret = 0;
	}
	else {
		printf("%s recover system time failed\n", __FUNCTION__);
		ret = -1;
	}

	return ret;

}

int32_t GLOBAL_setTimeFromFile(void)
{
	int ret = 0;

	if((ret = global_setSystime(RECOVER_SYSTIME_FILE_PATH)) < 0) {
		ret = global_setSystime(RECOVER_SYSTIME_FILE_TMP_PATH);
	}

	return ret;

}

/*
功能:保存struct timeval格式的时间到文件中
*/
int32_t GLOBAL_saveFileFromTime(void)
{
	unsigned char crc;
	char crcStr[8];
    struct timeval tv;
	FILE *fp = NULL;


	fp = fopen(RECOVER_SYSTIME_FILE_TMP_PATH, "w");
	if (NULL == fp) {
		printf("[%s:%s]Open file %s error !\n", __FUNCTION__, __FILE__, RECOVER_SYSTIME_FILE_TMP_PATH);
		return -1;
	}

	gettimeofday(&tv, NULL);
	crc = CRC_getByteCRC((char *)&tv, sizeof(struct timeval));
	sprintf(crcStr, "%d\n", crc);
	fwrite(crcStr, 1, sizeof(crcStr), fp);
	fwrite(&tv, 1, sizeof(struct timeval), fp); /* 写的struct文件 */
	fsync(fp);
	fclose(fp);

	COPY_FILE(RECOVER_SYSTIME_FILE_TMP_PATH, RECOVER_SYSTIME_FILE_PATH);

	return 0;
}

void GLOBAL_exit()
{
    GLOBAL_event_lock_deinit();
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

#if defined(WIFI)
    if(!IS_FILE_EXIST(REMOTE_UPGRADE_RESTART_FLAG_PATH)){
	    APP_WIFI_exit_wifi();
    }
#endif

}

/*
	标记是否存在旧格式录像文件
	主要用于程序启动时标记，后面直接查询GLOBAL_isOldTypeRecord接口判断是否有旧格式录像
*/
void GLOBAL_setOldTypeRecordFlag()
{
	if(CHECK_DIR_EXIST(TFCARD_OLD_RECORD_PATH)) {
		oldTypeRecord = true;
	}
	else {
		oldTypeRecord = false;
	}

}

/*
	存在旧格式录像返回true，false没有旧格式录像
*/
bool GLOBAL_isOldTypeRecord()
{
	return oldTypeRecord;

}

bool GLOBAL_sn_front()
{
    char sn[32];
    char sn_code[64];
    memset(sn, 0, sizeof(sn));
    if(0 == UC_SNumberGet(sn)) {
        sprintf(sn_code, "%s", sn);

        if(!strncmp(sn_code,"F",1)) {
            return true;
        }
        else {
            return false;
        }
    }

    return true;
}

int GLOBAL_enter_live()
{
	int ret;

	ret = pthread_mutex_lock(&gs_live_lock);
	if (0 != ret) {
		printf("%s:%d  pthread_mutex_lock failed!! ret: %d\n",
			   __FUNCTION__, __LINE__,
			   ret);
		return -1;
	}


	gs_live_cnt++;

	// turn on private led
	KEY_LED_set_mode2(LED_PRIV_ID, true, LED_LIGHT_MODE);


	ret = pthread_mutex_unlock(&gs_live_lock);
	if (0 != ret) {
		printf("%s:%d  pthread_mutex_unlock failed!! ret: %d\n",
			   __FUNCTION__, __LINE__,
			   ret);
	}

	return 0;
}

int GLOBAL_leave_live()
{
	int ret;

	ret = pthread_mutex_lock(&gs_live_lock);
	if (0 != ret) {
		printf("%s:%d  pthread_mutex_lock failed!! ret: %d\n",
			   __FUNCTION__, __LINE__,
			   ret);
		return -1;
	}


	if (gs_live_cnt > 0) {
		gs_live_cnt--;
	}

	if (gs_live_cnt <= 0) {
		// turn off private led
		KEY_LED_set_mode2(LED_PRIV_ID, false, LED_LIGHT_MODE);
	}


	ret = pthread_mutex_unlock(&gs_live_lock);
	if (0 != ret) {
		printf("%s:%d  pthread_mutex_unlock failed!! ret: %d\n",
			   __FUNCTION__, __LINE__,
			   ret);
	}

	return 0;
}

static bool cpuDetectorTrigger;
void *global_cpuDetector(void *arg)
{
#if defined(CPU_DETECT)
    unsigned int interUs = 3000 * 1000;     // 3绉掓娴嬩竴娆pu
    float sysCcpuUsage = 0.0;
    float sysCpuUsageLimit = 90;
    unsigned int maxCount = 10;     // 30绉掞紝鏍规嵁缁熻闂撮殧寰楀嚭
    unsigned int count = 0;

    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "cpuDetector");

    while(true == cpuDetectorTrigger)
    {
        sysCcpuUsage = sys_cpuUsage();

        if(sysCcpuUsage >= sysCpuUsageLimit)
        {
            count++;
            printf("current cpu usage %.2f count %d\n", sysCcpuUsage, count);
            if(maxCount <= count)
            {
                printf("cpu detector reboot system!!!\n");
                GLOBAL_reboot_system();
            }
        }
        else
        {
            count = 0;
        }

        usleep(interUs);

    }
#endif

}

void GLOBAL_cpuDetectInit()
{
#if defined(CPU_DETECT)

    pthread_t cpuDetectorTid;
    pthread_attr_t pthread_attr;

    int nRet;

    nRet = pthread_attr_init(&pthread_attr);
    if(0 == nRet)
    {
        pthread_attr_setstacksize(&pthread_attr, 65536);
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);

        cpuDetectorTrigger = true;
        pthread_create(&cpuDetectorTid, &pthread_attr, global_cpuDetector, NULL);

        pthread_attr_destroy(&pthread_attr);
    }
#endif

}

void GLOBAL_cpuDetectDestroy()
{
#if defined(CPU_DETECT)
    cpuDetectorTrigger = false;
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
#endif
}

void GLOBAL_IOTdaemonExit()
{
    /**
     * 鎿﹀啓flash鍓峩ill鎺塈OTdaemon
     */
    NK_SYSTEM("killall daemon_start.sh");
    NK_SYSTEM("killall iot.Daemon");

}


int GLOBAL_GetID(char * ID_String)
{
	char tmpID[32] = {0};
	char tmpSN[32] = {0};
	
	if(NULL == ID_String) return -1;

	if(0 == UC_SNumberGet(tmpSN)) {
		int i,index,temp;

		for(i=4; i<14; i++){
			if(tmpSN[i] != '0'){
				break;
			}
		}

		index = 0;
		for(; i<14; i++){
			tmpID[index] = tmpSN[i];
			index++;
		}
		tmpID[index] = '\0';

		strcpy(ID_String, tmpID);
		return 0;
	}

	return -1;
}

int GLOBAL_before_upgrade_destroy()
{
    char cmd[64] = {0};

    GLOBAL_cpuDetectDestroy();

#if defined(MSG)
    ESEE_msg_destroy();
#endif

    IPCAM_timer_destroy();

#if defined(WIFI)
	NK_WIFI_adapter_monitor_thread_stop();
#endif

#if defined(UART_PROTOCOL)
	UART_protocol_ptz_Destroy();
    APP_MOTOR_Exit();
#endif

#if defined(TS_RECORD)
    NK_TFRECORD_Stop(0);
#else
#if defined(TFCARD)
	TFCARD_stop_record();
#endif
#endif

#if defined(CLOUD_REC)
	NK_CLOUD_REC_Deinit();
#endif

#if defined(P2P)
    //GLOBAL_IOTdaemonExit();
    //P2P_sdkdestroy();
    snprintf(cmd, sizeof(cmd), "%s/shell/IOTDaemon -s disable_reboot", IPCAM_ENV_HOME_DIR);
    NK_SYSTEM(cmd);
#endif

    SOUND_releaseQueue();
	KeyTime_destroy();//stop key time
#ifdef LED_CTRL
	KEY_LED_ledDestroy();
#endif
	//WATCHDOG_disable();
	APP_OVERLAY_destroy();
	//TICKER_destroy();

    REBOOT_ONTIME_destroy();

}

