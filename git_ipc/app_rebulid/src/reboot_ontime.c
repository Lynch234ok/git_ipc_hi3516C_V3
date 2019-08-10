#ifdef REBOOT_ONTIME
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <NkUtils/assert.h>
#include <base/ja_process.h>
#include <sys/reboot.h>
#include "generic.h"
#include "app_debug.h"
#include "reboot_ontime.h"
#include <sys/prctl.h>
#include "base/cross_thread.h"

#define REBOOT_ONTIME_FLAG_FILE_PATH "/media/conf/reboot_ontime_flag"
#define TIME_OF_DAY_FILE_PATH "/media/conf/REBOOT_time"

typedef enum
{
	emWEEK_SUNDAY = 0,
	emWEEK_MONDAY,
	emWEEK_TUESDAY,
	emWEEK_WEDNESDAY,
	emWEEK_THURSDAY,
	emWEEK_FRIDAY,
	emWEEK_STAURDAY
}emWEEK;

typedef struct _REBOOT_ONTIME_T
{
	pthread_t detector_pid;
	NK_Boolean detector_trigger;
	NK_Int reboot_on_sec;
	NK_Int offset_sec;//可偏移秒数,防止其他线程阻塞,本线程到点没有被调度到,错过重启时机
	pthread_t delay_pid;
	NK_Int delay_time;
	fDestroySystem OnDestroy;
	NK_Int reboot_on_week;
}ST_REBOOT_ONTIME_T, *LP_REBOOT_ONTIME_T;

typedef struct _Reboot_TimeOfDay {

	struct timeval tv;
	struct timezone tz;
}ST_Reboot_TimeOfDay, *LP_Reboot_TimeOfDay;

static LP_REBOOT_ONTIME_T _reboot_ontime = NULL;

static inline bool NO_PROMPT(void)
{
	bool ret = false;
	if(IS_FILE_EXIST(REBOOT_ONTIME_FLAG_FILE_PATH)){
		ret = true;
	}
	return ret;
}

static NK_Int SaveTimeOfDay(void)
{
	ST_Reboot_TimeOfDay timeOfDay;
	FILE *fp = NULL;
	fp = fopen(TIME_OF_DAY_FILE_PATH, "w");
	if (NULL == fp) {
		printf("[%s:%s]Open file %s error !\n", __FUNCTION__, __FILE__, TIME_OF_DAY_FILE_PATH);
		return -1;
	}

	gettimeofday(&timeOfDay.tv, &timeOfDay.tz);
	fwrite(&timeOfDay, sizeof(timeOfDay), 1, fp); /* 写的struct文件*/
	fclose(fp); /*关闭文件*/

	return 0;
}

static NK_Int RecoverTimeOfDay(void)
{
	ST_Reboot_TimeOfDay timeOfDay;
	FILE *fp = NULL;
	fp = fopen(TIME_OF_DAY_FILE_PATH, "r");
	if (NULL == fp) {
		printf("[%s:%s]Open file %s error !\n", __FUNCTION__, __FILE__, TIME_OF_DAY_FILE_PATH);
		return -1;
	}

	fread(&timeOfDay, sizeof(timeOfDay), 1, fp);
	timeOfDay.tv.tv_sec -= 10;	/*重启时间大概要40秒*/
	settimeofday(&timeOfDay.tv, &timeOfDay.tz);

	fclose(fp); /*关闭文件*/

	return 0;
}

static NK_Int before_reboot()
{
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "echo 1 > %s", REBOOT_ONTIME_FLAG_FILE_PATH);
	NK_SYSTEM(cmd);
	return 0;
}

static NK_Int after_reboot()
{
	if(IS_FILE_EXIST(REBOOT_ONTIME_FLAG_FILE_PATH)){
		REMOVE_FILE(REBOOT_ONTIME_FLAG_FILE_PATH);
	}
	return 0;
}

static NK_Void detect_time(void)
{	
	prctl(PR_SET_NAME, "reboot_ontime_detect_time");
	while (_reboot_ontime->detector_trigger) {

		NK_UTC1970 utc = 0;
		struct tm setTm = {0};
		NK_Int sec = 0;
		NK_Int sec_min = 0;
		NK_Int sec_max = 0;
		
		utc = time(NULL);
		localtime_r((time_t *)(&utc), &setTm);
		sec = (setTm.tm_hour * 3600 + setTm.tm_min * 60 + setTm.tm_sec);
		if((sec_min = _reboot_ontime->reboot_on_sec - _reboot_ontime->offset_sec)< 0){
			sec_min = 0;
		}
		if((sec_max = _reboot_ontime->reboot_on_sec + _reboot_ontime->offset_sec) > 86400){
			sec_min = 86400;
		}

		if((_reboot_ontime->reboot_on_week == setTm.tm_wday) && (sec >= sec_min) && (sec <= sec_max) && !NK_REBOOT_ONTIME_is_flag_exist()){
			NK_Log()->info("------------------------------------\n\
reboot module it is time to reboot : reboot_on_sec = %d, offset_sec = %d\nsec = %d, sec_min = %d, sec_max = %d\n\
------------------------------------\n",_reboot_ontime->reboot_on_sec, _reboot_ontime->offset_sec, sec, sec_min, sec_max);
			if(_reboot_ontime->OnDestroy){
				_reboot_ontime->OnDestroy();
			}
			NK_Log()->info("end of OnDestroy");
			before_reboot();
			SaveTimeOfDay();
			usleep(100000);
			sync();
			//exit(0);
            GLOBAL_reboot_system();
		}

		//每隔 1 秒检测一次当前时间
		sleep(1);
	}
}

