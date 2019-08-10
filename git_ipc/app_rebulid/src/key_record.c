#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <app_tfcard.h>
#include <pthread.h>
#include "key_record.h"
#include <sys/prctl.h>

#if defined (PX_720)

static pthread_t rec_start_tid = (pthread_t)NULL;
static bool rec_start_trigger = false;

static void *key_rec_start(void *arg)
{
    rec_start_trigger = true;
	prctl(PR_SET_NAME, "key_rec_start");
    int ret = -1;
    while(rec_start_trigger) {
#if defined(TFCARD)
        ret = TFCARD_record_start("rec_key");
#endif
        if(ret < 0) {
            rec_start_trigger = false;
        }
        else {
            sleep(1);
        }
    }

    rec_start_tid = (pthread_t)NULL;
    pthread_exit(NULL);
}

static void key_rec_stop()
{
    rec_start_trigger = false;
    pthread_join(rec_start_tid, NULL);
    rec_start_tid = (pthread_t)NULL;

}

void KEY_REC_control(void)
{
    static bool start_key_rec = false;
    int ret = 0;
#if defined(TFCARD)
    ret = TFCARD_is_record_now();
#endif
    if(ret && !start_key_rec) {  // 没有进行按键录像下，检测日程录像是否在进行中
        printf("recording...!\n");
    }
    else {
        /* 开启按键录像管理线程，在已开始按键录像下，再按一下按键将会结束按键录像 */
        if(!rec_start_tid) {
            if(pthread_create(&rec_start_tid, 0, key_rec_start, NULL) == 0) {
                start_key_rec = true;
            }
        }
        else {
            key_rec_stop();
            start_key_rec = false;
        }
    }

}

#endif

