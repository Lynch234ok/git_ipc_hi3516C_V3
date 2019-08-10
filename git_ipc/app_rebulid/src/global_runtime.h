
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef GLOBAL_RUNTIME_H_
#define GLOBAL_RUNTIME_H_
#ifdef __cplusplus
extern "C" {
#endif

#define REMOTE_UPGRADE_RESTART_FLAG_PATH "/tmp/ota_flag"
#define TFCARD_OLD_RECORD_PATH	"/media/tf/record"

/* ��debugʱ�����Ա�ǿ���/�رչ��� */
#define G_DEBUG_FLAG_FILE      "/media/tf/debug.ini"

#define G_NK_MAX_TWOWAYTALK_NUM	(1)
#define G_NK_MAX_PLAYBACK_NUM	(2)

#if defined(HI3516E_V1)     // 16e优化内存后，custom改为应用分区挂载
#define IPCAM_ENV_HOME_DIR "/media/custom/ipcam"
#else
#define IPCAM_ENV_HOME_DIR "/usr/share/ipcam"
#endif

extern char g_esee_id[32];
extern bool g_authorized;
extern uint32_t g_hardware_version;
extern int g_encryp_chip_type;
extern uint32_t g_encryp_chip_odm1;
extern uint32_t g_encryp_chip_odm2;
extern int g_sensor_type;

extern int GLOBAL_event_lock_init();
extern int GLOBAL_event_lock_deinit();
extern int GLOBAL_event_lock();
extern int GLOBAL_event_unlock();
extern int GLOBAL_enter_twowaytalk();
extern int GLOBAL_leave_twowaytalk();
extern int GLOBAL_enter_playback();
extern int GLOBAL_leave_playback();
extern void GLOBAL_remove_conf_file();
extern uint32_t GLOBAL_get_sys_mem_mb();
extern uint32_t GLOBAL_reboot_system();
extern void GLOBAL_exit();

extern int GLOBAL_enter_live();
extern int GLOBAL_leave_live();

/*
����:��һ���ļ��ж�ȡstruct timeval��ʽʱ��,Ȼ������ϵͳʱ��
*/
extern int32_t GLOBAL_setTimeFromFile(void);

/*
����:����struct timeval��ʽ��ʱ�䵽�ļ���
*/
extern int32_t GLOBAL_saveFileFromTime(void);

/*
	����Ƿ���ھɸ�ʽ¼���ļ�
	��Ҫ���ڳ�������ʱ��ǣ�����ֱ�Ӳ�ѯGLOBAL_isOldTypeRecord�ӿ��ж��Ƿ��оɸ�ʽ¼��
*/
extern void GLOBAL_setOldTypeRecordFlag();

/*
	���ھɸ�ʽ¼�񷵻�true��falseû�оɸ�ʽ¼��
*/
extern bool GLOBAL_isOldTypeRecord();

extern bool GLOBAL_sn_front();

extern void GLOBAL_cpuDetectInit();
extern void GLOBAL_cpuDetectDestroy();

extern void GLOBAL_IOTdaemonExit();

extern int GLOBAL_GetID(char * ID_String);

extern int GLOBAL_before_upgrade_destroy();

#ifdef __cplusplus
};
#endif
#endif //GLOBAL_RUNTIME_H_