static NK_Void* delay_clean()
{
	prctl(PR_SET_NAME, "reboot_ontime_delay_clean");
	pthread_detach(pthread_self());
	//RecoverTimeOfDay();
	while((_reboot_ontime) && (_reboot_ontime->delay_time > 0)){
		--_reboot_ontime->delay_time;
		//APP_TRACE("------delay time : %d",_reboot_ontime->delay_time);
		sleep(1);
	}
	after_reboot();
	_reboot_ontime->delay_pid = (pthread_t)NULL;
	pthread_exit(NULL);
}

NK_Boolean NK_REBOOT_ONTIME_is_flag_exist()
{
	return NO_PROMPT();
}

static NK_Int reboot_rand_time(NK_Int reboot_time)
{
	struct timespec timetic;
	NK_Int time;
	clock_gettime(CLOCK_MONOTONIC, &timetic);
	srand((unsigned) timetic.tv_nsec);
	time = reboot_time + rand()%3600;
	return time;
}

NK_Int NK_REBOOT_ONTIME_init(NK_Int hourNum, fDestroySystem OnDestroy)
{
	NK_Int ret = -1;
    int restart_h = 0;
    int restart_m = 0;
    int restart_s = 0;

	if(!_reboot_ontime){
		_reboot_ontime = calloc(sizeof(ST_REBOOT_ONTIME_T), 1);
		NK_Int reboot_time;
		if((hourNum >= 0) && (hourNum < 24)){			
			reboot_time = hourNum * 3600;
		}else{
			reboot_time = 2 * 3600;
		}
		_reboot_ontime->reboot_on_sec = reboot_rand_time(reboot_time);
		_reboot_ontime->offset_sec = 5;
		_reboot_ontime->detector_trigger = NK_True;
		_reboot_ontime->OnDestroy = OnDestroy;
		_reboot_ontime->reboot_on_week = emWEEK_TUESDAY;
		ret = JA_THREAD_init0(&_reboot_ontime->detector_pid, detect_time, NULL, NULL, 0, NULL, 131072, 0);
		if(0 != ret){
			APP_TRACE("reboot module detect_time thread create failed!");
			free(_reboot_ontime);
			_reboot_ontime = NULL;
			return -1;
		}
        restart_h = _reboot_ontime->reboot_on_sec / 3600;
        restart_m = _reboot_ontime->reboot_on_sec % 3600 / 60;
        restart_s = _reboot_ontime->reboot_on_sec % 3600 % 60;
        printf("============reboot time==========\n");
        printf("week : %d\ntime : %d:%d:%d\n", _reboot_ontime->reboot_on_week, restart_h, restart_m, restart_s);
        printf("=================================\n");
		if(NO_PROMPT()){
			_reboot_ontime->delay_time = 3600;
			JA_THREAD_init0(&_reboot_ontime->delay_pid, delay_clean, NULL, NULL, 0, NULL, 131072, 0);
		}else{
			_reboot_ontime->delay_time = 0;
			_reboot_ontime->delay_pid = NULL;
		}

		APP_TRACE("reboot module init success!");
	}else{
		if((hourNum >= 0) && (hourNum < 24)){
			_reboot_ontime->reboot_on_sec = hourNum * 3600;
		}else{
			_reboot_ontime->reboot_on_sec = 2 * 3600;
		}
		APP_TRACE("reboot module has inited, change hourNum to %d!",hourNum);
	}
	return 0;
}

NK_Int NK_REBOOT_ONTIME_destroy()
{

	if(_reboot_ontime){
		_reboot_ontime->detector_trigger = NK_False;
		pthread_join(_reboot_ontime->detector_pid, NULL);
		if(_reboot_ontime->delay_pid){
			_reboot_ontime->delay_time = -1;
			while(_reboot_ontime->delay_pid){
				usleep(10000);
			}
		}
		after_reboot();
		free(_reboot_ontime);
	}
	APP_TRACE("reboot module destroy success!");
	_reboot_ontime = NULL;
	return 0;
}
#endif
