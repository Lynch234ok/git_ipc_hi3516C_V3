#include <pthread.h>
#include <stdint.h>
#include <sys/prctl.h>
#include <app_debug.h>
#include "msg_push.h"
#include "ticker.h"
#include "securedat.h"
#include <sys/prctl.h>
#include "sound.h"
#include "netsdk.h"

#include <sys/prctl.h>
#include "p2p/p2pdevice.h"
#include "base/cross_thread.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "OSSCloud/kp2p_cloud_message.h"


#if defined(MSG)
typedef struct EseeMsgPush{
	pthread_t pid;
    int msgPushTimer;
    int mdAlarmTimer;
    int inited;
}stEseeMsgPush;

static stEseeMsgPush MsgPushAttr = {
 .pid = (pthread_t)NULL,
 .msgPushTimer = 0,
 .mdAlarmTimer = 0,
 .inited = 0,
};  

void ESEE_msg_push_timer()
{
    if(MsgPushAttr.msgPushTimer > 0)
    {
        MsgPushAttr.msgPushTimer--;
    }

    if(MsgPushAttr.mdAlarmTimer > 0)
    {
        MsgPushAttr.mdAlarmTimer--;
    }

}

static void esee_msg_push_proc()
{	
	static char esee_id[16] = "";
	char sn_str[32] = {0};
    int64_t alarm_ts_s;
    ST_NSDK_SYSTEM_TIME systime;
    int timezone_second;


    pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "esee_msg_push_proc");

    NETSDK_conf_system_get_time(&systime);
    timezone_second = ((abs(systime.greenwichMeanTime) / 100) * 3600
                       + (abs(systime.greenwichMeanTime) % 100) * 60)
                      * (systime.greenwichMeanTime > 0 ? 1 : -1);

    if(0 == strlen(esee_id)){
		if(0 == UC_SNumberGet(sn_str)) {
			if(strlen(sn_str)> 10){
				if(sn_str[strlen(sn_str) - 10] == '0'){
					memcpy(esee_id, &sn_str[strlen(sn_str) - 10+1], 9);
				}else{
					memcpy(esee_id, &sn_str[strlen(sn_str) - 10], 10);
				}
			}
		}
	}
	printf("pid%d--esee ID: %s\n", MsgPushAttr.pid, esee_id);

#ifdef DANA_P2P
    //printf("%s[%d]:dana alarm msg send\n", __FUNCTION__, __LINE__);
    P2P_danaAlarmMsgSend();
#else
    //printf("%s[%d]:esee alarm msg send\n", __FUNCTION__, __LINE__);
    alarm_ts_s = (uint64_t)(time(NULL) + timezone_second);
    Esee_msg_send(esee_id, "motion detection", "md", alarm_ts_s);
#endif
	MsgPushAttr.pid = (pthread_t)NULL;
	printf("%s end\n", __FUNCTION__);
}

#define ALARM_PICTURE_FILE_NAME     "/tmp/snapshot_alarm.jpg"
static void esee_msg_push_proc2()
{
#if defined(MSG_PICTURE)
    int ret = 0;
    FILE *picFid = NULL;
    char fileName[32] = {0};

    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "esee_msg_push");


    picFid = fopen(ALARM_PICTURE_FILE_NAME, "wb+");

    /**
     *  如果截图成功就上传报警信息和报警截图文件，失败则只上传报警信息不上传文件
     */
    if((NULL != picFid)
        && (0 == sdk_enc->snapshot(0, kSDK_ENC_SNAPSHOT_QUALITY_HIGHEST, 640, 360, picFid)))
    {
        ret = KP2P_CloudMessagePushFile("motion detection", "md",
                                            time(NULL), 0, ALARM_PICTURE_FILE_NAME, KP2P_CLOUD_MESSAGE_PIC_JPG);
    }
    else
    {
        ret = KP2P_CloudMessagePushFile("motion detection", "md",
                                            time(NULL), 0, NULL, KP2P_CLOUD_MESSAGE_PIC_JPG);
        APP_TRACE("snapshot alarm picture failed!!!");
    }

    if(0 == ret)
    {
        APP_TRACE("KP2P_CloudMessagePushFile upload alarm picture %s and arlam message success", ALARM_PICTURE_FILE_NAME);
    }
    else
    {
        APP_TRACE("KP2P_CloudMessagePushFile upload file %s and arlam message failed", ALARM_PICTURE_FILE_NAME);
    }

    if(NULL != picFid)
    {
        fclose(picFid);
        picFid = NULL;
    }

    MsgPushAttr.pid = (pthread_t)NULL;
#endif

}

int ESEE_msg_push()
{
    ST_NSDK_SYSTEM_SETTING sysInfo;

    if(0 == MsgPushAttr.inited)
    {
        return -1;
    }

    if((0 >= MsgPushAttr.msgPushTimer) && (NULL == MsgPushAttr.pid))
    {
        MsgPushAttr.msgPushTimer = 60;
#if defined(MSG_PICTURE)
		JA_THREAD_init0(&MsgPushAttr.pid, esee_msg_push_proc2, NULL, NULL, 0, NULL, 131072, 0);
#else
        JA_THREAD_init0(&MsgPushAttr.pid, esee_msg_push_proc, NULL, NULL, 0, NULL, 131072, 0);
#endif

	}

    NETSDK_conf_system_get_setting_info(&sysInfo);
    if((0 >= MsgPushAttr.mdAlarmTimer) && (true == sysInfo.mdAlarm.MotionWarningTone))
    {
        MsgPushAttr.mdAlarmTimer = 7;
        if((true == IS_FILE_EXIST(DST_FILE_PATH_CST_SOUND)) && (0 == strcmp(sysInfo.mdAlarm.WarningToneType, "custom")))//文件存在 && system_1.json是custom
        {
            SearchFileAndPlay(SOUND_Alarm_custom, NK_True);
        }
        else
        {
            SearchFileAndPlay(SOUND_Alarm, NK_True);
        }
	}
	return 0;
}

int ESEE_msg_push_init()
{
    static char esee_id[16] = "";
    char sn_str[32] = {0};

    TICKER_add_task(ESEE_msg_push_timer, 1, false);

    if(0 == strlen(esee_id))
    {
        if(0 == UC_SNumberGet(sn_str))
        {
            if(strlen(sn_str) > 10)
            {
                if(sn_str[strlen(sn_str) - 10] == '0')
                {
                    memcpy(esee_id, &sn_str[strlen(sn_str) - 10 + 1], 9);
                }
                else
                {
                    memcpy(esee_id, &sn_str[strlen(sn_str) - 10], 10);
                }
            }
        }
    }

#if defined(MSG_PICTURE)
    KP2P_CloudMessageInit(NULL, esee_id);
#endif

    MsgPushAttr.inited = 1;

}

void ESEE_msg_destroy()
{
    MsgPushAttr.inited = 0;
    TICKER_del_task(ESEE_msg_push_timer);
    KP2P_CloudMessageDeinit();
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
}

#endif //defined MSG
