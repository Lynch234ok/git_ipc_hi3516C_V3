#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> 
#include <mpi_sys.h>
#include <inifile.h>
#include "bsp/keytime.h"
#include "../sound.h"
#include <sys/prctl.h>
#include "base/cross_thread.h"

#define ON 			(0)
#define RELEASE		true
#define PRESS		false

static stArg_Thread arg_Thread;
static pthread_t key_detection_pid = NULL;
static bool key_time_trigger = false;

static pthread_t ledDetectionPid = (pthread_t)NULL;
static bool ledTrigger = false;

static void* DetecPressTime(void *arg)
{
    static bool Release_Press = RELEASE;
	time_t deltaTime;
	struct timeval tBegin, tEnd;
    int reg_val[KEY_MAX_NUM];
    int reg_val_backup[KEY_MAX_NUM];
    memset(reg_val, 1, sizeof(reg_val));
    memset(reg_val_backup, 1, sizeof(reg_val_backup));
	pthread_detach(pthread_self() );
	gettimeofday(&tEnd, NULL);
	gettimeofday(&tBegin, NULL);
	prctl(PR_SET_NAME, "KeyDetecPressTime");
    while (key_time_trigger) {
		
		BSP_Get_Key_Val(reg_val);
		//下降沿检测
		/* key 1 */
        if(reg_val[0] != BSP_KEY_NULL) {
            //每释放一次才能有效
            if (Release_Press == RELEASE) {
        		if (reg_val_backup[0] == ON && reg_val[0] != ON) {
                    deltaTime = ((tEnd.tv_sec * 1000000 + tEnd.tv_usec) - (tBegin.tv_sec * 1000000 + tBegin.tv_usec)) / 1000000;
        			if (deltaTime < MINI_TIME) {
        				if (arg_Thread.funZero != NULL){
        					arg_Thread.funZero();
        				}
        			}else if (deltaTime >= TIME_3S) {
        				if(arg_Thread.funOne != NULL){
        					arg_Thread.funOne();
        				}
        			}
					/*else if (deltaTime >= MID_TIME && deltaTime < MAX_TIME) {
        				if(arg_Thread.funTwo != NULL){
        					arg_Thread.funTwo();
        				}
        			}else if (deltaTime >= MAX_TIME) {
        				if(arg_Thread.funThree != NULL){
        					arg_Thread.funThree();
        				}
        			}*/
        		}else if (reg_val_backup[0] == ON && reg_val[0] == ON) {//常按检测

                    deltaTime = ((tEnd.tv_sec * 1000000 + tEnd.tv_usec) - (tBegin.tv_sec * 1000000 + tBegin.tv_usec)) / 1000000;
                    if (deltaTime >= TIME_3S) {
                        if(arg_Thread.funOne != NULL){
                            arg_Thread.funOne();
                        }
                        Release_Press = PRESS;
                    }		
			    }
            }
    		if (reg_val[0] == ON) {

    			gettimeofday(&tEnd, NULL);
    		}else {

                Release_Press = RELEASE;
    			gettimeofday(&tBegin, NULL);
    		}
    		reg_val_backup[0] = reg_val[0];
        }

        /* key 2 */
        if(reg_val[1] != BSP_KEY_NULL) {
            if (reg_val_backup[1] == ON && reg_val[1] != ON) {
                if (arg_Thread.funFour!= NULL) {
                    arg_Thread.funFour();
                }
            }
            reg_val_backup[1] = reg_val[1];
        }
		usleep(100000);
	}

	return NULL;
}

