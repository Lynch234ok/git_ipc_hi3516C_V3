#ifndef _TF_CARD_H
#define _TF_CARD_H
#include <tfcard.h>
#include <pthread.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include "netsdk_def.h"

typedef struct TF_RECORD_PARAMETER{
	NK_Char record_type;//motion,or time 
	NK_Char motion_check;
	NK_Char  is_on_schedule_time;
    ST_NSDK_SYSTEM_REC_MANAGER recManager;
} ST_TF_RECORD_PARAMETER,*LP_TF_RECORD_PARAMETER;

typedef struct {
	NK_UInt8  RunningFlag;  //ThreadRunning
	NK_UInt8  LockingFlag;  //Whether to Exit Thread(Non-Lock)
	NK_UInt8  Update;     //Wthether to Update firame
	ST_TF_RECORD_PARAMETER st_TF_record_parameter;
	pthread_t PthreadID;
} TF_RL_Pack_t;


extern NK_Int TFCARD_record_start(NK_PChar record_type);
extern int TFCARD_start_record_thread();
extern int TFCARD_stop_record(void);
extern int TFCARD_init(int use_samba, char *path);
extern void TFCARD_destroy();

extern int TFCARD_is_record_now();

extern int TFCARD_start_record(void);

#ifdef __cplusplus
};
#endif
#endif

