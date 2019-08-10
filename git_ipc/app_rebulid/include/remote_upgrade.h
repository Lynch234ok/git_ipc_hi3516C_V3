#ifndef _REMOTE_UPGRADE_H_
#define _REMOTE_UPGRADE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#define REMOTE_UPGRADE_TEST_FLAG_FILE "/tmp/ota_test"

enum{
	REMOTE_UPGRADE_ERROR_NONE = 0,
	REMOTE_UPGRADE_ERROR_GETHOSTBYNAME,
	REMOTE_UPGRADE_ERROR_NO_SERVER_RESPONSE,
	REMOTE_UPGRADE_ERROR_XML_PRASE_ERROR,
	REMOTE_UPGRADE_ERROR_HARDWARE_SN_ERROR,
	REMOTE_UPGRADE_ERROR_SOC_ERROR,
	REMOTE_UPGRADE_ERROR_MD5_ERROR,
	REMOTE_UPGRADE_ERROR_VERSION_LIMIT,
	REMOTE_UPGRADE_ERROR_OEM_RESOURCE_LIMIT,
	REMOTE_UPGRADE_ERROR_SKIP_ROOTFS,
	REMOTE_UPGRADE_ERROR_NEEDNT_TO_UPDATE,
};

enum{
	OTA_STATUS_START = 0,
	OTA_STATUS_SUCCESS,
	OTA_STATUS_FAILED,
};

typedef int (*fOTA_STATUS)(int status);
typedef int (*fOTA_FLASH_SYNC)(char *file_path, void *pUpdataMem, int dataSize);
typedef int (*fOTA_ENV_PREPARE)(bool need_reboot);
typedef int (*fOTA_GET_RATE_OF_PROCESS)(void);

typedef struct REMOTE_UPGRADE_FUNCTION{
	fOTA_STATUS ota_status_trap;
	fOTA_FLASH_SYNC flash_sync;
	fOTA_ENV_PREPARE env_prepare;
	fOTA_GET_RATE_OF_PROCESS get_rate;
}ST_REMOTE_UPGRADE_FUNCTION, *LP_REMOTE_UPGRADE_FUNCTION;

typedef struct remote_upgrade_devinfo{
	char detail_path[128];
	char FWversion[32]; 
	char model[32];
	char OEMnum[32]; 
	char FWMagic[128];
	char firmware_path[128];
    char deviceSN[32];
	ST_REMOTE_UPGRADE_FUNCTION function;
}ST_REMOTE_UPGRADE_DEVINFO, *LP_REMOTE_UPGRADE_DEVINFO;


extern int OTA_init(bool need_reboot, LP_REMOTE_UPGRADE_DEVINFO attr);
extern int OTA_start();
extern int OTA_stop();
extern int OTA_get_rate();
extern int OTA_get_status();
extern int OTA_get_errno();



#ifdef __cplusplus
}
#endif

#endif //_REMOTE_UPGRADE_H_