int initKeyTime(funBackCall callBackZero
	, funBackCall callBackOne
	, funBackCall callBackTwo
	, funBackCall callBackThree
	, funBackCall callBackFour)
{
	if(key_detection_pid){
		printf("%s-%d: key detection thread has been inited!\n", __FUNCTION__, __LINE__);
	}
	memset(&arg_Thread, 0, sizeof(arg_Thread));
	int reval = 0;
	/*if (!callBackOne || !callBackTwo) {
		printf("Error!! The callback fun can not be NULL : %s , %s !!!\n",__FILE__,  __func__);
		return -1;
	}*/

	arg_Thread.funZero = callBackZero;
	arg_Thread.funOne = callBackOne;
	arg_Thread.funTwo = callBackTwo;
	arg_Thread.funThree = callBackThree;
    arg_Thread.funFour = callBackFour;
    key_time_trigger = true;

	reval = JA_THREAD_init0(&key_detection_pid, DetecPressTime, (void *)&arg_Thread, NULL, 0, NULL, 131072, 0);
	if (reval != 0) {
		
		 printf("%s: %s\n",__func__, strerror(reval));
		 return -1;
	}

	return 0;
}

static pthread_t getKeyStatusTid = (pthread_t)NULL;
static bool key_start_trigger = false;

/* 按键状态检测，当检测的按键全都有被按下状态的记录时，线程结束或者20秒线程结束 */
static void *key_getKeyStatus(void *arg)
{
    int i = 0;
    int loopCount = 1500;
    lpKeyInfo keyInfo = (lpKeyInfo)arg;
    int keyValTmp[KEY_MAX_NUM];
    int keyNum = 0;
    int numTmp = 0;
    memset(keyValTmp, BSP_KEY_RELEASE, sizeof(keyValTmp));
	prctl(PR_SET_NAME, "key_getKeyStatus");

    /* 先确定真实按键数，因为不存在的按键会返回-1 */
    BSP_Get_Key_Val(keyValTmp);
    for(i = 0; i < KEY_MAX_NUM; i++) {
        if((keyValTmp[i] == BSP_KEY_PRESS) 
            || (keyValTmp[i] == BSP_KEY_RELEASE)) {
            keyInfo->keyVal[i] = BSP_KEY_RELEASE;
            keyNum++;
        }
    }
    keyInfo->keyNum = keyNum;

    key_start_trigger = true;

    while(key_start_trigger) {

        BSP_Get_Key_Val(keyValTmp);
        for(i = 0; i < keyNum; i++) {
            if(keyValTmp[i] == BSP_KEY_PRESS) {
                keyInfo->keyVal[i] = BSP_KEY_PRESS;
            }
        }

        /* 检查keyNum个按键是否已被按下 */
        numTmp = 0;
        for(i = 0; i < keyNum; i++) {
            if(keyInfo->keyVal[i] == BSP_KEY_PRESS) {
                numTmp++;
            }
        }
        if(numTmp == keyNum) {
            break;
        }

        usleep(1000 * 10);
        loopCount--;
        if(loopCount <= 0) {
            key_start_trigger = false;
        }
    }
    getKeyStatusTid = (pthread_t)NULL;
    pthread_exit(NULL);
}

int KeyTime_destroy()
{
	if(key_detection_pid){	
    	printf("%s!\n",__func__);
		key_time_trigger  = false;
		pthread_join(key_detection_pid, NULL);
		key_detection_pid = (pthread_t)NULL;
		printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
		return 0;
	}
	return -1;
}

void KEY_startGetKeyAttr(lpKeyInfo KeyInfo)
{
    int i = 0;
    if(!getKeyStatusTid) {

        /* 初始化 */
        KeyInfo->keyNum = KEY_MAX_NUM;
        for(i = 0; i < KeyInfo->keyNum; i++) {
            KeyInfo->keyVal[i] = BSP_KEY_NULL;
        }

		JA_THREAD_init0(&getKeyStatusTid, key_getKeyStatus, (void *)KeyInfo, NULL, 0, NULL, 131072, 0);

    }
}

void KEY_stopGetKeyAttr()
{
    if(getKeyStatusTid != NULL) {
        key_start_trigger = false;
        pthread_join(getKeyStatusTid, NULL);
        getKeyStatusTid = (pthread_t)NULL;
    }

}

#ifdef LED_CTRL
/*
static int s_LedDynaMode = 0;

static void* LedLight(void *arg)
{
	struct timeval tCur;
	int LedMode = *((int*)arg);
	int MsCycle = 0, CurMs = 0, ActMs = 0, LedVal = 0;

	pthread_detach(pthread_self() );

	s_LedDynaMode = LedMode;

	gettimeofday(&tCur, NULL);
	ActMs = (tCur.tv_sec)*1000 + (tCur.tv_usec)/1000L;
	while(1) {

		switch (s_LedDynaMode) {
			case LED_MIN_MODE:
				MsCycle = LED_MIN_MS;
				break;

			case LED_MAX_MODE:
				MsCycle = LED_MAX_MS;
				break;
			case LED_DARK_MODE:
			case LED_LIGHT_MODE:
				MsCycle = 0x7fffffff;
			default:
				break;
		}
		gettimeofday(&tCur, NULL);
		CurMs = (tCur.tv_sec)*1000 + (tCur.tv_usec)/1000L;
		
		switch (s_LedDynaMode) {
			case LED_MIN_MODE:
			case LED_MAX_MODE:
				{
					if (CurMs > ActMs + MsCycle) {
						BSP_Led_Contrl(LedVal&0x1, true);
						LedVal = !LedVal;
						ActMs = CurMs;
					}else if(CurMs < ActMs){
						ActMs = CurMs;
					}
				}
				break;
			case LED_LIGHT_MODE:
				BSP_Led_Contrl(true, true);
				break;
			case LED_DARK_MODE:
				BSP_Led_Contrl(false, false);
				break;
		}
		usleep(50000);
	}

	return 0;
}

static int s_ledMode = 0;

int initLed(int LedMode)
{
	pthread_t tid;
	static bool LedModeInit = false;

	if (false == LedModeInit) {

		int reval = 0;
		if (LED_MIN_MODE != LedMode && LED_MAX_MODE != LedMode) {
			printf("Error!! The led mode is error : %s , %s !!!\n",__FILE__,  __func__);
			return -1;
		}
		s_ledMode = LedMode;

		reval = pthread_create(&tid, NULL, LedLight, (void*)&s_ledMode);
		if (reval != 0) {
			 printf("%s: %s\n",__func__, strerror(reval));
			 return -1;
		}
		LedModeInit = true;
	}else{
		s_LedDynaMode = LedMode;
	}

	return 0;
}

*/

/////////////////////////////////

static ST_LED_ATTR led_contrl;
static bool  LED_FACTORY_TEST = false;

static void* Led_Light(void *arg)
{
	struct timeval tCur;
	long long MsCycle[LED_NUM], CurMs = 0, ActMs[LED_NUM]; 
	int LedVal[LED_NUM];
	memset(MsCycle,0,sizeof(int)*LED_NUM);
	memset(ActMs,0,sizeof(int)*LED_NUM);
	memset(LedVal,0,sizeof(int)*LED_NUM);

	//pthread_detach(pthread_self() );
	prctl(PR_SET_NAME, "Led_Light");

	int i = 0;
	gettimeofday(&tCur, NULL);
	ActMs[0] = ((long long)tCur.tv_sec)*1000 + (tCur.tv_usec)/1000L;
    for(i=0; i < LED_NUM;i++){
		ActMs[i] = ActMs[0];
	}

    ledTrigger = true;
	while(true == ledTrigger) {
		for(i = 0; i<LED_NUM; i++){
			switch (led_contrl.LedMode[i]) {
				case LED_MIN_MODE:
					MsCycle[i] = LED_MIN_MS;
					break;

				case LED_MAX_MODE:
					MsCycle[i] = LED_MAX_MS;
					break;
				case LED_DARK_MODE:
				case LED_LIGHT_MODE:
					MsCycle[i] = 0x7fffffff;
				default:
					break;
			}
		}


		gettimeofday(&tCur, NULL);
		CurMs = ((long long)tCur.tv_sec)*1000 + (tCur.tv_usec)/1000L;

		for(i = 0; i<LED_NUM; i++){	
			if(true == led_contrl.LedStatus[i]){
				switch (led_contrl.LedMode[i]) {
					case LED_MIN_MODE:
					case LED_MAX_MODE:
						{
							if (CurMs > ActMs[i] + MsCycle[i]) {
								BSP_Led_Contrl(i, LedVal[i]&0x1, true);
								LedVal[i] = !LedVal[i];
								ActMs[i] = CurMs;
							}else if (CurMs < ActMs[i]){
								ActMs[i] = CurMs;
							}

						}
						break;
					case LED_LIGHT_MODE:
							BSP_Led_Contrl(i, true, true);									
						break;
					case LED_DARK_MODE:
							BSP_Led_Contrl(i, false, false);
						break;
					default:
						break;
				}

			}else if(false == led_contrl.LedStatus[i]){
				BSP_Led_Contrl(i, false, false);
			}
			
		}

		usleep(50000);
	}

    pthread_exit(NULL);
	return 0;
}


int initLedContrl(int LedID,bool LedStatus,int LedMode)
{
	static bool LedModeInit = false;
	if(LedID < 0 || LedID >= LED_NUM){
		printf("Error!! The led id is error : %s , %s !!!\n",__FILE__,  __func__);
		return -1;
	}

	if (LED_MIN_MODE != LedMode && LED_MAX_MODE != LedMode && LED_DARK_MODE != LedMode && LED_LIGHT_MODE != LedMode) {
		printf("Error!! The led mode is error : %s , %s !!!\n",__FILE__,  __func__);
		return -1;
	}

	if (false == LedModeInit) {
		memset(&led_contrl,0,sizeof(ST_LED_ATTR));

		int k = 0;
		for(k = 0; k <= LED_NUM-1;k++){
			led_contrl.LedStatus[k] = false;
			led_contrl.LedMode[k] = LED_DARK_MODE;
		}

		int reval = 0;

		led_contrl.LedStatus[LedID] = LedStatus;
		led_contrl.LedMode[LedID] = LedMode;
		
		reval = JA_THREAD_init0(&ledDetectionPid, Led_Light, NULL, NULL, 0, NULL, 131072, 0);
		if (reval != 0) {
			 printf("%s: %s\n",__func__, strerror(reval));
			 return -1;
		}
		LedModeInit = true;
	}else{
        if(LED_FACTORY_TEST == false){
            led_contrl.LedStatus[LedID] = LedStatus;
            led_contrl.LedMode[LedID] = LedMode;
        }
	}

	return 0;
}

int KEY_LED_get_mode(int LedID)
{

	if(LedID < LED_NUM){
		LED_FACTORY_TEST = true ;
		return led_contrl.LedMode[LedID];
	}else{
		return -1;
	}
}

int KEY_LED_set_mode(int LedID,bool LedStatus,int LedMode)
{
	if(LedID < LED_NUM){
		LED_FACTORY_TEST = true ;
		led_contrl.LedStatus[LedID] = LedStatus;
		led_contrl.LedMode[LedID] = LedMode;
		return 0;
	}else{
		return -1;
	}
}

int KEY_LED_set_mode2(int LedID,bool LedStatus,int LedMode)
{
	if(LedID < LED_NUM){
		led_contrl.LedStatus[LedID] = LedStatus;
		led_contrl.LedMode[LedID] = LedMode;
		return 0;
	}else{
		return -1;
	}
}

int KEY_LED_get_keep_time(int LedID)
{
	return 0;
}

int KEY_LED_set_keep_time(int LedID)
{
	return 0;
}

int KEY_LED_ledDestroy()
{
    if(NULL != ledDetectionPid)
    {
        ledTrigger = false;
        pthread_join(ledDetectionPid, NULL);
        ledDetectionPid = (pthread_t)NULL;
        printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
    }

    return 0;

}

#endif

